#ifndef DAEMONIPCDBUS_H
#define DAEMONIPCDBUS_H


#define KYLIN_USER_GUIDE_PATH ("/")

#define KYLIN_USER_GUIDE_SERVICE ("com.kylinUserGuide.hotel")

#define KYLIN_USER_GUIDE_INTERFACE ("com.guide.hotel")

#define SERVICE_NAME_SIZE (30)

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>

//Command handler here...
class DaemonIpcDbus : public QObject
{
    Q_OBJECT
    // 定义Interface名称为com.scorpio.test.value
//    Q_CLASSINFO("D-Bus Interface", "com.scorpio.test.value")
public:
    DaemonIpcDbus();
Q_SIGNALS:
    void request(QStringList);
public Q_SLOTS:
    int daemonIsNotRunning();
    void showGuide(QString appName);
    void request(QString command);
    void request(QString command, QString parameter);
};

#endif // DAEMONIPCDBUS_H
