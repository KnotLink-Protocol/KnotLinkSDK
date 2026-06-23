# KnotLink Python 客户端库

KnotLink 协议的 Python 实现，提供四种标准客户端身份：**询问者、回复者、发送者、订阅者**，以及键值对数据格式工具。

## 安装

```bash
pip install knotlink
```



## 快速开始

### 询问者 (OpenSocketQuerier)

```python
from knotlink import OpenSocketQuerier

querier = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
result = querier.query("Hello, Responser!")
print(result)
```

### 回复者 (OpenSocketResponser)

```python
from knotlink import OpenSocketResponser

def on_request(data):
    print(f"Received: {data}")
    return f"Echo: {data}"

responser = OpenSocketResponser("app.knotlinksdk.test", "test_socket")
responser.set_RecvFunc(on_request)
input("Press Enter to stop...")
```

### 发送者 (SignalSender)

```python
from knotlink import SignalSender

sender = SignalSender()
sender.set_config("app.knotlinksdk.test", "test_signal")
sender.emitt("Hello from Sender!")
```

### 订阅者 (SignalSubscriber)

```python
from knotlink import SignalSubscriber

def on_received(data):
    print(f"Received: {data}")

subscriber = SignalSubscriber("app.knotlinksdk.test", "test_signal")
subscriber.set_RecvFunc(on_received)
input("Press Enter to stop...")
```

### 键值对工具 (KLUDF)

KnotLink 数据格式标准化工具，提供键值对格式的序列化与反序列化能力。

```python
from knotlink import KLKVMap

# 创建键值对映射
kv = KLKVMap()
kv["name"] = "张三"
kv["course"] = "数学"
kv["score"] = "95"

# 序列化为字符串
serialized = kv.serialize()
print(serialized)  # "name=张三;course=数学;score=95"

# 反序列化为映射
new_kv = KLKVMap()
new_kv.deserialize(serialized)
print(new_kv.get("name"))  # "张三"
print(new_kv.get("missing", "默认值"))  # "默认值"
```



## 核心概念

| 身份                | 端口 | 说明                   |
| :------------------ | :--- | :--------------------- |
| 询问者 (Querier)    | 6376 | 主动发起请求，等待回复 |
| 回复者 (Responser)  | 6378 | 接收请求，回传结果     |
| 发送者 (Sender)     | 6370 | 单向发送信号           |
| 订阅者 (Subscriber) | 6372 | 监听信号，触发回调     |

## API 参考

### 询问者 OpenSocketQuerier

```python
OpenSocketQuerier(app_id, socket_id)
querier.query(data)                # 同步
await querier.query_async(data)    # 异步
```

### 回复者 OpenSocketResponser

```python
OpenSocketResponser(app_id, socket_id)
responser.set_RecvFunc(callback)
responser.send_back(data, question_id)
```

### 发送者 SignalSender

```python
SignalSender()
sender.set_config(app_id, signal_id)
sender.emitt(data)
```

### 订阅者 SignalSubscriber

```python
SignalSubscriber(app_id, signal_id)
subscriber.set_RecvFunc(callback)
```

### 键值对工具 KLKVMap

```python
KLKVMap()
kv.serialize()                     # 序列化为 "key1=val1;key2=val2" 格式
kv.deserialize(str)                # 从字符串反序列化
kv.get(key, default)               # 安全读取，键不存在时返回默认值（默认空字符串）
```

### 数据格式 KLUDF

```python
KLUDF()                            # 通用数据格式基类（预留扩展）
```



## 测试

```bash
# 单元测试
python tests/unit/nogui_querier_test.py

# 集成测试
python tests/integration/test_querier_responser_integration.py

# 循环压力测试
python tests/integration/test_querier_responser_integration_loop.py
```

测试标识符：`app.knotlinksdk.test` / `test_socket` / `test_signal`

## 依赖

- Python 3.8+
- 无外部依赖

## 许可证

MIT License
Copyright (c) 2024-2026 KnotLink Contributors

## 链接

- KnotLink 文档：https://knotlink.cn/
- KnotLink 协议：https://github.com/KnotLink-Protocol
- 问题反馈：[Issues · KnotLink-Protocol/KnotLinkSDK](https://github.com/KnotLink-Protocol/KnotLinkSDK/issues)
