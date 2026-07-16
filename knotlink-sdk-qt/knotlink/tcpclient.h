/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QByteArray>

class TcpClient : public QObject {
	Q_OBJECT
public:
	static const QByteArray MAGIC;       // 协议魔数：KK + 版本号 2.0
	static constexpr int MAGIC_LEN = 4;

	explicit TcpClient(QObject *parent = nullptr);
	~TcpClient();

	void connectToServer(const QString &ip, uint16_t port);
	void sendData(const QByteArray &data);  // 自动添加长度前缀

	signals:
	void connected();
	void disconnected();
	void receivedData(const QByteArray &data);  // 完整消息（不含长度前缀）

private slots:
	void socketConnected();
	void socketDisconnected();
	void readData();
	void handleError(QAbstractSocket::SocketError socketError);
	void sendHeartbeat();

private:
	QTcpSocket *tcpSocket;
	QTimer *heartBeatTimer;
	QByteArray buffer;          // 接收缓冲区，用于处理粘包
	void processBuffer();       // 解析缓冲区中的完整消息
	void writeWithLengthPrefix(const QByteArray &data); // 写入长度前缀+数据
};

#endif // TCPCLIENT_H
