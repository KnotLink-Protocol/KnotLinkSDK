/*
 * KnotLink SDK - JavaScript
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

const net = require('net');

class TcpClient {
    static MAGIC = Buffer.from([0x4B, 0x4B, 0x00, 0x02]); // KK + 版本号 2.0
    static MAGIC_LEN = 4;
    static HEADER_LEN = 8; // 魔数 + 长度字段

    constructor() {
        this.tcpSocket = new net.Socket();
        this.heartBeatInterval = 180000; // 3分钟
        this.heartBeatTimer = null;
        this.connected = false;
        this.receivedDataCallback = null;
        this.buffer = Buffer.alloc(0);        // 接收缓冲区
        this.MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB
    }

    // ---------- 连接服务器 ----------
    connectToServer(ip, port, callback) {
        this.tcpSocket.connect(port, ip, () => {
            this.connected = true;
            console.log("连接成功");
            this.startHeartBeat();
            if (callback) callback();
        });

        // 数据接收：追加到缓冲区并解析
        this.tcpSocket.on('data', (chunk) => {
            // 追加到缓冲区
            this.buffer = Buffer.concat([this.buffer, chunk]);
            this._processBuffer();
        });

        this.tcpSocket.on('error', (err) => {
            console.log("连接失败或接收数据失败：", err);
            this.connected = false;
        });

        this.tcpSocket.on('close', () => {
            console.log("连接已断开");
            this.connected = false;
            if (this.heartBeatTimer) {
                clearInterval(this.heartBeatTimer);
                this.heartBeatTimer = null;
            }
        });
    }

    // ---------- 发送数据（自动加长度前缀） ----------
    _writeWithLengthPrefix(data) {
        if (!this.connected) return;
        if (data.length > this.MAX_MSG_SIZE) {
            throw new Error(`消息过长: ${data.length} > ${this.MAX_MSG_SIZE}`);
        }
        // 构造协议头：4 字节魔数 + 4 字节大端长度前缀
        const lengthBuf = Buffer.alloc(4);
        lengthBuf.writeUInt32BE(data.length, 0);
        this.tcpSocket.write(Buffer.concat([TcpClient.MAGIC, lengthBuf, data]));
    }

    send_data(data) {
        this._writeWithLengthPrefix(data);
    }

    // ---------- 发送心跳 ----------
    sendHeartBeat() {
        if (this.connected) {
            this._writeWithLengthPrefix(Buffer.from("heartbeat", 'utf8'));
            console.log("发送心跳包成功");
        } else {
            console.log("无法发送心跳包，连接已断开。");
            if (this.heartBeatTimer) {
                clearInterval(this.heartBeatTimer);
                this.heartBeatTimer = null;
            }
        }
    }

    // ---------- 启动心跳定时器 ----------
    startHeartBeat() {
        if (this.heartBeatTimer) clearInterval(this.heartBeatTimer);
        this.heartBeatTimer = setInterval(() => {
            this.sendHeartBeat();
        }, this.heartBeatInterval);
    }

    // ---------- 接收缓冲解析 ----------
    _processBuffer() {
        const HEADER_LEN = TcpClient.HEADER_LEN;
        const MAGIC_LEN = TcpClient.MAGIC_LEN;
        while (true) {
            if (this.buffer.length < HEADER_LEN) break; // 协议头未完整

            // 校验魔数
            if (Buffer.compare(this.buffer.slice(0, MAGIC_LEN), TcpClient.MAGIC) !== 0) {
                console.log("魔数不匹配，断开连接");
                this.disconnect();
                return;
            }

            // 读取长度（大端，在魔数之后）
            const length = this.buffer.readUInt32BE(MAGIC_LEN);
            if (length === 0 || length > this.MAX_MSG_SIZE) {
                console.log(`无效消息长度: ${length}，断开连接`);
                this.disconnect();
                return;
            }
            if (this.buffer.length < HEADER_LEN + length) break; // 消息体未完整

            // 提取消息体（跳过魔数 + 长度）
            const msg = this.buffer.slice(HEADER_LEN, HEADER_LEN + length);
            this.buffer = this.buffer.slice(HEADER_LEN + length);

            // 处理消息：忽略心跳响应，其他触发回调
            if (msg.toString() === "heartbeat_response") {
                console.log("收到心跳响应，忽略");
            } else {
                console.log("收到数据：", msg);
                if (this.receivedDataCallback) {
                    this.receivedDataCallback(msg);
                }
            }
            // 继续循环处理可能存在的更多消息
        }
    }

    // ---------- 断开连接 ----------
    disconnect() {
        this.connected = false;
        this.tcpSocket.destroy();
        if (this.heartBeatTimer) {
            clearInterval(this.heartBeatTimer);
            this.heartBeatTimer = null;
        }
        this.buffer = Buffer.alloc(0);
        console.log("已断开连接");
    }
}

module.exports = TcpClient;