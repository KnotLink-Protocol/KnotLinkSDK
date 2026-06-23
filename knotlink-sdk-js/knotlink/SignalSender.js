/*
 * KnotLink SDK - JavaScript
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

// SignalSender.js
const TcpClient = require('./TcpClient'); // 确保 TcpClient.js 在同一目录下

class SignalSender {
    constructor(APPID = null, SignalID = null) {
        this.appID = APPID;
        this.signalID = SignalID;
        this.KLsender = new TcpClient();
        this.KLsender.connectToServer("127.0.0.1", 6370,() => {
            console.log("OKK");
        });

    }

    setConfig(APPID, SignalID) {
        this.appID = APPID;
        this.signalID = SignalID;
    }

    emitt(data) {
        this._emitt(this.appID, this.signalID, data);
    }

    _emitt(APPID, SignalID, data) {
        if (typeof data === 'string') {
            data = Buffer.from(data, 'utf-8');
        }
        this.__emitt(APPID, SignalID, data);
    }

    __emitt(APPID, SignalID, data) {
        const sKey = `${APPID}-${SignalID}&*&`;
        const sKeyBytes = Buffer.from(sKey, 'utf-8');
        const sData = Buffer.concat([sKeyBytes, data]);
        this.KLsender.send_data(sData);
    }
}

// 导出 SignalSender 类
module.exports = SignalSender;