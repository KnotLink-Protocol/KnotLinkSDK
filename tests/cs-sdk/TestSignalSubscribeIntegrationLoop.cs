// loop test: Signal-Subscribe (Sender + Subscriber)
using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

const int MAX_ITERATIONS = 5;
var cts = new CancellationTokenSource();

var t = Task.Run(() => RunSubscriber(cts.Token));
await Task.Delay(100);
await RunSender(cts.Token);
cts.Cancel();
await t;

static async Task RunSubscriber(CancellationToken ct)
{
    var sub = new SignalSubscriber("app.knotlinksdk.test", "test_signal")
    {
        OnSignalAsync = data =>
        {
            Console.WriteLine($"[Subscriber] Received: {data}");
            return Task.CompletedTask;
        }
    };
    Console.WriteLine("[Subscriber] Listening...");
    while (!ct.IsCancellationRequested)
        await Task.Delay(1000, ct);
}

static async Task RunSender(CancellationToken ct)
{
    await Task.Delay(1000, ct);
    var sender = new SignalSender("app.knotlinksdk.test", "test_signal");
    try
    {
        for (int i = 1; i <= MAX_ITERATIONS && !ct.IsCancellationRequested; i++)
        {
            var data = $"Signal #{i}";
            await sender.EmitAsync(data);
            Console.WriteLine($"[Sender] Sent: {data}");
            await Task.Delay(3000, ct);
        }
    }
    finally { sender.Dispose(); }
}
