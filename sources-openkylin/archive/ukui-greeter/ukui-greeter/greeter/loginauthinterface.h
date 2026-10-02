/*
 * Copyright (C) 2023 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef LOGIN_AUTH_INTERFACE_H
#define LOGIN_AUTH_INTERFACE_H

#include "loginplugininterface.h"

#define LOGINAUTH_CUSTOM_ID (0xFFFF)

/**
 * @brief 登录认证接口
 *
 */
class LoginAuthInterface : public LoginPluginInterface
{
public:
    /**
     * @brief 构造函数
     *
     */
    LoginAuthInterface(){}

    /**
     * @brief 析构函数
     *
     */
    virtual ~LoginAuthInterface(){}

    /**
     * @brief 获取插件类型
     * @return 插件类型
     */
    int getPluginType() { return MODULETYPE_AUTH; }

    /**
     * @brief 返回通用认证窗口
     * @return 无
     */
    virtual void returnCommAuth() {}

    /**
     * @brief 请求认证账户
     * @param strName 用户名
     * @return 无
     */
    virtual void requestAuthAccount(QString strName) = 0;

    /**
     * @brief 开始认证
     * @param nUid 用户id
     * @param strName 认证用户名
     * @return 无
     */
    virtual void startAuthenticate(int nUid, QString strName) = 0;

    /**
     * @brief 停止认证
     * @param nUid 用户id
     * @param strName 认证用户名
     * @return 无
     */
    virtual void stopAuthenticate(int nUid, QString strName) = 0;

    /**
     * @brief authenticateResult 认证结果
     * @param nResult 结果值 0，成功，其他失败
     * @param strMsg 结果描述
     */
    virtual void authenticateResult(int nResult, QString strMsg) = 0;
};

#define LoginAuthInterfaceIID "org.ukui.LoginAuthInterface"

Q_DECLARE_INTERFACE(LoginAuthInterface, LoginAuthInterfaceIID)

#endif // LOGIN_AUTH_INTERFACE_H
