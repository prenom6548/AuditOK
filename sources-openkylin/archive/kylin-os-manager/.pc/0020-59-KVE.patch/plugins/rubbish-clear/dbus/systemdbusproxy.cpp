/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "systemdbusproxy.h"
#include "systemdbushandler.h"

#include <QDebug>
#include <QThread>
namespace KylinRubbishClear
{

SystemDbusProxy::SystemDbusProxy(QObject *parent) : QObject(parent)
{
    m_handler = new SystemDbusHandler;

    QThread *handlerwork = new QThread;
    m_handler->moveToThread(handlerwork);
    handlerwork->start();

    connect(m_handler, &SystemDbusHandler::reportAlert, this, &SystemDbusProxy::reportAlert);
}

SystemDbusProxy::~SystemDbusProxy()
{
    this->exitService();
    m_handler->deleteLater();
}

QString SystemDbusProxy::demoInfo()
{
    return m_handler->demoInfo();
}

bool SystemDbusProxy::userIsActive(const QString &user, bool active)
{
    return m_handler->userIsActive(user, active);
}

CustomData SystemDbusProxy::getCustomData()
{
    return m_handler->getCustomData();
}

void SystemDbusProxy::sendCustomData(const CustomData &message)
{
    m_handler->sendCustomData(message);
}

void SystemDbusProxy::exitService()
{
    m_handler->exitService();
}
} // namespace KylinRubbishClear
