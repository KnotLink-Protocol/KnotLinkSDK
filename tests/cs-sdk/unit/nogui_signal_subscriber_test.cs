// unit test: SignalSubscriber (nogui)
using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

var sub = new SignalSubscriber("app.knotlinksdk.test", "test_signal")
{
    OnSignalAsync = data =>
    {
        Console.WriteLine($"Received: {data}");
        return Task.CompletedTask;
    }
};
Console.WriteLine("Subscriber listening...");
Thread.Sleep(60000);
sub.Dispose();
