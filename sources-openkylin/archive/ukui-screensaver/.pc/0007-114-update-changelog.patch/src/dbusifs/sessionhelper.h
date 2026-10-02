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

#ifndef SESSIONHELPER_H
#define SESSIONHELPER_H

#include <QFile>
#include <QTimer>
#include <QDBusArgument>
#include "definetypes.h"

namespace InhibitInfo
{
struct InhibitorInfo
{
    QString name;
    QString icon;
};

QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct);

const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct);
} // namespace InhibitInfo

Q_DECLARE_METATYPE(InhibitInfo::InhibitorInfo)

class QDBusInterface;
class Login1Helper;

class SessionHelper : public QObject
{
    Q_OBJECT
public:
    explicit SessionHelper(QObject *parent = nullptr);

    virtual ~SessionHelper();

    QStringList getLockCheckStatus(QString type);

    bool canAction(Action action);

    bool doAction(const QString &powerManagerfunc);

private:
    QDBusInterface *m_sessionInterface = nullptr;

    QTimer *up_to_time = nullptr;

    void doPowerManager(const QString &powerManagerfunc);

    bool playShutdownMusic(const QString &powerManagerfunc);
};

#endif // SESSIONHELPER_H
