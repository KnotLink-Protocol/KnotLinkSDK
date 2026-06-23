from knotlink import OpenSocketQuerier

q = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
try:
    result = q.query("Hello, Responser!")
    print(f"Response: {result}")
except Exception as e:
    print(f"Error: {e}")