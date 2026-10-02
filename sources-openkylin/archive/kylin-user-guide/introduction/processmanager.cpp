#include "processmanager.h"
#include <QProcess>
#include <QSettings>
#include <kysdk/kysdk-system/libkysysinfo.h>

processmanager::processmanager()
{
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        m_styleSettings = new QGSettings("org.ukui.style");
        connect(m_styleSettings, &QGSettings::changed, this, [=] (QString name){
            emit styleNameChanged(name);
        });
    }
}

QString processmanager::getUserName()
{

    QDBusInterface * m_pSystemUserIface = new QDBusInterface("org.freedesktop.Accounts",
                                                             "/org/freedesktop/Accounts",
                                                             "org.freedesktop.Accounts",
                                                             QDBusConnection::systemBus());

    QStringList users;
    QDBusReply<QList<QDBusObjectPath> > reply = m_pSystemUserIface->call("ListCachedUsers");
    if (reply.isValid()) {
        for (QDBusObjectPath op : reply.value())
            users << op.path();
    }

    UserInfomation user;
    for (QString objectpath : users){
         user = GetUserInformation(objectpath);
         if(user.current == true && user.logined == true){
             break;
         }
    }

    return  user.username;
}

UserInfomation processmanager::GetUserInformation(QString objpath)
{

    UserInfomation user;
    //默认值
    user.current = false;
    user.logined = false;
    user.autologin = false;

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                                    objpath,
                                                    "org.freedesktop.DBus.Properties",
                                                    QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        if (user.username == QString(g_get_user_name())) {
            user.current = true;
            user.logined = true;
        }
        user.autologin = false;
    }

    return user;

}

void processmanager::startAiAssistant()
{
    QString aiCommand = "kylin-aiassistant &";
    runCommand(aiCommand);
}

void processmanager::runCommand(const QString &command)
{
    QProcess *process = new QProcess();
    process->start(command);
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process]() {
        qDebug() << "Standard ai assistant Output:" << process->readAllStandardOutput();
    });

    QObject::connect(process, &QProcess::readyReadStandardError, [process]() {
        qDebug() << "Standard ai assistant Error:" << process->readAllStandardError();
    });
}


bool processmanager::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

QString processmanager::getThemeStyle()
{
    return m_styleSettings->get("styleName").toString();
}
