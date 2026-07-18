/*
 * KnotLink SDK - Qt/C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#ifndef KLUDF_H
#define KLUDF_H

#include <QMap>
#include <QString>
#include <QStringList>

class KLUDF
{
public:
    KLUDF() = default;
    virtual ~KLUDF() = default;
};

class KLKVMap : public QMap<QString, QString>
{
public:
    // 将 KLKVMap 序列化为键值对字符串
    QString serialize() const;

    // 将键值对字符串反序列化为 KVMap
    void deserialize(const QString& keyValueString);

    // 安全地读取键值对
    QString get(const QString& key, const QString& defaultVal = QString()) const;
};

#endif // KLUDF_H
