from collections import deque

def plus_court_cycle(graph):
    """
    Trouve le plus court cycle dans un graphe non orienté à l'aide de BFS.
    """
    shortest_cycle_length = float('inf')
    shortest_cycle_path = []

    for start in graph:
        # BFS depuis le sommet 'start'
        queue = deque([(start, -1, 0)])  # (sommet actuel, parent, distance)
        distance = {start: 0}
        parent = {start: -1}

        while queue:
            current, par, dist = queue.popleft()

            for neighbor in graph[current]:
                if neighbor not in distance:
                    # Nœud non visité
                    distance[neighbor] = dist + 1
                    parent[neighbor] = current
                    queue.append((neighbor, current, dist + 1))
                elif parent[current] != neighbor:
                    # Cycle détecté (le voisin est déjà visité et ce n'est pas notre parent)
                    cycle_length = distance[current] + distance[neighbor] + 1
                    
                    if cycle_length < shortest_cycle_length:
                        shortest_cycle_length = cycle_length
                        
                        # Reconstruire le cycle
                        path1, path2 = [], []
                        
                        # Chemin de current vers start
                        node = current
                        while node != -1:
                            path1.append(node)
                            node = parent[node]
                        
                        # Chemin de neighbor vers start
                        node = neighbor
                        while node != -1:
                            path2.append(node)
                            node = parent[node]
                        
                        # Trouver l'ancêtre commun
                        path1_set = set(path1)
                        common_ancestor = None
                        for node in path2:
                            if node in path1_set:
                                common_ancestor = node
                                break
                        
                        # Construire le cycle
                        cycle = []
                        node = current
                        while node != common_ancestor:
                            cycle.append(node)
                            node = parent[node]
                        cycle.append(common_ancestor)
                        
                        node = neighbor
                        path_to_ancestor = []
                        while node != common_ancestor:
                            path_to_ancestor.append(node)
                            node = parent[node]
                        
                        cycle.extend(reversed(path_to_ancestor))
                        shortest_cycle_path = cycle

    if shortest_cycle_length == float('inf'):
        return None, []
    return shortest_cycle_length, shortest_cycle_path


# Test
if __name__ == "__main__":
    graph = {
        'A': ['B', 'D'],
        'B': ['A', 'C'],
        'C': ['B', 'D'],
        'D': ['A', 'C']
    }

    length, cycle = plus_court_cycle(graph)
    if cycle:
        print("Plus court cycle trouvé :", cycle)
        print("Longueur du cycle :", length)
    else:
        print("Aucun cycle trouvé.")