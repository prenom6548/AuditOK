/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dbus-types.h"
#include <QSysInfo>
#include <QtEndian>

DbusImageStruct::DbusImageStruct(const QImage &image)
{
    width = image.size().width();
    height = image.size().height();
    if (image.format() == QImage::Format_ARGB32) {
        data = QByteArray((char *)image.bits(), image.sizeInBytes());
    } else {
        QImage image32 = image.convertToFormat(QImage::Format_ARGB32);
        data = QByteArray((char *)image32.bits(), image32.sizeInBytes());
    }

    // swap to network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        quint32 *uintBuf = (quint32 *)data.data();
        for (uint i = 0; i < data.size() / sizeof(quint32); ++i) {
            *uintBuf = qToBigEndian(*uintBuf);
            ++uintBuf;
        }
    }
}

// Marshall the ImageStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const DbusImageStruct &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.data;
    argument.endStructure();
    return argument;
}
#include <QDebug>

// Retrieve the ImageStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusImageStruct &icon)
{
    qint32 width = 0;
    qint32 height = 0;
    QByteArray data;

    if (argument.currentType() == QDBusArgument::StructureType) {
        argument.beginStructure();
        argument >> width;
        argument >> height;
        argument >> data;
        argument.endStructure();
    }
    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}

// Marshall the ImageVector data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const DbusImageVector &iconVector)
{
    argument.beginArray(qMetaTypeId<DbusImageStruct>());
    for (int i = 0; i < iconVector.size(); ++i) {
        argument << iconVector[i];
    }
    argument.endArray();
    return argument;
}

// Retrieve the ImageVector data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusImageVector &iconVector)
{
    iconVector.clear();

    if (argument.currentType() == QDBusArgument::ArrayType) {
        argument.beginArray();

        while (!argument.atEnd()) {
            DbusImageStruct element;
            argument >> element;
            iconVector.append(element);
        }

        argument.endArray();
    }

    return argument;
}

// Marshall the ToolTipStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const DbusToolTipStruct &toolTip)
{
    argument.beginStructure();
    argument << toolTip.icon;
    argument << toolTip.image;
    argument << toolTip.title;
    argument << toolTip.subTitle;
    argument.endStructure();

    return argument;
}

// Retrieve the ToolTipStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusToolTipStruct &toolTip)
{
    QString icon;
    DbusImageVector image;
    QString title;
    QString subTitle;

    if (argument.currentType() == QDBusArgument::StructureType) {
        argument.beginStructure();
        argument >> icon;
        argument >> image;
        argument >> title;
        argument >> subTitle;
        argument.endStructure();
    }

    toolTip.icon = icon;
    toolTip.image = image;
    toolTip.title = title;
    toolTip.subTitle = subTitle;

    return argument;
}