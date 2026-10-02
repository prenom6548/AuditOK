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
#include "lightdmhelper.h"
#include <QDebug>
#include <QLightDM/Greeter>
#include <QFileInfo>
#include "securityuser.h"
#include "../../src/common/global_utils.h"
#include <pwd.h>
#include "proxymodel.h"
#include "../../src/dbusifs/accountshelper.h"
#include "../../src/common/definetypes.h"
#include "../../src/dbusifs/uniauthservice.h"
#include "switchuserutils.h"
#include "freedesktophelper.h"
#include <QDBusArgument>
#include <QDBusMessage>
#include <QVariantMap>
#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

LightDMHelper::LightDMHelper(QSharedPointer<AccountsHelper> accountHelper, Configuration *config, QObject *parent)
    : QLightDM::Greeter(parent)
    , m_sessionsModel(nullptr)
    , m_secUser(SecurityUser::instance())
    , m_strCurUserName("")
    , m_accountServiceHelper(accountHelper)
    , m_configuration(config)
    , m_ldmSessions(new QMap<QString, std::shared_ptr<LightDMSessionInfo>>())
    , m_mapUsers(new QMap<QString, UserInfoPtr>())
    , m_dbusIfsLDM(nullptr)
{
    // 连接到lightdm
    if (!connectToDaemonSync()) {
        qDebug() << "connect to Daemon failed";
        exit(1);
    }
    initData();
}

void LightDMHelper::initData()
{
    // 获取会话信息
    m_sessionsModel = new QLightDM::SessionsModel(QLightDM::SessionsModel::LocalSessions, this);
    updateSessionsInfo();
    setSession(defaultSessionHint());
    // 获取用户信息
    initLDMSessionsInfo();
    m_isShowManualLogin = showManualLoginHint();
    if (!hideUsersHint()) {
        initAccountsUsersInfo();
        if (hasValidUsers() == 0) {
            m_isShowManualLogin = true;
        }
    } else {
        qDebug() << "hide users, show manual";
        m_isShowManualLogin = true;
    }
    updateUsersInfo();
    if (isGreeterMode()) {
        UniAuthService::instance()->AsyncQuickLoginUser("");
    } else if (!FreedesktopHelper::getInstance(false)->NameHasOwner(UNIAUTH_DBUS_INTERFACE)) {
        FreedesktopHelper::getInstance(false)->asyncStartService(UNIAUTH_DBUS_INTERFACE);
    }
}

bool LightDMHelper::setSession(const QString &session)
{
    bool bRet = false;
    if (!m_listSessions.contains(session))
        return bRet;
    if (!session.isEmpty() && session != m_strSession) {
        m_strSession = session;
        bRet = true;
    } else if (!session.isEmpty() && session == m_strSession) {
        bRet = true;
    }
    if (bRet)
        Q_EMIT currentSessionChanged(m_strSession);
    return bRet;
}

QString LightDMHelper::session()
{
    return m_strSession;
}

QList<QString> LightDMHelper::getSessionsInfo()
{
    return m_listSessions;
}

int LightDMHelper::getLoginUserCount()
{
    QList<UserInfoPtr> userInfos = m_mapUsers->values();
    int loginUserCount = 0;
    for (auto user : userInfos) {
        if (isUserLoggined(user->name())) {
            loginUserCount += 1;
        }
    }
    return loginUserCount;
}

void LightDMHelper::startSession()
{
    if (isAuthenticated()) {
        if (UniAuthService::instance()) {
            UniAuthService::instance()->SaveLastLoginUser(m_strCurUserName);
// 三权分立不保存秒级登录用户信息
#ifdef WITHKYSEC
            if (kysec_is_disabled() || !kysec_get_3adm_status()) {
#endif
                UniAuthService::instance()->SaveQuickLoginUser(m_strCurUserName);
#ifdef WITHKYSEC
            }
#endif
        }
        Q_EMIT authenticationSucess(m_strCurUserName);
    }

    if (!startSessionSync(m_strSession)) {
        Q_EMIT startSessionFailed();
        Q_EMIT showMessage(tr("failed to start session."), QLightDM::Greeter::MessageTypeError);
    }
}

QString LightDMHelper::getCurrentUser()
{
    return m_strCurUserName;
}

bool LightDMHelper::setCurrentUser(QString strUserName)
{
    bool bRet = false;
    if (!strUserName.isEmpty() && strUserName != m_strCurUserName) {
        if (findUserByName(strUserName)) {
            m_strCurUserName = strUserName;
            bRet = true;
        } else if (struct passwd *pwinfo = getpwnam(strUserName.toLatin1().data())) {
            m_strCurUserName = strUserName;
            bRet = true;
        }
    } else if (!strUserName.isEmpty() && strUserName == m_strCurUserName) {
        bRet = true;
    }
    if (bRet) {
        Q_EMIT currentUserChanged(m_strCurUserName);
    }
    return bRet;
}

UserInfoPtr LightDMHelper::findUserByUid(uid_t id)
{
    for (auto userInfoPtr : m_mapUsers->values()) {
        if (userInfoPtr->uid() == id) {
            return userInfoPtr;
        }
    }
    return nullptr;
}

UserInfoPtr LightDMHelper::findUserByName(QString strName)
{
    for (auto userInfoPtr : m_mapUsers->values()) {
        if (userInfoPtr->name() == strName) {
            return userInfoPtr;
        }
    }
    return nullptr;
}

bool LightDMHelper::hasSameUser(const UserInfoPtr userInfoPtr)
{
    for (auto user : m_mapUsers->values()) {
        if (user->uid() == userInfoPtr->uid() && user->name() == userInfoPtr->name()) {
            return true;
        }
    }
    return false;
}

bool LightDMHelper::hasValidUsers()
{
    int count = 0;
    for (auto user : m_mapUsers->values()) {
        if (m_secUser->isSecrityUser(user->name()))
            count++;
    }
    return (bool)(count > 0);
}

bool LightDMHelper::isSameUser(UserInfoPtr userA, UserInfoPtr userB)
{
    return userA->uid() == userB->uid() && userA->fullName() == userB->fullName() && userA->name() == userB->name()
           && userA->lang() == userB->lang() && userA->isLoggedIn() == userB->isLoggedIn()
           && userA->headImage() == userB->headImage() && userA->backGround() == userB->backGround();
}

void LightDMHelper::onUsersChanged()
{
    updateUsersInfo();
    Q_EMIT usersInfoChanged();
}

void LightDMHelper::updateUsersInfo()
{
    QList<UserInfoPtr> userInfos = m_mapUsers->values();
    for (auto user : userInfos) {
        if (isUserLoggined(user->name())) {
            user->updateLoggedIn(true);
        } else {
            user->updateLoggedIn(false);
        }
    }
    if (m_isShowManualLogin && (isGreeterMode() || (SwitchUserUtils::CanSwitchUser()))) {
        std::shared_ptr<UserInfo> userInfoPtr = std::make_shared<UserInfo>();
        userInfoPtr->updateFullName(tr("Login"));
        userInfoPtr->updateName("*login");
        if (!hasSameUser(userInfoPtr)) {
            const QString path = userInfoPtr->path().isEmpty() ? userInfoPtr->name() : userInfoPtr->path();
            m_mapUsers->insert(path, userInfoPtr);
        }
    }
    if (hasGuestAccountHint()) {
        std::shared_ptr<UserInfo> userInfoPtr = std::make_shared<UserInfo>();
        userInfoPtr->updateFullName(tr("Guest"));
        userInfoPtr->updateName("*guest");
        if (!hasSameUser(userInfoPtr)) {
            const QString path = userInfoPtr->path().isEmpty() ? userInfoPtr->name() : userInfoPtr->path();
            m_mapUsers->insert(path, userInfoPtr);
        }
    }
}

void LightDMHelper::updateSessionsInfo()
{
    if (m_sessionsModel) {
        for (int i = 0; i < m_sessionsModel->rowCount(QModelIndex()); i++) {
            QString name = m_sessionsModel->index(i).data(QLightDM::SessionsModel::KeyRole).toString();
            if (!name.isEmpty()) {
                m_listSessions.append(name);
            }
        }
    }
}

QList<UserInfoPtr> LightDMHelper::getUsersInfo()
{
    return m_mapUsers->values();
}

bool LightDMHelper::isUserLoggined(const QString &strUserName)
{
    QList<std::shared_ptr<LightDMSessionInfo>> sessionList = m_ldmSessions->values();
    auto it = std::find_if(
        sessionList.begin(), sessionList.end(), [strUserName](std::shared_ptr<LightDMSessionInfo> &session) {
            return session->userName() == strUserName;
        });
    if (it != sessionList.end()) {
        return true;
    }
    return false;
}

void LightDMHelper::initLDMSessionsInfo()
{
    if (!m_dbusIfsLDM) {
        m_dbusIfsLDM
            = new QDBusInterface(DM_DBUS_SERVICE, DM_DBUS_PATH, DM_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
        connect(m_dbusIfsLDM, SIGNAL(SessionAdded(QDBusObjectPath)), this, SLOT(onLDMSessionAdded(QDBusObjectPath)));
        connect(
            m_dbusIfsLDM, SIGNAL(SessionRemoved(QDBusObjectPath)), this, SLOT(onLDMSessionRemoved(QDBusObjectPath)));

        QStringList listSessions;
        QDBusInterface ifLDMProp(DM_DBUS_SERVICE, DM_DBUS_PATH, FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        QDBusMessage ret = ifLDMProp.call("GetAll", DM_DBUS_INTERFACE);
        QList<QVariant> outArgs = ret.arguments();
        QVariant first = outArgs.at(0);
        const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
        dbusArgs.beginMap();
        while (!dbusArgs.atEnd()) {
            QString key;
            QVariant value;
            dbusArgs.beginMapEntry();
            dbusArgs >> key >> value;
            if (key == "Sessions") {
                const QDBusArgument &dbusObjPaths = value.value<QDBusArgument>();
                QDBusObjectPath path;
                dbusObjPaths.beginArray();
                while (!dbusObjPaths.atEnd()) {
                    dbusObjPaths >> path;
                    listSessions << path.path();
                }
                dbusObjPaths.endArray();
            }
            dbusArgs.endMapEntry();
        }
        dbusArgs.endMap();
        for (auto session : listSessions) {
            std::shared_ptr<LightDMSessionInfo> ldmSessionInfo(new LightDMSessionInfo(session, this));
            m_ldmSessions->insert(session, ldmSessionInfo);
        }
    }
}

void LightDMHelper::onLDMSessionAdded(QDBusObjectPath objPath)
{
    qDebug() << "Add LDM Session, path:" << objPath.path();

    if (m_ldmSessions->contains(objPath.path())) {
        return;
    }
    std::shared_ptr<LightDMSessionInfo> ldmSessionInfo(new LightDMSessionInfo(objPath.path(), this));
    m_ldmSessions->insert(objPath.path(), ldmSessionInfo);
    onUsersChanged();
}

void LightDMHelper::onLDMSessionRemoved(QDBusObjectPath objPath)
{
    if (!m_ldmSessions->contains(objPath.path())) {
        return;
    }
    const std::shared_ptr<LightDMSessionInfo> session = m_ldmSessions->value(objPath.path());
    m_ldmSessions->remove(objPath.path());
    onUsersChanged();
}

void LightDMHelper::initAccountsUsersInfo()
{
    if (m_accountServiceHelper) {
        QStringList listUsers = m_accountServiceHelper->getUserList();
        for (auto user : listUsers) {
            LocalUserInfoPtr userInfo(new LocalUserInfo(user, this));
            connect(userInfo.get(), &UserInfo::userPropChanged, this, &LightDMHelper::onUsersChanged);
            m_mapUsers->insert(user, userInfo);
        }
        connect(
            m_accountServiceHelper.get(), SIGNAL(UserAdded(QDBusObjectPath)), this, SLOT(onUserAdded(QDBusObjectPath)));
        connect(
            m_accountServiceHelper.get(),
            SIGNAL(UserRemoved(QDBusObjectPath)),
            this,
            SLOT(onUserRemoved(QDBusObjectPath)));
    }
}

void LightDMHelper::onUserAdded(QDBusObjectPath objPath)
{
    qDebug() << "Add Accounts User, path:" << objPath.path();

    if (m_mapUsers->contains(objPath.path())) {
        return;
    }
    LocalUserInfoPtr userInfo(new LocalUserInfo(objPath.path(), this));
    connect(userInfo.get(), &UserInfo::userPropChanged, this, &LightDMHelper::onUsersChanged);
    m_mapUsers->insert(objPath.path(), userInfo);
    onUsersChanged();
}

void LightDMHelper::onUserRemoved(QDBusObjectPath objPath)
{
    if (!m_mapUsers->contains(objPath.path())) {
        return;
    }
    const UserInfoPtr userInfo = m_mapUsers->value(objPath.path());
    qDebug() << "Remove Accounts User, name:" << userInfo->name();
    m_mapUsers->remove(objPath.path());
    onUsersChanged();
}

LightDMSessionInfo::LightDMSessionInfo(const QString &strPath, QObject *parent) : QObject(parent), m_strPath(strPath)
{
    initData();
    initConnections();
}

LightDMSessionInfo::~LightDMSessionInfo()
{
    if (m_propertiesChangedConnected) {
        QDBusConnection::systemBus().disconnect(
            DM_DBUS_SERVICE,
            m_strPath,
            FD_PROPERTIES_INTERFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        m_propertiesChangedConnected = false;
    }
}

void LightDMSessionInfo::initData()
{
    QDBusInterface ifaceExtra(DM_DBUS_SERVICE, m_strPath, FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
    QDBusMessage ret = ifaceExtra.call("GetAll", DM_SESSION_INTERFACE);
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    dbusArgs.beginMap();
    while (!dbusArgs.atEnd()) {
        QString key;
        QVariant value;
        dbusArgs.beginMapEntry();
        dbusArgs >> key >> value;
        if (key == "Seat") {
            m_strSeatPath = value.toString();
        } else if (key == "UserName") {
            m_strUserName = value.toString();
        }
        dbusArgs.endMapEntry();
    }
    dbusArgs.endMap();
}

void LightDMSessionInfo::initConnections()
{
    if (!m_propertiesChangedConnected) {
        QDBusConnection::systemBus().connect(
            DM_DBUS_SERVICE,
            m_strPath,
            FD_PROPERTIES_INTERFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        m_propertiesChangedConnected = true;
    }
}

void LightDMSessionInfo::onPropertiesChanged(
    const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == DM_SESSION_INTERFACE) {
        QVariantMap::const_iterator itVar = changedProperties.constBegin();
        for (; itVar != changedProperties.constEnd(); itVar++) {
            QVariant varValue = itVar.value();
            if (itVar.key() == "Seat") {
                m_strSeatPath = varValue.toString();
            } else if (itVar.key() == "UserName") {
                m_strUserName = varValue.toString();
            }
        }
    }
}

QDebug operator<<(QDebug stream, const LightDMSessionInfo &sessionInfo)
{
    stream << "[" << sessionInfo.path() << sessionInfo.userName() << sessionInfo.seatPath() << "]";
    return stream;
}
