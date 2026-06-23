import tkinter as tk
from tkinter import scrolledtext
import threading
from knotlink import OpenSocketQuerier

class QuerierGUI:
    def __init__(self, root):
        self.root = root
        root.title("QuerierGUI")
        root.geometry("450x400")

        tk.Label(root, text="APP ID:").pack(pady=(10,0))
        self.appid_entry = tk.Entry(root)
        self.appid_entry.insert(0, "app.knotlinksdk.test")
        self.appid_entry.pack(pady=(0,5), fill=tk.X, padx=20)

        tk.Label(root, text="Socket ID:").pack()
        self.socketid_entry = tk.Entry(root)
        self.socketid_entry.insert(0, "test_socket")
        self.socketid_entry.pack(pady=(0,5), fill=tk.X, padx=20)

        tk.Label(root, text="Data to send:").pack()
        self.data_text = scrolledtext.ScrolledText(root, height=4)
        self.data_text.insert(tk.END, "Hello, Responser!")
        self.data_text.pack(padx=20, pady=5, fill=tk.X)

        self.send_btn = tk.Button(root, text="Send", command=self.send)
        self.send_btn.pack(pady=5)

        tk.Label(root, text="Result:").pack()
        self.result_text = scrolledtext.ScrolledText(root, height=6)
        self.result_text.pack(padx=20, pady=5, fill=tk.BOTH, expand=True)

        self.querier = OpenSocketQuerier(self.appid_entry.get(), self.socketid_entry.get())

    def send(self):
        self.send_btn.config(state=tk.DISABLED)
        self.result_text.insert(tk.END, "Sending...\n")
        threading.Thread(target=self._do_send, daemon=True).start()

    def _do_send(self):
        appid = self.appid_entry.get()
        socketid = self.socketid_entry.get()
        data = self.data_text.get("1.0", tk.END).strip()
        self.querier.set_config(appid, socketid)
        try:
            result = self.querier.query(data)
            if result is None:
                result = "(Empty response)"
            self.root.after(0, lambda: self._update_result(result))
        except Exception as e:
            err_msg = str(e)
            self.root.after(0, lambda: self._update_result(f"Error: {err_msg}"))
        finally:
            self.root.after(0, lambda: self.send_btn.config(state=tk.NORMAL))

    def _update_result(self, text):
        self.result_text.insert(tk.END, text + "\n")
        self.result_text.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = QuerierGUI(root)
    root.mainloop()