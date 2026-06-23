import time
from knotlink import SignalSubscriber

def on_received(data):
    print(f"Received: {data}")

sub = SignalSubscriber("app.knotlinksdk.test", "test_signal")
sub.set_RecvFunc(on_received)
print("Listening... (Ctrl+C to stop)")
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopped")