# exact_tsp.py
import itertools

def exact_tsp(dist):
    """
    Algorithme exact pour le problème du voyageur de commerce.
    dist : matrice des distances (liste de listes)
    Retourne : (coût minimal, chemin correspondant)
    """
    n = len(dist)
    best_cost = float('inf')
    best_path = None

    for perm in itertools.permutations(range(1, n)):  # permutations des villes sauf 0
        cost = dist[0][perm[0]] \
             + sum(dist[perm[i]][perm[i+1]] for i in range(n-2)) \
             + dist[perm[-1]][0]
        if cost < best_cost:
            best_cost = cost
            best_path = (0,) + perm + (0,)

    return best_cost, best_path


# Exemple de test rapide
if __name__ == "__main__":
    d = [
        [0, 10, 15, 20],
        [10, 0, 35, 25],
        [15, 35, 0, 30],
        [20, 25, 30, 0]
    ]
    cost, path = exact_tsp(d)
    print("Meilleur coût :", cost)
    print("Chemin :", path)
