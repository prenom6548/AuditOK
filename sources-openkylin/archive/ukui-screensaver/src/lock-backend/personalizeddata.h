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
#ifndef PERSONALIZEDDATA_H
#define PERSONALIZEDDATA_H

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QFile>
#include <QTimer>
#include <QEventLoop>
#include <QTimerEvent>
#include <QFileSystemWatcher>
#include <QSharedPointer>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>
#include <QModelIndex>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>

#include "ukui-log4qt.h"
#include "CSingleton.h"

class QDBusPendingCallWatcher;

class PersonalizedData : public QObject
{
    Q_OBJECT
public:
    explicit PersonalizedData(QString user);
    virtual ~PersonalizedData();

    ////////////////////ukui-greeter-conf//////////////////////////////
    QString dateType(void)
    {
        return m_dateType;
    }
    void dateType(QString val)
    {
        m_dateType = val;
    }

    int fontSize(void)
    {
        return m_fontSize;
    }
    void fontSize(int val)
    {
        m_fontSize = val;
    }

    int timeType(void)
    {
        return m_timeType;
    }
    void timeType(int val)
    {
        m_timeType = val;
    }

    QString backgroundPath(void)
    {
        return m_backgroundPath;
    }
    void backgroundPath(QString val)
    {
        m_backgroundPath = val;
    }

    QString color(void)
    {
        return m_color;
    }
    void color(QString val)
    {
        m_color = val;
    }

    /////////////////////usd-ukui-settings-daemon////////////////////////////////
    int cursor_size(void)
    {
        return m_cursor_size;
    }
    void cursor_size(int val)
    {
        m_cursor_size = val;
    }

    QString cursor_theme(void)
    {
        return m_cursor_theme;
    }
    void cursor_theme(QString val)
    {
        m_cursor_theme = val;
    }

    int scaling_factor(void)
    {
        return m_scaling_factor;
    }
    void scaling_factor(int val)
    {
        m_scaling_factor = val;
    }

    void getJsonData(QJsonObject &json);
    void setJson(const QJsonObject &json);

signals:
    void conf_changed(QString user);

protected:
    // ukui-greeter-conf
    QString m_dateType = "cn";
    int m_fontSize = 5;
    int m_timeType = 24;
    QString m_backgroundPath;
    QString m_color;

    // usd-ukui-settings-daemon
    int m_cursor_size = 48;
    QString m_cursor_theme;
    int m_scaling_factor = 1;

    //用户名
    QString m_user;
};

typedef QSharedPointer<PersonalizedData> KylinUserDatePtr;

/////////////////////////////////////////////////
/// \brief The PersonalizedDataMng class
///

class PersonalizedDataMng : public QObject
{
    Q_OBJECT
protected:
    explicit PersonalizedDataMng(void);
    virtual ~PersonalizedDataMng();

public:
    QString GetConfInformation(QString);

    KylinUserDatePtr GetUserPtr(QString user);
protected slots:
    void updateUserInformation(QString jsonstring);

protected:
    void __getUserInfomation(void);

    void wait_for_finish(QDBusPendingCallWatcher *call);

protected:
    QMap<QString, KylinUserDatePtr> m_userPersonalizedData;

    friend class KySingleTon<PersonalizedDataMng>;
};

typedef KySingleTon<PersonalizedDataMng> KYLINUSERDATAMNG;

#endif // PERSONALIZEDDATA_H
