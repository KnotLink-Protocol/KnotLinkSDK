# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from PyQt5.QtCore import QObject, pyqtSignal, QTimer
from PyQt5.QtNetwork import QTcpSocket, QHostAddress, QAbstractSocket
import struct

class TcpClient(QObject):
    connected = pyqtSignal()
    disconnected = pyqtSignal()
    receivedData = pyqtSignal(bytes)  # 发出完整消息（不含长度前缀）

    def __init__(self, parent=None):
        super().__init__(parent)
        self.tcpSocket = QTcpSocket(self)
        self.heartBeatTimer = QTimer(self)
        self.heartBeatTimer.setInterval(180000)  # 3分钟
        self.buffer = bytearray()                 # 接收缓冲区
        self.MAX_MSG_SIZE = 16 * 1024 * 1024      # 16MB

        self.tcpSocket.connected.connect(self.socketConnected)
        self.tcpSocket.disconnected.connect(self.socketDisconnected)
        self.tcpSocket.readyRead.connect(self.readData)
        self.tcpSocket.errorOccurred.connect(self.handleError)
        self.heartBeatTimer.timeout.connect(self.sendHeartbeat)

    # ---------- 连接 ----------
    def connectToServer(self, ip, port):
        self.tcpSocket.connectToHost(QHostAddress(ip), port)
        if not self.tcpSocket.waitForConnected(3000):
            print("连接失败：", self.tcpSocket.errorString())
            return False
        self.heartBeatTimer.start()
        return True

    def socketConnected(self):
        self.connected.emit()

    def socketDisconnected(self):
        self.disconnected.emit()
        self.heartBeatTimer.stop()

    # ---------- 发送数据（自动加长度前缀） ----------
    def _writeWithLengthPrefix(self, data: bytes):
        if self.tcpSocket.state() != QAbstractSocket.ConnectedState:
            print("无法发送数据，连接已断开。")
            return False
        if len(data) > self.MAX_MSG_SIZE:
            print(f"消息过长: {len(data)} > {self.MAX_MSG_SIZE}")
            return False
        # 构造 4 字节大端长度前缀
        length_bytes = struct.pack('>I', len(data))
        # 发送前缀 + 数据
        self.tcpSocket.write(length_bytes + data)
        return True

    def sendData(self, data: bytes):
        self._writeWithLengthPrefix(data)

    # ---------- 心跳 ----------
    def sendHeartbeat(self):
        if self.tcpSocket.state() == QAbstractSocket.ConnectedState:
            if self._writeWithLengthPrefix(b"heartbeat"):
                print("发送心跳包成功")
        else:
            print("无法发送心跳包，连接已断开。")
            self.heartBeatTimer.stop()

    # ---------- 接收数据 ----------
    def readData(self):
        # 读取所有可用数据
        new_data = self.tcpSocket.readAll().data()
        if not new_data:
            return
        self.buffer.extend(new_data)
        self._processBuffer()

    def _processBuffer(self):
        """从缓冲区解析完整消息"""
        while True:
            if len(self.buffer) < 4:
                break  # 长度字段未完整

            # 读取长度（大端）
            length = struct.unpack('>I', self.buffer[:4])[0]
            if length == 0 or length > self.MAX_MSG_SIZE:
                print(f"无效消息长度: {length}，断开连接")
                self.tcpSocket.disconnectFromHost()
                return

            if len(self.buffer) < 4 + length:
                break  # 消息体未完整

            # 提取消息体
            msg = bytes(self.buffer[4:4+length])
            # 移除已处理的数据
            del self.buffer[:4+length]

            # 处理消息：忽略心跳响应
            if msg == b"heartbeat_response":
                print("收到心跳响应，忽略")
            else:
                print("收到数据：", msg)
                self.receivedData.emit(msg)
            # 继续循环处理剩余缓冲区

    # ---------- 错误处理 ----------
    def handleError(self, socketError):
        print("错误：", socketError, "-", self.tcpSocket.errorString())

    # ---------- 断开 ----------
    def disconnect(self):
        self.tcpSocket.disconnectFromHost()
        self.buffer.clear()