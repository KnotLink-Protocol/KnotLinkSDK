import tkinter as tk
from tkinter import scrolledtext
from knotlink import OpenSocketResponser

class ResponserGUI:
    def __init__(self, root):
        self.root = root
        root.title("OpenSocketResponser Test GUI")
        root.geometry("450x400")

        tk.Label(root, text="APP ID:").pack(pady=(10,0))
        self.appid_entry = tk.Entry(root)
        self.appid_entry.insert(0, "app.knotlinksdk.test")
        self.appid_entry.pack(pady=(0,5), fill=tk.X, padx=20)

        tk.Label(root, text="Socket ID:").pack()
        self.socketid_entry = tk.Entry(root)
        self.socketid_entry.insert(0, "test_socket")
        self.socketid_entry.pack(pady=(0,10), fill=tk.X, padx=20)

        self.start_btn = tk.Button(root, text="Start Responser", command=self.start_responser)
        self.start_btn.pack(pady=5)

        self.log_text = scrolledtext.ScrolledText(root, height=12)
        self.log_text.pack(padx=20, pady=10, fill=tk.BOTH, expand=True)

        self.responser = None
        self.running = False

    def log(self, msg):
        self.root.after(0, lambda: self.log_text.insert(tk.END, msg + "\n"))
        self.root.after(0, lambda: self.log_text.see(tk.END))

    def start_responser(self):
        if self.running:
            return
        appid = self.appid_entry.get().strip()
        socketid = self.socketid_entry.get().strip()
        if not appid or not socketid:
            self.log("APP ID and Socket ID must be set.")
            return

        self.responser = OpenSocketResponser(appid, socketid)
        self.responser.set_RecvFunc(self.on_request)
        self.running = True
        self.start_btn.config(text="Running...", state=tk.DISABLED)
        self.log(f"Responser started for {appid}-{socketid}")

    def on_request(self, data):
        self.log(f"Received request: {data}")
        return f"Echo: {data}"

if __name__ == "__main__":
    root = tk.Tk()
    app = ResponserGUI(root)
    root.mainloop()