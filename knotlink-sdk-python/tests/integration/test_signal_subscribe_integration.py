#!/usr/bin/env python3
# test_signal_subscribe_integration.py
import time
import threading
from knotlink import SignalSubscriber, SignalSender

_shutdown = threading.Event()

def run_subscriber():
    def on_received(data):
        print(f"[Subscriber] Received: {data}")
    sub = SignalSubscriber("app.knotlinksdk.test", "test_signal")
    sub.set_RecvFunc(on_received)
    print("[Subscriber] Listening...")
    try:
        while not _shutdown.wait(timeout=1):
            pass
    finally:
        sub.disconnect()
        print("[Subscriber] Disconnected.")

def run_sender():
    _shutdown.wait(timeout=1)  # 等待 subscriber 准备就绪
    sender = SignalSender()
    sender.set_config("app.knotlinksdk.test", "test_signal")
    try:
        sender.emitt("Hello from Sender!")
        print("[Sender] Signal sent.")
    finally:
        sender.disconnect()
        print("[Sender] Disconnected.")
        _shutdown.set()  # 通知 subscriber 退出

if __name__ == "__main__":
    t = threading.Thread(target=run_subscriber, daemon=True)
    t.start()
    run_sender()
    t.join(timeout=5)