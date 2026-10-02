/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef UKCCCOMMON_H
#define UKCCCOMMON_H

#include <QRect>
#include <QCursor>
#include <QObject>
#include <QWidget>
#include <QSettings>
#include <QGSettings/QGSettings>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QVariantMap>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>
#include <kysdk/kysdk-base/libkydiagnostics.h>

#include "libukcc_global.h"

namespace ukcc
{
/**
 * @brief 提供判断、获取信息等功能性接口
 *
 */
class LIBUKCC_EXPORT UkccCommon : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief
     *
     */
    explicit UkccCommon();
    /**
     * @brief
     *
     */
    ~UkccCommon();

public:
    /**
     * @brief 将 widget 移动到屏幕中间
     *
     * @param widget 需要移动的窗口
     */
    static void centerToScreen(QWidget *widget);
    /**
     * @brief 获取鼠标所在屏幕大小
     *
     * @return 鼠标所在屏幕大小
     */
    static QRect sizeOnCursor();
    /**
     * @brief 根据 ukui-control-center-security-config.json 获取插件隐藏状态
     *
     * @return QVariantMap 插件隐藏状态列表
     */
    static QVariantMap getModuleHideStatus();
    /**
     * @brief 获取 CPU 信息
     *
     * @return QString
     */
    static QString getCpuInfo();
    /**
     * @brief 获取 CPU 架构
     *
     * @return QString CPU 架构
     */
    static QString getCpuArchitecture();

    /**
     * @brief 是否存在电池
     *
     * @return bool 是：true，否：false
     */
    static bool isExitBattery();
    /**
     * @brief set kwin cursorSize
     *
     * @param size 大小
     */
    static void setKwinMouseSize(int size);
    /**
     * @brief 是否 wayland
     *
     * @return bool 是：true，否：false
     */
    static bool isWayland();
    /**
     * @brief 是否 openkylin
     *
     * @return bool 是：true，否：false
     */
    static bool isOpenkylin();
    /**
     * @brief 是否 22.04 社区版
     *
     * @return bool 是：true，否：false
     */
    static bool isCommunity();
    /**
     * @brief 是否 mavis 平板
     *
     * @return bool 是：true，否：false
     */
    static bool isTablet();
    /**
     * @brief 是否 v10sp1-zyj
     *
     * @return bool 是：true，否：false
     */
    static bool isZJY();
    /**
     * @brief 获取主机名
     *
     * @return QString hostname
     */
    static QString getHostName();
    /**
     * @brief 埋点
     *
     * @param pluginName 插件名
     * @param settingsName 设置名
     * @param action 操作
     * @param value 设置的值
     * @return bool 是否埋点成功
     */
    static bool buriedSettings(QString pluginName, QString settingsName, QString action, QString value = nullptr);
    /**
     * @brief bool 转换为 string
     *
     * @param b true or false
     * @return QString "true" or "false"
     */
    static QString boolToString(bool b);

    /**
     * @brief 获取控制面板版本
     *
     * @return QString 版本号
     */
    static QString getUkccVersion();

    /**
     * @brief
     *
     * @return bool 是：true，否：false
     */
    static bool checkCompositorRunning();

    /**
     * @brief 获取system-product-name
     *
     * @return QString system-product-name
     */
    static QString getProductName();

    /**
     * @brief 是否支持特效模式
     *
     * @return bool 是：true，否：false
     */
    static bool isSupportsEffect();

    /**
     * @brief 获取GPU
     *
     * @return QString gpuName
     */
    static QString getGpuName();

    /**
     * @brief 判断是否华为
     *
     * @return bool 是：true，否：false
     */
    static bool isHuawei();

    /**
     * @brief 判断用户是否为域用户, 和/etc/passwd文件中用户做对比
     * @param username
     * @return bool 域用户：true，非域用户：false
     */
    static bool isDomainUser(const char* username);
};

}
#endif // UKCCCOMMON_H
