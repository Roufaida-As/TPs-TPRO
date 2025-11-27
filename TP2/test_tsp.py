# test_tsp.py
import time
import random
from exact_tsp import exact_tsp
from held_karp_tsp import held_karp

def random_dist_matrix(n, min_d=10, max_d=50):
    """Crée une matrice de distances symétrique aléatoire"""
    dist = [[0]*n for _ in range(n)]
    for i in range(n):
        for j in range(i+1, n):
            d = random.randint(min_d, max_d)
            dist[i][j] = dist[j][i] = d
    return dist


# En-tête du tableau
print("="*65)
print(f"{'n (villes)':^10} | {'Méthode exacte (coût)':^22} | {'Held-Karp (coût)':^22}")
print("-"*65)

# Corps du tableau
for n in range(4, 12):
    dist = random_dist_matrix(n)

    # Méthode exacte
    t1 = time.time()
    exact_cost, _ = exact_tsp(dist)
    t2 = time.time()
    exact_time = t2 - t1

    # Méthode Held–Karp
    t3 = time.time()
    held_cost = held_karp(dist)
    t4 = time.time()
    held_time = t4 - t3

    # Affichage dans le tableau
    print(f"{n:^10} | {exact_cost:^10} ({exact_time:>8.5f}s) | {held_cost:^10} ({held_time:>8.5f}s)")

print("="*65)
