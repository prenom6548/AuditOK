#include "mainwindow.h"
#include <QSettings>
#include <QDBusInterface>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QTranslator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
//    QTranslator *translator = new QTranslator (this);
//    qDebug()<<"the system.name is "<<QLocale::system().name();
//    translator->load("/usr/share/upgradenote/translations/" + QLocale::system().name());
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString path = homePath+"/.config/kylin-update-frontend/";
    QDir dir(path);
    if(!dir.exists())
    {
        qDebug()<<"No dir exits";
        return ;
    }

    QString filename = homePath+"/.config/kylin-update-frontend/reboot.config";
    QFile file(filename);
    if(!file.exists())
    {
        qDebug()<<"No file exits";
        return ;
    }
    QSettings *settings = new QSettings(filename,QSettings::IniFormat);
    settings->beginGroup("CONFIG");
    QString status = settings->value("reboot_status").toString();
    settings->endGroup();
    qDebug()<<status;
    if(status == "0")
    {
        qDebug()<<"don't need reboot";
        return ;
    }
    else
    {
        qDebug()<<"need reboot";
        QStringList actions;
        QString cmd = "gdbus call --system --dest com.kylin.systemupgrade --object-path /com/kylin/systemupgrade --method com.kylin.systemupgrade.interface.TriggerInstallOnShutdown \"reboot\"";
        qDebug()<<cmd;
        actions.append("UpgradeRebootNotify");
        actions.append(QObject::tr("Reboot"));
        actions.append("");
        actions.append(QObject::tr("Later"));
        QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
        QList<QVariant> args;
        args<<(QObject::tr("System-Upgrade"))
           <<((unsigned int) 0)
          <<("ukui-control-center")
         <<QObject::tr("ukui-control-center-upgrade") //控制面板-更新提示
        <<QObject::tr("It's suggested to reboot.") //显示的具体信息
        <<actions
        <<QVariantMap()
        <<(int)0;
        QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
        if (response.type() == QDBusMessage::ReplyMessage)
        { //判断method的返回值
            qDebug() << "发送成功！ID：" << response.arguments().takeFirst().toString();
            int ID = response.arguments().takeFirst().toUInt(); //保存消息ID
            qDebug()<<"消息ID： "<<ID;
        }
        else
        {
            qDebug() << "发送失败：" << response.type();
        }
        return ;
        QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                              "/org/freedesktop/Notifications",
                                              "org.freedesktop.Notifications",
                                              "ActionInvoked",
                                              this,SLOT(Reboot()));
    }
}

void MainWindow::Reboot()
{
    qDebug()<<"get in reboot()";

}

MainWindow::~MainWindow()
{
}

