/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tcpclient.h"
#include <QDebug>
#include <QDataStream>
#include <QtEndian>

Q_LOGGING_CATEGORY(knotlinkTcp, "knotlink.tcp")

const quint32 MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB，与服务器一致

TcpClient::TcpClient(QObject *parent) : QObject(parent) {
	tcpSocket = new QTcpSocket(this);
	heartBeatTimer = new QTimer(this);
	heartBeatTimer->setInterval(180000);  // 3分钟

	// 信号在构造函数中连接，避免 connectToServer 后错过事件
	connect(heartBeatTimer, &QTimer::timeout, this, &TcpClient::sendHeartbeat);
	connect(tcpSocket, &QTcpSocket::connected, this, &TcpClient::socketConnected);
	connect(tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::socketDisconnected);
	connect(tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::readData);
}

// Qt parent 机制自动管理子对象生命周期，无需手动 delete

void TcpClient::connectToServer(const QString &ip, uint16_t port) {
	tcpSocket->connectToHost(QHostAddress(ip), port);

	if (!tcpSocket->waitForConnected(3000)) {
		qCWarning(knotlinkTcp) << "Connection failed:" << tcpSocket->errorString();
		return;
	}

	heartBeatTimer->start();
}

void TcpClient::socketConnected() {
	sendHeartbeat();
	emit connected();
}

void TcpClient::socketDisconnected() {
	heartBeatTimer->stop();
	emit disconnected();
}

// ------------------------------------------------------------
// 写入数据：添加 4 字节大端长度前缀
// ------------------------------------------------------------
void TcpClient::writeWithLengthPrefix(const QByteArray &data) {
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_0);
	out << (quint32)data.size();   // 写入长度（4 字节，大端）
	block.append(data);            // 追加消息体
	tcpSocket->write(block);
}

// 对外发送数据（自动加长度前缀）
void TcpClient::sendData(const QByteArray &data) {
	if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
		writeWithLengthPrefix(data);
	} else {
		qCWarning(knotlinkTcp) << "Cannot send data, disconnected";
	}
}

// ------------------------------------------------------------
// 接收数据：追加到缓冲区，然后解析完整消息
// ------------------------------------------------------------
void TcpClient::readData() {
	QByteArray newData = tcpSocket->readAll();
	if (newData.isEmpty()) return;
	buffer.append(newData);
	processBuffer();
}

// ------------------------------------------------------------
// 解析缓冲区：根据长度前缀提取完整消息
// ------------------------------------------------------------
void TcpClient::processBuffer() {
	while (true) {
		if (buffer.size() < 4) break; // 长度字段未完整
		
		// 读取长度（大端序）
		quint32 len = qFromBigEndian<quint32>((const uchar*)buffer.constData());
		
		if (len == 0 || len > MAX_MSG_SIZE) {
			qCCritical(knotlinkTcp) << "Invalid message length:" << len << ", disconnecting";
			tcpSocket->disconnectFromHost();
			return;
		}
		
		if (buffer.size() < (int)(len + 4)) break; // 消息体未完整
		
		// 提取消息体（跳过长度字段）
		QByteArray msg = buffer.mid(4, len);
		buffer.remove(0, len + 4);
		
		// 处理消息：忽略心跳响应，其他发出信号
		if (msg == "heartbeat_response") {
			qCDebug(knotlinkTcp) << "Heartbeat response received, ignoring";
		} else {
			emit receivedData(msg);
		}
		// 继续循环，可能还有更多完整消息
	}
}

// ------------------------------------------------------------
// 发送心跳：使用带长度前缀的格式
// ------------------------------------------------------------
void TcpClient::sendHeartbeat() {
	if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
		writeWithLengthPrefix("heartbeat");
		qCDebug(knotlinkTcp) << "Heartbeat sent";
	} else {
		qCDebug(knotlinkTcp) << "Cannot send heartbeat, disconnected";
	}
}

// 错误处理（可选）
void TcpClient::handleError(QAbstractSocket::SocketError socketError) {
	qCWarning(knotlinkTcp) << "Socket error:" << socketError << "-" << tcpSocket->errorString();
}
