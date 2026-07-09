/*
 * KnotLink SDK - C# Integration Test
 * Request-Response pattern: Responser + Querier
 */

using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

internal static class TestRequestResponseIntegration
{
    private static readonly CancellationTokenSource Cts = new();

    public static async Task Main()
    {
        var t = Task.Run(RunResponser);
        await Task.Delay(100);
        await RunQuerier();
        Cts.Cancel();
        await t;
    }

    private static async Task RunResponser()
    {
        var res = new OpenSocketResponser("app.knotlinksdk.test", "test_socket")
        {
            OnQuestionAsync = data =>
            {
                Console.WriteLine($"[Responser] Received: {data}");
                return Task.FromResult($"Echo: {data}");
            }
        };

        Console.WriteLine("[Responser] Running...");
        while (!Cts.IsCancellationRequested)
        {
            await Task.Delay(1000, Cts.Token);
        }
    }

    private static async Task RunQuerier()
    {
        await Task.Delay(1000);
        var q = new OpenSocketQuerier("app.knotlinksdk.test", "test_socket");
        try
        {
            var result = q.Query("Hello, Responser!");
            Console.WriteLine($"[Querier] Response: {result}");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[Querier] Error: {ex.Message}");
        }
        finally
        {
            q.Dispose();
        }
    }
}
