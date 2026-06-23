/*
 * KnotLink SDK - C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef OPENSOCKETQUERIER_H
#define OPENSOCKETQUERIER_H

#include <QObject>
#include <QEventLoop>
#include "tcpclient.h"

class OpenSocketQuerier : public QObject
{
    Q_OBJECT
public:
    explicit OpenSocketQuerier(QObject *parent = nullptr);
    explicit OpenSocketQuerier(QString APPID, QString OpenSocketID ,QObject *parent = nullptr);// initialize and chang config
    void setConfig(QString APPID, QString OpenSocketID); // chang config
    QString query_l(QString data);
    void query(QString data); // emit data
    void query(QByteArray data); // emit data
    void query(QString APPID, QString OpenSocketID,QString data); // temp change config
    void query(QString APPID, QString OpenSocketID,QByteArray data); // temp change config

signals:
    void dataBack(QString data);
    void dataBack_a(QByteArray data);

public slots:
    void dataRecv(const QByteArray &data);

private:
    TcpClient* KLquerier;
    QString appID;
    QString openSocketID;
    bool lock = 0;
    void init();
};

#endif // OPENSOCKETQUERIER_H
