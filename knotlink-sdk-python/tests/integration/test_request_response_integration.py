#!/usr/bin/env python3
# test_request_response_integration.py
import time
import threading
from knotlink import OpenSocketResponser, OpenSocketQuerier

def run_responser():
    def on_request(data):
        print(f"[Responser] Received: {data}")
        return f"Echo: {data}"
    r = OpenSocketResponser("app.knotlinksdk.test", "test_socket")
    r.set_RecvFunc(on_request)
    print("[Responser] Running...")
    while True:
        time.sleep(1)

def run_querier():
    time.sleep(1)  # 等待 responser 启动
    q = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
    try:
        result = q.query("Hello, Responser!")
        print(f"[Querier] Response: {result}")
    except Exception as e:
        print(f"[Querier] Error: {e}")

if __name__ == "__main__":
    t = threading.Thread(target=run_responser, daemon=True)
    t.start()
    run_querier()