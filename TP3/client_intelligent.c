// Version complète : Hidouci / Hadim + corrections (estimation + timeout)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define elt(t,i,j) (t)[(i)*(N)+(j)]

typedef struct tconf {
    char *mat;
    double score;
    int i, j;
} Conf;

void initConf(Conf *c);
void gen_succ(Conf c, char plr, Conf T[], int *n);
double minmax_ab(Conf conf, char mode, int h, double alpha, double beta);
int terminale(Conf c, int *cout);
double estimation(Conf c, char plr);
void machinejoue(char pl, Conf *c);
int confcmp321(const void *a, const void *b);
int confcmp123(const void *a, const void *b);
void affect(Conf *dest, Conf src);

// directions (8)
int di[8] = {0,+1,+1,+1,0,-1,-1,-1};
int dj[8] = {+1,+1,0,-1,-1,-1,0,+1};

int tmo = 10;   // temps max par coup (sec)
int N = 7;     // taille du plateau
int M = 4;     // alignement gagnant
int HMAX = 2;  // profondeur max

clock_t start_time; // pour le timeout

// === MAIN === //
int main(int argc, char *argv[])
{

    int client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    char ip_serv[40];
    int port_serv, port_client;
    char buf[100];
    char pl_name[40];
    Conf c;
    int i,j,val;
    double cout;
    char pl, adv;

    if (argc > 1)
        HMAX = atoi(argv[1]);
    printf("HMAX = %d\n", HMAX);

    printf("Player name ? : ");
    fgets(pl_name, 40, stdin);
    pl_name[strcspn(pl_name, "\n")] = 0;

    printf("Server IP addr ? (127.0.0.1) : ");
    fgets(ip_serv, 20 , stdin);
    ip_serv[strcspn(ip_serv, "\n")] = 0;
    if (strlen(ip_serv) < 7)
        strcpy(ip_serv, "127.0.0.1");

    printf("Server Port Number ? (12345) : ");
    fgets(buf, 20 , stdin);
    if (strlen(buf) < 2)
        port_serv = 12345;
    else
        sscanf(buf, " %d", &port_serv);

    printf("Server : < ip = %s , port = %d > \n\n", ip_serv, port_serv);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_serv);
    inet_pton(AF_INET, ip_serv, &server_addr.sin_addr);

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Erreur connexion.\n");
        return 1;
    }

    getsockname(client_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    port_client = ntohs(client_addr.sin_port);
    printf("Client use port number : %d\n", port_client);

    send(client_sock, pl_name, strlen(pl_name)+1, 0);

    recv(client_sock, buf, sizeof(buf), 0);
    sscanf(buf, " %d %d %d %c", &N, &M, &tmo, &pl);
    printf("param: N=%d, M=%d, tmo=%d, pl=%c\n", N, M, tmo, pl);

    initConf(&c);
    srand(time(NULL));

    if (pl == 'X') {
        machinejoue(pl, &c);
        sprintf(buf, "%d %d\n", c.i, c.j);
        send(client_sock, buf, strlen(buf)+1, 0);
        printf("my move: %c at %d %d\n", pl, c.i, c.j);
        adv = 'O';
    } else adv = 'X';

    while (1) {
        recv(client_sock, buf, sizeof(buf), 0);
        if (buf[0] == adv) {
            sscanf(buf+5, "%d %d", &i, &j);
            printf("received opponent move: %c at %d %d\n", adv, i, j);
        } else {
            sscanf(buf+15, " %d", &val);
            printf("Game over, cost=%d\n", val);
            close(client_sock);
            return 0;
        }

        elt(c.mat,i,j) = adv;
        machinejoue(pl, &c);
        if (c.i == -1) break;

        sprintf(buf, "%d %d\n", c.i, c.j);
        send(client_sock, buf, strlen(buf)+1, 0);
        printf("my move: %c at %d %d (score=%.2f)\n", pl, c.i, c.j, c.score);
    }

    close(client_sock);
    return 0;
}

// === FONCTIONS === //

double estimation(Conf c, char plr)
{
    // Compte les séquences alignées pour chaque joueur
    int i,j,k,a,b,r;
    int scoreX = 0, scoreO = 0;

    for (i=0; i<N; i++)
      for (j=0; j<N; j++)
        if (elt(c.mat,i,j) != ' ') {
            for (k=0; k<4; k++) {
                int count = 0;
                a=i; b=j;
                while (a>=0 && a<N && b>=0 && b<N && elt(c.mat,a,b)==elt(c.mat,i,j)) {
                    count++;
                    a += di[k]; b += dj[k];
                }
                if (elt(c.mat,i,j)=='X') scoreX += count;
                else scoreO += count;
            }
        }

    double score = (scoreX - scoreO) / (double)(N*N);
    if (plr == 'O') score = -score;
    if (score > 1) score = 1;
    if (score < -1) score = -1;
    return score;
}

int terminale(Conf c, int *cout)
{
    int i,j,a,b,r,k;
    int matchNul = 1;

    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            if (elt(c.mat,i,j) != ' ') {
               for (k=0; k<4; k++) {
                   a = i+di[k]; b = j+dj[k]; r = 1;
                   while (r<M && a<N && a>=0 && b<N && b>=0) {
                       if (elt(c.mat,a,b)!=elt(c.mat,i,j)) break;
                       a+=di[k]; b+=dj[k]; r++;
                   }
                   if (r==M) {
                      *cout = (elt(c.mat,i,j)=='X') ? 1 : -1;
                      return 1;
                   }
               }
            } else matchNul = 0;

    if (matchNul) { *cout=0; return 1; }
    return 0;
}

double minmax_ab(Conf conf, char mode, int h, double alpha, double beta)
{
    if ((clock() - start_time) / CLOCKS_PER_SEC > tmo)
        return estimation(conf, mode);

    int n,i,j,cout;
    double score,score2;
    Conf T[N*N];

    if (terminale(conf,&cout)) return cout;
    if (h==0) return estimation(conf, mode);

    if (mode=='X') {
        gen_succ(conf,'X',T,&n);
        score=alpha;
        for (i=0;i<n;i++){
            score2=minmax_ab(T[i],'O',h-1,score,beta);
            if (score2>score) score=score2;
            if (score>=beta){for(j=0;j<n;j++)free(T[j].mat);return score;}
        }
    } else {
        gen_succ(conf,'O',T,&n);
        score=beta;
        for (i=0;i<n;i++){
            score2=minmax_ab(T[i],'X',h-1,alpha,score);
            if (score2<score) score=score2;
            if (score<=alpha){for(j=0;j<n;j++)free(T[j].mat);return score;}
        }
    }

    for (j=0;j<n;j++) free(T[j].mat);
    return score;
}

void machinejoue(char pl, Conf *c)
{
    int k,n;
    Conf T[N*N];
    double cout,val;

    c->i=-1; c->j=-1;
    start_time = clock();

    if (pl=='O'){
        cout=+2;
        gen_succ(*c,'O',T,&n);
        for(k=0;k<n;k++){
            val=minmax_ab(T[k],'X',HMAX,-2,cout);
            if(val<cout){cout=val; affect(c,T[k]); c->score=cout;}
        }
    } else {
        cout=-2;
        gen_succ(*c,'X',T,&n);
        for(k=0;k<n;k++){
            val=minmax_ab(T[k],'O',HMAX,cout,+2);
            if(val>cout){cout=val; affect(c,T[k]); c->score=cout;}
        }
    }
    for(k=0;k<n;k++) free(T[k].mat);
}

void initConf(Conf *c)
{
    int i,j;
    c->mat = malloc(N*N*sizeof(char));
    for(i=0;i<N;i++)
        for(j=0;j<N;j++)
            elt(c->mat,i,j)=' ';
    c->score=0; c->i=-1; c->j=-1;
}

void gen_succ(Conf c,char plr,Conf T[],int *n)
{
    int i,j;
    Conf d;
    *n=0;
    affect(&d,c);
    for(i=0;i<N;i++)
        for(j=0;j<N;j++)
            if(elt(c.mat,i,j)==' '){
                elt(d.mat,i,j)=plr;
                affect(&(T[*n]),d);
                T[*n].i=i; T[*n].j=j;
                (*n)++;
                free(d.mat);
                affect(&d,c);
            }
}

void affect(Conf *dest, Conf src)
{
    int i,j;
    dest->mat = malloc(N*N*sizeof(char));
    for(i=0;i<N;i++)
        for(j=0;j<N;j++)
            elt(dest->mat,i,j)=elt(src.mat,i,j);
    dest->score=src.score;
    dest->i=src.i;
    dest->j=src.j;
}
