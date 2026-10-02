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

#ifndef ACCOUNTINFORMATION_H
#define ACCOUNTINFORMATION_H

#include <QObject>
#include <QColor>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

namespace UkuiShortcut {

typedef struct UserInfo
{
    bool current;
    bool isLogged;
    bool autologin;
    bool noPwdLogin;

    int accountType;
    int passwdType;
    qint64 uid;

    QString objPath;
    QString username;
    QString realName;
    QString iconFile;
    QString passwd;
} UserInformation;

class PowerButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isEnable READ isEnable NOTIFY isEnableChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString tooltip READ tooltip NOTIFY tooltipChanged)
    Q_PROPERTY(QString percentage READ percentage NOTIFY percentageChanged)
    Q_PROPERTY(bool isPresent READ isPresent NOTIFY isPresentChanged)
public:
    explicit PowerButton(QObject *parent = nullptr);
    ~PowerButton() override;

    bool isEnable();
    QString icon();
    QString tooltip();
    QString percentage();
    bool isPresent();

Q_SIGNALS:
    void isEnableChanged();
    void iconChanged();
    void tooltipChanged();
    void percentageChanged();
    void isPresentChanged();

private:
    void initPowerBatteryIface();
    void getBatteryInfo();

private Q_SLOTS:
    void onServiceUnregistered();
    void onConnectorReady(QDBusInterface* interface);
    void onBatteryIconChanged(QString iconName);
    void onPercentageChanged(double percentage);
    void onBatteryShowIcon(bool isEnable);
    void onBatteryTooltip(QString tooltip);
    void onIsPresentChanged(bool isPersent);

private:
    bool      m_isEnable = false;
    QString   m_icon = "";
    QString   m_tooltip = "";
    QString   m_percentage = "";
    bool      m_isPresent = false;

    bool      m_iconNeedRefresh = false;
    bool      m_percentageNeedRefresh = false;
    bool      m_isEnableNeedRefresh = false;
    bool      m_tooltipNeedRefresh = false;
    bool      m_isPresentNeedRefresh = false;

    QDBusInterface *m_powerBatteryIface = nullptr;
    QDBusServiceWatcher *m_powerServiceWatcher = nullptr;
};

class AccountInformation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ getUsername NOTIFY userInfoChanged)
    Q_PROPERTY(QString realName READ getRealName NOTIFY userInfoChanged)
    Q_PROPERTY(QString iconFile READ getIconFile NOTIFY userInfoChanged)
    Q_PROPERTY(QString accountType READ getAccountType NOTIFY userInfoChanged)
public:
    explicit AccountInformation(QObject *parent = nullptr);

    ~AccountInformation() override;

    Q_INVOKABLE QString getUsername();
    Q_INVOKABLE QString getRealName();
    Q_INVOKABLE QString getIconFile();
    Q_INVOKABLE QString getAccountType();

    Q_INVOKABLE void openUserCenter();
    Q_INVOKABLE void openPeonyUser();
    Q_INVOKABLE void openControlCenter();
    Q_INVOKABLE void openPowerButton();


private:
    void initMemberVariable();
    void registeredAccountsDbus();
    void initUserInfo();
    QStringList getUserObjectPath();
    UserInformation GetUserInformation(const QString &objectPath);

private Q_SLOTS:
    void currentAccountUpdateSlot(const QString &property, const QMap<QString, QVariant> &propertyMap,
                                  const QStringList &propertyList);

Q_SIGNALS:
    void userInfoChanged();

private:
    QDBusInterface *m_systemUserIFace = nullptr;
    //用于监听当前登录用户的信息变化信号
    QDBusInterface *m_currentUserIFace = nullptr;

    UserInformation m_currentUserInfo;
    QString m_administrator;
    QString m_standardUser;
};

}

#endif // ACCOUNTINFORMATION_H
