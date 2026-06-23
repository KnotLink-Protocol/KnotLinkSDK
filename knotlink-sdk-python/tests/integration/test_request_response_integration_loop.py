#!/usr/bin/env python3
# test_request_response_integration_loop.py
# 访问-回放 模式集成测试 (Querier + Responser) - 循环发送

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
    time.sleep(1)
    q = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
    counter = 0
    try:
        while True:
            counter += 1
            data = f"Hello #{counter}"
            try:
                result = q.query(data)
                print(f"[Querier] Req: {data} → Res: {result}")
            except Exception as e:
                print(f"[Querier] Error: {e}")
            time.sleep(3)
    except KeyboardInterrupt:
        print("\n[Querier] Stopped by user")


if __name__ == "__main__":
    t = threading.Thread(target=run_responser, daemon=True)
    t.start()
    run_querier()