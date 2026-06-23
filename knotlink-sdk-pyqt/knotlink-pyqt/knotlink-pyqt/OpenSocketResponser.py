# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from PyQt5.QtCore import QObject, pyqtSignal
from .tcpclient import TcpClient  

class OpenSocketResponser(QObject):
    receivedData = pyqtSignal(str, str)
    receivedData_a = pyqtSignal(bytes, str)

    def __init__(self, APPID: str, OpenSocketID: str, parent=None):
        super().__init__(parent)
        self.appID = APPID
        self.openSocketID = OpenSocketID
        self.init()

    def init(self):
        self.KLresponser = TcpClient(self)
        self.KLresponser.connectToServer("127.0.0.1", 6378)
        self.KLresponser.receivedData.connect(self.dataRecv)
        s_key = f"{self.appID}-{self.openSocketID}"
        s_key_bytes = s_key.encode("utf-8")  # 转换为 UTF-8 编码的 QByteArray
        self.KLresponser.sendData(s_key_bytes)

    def dataRecv(self, data: bytes):
        s_data = data.decode("utf-8")

        delimiter = "&*&"  # 分隔符
        parts = s_data.split(delimiter)  # 按分隔符分割字符串

        if len(parts) != 2:  # 确保分割后有两部分
            print(f"Invalid data format. Expected two parts separated by {delimiter}")
            return

        key = parts[0]  # 前一部分作为 key
        t_data = parts[1]  # 后一部分作为 t_data
        print(f"Key: {key}")
        print(f"t_data: {t_data}")

        self.receivedData_a.emit(t_data.encode("utf-8"), key)
        self.receivedData.emit(t_data, key)

    def sendBack(self, data: str, questionID: str):
        self.sendBack_a(data.encode("utf-8"), questionID)

    def sendBack_a(self, data: bytes, questionID: str):
        data_r = f"{questionID}&*&{data.decode('utf-8')}"
        self.KLresponser.sendData(data_r.encode("utf-8"))
