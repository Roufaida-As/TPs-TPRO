# held_karp_tsp.py
from itertools import combinations

def held_karp(dist):
    """
    Algorithme de Bellman-Held-Karp (programmation dynamique)
    dist : matrice des distances (liste de listes)
    Retourne : (coût minimal)
    """
    n = len(dist)
    C = {}

    # Initialisation : ensembles {0, j}
    for j in range(1, n):
        C[(frozenset([0, j]), j)] = dist[0][j]

    # Étapes de la programmation dynamique
    for s in range(3, n + 1):
        for subset in combinations(range(1, n), s - 1):
            S = frozenset([0, *subset])
            for j in subset:
                prev = S - {j}
                C[(S, j)] = min(C[(prev, k)] + dist[k][j] for k in prev if k != 0)

    # Solution finale
    all_nodes = frozenset(range(n))
    best_cost = min(C[(all_nodes, j)] + dist[j][0] for j in range(1, n))
    return best_cost


# Exemple de test rapide
if __name__ == "__main__":
    d = [
        [0, 10, 15, 20],
        [10, 0, 35, 25],
        [15, 35, 0, 30],
        [20, 25, 30, 0]
    ]
    cost = held_karp(d)
    print("Coût minimal (Held-Karp) :", cost)
