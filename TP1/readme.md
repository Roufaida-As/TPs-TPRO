Complexité de l’algorithme

On fait un BFS pour chaque sommet.

Un BFS sur un graphe à |S| sommets et |A| arêtes → O(|S| + |A|).

On refait ça |S| fois (depuis chaque sommet).

Donc :  Complexite totale = O(∣S∣×(∣S∣+∣A∣))
	​

Cas particuliers :

Pour un graphe dense (|A| ≈ |S|²) → O(|S|³).

Pour un graphe peu dense (|A| ≈ |S|) → O(|S|²).