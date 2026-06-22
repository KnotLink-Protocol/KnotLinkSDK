#ifndef OPENSOCKETRESPONSER_H
#define OPENSOCKETRESPONSER_H

#include <QObject>
#include "tcpclient.h"

class OpenSocketResponser : public QObject
{
    Q_OBJECT
public:
    explicit OpenSocketResponser(QString APPID, QString OpenSocketID,QObject *parent = nullptr);
    void sendBack(const QString &data,QString questionID);
    void sendBack(const QByteArray &data,QString questionID);

signals:
    void receivedData(const QString &data,QString questionID);
    void receivedData_a(const QByteArray &data,QString questionID);

public slots:
    void dataRecv(const QByteArray &data);

private:
    TcpClient* KLresponser;
    QString appID;
    QString openSocketID;
    void init();
};

#endif // OPENSOCKETRESPONSER_H
