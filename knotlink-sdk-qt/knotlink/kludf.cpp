/*
 * KnotLink SDK - Qt/C++
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

#include "kludf.h"

// ---------- KLKVMap::serialize ----------
// 格式: key1=value1;key2=value2;key3=value3

QString KLKVMap::serialize() const
{
    QStringList pairs;
    pairs.reserve(size());
    for (auto it = cbegin(); it != cend(); ++it)
        pairs.append(it.key() + '=' + it.value());
    return pairs.join(';');
}

// ---------- KLKVMap::deserialize ----------

void KLKVMap::deserialize(const QString& keyValueString)
{
    clear();
    if (keyValueString.isEmpty()) return;

    const QStringList pairs = keyValueString.split(';');
    for (const QString& pair : pairs) {
        if (pair.isEmpty()) continue;
        const int eqPos = pair.indexOf('=');
        if (eqPos >= 0)
            insert(pair.left(eqPos), pair.mid(eqPos + 1));
    }
}

// ---------- KLKVMap::get ----------

QString KLKVMap::get(const QString& key, const QString& defaultVal) const
{
    return value(key, defaultVal);
}
