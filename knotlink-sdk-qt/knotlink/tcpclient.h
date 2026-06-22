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
