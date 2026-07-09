# SignalSender.py
# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from .tcpclient import TcpClient  # 确保 tcpclient.py 在同一目录下
import time
from typing import Optional

class SignalSender:
    def __init__(self, APPID: Optional[str] = None, SignalID: Optional[str] = None) -> None:
        self.appID = APPID
        self.signalID = SignalID
        self.KLsender = TcpClient()
        self.KLsender.connect_to_server("127.0.0.1", 6370)

    def set_config(self, APPID: str, SignalID: str) -> None:
        self.appID = APPID
        self.signalID = SignalID

    def emitt(self, data: str) -> None:
        """发送信号（命名 emitt 而非 emit，避免与 PyQt/PySide 的 emit 关键字冲突）"""
        self._emitt(self.appID, self.signalID, data)

    def _emitt(self, APPID, SignalID, data):
        if isinstance(data, str):
            data = data.encode('utf-8')
        self.__emitt(APPID, SignalID, data)

    def __emitt(self, APPID, SignalID, data):
        s_key = f"{APPID}-{SignalID}&*&"
        s_key_bytes = s_key.encode('utf-8')
        s_data = s_key_bytes + data
        self.KLsender.send_data(s_data)

    def disconnect(self) -> None:
        """断开连接并释放资源"""
        if hasattr(self, 'KLsender') and self.KLsender:
            self.KLsender.disconnect()

    def close(self) -> None:
        """断开连接（disconnect 的别名）"""
        self.disconnect()

    def __enter__(self) -> "SignalSender":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()
        return False

# 示例用法
if __name__ == "__main__":
    signal_sender = SignalSender(APPID="1", SignalID="1")
    signal_sender.emitt("Hello, Server!")
    # 等待一段时间以确保数据发送完成
    time.sleep(10)
    signal_sender.KLsender.disconnect()