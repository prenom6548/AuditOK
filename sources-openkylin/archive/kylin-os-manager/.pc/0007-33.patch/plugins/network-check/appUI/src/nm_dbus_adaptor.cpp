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

#include "nm_dbus_adaptor.h"
#include <QDebug>
NmDbusAdaptor::NmDbusAdaptor(QObject *parent) : QObject(parent)
{
    // m_NmDBus = new KylinDBus();
}
NmDbusAdaptor::~NmDbusAdaptor()
{
    // if (m_NmDBus)
    // {
    //     delete m_NmDBus;
    // }
}
bool NmDbusAdaptor::checkHWConfig(QStringList &configInfo)
{
    if (isWiredCableOn() || isWirelessCardOn()) {
        return true;
    } else {
        return false;
    }
}

bool NmDbusAdaptor::isWiredCableOn() const
{
    //    return m_NmDBus->isWiredCableOn;
    return true;
}

bool NmDbusAdaptor::isWirelessCardOn() const
{
    //    return m_NmDBus->isWirelessCardOn;
    return true;
}
