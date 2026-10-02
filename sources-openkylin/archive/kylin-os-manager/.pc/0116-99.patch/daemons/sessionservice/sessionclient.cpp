#include "sessionclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QProcess>
#include <stdio.h>
#include <dlfcn.h>
#include <QDir>
#include "database/drivermanagerdatabase.h"
#include "../service/dbus_args.h"

#define SECURITY_LIB "/usr/lib/kysdk/kysdk-security/libkyprocesssec.so"
typedef int (*func)(const char *filepath);
SessionClient::SessionClient(QObject *parent)
    : QObject(parent)
{
    initHotplugClient();

    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/ServiceSupport",
                                         "com.KylinOsManager.ServiceSupport",
                                         "RunGlobalFeedback",
                                         this,
                                         SLOT(on_RunGlobalFeedback(int,QString,QString,QString,QString)));
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/DiskMonitor",
                                         "com.KylinOsManager.DiskMonitor",
                                         "LogFileTotalSizeToplimit",
                                         this,
                                         SLOT(on_NotifyPopupLogFileSizeToplimit()));
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/DiskMonitor",
                                         "com.KylinOsManager.DiskMonitor",
                                         "LogFileSizeToplimit",
                                         this,
                                         SLOT(on_NotifyPopupLogFileSizeToplimit()));
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/DiskMonitor",
                                         "com.KylinOsManager.DiskMonitor",
                                         "LogFileExecption",
                                         this,
                                         SLOT(on_NotifyPopupLogFileException()));
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/DiskMonitor",
                                         "com.KylinOsManager.DiskMonitor",
                                         "DiskUsageRateToplimit",
                                         this,
                                         SLOT(on_NotifyPopupDiskUsageRateToplimit()));
    addWhiteToNetCtlFile();
}

void SessionClient::initHotplugClient() {
    // 连接信号槽
    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_USB_HOT_INFO_TOOL,
                                         dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, "newDeviceConnectSignal", this,
                                         SLOT(onNewDeviceConnectSlot(QString)));
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications", "ActionInvoked", this,
                                          SLOT(actionInvokeSlot(quint32,QString)));
}

void SessionClient::onNewDeviceConnectSlot(QString devString) {
    qDebug() << "疑似扫描仪接入：" << devString;
    if(!devString.contains("upgradeable")){
        qDebug() << "不可升级";
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(devString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject devJson = jsonValue.toObject();
        if (!(devJson["upgradeable"].toBool())){
            return;
        }else{
            switch (devJson["type"].toInt()) {
            case 0:
                m_devType = "None";
                qDebug() << "没类型";
                break;
            case 1:
                m_devType = "printer";
                qDebug() << "打印机";
                break;
            case 2:
                m_devType = "scanner";
                qDebug() << "扫描仪";
                break;
            case 3:
                m_devType = "graphiccard";
                break;
            default:
                break;
            }
            m_devModel = devJson["model"].toString();
            QJsonArray driverArray = devJson["driverNameList"].toArray();
            QJsonObject driverObject = driverArray[0].toObject();
            m_driverInfo = driverObject["driverName"].toString().remove(";") + "=" + driverObject["version"].toString();
            qDebug() << "待安装：" << m_driverInfo;
            m_driverSize = driverObject["size"].toString();
            m_installer = new AptInstaller(m_driverInfo);
            if (oninstall == driverObject["driverName"].toString().remove(";") || m_devType == "printer"){
                return;
            }
        }
    }

    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("The system detects that a new device has an installable driver. Do you want to install?"));
    args.push_back(QStringList() << "kylin-os-manager-device-manager-close" << tr("Close") << "kylin-os-manager-device-manager-install" << tr("Install"));
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)0);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
}

void SessionClient::actionInvokeSlot(quint32 id, QString actionkey) {
    if(actionkey == "kylin-os-manager-device-manager-close"){
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "CloseNotification");
        QList<QVariant> args;
        args.push_back(id);
        message.setArguments(args);
        auto pending = QDBusConnection::sessionBus().asyncCall(message);
    }

    if (actionkey == "kylin-os-manager-device-manager-install"){
        connect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
        connect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
        m_installer->startWorker();
        oninstall = m_driverInfo.split("=")[0];

        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "Notify");
        QList<QVariant> args;
        args.push_back(tr("Kylin Os Manager"));
        args.push_back((quint32)0);
        args.push_back("kylin-os-manager");
        args.push_back(tr("Device Manager"));
        args.push_back(tr("Start installing the corresponding driver"));
        args.push_back(QStringList());
        args.push_back(QMap<QString, QVariant>());
        args.push_back((qint32)-1);
        message.setArguments(args);

        auto pending = QDBusConnection::sessionBus().asyncCall(message);
        auto watcher = new QDBusPendingCallWatcher(pending);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (!watcher->isValid() || watcher->isError()) {
                qCritical() << "Notify D-Bus share object is error.";
                return;
            }
            QDBusPendingReply<quint32> reply = *watcher;
            if (!reply.isValid() || reply.isError()) {
                qCritical() << "Notify D-Bus return vaule is not valid.";
                return;
            }
        });
    }
    if(actionkey == "kylin-os-manager-device-manager-open-scanner"){
        QProcess proc;
        proc.closeWriteChannel();
        proc.startDetached("kylin-scanner");
    }

    if (actionkey == "kylin-os-manager-junk-clean-one-click-clean") {
        QProcess process;
        process.setProgram("kylin-os-manager");
        process.setArguments({"--jumpTab=JunkClean", "--pluginMessage=MaxDiskCapacity"});
        process.startDetached();
    }
}

void SessionClient::showSuccessMsg() {
    oninstall = "";
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("Driver Installed"));
    if(m_devType == "scanner"){
        args.push_back(QStringList() << "kylin-os-manager-device-manager-open-scanner" << tr("Open Scanner"));
    }else{
        args.push_back(QStringList());
    }
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
    // 让服务端刷新设备信息
    if(DATABASE_INSTANCE.initDatabase() == DB_OP_SUCC){
        QStringList data;
        data.append(m_devType);
        data.append(m_devModel);
        data.append(m_driverInfo.split("=")[0]);
        data.append(m_driverInfo.split("=")[1]);
        data.append(m_driverSize);
        DATABASE_INSTANCE.insertDataSlot(data);
    }

    QDBusInterface iface(dbus::DBUS_SERVICENAME, dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("installSucceed", m_driverInfo);
    } else {
        qWarning() << "Failed to connect to existing service";
    }
}

void SessionClient::showFailMsg() {
    oninstall = "";
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("Driver install failed."));
    args.push_back(QStringList());
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
}

void SessionClient::on_RunGlobalFeedback(int mode, QString error_code, QString exec_name, QString path, QString restart) {
    QProcess process;
    process.setProgram("/opt/kylin-os-manager/tools/service-support-mini");
    process.setArguments({
        QString("-mode=%1").arg(mode),
        QString("-code=%1").arg(error_code),
        QString("-exe=%1").arg(exec_name),
        QString("-path=%1").arg(path),
        QString("-restart=%1").arg(restart)
    });
    process.startDetached();
}

void SessionClient::on_NotifyPopupLogFileSizeToplimit() {
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Kylin Os Manager"));
    args.push_back(tr("Abnormal writing of terminal log, please contact the administrator"));
    QStringList actions;
    actions.push_back("kylin-os-manager-junk-clean-one-click-clean");
    actions.push_back("One click cleaning");
    args.push_back(actions);
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();

        if (watcher->isError() || !watcher->isValid()) {
            qCritical() << "Log file exception popup window fail: interface call fail.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (reply.isError() || !reply.isValid()) {
            qCritical() << "Log file exception popup window fail: get interface return value fail.";
            return;
        }
        if (reply.count() <= 0) {
            qCritical() << "Log file exception popup window fail: interface return value count error.";
            return;
        }

        auto id = reply.argumentAt<0>();
        qInfo() << "Popup log file exception notify window(id = " << id << ")";
    });
}

void SessionClient::on_NotifyPopupLogFileException() {
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Kylin Os Manager"));
    args.push_back(tr("Detected system abnormality, please contact the technical service immediately or call 400!"));
    args.push_back(QStringList());
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();

        if (watcher->isError() || !watcher->isValid()) {
            qCritical() << "Log file exception popup window fail: interface call fail";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (reply.isError() || !reply.isValid()) {
            qCritical() << "Log file exception popup window fail: get interface return value fail";
            return;
        }
        if (reply.count() <= 0) {
            qCritical() << "Log file exception popup window fail: interface return value count error";
            return;
        }

        auto id = reply.argumentAt<0>();
        qInfo() << "Popup log exception notify window(id = " << id << ")";
    });
}

void SessionClient::on_NotifyPopupDiskUsageRateToplimit() {
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Kylin Os Manager"));
    args.push_back(tr("The avaliable disk space is less than 20%, please clean it up in a timely manner"));
    QStringList actions;
    actions.push_back("kylin-os-manager-junk-clean-one-click-clean");
    actions.push_back("One click cleaning");
    args.push_back(actions);
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();

        if (watcher->isError() || !watcher->isValid()) {
            qCritical() << "Disk usage rate toplimit popup window fail: interface call fail.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (reply.isError() || !reply.isValid()) {
            qCritical() << "Disk usage rate toplimit popup window fail: get interface return value fail.";
            return;
        }
        if (reply.count() <= 0) {
            qCritical() << "Disk usage rate toplimit popup window fail: interface return value count error.";
            return;
        }

        auto id = reply.argumentAt<0>();
        qInfo() << "Popup disk usage rate toplimit notify window(id = " << id << ")";
    });
}
void SessionClient::addWhiteToNetCtlFile()
{
    /* 联网控制白名单加载 */
    // 判断联网控制是否开启
    bool netContorlIsOpen = false;
    QProcess processNetControlCheck;
    processNetControlCheck.start("getstatus");
    processNetControlCheck.waitForFinished();
    QString strOutPut = processNetControlCheck.readAllStandardOutput();
    if (strOutPut.isEmpty()) {
        qDebug() << " process 'getstatus' get info error " << processNetControlCheck.readAllStandardError();
    } else {
        QStringList list = strOutPut.split("\n");
        for(QString line : list) {
            netContorlIsOpen = false;
            if (line.contains("net control")) {
                if (!line.contains("off")) {
                    netContorlIsOpen = true;
                    qDebug() << " kyset netControl Is Open ";
                } else {
                    netContorlIsOpen = false;
                    qDebug() << " kyset netControl Is Close";
                }
                break;
            }
        }
    }
    if (!netContorlIsOpen) {
        qDebug() << " NetWorkCheck Is Close, Add white failed ";
        return;
    }
    QString filePath = "/tmp/kom-rc-tool";
    QDir dir(filePath);
    if (!dir.exists()) {
        dir.mkpath(filePath);
    }
    QString toolDataPath = "/tmp/kom-rc-tool/kom-rc-tool-data";
    QString toolDataPathDelete = toolDataPath + " (deleted)";
    QFile toolDatafile(toolDataPath);
    QFile toolDataDeletefile(toolDataPathDelete);

    if (!toolDataDeletefile.exists()) {
        if (!toolDataDeletefile.open(QIODevice::ReadWrite)) {
            qDebug() << " open kom-rc.ini file error ";
        } 
        toolDataDeletefile.close();
    }
    
    if (!toolDatafile.exists()) {    
        if (!toolDatafile.open(QIODevice::ReadWrite)) {
            qDebug() << " open kom-rc.ini file error ";
        } 
        toolDatafile.close();
    }

    addWhiteToKysecNetctl(toolDataPath);
    addWhiteToKysecNetctl(toolDataPathDelete);
    
    toolDatafile.remove();
    toolDataDeletefile.remove();
}

void SessionClient::addWhiteToKysecNetctl(QString filePath)
{
    void *securityFd = dlopen(SECURITY_LIB, RTLD_LAZY);
    if (!securityFd) {
        char *err = dlerror();
        printf("%s : %s", "dlopen error", err);
    } else {
        // /tmp/kom-rc-tool/kom-rc-tool-data 或者 /tmp/kom-rc-tool/kom-rc-tool-data (deleted)
        func fun;
        *(void **) (&fun) = dlsym(securityFd, "kdk_process_enable_networking");

        std::string str = filePath.toStdString();
        const char * filePathChar = str.c_str();
        
        qDebug() << " filePath : " << filePath;
        qDebug() << " kom-rc-tool-data filePath : " << filePathChar;

        int res = fun(filePathChar);
        if (res != 0) {
            qCritical() << " 联网控制添加白名单失败！";
        } else {
            qDebug() << " 联网控制添加白名单成功！";
        }
    }
}
