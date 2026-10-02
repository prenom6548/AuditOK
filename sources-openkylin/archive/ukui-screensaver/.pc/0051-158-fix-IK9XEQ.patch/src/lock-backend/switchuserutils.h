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
#ifndef SWITCHUSERUTILS_H
#define SWITCHUSERUTILS_H

#include <QObject>

/**
 * @brief
 * 用户显示接口信息结构体
 */
typedef struct UserDisplayIfInfo_s
{
    QString strSessionPath; /**< 用户显示会话路径 */
    QString strSeatPath;    /**< 用户Seat路径 */
    QString strUserName;    /**< 用户名 */
} UserDisplayIfInfo;

/**
 * @brief
 *  切换用户单元
 */
class SwitchUserUtils
{
public:
    /**
     * @brief
     * 构造函数
     */
    SwitchUserUtils();

    /**
     * @brief
     * 静态接口：获取当前进程执行所在用户名
     * @return QString 用户名
     */
    static QString GetCurUserName();
    /**
     * @brief
     * 静态接口：通过用户名获取用户id
     * @param strUserName
     * @return int
     */
    static int GetUidByName(QString strUserName);
    /**
     * @brief
     * 静态接口：获取用户显示接口信息
     * @param strUserName 用户名
     * @return UserDisplayIfInfo 显示接口信息
     */
    static UserDisplayIfInfo GetUserUDII(QString strUserName);
    /**
     * @brief
     * 静态接口：切换用户会话
     * @param seatPath 用户会话所在的lightdm seat路径
     * @param toUDII 即将切换的用户lightdm显示信息
     * @return int 0：激活其他会话；1：切换到登录指定用户；2:切换到登录并选择用户；<0:切换异常
     */
    static int SwitchToUserSession(QString seatPath, UserDisplayIfInfo &toUDII);

    /**
     * @brief
     * 静态接口：切换到用户的锁屏切换用户界面
     * @return bool true: 切换成功；false：失败
     */
    static bool SwitchToUserLock();
    /**
     * @brief CanSwitchUser 当前坐席是否允许切换用户
     * @return bool true：允许，否则不允许
     */
    static bool CanSwitchUser();
};

#endif // SWITCHUSERUTILS_H
