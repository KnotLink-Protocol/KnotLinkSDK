#!/usr/bin/env python3
# test_request_response_integration.py
import time
import threading
from knotlink import OpenSocketResponser, OpenSocketQuerier

_shutdown = threading.Event()

def run_responser():
    def on_request(data):
        print(f"[Responser] Received: {data}")
        return f"Echo: {data}"
    r = OpenSocketResponser("app.knotlinksdk.test", "test_socket")
    r.set_RecvFunc(on_request)
    print("[Responser] Running...")
    try:
        while not _shutdown.wait(timeout=1):
            pass
    finally:
        r.disconnect()
        print("[Responser] Disconnected.")

def run_querier():
    _shutdown.wait(timeout=1)  # 等待 responser 启动
    q = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
    try:
        result = q.query("Hello, Responser!")
        print(f"[Querier] Response: {result}")
    except Exception as e:
        print(f"[Querier] Error: {e}")
    finally:
        q.disconnect()
        print("[Querier] Disconnected.")
        _shutdown.set()  # 通知 responser 退出

if __name__ == "__main__":
    t = threading.Thread(target=run_responser, daemon=True)
    t.start()
    run_querier()
    t.join(timeout=5)