/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef OUTPUTINFO_H
#define OUTPUTINFO_H

#include <QObject>
#include <QRect>

/**
 * @since 2.5.1.0
 */

class OutputInfo : public QObject
{
    Q_OBJECT
public:
    ~OutputInfo();

    QString outputName();
    QRect usableArea();

private:
    explicit OutputInfo(QObject *parent = nullptr);
    void setOutputName(const QString &outputName);
    void setUsableArea(const QRect &usableArea);

    friend class OutputUsableAreaManager;
    class Private;
    QScopedPointer<Private> d;
};

#endif // OUTPUTINFO_H
