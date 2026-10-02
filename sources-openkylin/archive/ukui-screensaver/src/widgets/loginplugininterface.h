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
#ifndef LOGINPLUGININTERFACE_H
#define LOGINPLUGININTERFACE_H

#include <QString>
#include <QWidget>
#include <QStringLiteral>

#define LOGINPLUGIN_VERSION "1.0.0"

/**
 * @brief 登录插件接口类
 *
 */
class LoginPluginInterface
{
public:
    /**
     * @brief 插件模块类型枚举：认证、工具、通知
     *
     */
    enum ModuleType
    {
        MODULETYPE_AUTH,
        MODULETYPE_TOOL,
        MODULETYPE_WINDOW,
        MODULETYPE_NOTIFY,
        MODULETYPE_MAX
    };
    /**
     * @brief 登录插件环境：登录、锁屏
     *
     */
    enum LoginPluginEnv
    {
        LOGINPLUGINENV_LOGIN,      // 登录
        LOGINPLUGINENV_LOCKSCREEN, // 锁屏
    };
    /**
     * @brief 构造函数
     *
     */
    LoginPluginInterface() {}
    /**
     * @brief 析构函数
     *
     */
    virtual ~LoginPluginInterface() {}

    /**
     * @brief 获取插件展示名称
     * @return 插件展示名称
     */
    virtual QString getPluginName() = 0;

    /**
     * @brief 获取插件唯一名称
     * @return 插件唯一名称
     */
    virtual QString name() = 0;

    /**
     * @brief 获取插件类型
     * @return 插件类型
     */
    virtual int getPluginType() = 0;

    /**
     * @brief 获取插件图标
     * @return 插件图标路径（绝对路径或主题图标名称）
     */
    virtual QString icon() = 0;

    /**
     * @brief 获取插件主窗口句柄
     * @param 父窗口句柄
     * @return 插件主窗口句柄
     */
    virtual QWidget *getPluginMainWnd(QWidget *parent = nullptr) = 0;

    /**
     * @brief 插件是否需要加载
     * @return true 加载；false 不加载
     */
    virtual bool needLoad() = 0;

    /**
     * @brief 设置主窗口背景
     * @param 背景图路径
     * @return 无
     */
    virtual void setBackground(QString strPicPath) {}

    /**
     * @brief 获取登录插件环境
     * @return LoginPluginEnv
     */
    virtual int getLoginPluginEnv() = 0;

    /**
     * @brief 获取插件接口版本信息
     * @return version
     */
    QString getVersion()
    {
        return LOGINPLUGIN_VERSION;
    }

    /**
     * @brief onMessage 主程序向插件传递消息数据
     * @param strMsgJson json格式数据
     * @return json格式数据
     */
    virtual QString onMessage(QString strMsgJson) = 0;

    /**
     * @brief onRequest 插件向主程序请求数据
     * @param strReqJson json格式数据
     * @return json格式数据
     */
    virtual QString onRequest(QString strReqJson) = 0;
};

#define LoginPluginInterfaceIID "org.ukui.LoginPluginInterface"

Q_DECLARE_INTERFACE(LoginPluginInterface, LoginPluginInterfaceIID)

#endif // LOGINPLUGININTERFACE_H
