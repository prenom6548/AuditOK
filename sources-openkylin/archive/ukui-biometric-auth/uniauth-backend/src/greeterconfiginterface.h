/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef GREETERCONFIGINTERFACE_H
#define GREETERCONFIGINTERFACE_H

#include <QObject>
#include <QtDBus>
#include <QDBusAbstractInterface>
#include "greeterconfig.h"

class GreeterConfigInterface : public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.UniauthBackend")
public:
    explicit GreeterConfigInterface(QObject *parent = nullptr);

public Q_SLOTS:
    // virutal keyboard enable
    void setVirtualKeyboardEnable(bool isEnable);
    bool getVirtualKeyboardEnable();
    bool resetVirtualKeyboardEnable();
    // user switch enable
    void setUserSwitchEnable(bool isEnable);
    bool getUserSwitchEnable();
    bool resetUserSwitchEnable();
    // network enable
    void setNetworkEnable(bool isEnable);
    bool getNetworkEnable();
    bool resetNetworkEnable();
    // power manager enable
    void setPowerManagerEnable(bool isEnable);
    bool getPowerManagerEnable();
    bool resetPowerManagerEnable();
    // lock screen datetime enable
    void setLockScreenDatetimeEnable(bool isEnable);
    bool getLockScreenDatetimeEnable(QString strUserName);
    bool resetLockScreenDatetimeEnable();
    // screensaver datetime enable
    void setScreensaverDatetimeEnable(bool isEnable);
    bool getScreensaverDatetimeEnable(QString strUserName);
    bool resetScreensaverDatetimeEnable();
    // login displayed user count
    void setLoginDisplayedUserCount(int count);
    int getLoginDisplayedUserCount();
    bool resetLoginDisplayedUserCount();
    // default select manual login
    bool getSelectManualLoginDefault();

    /**
     * @brief printConfig 以json格式打印所有有效的、默认的、修改的配置项
     * @param nType 1 默认配置；2 修改的配置；其他 所有有效的配置
     * @return json字符串
     */
    QString printConfig(int nType, QString strUserName);

Q_SIGNALS:
    void changed(const QString &group, const QString &key);
    void userConfigChanged(const QString &group, const QString &key, const QString &strUserName);

    void virtualKeyboardEnableChanged(bool isEnable);
    void userSwitchEnableChanged(bool isEnable);
    void networkEnableChanged(bool isEnable);
    void powerManagerEnableChanged(bool isEnable);

    void lockScreenDatetimeEnableChanged(bool isEnable, QString strUserName);
    void screensaverDatetimeChanged(bool isEnable, QString strUserName);
    void loginDisplayedUserCountChanged(int count);
private Q_SLOTS:
    void onChanged(const QString &group, const QString &key);
    void onChangedForUser(const QString &group, const QString &key, const QString &strUserName);

private:
    bool checkEnviron(int pid);
    bool ensureGlobalConfigWriteAllowed();
    bool ensureGlobalConfigReadAllowed();
    bool ensureUserConfigAccessAllowed(const QString &targetUserName);
    QString getUserNameFromCaller();

private:
    GreeterConfig *m_config = nullptr;
};

#endif // GREETERCONFIGINTERFACE_H
