/*
 * KnotLink SDK - Java
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.util.logging.Logger;

/**
 * 持久化 TCP 查询器（单请求模式，与 Rust 版本语义一致）。
 * 同一时间仅支持一个等待响应的请求，后发会覆盖前一个。
 */
public class OpenSocketQuerier implements AutoCloseable {
    private static final Logger LOGGER = Logger.getLogger(OpenSocketQuerier.class.getName());
    private static final String DEFAULT_SERVER_ADDR = "127.0.0.1:6376";

    private final String appId;
    private final String openSocketId;
    private final TcpClient tcpClient;

    // 当前等待响应的 Future（同一时间仅一个）
    private final AtomicReference<CompletableFuture<String>> pendingFuture = new AtomicReference<>();

    // ---------- 构造函数 ----------
    public OpenSocketQuerier(String appId, String openSocketId, String serverAddr) throws IOException {
        this.appId = appId;
        this.openSocketId = openSocketId;

        String[] parts = serverAddr.split(":");
        if (parts.length != 2) {
            throw new IllegalArgumentException("Invalid server address format, expected host:port");
        }
        String host = parts[0];
        int port;
        try {
            port = Integer.parseInt(parts[1]);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid port number", e);
        }

        this.tcpClient = new TcpClient();
        if (!tcpClient.connectToServer(host, port)) {
            throw new IOException("Failed to connect to KnotLink server at " + serverAddr);
        }

        tcpClient.setDataReceivedListener(this::onDataReceived);
        LOGGER.info("OpenSocketQuerier connected to " + serverAddr);
    }

    public OpenSocketQuerier(String appId, String openSocketId) throws IOException {
        this(appId, openSocketId, DEFAULT_SERVER_ADDR);
    }

    // ---------- 数据接收回调 ----------
    private void onDataReceived(String data) {
        CompletableFuture<String> future = pendingFuture.getAndSet(null);
        if (future != null && !future.isDone()) {
            future.complete(data);
            LOGGER.fine(() -> "Query response received: " + data);
        } else {
            LOGGER.fine(() -> "Ignored unexpected data (no pending future): " + data);
        }
    }

    // ---------- 构建请求包 ----------
    private String buildPacket(String question) {
        return appId + "-" + openSocketId + "&*&" + question;
    }

    // ---------- 异步查询（等待响应） ----------
    public CompletableFuture<String> queryL(String question) {
        String packet = buildPacket(question);
        tcpClient.sendData(packet);

        CompletableFuture<String> future = new CompletableFuture<>();
        // 覆盖之前的 pending（与 Rust 行为一致）
        pendingFuture.set(future);
        LOGGER.fine(() -> "QueryL sent: " + question);
        return future;
    }

    // ---------- 发送查询但不等待响应 ----------
    public void queryAsync(String question) {
        String packet = buildPacket(question);
        tcpClient.sendData(packet);
        LOGGER.fine(() -> "QueryAsync sent: " + question);
    }

    // ---------- 同步阻塞查询 ----------
    public String query(String question, long timeout, TimeUnit unit) throws Exception {
        return queryL(question).get(timeout, unit);
    }

    public String query(String question) throws Exception {
        return query(question, 5, TimeUnit.SECONDS);
    }

    // ---------- 关闭 ----------
    @Override
    public void close() {
        CompletableFuture<String> future = pendingFuture.getAndSet(null);
        if (future != null && !future.isDone()) {
            future.cancel(true);
        }
        tcpClient.close();
        LOGGER.info("OpenSocketQuerier closed");
    }

    // ---------- 工厂方法 ----------
    public static OpenSocketQuerier connect(String appId, String openSocketId, String serverAddr) throws IOException {
        return new OpenSocketQuerier(appId, openSocketId, serverAddr);
    }

    public static OpenSocketQuerier connect(String appId, String openSocketId) throws IOException {
        return new OpenSocketQuerier(appId, openSocketId);
    }
}