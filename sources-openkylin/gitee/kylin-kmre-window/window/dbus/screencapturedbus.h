/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "typedef.h"
#include "singleton.h"
#include <QtDBus/QDBusInterface>
#include <QVariantMap>
#include <QThread>

namespace kmre {

class ScreenCaptureDbus : public QObject, public SingletonP<ScreenCaptureDbus>
{
    Q_OBJECT
public:
    void graphicCapture(const QString& path, int delay, uint id);
    QString introspect();
    QVariantMap getSaveInfo();

private slots:
    void onInit();

signals:
    void sigCaptureTaken(uint id, QByteArray rawImage);
    void sigCaptureCopy(uint id);
    void sigCaptureExit(uint id);
    void sigCaptureFailed(uint id);

private:
    QThread* mDbusThread;
    up<QDBusInterface> mScreenShotDbusIntrospectable;
    up<QDBusInterface> mScreenShotDbusKylinInterface;

    ScreenCaptureDbus();
    ~ScreenCaptureDbus();

    friend SingletonP<ScreenCaptureDbus>;
};

}


