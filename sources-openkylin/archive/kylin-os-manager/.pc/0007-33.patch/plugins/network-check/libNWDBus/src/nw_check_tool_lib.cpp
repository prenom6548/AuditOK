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

#include "nw_check_tool_lib.h"
NWCheckToolLib *NWCheckToolLib::getInstance()
{
    static NWCheckToolLib instance;
    return &instance;
}
NWCheckToolLib::NWCheckToolLib()
{
    m_kyDBus = new KylinDBus();
}
bool NWCheckToolLib::hasValidNetCard()
{
    m_kyDBus->getObjectPath();
    m_kyDBus->getPhysicalCarrierState(0);
    if (m_kyDBus->isWiredCableOn || m_kyDBus->isWirelessCardOn) {
        return true;
    } else {
        return false;
    }
}
bool NWCheckToolLib::hasValidConnection()
{
    m_primaryConnectionPath = m_kyDBus->getPrimaryConnection();

    if (m_primaryConnectionPath == "/") {
        return false;
    } else {
        primaryConnUUID();
        return true;
    }
}

bool NWCheckToolLib::isWiredConnection()
{
    m_isPrimaryConnectionWired = m_kyDBus->isWiredConnection();
    return m_isPrimaryConnectionWired;
}

QString NWCheckToolLib::primaryConnUUID()
{
    m_primaryConnectionUUID = m_kyDBus->getPrimaryConnUUID();

    return m_primaryConnectionUUID;
}
bool NWCheckToolLib::isIPAutoConfig()
{
    if (m_primaryConnectionPath == "/") {
        //        ipConfigRes = "无首选网络";
        return false;
    }

    m_primarySettingPath = m_kyDBus->getPrimarySetting();
    if (m_primarySettingPath == "/") {
        //        ipConfigRes = "无首选网络";
        return false;
    }

    m_primarySettingPath = m_kyDBus->getPrimaryConfig();
    if (m_primarySettingPath == "/") {
        //        ipConfigRes = "无首选网络";
        return false;
    }
    bool res = m_kyDBus->isPrimaryNetDHCP();

    if (res) {
        qDebug() << "it is DHCP mode ip";
    } else {
        qDebug() << "it isnot DHCP mode ip";
    }

    return res;
}

bool NWCheckToolLib::isSameVlan()
{
    if (!m_kyDBus->isPrimaryNetDHCP()) {
        QString res = "";
        if (m_kyDBus->isSameVlan()) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


bool NWCheckToolLib::isDHCPOK()
{
    if (m_kyDBus->isDHCPOK()) {
        return true;
    } else {
        return false;
    }
}

QStringList NWCheckToolLib::getDNS()
{
    return m_kyDBus->getDNS();
}

bool NWCheckToolLib::isDNSSet()
{
    if (m_kyDBus->isDNSSet()) {
        return true;
    } else {
        return false;
    }
}

bool NWCheckToolLib::hostFileisLegal()
{
    if (m_kyDBus->isWiredCableOn || m_kyDBus->isWirelessCardOn) {
        return true;
    } else {
        return false;
    }
}
