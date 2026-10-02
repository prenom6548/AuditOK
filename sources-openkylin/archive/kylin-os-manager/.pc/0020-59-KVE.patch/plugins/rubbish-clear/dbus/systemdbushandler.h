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

#ifndef SYSTEMDBUSHANDLER_H
#define SYSTEMDBUSHANDLER_H

#include "systemdbusproxy.h"
#include "systeminterface.h"
#include "customdata.h"

#include <QObject>
#include <QDebug>
#include <QDBusConnection>
namespace KylinRubbishClear
{

class SystemDbusHandler : public QObject
{
    Q_OBJECT

public:
    explicit SystemDbusHandler(QObject *parent = 0) : QObject(parent)
    {
        m_sysDbus = new SystemInterface("com.kylin.assistant.qsystemdbus", "/com/kylin/assistant/qsystemdbus",
                                        QDBusConnection::systemBus(), this);
        connect(m_sysDbus, &SystemInterface::reportAlert, this, &SystemDbusHandler::reportAlert);
    }

    QString demoInfo()
    {
        return m_sysDbus->demoInfo();
    }

    bool userIsActive(const QString &user, bool active)
    {
        return m_sysDbus->userIsActive(user, active);
    }

    CustomData getCustomData()
    {
        return m_sysDbus->getCustomData();
    }

    void sendCustomData(const CustomData &message)
    {
        m_sysDbus->sendCustomData(message);
    }

    void exitService()
    {
        m_sysDbus->exitService();
    }

Q_SIGNALS:
    void reportAlert(int ret, const QString &description);

private:
    SystemInterface *m_sysDbus = nullptr;
};

} // namespace KylinRubbishClear

#endif
