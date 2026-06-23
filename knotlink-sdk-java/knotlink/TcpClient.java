/*
 * KnotLink SDK - Java
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.io.*;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class TcpClient {
    private Socket socket;
    private OutputStream out;
    private InputStream in;
    private ScheduledExecutorService scheduler;
    private final byte[] heartbeatMessageBytes = "heartbeat".getBytes();
    private final byte[] heartbeatResponseBytes = "heartbeat_response".getBytes();
    private boolean running = false;

    // 接收缓冲区
    private ByteBuffer buffer = ByteBuffer.allocate(0);
    private static final int MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB

    public TcpClient() {
    }

    // ---------- 连接服务器 ----------
    public boolean connectToServer(String host, int port) {
        try {
            this.socket = new Socket(host, port);
            this.out = this.socket.getOutputStream();
            this.in = this.socket.getInputStream();
            System.out.println("Connected to server at " + host + ":" + port);
            this.running = true;
            this.startHeartbeat();
            new Thread(this::readData).start();
            return true;
        } catch (IOException e) {
            System.err.println("Failed to connect to server: " + e.getMessage());
            return false;
        }
    }

    // ---------- 发送数据（内部带长度前缀） ----------
    private void writeWithLengthPrefix(byte[] data) throws IOException {
        if (data.length > MAX_MSG_SIZE) {
            throw new IOException("Message too large: " + data.length);
        }
        // 构造 4 字节大端长度前缀
        ByteBuffer lengthBuf = ByteBuffer.allocate(4);
        lengthBuf.putInt(data.length); // 默认大端
        out.write(lengthBuf.array());
        out.write(data);
        out.flush();
    }

    // 对外发送 String（自动转字节）
    public void sendData(String data) {
        if (out == null) {
            System.err.println("Socket is not connected.");
            return;
        }
        try {
            byte[] bytes = data.getBytes("UTF-8");
            writeWithLengthPrefix(bytes);
        } catch (IOException e) {
            System.err.println("Send data error: " + e.getMessage());
        }
    }

    // 对外发送 byte[]
    public void sendData(byte[] data) {
        if (out == null) {
            System.err.println("Socket is not connected.");
            return;
        }
        try {
            writeWithLengthPrefix(data);
        } catch (IOException e) {
            System.err.println("Send data error: " + e.getMessage());
        }
    }

    // ---------- 心跳 ----------
    private void startHeartbeat() {
        if (this.scheduler == null || this.scheduler.isShutdown()) {
            this.scheduler = Executors.newSingleThreadScheduledExecutor();
        }
        this.scheduler.scheduleAtFixedRate(() -> {
            if (this.running && !socket.isClosed()) {
                try {
                    writeWithLengthPrefix(heartbeatMessageBytes);
                    System.out.println("发送心跳包成功");
                } catch (IOException e) {
                    System.err.println("发送心跳包失败：" + e.getMessage());
                }
            }
        }, 1L, 3L, TimeUnit.MINUTES);
    }

    private void stopHeartbeat() {
        this.running = false;
        if (this.scheduler != null) {
            this.scheduler.shutdown();
            this.scheduler = null;
        }
    }

    // ---------- 接收数据线程 ----------
    private void readData() {
        System.out.println("DEBUG: Start reading from server...");
        try {
            byte[] chunk = new byte[4096];
            while (running && !socket.isClosed()) {
                int bytesRead = in.read(chunk);
                if (bytesRead == -1) break;
                // 追加到缓冲区
                byte[] newData = new byte[buffer.remaining() + bytesRead];
                buffer = ByteBuffer.wrap(ByteBuffer.allocate(buffer.remaining() + bytesRead)
                        .put(buffer.array(), 0, buffer.remaining())
                        .put(chunk, 0, bytesRead).array());
                processBuffer();
            }
        } catch (IOException e) {
            System.err.println("Socket error: " + e.getMessage());
        } finally {
            stopHeartbeat();
            System.out.println("Server disconnected.");
        }
    }

    // ---------- 解析缓冲区 ----------
    private void processBuffer() {
        while (true) {
            if (buffer.remaining() < 4) break; // 长度字段未完整

            // 读取长度（大端）
            int length = buffer.getInt(0);
            if (length == 0 || length > MAX_MSG_SIZE) {
                System.err.println("Invalid message length: " + length + ", disconnecting");
                disconnect();
                return;
            }
            if (buffer.remaining() < 4 + length) break; // 消息体未完整

            // 提取消息体
            byte[] msgBytes = new byte[length];
            buffer.position(4);
            buffer.get(msgBytes);
            // 移除已处理的部分（包括长度前缀）
            buffer.position(4 + length);
            buffer = buffer.slice(); // 剩余部分

            // 处理消息：忽略心跳响应
            if (msgBytes.length == heartbeatResponseBytes.length && 
                new String(msgBytes).equals(new String(heartbeatResponseBytes))) {
                System.out.println("收到心跳响应，忽略");
            } else {
                System.out.println("收到数据：");
                // 转换为字符串并通知监听器
                try {
                    String received = new String(msgBytes, "UTF-8");
                    if (dataReceivedListener != null) {
                        dataReceivedListener.onDataReceived(received);
                    }
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            }
            // 继续处理剩余数据
        }
    }

    // ---------- 监听器接口 ----------
    public interface DataReceivedListener {
        void onDataReceived(String data);
    }

    private DataReceivedListener dataReceivedListener;

    public void setDataReceivedListener(DataReceivedListener listener) {
        this.dataReceivedListener = listener;
        System.out.println("DataReceivedListener set successfully.");
    }

    // ---------- 断开连接 ----------
    public void disconnect() {
        this.running = false;
        stopHeartbeat();
        try {
            if (this.socket != null && !this.socket.isClosed()) {
                this.socket.close();
            }
        } catch (IOException e) {
            System.err.println("Error closing socket: " + e.getMessage());
        }
        buffer.clear();
        System.out.println("已断开连接");
    }

    // 兼容旧 API
    public void close() {
        disconnect();
    }
}