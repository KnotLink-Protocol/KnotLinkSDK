import tkinter as tk
from tkinter import scrolledtext
from knotlink import SignalSender

class SenderGUI:
    def __init__(self, root):
        self.root = root
        root.title("SignalSender Test")
        root.geometry("450x350")

        tk.Label(root, text="APPID:").pack(pady=(10,0))
        self.appid_entry = tk.Entry(root)
        self.appid_entry.insert(0, "app.knotlinksdk.test")
        self.appid_entry.pack(pady=(0,5), fill=tk.X, padx=20)

        tk.Label(root, text="SignalID:").pack()
        self.signalid_entry = tk.Entry(root)
        self.signalid_entry.insert(0, "test_signal")
        self.signalid_entry.pack(pady=(0,5), fill=tk.X, padx=20)

        tk.Label(root, text="Data to Send:").pack()
        self.data_text = scrolledtext.ScrolledText(root, height=4)
        self.data_text.insert(tk.END, "Hello from Sender!")
        self.data_text.pack(padx=20, pady=5, fill=tk.X)

        self.send_btn = tk.Button(root, text="Send Data", command=self.send_data)
        self.send_btn.pack(pady=10)

        self.sender = SignalSender()

    def send_data(self):
        appid = self.appid_entry.get().strip()
        signalid = self.signalid_entry.get().strip()
        data = self.data_text.get("1.0", tk.END).strip()
        if not appid or not signalid:
            return
        self.sender.set_config(appid, signalid)
        self.sender.emitt(data)
        self.send_btn.config(text="Sent!", state=tk.DISABLED)
        self.root.after(1000, lambda: self.send_btn.config(text="Send Data", state=tk.NORMAL))

if __name__ == "__main__":
    root = tk.Tk()
    app = SenderGUI(root)
    root.mainloop()