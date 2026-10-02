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

#include "upowerdbus.h"

UpowerDBus::UpowerDBus()
{
    upowerinfo = new UPowerInfo;
    upowerinfo->initUpowerInfo();
    connect(upowerinfo, &UPowerInfo::acChanged, this, &UpowerDBus::dealPowerState);
    connect(upowerinfo, &UPowerInfo::lidChanged, this, &UpowerDBus::dealLidState);
}

UpowerDBus::~UpowerDBus()
{
    delete upowerinfo;
}

QString UpowerDBus::UPowerVersion()
{
    showServiceMsg();
    return "3.1";
}

QString UpowerDBus::CanHibernate()
{
    showServiceMsg();
    return upowerinfo->CanHibernate();
}

QString UpowerDBus::MachineType()
{
    showServiceMsg();
    return upowerinfo->MachineType();
}

bool UpowerDBus::OnBattery()
{
    showServiceMsg();
    return upowerinfo->OnBattery();
}

bool UpowerDBus::LidIsClosed()
{
    showServiceMsg();
    return upowerinfo->LidIsClosed();
}

QString UpowerDBus::DaemonVersion()
{
    showServiceMsg();
    return upowerinfo->DaemonVersion();
}

//*****************************************

/******************************************/

bool UpowerDBus::CanAdjustBacklight()
{
    showServiceMsg();
    return upowerinfo->CanAdjustBacklight();
}

void UpowerDBus::dealPowerState(bool state)
{
    emit PowerState(state);
}

void UpowerDBus::dealLidState(bool state)
{
    emit LidState(state);
}
