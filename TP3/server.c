// TCP Server for tictactoe (N,M) game : align M symbols in NxN dynamic grid
// interface : text
// Hidouci & Hadim / TP TPRO / 2CS / ESI 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ===== MODIFICATIONS POUR WINDOWS =====
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
// ======================================


// to access t[i][j]
#define elt(t,i,j) (t)[(i)*(N)+(j)]

// state configuration
typedef struct tconf {
    char *mat;
    double score;
} Conf;


/* Functions interface */

// initialize c
void initConf( Conf *c);

// Generate all possible moves for player plr from the current configuration c
// n: the number of possible moves
// T: array containing the n generated configurations
void gen_succ( Conf c, char plr , Conf T[] , int *n );

// Check if c is an end_of_game config
// cout: the cost of c ( -1, 0 or +1 )
int terminale( Conf c , int *cout );

// Display in text mode the current config c
void displayConf( Conf c  );

// Control the progress of a game between the 2 connected players
void game( Conf c );

/*********************/


/* Global variables */

// to test the 8 directions from cell i,j : E, SE, S, SW, W, NW et N
int di[8] = { 0,+1,+1,+1, 0,-1,-1,-1 } ;
int dj[8] = {+1,+1, 0,-1,-1,-1, 0,+1 } ;
// in this program we considere only the first 4 ones E, SE, S et SW
//the other directions are obtained by symmetry from the first 4 ones

// default game-parameter values ...
// timeout (in seconds) for each player
int tmo = 5;
// grid size
int N = 7;
//winning alignment length
int M = 4; /* length of the winning alignment : M <= N */

// sockets
SOCKET server_sock, client1_sock, client2_sock;
struct sockaddr_in server_addr, client1_addr, client2_addr;
int addr_size;

// client names
char n1[40];
char n2[40];

/*********************/


/* main program */

int main( int argc , char *argv[] )
{
    // ===== INITIALISATION WINSOCK =====
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Erreur initialisation Winsock: %d\n", WSAGetLastError());
        return 1;
    }
    // ==================================

    Conf c;
    char ch[20];

    char buffer[1024];
    int i;

    // when calling this program from a command line console, give the following parameters:
    if ( argc == 3 ) { // 2 parameters :
        N = atoi(argv[1]);   // N : the grid size
        M = atoi(argv[2]);   // M : the winiing alignment length
    }
    if ( argc == 4 ) { // or 3 parameters :
        N = atoi(argv[1]);   // N : the grid size
        M = atoi(argv[2]);   // M : the winiing alignment length
        tmo = atoi(argv[3]); // tmo: the time allowed to each player for each move
    }

    // Create the server socket (type:tcp)
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Erreur creation socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind to server ip addr and port number
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);        // server port number = 12345
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Listen on all interfaces
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Erreur bind: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    // wait for clients to connect ...
    if (listen(server_sock, 2) == SOCKET_ERROR) {
        printf("Erreur listen: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    printf("TicTacToe server (grid_size=%d, Align_len=%d, timeout=%d)\n", N, M, tmo);
    printf("listening port number : 12345 ..."); fflush(stdout);

    addr_size = sizeof(client1_addr);
    client1_sock = accept(server_sock, (struct sockaddr*)&client1_addr, &addr_size);
    if (client1_sock == INVALID_SOCKET) {
        printf("Erreur accept client1: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    
    // get the client1 name
    recv(client1_sock, n1, sizeof(n1), 0);

    for (i=0; i<sizeof(n1); i++)
        if (n1[i] == '\n')
            break;
    n1[i] = '\0';
    printf("\nclient1 : \"%s\" is player X\n", n1);

    addr_size = sizeof(client2_addr);
    client2_sock = accept(server_sock, (struct sockaddr*)&client2_addr, &addr_size);
    if (client2_sock == INVALID_SOCKET) {
        printf("Erreur accept client2: %d\n", WSAGetLastError());
        closesocket(client1_sock);
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }
    
    // get the client2 name
    recv(client2_sock, n2, sizeof(n2), 0);

    for (i=0; i<sizeof(n2); i++)
        if (n2[i] == '\n')
            break;
    n2[i] = '\0';
    printf("client2 : \"%s\" is player O\n", n2);

    // send game parameters ( N , M , timeout , player ) to clients
    sprintf( buffer , "%d %d %d X\n", N, M, tmo);
    send(client1_sock, buffer, strlen(buffer)+1, 0);
    sprintf( buffer , "%d %d %d O\n", N, M, tmo);
    send(client2_sock, buffer, strlen(buffer)+1, 0);

    // init the first configuration (an empty grid)
    initConf( &c );

    // main loop
    game( c );

    // Close connections
    closesocket(client1_sock);
    closesocket(client2_sock);
    closesocket(server_sock);

    free(c.mat);

    WSACleanup();

	return 0;

}   // main


/*************************************************************************/


/* Functions implemtation */

// Display (in simple text mode) the current config c
void displayConf( Conf c )
{
    int i,j;
    for (i=0; i<N; i++) {
        for (j=0; j<N; j++)
            printf("|%c",elt(c.mat,i,j));
        printf("|\n");
    }
    printf("\tscore = %d\n",c.score);
}


void game( Conf c )
{
    int cout, stop;
    char tour;
    char buf[20];
    int i,j,k;
    int clt;
    int rbytes;
    DWORD timeout_ms = tmo * 1000 + 20;  // timeout en millisecondes

    // set timeout option for blocking receive operations on client1 socket
    if (setsockopt(client1_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms)) < 0) {
        printf("Error setting SO_RCVTIMEO on client1_sock: %d\n", WSAGetLastError());
        closesocket(client1_sock);
        closesocket(client2_sock);
        closesocket(server_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // set timeout option for blocking receive operations on client2 socket
    if (setsockopt(client2_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms)) < 0) {
        printf("Error setting SO_RCVTIMEO on client2_sock: %d\n", WSAGetLastError());
        closesocket(client1_sock);
        closesocket(client2_sock);
        closesocket(server_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    stop = 0;
    tour = 'X';

    // Main Loop ...
    while ( !stop ) {
        displayConf( c );

        if ( tour == 'X' ) {

            // wait to get a move from client1
            rbytes = recv(client1_sock, buf, sizeof(buf), 0);
            if (rbytes == SOCKET_ERROR) {
               int err = WSAGetLastError();
               if (err == WSAETIMEDOUT) {
                  printf("Receive from client1 timed out\n");
                  stop = 3;
                  cout = -1;
               }
               else {
                 printf("Error during recv from client1: %d\n", err);
                 closesocket(client1_sock);
                 closesocket(client2_sock);
                 closesocket(server_sock);
                 WSACleanup();
                 exit(EXIT_FAILURE);
               }
            }
            else
               if (rbytes == 0) {
                  printf("Connection closed by client1\n");
                  stop = 3;
                  cout = -1;
               }
               else {
                  buf[ sizeof(buf)-1 ] = '\0';
                  sscanf(buf, "%d %d", &i , &j);
                  printf("X at <%d,%d>\n", i , j);
                  if ( i >= 0 && i < N && j >= 0 && j < N && elt(c.mat,i,j) == ' ' ) {
                     elt(c.mat,i,j) = 'X';

                     // inform the other player of the move that has just been made
                     sprintf(buf, "X at %d %d\n", i,j);
                     send(client2_sock, buf, strlen(buf)+1, 0);

                }
                  else {
                     printf("*** illegal move from player X ***\n");
                     stop = 3;
                     cout = -1;
                  }
               }
        }

        else {

            // wait to get a move from client2
            rbytes = recv(client2_sock, buf, sizeof(buf), 0);
            if (rbytes == SOCKET_ERROR) {
               int err = WSAGetLastError();
               if (err == WSAETIMEDOUT) {
                  printf("Client2 timed out\n");
                  stop = 4;
                  cout = 1;
               }
               else {
                 printf("client2_sock : Error during recv: %d\n", err);
                 closesocket(client1_sock);
                 closesocket(client2_sock);
                 closesocket(server_sock);
                 WSACleanup();
                 exit(EXIT_FAILURE);
               }
            }
            else
               if (rbytes == 0) {
                  printf("Connection closed by client2\n");
                  stop = 4;
                  cout = 1;
               }
               else {
                  buf[ sizeof(buf)-1 ] = '\0';
                  sscanf(buf, "%d %d", &i , &j);
                  printf("O at <%d,%d>\n", i , j);
                  if ( i >= 0 && i < N && j >= 0 && j < N && elt(c.mat,i,j) == ' ' ) {
                     elt(c.mat,i,j) = 'O';

                     // inform the other player (client1) of the move that has just been made
                     sprintf(buf, "O at %d %d\n", i,j);
                     send(client1_sock, buf, strlen(buf)+1, 0);

                }
                  else {
                     printf("*** illegal move from player O ***\n");
                     stop = 4;
                     cout = 1;
                  }
               }

        }

        if ( terminale( c , &cout ) )
            stop = 1;
        else
            tour = ( tour == 'X' ? 'O' : 'X' );

    } // end_while (Main Loop)

    displayConf( c );

    if ( cout == 1 )
        printf("%s (X) is the winner\n", n1);
    else
        if ( cout == -1 )
            printf("%s (O) is the winner\n", n2);
        else
            printf("No winner (draw)\n");

    sprintf(buf, "gameover cost = %d\n", cout);
    send(client1_sock, buf, strlen(buf)+1, 0);
    send(client2_sock, buf, strlen(buf)+1, 0);

} // game


// initilize c
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

} // terminale

/*****************************************/