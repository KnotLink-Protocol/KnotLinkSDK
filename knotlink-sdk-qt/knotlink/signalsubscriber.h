#ifndef SIGNALSUBSRIBER_H
#define SIGNALSUBSRIBER_H

#include <QObject>
#include "tcpclient.h"

class SignalSubscriber : public QObject
{
    Q_OBJECT
public:
    explicit SignalSubscriber(QString APPID, QString SignalID,QObject *parent = nullptr);
    void subscribe(QString APPID, QString SignalID);

signals:
    void receivedData(const QString &data);
    void receivedData_a(const QByteArray &data);

public slots:
    void dataRecv(const QByteArray &data);

private:
    TcpClient* KLsubscriber;
    QString appID;
    QString signalID;
    void init();
};

#endif // SIGNALSUBSRIBER_H
