// unit test: OpenSocketQuerier (nogui)
using System;
using KnotLink;

var q = new OpenSocketQuerier("app.knotlinksdk.test", "test_socket");
try
{
    var result = q.Query("Hello, Responser!");
    Console.WriteLine($"Response: {result}");
}
catch (Exception e)
{
    Console.Error.WriteLine($"Error: {e.Message}");
}
q.Dispose();
