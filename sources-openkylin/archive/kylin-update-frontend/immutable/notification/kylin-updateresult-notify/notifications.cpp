#include "notifications.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>
#include <QLocale>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QApplication>
#include <QProcess>
#include <locale.h>
#include <iostream>
#include <QTranslator>
#include <QIcon>
#include <QCoreApplication>
#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>
Notifications::Notifications(QObject *parent)
    : QObject(parent)
{
    //连接到NotificationClosed信号
    QDBusConnection::systemBus().connect("org.freedesktop.Notifications",
                                         "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications",
                                         "onNotificationClosed",this,SLOT(onNotificationClosed(uint32_t,uint32_t))
                                         );
    //连接到ActionInvoked信号
    QDBusConnection::systemBus().connect("org.freedesktop.Notifications",
                                         "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications",
                                         "onNotificationClosed",this,SLOT(onActionInvoked(uint32_t,QString))
                                         );
}

//获取当前系统的区域设置信息
QString Notifications::getCurrentLocale(){
    QLocale locale = QLocale::system();
    return locale.name();
}

int Notifications::notifications(const QString &sigvalue){
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if(sigvalue == "success")
    {
        QStringList actions;
        actions.append("OK");
        actions.append(QObject::tr("OK"));
        QList<QVariant> args;
        QVariantMap hints;
        hints.insert("x-ukui-popup-timeout",-1);

        args<<QObject::tr("System Update")
        <<((unsigned int) 0)
        <<"/usr/share/icons/ukui-icon-theme-default/16x16/apps/ukui-control-center.png"
        <<QObject::tr("System Update")
        <<QObject::tr("upgrade success!")
        <<actions
        <<hints
        <<(int)0;
        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    }
    else if(sigvalue == "failed")
    {
        qInfo()<<"upgrade fail ！";
        QStringList actions;
        actions.append("OK");
        actions.append(QObject::tr("OK"));
        QList<QVariant> args;
        QVariantMap hints;
        hints.insert("x-ukui-popup-timeout",-1);

        args<<QObject::tr("System Update")
        <<((unsigned int) 0)
        <<"/usr/share/icons/ukui-icon-theme-default/16x16/apps/ukui-control-center.png"
        <<QObject::tr("System Update")
        <<QObject::tr("Update Failed!")
        <<actions
        <<hints
        <<(int)0;
        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    }
    else if(sigvalue == "error")
    {
        qInfo()<<"upgrade error ！";
//        QStringList actions;
//        actions.append("OK");
//        actions.append(QObject::tr("OK"));
//        QList<QVariant> args;
//        QVariantMap hints;
//        hints.insert("x-ukui-popup-timeout",-1);

//        args<<QObject::tr("System Update")
//        <<((unsigned int) 0)
//        <<"/usr/share/icons/ukui-icon-theme-default/16x16/apps/ukui-control-center.png"
//        <<QObject::tr("System Update")
//        <<QObject::tr("The backend is abnormal,please check for updates on settings.")
//        <<actions
//        <<hints
//        <<(int)0;
//        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    }
    else
    {
        qInfo()<<"upgrade failed and error code is :" << sigvalue;
        QString str;
        str = QString("%1%2%3").arg(QObject::tr("upgrade fail! Error code(")).arg(sigvalue).arg(")");
        QStringList actions;
        actions.append("OK");
        actions.append(QObject::tr("OK"));
        QList<QVariant> args;
        QVariantMap hints;
        hints.insert("x-ukui-popup-timeout",-1);

//        const char* constState = state.toUtf8().constData();
        args<<QObject::tr("System Update")
        <<((unsigned int) 0)
        <<"/usr/share/icons/ukui-icon-theme-default/16x16/apps/ukui-control-center.png"
        <<QObject::tr("System Update")
        <<QObject::tr(str.toUtf8().constData())
        <<actions
        <<hints
        <<(int)0;
        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    }
}

void Notifications::onNotificationClosed(uint32_t id, uint32_t reason){
    qInfo() << "Notification closed.id:" << id;
    switch(reason){
    case 1:
        qInfo() << "超时关闭";
        break;
    case 2:
        qInfo() << "通知被用户关闭";
        break;
    case 3:
        qInfo() << "通过CloseNotification关闭";
        break;
    case 4:
        qInfo() <<"未定义其他原因";
        break;
    default:
        break;
    }
}

void Notifications::onActionInvoked(uint32_t id, const QString &action){
    qInfo() << "Action invoked.id:" << id;
}


Notifications::~Notifications()
{
}

