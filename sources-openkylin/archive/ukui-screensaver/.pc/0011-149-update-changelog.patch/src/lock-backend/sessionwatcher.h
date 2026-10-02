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
#ifndef SESSIONWATCHER_H
#define SESSIONWATCHER_H

#include <QObject>
#include <QDBusObjectPath>
#include <QGSettings>
#include <QString>
#include <QTimer>
#include <QSettings>
#include <QProcess>
#include <QSharedPointer>
#include "gsettingshelper.h"

class SessionWatcher : public QObject
{
    Q_OBJECT
public:
    enum
    {
        SESSION_STATUS_SCREENSAVER = 1, // 屏保
        SESSION_STATUS_SCREENCLOSE = 2, // 关屏
        SESSION_STATUS_SLEEPED = 4,     // 休眠/睡眠
    };
    explicit SessionWatcher(QSharedPointer<GSettingsHelper> m_gsettingsHelper, QObject *parent = nullptr);

Q_SIGNALS:
    void sessionIdle();
    void sessionLockIdle();
    void sessionIdleExit();

public Q_SLOTS:
    void onStatusChanged(unsigned int status);
    void onSessionRemoved(const QDBusObjectPath &objectPath);
    void onLockScreenConfigChanged(QString strKey, QVariant value);

private:
    QString sessionPath;
    int idleDelay = -1;
    int m_idleLock = -1;
    int m_nLockTimeout = -1;
    QTimer *m_timer = nullptr;
    QTimer *m_timer2 = nullptr;
    bool m_lidState = false;
    double defaultFontSize;
    bool lockState;
    int m_nStatus = 0; // 当前状态
    QTimer *m_timerLock = nullptr;
    QSharedPointer<GSettingsHelper> m_gsettingsHelper = nullptr;
};

#endif // SESSIONWATCHER_H
