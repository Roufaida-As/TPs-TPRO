/*
Enhanced Taquin 15 (4x4) – A* and WA* with step-by-step animation in GTK
- Heap-based priority queue (O(log n))
- Heuristics: misplaced tiles and Manhattan
- WA*: f = g + p*h
- Animated GUI solution display

Compile (WSL with GTK):
  sudo apt install libgtk-3-dev
  gcc tp4.c -o taquin `pkg-config --cflags --libs gtk+-3.0` -O2
  ./taquin
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 4
#define MAX_HEAP 2000000

typedef char conf[N][N];
conf BUT = {
    {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 0}};

typedef struct noeud
{
    conf m;
    int g, h;
    double f;
    struct noeud *pere;
} Noeud;

/* HEAP */
typedef struct
{
    Noeud **tab;
    int n;
} Heap;
void heap_init(Heap *h)
{
    h->tab = malloc(sizeof(Noeud *) * MAX_HEAP);
    h->n = 0;
}
void heap_swap(Noeud **a, Noeud **b)
{
    Noeud *t = *a;
    *a = *b;
    *b = t;
}
void heap_push(Heap *h, Noeud *x)
{
    int i = h->n++;
    h->tab[i] = x;
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (h->tab[p]->f <= h->tab[i]->f)
            break;
        heap_swap(&h->tab[p], &h->tab[i]);
        i = p;
    }
}
Noeud *heap_pop(Heap *h)
{
    if (h->n == 0)
        return NULL;
    Noeud *res = h->tab[0];
    h->tab[0] = h->tab[--h->n];
    int i = 0;
    while (1)
    {
        int g = i, l = 2 * i + 1, r = 2 * i + 2;
        if (l < h->n && h->tab[l]->f < h->tab[g]->f)
            g = l;
        if (r < h->n && h->tab[r]->f < h->tab[g]->f)
            g = r;
        if (g == i)
            break;
        heap_swap(&h->tab[i], &h->tab[g]);
        i = g;
    }
    return res;
}

/* HEURISTICS */
int h_misplaced(conf m)
{
    int c = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (m[i][j] && m[i][j] != BUT[i][j])
                c++;
    return c;
}
int h_manhattan(conf m)
{
    int d = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            int v = m[i][j];
            if (!v)
                continue;
            int ti = (v - 1) / N, tj = (v - 1) % N;
            d += abs(i - ti) + abs(j - tj);
        }
    return d;
}

/* UTILS */
int is_goal(conf m) { return memcmp(m, BUT, sizeof(conf)) == 0; }
void pos_vide(conf m, int *x, int *y)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (m[i][j] == 0)
            {
                *x = i;
                *y = j;
                return;
            }
}
int exists_in_path(Noeud *n, conf m)
{
    while (n)
    {
        if (memcmp(n->m, m, sizeof(conf)) == 0)
            return 1;
        n = n->pere;
    }
    return 0;
}

/* SOLVER */
typedef struct
{
    int nodes, max_frontier, cost;
    double time_ms;
} Stats;
Noeud **reconstruct_path(Noeud *goal, int *length)
{
    int len = 0;
    for (Noeud *p = goal; p; p = p->pere)
        len++;
    Noeud **path = malloc(sizeof(Noeud *) * len);
    int idx = len - 1;
    for (Noeud *p = goal; p; p = p->pere)
        path[idx--] = p;
    *length = len;
    return path;
}
Noeud *solve(conf start, int heuristic, double p, Stats *S, Noeud ***path, int *path_len)
{
    clock_t t0 = clock();
    Heap H;
    heap_init(&H);
    Noeud *r = malloc(sizeof(Noeud));
    memcpy(r->m, start, sizeof(conf));
    r->g = 0;
    r->h = (heuristic == 0) ? h_misplaced(start) : h_manhattan(start);
    r->f = r->g + p * r->h;
    r->pere = NULL;
    heap_push(&H, r);
    S->nodes = 0;
    S->max_frontier = 1;
    while (H.n)
    {
        if (H.n > S->max_frontier)
            S->max_frontier = H.n;
        Noeud *e = heap_pop(&H);
        S->nodes++;
        if (is_goal(e->m))
        {
            S->cost = e->g;
            S->time_ms = 1000.0 * (clock() - t0) / CLOCKS_PER_SEC;
            *path = reconstruct_path(e, path_len);
            return e;
        }
        int x, y;
        pos_vide(e->m, &x, &y);
        int dx[4] = {-1, 1, 0, 0}, dy[4] = {0, 0, -1, 1};
        for (int k = 0; k < 4; k++)
        {
            int nx = x + dx[k], ny = y + dy[k];
            if (nx < 0 || ny < 0 || nx >= N || ny >= N)
                continue;
            Noeud *v = malloc(sizeof(Noeud));
            memcpy(v->m, e->m, sizeof(conf));
            v->m[x][y] = v->m[nx][ny];
            v->m[nx][ny] = 0;
            if (exists_in_path(e, v->m))
            {
                free(v);
                continue;
            }
            v->g = e->g + 1;
            v->h = (heuristic == 0) ? h_misplaced(v->m) : h_manhattan(v->m);
            v->f = v->g + p * v->h;
            v->pere = e;
            heap_push(&H, v);
        }
    }
    return NULL;
}

/* GUI */
GtkWidget *grid, *info;
Noeud **anim_path;
int anim_len, anim_idx = 0;

void draw_conf(conf m)
{
    GList *c = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList *l = c; l; l = l->next)
        gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(c);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            char buf[8] = "";
            if (m[i][j])
                sprintf(buf, "%d", m[i][j]);
            GtkWidget *b = gtk_button_new_with_label(buf);
            gtk_widget_set_size_request(b, 60, 60);
            gtk_grid_attach(GTK_GRID(grid), b, j, i, 1, 1);
        }
    gtk_widget_show_all(grid);
}

gboolean animate_solution(gpointer d)
{
    if (anim_idx >= anim_len)
        return FALSE;
    draw_conf(anim_path[anim_idx++]->m);
    return TRUE;
}

void on_solve(GtkButton *b, gpointer d)
{
    int heuristic = GPOINTER_TO_INT(d);
    conf init = {{9, 0, 6, 10}, {3, 2, 12, 4}, {1, 5, 8, 7}, {13, 14, 11, 15}};
    Stats S;
    Noeud **path;
    int path_len;
    solve(init, heuristic, 1.5, &S, &path, &path_len);
    anim_path = path;
    anim_len = path_len;
    anim_idx = 0;
    g_timeout_add(500, animate_solution, NULL);
    char txt[256];
    sprintf(txt, "Cost=%d | Nodes=%d | Max Frontier=%d | Time=%.1f ms", S.cost, S.nodes, S.max_frontier, S.time_ms);
    gtk_label_set_text(GTK_LABEL(info), txt);
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Taquin 15 – Animated A*/WA*");
    gtk_container_set_border_width(GTK_CONTAINER(win), 10);
    g_signal_connect(win, "destroy", gtk_main_quit, NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    GtkWidget *b1 = gtk_button_new_with_label("A* (Misplaced)");
    GtkWidget *b2 = gtk_button_new_with_label("WA* (Manhattan)");
    g_signal_connect(b1, "clicked", G_CALLBACK(on_solve), GINT_TO_POINTER(0));
    g_signal_connect(b2, "clicked", G_CALLBACK(on_solve), GINT_TO_POINTER(1));
    gtk_box_pack_start(GTK_BOX(hbox), b1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), b2, TRUE, TRUE, 0);
    info = gtk_label_new("Ready");
    gtk_box_pack_start(GTK_BOX(vbox), info, FALSE, FALSE, 0);
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
