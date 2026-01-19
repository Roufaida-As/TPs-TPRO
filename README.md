# TPs-TPRO - Travaux Pratiques TPRO (2CS - ESI 2025)

A collection of practical assignments in algorithmic problem-solving and networking. This repository contains implementations of various algorithms and applications developed during the TPRO course.

##  Project Structure

### TP1 - Shortest Cycle Detection in Graphs
**Directory:** [TP1/](TP1/)

Find the shortest cycle in an undirected graph using Breadth-First Search (BFS).

**Files:**
- `tp1.py` - Core algorithm implementation using BFS
- `tp1-gui.py` - Interactive GUI for graph visualization and shortest cycle detection
- `readme.md` - Algorithm complexity analysis

**Key Features:**
- BFS-based cycle detection algorithm
- Reconstruction of the shortest cycle path
- Interactive graphical interface for building and analyzing graphs
- Complexity: O(|S| × (|S| + |A|)) where |S| is vertices and |A| is edges

**Technologies:** Python, Tkinter

---

### TP2 - Traveling Salesman Problem (TSP) Algorithms
**Directory:** [TP2/](TP2/)

Comparison of exact and dynamic programming solutions for the TSP problem.

**Files:**
- `exact_tsp.py` - Exact brute-force algorithm using permutations
- `held_karp_tsp.py` - Bellman-Held-Karp dynamic programming algorithm
- `test_tsp.py` - Comprehensive performance benchmarking tool
- `tp2-gui.py` - GUI application for visualization
- `tp2-gui.html` - HTML interface

**Key Algorithms:**
- **Exact TSP:** O(n! × n) - Tests all permutations for small instances
- **Held-Karp Algorithm:** O(n² × 2ⁿ) - Dynamic programming approach using subset enumeration

**Features:**
- Performance comparison between exact and approximate solutions
- Benchmarking with varying problem sizes (4-11 cities)
- Timing analysis and results visualization

**Technologies:** Python, NumPy

---

### TP3 - Network Tic-Tac-Toe Game (NxM Alignment)
**Directory:** [TP3/](TP3/)

A distributed implementation of Tic-Tac-Toe with generalized NxN grid and M-in-a-row winning condition, using TCP sockets and game tree search algorithms.

**Files:**
- `server.c` - TCP server managing game state and player connections
- `client_example.c` - Example client with basic Minimax + Alpha-Beta pruning
- `client_intelligent.c` - Enhanced client with improved evaluation function and timeout management
- `tictactoe_gui.py` - Python GUI for the game

**Architecture:**
- **Server:** Manages game state, validates moves, tracks players
- **Clients:** Implement AI using game tree search:
  - Minimax algorithm with Alpha-Beta pruning for move selection
  - Estimation functions to evaluate board positions
  - Timeout-based search depth limitation

**Key Features:**
- Configurable board size (N×N) and winning condition (M in a row)
- TCP-based client-server communication
- Game tree exploration with pruning
- Time-limited move computation

**Technologies:** C (sockets, threading), Python

---

### TP4 - Enhanced 15-Puzzle Solver with GUI Animation
**Directory:** [TP4/](TP4/)

An advanced 15-puzzle solver using A* and Weighted A* (WA*) algorithms with step-by-step animated visualization in GTK.

**File:**
- `tp4.c` - Complete implementation with A* search and GUI

**Algorithm Details:**
- **Search Method:** A* and Weighted A* (f = g + p×h)
- **Heuristics Implemented:**
  - Misplaced tiles heuristic
  - Manhattan distance heuristic
- **Optimization:** Heap-based priority queue (O(log n) operations)
- **GUI:** GTK-based animated solution display

**Features:**
- Efficient state-space search with heap-based priority queue
- Multiple heuristic functions for path planning
- Visual animation of solution steps in GTK interface
- Optimized for 4×4 puzzle grids
- Performance optimized with -O2 compilation flag

**Compilation (WSL with GTK):**
```bash
sudo apt install libgtk-3-dev
gcc tp4.c -o taquin `pkg-config --cflags --libs gtk+-3.0` -O2
./taquin
```

**Technologies:** C, GTK3, Data Structures (Heaps)


---

##  Key Concepts Covered

- **Graph Theory:** BFS, cycle detection, connectivity analysis
- **Dynamic Programming:** Subset enumeration (Held-Karp), optimal substructure
- **Game Theory:** Minimax algorithm, Alpha-Beta pruning, game trees
- **Search Algorithms:** A*, Weighted A*, heuristic-based optimization
- **Network Programming:** TCP sockets, client-server architecture
- **Data Structures:** Heaps, priority queues, adjacency lists
- **Performance Analysis:** Complexity analysis, benchmarking, timing comparisons


---

##  License

These assignments are educational materials for the TPRO course at ESI.

---

##  Author

Roufaida-As

---

##  Contributing

This is an educational repository for course assignments. Feel free to reference these implementations for learning purposes.

