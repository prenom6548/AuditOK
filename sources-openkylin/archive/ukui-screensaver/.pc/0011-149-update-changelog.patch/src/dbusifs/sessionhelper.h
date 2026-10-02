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
#include <QSharedPointer>
#include "../common/definetypes.h"

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
    explicit SessionHelper(QSharedPointer<Login1Helper> login1Helper, QObject *parent = nullptr);

    virtual ~SessionHelper();

    QStringList getLockCheckStatus(QString type);

    bool canAction(Action action);

    bool doAction(const QString &powerManagerfunc);

private Q_SLOTS:
    void onSessionDbusActiveChanged(const QString &strServerName, bool isActive);

private:
    void init();

    void doPowerManager(const QString &powerManagerfunc);

    bool playShutdownMusic(const QString &powerManagerfunc);

private:
    QDBusInterface *m_sessionInterface = nullptr;

    QTimer *m_timerPlayMusic = nullptr;

    QSharedPointer<Login1Helper> m_login1Helper = nullptr;
};

#endif // SESSIONHELPER_H
