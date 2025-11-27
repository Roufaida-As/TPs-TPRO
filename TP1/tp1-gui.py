import tkinter as tk
from tkinter import messagebox, filedialog
import pickle
from collections import deque


# -----------------------------------------------------------
# shortest cycle function
# -----------------------------------------------------------

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


# -----------------------------------------------------------
# Interactive Graph GUI
# -----------------------------------------------------------
class GraphGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Interactive Graph Editor - Shortest Cycle")

        self.canvas = tk.Canvas(root, width=800, height=500, bg="white")
        self.canvas.pack(fill="both", expand=True)

        # Data structures
        self.nodes = {}          # node_id → {"x": , "y": , "circle": canvas obj, "text": canvas obj}
        self.edges = []          # list of (node1, node2, line_id)
        self.graph = {}          # adjacency list
        self.node_counter = 1

        self.selected_node = None
        self.dragging_node = None

        # Buttons
        toolbar = tk.Frame(root)
        toolbar.pack(pady=5)

        tk.Button(toolbar, text="Compute Shortest Cycle", command=self.compute_cycle).pack(side="left", padx=10)
        tk.Button(toolbar, text="Save Graph", command=self.save_graph).pack(side="left", padx=10)
        tk.Button(toolbar, text="Load Graph", command=self.load_graph).pack(side="left", padx=10)

        # Canvas events
        self.canvas.bind("<Button-1>", self.left_click)
        self.canvas.bind("<B1-Motion>", self.drag)
        self.canvas.bind("<ButtonRelease-1>", self.stop_drag)

    # -----------------------------------------------------------
    # Node and edge creation
    # -----------------------------------------------------------

    def left_click(self, event):
        clicked = self.find_node(event.x, event.y)

        if clicked:  # Clicked on an existing node
            if self.selected_node is None:
                self.selected_node = clicked
                self.highlight_node(clicked)
            else:
                if clicked != self.selected_node:
                    self.create_edge(self.selected_node, clicked)
                self.unhighlight_node(self.selected_node)
                self.selected_node = None
        else:
            self.create_node(event.x, event.y)

    def create_node(self, x, y):
        node_id = str(self.node_counter)
        self.node_counter += 1

        r = 20
        circle = self.canvas.create_oval(x-r, y-r, x+r, y+r, fill="lightblue")
        text = self.canvas.create_text(x, y, text=node_id)

        self.nodes[node_id] = {"x": x, "y": y, "circle": circle, "text": text}
        self.graph[node_id] = []

    def create_edge(self, n1, n2):
        # Avoid duplicates
        if n2 in self.graph[n1]:
            return

        x1, y1 = self.nodes[n1]["x"], self.nodes[n1]["y"]
        x2, y2 = self.nodes[n2]["x"], self.nodes[n2]["y"]

        line = self.canvas.create_line(x1, y1, x2, y2, width=2)
        self.edges.append((n1, n2, line))

        self.graph[n1].append(n2)
        self.graph[n2].append(n1)

    # -----------------------------------------------------------
    # Node selection highlight
    # -----------------------------------------------------------
    def highlight_node(self, node_id):
        self.canvas.itemconfig(self.nodes[node_id]["circle"], outline="red", width=3)

    def unhighlight_node(self, node_id):
        self.canvas.itemconfig(self.nodes[node_id]["circle"], outline="black", width=1)

    # -----------------------------------------------------------
    # Dragging nodes
    # -----------------------------------------------------------
    def drag(self, event):
        node_id = self.find_node(event.x, event.y)
        if not node_id:
            return

        self.dragging_node = node_id

        # move graphical items
        r = 20
        self.canvas.coords(
            self.nodes[node_id]["circle"],
            event.x - r, event.y - r, event.x + r, event.y + r
        )
        self.canvas.coords(self.nodes[node_id]["text"], event.x, event.y)

        # update coordinates
        self.nodes[node_id]["x"] = event.x
        self.nodes[node_id]["y"] = event.y

        # move connected edges
        self.update_edges()

    def stop_drag(self, event):
        self.dragging_node = None

    def update_edges(self):
        for n1, n2, line in self.edges:
            x1, y1 = self.nodes[n1]["x"], self.nodes[n1]["y"]
            x2, y2 = self.nodes[n2]["x"], self.nodes[n2]["y"]
            self.canvas.coords(line, x1, y1, x2, y2)

    # -----------------------------------------------------------
    # Node detection
    # -----------------------------------------------------------
    def find_node(self, x, y):
        for node_id, info in self.nodes.items():
            cx, cy = info["x"], info["y"]
            if (x - cx)**2 + (y - cy)**2 <= 20**2:
                return node_id
        return None

    # -----------------------------------------------------------
    # Cycle computation
    # -----------------------------------------------------------
    def compute_cycle(self):
        length, cycle = plus_court_cycle(self.graph)

        if not cycle:
            messagebox.showinfo("Résultat", "Aucun cycle trouvé.")
            return

        messagebox.showinfo("Cycle trouvé",
                            f"Cycle : {' → '.join(cycle)}\nLongueur : {length}")

    # -----------------------------------------------------------
    # Save & Load
    # -----------------------------------------------------------
    def save_graph(self):
        filename = filedialog.asksaveasfilename(defaultextension=".pkl",
                                                filetypes=[("Pickle file", "*.pkl")])
        if not filename:
            return

        data = {
            "nodes": self.nodes,
            "edges": self.edges,
            "graph": self.graph,
            "counter": self.node_counter
        }

        with open(filename, "wb") as f:
            pickle.dump(data, f)

        messagebox.showinfo("OK", "Graph saved!")

    def load_graph(self):
        filename = filedialog.askopenfilename(filetypes=[("Pickle file", "*.pkl")])
        if not filename:
            return

        with open(filename, "rb") as f:
            data = pickle.load(f)

        # Clear the canvas
        self.canvas.delete("all")

        self.nodes = data["nodes"]
        self.edges = data["edges"]
        self.graph = data["graph"]
        self.node_counter = data["counter"]

        # Rebuild graphics
        for node_id, info in self.nodes.items():
            x, y = info["x"], info["y"]
            r = 20
            circle = self.canvas.create_oval(x-r, y-r, x+r, y+r, fill="lightblue")
            text = self.canvas.create_text(x, y, text=node_id)
            info["circle"], info["text"] = circle, text

        for n1, n2, _ in self.edges:
            x1, y1 = self.nodes[n1]["x"], self.nodes[n1]["y"]
            x2, y2 = self.nodes[n2]["x"], self.nodes[n2]["y"]
            line = self.canvas.create_line(x1, y1, x2, y2, width=2)
            # Update line reference
            for e in self.edges:
                if e[0] == n1 and e[1] == n2:
                    e = (n1, n2, line)

        messagebox.showinfo("OK", "Graph loaded!")


# -----------------------------------------------------------
# Run GUI
# -----------------------------------------------------------
root = tk.Tk()
GraphGUI(root)
root.mainloop()
