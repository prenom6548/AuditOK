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
#ifndef GREETERSERVICE_H
#define GREETERSERVICE_H

#include <QtDBus>
#include <QDBusAbstractInterface>

/**
 * @brief 用户信息
 */
struct WillLoginUserInfo
{
    QString strUserName = "";
    bool isOneKeyLogin = false;
};

QDBusArgument &operator <<(QDBusArgument &arg, const WillLoginUserInfo &willLoginUserInfo);
const QDBusArgument &operator >>(const QDBusArgument &arg, WillLoginUserInfo &willLoginUserInfo);
QDebug operator <<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo);

Q_DECLARE_METATYPE(WillLoginUserInfo)

/**
 * @brief 登录服务客户端类
 *
 */
class GreeterService : public QDBusAbstractInterface
{
    Q_OBJECT
private:
    /**
     * @brief 构造函数
     *
     * @param parent 父指针
     */
    explicit GreeterService(QObject *parent = nullptr);

public:
    /**
     * @brief 单实例
     *
     * @param parent 父指针
     * @return GreeterService 单实例对象指针
     */
    static GreeterService *instance(QObject *parent = nullptr);
    /**
     * @brief 用户名是否有效
     *
     * @param strUserName 用户名
     * @return bool true 有效，否则 无效
     */
    bool isUserNameValid(QString strUserName);

public Q_SLOTS:
    /**
     * @brief 一键切换并登录用户
     *
     * @param strUserName 用户名
     */
    void SwitchToUser(QString strUserName);
    /**
     * @brief 切换到登录用户
     *
     * @param strUserName 用户名
     */
    void SwitchToGreeterUser(QString strUserName);
    /**
     * @brief 将要登录的用户信息
     *
     * @return WillLoginUserInfo 用户信息
     */
    WillLoginUserInfo getWillSwitchUser();

Q_SIGNALS:
    /**
     * @brief 将要登录的用户名改变
     *
     * @param strUserName 新用户名（需对空串异常处理）
     * @param isOneKeyLogin 是否需要一键登录
     */
    void userChanged(WillLoginUserInfo willLoginUser);

private:
    static GreeterService *m_instance; /** 登录服务单实例指针 */
};

#endif // GREETERSERVICE_H
