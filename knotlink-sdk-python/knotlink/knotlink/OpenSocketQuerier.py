# OpenSocketQuerier.py
# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

from .tcpclient import TcpClient  # 确保 tcpclient.py 在同一目录下
import threading
from typing import Callable, Optional

class OpenSocketQuerier:
    def __init__(self, APPID: Optional[str] = None, OpenSocketID: Optional[str] = None) -> None:
        self.appID = APPID
        self.openSocketID = OpenSocketID
        self.callback = None  # 用户设置的回调函数
        self.query_result = None  # 用于存储同步查询的结果
        self.query_event = threading.Event()  # 用于同步查询的事件
        self.is_querying = False  # 标记是否正在进行同步查询
        self._state_lock = threading.Lock()  # 保护共享状态的锁
        self._timeout_occurred = False  # 超时标志，防止过期结果覆盖
        self.init()

    def init(self):
        self.KLquerier = TcpClient()
        self.KLquerier.connect_to_server("127.0.0.1", 6376)
        self.KLquerier.received_data_callback = self._internal_data_recv  # 设置内部数据接收回调

    def set_config(self, APPID: str, OpenSocketID: str) -> None:
        self.appID = APPID
        self.openSocketID = OpenSocketID

    def query(self, data: str, timeout: float = None) -> str:
        """同步查询，等待服务端响应。
        :param data: 发送的查询数据
        :param timeout: 超时时间（秒），None 表示无限等待
        :return: 服务端响应的字符串
        :raises TimeoutError: 超时未收到响应
        """
        with self._state_lock:
            self.query_result = None  # 重置查询结果
            self.query_event.clear()  # 重置事件
            self.is_querying = True  # 标记开始同步查询
            self._timeout_occurred = False
        self._query(self.appID, self.openSocketID, data.encode("utf-8"))
        success = self.query_event.wait(timeout=timeout)  # 等待事件被触发
        with self._state_lock:
            self.is_querying = False  # 标记结束同步查询
            if not success:
                self._timeout_occurred = True
                raise TimeoutError(f"query timed out after {timeout}s")
            return self.query_result

    def query_async(self, data: str) -> None:
        self._query(self.appID, self.openSocketID, data.encode("utf-8"))

    def _query(self, APPID: str, OpenSocketID: str, data: bytes):
        s_key = f"{APPID}-{OpenSocketID}&*&"
        s_key_bytes = s_key.encode("utf-8")
        s_data = s_key_bytes + data
        self.KLquerier.send_data(s_data)

    def _internal_data_recv(self, data: bytes):
        received_text = data.decode("utf-8", errors="replace")  # 解码为字符串
        with self._state_lock:
            if self.is_querying and not self._timeout_occurred:
                # 正在进行同步查询，保存结果并触发事件
                self.query_result = received_text
                self.query_event.set()
                return
        # 不是同步查询（或已超时），调用用户设置的回调函数
        if self.callback:
            self.callback(received_text)

    def set_RecvFunc(self, callback: Callable[[str], None]) -> None:
        """
        设置接收到数据后的回调函数。
        :param callback: 回调函数，接收一个 str 类型的参数。
        """
        self.callback = callback

    def disconnect(self) -> None:
        """断开连接并释放资源"""
        if hasattr(self, 'KLquerier') and self.KLquerier:
            self.KLquerier.disconnect()
        # 唤醒可能在 query() 中等待的线程
        with self._state_lock:
            self._timeout_occurred = True
            if self.query_event:
                self.query_event.set()

    def close(self) -> None:
        """断开连接（disconnect 的别名）"""
        self.disconnect()

    def __enter__(self) -> "OpenSocketQuerier":
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()
        return False

# 示例用法
if __name__ == "__main__":
    def my_callback(data: str):
        print(f"Callback received data: {data}")

    querier = OpenSocketQuerier(APPID="0x00000001", OpenSocketID="0x00000011")
    querier.set_RecvFunc(my_callback)  # 设置回调函数

    # 同步查询示例
    result = querier.query("Hello, Server!")
    print(f"Synchronous query result: {result}")

    import time
    # 异步查询示例
    querier.query_async("Another message")
    time.sleep(60)  # 等待一段时间以接收数据

    querier.KLquerier.disconnect()