// OpenSocketQuerier.js
const TcpClient = require('./TcpClient');

class OpenSocketQuerier {
    /**
     * 构造函数
     * @param {string} APPID - 应用ID
     * @param {string} OpenSocketID - 开放套接字ID
     */
    constructor(APPID = null, OpenSocketID = null) {
        this.appID = APPID;
        this.openSocketID = OpenSocketID;
        this.tcpClient = new TcpClient();
        this.responseCallback = null; // 用于异步回调
        this.pendingResolve = null;   // 用于 Promise 同步
        this.pendingReject = null;

        // 连接服务器（端口 6376）
        this.tcpClient.connectToServer('127.0.0.1', 6376, () => {
            console.log('[Querier] 连接到服务器成功');
        });

        // 设置数据接收回调
        this.tcpClient.receivedDataCallback = (data) => {
            this._handleData(data);
        };
    }

    /**
     * 设置当前使用的 APPID 和 OpenSocketID
     */
    setConfig(APPID, OpenSocketID) {
        this.appID = APPID;
        this.openSocketID = OpenSocketID;
    }

    /**
     * 异步查询（不阻塞，通过回调返回结果）
     * @param {string|Buffer} data - 要发送的数据
     * @param {Function} callback - 回调函数 (err, result) => {}
     */
    query(data, callback) {
        if (typeof data === 'string') {
            data = Buffer.from(data, 'utf-8');
        }
        this.responseCallback = callback;
        this._sendQuery(this.appID, this.openSocketID, data);
    }

    /**
     * 同步查询（返回 Promise，await 方式等待结果）
     * @param {string|Buffer} data - 要发送的数据
     * @returns {Promise<string>} 返回响应的字符串
     */
    querySync(data) {
        return new Promise((resolve, reject) => {
            if (typeof data === 'string') {
                data = Buffer.from(data, 'utf-8');
            }
            this.pendingResolve = resolve;
            this.pendingReject = reject;
            this._sendQuery(this.appID, this.openSocketID, data);
        });
    }

    /**
     * 临时更换 APPID 和 OpenSocketID 发送查询（异步）
     */
    queryWith(APPID, OpenSocketID, data, callback) {
        if (typeof data === 'string') {
            data = Buffer.from(data, 'utf-8');
        }
        this.responseCallback = callback;
        this._sendQuery(APPID, OpenSocketID, data);
    }

    /**
     * 内部发送方法
     */
    _sendQuery(APPID, OpenSocketID, data) {
        const key = `${APPID}-${OpenSocketID}&*&`;
        const keyBuffer = Buffer.from(key, 'utf-8');
        const packet = Buffer.concat([keyBuffer, data]);
        this.tcpClient.send_data(packet);
    }

    /**
     * 处理接收到的数据
     */
    _handleData(data) {
        const result = data.toString('utf-8');
        console.log('[Querier] 收到响应:', result);

        // 如果有回调，调用回调
        if (this.responseCallback) {
            const cb = this.responseCallback;
            this.responseCallback = null;
            cb(null, result);
        }

        // 如果有等待的 Promise，解析它
        if (this.pendingResolve) {
            const resolve = this.pendingResolve;
            const reject = this.pendingReject;
            this.pendingResolve = null;
            this.pendingReject = null;
            resolve(result);
        }
    }

    /**
     * 断开连接
     */
    disconnect() {
        this.tcpClient.disconnect();
    }
}

module.exports = OpenSocketQuerier;