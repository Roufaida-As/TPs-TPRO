// Example of TCP Client tictactoe NxN
// this client implements a standard minmax with alpha/beta pruning algorithm
// - the "Estimation function" returns random scores
// - the "timeout" is not taken into account
// TODO:
// - implement an effective estimation function
// - limit the exploration time to at most TMO seconds
// Hidouci & Hadim / TP3 TPRO / 2CS / ESI 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

// access to 2D-dynamic arrays t[N][N] : elt(t,i,j) --> t[i,j]
#define elt(t,i,j) (t)[(i)*(N)+(j)]


// type of a configuration state
typedef struct tconf {
    char *mat;     // the 2d grid (NxN) of chars
    double score;
    int i,j;       // the last move (coord of the last updated cell)
} Conf;


/* Functions interface */

void initConf( Conf *c);
void gen_succ( Conf c, char plr , Conf T[] , int *n );
double minmax_ab( Conf conf, char mode , int h , double alpha , double beta );
int terminale( Conf c , int *cout );
double estimation( Conf c , char plr );
void machinejoue( char pl, Conf *c );
int confcmp321(const void *a, const void *b);
int confcmp123(const void *a, const void *b);
void affect( Conf *dest , Conf src );


/* Global variables */

// indice increments to test the 8 directions in a grid
int di[8] = { 0,+1,+1,+1, 0,-1,-1,-1 } ;
int dj[8] = {+1,+1, 0,-1,-1,-1, 0,+1 } ;

// default game parameters
int tmo = 10;  // timeout allowed to the player (actually not used)
int N = 7;    // grid size ( N x N )
int M = 4;    // length of winning alignment in the 8 directions
int HMAX = 2; // maximum exploration depth


/********************************************************/

int main( int argc , char *argv[] )
{
    // socket to connect with the game server
    int client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char ip_serv[40];
    int port_serv, port_client;

    char buf[100];
    char pl_name[40];

    Conf c;
    int i, j, val;
    double cout;

    char pl;  // the actual player  : 'X' or 'O'
    char adv; // the opponent player: 'O' or 'X'

    if ( argc > 1 ) { // get HMAX from command line parameter
        HMAX = atoi(argv[1]);
    }
    printf("HMAX = %d\n", HMAX);

    // get other param from std input...
    printf("Player name ? : ");
    fgets(pl_name, 40, stdin);
    pl_name[39] = '\0';
    pl_name[ strlen(pl_name)-1] = '\0';  // to discard '\n' from the input

    printf("Server IP addr ? (127.0.0.1) : ");
    fgets(ip_serv, 20 , stdin);
    ip_serv[ strlen(ip_serv)-1 ] = '\0'; // to discard '\n'
    if ( strlen(ip_serv) < 7 )
        strcpy( ip_serv, "127.0.0.1" );

    printf("Server Port Number ? (12345) : ");
    fgets(buf, 20 , stdin);
    if ( strlen(buf) < 2 )
        port_serv = 12345;
    else
        sscanf(buf, " %d", &port_serv);

    printf("Server : < ip = %s , port = %d > \n\n", ip_serv, port_serv);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_serv);
    inet_pton(AF_INET, ip_serv, &server_addr.sin_addr);

    // Connect to server ...
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("Erreur creation socket");
        return 1;
    }

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erreur connexion");
        close(client_sock);
        return 1;
    }

    // get informations on client socket
    if (getsockname(client_sock, (struct sockaddr *)&client_addr, &client_addr_len) == -1) {
       perror("Erreur lors de la récupération des informations du socket");
       close(client_sock);
       exit(EXIT_FAILURE);
    }
    // client port number
    port_client = ntohs(client_addr.sin_port);
    printf("Client use port number : %d\n", port_client);

    // send player name to server
    send(client_sock, pl_name, strlen(pl_name)+1, 0);
    printf("name sent to server = \"%s\"\n", pl_name);

    // get game parameters from the server :
    // N (gridSize) M(alignmentLength) tmo(timeout allowed for one move) pl(playerType : 'X' or 'O')
    recv(client_sock, buf, sizeof(buf), 0);
    sscanf( buf, " %d %d %d %c", &N, &M, &tmo, &pl );
    printf("param received: N=%d, M=%d, tmo=%d, pl=%c\n", N, M, tmo, pl);

    initConf( &c );

    // initialize random numbers generator
    srand( time(NULL));

    // the first move to start the game is from player 'X' ...
    if ( pl == 'X' ) {
        // if this client is player 'X', then choose its first move, the result conf is c
        machinejoue(pl , &c );

        i = c.i;
        j = c.j;
        if ( i == -1 ) {
            i = 0;
            j = 0;
            elt( c.mat,i,j) = 'X';
        }

        // respond to the server with the first chosen move (i,j) ...
        sprintf(buf, "%d %d\n", i, j);
        send(client_sock, buf, strlen(buf)+1, 0);
        printf("my move : %c at %d %d\n", pl, i, j);

        // the opposing player is 'O'
        adv = 'O';

    }
    else {
        // if this client is player 'O' then just wait the prompt from server ...
        // the opposing player is 'X'
        adv = 'X';
    }

    while (1) {

        // wait the prompt from server (containing also the last opponent player move)
        recv(client_sock, buf, sizeof(buf), 0);
        if ( buf[0] == adv ) {
            sscanf( buf+5, "%d %d", &i, &j );
            printf("received opponent move: %c at %d %d\n", adv, i, j);
        }
        else {
            // gameover
            sscanf( buf+15, " %d", &val);
            printf("received an end_of_game: gameover cost = %d\n", val);
            close( client_sock );
            break;
        }

        elt(c.mat,i,j) = adv;

        // choose a move ...
        machinejoue(pl , &c );

        i = c.i;
        j = c.j;
        if ( i == -1 ) {
            close( client_sock );
            break;
        }

        // respond to the server with the chosen move (i,j) ...
        sprintf(buf, "%d %d\n", i, j);
        send(client_sock, buf, strlen(buf)+1, 0);
        printf("my move : %c at %d %d (cost=%5.2f)\n", pl, i, j, c.score);

    } // end_while (1)



    return 0;

}  // main



/* Functions implementation */


// Function for estimating the quality of a configuration 'c' for player 'plr'
double estimation( Conf c , char plr )
{
    // *** REPLACE WITH YOUR OWN ESTIMATION ***
    return ( ( (double)(rand())/RAND_MAX ) - 0.5 ) * 1.9;
}


// check if configuration c is an endgame
int terminale( Conf c , int *cout )
// cout : cost (-1, 0 or +1) of final config c
{
    int i,j,a,b,r,k;
    int matchNul = 1;

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if ( elt(c.mat,i,j) != ' ' ) {
               // check the 4 first directions from cell mat[i][j] ...
               // (the other 4 remaining directions will be checked by symmetry)
               for (k=0; k<4; k++) {
                   a = i+di[k];
                   b = j+dj[k];
                   r = 1;
                   while ( r<M && a<N && a>=0 && b<N && b>=0 )
                   {
                       if ( elt(c.mat,a,b) != elt(c.mat,i,j) )
                           break;
                       else {
                           a += di[k];
                           b += dj[k];
                           r++;
                       }
                   }
                   if ( r== M ) {
                      if (elt(c.mat,i,j) == 'X')
                           c.score = *cout = 1;
                      else
                           c.score = *cout = -1;
                      return 1;
                   }
               } // for k
            }
            else // (i.e.  c.mat[i][j] is ' ' )
                matchNul = 0;
        // end_for j
    //end_for i

    if ( matchNul ) {
       // all cells are occupied and there is no victory for any player, so it is a draw (cost = 0)
       c.score = *cout = 0;
       return 1;
    }

    // there is no victory and there is at least one empty cell ==> c is non-terminal conf
    return 0;
}


// minmax with alpha/beta pruning
// explores at most h levels, starting from config conf
double minmax_ab( Conf conf, char mode , int h , double alpha, double beta )
{
   int n, i, j, cout;
   double score, score2;
   Conf T[N*N];


   // If we have reached the end of the game, we return the score: -1, 0 or +1
   if ( terminale(conf, &cout) )
      return cout;   // cout = -1, 0 or +1

   // if we reach the maximum depth of exploration, we return an estimate in ]-1 , +1[
   if ( h == 0 )
       return estimation( conf , mode );

   // otherwise we continue to descend (recursively) in the tree...
   if ( mode == 'X' ) {

      gen_succ( conf, 'X', T, &n );

      score = alpha;
      for ( i=0; i<n; i++ ) {
          score2 = minmax_ab( T[i], 'O' , h-1 , score , beta );
          if (score2 > score)
              score = score2;
          if (score >= beta) {
              // free T[]...
              for (j=0; j<n; j++)
                  free(T[j].mat);

              return score;   // Beta type pruning
          }
      }
   }
   else  { // i.e. mode == 'O'

      gen_succ( conf, 'O', T , &n );

      score = beta;
      for ( i=0; i<n; i++ ) {
          score2 = minmax_ab( T[i], 'X' , h-1 , alpha , score );
          if (score2 < score)
              score = score2;
          if (score <= alpha) {
              // free T[]...
              for (j=0; j<n; j++)
                  free(T[j].mat);

              return score;   // Alpha type pruning
          }
      }

   } // end [if ( mode == 'X' ) ... else ( mode == 'O' ) ...]

   // free T[]...
   for (j=0; j<n; j++)
        free(T[j].mat);

   return score;

} // end of minmax


// choose a move (c) for player pl
// the returned conf c contains i=-1 and j=-1 when there in no legal move for player pl
// otherwise < c.i , c.j > is the new position for the player's symbol
void machinejoue( char pl, Conf *c )
{
    int k;
    Conf T[N*N];
    int n;
    double cout, val;
    char adv;

    c->i = -1;
    c->j = -1;

    if ( pl == 'O') {
       cout = +2;    // +infinity
       gen_succ( *c , 'O' , T , &n );
       for (k=0; k<n ; k++)  {
              val = minmax_ab( T[k] , 'X' , HMAX , -2 , cout );
              if ( val < cout) {
                  cout = val;
                  affect(c,T[k]);
                  c->score = cout;
              }
       }

    }
    else {  //i.e. pl == 'X'
       cout = -2;    // -infinity
       gen_succ( *c , 'X' , T , &n );
       for (k=0; k<n; k++) {
              val = minmax_ab( T[k] , 'O' , HMAX , cout , +2 );
              T[k].score = val;
              if ( val > cout) {
                  cout = val;
                  affect(c,T[k]);
                  c->score = cout;
              }
       } //for k

    } // end if pl == 'O' ... else ...

    // free T[]
    for (k=0; k<n; k++)
        free(T[k].mat);

} // end machinejoue


void initConf( Conf *c)
{
    int i,j;

    c->mat = malloc( N*N * sizeof(char) );
    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
           //c->mat[i][j] = ' ';
           elt(c->mat,i,j) = ' ';
        }
    }
    c->score = 0;
    c->i = -1;
    c->j = -1;
}


void gen_succ( Conf c, char plr , Conf T[] , int *n )
{
    int i,j;
    Conf d;

    *n = 0;

    affect(&d,c);
    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if ( elt(c.mat,i,j) == ' ' ) {
                elt(d.mat,i,j) = plr;

                affect(&(T[ *n ]) , d);

                T[ *n ].i = i;
                T[ *n ].j = j;
                (*n)++;

                free( d.mat );
                affect(&d,c);
            }

}  // end gen_succ


/* Comparison functions used with qsort     */
// 'a' and 'b' are configurations
int confcmp123(const void *a, const void *b)        // ascending order
{
    double x = ((Conf *)a)->score, y = ((Conf *)b)->score;
    if ( x < y )
        return -1;
    if ( x == y )
        return 0;
    return 1;
}  // end confcmp123


int confcmp321(const void *a, const void *b)        // descending order
{
    double x = ((Conf *)a)->score, y = ((Conf *)b)->score;
    if ( x < y )
        return 1;
    if ( x == y )
        return 0;
    return -1;
}  // end confcmp321


// Conf assignment : dest <-- src
void affect( Conf *dest , Conf src )
{
    int i,j;

    dest->mat = malloc( N*N * sizeof(char) );
    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            elt(dest->mat,i,j) = elt(src.mat,i,j);
    dest->score = src.score;
    dest->i = src.i;
    dest->j = src.j;
}