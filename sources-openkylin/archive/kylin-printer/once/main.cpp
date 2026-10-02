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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QObject>
#include <QScreen>
#include <fcntl.h>
#include <syslog.h>

#include "device_map.h"
#include "log.h"

int main(int argc, char *argv[])
{
    // 统一日志输出
#ifndef TEST_DEBUG
    qInstallMessageHandler(logOutput);
#else
    qInstallMessageHandler(myMessageOutput);
#endif
    QCoreApplication app(argc, argv);

    DeviceMapOnce *once = new DeviceMapOnce();
    QObject::connect(once, &DeviceMapOnce::done, once, &DeviceMapOnce::finished);
    QObject::connect(once, &DeviceMapOnce::done, &app, &QCoreApplication::quit);
    once->startWorker();

    return app.exec();
}
