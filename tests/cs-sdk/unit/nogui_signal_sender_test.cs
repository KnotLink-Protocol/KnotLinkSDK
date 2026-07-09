// unit test: SignalSender (nogui)
using System;
using System.Threading.Tasks;
using KnotLink;

var sender = new SignalSender("app.knotlinksdk.test", "test_signal");
await sender.EmitAsync("Hello from Sender!");
Console.WriteLine("Signal sent.");
sender.Dispose();
