#include "sessionclient.h"
#include <QDir>
#include <QTextCodec>
#include <QSettings>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QFile>
#include <stdio.h>
#include <dlfcn.h>
#include "database/drivermanagerdatabase.h"
#include "../service/dbus_args.h"

#define SECURITY_LIB "/usr/lib/kysdk/kysdk-security/libkyprocesssec.so"
#define LOG_SIZE_EXCEPTION_POPUP_CONFIG_FILE_NAME "log_size_exception_popup.conf"
#define DISK_SPACE_FULL_POPUP_CONFIG_FILE_NAME "disk_space_full_popup.conf"
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
                                         SLOT(on_RunGlobalFeedback(int,QString,QString,QString,QString,QString)));
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
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/PerformanceMonitor",
                                         "com.KylinOsManager.PerformanceMonitor",
                                         "MemoryExceedsThreshold",
                                         this,
                                         SLOT(on_MemoryExceedsThreshold()));
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

void SessionClient::on_RunGlobalFeedback(int mode, QString error_code, QString exec_name, QString path, QString restart, QString crash_stack_path) {
    qInfo() << "run service support mini";

    QProcess process;
    process.setProgram("/opt/kylin-os-manager/tools/service-support-mini");
    process.setArguments({
        QString("-mode=%1").arg(mode),
        QString("-code=%1").arg(error_code),
        QString("-exe=%1").arg(exec_name),
        QString("-path=%1").arg(path),
        QString("-restart=%1").arg(restart),
        QString("-crashStackPath=%1").arg(crash_stack_path)
    });
    process.startDetached();
}

void SessionClient::on_NotifyPopupLogFileSizeToplimit() {
    if (isjunkCleanBoxShow()) {
        return;
    }
    bool more_than_three{false};
    do {
        QFile file("/etc/kylin-os-manager/size_exception_files.conf");
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Log file size toplimit open conf file fail";
            break;
        }
        const auto data = file.readAll();
        file.close();

        // 反序列化
        const auto doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qCritical() << "Log file size toplimit unmarshal fail";
            break;
        }
        if (!doc.isObject()) {
            qCritical() << "Log file size toplimit json format error";
            break;
        }

        // 获取异常次数
        const auto root = doc.object();
        if (!root.contains("exception_count")) {
            qCritical() << "Log file size toplimit exception_count field miss";
            break;
        }
        const auto exception_count = root.value("exception_count").toInt();
        exception_count > 3? more_than_three = true: more_than_three = false;
    } while (0);

    if (!more_than_three) {
        // 小于等于 3 次弹气泡弹窗
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "Notify");
        QList<QVariant> args;
        args.push_back(tr("Kylin Os Manager"));
        args.push_back((quint32)0);
        args.push_back("kylin-os-manager");
        args.push_back(tr("Kylin Os Manager"));
        args.push_back(tr("Detected log files that are too large, it is recommended to clean them up"));
        QStringList actions;
        actions.push_back("kylin-os-manager-junk-clean-cancel");
        actions.push_back(tr("Cancel"));
        actions.push_back("kylin-os-manager-junk-clean-one-click-clean");
        actions.push_back(tr("One click cleaning"));
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
    } else {
        // 大于 3 次弹独立弹窗
        if (log_size_exception_popup_.isNull()) {
            log_size_exception_popup_ = new DiskMonitorExceptionPopup(LOG_SIZE_EXCEPTION_POPUP_CONFIG_FILE_NAME);
            log_size_exception_popup_->SetTitle(tr("Detected log files that are too large, please go to Kylin Os Manager for cleaning!"));
            log_size_exception_popup_->Show();
        } else {
            log_size_exception_popup_->Show();
        }
    }
}


bool SessionClient::isjunkCleanBoxShow()
{
    if (!log_size_exception_popup_.isNull() && !log_size_exception_popup_->isHidden()) {
        return true;
    }
    if (!disk_space_full_popup_.isNull() && !disk_space_full_popup_->isHidden()) {
        return true;
    }
    return false;
}

void SessionClient::on_NotifyPopupLogFileException() {
    if (service_support_popup_.isNull()) {
        service_support_popup_ = new ServiceSupportPopup;
        service_support_popup_->show();
    } else {
        service_support_popup_->show();
    }
}

void SessionClient::on_NotifyPopupDiskUsageRateToplimit() {
    if (isjunkCleanBoxShow()) {
        return;
    }
    // 每天只弹1次
    bool ok{false};
    do {
        // 读取文件内容
        QFile file("/etc/kylin-os-manager/disk_capacity_exception.conf");
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Disk usage rate toplimit open config file fail";
            break;
        }
        const auto data = file.readAll();
        file.close();

        // 反序列化
        const auto doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qCritical() << "Disk usage rate toplimit unmarshal fail";
            break;
        }
        if (!doc.isObject()) {
            qCritical() << "Disk usage rate toplimit config file format error";
            break;
        }

        const auto root = doc.object();
        if (!root.contains("exception_count")) {
            qCritical() << "Disk usage rate toplimit exception_count field miss";
            break;
        }

        const auto exception_count = root.value("exception_count").toInt();
        exception_count == 1? ok = true: ok = false;
    } while (0);

    if (ok) {
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "Notify");
        QList<QVariant> args;
        args.push_back(tr("Kylin Os Manager"));
        args.push_back((quint32)0);
        args.push_back("kylin-os-manager");
        args.push_back(tr("Kylin Os Manager"));
        args.push_back(tr("The avaliable disk space is less than %1%, please clean it up in a timely manner").arg(10));
        QStringList actions;
        actions.push_back("kylin-os-manager-junk-clean-cancel");
        actions.push_back(tr("Cancel"));
        actions.push_back("kylin-os-manager-junk-clean-one-click-clean");
        actions.push_back(tr("One click cleaning"));
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
    } else {
        // // 大于 3 次
        // if (disk_space_full_popup_.isNull()) {
        //     disk_space_full_popup_ = new DiskMonitorExceptionPopup(DISK_SPACE_FULL_POPUP_CONFIG_FILE_NAME);
        //     disk_space_full_popup_->SetTitle(tr("The System disk space is tight, please go to Kylin Os Manager for cleaning!"));
        //     disk_space_full_popup_->Show();
        // } else {
        //     disk_space_full_popup_->Show();
        // }
    }
}

void SessionClient::on_MemoryExceedsThreshold() {
    QSettings settings("/etc/kylin-os-manager/kylin-os-manager-plugin.ini", QSettings::Format::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    auto value = settings.value("SystemService/MemoryThresholdMonitor_PromptContent");
    if (value.isNull()) {
        qCritical() << "[Memory exceeds threshold handle]: Failed to obtain prompt information";
        return;
    }
    auto prompt_content = value.toString();

    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back("麒麟管家");
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back("麒麟管家");
    args.push_back(prompt_content);
    args.push_back(QStringList());
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();

        if (watcher->isError() || !watcher->isValid()) {
            qCritical() << "[Memory exceeds threshold handle]: Error in sharing object when calling notification pop-up";
            return;
        }

        QDBusPendingReply<quint32> reply = *watcher;
        if (reply.isError() || !reply.isValid()) {
            qCritical() << "[Memory exceeds threshold handle]: Fail to retrieve the return value from the notification pop-up interface";
            return;
        }
        if (reply.count() <= 0) {
            qCritical() << "[Memory exceeds threshold handle]: Notification pop-up interface return incorrect number of values";
            return;
        }

        auto id = reply.argumentAt<0>();
        qInfo() << "[Memory exceeds threshold handle]: Pop up notification pop-up window: " << id;
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
