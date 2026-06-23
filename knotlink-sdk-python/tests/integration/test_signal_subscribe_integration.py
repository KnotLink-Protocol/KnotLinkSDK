#!/usr/bin/env python3
# test_signal_subscribe_integration.py
import time
import threading
from knotlink import SignalSubscriber, SignalSender

def run_subscriber():
    def on_received(data):
        print(f"[Subscriber] Received: {data}")
    sub = SignalSubscriber("app.knotlinksdk.test", "test_signal")
    sub.set_RecvFunc(on_received)
    print("[Subscriber] Listening...")
    while True:
        time.sleep(1)

def run_sender():
    time.sleep(1)  # 等待 subscriber 准备就绪
    sender = SignalSender()
    sender.set_config("app.knotlinksdk.test", "test_signal")
    sender.emitt("Hello from Sender!")
    print("[Sender] Signal sent.")

if __name__ == "__main__":
    t = threading.Thread(target=run_subscriber, daemon=True)
    t.start()
    run_sender()