#ifndef DBUSCONNECT_H
#define DBUSCONNECT_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#define UPGRADE_PATH "/com/kylin/RevisionsManager"
#define UPGRADE_SERVICE "com.kylin.RevisionsManager"
#define UPGRADE_INTERFACE "com.kylin.RevisionsManager.interface"

class dbusconnect : public QObject
{
    Q_OBJECT
public:
    explicit dbusconnect(QObject *parent = nullptr);
    ~dbusconnect();
    static dbusconnect *getInstance(QObject *parenet  = nullptr);

    QDBusInterface *interface;  //dbus接口

//private:
public:
    static dbusconnect *updateMutual;  //UpdateDbus类静态对象

signals:

};

#endif // DBUSCONNECT_H
