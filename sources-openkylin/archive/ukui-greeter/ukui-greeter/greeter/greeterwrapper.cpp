/* greeterwrapper.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "greeterwrapper.h"
#include "rootWindowBackground.h"
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QDebug>
#include <QBitArray>
#include <QString>
#include <QFile>
#include <QTextCodec>
#include "../../ukui-greeter/common/globalv.h"
#include "../../ukui-greeter/common/configuration.h"
#include <QTextStream>
#include <QDBusPendingReply>


#define KSPLASH_ENV_PATH "/tmp/ukui-ksplash-env"

GreeterWrapper* GreeterWrapper::m_instance = nullptr;
GreeterWrapper::GreeterWrapper(QObject *parent) : QLightDM::Greeter(parent)
{
    //连接到lightdm
    if(!connectToDaemonSync()){
        qDebug() << "connect to Daemon failed";
        exit(1);
    }

    drawBackgroundType = 0;
    drawBackgroundColor = 0x0;

    /*screenMonitor是重力感应的脚本，因为登录界面暂无接口可以拉起进程，这里放在
     * 了登录界面去判断如果脚本存在，则主动拉起*/
    QString cmd = "/usr/bin/screenMonitor";
    if(QFile(cmd).exists()){
        screenMonitorProcess = new QProcess(this);
        screenMonitorProcess->start(cmd);
    }
}

GreeterWrapper* GreeterWrapper::instance(QObject *parent)
{
    if (!m_instance) {
        m_instance = new GreeterWrapper(parent);
    }
    return m_instance;
}

void GreeterWrapper::setLang(const QString &lang)
{
    this->m_language = lang;
}

QString GreeterWrapper::lang()
{
    return this->m_language;
}

void GreeterWrapper::setSession(const QString &session)
{
    m_session = session;
}

QString GreeterWrapper::session()
{
    return m_session;
}

QString GreeterWrapper::userName()
{
    return m_userName;
}

void GreeterWrapper::setUserName(const QString &userName)
{
    m_userName = userName;
}

void GreeterWrapper::setrootWindowBackground(int type,unsigned int color,QString filename)
{
    drawBackgroundType = type;
    if(drawBackgroundType == rootWinPicture)
        m_rootWindowBackground = filename;
    else if(drawBackgroundType == rootWinColor)
        drawBackgroundColor = color;  
}

void GreeterWrapper::setRootWindowBgOptions(int nOption)
{
    m_nPicOptions = nOption;
}

bool GreeterWrapper::getdrawBackgroundIsStarted()
{
    return drawBackgroundIsStarted;
}

void GreeterWrapper::setrootWindow()
{
    drawBackgroundIsStarted = true;
    Configuration  *m_configure = Configuration::instance();
    QString m_defaultBackgroundPath = m_configure->getDefaultBackgroundName();
    if(m_rootWindowBackground.isEmpty())
        m_rootWindowBackground = m_defaultBackgroundPath;
    if(m_rootWindowBackground.isEmpty())
        return;

    char*  path;
    QByteArray ba = m_rootWindowBackground.toLatin1(); // must
    path=ba.data();

    if(drawBackgroundType == 0)
        setRootWindowBackground(0,0,path,m_nPicOptions);
    else
        setRootWindowBackground(1,drawBackgroundColor,NULL,m_nPicOptions);
    drawBackgroundIsStarted = false;
}

QString GreeterWrapper::getEnsureShareDir(QString username)
{
    return ensureSharedDataDirSync(username);
}

void GreeterWrapper::closeNM()
{
    QDBusInterface interfaceNM("org.greeter.nm",
                                 "/org/greeter/nm",
                                 "org.greeter.nm",
                                 QDBusConnection::systemBus());
    interfaceNM.call("exitGreeterHelp");
}

void GreeterWrapper::startSession()
{
    if(isAuthenticated())
        Q_EMIT authenticationSucess(authenticationUser());

    //closeNM();

    if(screenMonitorProcess){
    	screenMonitorProcess->kill();
    }
    //启动session
    //setrootWindow();
    draw_background();
    qDebug()<<"start session";

    if (QString(qgetenv("XDG_SESSION_TYPE")) != "wayland") {
        if (!userLoggedIn())
            showKsplashAnimation();
    }

    if(!startSessionSync(m_session)) {
        Q_EMIT startSessionFailed();
        Q_EMIT showMessage(tr("failed to start session."), QLightDM::Greeter::MessageTypeError);
    }
}


bool GreeterWrapper::userLoggedIn()
{
    QString user = getCurUserName();
    bool loggedIn = false;

    QDBusInterface loginInterface(QStringLiteral("org.freedesktop.login1"),
                                  QStringLiteral("/org/freedesktop/login1"),
                                  QStringLiteral("org.freedesktop.login1.Manager"),
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        QDBusMessage message = loginInterface.call("ListUsers");

        QVector<QString> userNameList;
        const QDBusArgument &dbusArgs = message.arguments().at(0).value<QDBusArgument>();
        dbusArgs.beginArray();
        while(!dbusArgs.atEnd()) {
            dbusArgs.beginStructure();
            int uid;
            QString userName;
            QDBusObjectPath objpath;
            dbusArgs >> uid >> userName >> objpath;
            dbusArgs.endStructure();
            userNameList.push_back(userName);
        }
        dbusArgs.endArray();

        for (const QString &userName : userNameList) {
            if (userName == user) {
                loggedIn = true;
            }
        }
    }
    return loggedIn;
}

void GreeterWrapper::showKsplashAnimation()
{
    QString display = qgetenv("DISPLAY");
    QString user = getCurUserName();

    QFile file(KSPLASH_ENV_PATH);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream out(&file);
        out << "DISPLAY=" << display << endl;
        out << "UserName=" << user << endl;
        file.close();

        QDBusInterface splashInterface(QStringLiteral("org.ukui.KSplash"),
                                QStringLiteral("/KSplash"),
                                QStringLiteral("org.ukui.KSplash"),
                                QDBusConnection::systemBus());

        if (splashInterface.isValid()) {
            splashInterface.asyncCall("setStage", "ksplashStarted");
        }
    } else {
        qDebug() << "open ksplash file failed";
    }

}
