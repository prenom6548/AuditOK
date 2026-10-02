/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QStandardPaths>
#include "sysdbusregister.h"
#include "syslog-handler.hpp"

// 初始化静态成员变量
QMutex SyslogHandler::mutex;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("Kylin Team");
    app.setApplicationName("ukui-powermanagement-service");

    // 安装消息处理程序
    qInstallMessageHandler(SyslogHandler::staticMessageHandler);

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService("org.ukui.powermanagement")) {
        qCritical() << "QDbus register service failed reason:" << systemBus.lastError();
        exit(0);
    }

    if (!systemBus.registerObject(
            "/", new SysdbusRegister(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties)) {
        qCritical() << "QDbus register object failed reason:" << systemBus.lastError();
        exit(0);
    }

    return app.exec();
}
