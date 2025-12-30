import socket
import threading
import tkinter as tk
from tkinter import messagebox, simpledialog

# --- Configuration ---
SERVER_IP = '127.0.0.1'
SERVER_PORT = 12345
N = 7  # Default grid size, will be updated from server
M = 4  # Default alignment, will be updated from server
TMO = 10
PLAYER = None

class TicTacToeClient:
    def __init__(self, master):
        self.master = master
        self.master.title("TicTacToe TCP Client")
        self.grid = []
        self.sock = None
        self.player = None
        self.adv = None
        self.my_turn = False
        self.cells = []
        self.info = None
        self.create_widgets()
        self.connect_to_server()

    def create_widgets(self):
        self.frame = tk.Frame(self.master)
        self.frame.pack()
        for i in range(N):
            row = []
            for j in range(N):
                btn = tk.Button(self.frame, text=' ', width=3, height=1,
                                font=('Arial', 20), command=lambda x=i, y=j: self.play_move(x, y))
                btn.grid(row=i, column=j)
                row.append(btn)
            self.cells.append(row)
        self.info = tk.Label(self.master, text="Connecting...", font=('Arial', 14))
        self.info.pack(pady=10)

    def connect_to_server(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))
            self.master.destroy()
            return
        self.ask_player_name()
        threading.Thread(target=self.listen_server, daemon=True).start()

    def ask_player_name(self):
        name = simpledialog.askstring("Player Name", "Enter your name:")
        if not name:
            name = "Player"
        self.sock.sendall((name + '\0').encode())

    def listen_server(self):
        global N, M, TMO
        # Receive game parameters
        params = self.sock.recv(100).decode()
        parts = params.strip().split()
        N, M, TMO, self.player = int(parts[0]), int(parts[1]), int(parts[2]), parts[3]
        self.adv = 'O' if self.player == 'X' else 'X'
        self.info.config(text=f"You are player {self.player}. Waiting for game...")
        self.my_turn = (self.player == 'X')
        if self.my_turn:
            self.info.config(text="Your turn!")
        while True:
            data = self.sock.recv(100).decode()
            if not data:
                break
            if data.startswith(self.adv):
                # Opponent move (format: 'X at i j' or 'O at i j')
                parts = data.strip().split()
                if len(parts) >= 4 and parts[1] == 'at':
                    i, j = int(parts[2]), int(parts[3])
                else:
                    # fallback: try last two parts
                    i, j = int(parts[-2]), int(parts[-1])
                self.update_cell(i, j, self.adv)
                self.my_turn = True
                self.info.config(text="Your turn!")
            elif "gameover" in data:
                # Parse cost value safely (remove whitespace and null bytes)
                cost_str = data.strip().split('=')[1]
                cost_str = cost_str.strip().replace('\x00', '').replace('\n', '')
                try:
                    cost = int(cost_str)
                except ValueError:
                    cost = 0
                if cost == 1:
                    msg = "Player X wins!"
                elif cost == -1:
                    msg = "Player O wins!"
                else:
                    msg = "Draw!"
                messagebox.showinfo("Game Over", msg)
                self.master.quit()
                break

    def play_move(self, i, j):
        if not self.my_turn:
            return
        if self.cells[i][j]['text'] != ' ':
            return
        self.update_cell(i, j, self.player)
        self.sock.sendall(f"{i} {j}\n".encode())
        self.my_turn = False
        self.info.config(text="Waiting for opponent...")

    def update_cell(self, i, j, symbol):
        self.cells[i][j]['text'] = symbol
        self.cells[i][j]['state'] = 'disabled'

if __name__ == "__main__":
    root = tk.Tk()
    app = TicTacToeClient(root)
    root.mainloop()
