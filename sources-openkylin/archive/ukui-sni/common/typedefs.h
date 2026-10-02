/*

    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QVector>
#include <QImage>
#include <QList>

struct DbusImageStruct {
    DbusImageStruct(){};
    explicit DbusImageStruct(const QImage &image);
    int width;
    int height;
    QByteArray data;
};

typedef QVector<DbusImageStruct> DbusImageVector;

struct DbusToolTipStruct {
    QString icon;
    DbusImageVector image;
    QString title;
    QString subTitle;
};

typedef QList<quint32> UintList;
Q_DECLARE_METATYPE(UintList)
Q_DECLARE_METATYPE(DbusImageStruct)
Q_DECLARE_METATYPE(DbusImageVector)
Q_DECLARE_METATYPE(DbusToolTipStruct)
