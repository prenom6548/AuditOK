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
#ifndef LIGHTDMHELPER_H
#define LIGHTDMHELPER_H

#include <QObject>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>
#include <QList>
#include <QModelIndex>
#include <QDBusInterface>
#include "../userinfo.h"
#include "../../src/common/configuration.h"
#include <QSharedPointer>

class SecurityUser;
class KyUsersModel;
class AccountsHelper;
class LightDMSessionInfo;
/**
 * @brief lightdm工具类
 *
 */
class LightDMHelper : public QLightDM::Greeter
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit LightDMHelper(QSharedPointer<AccountsHelper> accountHelper, Configuration *config, QObject *parent = nullptr);

public:
    /**
     * @brief 设置会话
     *
     * @param session 会话名称
     * @return bool true 设置成功，其他失败
     */
    bool setSession(const QString &session);
    /**
     * @brief 会话
     *
     * @return QString 会话名称
     */
    QString session();
    /**
     * @brief 根据用户id查找用户信息
     *
     * @param id 用户id
     * @return UserInfoPtr 用户信息指针
     */
    UserInfoPtr findUserByUid(uid_t id);
    /**
     * @brief 根据用户名查找用户信息
     *
     * @param strName 用户名
     * @return UserInfoPtr 用户信息指针
     */
    UserInfoPtr findUserByName(QString strName);
    /**
     * @brief 是否存在相同用户信息
     *
     * @param userInfoPtr 用户信息
     * @return bool true 有，false 无
     */
    bool hasSameUser(const UserInfoPtr userInfoPtr);
    /**
     * @brief 获取用户信息列表
     *
     * @return QList<UserInfoPtr> 信息列表
     */
    QList<UserInfoPtr> getUsersInfo();

    QString getCurrentUser();

    bool setCurrentUser(QString strUserName);

    QList<QString> getSessionsInfo();

    int getLoginUserCount();

public Q_SLOTS:
    /**
     * @brief 启动会话
     *
     */
    void startSession();
    /**
     * @brief 用户更新
     */
    void onUsersChanged();

    void onLDMSessionAdded(QDBusObjectPath objPath);

    void onLDMSessionRemoved(QDBusObjectPath objPath);

    void onUserAdded(QDBusObjectPath objPath);

    void onUserRemoved(QDBusObjectPath objPath);

Q_SIGNALS:
    /**
     * @brief 认证成功
     *
     * @param userName 用户名
     */
    void authenticationSucess(QString userName);
    /**
     * @brief 启动会话失败
     *
     */
    void startSessionFailed();
    /**
     * @brief 用户信息更新
     *
     */
    void usersInfoChanged();

    void currentUserChanged(QString strUserName);

    void currentSessionChanged(QString strSession);

private:
    /**
     * @brief 初始化数据
     *
     */
    void initData();
    /**
     * @brief 是否有有效用户
     *
     * @return bool true 有，false 无
     */
    bool hasValidUsers();
    /**
     * @brief 是否为同一个用户信息
     *
     * @param UserInfoPtr 用户A
     * @param UserInfoPtr 用户B
     * @return bool true 相同，false 不同
     */
    bool isSameUser(UserInfoPtr, UserInfoPtr);
    /**
     * @brief 更新用户信息
     *
     */
    void updateUsersInfo();

    void updateSessionsInfo();

    void initLDMSessionsInfo();

    bool isUserLoggined(const QString &strUserName);

    void initAccountsUsersInfo();

private:
    bool m_isShowManualLogin;                 /**< 是否显示手动登录 */
    QString m_strSession;                     /**< 当前会话 */
    QString m_strCurUserName;                 /**< 当前选择用户 */
    SecurityUser *m_secUser = nullptr;        /**< 安全用户信息指针 */
    QLightDM::SessionsModel *m_sessionsModel; /**< lightdm 会话数据指针 */
    QList<QString> m_listSessions;            /**< 会话列表 */
    QMap<QString, UserInfoPtr> *m_mapUsers;   /**< accounts 用户信息列表指针 */
    QSharedPointer<AccountsHelper> m_accountServiceHelper = nullptr;
    QMap<QString, std::shared_ptr<LightDMSessionInfo>> *m_ldmSessions;
    QDBusInterface *m_dbusIfsLDM;
    Configuration *m_configuration = nullptr;
};

class LightDMSessionInfo : public QObject
{
    Q_OBJECT
public:
    explicit LightDMSessionInfo(const QString &strPath, QObject *parent = nullptr);

    virtual ~LightDMSessionInfo();

    inline QString path() const
    {
        return m_strPath;
    }
    inline QString userName() const
    {
        return m_strUserName;
    }
    inline QString seatPath() const
    {
        return m_strSeatPath;
    }

private:
    void initData();
    void initConnections();

private Q_SLOTS:
    void onPropertiesChanged(const QString &, const QVariantMap &, const QStringList &);

private:
    QString m_strPath;
    bool m_propertiesChangedConnected = false;
    QString m_strSeatPath;
    QString m_strUserName;
};

QDebug operator<<(QDebug stream, const LightDMSessionInfo &userInfo);

#endif // LIGHTDMHELPER_H
