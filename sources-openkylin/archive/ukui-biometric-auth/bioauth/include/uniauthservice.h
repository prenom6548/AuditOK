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
#ifndef UNIAUTH_SERVICE_H
#define UNIAUTH_SERVICE_H

#include <QtDBus>
#include <QDBusAbstractInterface>

enum authEnableType {
    ENABLETYPE_BIO,         // 全局总使能
    ENABLETYPE_SAVER,       // 锁屏
    ENABLETYPE_GREETER,     // 登录
    ENABLETYPE_POLKIT,      // 授权
    ENABLETYPE_SU,          // 暂保留
    ENABLETYPE_SUDO,        // 暂保留
    ENABLETYPE_LOGIN,       // 暂保留
};

class UniAuthService : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit UniAuthService(QObject *parent = nullptr);

public Q_SLOTS:
    // 设置默认设备
    void setDefaultDevice(int bioDevType, QString deviceName);
    // 获取默认设备
    QString getDefaultDevice(QString userName, int bioDevType);
    // 获取所有默认设备
    QStringList getAllDefaultDevice(QString userName);
    //生物特征开关接口
    bool getBioAuthStatus(QString userName, int bioAuthType);
    void setBioAuthStatus(int bioAuthType, bool status);
    // 获取最大失败次数
    int getMaxFailedTimes();
    // 获取是否使能微信扫码登录
    bool getQRCodeEnable();
    // 获取是否双认证
    bool getDoubleAuth();
    // 获取用户绑定
    bool getUserBind();
    // 获取是否在控制面板显示
    bool getIsShownInControlCenter();
    // 获取是否使用第一个设备
    bool getUseFirstDevice();
    // 获取是否隐藏切换按钮
    bool getHiddenSwitchButton();
    // 获取正在认证的应用
    QStringList getAuthingApp();
    // 获取应用认证的驱动
    int getAuthingAppDrivd(QString appName);

    // 获取优先级最高的认证类型
    int getDefaultAuthType(QString userName);
public:
    bool isActivatable();
    void recordAuthDrive(QString appName,int drvid,bool insert);

Q_SIGNALS:
    //默认设备改变
    void defaultDeviceChanged(QString userName, int bioDevType, QString deviceName);
    //开关状态改变
    void bioAuthStatusChanged(QString userName, int type, bool status);

private:
    bool m_isActivatable;
};

#endif // UNIAUTH_SERVICE_H
