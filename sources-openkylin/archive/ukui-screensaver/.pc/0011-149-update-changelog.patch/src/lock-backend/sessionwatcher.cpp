/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "sessionwatcher.h"
#include <QDBusInterface>
#include <QDebug>
#include <QFile>
#include <QDBusReply>
#include "../../src/common/definetypes.h"
#include "../../src/common/utils.h"

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"

SessionWatcher::SessionWatcher(QSharedPointer<GSettingsHelper> helper, QObject *parent) : QObject(parent), m_gsettingsHelper(helper)
{
    sessionPath = qgetenv("XDG_SESSION_PATH");

    // defaultFontSize = getDefaultFontSize();
    defaultFontSize = 10;

    QDBusInterface *interface = new QDBusInterface(
        SM_DBUS_SERVICE, SM_DBUS_PATH, SM_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);

    connect(interface, SIGNAL(StatusChanged(unsigned int)), this, SLOT(onStatusChanged(unsigned int)));

    QDBusInterface *displayManagerInterface
        = new QDBusInterface(DM_DBUS_SERVICE, DM_DBUS_PATH, DM_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
    connect(
        displayManagerInterface,
        SIGNAL(SessionRemoved(QDBusObjectPath)),
        this,
        SLOT(onSessionRemoved(QDBusObjectPath)));

    connect(
        m_gsettingsHelper.get(), &GSettingsHelper::lockScreenConfigChanged, this, &SessionWatcher::onLockScreenConfigChanged);
    // 重置屏保从不字段
    bool isEnable = m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_ACTIVATION_ENABLED).toBool();
    if (!isEnable) {
        m_gsettingsHelper->SetLockScreenConf(KEY_IDLE_ACTIVATION_ENABLED, true);
        m_gsettingsHelper->SetLockScreenConf(KEY_IDLE_DELAY, -1);
    }
    // 重置锁屏时间(不处理超时锁屏220620)
    m_gsettingsHelper->SetLockScreenConf(KEY_IDLE_LOCK, -1);

    idleDelay = m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_DELAY).toInt();
    m_idleLock = m_gsettingsHelper->GetLockScreenConf(KEY_IDLE_LOCK).toInt();

    QVariant lockTimeout = m_gsettingsHelper->GetSessionConf(KEY_LOCK_TIMEOUT);
    if (!lockTimeout.isNull()) {
        m_nLockTimeout = lockTimeout.toInt();
    }

    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT)) {
        // 重置屏保休息时间字段
        QVariant varIsShow = m_gsettingsHelper->GetScreenSaverConf(KEY_SHOW_REST_TIME);
        if (!varIsShow.isNull() && !varIsShow.toBool()) {
            m_gsettingsHelper->SetScreenSaverConf(KEY_SHOW_REST_TIME, true);
            m_gsettingsHelper->SetScreenSaverConf(KEY_SHOW_CUSTOM_REST_TIME, varIsShow.toBool());
            m_gsettingsHelper->SetScreenSaverConf(KEY_SHOW_UKUI_REST_TIME, varIsShow.toBool());
        }
    }

    // for PowerManager
    // 同步旧的电源管理唤醒是否需密码配置
    FILE *fp = popen("xset s 0 0", "r");
    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    if (m_gsettingsHelper) {
        bool ret = m_gsettingsHelper->GetPowerManagerConf(KEY_LOCK_SUSPEND).toBool();
        if (ret) {
            m_gsettingsHelper->SetPowerManagerConf(KEY_LOCK_SUSPEND, false);
        }
        ret = m_gsettingsHelper->GetPowerManagerConf(KEY_LOCK_HIBERNATE).toBool();
        if (ret) {
            m_gsettingsHelper->SetPowerManagerConf(KEY_LOCK_HIBERNATE, false);
        }
        bool oldValue = m_gsettingsHelper->GetPowerManagerConf(KEY_LOCK_BLANKSCREEN).toBool();
        if (!oldValue) {
            m_gsettingsHelper->SetLockScreenConf(KEY_CLOSE_ACTIVATION_ENABLED, oldValue);
            m_gsettingsHelper->SetPowerManagerConf(KEY_LOCK_BLANKSCREEN, true);
        }
    }
}

void SessionWatcher::onStatusChanged(unsigned int status)
{
    if (status == SESSION_IDLE) {
        qDebug() << idleDelay;
        if (idleDelay != -1) {
            if (!m_timer) {
                m_timer = new QTimer(this);
                connect(m_timer, &QTimer::timeout, this, [&] {
                    Q_EMIT sessionIdle();
                    m_timer->stop();
                });
            }
            int time = (idleDelay - 1) * 60000;
            if (time < 0)
                time = 0;

            m_timer->start(time);
        }

        if (m_idleLock != -1) {
            if (!m_timer2) {
                m_timer2 = new QTimer(this);
                connect(m_timer2, &QTimer::timeout, this, [&] {
                    Q_EMIT sessionLockIdle();
                    m_timer2->stop();
                });
            }
            int lockTime = (m_idleLock - 1) * 60000;
            if (lockTime < 0)
                lockTime = 0;

            m_timer2->start(lockTime);
        }

    } else if (status == SESSION_AVAILABLE) {
        if (m_timer && m_timer->isActive()) {
            m_timer->stop();
        }
        if (m_timer2 && m_timer2->isActive()) {
            m_timer2->stop();
        }
        Q_EMIT sessionIdleExit();
    }
}

void SessionWatcher::onSessionRemoved(const QDBusObjectPath &objectPath)
{
    //如果session注销了，则结束进程
    if (objectPath.path() == sessionPath)
        exit(0);
}

void SessionWatcher::onLockScreenConfigChanged(QString strKey, QVariant value)
{
    if (strKey == KEY_IDLE_DELAY) {
        idleDelay = value.toInt();
    } else if (strKey == KEY_IDLE_LOCK) {
        m_idleLock = value.toInt();
    } else if (strKey == KEY_LOCK_TIMEOUT) {
        m_nLockTimeout = value.toInt();
    }
}
