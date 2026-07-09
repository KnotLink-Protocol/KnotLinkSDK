// loop test: Request-Response (Querier + Responser)
using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

const int MAX_ITERATIONS = 5;
var cts = new CancellationTokenSource();

var t = Task.Run(() => RunResponser(cts.Token));
await Task.Delay(100);
await RunQuerier(cts.Token);
cts.Cancel();
await t;

static async Task RunResponser(CancellationToken ct)
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
    while (!ct.IsCancellationRequested)
        await Task.Delay(1000, ct);
}

static async Task RunQuerier(CancellationToken ct)
{
    await Task.Delay(1000, ct);
    var q = new OpenSocketQuerier("app.knotlinksdk.test", "test_socket");
    try
    {
        for (int i = 1; i <= MAX_ITERATIONS && !ct.IsCancellationRequested; i++)
        {
            var data = $"Hello #{i}";
            try
            {
                var result = q.Query(data);
                Console.WriteLine($"[Querier] Req: {data} -> Res: {result}");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[Querier] Error: {ex.Message}");
            }
            await Task.Delay(3000, ct);
        }
    }
    finally { q.Dispose(); }
}
