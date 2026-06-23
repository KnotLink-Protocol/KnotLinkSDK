/*
 * KnotLink SDK - JavaScript
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

// OpenSocketResponser.js
const TcpClient = require('./TcpClient');

class OpenSocketResponser {
    /**
     * 构造函数
     * @param {string} APPID - 应用ID
     * @param {string} OpenSocketID - 开放套接字ID
     */
    constructor(APPID, OpenSocketID) {
        this.appID = APPID;
        this.openSocketID = OpenSocketID;
        this.tcpClient = new TcpClient();

        // 连接服务器（端口 6378）
        this.tcpClient.connectToServer('127.0.0.1', 6378, () => {
            console.log('[Responser] 连接到服务器成功');
            // 注册自己：发送 APPID-OpenSocketID
            const regKey = `${this.appID}-${this.openSocketID}`;
            this.tcpClient.send_data(Buffer.from(regKey, 'utf-8'));
            console.log('[Responser] 注册 Key:', regKey);
        });

        // 设置数据接收回调
        this.tcpClient.receivedDataCallback = (data) => {
            this._handleRequest(data);
        };
    }

    /**
     * 处理收到的请求
     */
    _handleRequest(data) {
        const raw = data.toString('utf-8');
        console.log('[Responser] 收到请求:', raw);

        // 解析格式: questionID&*&actualData
        const delimiter = '&*&';
        const parts = raw.split(delimiter);
        if (parts.length !== 2) {
            console.error('[Responser] 数据格式错误，预期包含分隔符', delimiter);
            return;
        }

        const questionID = parts[0];
        const requestData = parts[1];

        // 触发事件：外部通过监听 'request' 事件处理
        this.emit('request', questionID, requestData);
    }

    /**
     * 回复请求
     * @param {string|Buffer} data - 回复的数据
     * @param {string} questionID - 对应的请求ID
     */
    sendBack(data, questionID) {
        if (typeof data === 'string') {
            data = Buffer.from(data, 'utf-8');
        }
        const response = `${questionID}&*&${data.toString('utf-8')}`;
        this.tcpClient.send_data(Buffer.from(response, 'utf-8'));
        console.log('[Responser] 回复:', response);
    }

    /**
     * 简单的 EventEmitter 风格
     */
    on(event, handler) {
        if (event === 'request') {
            this._requestHandler = handler;
        }
    }

    emit(event, ...args) {
        if (event === 'request' && this._requestHandler) {
            this._requestHandler(...args);
        }
    }

    /**
     * 断开连接
     */
    disconnect() {
        this.tcpClient.disconnect();
    }
}

module.exports = OpenSocketResponser;