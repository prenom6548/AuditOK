/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "account-information.h"
#include "app-manager.h"
#include "dbus-connector.h"

#include <QDebug>
#include <QProcess>
#include <QDBusReply>

#define DEFAULT_USER_ICON_FILE        ":/icon/default-community-image.png"
#define KYLIN_POWER_NORMAL_PATH_MAJOR ":/icon/pad_mainpower.svg"

#define ADMINISTRATORS 1
#define STANDARD_USER  0

#define POWER_BUS_NAME "org.ukui.upower"
#define POWER_BUS_PATH "/upower/BatteryInfo"
#define POWER_BUS_INTERFACE "org.ukui.upower.battery"

using namespace UkuiShortcut;
using namespace UkuiQuick;

AccountInformation::AccountInformation(QObject *parent) : QObject(parent)
{
    initMemberVariable();
    registeredAccountsDbus();
    initUserInfo();
}

AccountInformation::~AccountInformation()
= default;

void AccountInformation::initMemberVariable()
{
    m_administrator = QObject::tr("Administrator");
    m_standardUser = QObject::tr("Standard user");
}

void AccountInformation::registeredAccountsDbus()
{
    m_systemUserIFace = new QDBusInterface("org.freedesktop.Accounts",
                                           "/org/freedesktop/Accounts",
                                           "org.freedesktop.Accounts",
                                           QDBusConnection::systemBus(), this);
}

void AccountInformation::initUserInfo()
{
    QStringList objectPaths = getUserObjectPath();
    UserInformation user;

    for (const QString& objectPath : objectPaths){
        user = GetUserInformation(objectPath);
        if(user.current && user.isLogged) {
            m_currentUserInfo = user;
            m_currentUserIFace = new QDBusInterface("org.freedesktop.Accounts",
                                                    objectPath,
                                                    "org.freedesktop.DBus.Properties",
                                                    QDBusConnection::systemBus(), this);

            m_currentUserIFace->connection().connect("org.freedesktop.Accounts", objectPath,
                                                     "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                                     this, SLOT(currentAccountUpdateSlot(QString, QMap<QString, QVariant>, QStringList)));

            break;
        }
    }
}

QStringList AccountInformation::getUserObjectPath()
{
    QStringList users;

    QDBusReply<QList<QDBusObjectPath> > reply = m_systemUserIFace->call("ListCachedUsers");
    if (reply.isValid()) {
        for (const QDBusObjectPath& op : reply.value()) {
            users << op.path();
        }
    }

    return users;
}

UserInformation AccountInformation::GetUserInformation(const QString& objectPath)
{
    UserInformation user;

    //默认值
    user.current = false;
    user.isLogged = false;
    user.autologin = false;

    QDBusInterface userInterface("org.freedesktop.Accounts",
                                 objectPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > reply = userInterface.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        user.realName = propertyMap.find("RealName").value().toString();
        user.accountType = propertyMap.find("AccountType").value().toInt();
        user.iconFile = propertyMap.find("IconFile").value().toString();
        user.passwdType = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
        user.autologin = false;
        user.objPath = objectPath;

        if (user.username == QString(qgetenv("USER"))) {
            user.current = true;
            user.isLogged = true;
        }
    } else {
        qDebug() << "GetUserInformation: reply failed";
    }

    return user;
}

void AccountInformation::currentAccountUpdateSlot(const QString& property, const QMap<QString, QVariant>& propertyMap,
                                                  const QStringList& propertyList)
{
    Q_UNUSED(property)
    Q_UNUSED(propertyList)
    QStringList keys = propertyMap.keys();

    if (keys.contains("IconFile")) {
        m_currentUserInfo.iconFile = propertyMap.value("IconFile").toString();
    }

    if (keys.contains("UserName")) {
        m_currentUserInfo.username = propertyMap.value("UserName").toString();
    }

    if (keys.contains("RealName")) {
        m_currentUserInfo.realName = propertyMap.value("RealName").toString();
    }

    if (keys.contains("AccountType")) {
        m_currentUserInfo.accountType = propertyMap.value("AccountType").toInt();
    }

    Q_EMIT userInfoChanged();
}

QString AccountInformation::getUsername()
{
    return m_currentUserInfo.username;
}

QString AccountInformation::getRealName()
{
    return m_currentUserInfo.realName;
}

QString AccountInformation::getIconFile()
{
    if (m_currentUserInfo.iconFile.isEmpty()) {
        return {DEFAULT_USER_ICON_FILE};
    }

    return m_currentUserInfo.iconFile;
}

QString AccountInformation::getAccountType()
{
    if (m_currentUserInfo.accountType == ADMINISTRATORS) {
        return m_administrator;
    }

    if (m_currentUserInfo.accountType == STANDARD_USER) {
        return m_standardUser;
    }

    return "";
}

void AccountInformation::openUserCenter()
{
    QStringList args;
    args.append("-m");
    args.append("Userinfo");
    Sidebar::AppManager::getInstance(this)->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", args, "ukui-control-center");
}

void AccountInformation::openPeonyUser()
{
    Sidebar::AppManager::getInstance(this)->launchAppWithArguments("/usr/share/applications/peony.desktop", QStringList(), "peony ～/");
}

void AccountInformation::openControlCenter()
{
    QStringList args;
    args.append("-m");
    args.append("Boot");
    Sidebar::AppManager::getInstance(this)->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", args, "ukui-control-center");
}

void AccountInformation::openPowerButton()
{
    Sidebar::AppManager::getInstance(this)->launchApp("/usr/share/applications/ukui-session-tools.desktop", "ukui-session-tools");
}

PowerButton::PowerButton(QObject *parent) : QObject(parent)
{
    initPowerBatteryIface();
}

PowerButton::~PowerButton()
{
    if (m_powerBatteryIface) {
        delete m_powerBatteryIface;
        m_powerBatteryIface = nullptr;
    }

    if (!m_powerServiceWatcher) {
        delete m_powerServiceWatcher;
        m_powerServiceWatcher = nullptr;
    }
}

bool PowerButton::isEnable()
{
    return m_isEnable;
}

QString PowerButton::icon()
{
    return m_icon;
}

QString PowerButton::tooltip()
{
    return m_tooltip;
}

QString PowerButton::percentage()
{
    return m_percentage;
}

bool PowerButton::isPresent()
{
    return m_isPresent;
}

void PowerButton::initPowerBatteryIface()
{
    DbusConnector* dbusConnector = new DbusConnector(POWER_BUS_NAME,
                                                     POWER_BUS_PATH,
                                                     POWER_BUS_INTERFACE,
                                                     QDBusConnection::sessionBus());
    connect(dbusConnector, &DbusConnector::ready, this, &PowerButton::onConnectorReady);
    dbusConnector->start();
}

void PowerButton::getBatteryInfo()
{
    if (!m_powerBatteryIface) return;

    m_iconNeedRefresh = true;
    auto iconWatcher = new QDBusPendingCallWatcher(m_powerBatteryIface->asyncCall(QStringLiteral("IconName")), this);
    connect(iconWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QString> reply = *self;
        if (self) {
            if (reply.isError()) {
                qDebug() << "PowerBatteryIface call IconName error" << reply.error().message();
            } else {
                if (m_iconNeedRefresh) {
                    onBatteryIconChanged(reply.value());
                }
            }
            self->deleteLater();
        }
    });

    m_percentageNeedRefresh = true;
    auto percentageWatcher = new QDBusPendingCallWatcher(m_powerBatteryIface->asyncCall(QStringLiteral("Percentage")), this);
    connect(percentageWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        QDBusPendingReply<double> reply = *self;
        if (self) {
            if (reply.isError()) {
                qDebug() << "PowerBatteryIface call Percentage error" << reply.error().message();
            } else {
                if (m_percentageNeedRefresh) {
                    onPercentageChanged(reply.value());
                }
            }
            self->deleteLater();
        }
    });

    m_isEnableNeedRefresh = true;
    auto enableWatcher = new QDBusPendingCallWatcher(m_powerBatteryIface->asyncCall(QStringLiteral("ShowIcon")), this);
    connect(enableWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        QDBusPendingReply<bool> reply = *self;
        if (self) {
            if (reply.isError()) {
                qDebug() << "PowerBatteryIface call ShowIcon error" << reply.error().message();
            } else {
                if (m_isEnableNeedRefresh) {
                    onBatteryShowIcon(reply.value());
                }
            }
            self->deleteLater();
        }
    });

    m_tooltipNeedRefresh = true;
    auto tooltipWatcher = new QDBusPendingCallWatcher(m_powerBatteryIface->asyncCall(QStringLiteral("GetBatteryTooltip")), this);
    connect(tooltipWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QString> reply = *self;
        if (self) {
            if (reply.isError()) {
                qDebug() << "PowerBatteryIface call GetBatteryTooltip error" << reply.error().message();
            } else {
                if (m_tooltipNeedRefresh) {
                    onBatteryTooltip(reply.value());
                }
            }
            self->deleteLater();
        }
    });

    m_isPresentNeedRefresh = true;
    auto isPresentWatcher = new QDBusPendingCallWatcher(m_powerBatteryIface->asyncCall(QStringLiteral("IsPresent")), this);
    connect(isPresentWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *self) {
        QDBusPendingReply<bool> reply = *self;
        if (self) {
            if (reply.isError()) {
                qDebug() << "PowerBatteryIface call IsPresent error" << reply.error().message();
            } else {
                if (m_isPresentNeedRefresh) {
                    onIsPresentChanged(reply.value());
                }
            }
            self->deleteLater();
        }
    });
}

void PowerButton::onServiceUnregistered()
{
    if (m_powerBatteryIface) {
        delete m_powerBatteryIface;
        m_powerBatteryIface = nullptr;
    }

    m_isEnable = false;
    Q_EMIT isEnableChanged();

    initPowerBatteryIface();
}

void PowerButton::onConnectorReady(QDBusInterface *interface)
{
    if (!interface || !interface->isValid()) {
        qWarning() << "Init powerBatteryIface error";
        return;
    }
    m_powerBatteryIface = interface;
    m_isEnable = true;
    Q_EMIT isEnableChanged();

    getBatteryInfo();

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              QStringLiteral("BatteryIcon"), this, SLOT(onBatteryIconChanged(QString)))) {
        qWarning() << "PowerBatteryIface connect signal BatteryIcon failed!";
    }

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              QStringLiteral("PercentageChanged"), this, SLOT(onPercentageChanged(double)))) {
        qWarning() << "PowerBatteryIface connect signal PercentageChanged failed!";
    }

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              QStringLiteral("BatteryShowIcon"), this, SLOT(onBatteryShowIcon(bool)))) {
        qWarning() << "PowerBatteryIface connect signal BatteryShowIcon failed!";
    }

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              QStringLiteral("BatteryTooltip"), this, SLOT(onBatteryTooltip(QString)))) {
        qWarning() << "PowerBatteryIface connect signal BatteryTooltip failed!";
    }

    if(!QDBusConnection::sessionBus().connect(POWER_BUS_NAME,
                                              POWER_BUS_PATH,
                                              POWER_BUS_INTERFACE,
                                              QStringLiteral("IsPresentChanged"), this, SLOT(onIsPresentChanged(bool)))) {
        qWarning() << "PowerBatteryIface connect signal BatteryTooltip failed!";
    }

    if (!m_powerServiceWatcher) {
        m_powerServiceWatcher = new QDBusServiceWatcher(POWER_BUS_NAME,
                                                        QDBusConnection::sessionBus(),
                                                        QDBusServiceWatcher::WatchForUnregistration,this);
        if (m_powerServiceWatcher) {
            connect(m_powerServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &PowerButton::onServiceUnregistered);
        } else {
            qDebug()<<"PowerServiceWatcher init error";
        }
    }
}

void PowerButton::onBatteryIconChanged(QString iconName)
{
    m_iconNeedRefresh = false;
    if (iconName == m_icon) return;
    m_icon = iconName;
    Q_EMIT iconChanged();
}

void PowerButton::onPercentageChanged(double percentage)
{
    m_percentageNeedRefresh = false;
    QString per = QString::number(percentage, 'f', 0);
    if (per == m_percentage) return;
    m_percentage = per;
    Q_EMIT percentageChanged();
}

void PowerButton::onBatteryShowIcon(bool isEnable)
{
    m_isEnableNeedRefresh = false;
    if (isEnable == m_isEnable) return;
    m_isEnable = isEnable;
    Q_EMIT isEnableChanged();
}

void PowerButton::onBatteryTooltip(QString tooltip)
{
    m_tooltipNeedRefresh = false;
    if (tooltip == m_tooltip) return;
    m_tooltip = tooltip;
    Q_EMIT tooltipChanged();
}

void PowerButton::onIsPresentChanged(bool isPersent)
{
    m_isPresentNeedRefresh = false;
    if (isPersent == m_isPresent) return;
    m_isPresent = isPersent;
    Q_EMIT isPresentChanged();
}
