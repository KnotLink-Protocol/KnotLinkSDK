// SignalSubscriber.js
const TcpClient = require('./TcpClient'); // 确保 TcpClient.js 在同一目录下

class SignalSubscriber {
    constructor(APPID, SignalID) {
        this.appID = APPID;
        this.signalID = SignalID;
        this.callback = null; // 用于存储回调函数
        this.init();
    }

    init() {
        this.KLsubscriber = new TcpClient();
        this.KLsubscriber.connectToServer("127.0.0.1", 6372, () => {
            console.log("OKK");
            const sKey = `${this.appID}-${this.signalID}`;
            const sKeyBytes = Buffer.from(sKey, 'utf-8');
            this.KLsubscriber.send_data(sKeyBytes);
        });

        this.KLsubscriber.receivedDataCallback = (data) => {
            this.dataRecv(data);
        };
    }

    subscribe(APPID, SignalID) {
        this.appID = APPID;
        this.signalID = SignalID;
        const sKey = `${this.appID}-${this.signalID}`;
        const sKeyBytes = Buffer.from(sKey, 'utf-8');
        this.KLsubscriber.send_data(sKeyBytes);
    }

    dataRecv(data) {
        try {
            const receivedText = data.toString('utf-8'); // 解码为字符串
            if (this.callback) {
                this.callback(receivedText);
            }
        } catch (e) {
            console.error(`解码失败: ${e}`);
        }
    }

    setRecvFunc(callback) {
        this.callback = callback;
    }
}

module.exports = SignalSubscriber;