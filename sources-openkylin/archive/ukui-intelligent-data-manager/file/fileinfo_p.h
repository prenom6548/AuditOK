/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEINFO_P_H
#define FILEINFO_P_H

#include <QMutex>
#include <QColor>

namespace UKUI {
namespace IDM {
class FileInfoPrivate
{
public:
    enum State
    {
        Invalid,
        Valid
    };

    FileInfoPrivate() {}

    State state = Invalid;

    QString uri;
    QString path;
    QString parentPath;

    QString name;
    QString iconName;
    QString createDateString;
    QString modifyDateString;
    QString openDateString;
    QString dateString;
    quint64 timeAccessMsec = -1;
    quint64 timeModifiedMsec = -1;
    quint64 timeCreatedMsec = -1;
    qint64 timeRecentModified = -1;
    QString version;

    QString mimeType;
    QString fileType;

    quint64 size;
    QString sizeFormat;

    QList<int> labelIds;
    QList<QVariant> labelColors;

    quint64 openCount = 0;

    QMutex mutex;
};
}
}

#endif // FILEINFO_P_H
