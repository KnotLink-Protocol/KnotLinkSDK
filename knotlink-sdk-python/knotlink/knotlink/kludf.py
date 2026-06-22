# KLUDF.py
# Copyright (C) 2025 HXH
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see https://www.gnu.org/licenses/

class KLUDF:
    def __init__(self):
        pass


class KLKVMap(dict):
    def serialize(self):
        """
        将 KLKVMap 序列化为键值对字符串
        :return: 键值对字符串
        """
        return ";".join(f"{key}={value}" for key, value in self.items())

    def deserialize(self, keyValueString):
        """
        将键值对字符串反序列化为 KLKVMap
        :param keyValueString: 键值对字符串
        """
        self.clear()  # 清空当前映射
        pairs = keyValueString.split(";")
        for pair in pairs:
            keyValue = pair.split("=", maxsplit=1)
            if len(keyValue) == 2:
                self[keyValue[0]] = keyValue[1]

    def get(self, key , defaultVal = ""):
        """
        安全地读取键值对
        :param key: 键
        :return: 键对应的值，如果键不存在则返回空字符串
        """
        return super().get(key, defaultVal)


# 测试代码
if __name__ == "__main__":
    kv_map = KLKVMap()
    kv_map["key1"] = "value1"
    kv_map["key2"] = "value2"

    print("序列化前：", kv_map)
    serialized = kv_map.serialize()
    print("序列化后：", serialized)

    new_kv_map = KLKVMap()
    new_kv_map.deserialize(serialized)
    print("反序列化后：", new_kv_map)

    print("获取 key1 的值：", new_kv_map.get("key1"))
    print("获取不存在的 key3 的值：", new_kv_map.get("key3"))