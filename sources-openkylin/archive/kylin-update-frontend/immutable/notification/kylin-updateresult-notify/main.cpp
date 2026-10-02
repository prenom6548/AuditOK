#include "notifications.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QtDBus/QtDBus>
#include "notifications.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariant>
#include <notifications.h>
#include <QDBusConnection>
#include <QtDBus>
#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    initUkuiLog4qt("kylin-updateresult-notify");
    QTranslator translator;
    const QString locale = Notifications::getCurrentLocale();
    qInfo() << "the system.locale is" << locale;
    const QString trdocument = QString("%1%2%3").arg("/usr/share/upgrade/translations/kylin-updateresult-notify_").arg(locale).arg(".qm");
    qInfo() << "the load document is " << trdocument;
    if(translator.load(trdocument)){
        qInfo() << "success to load translation file!";
        a.installTranslator(&translator);
    }else{
        qInfo() << ("Failed to load translation file!");
    }
    Notifications notification;
    qInfo()<<"prepare to popup notification in 50s ";
    QDBusInterface iface("com.kylin.systemupgrade",
                         "/com/kylin/systemupgrade",
                         "com.kylin.systemupgrade.interface",
                         QDBusConnection::systemBus());
    if (!iface.isValid()) {
        qInfo()<< "the type is " << iface.lastError().message();
        Notifications::notifications("error");
        return 1;
    }
    QDBusReply<QString> reply = iface.call("CheckPopupOnPoweroffInstalled");
    QDBusMessage replyMessage =iface.call("CheckPopupOnPoweroffInstalled");
    if (replyMessage.type() == QDBusMessage::ErrorMessage) {
        qInfo()<< "the type is " << replyMessage.errorMessage();
        Notifications::notifications("error");
        return 1;
    }

    const QString result = reply.value();
    qInfo() << "the result is " << result << endl;
    if(result == "success"){
        Notifications::notifications("success");
    }else if(result == "none"){
        return 0;
    }else{
       Notifications::notifications(result);
    }
    return 0;
}
