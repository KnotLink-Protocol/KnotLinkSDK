/*
 * KnotLink SDK - C# Integration Test
 * Signal-Subscribe pattern: Subscriber + Sender
 */

using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

internal static class TestSignalSubscribeIntegration
{
    private static readonly CancellationTokenSource Cts = new();

    public static async Task Main()
    {
        var t = Task.Run(RunSubscriber);
        await Task.Delay(100);
        await RunSender();
        Cts.Cancel();
        await t;
    }

    private static async Task RunSubscriber()
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
        while (!Cts.IsCancellationRequested)
        {
            await Task.Delay(1000, Cts.Token);
        }
    }

    private static async Task RunSender()
    {
        await Task.Delay(1000);
        var sender = new SignalSender("app.knotlinksdk.test", "test_signal");
        try
        {
            await sender.EmitAsync("Hello from Sender!");
            Console.WriteLine("[Sender] Signal sent.");
        }
        finally
        {
            sender.Dispose();
        }
    }
}
