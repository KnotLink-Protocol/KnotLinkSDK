from knotlink import SignalSender

sender = SignalSender()
sender.set_config("app.knotlinksdk.test", "test_signal")
sender.emitt("Hello from Sender!")
print("Signal sent.")