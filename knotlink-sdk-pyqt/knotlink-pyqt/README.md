# KnotLinkClient_PyQt 使用教程

## 1. 安装

在使用 KnotLinkClient_PyQt 之前，需要先安装该库。可以通过以下命令进行安装：

```bash
pip install KnotLinkClient_PyQt
```

**注意**：KnotLinkClient_PyQt 依赖于 PyQt5 库。如果尚未安装 PyQt5，可以通过以下命令安装：

```bash
pip install PyQt5
```

## 2. 更新

如果你已经安装了 KnotLinkClient_PyQt，可以通过以下命令更新到最新版本：

```bash
pip install --upgrade KnotLinkClient_PyQt
```

## 3. 库结构

KnotLinkClient_PyQt 包含四个子模块，分别是：

- **SignalSender**：用于发送信号。
- **SignalSubscriber**：用于订阅信号。
- **OpenSocketQuerier**：用于发起 Socket 查询。
- **OpenSocketResponser**：用于响应 Socket 查询。

这些模块可以通过以下方式导入：

```
pythonfrom KnotLinkClient_PyQt import SignalSender, SignalSubscriber, OpenSocketQuerier, OpenSocketResponser
```

## 4. 快速上手

### 4.1 信号-订阅模式

#### 4.1.1 发送信号

1. **导入库**：

   ```python
   from KnotLinkClient_PyQt import SignalSender
   ```

2. **定义变量**：

   ```python
   signalSender = SignalSender()
   ```

3. **设置 APPID 和 SignalID**：

   ```python
   signalSender.setConfig(appID, signalID)
   ```

   **注意**：`appID` 和 `signalID` 的格式需要符合 KnotLink 的要求。

4. **发送信号**：

   ```python
   signalSender.emitt(data)
   ```

   其中 `data` 是要发送的数据，必须是字符串格式。

#### 4.1.2 订阅信号

1. **导入库**：

   ```python
   from KnotLinkClient_PyQt import SignalSubscriber
   ```

2. **定义变量**：

   ```python
   signalSubscriber = SignalSubscriber(appid, signalid)
   ```

   其中 `appid` 和 `signalid` 是目标信号的 APPID 和 SignalID。

3. **定义接收数据的函数**：

   ```python
   def on_received_data(data: str):
       print(f"接收到字符串数据：{data}")
   ```

4. **连接槽函数**：

   ```python
   signalSubscriber.received_data.connect(on_received_data)
   ```

   当信号到达时，`on_received_data` 函数会被调用。

### 4.1.3 完整测试代码

#### 4.1.3.1 SignalSender 测试代码

```python
from KnotLinkClient_PyQt import SignalSender

if __name__ == "__main__":
    import sys
    from PyQt5.QtWidgets import QApplication

    app = QApplication(sys.argv)

    signalSender = SignalSender()
    signalSender.setConfig("0x00100000", "0x00000010")
    signalSender.emitt("data")

    sys.exit(app.exec_())
```

#### 4.1.3.2 SignalSubscriber 测试代码

```python
from KnotLinkClient_PyQt import SignalSubscriber

if __name__ == "__main__":
    import sys
    from PyQt5.QtWidgets import QApplication

    app = QApplication(sys.argv)

    signalSubscriber = SignalSubscriber("0x00100000", "0x00000010")
    def on_received_data(data: str       ):
 print(f"接收到字符串数据：{data}")
    signalSubscriber.received_data.connect(on_received_data)

    sys.exit(app.exec_())
```

### 4.1.4 测试步骤

1. **运行订阅程序**：先启动 `SignalSubscriber_Test.py`。
2. **发送信号**：再运行 `SignalSender_Test.py`。
3. **观察结果**：如果订阅程序成功接收到信号，控制台会打印接收到的数据。

## 4.2 询问-回复模式



## 4.3 注意事项

- 所有代码必须在 PyQt5 的框架下运行，否则可能会出现类似以下错误：
  ```
  QObject::startTimer: Timers can only be used with threads started with QThread
  ```

## 4.4 图形化测试代码

你可以通过以下链接下载图形化测试代码：

- [下载图形化测试代码](#)

（此处需要提供实际的下载链接，或者说明如何获取代码）

---

## 5. 其他模块（OpenSocketQuerier 和 OpenSocketResponser）

目前教程中尚未包含 **OpenSocketQuerier** 和 **OpenSocketResponser** 的使用方法。如果你需要了解这两个模块的使用，请参考 KnotLink 的官方文档，或者等待后续更新。

---

希望这份教程对你有帮助！如果有任何问题或需要进一步的指导，请随时联系。