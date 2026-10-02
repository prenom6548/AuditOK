#include "traydbus.h"
#include <iostream>
#include <ostream>
#include <QDateTime>

//托盘d-bus 服务
traydbusservice::traydbusservice()
{

}

QStringList traydbusservice::getCrucial()
{
    QFile file("/var/lib/kylin-software-properties/template/crucial.list");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return crucial;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str = QString(line);
        qDebug()<<str;
        crucial = str.split(QRegExp("[ ]"));
        qDebug()<<"d-bus crucial: "<<crucial;
    }
    return crucial;
}
QStringList traydbusservice::getImportant()
{
    QFile file("/var/lib/kylin-software-properties/template/important.list");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return important;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str = QString(line);
        qDebug()<<str;
        important = str.split(QRegExp("[ ]"));
        qDebug()<<"d-bus important: "<<important;
    }
    return important;
}
void traydbusservice::connectSuccessslots()
{
    emit connectSuccessSignal();
}
void  traydbusservice::quitslots()
{
    emit quitsignal();
}


//连接源管理器的d-bus································

source_dbus* source_dbus::sourceMutual = nullptr;
using namespace std;
/**
 * @brief source_dbus::getInstance
 * 单例
 * @return
 */
source_dbus* source_dbus::getInstance()
{
    static QMutex mutex;
    if (nullptr == sourceMutual) {
        QMutexLocker locker(&mutex);
        sourceMutual = new source_dbus;
    }
    return sourceMutual;
}

source_dbus::source_dbus(QObject *parent)
    :QObject(parent)
{
    connectTimer = new QTimer(this);
    connect(connectTimer, SIGNAL(timeout()), this, SLOT(connectDbus()));
}

source_dbus::~source_dbus()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
}

/**
 * @brief source_dbus::initSource_Dbus
 * 连接源管理器dbus
 * @return
 */
bool source_dbus::initSource_Dbus()
{
    //创建QDBusInterface接口
    interface1 = new QDBusInterface("com.kylin.software.properties", "/com/kylin/software/properties",
                                   "com.kylin.software.properties.interface",
                                    QDBusConnection::systemBus());
    if (!interface1->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        exit(1);
    }

    interface1->setTimeout(123456789);
    //调用远程的value方法，源管理器update
    qDebug()<<"Start updateSourceTemplate ...";
    QDBusReply<bool> reply1 = interface1->call(QDBus::Block,  "updateSourceTemplate");
    if (reply1.isValid()) {
        if (reply1.value()) {
//            QDBusReply<int> reply2 = interface1->call(QDBus::Block,  "updateSourcePackages","kylin-background-upgrade");
//            if (reply2.isValid()) {
//                qDebug()<<"updateSourcePackages return value:"<<reply2.value();
//                if (reply2.value() == 100) {
//                     qDebug()<<"源管理器 updateSource 连接成功";
//                     emit ready();
//                } else  {
//                    qDebug()<<"源管理器updateSourcePackages failed，连接失败 ";
//                    return false;
//                }
//            } else {
//                qDebug() << "源管理器updateSourcePackages failed ";
//                return false;
//            }
            emit ready();
        } else {
            qDebug()<<"updateSourceTemplate failed ";
            return false;
        }
    } else {
        qDebug() << "updateSourceTemplate method called failed ";
        return false;
    }
    return true;
}

/**
 * @brief source_dbus::connectDbus
 * 重连机制
 */
void source_dbus::connectDbus()
{
    connectTimer->start(3000);
    if (initSource_Dbus()) {
        connectTimer->stop();
    } else {
        num ++;
        if (num >2) {
            qDebug()<<"updateSourceTemplate failed, exit.";
            exit(0);
        }
        qDebug()<<"updateSourceTemplate failed，Retries are in progress for the "<<num<<"th time.";
    }
}

//连接更新管理器的d-bus································

update_dbus* update_dbus::updateMutual = nullptr;
using namespace std;

update_dbus* update_dbus::getInstance()
{
    static QMutex mutex;
    if (nullptr == updateMutual) {
        QMutexLocker locker(&mutex);
        updateMutual = new update_dbus;
    }
    return updateMutual;
}


update_dbus::update_dbus(QObject *parent)
    :QObject(parent)
{
    initUpdate_Dbus();

}

update_dbus::~update_dbus()
{

}

/**
 * @brief update_dbus::initUpdate_Dbus
 * 连接更新管理器dbus
 */
void update_dbus::initUpdate_Dbus()
{
    //创建QDBusInterface接口
    interface = new QDBusInterface("com.kylin.systemupgrade", "/com/kylin/systemupgrade",
                                   "com.kylin.systemupgrade.interface",
                                    QDBusConnection::systemBus());
    if (!interface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        qDebug() << "Failed to connect /com/kylin/systemupgrade ";
        exit(1);
    }
    interface_utils = new QDBusInterface("com.kylin.systemupgrade", "/com/kylin/systemupgrade/utils",
                                   "com.kylin.systemupgrade.interface",
                                    QDBusConnection::systemBus());
    if (!interface_utils->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        qDebug() << "Failed to connect /com/kylin/systemupgrade ";
        exit(1);
    }

    //监听D-bus信号，获取安装信息与进度
//    connect(interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList, int, QString, QString)),this,SLOT(getsignal(QStringList, int, QString, QString)));
//    QDBusConnection::systemBus().connect(QString("com.kylin.systemupgrade"), QString("/com/kylin/systemupgrade"),
//                                         QString("com.kylin.systemupgrade.interface"),
//                                         QString("UpdateDloadAndInstStaChanged"), this, SLOT(getsignal(QStringList, int32_t, QString, QString)));

}

/**
 * @brief update_dbus::getsignal
 * 获取下载进度
 */
void update_dbus::getsignal(QStringList pkgs, int progess, QString status, QString current_details)
{
    QString aptStatus = "";
//    QString aptAppName;
    int aptPercent = 0;

//    QVariant dateQVariant;
//    aptStatus = arg;

//    QVariantMap::Iterator it;
//    for (it = key.begin(); it != key.end(); ++it) {
//        if (it.key() == "apt_appname") {
//            dateQVariant = it.value();
//            aptAppName = dateQVariant.toString();
//        }
//        if (it.key() == "apt_percent") {
//            dateQVariant = it.value();
//            aptPercent = dateQVariant.toFloat();
//        }
//    }

//    if (arg == "apt_start" && aptD_busStatus == true) {
//         aptD_busStatus =false;
//    }
//    if ((arg == "apt_start" || arg == "apt_finish") && (aptPercent == 0 || aptPercent == 100)) {
//         emit aptAppNames_Percentsignal(aptAppName,aptPercent,true);
//    } else if (arg == "apt_error" ) {
//        emit aptAppNames_Percentsignal(aptAppName,aptPercent,false);
//    }
    if (aptPercent != progess) {
        aptPercent = progess;
        emit aptAppNames_Percentsignal(pkgs.at(0),aptPercent,status);
    }

}

/**
 * @brief update_dbus::checkForUpdates
 * 检测强制推送列表
 */
QStringList update_dbus::checkForUpdates(QStringList arg)
{
    QDBusReply<QStringList> reply = interface_utils->call("CheckInstalledOrUpgrade",arg);
    if (reply.isValid())
    {
        qDebug()<<"The upgradeable list："<<reply.value();
        return reply.value();

    } else {
        qDebug() << "Get upgradeable list failed, exit";
        exit(0);
    }
}

/**
 * @brief update_dbus::checkForUpdateDect
 * 检测系统更新
 */
bool update_dbus::checkForUpdateDect()
{
    qDebug()<<"Start to check system upgrade ...";
    connect(interface,SIGNAL(UpdateDetectFinished(bool, QStringList, QString, QString)),this,SLOT(UpdateDectSlot(bool, QStringList, QString, QString)));
    QDBusReply<QVariantList> res = interface->asyncCall("UpdateDetect");
    qDebug() << "UpdateDetect (mode: MODE_UPDATE_DETECT) ...";
    return true;
}

/**
 * @brief update_dbus::installAndUpgrade
 * 静默升级入口
 */
bool update_dbus::installAndUpgrade(QStringList pkgNames)
{
    installpkgs = pkgNames;

    //调用D-bus接口，安装软件
    connect(interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
//    QDBusPendingCall call = interface_utils->asyncCall("InstallPackages","kylin-background-upgrade", installpkgs);
    QDBusPendingCall call = interface->asyncCall("UpdateCache");
    qDebug() << "UpdateCache (mode: MODE_UPDATE_CACHE) ...";  //MODE_UPDATE_CACHE
    return true;
}

void update_dbus::slotInstallFinish(bool status, QStringList pkgs,QString error,QString details)
{
    emit UpdateInstallFinished(status,pkgs,error,details);
}

void update_dbus::UpdateDectSlot(bool status, QStringList pkgs,QString error,QString details)
{
    emit UpdateDectFinished(status,pkgs,error,details);
}

void update_dbus::slotUpdateCache(bool status,QStringList pkgslist,QString errorstring,QString errorcode)
{
    if (status) {
        qDebug() << "MODE_UPDATE_CACHE finish .";
        connect(interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(slotInstallFinish(bool,QStringList,QString,QString)));
        interface->asyncCall("InstallPackages", installpkgs);
    } else {
        qDebug() << "Call UpdateCache (mode: MODE_UPDATE_CACHE) failed, exit!";
        exit(0);
    }
}

/**
 * @brief update_dbus::listenState
 * 监听状态
 * @return
 */
bool update_dbus::listenState()
{
    QDBusReply<bool> r4eply = interface->call("get_important_status");
    return r4eply;
}


//连接电源的d-bus································

power_dbus::power_dbus(QObject *parent)
    :QObject(parent)
{

}

power_dbus::~power_dbus()
{

}

/**
 * @brief power_dbus::initDbusaddrest
 * 连接电源管理器的dbus，获取dbus路径
 * @return
 */
bool power_dbus::initDbusaddrest()
{

    QStringList users;
    QDBusInterface m_interface1( "org.freedesktop.UPower",
                                "/org/freedesktop/UPower",
                                "org.freedesktop.UPower",
                                QDBusConnection::systemBus() );
    if (!m_interface1.isValid()) {
        qDebug() << "< org.freedesktop.UPower > initialization failed!";
        return false;
    }

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface1.call("EnumerateDevices");


    if (obj_reply.isValid()) {
        for (QDBusObjectPath op : obj_reply.value())
             users << op.path();
        if (users.size()==1 || users.isEmpty()) {
            qDebug()<<"Failed to obtain the remaining battery capacity, The built-in power supply cannot be detected ...";
            return false;
        }
        powerpath=users.at(1);
        return true;
     }
    return true;
}

/**
 * @brief power_dbus::getPower_Dbus
 * 连接电源管理器的dbus
 * @return
 */
QString power_dbus::getPower_Dbus()
{
    QDBusInterface m_interface( "org.freedesktop.UPower",
                                powerpath,
                                "org.freedesktop.DBus.Properties",
                                QDBusConnection::systemBus());

    if (!m_interface.isValid()) {
        qDebug() << "电源管理器dbus接口初始化失败";
        return "false";
    }

    QDBusReply<QVariant> obj_reply = m_interface.call("Get","org.freedesktop.UPower.Device","Percentage");
    QString Ele_surplus=obj_reply.value().toString();
    return Ele_surplus;
}

//判断是否为活跃用户的dbus·······························

UserIdStatus::UserIdStatus(QObject *parent)
    :QObject(parent)
{
    initUserIdStatus();
}

UserIdStatus::~UserIdStatus()
{

}


void UserIdStatus::initConnectionInfo()
{
//    qRegisterMetaType<MyClass>("MyClass");
//    QDBusInterface m_interface1("org.freedesktop.login1",
//                                "/org/freedesktop/login1",
//                                "org.freedesktop.login1.Manager",
//                                QDBusConnection::systemBus() );
//    if (!m_interface1.isValid()) {
//        qDebug() << "dbus接口初始化失败";
//        return;
//    }

//    QDBusMessage res = m_interface1.call("ListUsers");

//    qDebug() << res.arguments().at(0).value<MyClass>().IDName;
//    qDebug()<<res;
}

void UserIdStatus::initUserIdStatus()
{
    static int uid = -1;
    uid = getuid();
    QString uIDaddress = QString("/org/freedesktop/login1/user/_%1").arg(uid);
    QDBusInterface m_interface1("org.freedesktop.login1",
                                uIDaddress,
                                "org.freedesktop.DBus.Properties",
                                QDBusConnection::systemBus());
    if (!m_interface1.isValid()) {
        qDebug() << "dbus接口初始化失败";
        return;
    }

     QDBusReply<QVariant> res = m_interface1.call("Get","org.freedesktop.login1.User","State");
     UIDStatus = res.value().toString();
}




