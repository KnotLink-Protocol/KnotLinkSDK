// unit test: OpenSocketResponser (nogui)
using System;
using System.Threading;
using System.Threading.Tasks;
using KnotLink;

var res = new OpenSocketResponser("app.knotlinksdk.test", "test_socket")
{
    OnQuestionAsync = data =>
    {
        Console.WriteLine($"Received: {data}");
        return Task.FromResult($"Echo: {data}");
    }
};
Console.WriteLine("Responser running...");
Thread.Sleep(60000);
res.Dispose();
