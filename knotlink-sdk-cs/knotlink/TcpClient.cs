/*
 * KnotLink SDK - C#
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace KnotLink
{
    public sealed class KlTcpClient : IAsyncDisposable, IDisposable
    {
        private const string HeartbeatMessage = "heartbeat";
        private const string HeartbeatResponse = "heartbeat_response";
        private const int MaxMessageSize = 16 * 1024 * 1024; // 16MB

        private readonly TimeSpan _heartbeatInterval;
        private TcpClient? _client;
        private NetworkStream? _stream;
        private readonly CancellationTokenSource _cts = new();
        private Task? _readTask;
        private Task? _heartbeatTask;
        private readonly byte[] _lenBuffer = new byte[4];
        private readonly MemoryStream _recvBuffer = new();

        public Func<string, Task>? OnDataReceivedAsync { get; set; }
        public bool Running => _client?.Connected == true && !_cts.IsCancellationRequested;

        public KlTcpClient(TimeSpan? heartbeatInterval = null)
        {
            _heartbeatInterval = heartbeatInterval ?? TimeSpan.FromMinutes(3);
        }

        // ---------- 连接 ----------
        public async Task<bool> ConnectAsync(string host, int port)
        {
            _client = new TcpClient();
            await _client.ConnectAsync(host, port).ConfigureAwait(false);
            _stream = _client.GetStream();

            _readTask = Task.Run(() => ReadLoopAsync(_cts.Token));
            _heartbeatTask = Task.Run(() => HeartbeatLoopAsync(_cts.Token));
            return true;
        }

        // ---------- 发送（加长度前缀） ----------
        public async Task SendAsync(string data, CancellationToken cancellationToken = default)
        {
            if (_stream == null) throw new InvalidOperationException("Not connected.");

            byte[] payload = Encoding.UTF8.GetBytes(data);
            if (payload.Length > MaxMessageSize)
                throw new ArgumentException($"Message too large: {payload.Length} > {MaxMessageSize}");

            // 4 字节大端长度头
            byte[] lenBytes = BitConverter.GetBytes(payload.Length);
            if (BitConverter.IsLittleEndian) Array.Reverse(lenBytes);

            await _stream.WriteAsync(lenBytes, 0, 4, cancellationToken).ConfigureAwait(false);
            await _stream.WriteAsync(payload, 0, payload.Length, cancellationToken).ConfigureAwait(false);
            await _stream.FlushAsync(cancellationToken).ConfigureAwait(false);
        }

        // ---------- 接收循环（缓冲 + 解析长度前缀） ----------
        private async Task ReadLoopAsync(CancellationToken cancellationToken)
        {
            if (_stream == null) return;

            byte[] chunk = new byte[4096];
            try
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    int bytesRead = await _stream.ReadAsync(chunk, 0, chunk.Length, cancellationToken).ConfigureAwait(false);
                    if (bytesRead == 0) break; // 对端关闭

                    lock (_recvBuffer)
                    {
                        _recvBuffer.Write(chunk, 0, bytesRead);
                    }

                    // 尽可能多地解析出完整消息
                    while (TryExtractMessage(out byte[]? msgBytes))
                    {
                        if (msgBytes == null) break;
                        string text = Encoding.UTF8.GetString(msgBytes);
                        if (text == HeartbeatResponse)
                            continue; // 忽略心跳回复

                        if (OnDataReceivedAsync != null)
                            await OnDataReceivedAsync(text).ConfigureAwait(false);
                    }
                }
            }
            catch (OperationCanceledException) { /* 正常退出 */ }
            catch (Exception) { /* 网络错误，静默退出 */ }
            finally
            {
                // 连接断开，取消所有任务
                _cts.Cancel();
            }
        }

        // ---------- 从缓冲区提取一条完整消息 ----------
        private bool TryExtractMessage(out byte[]? message)
        {
            message = null;
            lock (_recvBuffer)
            {
                long bufferLen = _recvBuffer.Length;
                if (bufferLen < 4) return false; // 连长度头都不够

                // 读取长度头（大端）
                _recvBuffer.Position = 0;
                _recvBuffer.Read(_lenBuffer, 0, 4);
                int msgLen = BitConverter.ToInt32(_lenBuffer, 0);
                if (BitConverter.IsLittleEndian) msgLen = System.Net.IPAddress.NetworkToHostOrder(msgLen);

                if (msgLen <= 0 || msgLen > MaxMessageSize)
                {
                    // 无效长度，清空缓冲区并断开
                    _recvBuffer.SetLength(0);
                    _recvBuffer.Position = 0;
                    throw new InvalidDataException($"Invalid message length: {msgLen}");
                }

                if (bufferLen < 4 + msgLen) return false; // 消息体未完整

                // 取出完整消息
                byte[] msg = new byte[msgLen];
                _recvBuffer.Position = 4;
                _recvBuffer.Read(msg, 0, msgLen);

                // 删除已读取的数据（4 + msgLen）
                byte[] remaining = new byte[bufferLen - 4 - msgLen];
                _recvBuffer.Position = 4 + msgLen;
                _recvBuffer.Read(remaining, 0, remaining.Length);
                _recvBuffer.SetLength(0);
                _recvBuffer.Position = 0;
                _recvBuffer.Write(remaining, 0, remaining.Length);

                message = msg;
                return true;
            }
        }

        // ---------- 心跳循环 ----------
        private async Task HeartbeatLoopAsync(CancellationToken cancellationToken)
        {
            try
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    await Task.Delay(_heartbeatInterval, cancellationToken).ConfigureAwait(false);
                    try
                    {
                        await SendAsync(HeartbeatMessage, cancellationToken).ConfigureAwait(false);
                    }
                    catch { /* 忽略心跳发送失败 */ }
                }
            }
            catch (OperationCanceledException) { /* 正常 */ }
        }

        // ---------- 释放 ----------
        public void Dispose()
        {
            _cts.Cancel();
            try { _readTask?.Wait(TimeSpan.FromSeconds(2)); } catch { }
            try { _heartbeatTask?.Wait(TimeSpan.FromSeconds(2)); } catch { }

            _stream?.Dispose();
            _client?.Close();
            _cts.Dispose();
            _recvBuffer.Dispose();
        }

        public async ValueTask DisposeAsync()
        {
            _cts.Cancel();
            if (_readTask != null) try { await _readTask; } catch { }
            if (_heartbeatTask != null) try { await _heartbeatTask; } catch { }

            _stream?.Dispose();
            _client?.Close();
            _cts.Dispose();
            await _recvBuffer.DisposeAsync();
        }
    }
}