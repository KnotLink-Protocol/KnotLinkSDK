import tkinter as tk
from tkinter import scrolledtext
from knotlink import SignalSubscriber

class SubscriberGUI:
    def __init__(self, root):
        self.root = root
        root.title("Signal Subscriber Test GUI")
        root.geometry("450x400")

        tk.Label(root, text="APPID:").pack(pady=(10,0))
        self.appid_entry = tk.Entry(root)
        self.appid_entry.insert(0, "app.knotlinksdk.test")
        self.appid_entry.pack(pady=(0,10), fill=tk.X, padx=20)

        tk.Label(root, text="SignalID:").pack()
        self.signalid_entry = tk.Entry(root)
        self.signalid_entry.insert(0, "test_signal")
        self.signalid_entry.pack(pady=(0,10), fill=tk.X, padx=20)

        self.subscribe_btn = tk.Button(root, text="Subscribe", command=self.subscribe)
        self.subscribe_btn.pack(pady=5)

        tk.Label(root, text="Received Data:").pack()
        self.text_display = scrolledtext.ScrolledText(root, height=10)
        self.text_display.pack(padx=20, pady=(0,20), fill=tk.BOTH, expand=True)

        self.subscriber = None

    def subscribe(self):
        appid = self.appid_entry.get().strip()
        signalid = self.signalid_entry.get().strip()
        if not appid or not signalid:
            self.text_display.insert(tk.END, "APPID and SignalID must be defined.\n")
            return

        self.subscriber = SignalSubscriber(appid, signalid)
        self.subscriber.set_RecvFunc(self.on_received)

    def on_received(self, data):
        self.root.after(0, lambda: self.text_display.insert(tk.END, f"Received: {data}\n"))

if __name__ == "__main__":
    root = tk.Tk()
    app = SubscriberGUI(root)
    root.mainloop()