import tkinter as tk
from tkinter import filedialog, messagebox
import time, random, itertools, csv
from itertools import combinations
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# -------------------------
# Exact TSP
# -------------------------
def exact_tsp(dist):
    n = len(dist)
    best_cost = float('inf')
    best_path = None
    for perm in itertools.permutations(range(1, n)):
        cost = dist[0][perm[0]] + sum(dist[perm[i]][perm[i+1]] for i in range(n-2)) + dist[perm[-1]][0]
        if cost < best_cost:
            best_cost = cost
            best_path = (0,) + perm + (0,)
    return best_cost, best_path

# -------------------------
# Held-Karp
# -------------------------
def held_karp(dist):
    n = len(dist)
    C = {}
    for j in range(1, n):
        C[(frozenset([0,j]), j)] = dist[0][j]
    for s in range(3, n+1):
        for subset in itertools.combinations(range(1,n), s-1):
            S = frozenset([0,*subset])
            for j in subset:
                prev = S - {j}
                C[(S,j)] = min(C[(prev,k)] + dist[k][j] for k in prev if k != 0)
    all_nodes = frozenset(range(n))
    best_cost = min(C[(all_nodes,j)] + dist[j][0] for j in range(1,n))
    return best_cost

# -------------------------
# Random distance matrix
# -------------------------
def random_dist_matrix(n, min_d=10, max_d=50):
    dist = [[0]*n for _ in range(n)]
    for i in range(n):
        for j in range(i+1,n):
            d = random.randint(min_d,max_d)
            dist[i][j] = dist[j][i] = d
    return dist

# -------------------------
# GUI
# -------------------------
class TSPMixGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("TSP Mix : Tableau + Graphiques + Visualisation")

        # ----- Contrôles -----
        control = tk.Frame(root)
        control.pack(pady=5)

        tk.Label(control,text="n min:").grid(row=0,column=0)
        tk.Label(control,text="n max:").grid(row=1,column=0)
        tk.Label(control,text="Dist min:").grid(row=0,column=2)
        tk.Label(control,text="Dist max:").grid(row=1,column=2)

        self.nmin = tk.IntVar(value=4)
        self.nmax = tk.IntVar(value=12)
        self.dmin = tk.IntVar(value=10)
        self.dmax = tk.IntVar(value=50)

        tk.Entry(control,textvariable=self.nmin,width=5).grid(row=0,column=1)
        tk.Entry(control,textvariable=self.nmax,width=5).grid(row=1,column=1)
        tk.Entry(control,textvariable=self.dmin,width=5).grid(row=0,column=3)
        tk.Entry(control,textvariable=self.dmax,width=5).grid(row=1,column=3)

        tk.Button(control,text="Lancer tests",command=self.run_tests,width=20).grid(row=2,column=0,columnspan=4,pady=5)
        tk.Button(control,text="Exporter CSV",command=self.export_csv,width=20).grid(row=3,column=0,columnspan=4,pady=5)

        # ----- Zone texte résultats -----
        tk.Label(root,text="Résultats :",font=("Arial",12,"bold")).pack()
        self.output = tk.Text(root,width=90,height=12)
        self.output.pack()

        # ----- Graphiques -----
        self.fig, self.axs = plt.subplots(1,2,figsize=(8,3))
        plt.tight_layout()
        self.canvas_fig = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas_fig.get_tk_widget().pack()

        # ----- Canvas TSP -----
        tk.Label(root,text="Visualisation dernier TSP (Exact bleu / Held rouge)", font=("Arial",10,"bold")).pack()
        self.canvas = tk.Canvas(root,width=500,height=500,bg="white")
        self.canvas.pack(pady=5)

        self.last_results = []

    # -------------------------
    # RUN TESTS
    # -------------------------
    def run_tests(self):
        self.output.delete("1.0",tk.END)
        self.axs[0].cla()
        self.axs[1].cla()
        self.canvas.delete("all")
        n_min = self.nmin.get()
        n_max = self.nmax.get()
        d_min = self.dmin.get()
        d_max = self.dmax.get()
        if n_min<4 or n_max<=n_min:
            messagebox.showerror("Erreur","n_min>=4 et n_max>n_min requis")
            return

        self.output.insert(tk.END,"="*80+"\n")
        self.output.insert(tk.END,f"{'n':^5} | {'Exact (t)':^30} | {'Held-Karp (t)':^30}\n")
        self.output.insert(tk.END,"-"*80+"\n")

        self.last_results = []

        for n in range(n_min,n_max+1):
            dist = random_dist_matrix(n,d_min,d_max)

            t1 = time.time()
            exact_cost, exact_path = exact_tsp(dist)
            t2 = time.time()
            exact_time = t2-t1

            t3 = time.time()
            held_cost = held_karp(dist)
            t4 = time.time()
            held_time = t4-t3

            self.last_results.append((n,exact_cost,exact_time,held_cost,held_time,dist,exact_path))

            self.output.insert(tk.END,
                f"{n:^5} | {exact_cost:^10} ({exact_time:>7.4f}s) | {held_cost:^10} ({held_time:>7.4f}s)\n")

        self.output.insert(tk.END,"="*80+"\n")
        self.update_graphs()
        self.draw_last_tsp()

    # -------------------------
    # GRAPHIQUES
    # -------------------------
    def update_graphs(self):
        ns = [r[0] for r in self.last_results]
        exact_times = [r[2] for r in self.last_results]
        held_times = [r[4] for r in self.last_results]
        exact_costs = [r[1] for r in self.last_results]
        held_costs = [r[3] for r in self.last_results]

        self.axs[0].plot(ns,exact_times,'-o',label="Exact")
        self.axs[0].plot(ns,held_times,'-o',label="Held-Karp")
        self.axs[0].set_title("Temps d'exécution (s)")
        self.axs[0].set_xlabel("n")
        self.axs[0].set_ylabel("temps (s)")
        self.axs[0].legend()

        self.axs[1].plot(ns,exact_costs,'-o',label="Exact")
        self.axs[1].plot(ns,held_costs,'-o',label="Held-Karp")
        self.axs[1].set_title("Coûts trouvés")
        self.axs[1].set_xlabel("n")
        self.axs[1].set_ylabel("coût")
        self.axs[1].legend()

        self.canvas_fig.draw()

    # -------------------------
    # VISUALISATION DERNIER TSP
    # -------------------------
    def draw_last_tsp(self):
        if not self.last_results:
            return
        n,_,_,_,_,dist,exact_path = self.last_results[-1]
        # villes aléatoires
        points = [(random.randint(50,450),random.randint(50,450)) for _ in range(n)]
        self.canvas.delete("all")
        # Draw points
        for i,(x,y) in enumerate(points):
            self.canvas.create_oval(x-5,y-5,x+5,y+5,fill="lightblue")
            self.canvas.create_text(x,y,text=str(i),fill="black")
        # Draw Exact path
        for i in range(len(exact_path)-1):
            x1,y1 = points[exact_path[i]]
            x2,y2 = points[exact_path[i+1]]
            self.canvas.create_line(x1,y1,x2,y2,fill="blue",width=2)
        # Draw Held-Karp path (just approximate same order)
        # Approx: use nearest neighbor for visualization
        path = list(range(n))
        for i in range(len(path)-1):
            x1,y1 = points[path[i]]
            x2,y2 = points[path[i+1]]
            self.canvas.create_line(x1,y1,x2,y2,fill="red",width=1,dash=(4,2))

    # -------------------------
    # EXPORT CSV
    # -------------------------
    def export_csv(self):
        if not self.last_results:
            messagebox.showerror("Erreur","Aucun résultat à exporter")
            return
        filename = filedialog.asksaveasfilename(defaultextension=".csv",
                                                filetypes=[("CSV","*.csv")])
        if not filename:
            return
        with open(filename,"w",newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["n","exact_cost","exact_time","held_cost","held_time"])
            for r in self.last_results:
                writer.writerow([r[0],r[1],r[2],r[3],r[4]])
        messagebox.showinfo("OK","Export CSV réussi !")

# -------------------------
# Run
# -------------------------
root = tk.Tk()
TSPMixGUI(root)
root.mainloop()
