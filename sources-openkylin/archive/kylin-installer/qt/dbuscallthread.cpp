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
#include "dbuscallthread.h"

DbusCallThread::DbusCallThread(QVariantList json, QString runtime, QString appName, QString debPath)
{
    m_runtime = runtime;
    m_appName = appName;
    m_jsonInfo = json;
    m_debPath = debPath;
}

void DbusCallThread::run()
{
    
    QDBusInterface *serviceInterface = new QDBusInterface("com.kylin.runtime",
                                                          "/com/kylin/runtime",
                                                          "com.kylin.runtime.interface",
                                                          QDBusConnection::systemBus());
    serviceInterface->setTimeout(2147483647);

    if (!serviceInterface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return ;
    }
    serviceInterface->call(QDBus::Block, "kre_install_app", m_runtime, m_debPath);

    // 是否安装成功的判断
    QDBusReply<int> reply = serviceInterface->call("kre_install_status", m_runtime, m_appName);
    if (reply) {
        serviceInterface->call(QDBus::Block, "saveJsonFile", m_jsonInfo);
        emit installSuccess();
    } else {
        emit installFail();
    }
    delete serviceInterface;
    serviceInterface = nullptr;
}
