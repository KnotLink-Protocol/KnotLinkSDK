# KnotLink SDK

KnotLink 是一个轻量级、语义化的跨语言软件互联协议。本仓库提供 KnotLink 协议的多语言 SDK 实现，涵盖 Python、C++、JavaScript、Lua、Rust 等主流编程语言，同时提供 PyQt/Qt 的 GUI 绑定以及 Blockly 图形化编程支持。

通过 KnotLink SDK，开发者可以快速为现有软件接入 KnotLink 互联生态，实现应用间的功能发现与调用。

## 语言支持概览

| 语言                 | SDK 目录               | 适用场景                                               |
| :------------------- | :--------------------- | :----------------------------------------------------- |
| Python               | `knotlink-sdk-python`  | 服务端脚本、AI 集成、桌面应用、自动化工具              |
| PyQt                 | `knotlink-sdk-pyqt`    | Qt/PyQt 桌面应用、Python GUI 程序                      |
| Qt (C++)             | `knotlink-sdk-qt`      | Qt/C++ 桌面应用、跨平台 GUI 程序                       |
| C++                  | `knotlink-sdk-cpp`     | 高性能桌面应用、游戏模组、系统级软件                   |
| JavaScript / Node.js | `knotlink-sdk-js`      | Web 应用、Electron、Tauri 后端、浏览器扩展             |
| Java                 | `knotlink-sdk-java`    | Java 桌面应用、Minecraft 模组 (Forge/Fabric)           |
| C#                   | `knotlink-sdk-cs`      | Windows 桌面应用 (WinForms/WPF)、Unity 游戏、.NET 生态 |
| Dart / Flutter       | `knotlink-sdk-dart`    | 跨平台移动应用 (iOS/Android)、Flutter 桌面应用         |
| Lua                  | `knotlink-sdk-lua`     | 游戏模组 (如 Minecraft 插件)、嵌入式脚本、OpenResty    |
| Rust                 | `knotlink-sdk-rust`    | 高性能系统编程、安全敏感应用、WebAssembly              |
| Blockly              | `knotlink-sdk-blockly` | 图形化编程教育、低代码平台、儿童编程工具               |

## 目录结构

```text
knotlink-sdk-<lang>/
├── examples/               # 示例代码
├── knotlink/               # 核心库源码
├── tests/
│   ├── unit/               # 单元测试
│   │   ├── gui_*.py        # GUI 测试工具
│   │   └── nogui_*.py      # 命令行测试工具
│   └── integration/        # 集成测试
│       ├── test_*_integration.py       # 单次发送测试
│       └── test_*_integration_loop.py  # 循环发送测试
├── LICENSE
└── README.md
```

## 下载 SDK

SDK均存储在开源仓库https://github.com/KnotLink-Protocol/KnotLinkSDK，可下载zip或pull。

## 安装库

各语言库文件的安装方式如下：

### Python

```bash
pip install knotlink
```

### Node.js / JavaScript（未上线安装）

```bash
npm install knotlink
```

### Lua（未上线安装）

使用 LuaRocks 安装：

```bash
luarocks install knotlink
```

### C++（未上线安装）

使用 CMake 集成：

```cmake
add_subdirectory(knotlink-sdk-cpp)
target_link_libraries(your_target knotlink)
```

### Rust（未上线安装）

在 `Cargo.toml` 中添加：

```toml
[dependencies]
knotlink = { path = "knotlink" }
```

### PyQt / Qt（未上线安装）

Python PyQt 版本：

```bash
pip install knotlink-pyqt
```

C++ Qt 版本：使用 CMake 集成，需确保 Qt 已安装。

### Blockly

将 `knotlink` 目录复制到项目中的 `blockly/generators/` 或通过 npm 安装（待发布）。

## 快速开始

以下以 Python 为例，其他语言 SDK 的用法类似。

### 询问（询问者模式）

```python
from knotlink import OpenSocketQuerier

querier = OpenSocketQuerier("app.knotlinksdk.test", "test_socket")
result = querier.query("Hello, Responser!")
print(result)  # Echo: Hello, Responser!
```

### 回复（回复者模式）

```python
from knotlink import OpenSocketResponser

def on_request(data):
    print(f"Received: {data}")
    return f"Echo: {data}"

responser = OpenSocketResponser("app.knotlinksdk.test", "test_socket")
responser.set_RecvFunc(on_request)
```

### 发送信号（发信者模式）

```python
from knotlink import SignalSender

sender = SignalSender()
sender.set_config("app.knotlinksdk.test", "test_signal")
sender.emitt("Hello from Sender!")
```

### 订阅信号（订阅者模式）

```python
from knotlink import SignalSubscriber

def on_received(data):
    print(f"Received: {data}")

subscriber = SignalSubscriber("app.knotlinksdk.test", "test_signal")
subscriber.set_RecvFunc(on_received)
# 保持监听
```

## 测试

### 单元测试

各语言 SDK 均提供单元测试工具，位于 `tests/unit/` 目录下。

#### GUI 测试工具

- `gui_opensocket_querier_test.py` — 询问者图形界面
- `gui_opensocket_responser_test.py` — 回复者图形界面
- `gui_signal_sender_test.py` — 发信者图形界面
- `gui_signal_subscriber_test.py` — 订阅者图形界面

运行方式（以 Python 为例）：

```bash
python tests/unit/gui_opensocket_querier_test.py
```

#### 非GUI测试工具

- `nogui_opensocket_querier_test.py` — 询问者测试
- `nogui_opensocket_responser_test.py` —  回复者测试
- `nogui_signal_sender_test.py` — 发信者测试
- `nogui_signal_subscriber_test.py` — 订阅者测试

运行方式（IDE也可）：

```bash
python tests/unit/nogui_opensocket_querier_test.py
```

### 集成测试

集成测试位于 `tests/integration/` 目录，用于验证完整通信链路。

- `test_request_response_integration.py` — 询问-回复模式单次测试
- `test_request_response_integration_loop.py` — 询问-回复模式循环压力测试
- `test_signal_subscribe_integration.py` — 信号-订阅模式单次测试
- `test_signal_subscribe_integration_loop.py` — 信号-订阅模式循环压力测试

运行方式（IDE也可）：

```bash
# 单次测试
python tests/integration/test_request_response_integration.py

# 循环压力测试（按 Ctrl+C 停止）
python tests/integration/test_request_response_integration_loop.py
```

### 测试默认配置

所有测试使用统一的默认标识符：

| 字段     | 值                     |
| :------- | :--------------------- |
| APPID    | `app.knotlinksdk.test` |
| SocketID | `test_socket`          |
| SignalID | `test_signal`          |

GUI 测试工具支持手动修改上述字段；非GUI工具可通过修改源码参数进行配置。

## 贡献指南

欢迎为 KnotLink SDK 贡献代码。请遵循以下流程：

1. Fork 本仓库。
2. 创建功能分支：`git checkout -b feature/your-feature`。
3. 提交变更：`git commit -m "Add your feature"`。
4. 推送分支：`git push origin feature/your-feature`。
5. 提交 Pull Request。

各语言 SDK 的代码风格应遵循该语言的社区规范。提交前请确保测试通过。

## 许可证

KnotLink SDK 采用 MIT 许可证，详见 `LICENSE` 文件。

## 相关链接

- KnotLink 文档：[KnotLink文档](https://knotlink.cn)
- KnotLink 服务端：[KnotLink-Protocol/KnotLinkService](https://github.com/KnotLink-Protocol/KnotLinkService)
- 问题反馈：[Issues · KnotLink-Protocol/KnotLinkSDK](https://github.com/KnotLink-Protocol/KnotLinkSDK/issues)

## 各语言 SDK 开发状态

| 语言                 | 状态   | 示例 |
| :------------------- | :----- | :--- |
| Python               | 稳定   | -    |
| Node.js / JavaScript | 开发中 | -    |
| C++                  | 开发中 | -    |
| PyQt                 | 稳定   | -    |
| Qt (C++)             | 稳定   | -    |
| Java                 | 开发中 | -    |
| C#                   | 开发中 | -    |
| Dart / Flutter       | 规划中 | -    |
| Lua                  | 规划中 | -    |
| Rust                 | 稳定   | -    |
| Blockly              | 规划中 | -    |