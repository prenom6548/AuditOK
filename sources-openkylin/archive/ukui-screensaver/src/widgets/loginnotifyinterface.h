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
#ifndef LOGIN_NOTIFY_INTERFACE_H
#define LOGIN_NOTIFY_INTERFACE_H

#include "loginplugininterface.h"

/**
 * @brief 通知类窗口接口
 *
 */
class LoginNotifyInterface : public LoginPluginInterface
{
public:
    /**
     * @brief 构造函数
     *
     */
    LoginNotifyInterface() {}

    /**
     * @brief 析构函数
     *
     */
    virtual ~LoginNotifyInterface() {}

    /**
     * @brief 获取插件类型
     * @return 插件类型
     */
    int getPluginType()
    {
        return MODULETYPE_NOTIFY;
    }
};

#define LoginNotifyInterfaceIID "org.ukui.LoginNotifyInterface"

Q_DECLARE_INTERFACE(LoginNotifyInterface, LoginNotifyInterfaceIID)

#endif // LOGIN_NOTIFY_INTERFACE_H
