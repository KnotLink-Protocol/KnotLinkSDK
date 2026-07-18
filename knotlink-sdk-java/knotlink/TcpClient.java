/*
 * KnotLink SDK - Java
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.io.*;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class TcpClient implements AutoCloseable {
    private Socket socket;
    private OutputStream out;
    private InputStream in;
    private ScheduledExecutorService scheduler;
    private Thread readThread;
    private final byte[] heartbeatMessageBytes = "heartbeat".getBytes(StandardCharsets.UTF_8);
    private final byte[] heartbeatResponseBytes = "heartbeat_response".getBytes(StandardCharsets.UTF_8);
    private volatile boolean running = false;

    // 接收缓冲区
    private ByteBuffer buffer = ByteBuffer.allocate(0);
    private static final int MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB
    private static final byte[] MAGIC = {0x4B, 0x4B, 0x00, 0x02}; // KK + 版本号 2.0
    private static final int MAGIC_LEN = 4;
    private static final int HEADER_LEN = MAGIC_LEN + 4; // 8: 魔数 + 长度字段

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
            this.readThread = new Thread(this::readData, "KnotLink-TcpClient-Reader");
            this.readThread.setDaemon(true);
            this.readThread.start();
            return true;
        } catch (IOException e) {
            System.err.println("Failed to connect to server: " + e.getMessage());
            return false;
        }
    }

    // ---------- 发送数据（内部带长度前缀） ----------
    private synchronized void writeWithLengthPrefix(byte[] data) throws IOException {
        if (data.length > MAX_MSG_SIZE) {
            throw new IOException("Message too large: " + data.length);
        }
        // 构造协议头：4 字节魔数 + 4 字节大端长度
        ByteBuffer headerBuf = ByteBuffer.allocate(HEADER_LEN);
        headerBuf.put(MAGIC);
        headerBuf.putInt(data.length); // 默认大端
        out.write(headerBuf.array());
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
            byte[] bytes = data.getBytes(StandardCharsets.UTF_8);
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
            if (this.running && socket != null && !socket.isClosed()) {
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
            this.scheduler.shutdownNow();
            this.scheduler = null;
        }
    }

    // ---------- 接收数据线程 ----------
    private void readData() {
        System.out.println("DEBUG: Start reading from server...");
        try {
            byte[] chunk = new byte[4096];
            while (running && socket != null && !socket.isClosed()) {
                int bytesRead = in.read(chunk);
                if (bytesRead == -1) break;
                appendToBuffer(chunk, bytesRead);
                processBuffer();
            }
        } catch (IOException e) {
            System.err.println("Socket error: " + e.getMessage());
        } catch (RuntimeException e) {
            System.err.println("Socket protocol error: " + e.getMessage());
        } finally {
            stopHeartbeat();
            System.out.println("Server disconnected.");
        }
    }

    private void appendToBuffer(byte[] chunk, int bytesRead) {
        byte[] pending = new byte[buffer.remaining()];
        buffer.get(pending);

        byte[] newData = new byte[pending.length + bytesRead];
        System.arraycopy(pending, 0, newData, 0, pending.length);
        System.arraycopy(chunk, 0, newData, pending.length, bytesRead);
        buffer = ByteBuffer.wrap(newData);
    }

    // ---------- 解析缓冲区 ----------
    private void processBuffer() {
        while (true) {
            if (buffer.remaining() < HEADER_LEN) break; // 协议头未完整

            // 用 duplicate 窥探协议头，不消费
            ByteBuffer dup = buffer.duplicate();

            // 校验魔数
            byte[] magicChk = new byte[MAGIC_LEN];
            dup.get(magicChk);
            if (!Arrays.equals(magicChk, MAGIC)) {
                System.err.println("Magic mismatch, disconnecting");
                disconnect();
                return;
            }

            // 读取长度（大端）
            int length = dup.getInt();
            if (length <= 0 || length > MAX_MSG_SIZE) {
                System.err.println("Invalid message length: " + length + ", disconnecting");
                disconnect();
                return;
            }
            if (dup.remaining() < length) break; // 消息体未完整

            // 消费协议头 + 消息体
            buffer.position(buffer.position() + HEADER_LEN);
            byte[] msgBytes = new byte[length];
            buffer.get(msgBytes);
            buffer = buffer.slice();

            // 处理消息：忽略心跳响应
            if (Arrays.equals(msgBytes, heartbeatResponseBytes)) {
                System.out.println("收到心跳响应，忽略");
            } else {
                System.out.println("收到数据：");
                String received = new String(msgBytes, StandardCharsets.UTF_8);
                if (dataReceivedListener != null) {
                    try {
                        dataReceivedListener.onDataReceived(received);
                    } catch (RuntimeException e) {
                        System.err.println("DataReceivedListener error: " + e.getMessage());
                    }
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
        buffer = ByteBuffer.allocate(0);
        System.out.println("已断开连接");
    }

    // 兼容旧 API
    @Override
    public void close() {
        disconnect();
    }
}
