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
#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <memory>
#include <QDebug>

class UserInfo : public QObject
{
    Q_OBJECT
public:
    explicit UserInfo(QObject *parent = nullptr);
    explicit UserInfo(const UserInfo &userInfo);
    virtual ~UserInfo();

    bool operator==(const UserInfo &userInfo) const;

public:
    inline bool isLoggedIn() const
    {
        return m_isLoggedIn;
    }
    inline uid_t uid() const
    {
        return m_uId;
    }
    inline QString headImage() const
    {
        return m_strHeadImage;
    }
    inline QString fullName() const
    {
        return m_strFullName;
    }
    inline QString backGround() const
    {
        return m_strBackground;
    }
    inline QString lang() const
    {
        return m_strLang;
    }
    inline QString name() const
    {
        return m_strName;
    }
    inline QString greeterBackGround() const
    {
        return m_strGreeterBackground;
    }
    inline QString backGroundColor() const
    {
        return m_strBackgroundColor;
    }
    inline bool isSystemAccount() const
    {
        return m_isSystemAccount;
    }

    virtual inline QString path() const
    {
        return QString();
    }

public:
    void updateLoggedIn(const bool &isLoggedIn);
    void updateUid(const uid_t &id);
    void updateHeadImage(const QString &path);
    void updateFullName(const QString &fullName);
    void updateBackground(const QString &backGround);
    void updateLang(const QString &lang);
    void updateName(const QString &name);
    void updateGreeterBackground(const QString &background);
    void updateBackgroundColor(const QString &color);
    void updateSystemAccount(const bool &isSystemAccount);

Q_SIGNALS:
    void userPropChanged(const QString &userName);

protected:
    bool m_isLoggedIn;              // 是否已登录
    uid_t m_uId;                    // 用户 uid
    QString m_strHeadImage;         // 用户头像
    QString m_strFullName;          // 用户全名
    QString m_strBackground;        // 用户界面背景
    QString m_strLang;              // 用户语言
    QString m_strName;              // 用户名
    QString m_strGreeterBackground; // 登录界面背景
    QString m_strBackgroundColor;   // 桌面背景颜色
    bool m_isSystemAccount;         // 系统用户
};

typedef std::shared_ptr<UserInfo> UserInfoPtr;

QDebug operator<<(QDebug stream, const UserInfo &userInfo);

class LocalUserInfo : public UserInfo
{
    Q_OBJECT
public:
    explicit LocalUserInfo(const uid_t &uId, QObject *parent = nullptr);
    explicit LocalUserInfo(const QString &strPath, QObject *parent = nullptr);
    explicit LocalUserInfo(const LocalUserInfo &localUser);

    virtual ~LocalUserInfo();

    inline QString path() const override
    {
        return m_strPath;
    }

private:
    void initData();
    void initConnections();

private Q_SLOTS:
    void onPropertiesChanged(const QString &, const QVariantMap &, const QStringList &);

private:
    QString m_strPath;
    bool m_propertiesChangedConnected = false;
};

typedef std::shared_ptr<LocalUserInfo> LocalUserInfoPtr;

QDebug operator<<(QDebug stream, const LocalUserInfo &userInfo);

#endif // USERINFO_H
