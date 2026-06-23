# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from PyQt5.QtCore import QObject
from .tcpclient import TcpClient

class SignalSender(QObject):
    def __init__(self, APPID=None, SignalID=None, parent=None):
        super().__init__(parent)
        self.appID = APPID
        self.signalID = SignalID
        self.KLsender = TcpClient(self)
        self.KLsender.connectToServer("127.0.0.1", 6370)

    def setConfig(self, APPID, SignalID):
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
        self.KLsender.sendData(s_data)