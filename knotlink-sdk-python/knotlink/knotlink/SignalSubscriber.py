# SignalSubscriber.py
# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from .tcpclient import TcpClient  # 确保 tcpclient.py 在同一目录下

class SignalSubscriber:
    def __init__(self, APPID, SignalID):
        self.appID = APPID
        self.signalID = SignalID
        self.callback = None  # 用于存储回调函数
        self.init()

    def init(self):
        self.KLsubscriber = TcpClient()
        self.KLsubscriber.connect_to_server("127.0.0.1", 6372)
        print("OKK")
        self.KLsubscriber.received_data_callback = self.data_recv  # 设置数据接收回调
        s_key = f"{self.appID}-{self.signalID}"
        s_key_bytes = s_key.encode('utf-8')
        self.KLsubscriber.send_data(s_key_bytes)

    def subscribe(self, APPID, SignalID):
        self.appID = APPID
        self.signalID = SignalID
        s_key = f"{self.appID}-{self.signalID}"
        s_key_bytes = s_key.encode('utf-8')
        self.KLsubscriber.send_data(s_key_bytes)

    def data_recv(self, data):
        # 将接收到的字节数据解码为字符串
        try:
            received_text = data.decode('utf-8', errors='ignore')  # 解码并忽略错误
        except Exception as e:
            print(f"解码失败: {e}")
            received_text = ""
        
        # 调用用户设置的回调函数（如果已设置）
        if self.callback:
            self.callback(received_text)

    def set_RecvFunc(self, callback):
        """
        设置接收到数据后的回调函数。
        :param callback: 回调函数，接收一个 str 类型的参数。
        """
        self.callback = callback

    def disconnect(self):
        """断开连接并释放资源"""
        if hasattr(self, 'KLsubscriber') and self.KLsubscriber:
            self.KLsubscriber.disconnect()

    def close(self):
        """断开连接（disconnect 的别名）"""
        self.disconnect()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()
        return False

# 示例用法
if __name__ == "__main__":
    def my_callback(data: str):
        print(f"Callback received data: {data}")

    subscriber = SignalSubscriber(APPID="1", SignalID="1")
    subscriber.set_RecvFunc(my_callback)  # 设置回调函数
    # 等待一段时间以接收数据
    import time
    time.sleep(60)
    subscriber.KLsubscriber.disconnect()