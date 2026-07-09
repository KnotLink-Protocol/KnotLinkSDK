/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "signalsubscriber.h"
#include <QDebug>

Q_LOGGING_CATEGORY(knotlinkSubscriber, "knotlink.subscriber")

SignalSubscriber::SignalSubscriber(QString APPID, QString SignalID, QObject *parent)
    : QObject(parent), appID(APPID), signalID(SignalID)
{
    init();
}


void SignalSubscriber::init()
{
    KLsubscriber = new TcpClient(this);
    KLsubscriber->connectToServer("127.0.0.1",6372);
    connect(KLsubscriber, &TcpClient::receivedData, this, &SignalSubscriber::dataRecv);
    QString s_key = appID + "-" + signalID;
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    KLsubscriber->sendData(s_key_bytes);
}

void SignalSubscriber::subscribe(QString APPID, QString SignalID)
{
    appID = APPID;
    signalID = SignalID;
    QString s_key = appID + "-" + signalID;
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    KLsubscriber->sendData(s_key_bytes);
}

void SignalSubscriber::dataRecv(const QByteArray &data)
{
    emit receivedData_a(data);
    // 将 QByteArray 转换为 QString
    QString receivedText = QString::fromUtf8(data);
    emit receivedData(receivedText);
}

