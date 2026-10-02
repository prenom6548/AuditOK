/*
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
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

#ifndef LOCKCHECKER_H
#define LOCKCHECKER_H

#include "ukuilockinfo.h"
#include <QFile>
#include <QDBusArgument>

//为了兼容性，要在tools中加入ukuilockinfo.h，而在该头文件中已经定义了InhibitorInfo，为了避免重复定义，注销此处定义
//namespace InhibitInfo {
//    struct InhibitorInfo {
//        QString name;
//        QString icon;
//    };

//    QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct);

//    const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct);
//}

//Q_DECLARE_METATYPE(InhibitInfo::InhibitorInfo)

class LockChecker
{
public:
    LockChecker();
    ~LockChecker();

public:
    static QStringList getLoginedUsers();

    static QVector<InhibitInfo::InhibitorInfo> listInhibitor(QString type);

    static int getCachedUsers();

private:
    static QString getName(QFile *a);

    static bool getInhibitorLocal(QVector<InhibitInfo::InhibitorInfo> &inhibitVec, QString type);
};

#endif // LOCKCHECKER_H
