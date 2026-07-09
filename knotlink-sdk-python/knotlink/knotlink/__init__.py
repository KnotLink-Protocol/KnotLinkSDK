# KnotLink SDK - Python
# Copyright (c) 2024-2026 KnotLink Contributors
# SPDX-License-Identifier: MIT

# knotlink/__init__.py
import logging

from .kludf import KLUDF, KLKVMap
from .OpenSocketQuerier import OpenSocketQuerier
from .OpenSocketResponser import OpenSocketResponser
from .SignalSender import SignalSender
from .SignalSubscriber import SignalSubscriber
from .tcpclient import TcpClient

__all__ = [
    "KLUDF",
    "KLKVMap",
    "OpenSocketQuerier",
    "OpenSocketResponser",
    "SignalSender",
    "SignalSubscriber",
    "TcpClient",
    "enable_logging",
]


def enable_logging(level: int = logging.DEBUG) -> None:
    """一键开启 SDK 日志输出到控制台。
    :param level: 日志级别，默认 DEBUG
    """
    logging.basicConfig(
        level=level,
        format="[%(asctime)s] %(name)s %(levelname)s: %(message)s",
        datefmt="%H:%M:%S",
    )