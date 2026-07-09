# tcpclient.py
# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

import socket
import threading
import struct
import time
import logging

logger = logging.getLogger(__name__)

class TcpClient:
    def __init__(self) -> None:
        self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.heart_beat_interval = 180  # 3分钟
        self.heart_beat_timer = None
        self._connected_event = threading.Event()  # 线程安全的连接状态
        self.received_data_callback = None
        self.buffer = b''  # 接收缓冲区，用于粘包处理
        self.lock = threading.Lock()
        self.MAX_MSG_SIZE = 16 * 1024 * 1024  # 16MB

    @property
    def connected(self):
        """线程安全的连接状态"""
        return self._connected_event.is_set()

    @connected.setter
    def connected(self, value):
        if value:
            self._connected_event.set()
        else:
            self._connected_event.clear()

    def connect_to_server(self, ip: str, port: int) -> None:
        self._server_ip = ip
        self._server_port = port
        try:
            self.tcp_socket.connect((ip, port))
            self.connected = True
            logger.info("Connected to %s:%s", ip, port)
            self.start_heart_beat()
            threading.Thread(target=self.receive_data, daemon=True).start()
        except socket.error as e:
            logger.error("Connection to %s:%s failed: %s", ip, port, e)

    def start_heart_beat(self):
        if not self.connected:
            return
        self.heart_beat_timer = threading.Timer(self.heart_beat_interval, self.send_heart_beat)
        self.heart_beat_timer.daemon = True
        self.heart_beat_timer.start()

    # ---------- 发送数据：添加长度前缀 ----------
    def _write_with_length_prefix(self, data: bytes):
        """写入 4 字节大端长度前缀 + 数据"""
        if not self.connected:
            return
        try:
            length = len(data)
            if length > self.MAX_MSG_SIZE:
                raise ValueError(f"消息过长: {length} > {self.MAX_MSG_SIZE}")
            # 构造长度前缀（大端）
            prefix = struct.pack('>I', length)  # >I 表示 unsigned int 大端
            self.tcp_socket.sendall(prefix + data)
        except socket.error as e:
            logger.error("Failed to send data: %s", e)
            self.connected = False

    def send_data(self, data: bytes) -> None:
        """对外发送数据接口，自动加长度前缀"""
        self._write_with_length_prefix(data)

    def send_heart_beat(self):
        """发送心跳（带长度前缀）"""
        if self.connected:
            self._write_with_length_prefix(b"heartbeat")
            logger.debug("Heartbeat sent successfully")
            # 只在仍然连接时重新启动定时器
            self.start_heart_beat()
        else:
            logger.debug("Cannot send heartbeat, disconnected")

    # ---------- 接收数据：缓冲解析 ----------
    def receive_data(self):
        while self.connected:
            try:
                chunk = self.tcp_socket.recv(4096)  # 一次尽量多读
                if not chunk:
                    logger.warning("Connection lost")
                    self.connected = False
                    break
                with self.lock:
                    self.buffer += chunk
                self._process_buffer()
            except socket.error as e:
                logger.error("Failed to receive data: %s", e)
                self.connected = False
                break

    def _process_buffer(self):
        """从缓冲区中提取完整消息并处理"""
        while True:
            with self.lock:
                if len(self.buffer) < 4:
                    break  # 长度字段未完整
                # 读取长度（大端）
                length = struct.unpack('>I', self.buffer[:4])[0]
                if length == 0 or length > self.MAX_MSG_SIZE:
                    logger.error("Invalid message length: %d, disconnecting", length)
                    self.connected = False
                    self.tcp_socket.close()
                    return
                if len(self.buffer) < 4 + length:
                    break  # 消息体未完整
                # 提取消息体
                msg = self.buffer[4:4+length]
                self.buffer = self.buffer[4+length:]  # 移除已处理的数据
            # 处理消息（在锁外，避免阻塞）
            if msg == b"heartbeat_response":
                logger.debug("Heartbeat response received, ignoring")
            else:
                logger.debug("Received data: %s", msg)
                self.handle_received_data(msg)
            # 继续循环处理剩余缓冲区

    def handle_received_data(self, data: bytes) -> None:
        """处理接收到的完整消息（由子类或外部回调处理）"""
        if self.received_data_callback:
            self.received_data_callback(data)

    def disconnect(self) -> None:
        # 先标记断开，防止 send_heart_beat 重新创建定时器
        self.connected = False
        # 取消心跳定时器
        if self.heart_beat_timer:
            self.heart_beat_timer.cancel()
            self.heart_beat_timer = None
        # 安全关闭 socket
        try:
            self.tcp_socket.close()
        except socket.error:
            pass
        logger.info("Disconnected")


# 示例用法（与之前一致）
if __name__ == "__main__":
    client = TcpClient()
    client.connect_to_server("127.0.0.1", 6370)
    time.sleep(1)
    client.send_data(b"Hello, Server!")
    time.sleep(60)
    client.disconnect()