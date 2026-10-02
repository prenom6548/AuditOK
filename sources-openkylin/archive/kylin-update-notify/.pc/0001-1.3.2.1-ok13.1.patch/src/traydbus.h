#ifndef TRAYDBUSSERVICE_H
#define TRAYDBUSSERVICE_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusConnection>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QProcess>

#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

class traydbusservice : public QObject
{
    Q_OBJECT
    //定义Interface名称为com.scorpio.test.value
    Q_CLASSINFO("D-Bus Interface", "com.scorpio.test.value")
public:
    traydbusservice();

    QStringList crucial;
    QStringList important;

public slots:
    QStringList getCrucial();
    QStringList getImportant();

    void        connectSuccessslots();
    void        quitslots();

private:

Q_SIGNALS: // SIGNALS
    QString ready(QString ret);
    QString connectSuccessSignal();
    QString quitsignal();

};

//连接源管理器的d-bus································

class source_dbus: public QObject
{
    Q_OBJECT

public:
    explicit source_dbus(QObject *parent = nullptr);
    ~source_dbus();
    QString           serialNumber;
    QTimer            *connectTimer;
    int               num=0;
    static source_dbus *getInstance();
    QDBusInterface     *interface1;             //连接源更新管理器的D-bus

    bool initSource_Dbus();
    static source_dbus *sourceMutual;           //UpdateDbus类静态对象

public slots:
    void connectDbus();

Q_SIGNALS:
    void ready();
};

//连接更新管理器的d-bus································

class update_dbus: public QObject
{
    Q_OBJECT

public:
    explicit update_dbus(QObject *parent = nullptr);
    ~update_dbus();
    static update_dbus *getInstance();
    QDBusInterface     *interface;                      //连接更新管理器的D-bus
    QDBusInterface     *interface_utils;                //kylin-system-updater utils
    void initUpdate_Dbus();
    static update_dbus *updateMutual;                   //UpdateDbus类静态对象
    QStringList checkForUpdates(QStringList arg);       //检测是否有更新
    bool checkForUpdateDect();       //检测系统更新
    bool installAndUpgrade(QStringList pkgNames);       //静默升级入口
    QTimer             *timer;

    bool aptD_busStatus=true;                           //发送状态的标志位

    bool listenState();
    QStringList installpkgs;
Q_SIGNALS:
    void ready();
    void aptAppNames_Percentsignal(QString arg,int32_t args,QString state); //发送正在下载安装的包名以及进度
    void dowloadStart();
    void UpdateInstallFinished(bool,QStringList,QString,QString);
    void UpdateDectFinished(bool,QStringList,QString,QString);

protected slots:
     void getsignal(QStringList, int, QString, QString);
     void slotUpdateCache(bool,QStringList,QString,QString);
     void slotInstallFinish(bool,QStringList,QString,QString);
     void UpdateDectSlot(bool,QStringList,QString,QString);
};

//连接电源的d-bus································

class power_dbus: public QObject
{
    Q_OBJECT
public:
    power_dbus(QObject *parent = nullptr);
    ~power_dbus();
    QDBusInterface    *interface;                       //连接更新管理器的D-bus
    bool initDbusaddrest();
    QString getPower_Dbus();
    QString   powerpath;


Q_SIGNALS:
    void ready();

protected slots:

};

//判断是否为活跃用户的dbus·······························


class MyClass
{
public:
    uint32_t uidnum;
    QString IDName;
    QDBusObjectPath address;
};
Q_DECLARE_METATYPE(MyClass)

class UserIdStatus: public QObject
{
    Q_OBJECT
public:
    UserIdStatus(QObject *parent = nullptr);
    ~UserIdStatus();
    QString UIDStatus;
    void initConnectionInfo();
    void initUserIdStatus();

Q_SIGNALS:
    void ready();

protected slots:

};


#endif // TRAYDBUSSERVICE_H
