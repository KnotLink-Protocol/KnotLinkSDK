# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot
from .tcpclient import TcpClient

class SignalSubscriber(QObject):
    received_data = pyqtSignal(str)
    received_data_a = pyqtSignal(bytes)

    def __init__(self, APPID, SignalID, parent=None):
        super().__init__(parent)
        self.appID = APPID
        self.signalID = SignalID
        self.init()

    def init(self):
        self.KLsubscriber = TcpClient(self)
        self.KLsubscriber.connectToServer("127.0.0.1", 6372)
        print("OKK")
        self.KLsubscriber.receivedData.connect(self.dataRecv)
        s_key = f"{self.appID}-{self.signalID}"
        s_key_bytes = s_key.encode('utf-8')
        self.KLsubscriber.sendData(s_key_bytes)

    def subscribe(self, APPID, SignalID):
        self.appID = APPID
        self.signalID = SignalID
        s_key = f"{self.appID}-{self.signalID}"
        s_key_bytes = s_key.encode('utf-8')
        self.KLsubscriber.sendData(s_key_bytes)

    @pyqtSlot(bytes)
    def dataRecv(self, data):
        self.received_data_a.emit(data)
        received_text = data.decode('utf-8')
        self.received_data.emit(received_text)