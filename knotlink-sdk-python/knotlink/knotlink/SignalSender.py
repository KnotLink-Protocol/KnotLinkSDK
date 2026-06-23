# SignalSender.py
# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from .tcpclient import TcpClient  # 确保 tcpclient.py 在同一目录下
import time

class SignalSender:
    def __init__(self, APPID=None, SignalID=None):
        self.appID = APPID
        self.signalID = SignalID
        self.KLsender = TcpClient()
        self.KLsender.connect_to_server("127.0.0.1", 6370)

    def set_config(self, APPID, SignalID):
        self.appID = APPID
        self.signalID = SignalID

    def emitt(self, data):
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

# 示例用法
if __name__ == "__main__":
    signal_sender = SignalSender(APPID="1", SignalID="1")
    signal_sender.emitt("Hello, Server!")
    # 等待一段时间以确保数据发送完成
    time.sleep(10)
    signal_sender.KLsender.disconnect()