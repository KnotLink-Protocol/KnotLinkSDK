#ifndef SIGNALSENDER_H
#define SIGNALSENDER_H

#include <QObject>
#include "tcpclient.h"

class SignalSender : public QObject
{
    Q_OBJECT
public:
    explicit SignalSender(QObject *parent = nullptr);
    explicit SignalSender(QString APPID, QString SignalID ,QObject *parent = nullptr);// initialize and chang config
    void setConfig(QString APPID, QString SignalID); // chang config
    void emitt(QString data); // emit data
    void emitt(QByteArray data); // emit data
    void emitt(QString APPID, QString SignalID,QString data); // temp change config
    void emitt(QString APPID, QString SignalID,QByteArray data); // temp change config

signals:

public slots:

private:
    TcpClient* KLsender;
    QString appID;
    QString signalID;
    void init();
};

#endif // SIGNALSENDER_H
