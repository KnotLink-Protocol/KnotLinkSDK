// main.js
const SignalSender = require('./SignalSender'); // 引入 SignalSender 类

const signalSender = new SignalSender("1", "1");

// 等待一段时间以确保数据发送完成
setTimeout(() => {
    signalSender.emitt("Hello, Server!");
}, 10000);

/*
// main.js
const SignalSubscriber = require('./SignalSubscriber'); // 引入 SignalSubscriber 类

const subscriber = new SignalSubscriber("1", "1");

subscriber.setRecvFunc((data) => {
    console.log(`Callback received data: ${data}`);
});

// 等待一段时间以接收数据
setTimeout(() => {
    subscriber.KLsubscriber.disconnect();
}, 60000); // 等待60秒后断开连接
*/