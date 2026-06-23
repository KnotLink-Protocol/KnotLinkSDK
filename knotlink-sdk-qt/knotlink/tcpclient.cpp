/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "tcpclient.h"
#include <QDebug>
#include <QDataStream>
#include <QtEndian>   // 提供 qFromBigEndian, qToBigEndian

const quint32 MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB，与服务器一致

TcpClient::TcpClient(QObject *parent) : QObject(parent) {
	tcpSocket = new QTcpSocket(this);
	heartBeatTimer = new QTimer(this);
	heartBeatTimer->setInterval(180000);  // 3分钟
}

TcpClient::~TcpClient() {
	delete tcpSocket;
	if (heartBeatTimer != nullptr) {
		delete heartBeatTimer;
	}
}

// 连接服务器
void TcpClient::connectToServer(const QString &ip, uint16_t port) {
	tcpSocket->connectToHost(QHostAddress(ip), port);
	
	if (!tcpSocket->waitForConnected(3000)) {
		qDebug() << "连接失败：" << tcpSocket->errorString();
		return;
	}
	
	// 连接定时器信号
	connect(heartBeatTimer, &QTimer::timeout, this, &TcpClient::sendHeartbeat);
	heartBeatTimer->start();
	
	connect(tcpSocket, &QTcpSocket::connected, this, &TcpClient::socketConnected);
	connect(tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::socketDisconnected);
	connect(tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::readData);
	// 可选错误处理
}

void TcpClient::socketConnected() {
	// 立即发送一次心跳（可选）
	sendHeartbeat();
	emit connected();
}

void TcpClient::socketDisconnected() {
	emit disconnected();
	heartBeatTimer->stop();
	delete heartBeatTimer;
	heartBeatTimer = nullptr;
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
		qDebug() << "无法发送数据，连接已断开。";
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
			qDebug() << "无效的消息长度:" << len << "，断开连接";
			tcpSocket->disconnectFromHost();
			return;
		}
		
		if (buffer.size() < (int)(len + 4)) break; // 消息体未完整
		
		// 提取消息体（跳过长度字段）
		QByteArray msg = buffer.mid(4, len);
		buffer.remove(0, len + 4);
		
		// 处理消息：忽略心跳响应，其他发出信号
		if (msg == "heartbeat_response") {
			qDebug() << "收到心跳响应，忽略";
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
		qDebug() << "发送心跳包";
	} else {
		qDebug() << "无法发送心跳包，连接已断开。";
	}
}

// 错误处理（可选）
void TcpClient::handleError(QAbstractSocket::SocketError socketError) {
	qDebug() << "错误：" << socketError << " - " << tcpSocket->errorString();
}
