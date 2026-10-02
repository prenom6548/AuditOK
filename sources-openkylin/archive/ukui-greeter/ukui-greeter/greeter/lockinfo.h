/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef LOCKINFO_H
#define LOCKINFO_H

#include <QString>
#include <QDBusArgument>
#include <QVector>

namespace InhibitInfo {
    struct InhibitorInfo {
        QString name;
        QString icon;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct);
}

Q_DECLARE_METATYPE(InhibitInfo::InhibitorInfo)

namespace SystemdInhibitor {
    struct Inhibitor {
        QString action;
        QString name;
        QString reason;
        QString mode;
        int uid;
        int pid;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct);
}

Q_DECLARE_METATYPE(SystemdInhibitor::Inhibitor)

class lockinfo
{
public:
    lockinfo();
    ~lockinfo();

public:
    enum InhibitorType {
        logout,
        suspend,
        shutdown
    };

    enum buttonType {
        switchuserBtn,
        hibernateBtn,
        suspendBtn,
        lockscreenBtn,
        logoutBtn,
        rebootBtn,
        shutdownBtn
    };

    //static QVector<InhibitInfo::InhibitorInfo> listInhibitorInfo(lockinfo::InhibitorType type);
    static bool getSystemdInhibitor(QString type);

private:



};

#endif // LOCKINFO_H
