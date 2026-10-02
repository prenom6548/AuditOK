/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __NAVIGAITON_UKCC_H__
#define __NAVIGATION_UKCC_H__

#include <QObject>
#include <QDBusContext>
#include <QProcess>
#include <QFile>

#include "process_pool.h"


#define Start_Process(a, b) \
    if (m_processPool->startProcess(a, b)) {   \
        return;    \
    }   \
    if (calledFromDBus()) { \
    sendErrorReply(QDBusError::ErrorType::NoReply, QStringLiteral("The app start error.")); \
    }


#define Peek_app(a) \
        if (false == a) {   \
          sendErrorReply(QDBusError::ErrorType::NotSupported, QStringLiteral("The app does not install"));\
          return;   \
        }

const QString c_Ukcc = "ukui-control-center";
const QString c_Kyom = "kylin-os-manager";
const QString Uksm = "ukui-system-monitor";
const QString Ukclock = "ukui-clock";
const QString Ukmusic = "kylin-music";
const QString Ukvideo = "kylin-video";



#define Peek_Ukcc() Peek_app(m_isUkccInstall)
#define Peek_Kyom() Peek_app(m_isUkccInstall)
#define Peek_Uksm() Peek_app(m_isUkccInstall)
#define Peek_Ukclock() Peek_app(m_isUkccInstall)
#define Peek_Ukmusic() Peek_app(m_isUkccInstall)
#define Peek_Ukvideo() Peek_app(m_isUkccInstall)

class NavigationUKcc: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Navigations")

public:
    explicit NavigationUKcc(QObject *parent = nullptr);
    ~NavigationUKcc();

public Q_SLOTS:
    /**
     * @brief OpenUkccUserInfo 修改用户密码
     */
    void OpenUkccUserInfo();

    /**
     * @brief OpenUkccAudio 选择输入输出设备
     */
    void OpenUkccAudio();

    /**
     * @brief OpenUkccPower 打开电源管理界面
     */
    void OpenUkccPower();

    /**
     * @brief OpenUkccBluetooth 打开蓝牙界面
     */
    void OpenUkccBluetooth();

    /**
     * @brief OpenUkccDate  自定义时区时间
     */
    void OpenUkccDate();

    /**
        * @brief OpenUkccAutostart 管理开机自启动程序
        */
    void OpenUkccAutostart();

    /**
        * @brief OpenUkccLoginOptions 配置登录选项（如自动登录）
        */
    void OpenUkccLoginOptions();

    /**
        * @brief OpenUkccTouchpad 配置触摸板设置
        */
    void OpenUkccTouchpad();

    /**
        * @brief OpenUkccTouchscreen 配置触摸屏设置
        */
    void OpenUkccTouchscreen();

    /**
        * @brief OpenUkccNetconnect 管理有线网络连接
        */
    void OpenUkccNetconnect();

    /**
        * @brief OpenUkccWlanconnect 管理无线网络连接
        */
    void OpenUkccWlanconnect();

    /**
        * @brief OpenUkccTheme 打开主题设置页面
        */
    void OpenUkccTheme();

    /**
        * @brief OpenUkccScreensaver 配置屏幕保护程序
        */
    void OpenUkccScreensaver();

    /**
        * @brief OpenUkccUpgrade 检查系统更新
        */
    void OpenUkccUpgrade();

    /**
        * @brief OpenUkccSecurityCenter 打开安全中心
        */
    void OpenUkccSecurityCenter();

    /**
        * @brief OpenUkccDisplay 配置显示器设置
        */
    void OpenUkccDisplay();

    /**
        * @brief OpenUkccScreenlock 锁屏个性化设置
        */
    void OpenUkccScreenlock();

    /**
        * @brief OpenUkccNotice 设置系统通知
        */
    void OpenUkccNotice();

    /**
        * @brief OpenUkccVino 配置远程桌面（Vino）
        */
    void OpenUkccVino();

    /**
        * @brief OpenUkccPrinter 管理打印机设置
        */
    void OpenUkccPrinter();

    /**
        * @brief OpenUkccShortcut 配置系统快捷键
        */
    void OpenUkccShortcut();

    /**
        * @brief OpenUkccKeyboard 配置键盘布局和输入法
        */
    void OpenUkccKeyboard();

    /**
        * @brief OpenUkccProxy 配置网络代理
        */
    void OpenUkccProxy();

    /**
        * @brief OpenUkccMobileHotspot 配置移动热点
        */
    void OpenUkccMobileHotspot();

    /**
        * @brief OpenUkccArea 设置系统区域和语言
        */
    void OpenUkccArea();

    /**
        * @brief OpenUkccBackup 打开备份还原工具
        */
    void OpenUkccBackup();

    /**
     * @brief OpenSystemConfigInfo 查询系统配置信息
     */
    void OpenKyomDeviceManager();

    /**
     * @brief OpenUsmAbout 查询系统状态
     */
    void OpenUksmAbout();

    /**
     * @brief OpenUkClock 闹钟设置
     */
    void OpenUkClock();
    /**
     * @brief OpenUsmAbout 多媒体-音乐播放
     */
    void OpenKyMusic();

    /**
     * @brief OpenUsmAbout 多媒体-视频播放
     */
    void OpenKyVideo();

private:
    void checkAppInstall();
    bool checkAppExist(QString appPath);
private:
    bool m_isUkccInstall = true;
    bool m_isKyomInstall = true;
    bool m_isUksmInstall = true;
    bool m_isUkclockInstall = true;
    bool m_isUkmusicInstall = true;
    bool m_isUkvideroInstall = true;

    const QString c_Ukcc = "ukui-control-center";
    const QString c_Kyom = "kylin-os-manager";
    const QString c_Uksm = "ukui-system-monitor";
    const QString c_Ukclock = "ukui-clock";
    const QString c_Ukmusic = "kylin-music";
    const QString c_Ukvideo = "kylin-video";

    ProcessPool *m_processPool;
};

#endif  //#define __NAVIGATION_H__

