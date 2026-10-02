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

#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include <QCoreApplication>
#include <QObject>
#include <QtDBus>
#include <QDBusAbstractInterface>
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QVariant>

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
 * @brief 登录服务接口类
 *
 */
class ServiceInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.GreeterDaemon")
    Q_PROPERTY(bool showLoginPrompt READ getShowLoginPrompt)
    Q_PROPERTY(bool hideTitle READ getHideTitle)
    Q_PROPERTY(QString promptTitle READ getPromptTitle)
    Q_PROPERTY(QString promptText READ getPromptText)
    Q_PROPERTY(QString promptTextFilePath READ getPromptTextFilePath)
public:
    /**
     * @brief 构造函数
     *
     */
    ServiceInterface();
    bool getShowLoginPrompt();
    bool getHideTitle();
    QString getPromptTitle();
    QString getPromptText();
    QString getPromptTextFilePath();

private:
    QVariant getValue(const QString &key);
    void setValue(const QString &key, const QVariant &value);

public slots:
    /**
     * @brief 一键切换并登录用户
     *
     * @param strUserName 用户名
     * @return int 0 成功，否则失败
     */
    int SwitchToUser(QString strUserName);
    /**
     * @brief 切换到登录用户
     *
     * @param strUserName 用户名
     * @return int 0 成功，否则失败
     */
    int SwitchToGreeterUser(QString strUserName);
    /**
     * @brief 获取将要登录的用户名
     *
     * @return WillLoginUserInfo 用户信息
     */
    WillLoginUserInfo getWillSwitchUser();
    /**
     * @brief 设置是否打开登录特别提示 
     *
     * @param prompt 开关状态 
     * @return 
     */
    void setShowLoginPrompt(bool prompt);
    /**
     * @brief 设置特别提示标题
     *
     * @param title 标题 
     * @return 
     */
    void setPromptTitle(QString title);
    /**
     * @brief 设置特别提示文本内容
     *
     * @param text 文本内容 
     * @return 
     */
    void setPromptText(QString text);
    /**
     * @brief 设置特别提示文本路径
     *
     * @param filePath 文本文件路径 
     * @return 
     */
    void setPromptTextFilePath(QString filePath);
    /**
     * @brief 获取是否需要展示提示界面 
     *
     * @return bool  
     */
    bool getGreeterShowPrompt();
    /**
     * @brief 设置隐藏标题 
     * @return bool  
     */
    void setHideTitle(bool isHide);
signals:
    /**
     * @brief 将要登录的用户名改变
     *
     * @param willLoginUser 新用户信息（需对空用户名异常处理）
     */
    void userChanged(WillLoginUserInfo willLoginUser);

private:
    WillLoginUserInfo m_willLoginUser; /** 将登录的用户信息 */
    QSettings *settings = nullptr;
};

#endif // SERVICEINTERFACE_H
