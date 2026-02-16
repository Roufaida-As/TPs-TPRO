#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <errno.h>

#pragma comment(lib, "ws2_32.lib")

// Correction des types pour Windows (MinGW 64-bit)
typedef int socklen_t;
// On ne redéfinit pas ssize_t car il existe déjà dans votre environnement
#define close closesocket

#define elt(t,i,j) (t)[(i)*(N)+(j)]

typedef struct tconf {
    char *mat;
    double score;
} Conf;

int di[8] = { 0,+1,+1,+1, 0,-1,-1,-1 } ;
int dj[8] = {+1,+1, 0,-1,-1,-1, 0,+1 } ;
int tmo = 10, N = 7, M = 4;
SOCKET server_sock, client1_sock, client2_sock;
struct sockaddr_in server_addr, client1_addr, client2_addr;
char n1[40], n2[40];

void initConf(Conf *c) {
    c->mat = (char*)malloc(N * N * sizeof(char));
    for (int i = 0; i < N * N; i++) c->mat[i] = ' ';
    c->score = 0;
}

int terminale(Conf c, int *cout) {
    int i, j, k, r, a, b, matchNul = 1;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (elt(c.mat, i, j) != ' ') {
                for (k = 0; k < 4; k++) {
                    a = i + di[k]; b = j + dj[k]; r = 1;
                    while (r < M && a < N && a >= 0 && b < N && b >= 0 && elt(c.mat, a, b) == elt(c.mat, i, j)) {
                        a += di[k]; b += dj[k]; r++;
                    }
                    if (r == M) {
                        *cout = (elt(c.mat, i, j) == 'X') ? 1 : -1;
                        return 1;
                    }
                }
            } else matchNul = 0;
        }
    }
    if (matchNul) { *cout = 0; return 1; }
    return 0;
}

void displayConf(Conf c) {
    printf("\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) printf("|%c", elt(c.mat, i, j));
        printf("|\n");
    }
}

void game(Conf c) {
    int cout, stop = 0;
    char tour = 'X', buf[1024];
    int i, j;
    // Timeout de tmo secondes + 20ms
    DWORD timeout = tmo * 1000 + 20; 

    // Sous Windows, setsockopt utilise DWORD pour le timeout
    setsockopt(client1_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(client2_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    while (!stop) {
        displayConf(c);
        SOCKET current_clt = (tour == 'X') ? client1_sock : client2_sock;
        memset(buf, 0, sizeof(buf));
        
        printf("Attente du coup de %c (%s)...\n", tour, (tour == 'X' ? n1 : n2));
        int rbytes = recv(current_clt, buf, sizeof(buf), 0);

        if (rbytes <= 0) {
            printf("TEMPS ECOULE ou deconnexion pour le joueur %c\n", tour);
            stop = 1; cout = (tour == 'X') ? -1 : 1;
        } else {
            if (sscanf(buf, "%d %d", &i, &j) == 2) {
                if (i >= 0 && i < N && j >= 0 && j < N && elt(c.mat, i, j) == ' ') {
                    elt(c.mat, i, j) = tour;
                    printf("Joueur %c joue en <%d,%d>\n", tour, i, j);
                    sprintf(buf, "%c at %d %d\n", tour, i, j);
                    send((tour == 'X' ? client2_sock : client1_sock), buf, strlen(buf), 0);
                    
                    if (terminale(c, &cout)) stop = 1;
                    else tour = (tour == 'X' ? 'O' : 'X');
                } else {
                    printf("Coup illegal de %c\n", tour);
                    stop = 1; cout = (tour == 'X') ? -1 : 1;
                }
            }
        }
    }
    
    displayConf(c);
    if (cout == 1) printf("GAGNANT : %s (X)\n", n1);
    else if (cout == -1) printf("GAGNANT : %s (O)\n", n2);
    else printf("MATCH NUL\n");

    sprintf(buf, "gameover cost = %d\n", cout);
    send(client1_sock, buf, strlen(buf), 0);
    send(client2_sock, buf, strlen(buf), 0);
}

int main(int argc, char *argv[]) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    if (argc >= 3) { N = atoi(argv[1]); M = atoi(argv[2]); }
    if (argc == 4) tmo = atoi(argv[3]);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 2);

    printf("SERVEUR TicTacToe NxN lancé sur le port 12345\n");
    printf("Parametres: N=%d, M=%d, Timeout=%ds\n", N, M, tmo);
    printf("En attente des joueurs...\n");

    socklen_t addr_size = sizeof(client1_addr);
    client1_sock = accept(server_sock, (struct sockaddr*)&client1_addr, &addr_size);
    recv(client1_sock, n1, sizeof(n1), 0);
    strtok(n1, "\n\r"); // Nettoyer le nom
    printf("Joueur 1 (X) connecté: %s\n", n1);

    client2_sock = accept(server_sock, (struct sockaddr*)&client2_addr, &addr_size);
    recv(client2_sock, n2, sizeof(n2), 0);
    strtok(n2, "\n\r"); // Nettoyer le nom
    printf("Joueur 2 (O) connecté: %s\n", n2);

    char msg[100];
    sprintf(msg, "%d %d %d X\n", N, M, tmo); send(client1_sock, msg, strlen(msg), 0);
    sprintf(msg, "%d %d %d O\n", N, M, tmo); send(client2_sock, msg, strlen(msg), 0);

    Conf c; initConf(&c);
    game(c);

    closesocket(client1_sock); closesocket(client2_sock); closesocket(server_sock);
    WSACleanup();
    return 0;
}