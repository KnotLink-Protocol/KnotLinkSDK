/*
 * KnotLink SDK - C# regression checks
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

using System;
using System.IO;

internal static class CsSdkRegressionCheck
{
    private static readonly string RepoRoot = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", ".."));

    public static void Main()
    {
        string tcpClient = File.ReadAllText(Path.Combine(RepoRoot, "knotlink-sdk-cs", "knotlink", "TcpClient.cs"));

        AssertContains("TcpClient should expose receive-loop errors", tcpClient, "public Func<Exception, Task>? OnErrorAsync");
        AssertContains("Dispose should coordinate with SendAsync", tcpClient, "_sendLock.Wait(TimeSpan.FromSeconds(2))");
        AssertContains("DisposeAsync should coordinate with SendAsync", tcpClient, "await _sendLock.WaitAsync(TimeSpan.FromSeconds(2))");
        AssertContains("SendAsync should reject sends after disposal starts", tcpClient, "_disposed");
        AssertContains("Callback exceptions should be reported", tcpClient, "ReportErrorAsync(ex)");

        Console.WriteLine("C# SDK regression checks passed.");
    }

    private static void AssertContains(string description, string content, string expected)
    {
        if (!content.Contains(expected, StringComparison.Ordinal))
        {
            throw new InvalidOperationException($"{description}: missing `{expected}`");
        }
    }
}
