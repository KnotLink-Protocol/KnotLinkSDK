// test.js
const OpenSocketQuerier = require('./OpenSocketQuerier');
const OpenSocketResponser = require('./OpenSocketResponser');

// 模拟一个响应器
const responser = new OpenSocketResponser('TestApp', 'QueryService');

// 监听请求事件
responser.on('request', (questionID, requestData) => {
    console.log(`[Responser] 收到请求 ID: ${questionID}, 数据: ${requestData}`);
    // 模拟处理并回复
    const response = `已处理: ${requestData}`;
    responser.sendBack(response, questionID);
});

// 等待响应器初始化完成
setTimeout(() => {
    // 创建查询器
    const querier = new OpenSocketQuerier('TestApp', 'QueryService');

    // 使用异步回调方式查询
    querier.query('Hello, Server!', (err, result) => {
        if (err) {
            console.error('[Querier] 查询失败:', err);
        } else {
            console.log('[Querier] 回调结果:', result);
        }
    });

    // 使用 Promise 方式查询
    querier.querySync('Another query')
        .then(result => {
            console.log('[Querier] Promise 结果:', result);
        })
        .catch(err => {
            console.error('[Querier] Promise 失败:', err);
        });

}, 1000); // 等待 responser 连接和注册完成