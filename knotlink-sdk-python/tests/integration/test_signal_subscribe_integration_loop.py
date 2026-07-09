#!/usr/bin/env python3
# test_signal_subscribe_integration_loop.py
# 发信-订阅 模式集成测试 (Sender + Subscriber) - 循环发送

import time
import threading
from knotlink import SignalSubscriber, SignalSender

_shutdown = threading.Event()
MAX_ITERATIONS = 5  # 最大循环次数，防止无限运行


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
    _shutdown.wait(timeout=1)
    sender = SignalSender()
    sender.set_config("app.knotlinksdk.test", "test_signal")
    counter = 0
    try:
        while counter < MAX_ITERATIONS and not _shutdown.is_set():
            counter += 1
            data = f"Signal #{counter}"
            sender.emitt(data)
            print(f"[Sender] Sent: {data}")
            time.sleep(3)
    except KeyboardInterrupt:
        print("\n[Sender] Stopped by user")
    finally:
        sender.disconnect()
        _shutdown.set()
        print("[Sender] Disconnected.")


if __name__ == "__main__":
    t = threading.Thread(target=run_subscriber, daemon=True)
    t.start()
    run_sender()
    t.join(timeout=10)