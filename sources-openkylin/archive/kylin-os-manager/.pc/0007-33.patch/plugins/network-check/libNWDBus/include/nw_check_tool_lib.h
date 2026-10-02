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

#ifndef NWCHECKTOOLLIB_H
#define NWCHECKTOOLLIB_H

#include "NWCheckToolLib_global.h"
#include "kylin-dbus-interface.h"
#define g_nwCheckTool (NWCheckToolLib::getInstance())
class NWCHECKTOOLLIB_EXPORT NWCheckToolLib
{
public:
    static NWCheckToolLib *getInstance();
    bool hasValidNetCard();
    bool hasValidConnection();
    bool isWiredConnection();
    bool isIPAutoConfig();
    bool isSameVlan();
    bool isDHCPOK();
    bool isDNSSet();
    QStringList getDNS();
    bool hostFileisLegal();

private:
    NWCheckToolLib();
    QString primaryConnUUID();
    KylinDBus *m_kyDBus = nullptr;

    QString m_primaryConnectionPath = "";
    bool m_isPrimaryConnectionWired = "";
    QString m_primaryConnectionUUID = "";

    QString m_primarySettingPath = "";
};

#endif // NWCHECKTOOLLIB_H
