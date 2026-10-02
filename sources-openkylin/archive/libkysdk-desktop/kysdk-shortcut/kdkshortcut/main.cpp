/*
 * libkysdk-shortcut's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QDBusConnection>

#include "keybindingsserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.kylin.kysdk.shortcut"))
    {
        qDebug() << "com.kylin.kysdk.shortcut 服务注册失败";
        return -1;
    }

    KeybindingsServer *server = new KeybindingsServer();
    if (!connection.registerObject("/com/kylin/kysdk/shortcut", server,
                                   QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/shortcut 对象注册失败";
        delete server;
        return -1;
    }

    return a.exec();
}
