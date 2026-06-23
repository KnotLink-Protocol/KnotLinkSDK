/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "opensocketquerier.h"

OpenSocketQuerier::OpenSocketQuerier(QObject *parent) : QObject(parent)
{
    init();
}

// 带参数的构造函数
OpenSocketQuerier::OpenSocketQuerier(QString APPID, QString OpenSocketID, QObject *parent)
    : QObject(parent), appID(APPID), openSocketID(OpenSocketID) // 初始化 appID 和 signalID
{
    init();
}

void OpenSocketQuerier::init()
{
    KLquerier = new TcpClient(this);
    KLquerier->connectToServer("127.0.0.1",6376);
    connect(KLquerier, &TcpClient::receivedData, this, &OpenSocketQuerier::dataRecv);
}

void OpenSocketQuerier::setConfig(QString APPID, QString OpenSocketID){
    appID = APPID;
    openSocketID = OpenSocketID;
}

QString OpenSocketQuerier::query_l(QString data)
{

    QString result;

    QEventLoop loop; // 创建局部事件循环
    QObject::connect(KLquerier, &TcpClient::receivedData, [this,&loop, &result](const QByteArray &data) {
        result = QString::fromUtf8(data); // 保存信号值
        lock = 0;
        loop.quit();   // 退出事件循环
    });

    query(data);
    lock = 1;
    loop.exec();             // 启动事件循环，阻塞直到退出

    return result;
}

void OpenSocketQuerier::query(QByteArray data)
{
    query(appID,openSocketID,data);
}

void OpenSocketQuerier::query(QString data)
{
    query(appID,openSocketID,data.toUtf8());
}

void OpenSocketQuerier::query(QString APPID, QString OpenSocketID,QString data)
{
    query(APPID, OpenSocketID,data.toUtf8());
}

void OpenSocketQuerier::query(QString APPID, QString OpenSocketID,QByteArray data)
{
    QString s_key = APPID + "-" + OpenSocketID;
    s_key += "&*&";
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    // 创建 s_data，将 s_key 和 data 合并
    QByteArray s_data = s_key_bytes; // 初始化 s_data 为 s_key
    s_data.append(data); // 将 data 追加到 s_data 中
    KLquerier->sendData(s_data);
}

void OpenSocketQuerier::dataRecv(const QByteArray &data)
{
    if(lock) return;
    emit dataBack_a(data);
    // 将 QByteArray 转换为 QString
    QString receivedText = QString::fromUtf8(data);
    emit dataBack(receivedText);
}
