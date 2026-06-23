const net = require('net');

class TcpClient {
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
        // 构造 4 字节大端长度前缀
        const lengthBuf = Buffer.alloc(4);
        lengthBuf.writeUInt32BE(data.length, 0);
        // 发送前缀 + 数据
        this.tcpSocket.write(Buffer.concat([lengthBuf, data]));
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
        while (true) {
            if (this.buffer.length < 4) break; // 长度字段未完整

            // 读取长度（大端）
            const length = this.buffer.readUInt32BE(0);
            if (length === 0 || length > this.MAX_MSG_SIZE) {
                console.log(`无效消息长度: ${length}，断开连接`);
                this.disconnect();
                return;
            }
            if (this.buffer.length < 4 + length) break; // 消息体未完整

            // 提取消息体
            const msg = this.buffer.slice(4, 4 + length);
            // 移除已处理的部分
            this.buffer = this.buffer.slice(4 + length);

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