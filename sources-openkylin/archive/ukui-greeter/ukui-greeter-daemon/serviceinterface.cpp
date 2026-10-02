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
#include "serviceinterface.h"
#include <qdbusconnection.h>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QTextCodec>
#define CONFIG_FILE "/usr/share/ukui-greeter/ukui-greeter.conf"

/*
* ukui-greeter.conf 例子
* 
* [Greeter]
* promptTextFilePath=/usr/share/ukui-greeter/ukui-greeter.txt1
* promptTitle=这是标题
* showLoginPrompt=true
*
*/

/* For the type WillLoginUserInfo */
QDBusArgument &operator<<(QDBusArgument &argument, const WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument >> willLoginUserInfo.strUserName >> willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

QDebug operator <<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo)
{
    stream << "WillLogUser ["
           << willLoginUserInfo.strUserName
           << willLoginUserInfo.isOneKeyLogin
           << "]";
    return stream;
}

ServiceInterface::ServiceInterface()
{
    qRegisterMetaType<WillLoginUserInfo>("WillLoginUserInfo");
    qDBusRegisterMetaType<WillLoginUserInfo>();

    m_willLoginUser.strUserName = "";
    m_willLoginUser.isOneKeyLogin = false;
    bool res = QDBusConnection::systemBus().registerService("org.ukui.GreeterDaemon");
    if(!res){
        exit(0);
    }
    res = QDBusConnection::systemBus().registerObject("/org/ukui/GreeterDaemon", "org.ukui.GreeterDaemon",
                                                      this, QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals|QDBusConnection::ExportAllProperties);
    if(!res){
        exit(0);
    }

    settings = new QSettings(CONFIG_FILE,QSettings::IniFormat);
    settings->setIniCodec(QTextCodec::codecForName("utf-8"));
}

int ServiceInterface::SwitchToUser(QString strUserName)
{
    m_willLoginUser.strUserName = strUserName;
    m_willLoginUser.isOneKeyLogin = true;
    Q_EMIT userChanged(m_willLoginUser);
    return 0;
}

int ServiceInterface::SwitchToGreeterUser(QString strUserName)
{
    m_willLoginUser.strUserName = strUserName;
    m_willLoginUser.isOneKeyLogin = false;
    Q_EMIT userChanged(m_willLoginUser);
    return 0;
}

WillLoginUserInfo ServiceInterface::getWillSwitchUser()
{
    return m_willLoginUser;
}

QVariant ServiceInterface::getValue(const QString &key)
{
    settings->beginGroup("Greeter");
    QVariant value = settings->value(key);
    settings->endGroup();

    return value;
}

void ServiceInterface::setValue(const QString &key, const QVariant &value)
{
    settings->beginGroup("Greeter");
    settings->setValue(key, value);
    settings->endGroup();
    settings->sync();
}

bool ServiceInterface::getGreeterShowPrompt()
{
    //如果开关为关，返回false
    if(!getShowLoginPrompt())
        return false;

    //如果标题为空，返回false
    if(getPromptTitle().isEmpty())
        return false;

    //prompttext不为空或者文件不为空，返回true
    if(getPromptText().isEmpty()){
        if(getPromptTextFilePath().isEmpty()){
            return false;
        }else{
            QFile file(getPromptTextFilePath());
            if(file.exists() && file.size() >  0)
                return true;
            else
                return false;
        }
    }
    return true;
}

bool ServiceInterface::getShowLoginPrompt()
{
    if(settings->contains("Greeter/showLoginPrompt"))
        return getValue("showLoginPrompt").toBool();
    else
        return false;
}

bool ServiceInterface::getHideTitle()
{
    if(settings->contains("Greeter/hideTitle"))
        return getValue("hideTitle").toBool();
    else
        return true;
}

QString ServiceInterface::getPromptTitle()
{
    if(settings->contains("Greeter/promptTitle"))
        return getValue("promptTitle").toString();
    else
        return "";
}

QString ServiceInterface::getPromptText()
{
    if(settings->contains("Greeter/promptText"))
        return getValue("promptText").toString();
    else
        return "";
}

QString ServiceInterface::getPromptTextFilePath()
{
    if(settings->contains("Greeter/promptTextFilePath"))
        return getValue("promptTextFilePath").toString();
    else
        return "";
}

void ServiceInterface::setShowLoginPrompt(bool prompt)
{
    setValue("showLoginPrompt",prompt);
}

void ServiceInterface::setHideTitle(bool isHide)
{
    setValue("hideTitle",isHide);
}

void ServiceInterface::setPromptTitle(QString title)
{
    setValue("promptTitle",title);
}

void ServiceInterface::setPromptText(QString text)
{
    setValue("promptText",text);
}

void ServiceInterface::setPromptTextFilePath(QString filePath)
{
    setValue("promptTextFilePath",filePath);
}

