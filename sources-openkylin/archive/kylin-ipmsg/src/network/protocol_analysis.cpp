/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "protocol_analysis.h"

#include <QDebug>

#define PACKAGE_SIZE (1024 * 100)

ProtocolAnalysis::ProtocolAnalysis() {}

ProtocolAnalysis::~ProtocolAnalysis() {}

QMap<QByteArray, QByteArray> ProtocolAnalysis::analysisHeadBody(QByteArray str)
{
    QMap<QByteArray, QByteArray> dst;
    QByteArray key;
    QByteArray value;
    dst.clear();
    key.clear();
    value.clear();

    char *first = NULL;
    char *separator = NULL;
    char *end = NULL;
    char *data = NULL;
    char *tmp = NULL;

    if (str.isEmpty()) {
        qDebug() << "Waring : ProtocolAnalysis , analysisHeadBody , orignal str is empty!!!";
        return dst;
    }

    char *originalStr = str.data();

    /* 获取指向数据第一个字符的指针 */
    data = strstr(originalStr, "\r\n\r\n");
    if (data == NULL) {
        qDebug() << "Error : ProtocolAnalysis , analysisHeadBody , package head error";
        return dst;
    }
    data = data + 4;

    /* 解析包头key */
    first = originalStr;

    while (first + 2 != data) {
        separator = strchr(first, ':');
        end = strstr(first, "\r\n");

        int size = separator - first + 1;
        tmp = (char *)malloc(size);
        memset(tmp, 0x00, size);

        strncpy(tmp, first, separator - first);

        /* 保存key */
        key = QByteArray(tmp, separator - first);

        /* 解析value */
        size = end - separator;
        tmp = (char *)realloc(tmp, size);
        memset(tmp, 0x00, size);

        strncpy(tmp, separator + 1, end - separator - 1);

        /* 保存value */
        value = QByteArray(tmp, end - separator - 1);

        /* 保存key , value */
        dst.insert(key, value);

        /* 释放堆内存 */
        free(tmp);
        tmp = NULL;

        /* 重置first指针 */
        first = end + 2;
    }

    /* 解析包体 */
    /* 从包头中获取包体长度 */
    char buff[PACKAGE_SIZE];
    memset(buff, 0x00, sizeof(buff));

    int bodySize = dst.value(QByteArray("body_size")).toInt();
    memcpy(buff, data, bodySize);

    key = QByteArray("body");
    value = QByteArray(buff, bodySize);

    dst.insert(key, value);

    return dst;
}

QByteArray ProtocolAnalysis::encapsulationHeadBody(QMap<QByteArray, QByteArray> map)
{

    QByteArray dst;
    QByteArray data;

    dst.clear();
    data.clear();

    if (map.isEmpty()) {
        qDebug() << "Waring : ProtocolAnalysis , encapsulationHeadBody , map is empty!!!";
        return dst;
    }


    data = map.value("body", "");

    /* 遍历map , 拼接包头 */
    QMap<QByteArray, QByteArray>::iterator begin = map.begin();
    QMap<QByteArray, QByteArray>::iterator end = map.end();

    while (begin != end) {
        QByteArray key = begin.key();
        QByteArray value = begin.value();

        if (key != QString("body")) {
            dst.append(key);
            dst.append(":");
            dst.append(value);
            dst.append("\r\n");
        }
        begin++;
    }

    dst.append("\r\n");

    if (!data.isEmpty()) {
        dst.append(data);
    }

    /* 补齐单包长度 */
    dst.resize(PACKAGE_SIZE);

    return dst;
}
