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
#ifndef GREETERCONFIG_H
#define GREETERCONFIG_H

#include <QtDBus>
#include <QDBusAbstractInterface>

class GreeterConfig : public QDBusAbstractInterface
{
    Q_OBJECT
private:
    explicit GreeterConfig(QObject *parent = nullptr);

public:
    /**
     * @brief 单实例
     *
     * @param parent 父指针
     * @return GreeterConfig 单实例对象指针
     */
    static GreeterConfig *instance(QObject *parent = nullptr);

public Q_SLOTS:
    // 设置/获取虚拟键盘使能
    void setVirtualKeyboardEnable(bool isEnable);
    bool getVirtualKeyboardEnable();
    bool resetVirtualKeyboardEnable();
    // 设置/获取用户切换使能
    void setUserSwitchEnable(bool isEnable);
    bool getUserSwitchEnable();
    bool resetUserSwitchEnable();
    // 设置/获取网络使能
    void setNetworkEnable(bool isEnable);
    bool getNetworkEnable();
    bool resetNetworkEnable();
    // 设置/获取电源管理使能
    void setPowerManagerEnable(bool isEnable);
    bool getPowerManagerEnable();
    bool resetPowerManagerEnable();
    // 锁屏日期时间使能
    void setLockScreenDatetimeEnable(bool isEnable);
    bool getLockScreenDatetimeEnable(QString strUserName);
    bool resetLockScreenDatetimeEnable();
    // 屏保时间日期使能
    void setScreensaverDatetimeEnable(bool isEnable);
    bool getScreensaverDatetimeEnable(QString strUserName);
    bool resetScreensaverDatetimeEnable();
    // 是否默认选择手动输用户名登录
    bool getSelectManualLoginDefault();

Q_SIGNALS:
    //功能使能状态改变
    void virtualKeyboardEnableChanged(bool isEnable);
    void userSwitchEnableChanged(bool isEnable);
    void networkEnableChanged(bool isEnable);
    void powerManagerEnableChanged(bool isEnable);
    void lockScreenDatetimeEnableChanged(bool isEnable, QString strUserName);
    void screensaverDatetimeChanged(bool isEnable, QString strUserName);

    //开关状态改变
    void changed(const QString &group, const QString &key);

private:
    static std::unique_ptr<GreeterConfig> m_instance; /** 统一认证配置服务单实例指针 */
    static std::once_flag m_initFlag;
};

#endif // GREETERCONFIG_H
