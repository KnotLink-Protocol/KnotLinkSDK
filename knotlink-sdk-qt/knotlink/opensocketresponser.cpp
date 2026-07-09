/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "opensocketresponser.h"
#include <QDebug>

Q_LOGGING_CATEGORY(knotlinkResponser, "knotlink.responser")

OpenSocketResponser::OpenSocketResponser(QString APPID, QString OpenSocketID, QObject *parent)
    : QObject(parent), appID(APPID), openSocketID(OpenSocketID)
{
    init();
}


void OpenSocketResponser::init()
{
    KLresponser = new TcpClient(this);
    KLresponser->connectToServer("127.0.0.1",6378);
    connect(KLresponser, &TcpClient::receivedData, this, &OpenSocketResponser::dataRecv);
    QString s_key = appID + "-" + openSocketID;
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    KLresponser->sendData(s_key_bytes);
}

void OpenSocketResponser::dataRecv(const QByteArray &data)
{
    QString s_data = QString::fromUtf8(data);

    QString delimiter = "&*&"; // 分隔符
    QStringList parts = s_data.split(delimiter); // 按分隔符分割字符串

    if (parts.size() < 2) {
        qCWarning(knotlinkResponser) << "Invalid data format, missing delimiter:" << delimiter;
        return;
    }

    QString key = parts[0];
    // payload 是分隔符之后的所有内容（payload 本身可能包含 &*&）
    QString t_data = s_data.mid(key.length() + delimiter.length());

    emit receivedData_a(t_data.toUtf8(),key);
    emit receivedData(t_data,key);
}

void OpenSocketResponser::sendBack(const QString &data,QString questionID)
{
    sendBack(data.toUtf8(),questionID);
}

void OpenSocketResponser::sendBack(const QByteArray &data, QString questionID)
{
    QByteArray result = questionID.toUtf8() + "&*&" + data;
    KLresponser->sendData(result);
}
