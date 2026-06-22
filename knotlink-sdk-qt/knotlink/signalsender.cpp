#include "signalsender.h"

SignalSender::SignalSender(QObject *parent) : QObject(parent)
{
    init();
}

// 带参数的构造函数
SignalSender::SignalSender(QString APPID, QString SignalID, QObject *parent)
    : QObject(parent), appID(APPID), signalID(SignalID) // 初始化 appID 和 signalID
{
    init();
}

void SignalSender::init()
{
    KLsender = new TcpClient(this);
    KLsender->connectToServer("127.0.0.1",6370);
}

void SignalSender::setConfig(QString APPID, QString SignalID){
    appID = APPID;
    signalID = SignalID;
}

void SignalSender::emitt(QByteArray data)
{
    emitt(appID,signalID,data);
}

void SignalSender::emitt(QString data)
{
    emitt(appID,signalID,data.toUtf8());
}

void SignalSender::emitt(QString APPID, QString SignalID,QString data)
{
    emitt(APPID, SignalID,data.toUtf8());
}

void SignalSender::emitt(QString APPID, QString SignalID,QByteArray data)
{
    QString s_key = APPID + "-" + SignalID;
    s_key += "&*&";
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    // 创建 s_data，将 s_key 和 data 合并
    QByteArray s_data = s_key_bytes; // 初始化 s_data 为 s_key
    s_data.append(data); // 将 data 追加到 s_data 中
    KLsender->sendData(s_data);
}
