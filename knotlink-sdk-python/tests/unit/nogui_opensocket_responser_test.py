import time
from knotlink import OpenSocketResponser

def on_request(data):
    print(f"Received: {data}")
    return f"Echo: {data}"

r = OpenSocketResponser("app.knotlinksdk.test", "test_socket")
r.set_RecvFunc(on_request)
print("Responser running... (Ctrl+C to stop)")
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopped")