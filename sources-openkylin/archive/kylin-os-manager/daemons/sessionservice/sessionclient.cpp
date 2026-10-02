#include "sessionclient.h"
#include <QDir>
#include <QDateTime>
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
#include "kom-configure.h"
#include "kom-utils.h"
#include "database/drivermanagerdatabase.h"
#include "../service/dbus_args.h"

#define SECURITY_LIB "/usr/lib/kysdk/kysdk-security/libkyprocesssec.so"
#define LOG_SIZE_EXCEPTION_POPUP_CONFIG_FILE_NAME "log_size_exception_popup.conf"
#define DISK_SPACE_FULL_POPUP_CONFIG_FILE_NAME "disk_space_full_popup.conf"
typedef int (*func)(const char *filepath);

SessionClient::SessionClient(QObject *parent)
    : QObject(parent)
{
    StartRootService();
    initHotplugClient();

#ifndef ICBC_ENVIRONMENT
    QDBusConnection::systemBus().connect("",
                                         "/com/KylinOsManager/ServiceSupport",
                                         "com.KylinOsManager.ServiceSupport",
                                         "RunGlobalFeedback",
                                         this,
                                         SLOT(on_RunGlobalFeedback(int,QString,QString,QString,QString,QString)));
#endif
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
}

void SessionClient::initHotplugClient() {
    qDebug() << "[SessionClient] initHotplugClient called, thread=" << QThread::currentThread();
    // 连接信号槽
    bool connected1 = QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_USB_HOT_INFO_TOOL,
                                         dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, "newDeviceConnectSignal", this,
                                         SLOT(onNewDeviceConnectSlot(QString)));
    qDebug() << "[SessionClient] Connected to newDeviceConnectSignal:" << connected1;
    
    bool connected2 = QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications", "ActionInvoked", this,
                                          SLOT(actionInvokeSlot(quint32,QString)));
    qDebug() << "[SessionClient] Connected to ActionInvoked signal:" << connected2;
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
                return;
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
            m_driverInfo.clear();
            for (int i = 0; i < driverArray.size(); i++)
            {
                QJsonObject driverObject = driverArray[i].toObject();
                QString driverInfo = driverObject["driverName"].toString() + "=" + driverObject["version"].toString();
                m_driverInfo.append(driverInfo);
            }
            qDebug() << "待安装：" << m_driverInfo;
            // 判断待安装的包本地是否已经安装，如果所有包都已安装且版本正确才return
            // 如果某个包缺失或版本不匹配，需要继续安装流程
            QStringList packagesToInstall;
            
            for (int i = 0; i < m_driverInfo.length(); i++)
            {
                qDebug() << "当前检测包：  " << m_driverInfo[i];
                QString packageName = m_driverInfo[i].split("=")[0];
                QString pkgversion = m_driverInfo[i].split("=")[1];
                QDBusConnection bus1 = QDBusConnection::systemBus();
                QDBusInterface manager1("com.kylin.UpgradeStrategies", "/com/kylin/UpgradeStrategies", "com.kylin.UpgradeStrategies.interface", bus1);
                QDBusMessage response1 = manager1.call("PackageInstallStatus", packageName, "");
                int pkg_exist_res = response1.arguments().at(0).toInt();
                
                // 检查包是否已安装且版本正确
                bool needInstall = false;
                if(pkg_exist_res == 0){
                    QDBusConnection bus2 = QDBusConnection::systemBus();
                    QDBusInterface manager2("com.kylin.UpgradeStrategies", "/com/kylin/UpgradeStrategies", "com.kylin.UpgradeStrategies.interface", bus2);
                    QDBusMessage response2 = manager2.call("PackageInstallVersion", packageName);
                    QString version_res = response2.arguments().at(0).toString();
                    if(version_res == pkgversion){
                        qDebug() << m_driverInfo[i] << "包本地存在且版本正确，跳过安装";
                        // 不添加到待安装列表，但继续检查其他包
                        continue;
                    } else {
                        qDebug() << m_driverInfo[i] << "包本地存在但版本不同，需要安装";
                        needInstall = true;
                    }
                } else {
                    qDebug() << m_driverInfo[i] << "包本地不存在，需要安装";
                    needInstall = true;
                }
                
                // 如果需要安装，检查这个包是否可以在线安装
                if (needInstall) {
                    QDBusConnection bus3 = QDBusConnection::systemBus();
                    QDBusInterface manager3("com.kylin.UpgradeStrategies", "/com/kylin/UpgradeStrategies", "com.kylin.UpgradeStrategies.interface", bus3);
                    QDBusMessage response3 = manager3.call("PackageIsOnlineInstallable", packageName, pkgversion);
                    bool result = response3.arguments().at(0).toBool();
                    if(result == false){
                        qDebug() << m_driverInfo[i] << "包无法在线安装，跳过该包";
                        // 继续检查其他包，不直接返回
                        continue;
                    }
                    qDebug() << m_driverInfo[i] << "包可以在线安装";
                    // 添加到待安装列表
                    packagesToInstall.append(m_driverInfo[i]);
                }
            }
            
            // 如果没有任何需要安装的包（所有包都已安装且版本正确），直接返回
            if (packagesToInstall.isEmpty()) {
                qDebug() << "所有驱动包都已安装且版本正确，无需安装";
                m_driverInfo.clear();
                return;
            }
            
            // 更新待安装列表为需要安装的包
            m_driverInfo = packagesToInstall;
            qDebug() << "[SessionClient] Final packages to install:" << m_driverInfo;
            m_driverSize = "1";
            
            qDebug() << "[SessionClient] Creating new AptInstaller, previous m_installer=" << (void*)m_installer;
            // 如果之前的 installer 存在，说明之前的安装流程还未完成
            // 由于 AptInstaller 使用了 moveToThread，对象在另一个线程中，不能安全删除
            // 但是如果 oninstall 为空，说明用户没有点击安装，应该允许重新创建
            // 只有在 oninstall 不为空时（正在安装中），才跳过
            if (m_installer != nullptr && !oninstall.isEmpty()) {
                qDebug() << "[SessionClient] Previous installer still exists and installation in progress (oninstall=" << oninstall << "), skipping to avoid crash.";
                m_driverInfo.clear();
                return;
            }
            // 如果 installer 存在但 oninstall 为空，说明之前的通知被关闭或用户未操作
            // 可以安全地清除旧的 installer（它会自己清理），然后创建新的
            if (m_installer != nullptr) {
                qDebug() << "[SessionClient] Previous installer exists but no installation in progress, clearing it...";
                // 断开信号连接，避免旧 installer 的回调
                disconnect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
                disconnect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
                // 让旧的 installer 自己清理（它会通过 finished 信号自己调用 deleteLater）
                // 但我们立即创建新的，所以先清除指针
                m_installer = nullptr;
                qDebug() << "[SessionClient] Previous installer cleared, will create new one";
            }
            m_installer = new AptInstaller(m_driverInfo);
            qDebug() << "[SessionClient] New AptInstaller created:" << (void*)m_installer;
            
            QString driverInfoStr = m_driverInfo.join(",");
            qDebug() << "[SessionClient] Checking if should skip: oninstall=" << oninstall << "driverInfoStr=" << driverInfoStr << "devType=" << m_devType;
            if (oninstall == driverInfoStr || m_devType == "printer"){
                qDebug() << "[SessionClient] Skipping installation (already installing or printer type)";
                m_driverInfo.clear();
                return;
            }
            qDebug() << "[SessionClient] Ready to show installation notification, will proceed to notification code";
            qDebug() << "[SessionClient] Exiting else block, for loop continues";
        }
    }
    qDebug() << "[SessionClient] For loop finished, proceeding to notification code. m_driverInfo=" << m_driverInfo << "m_installer=" << (void*)m_installer;

    if (m_installer == nullptr || m_driverInfo.isEmpty()) {
        qDebug() << "[SessionClient] Cannot show notification: installer=" << (void*)m_installer << "driverInfo empty=" << m_driverInfo.isEmpty();
        return;
    }

    qDebug() << "[SessionClient] Creating notification message...";
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
    qDebug() << "[SessionClient] actionInvokeSlot called: id=" << id << "actionkey=" << actionkey << "thread=" << QThread::currentThread();
    
    if(actionkey == "kylin-os-manager-device-manager-close"){
        qDebug() << "[SessionClient] Close notification requested, id=" << id;
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "CloseNotification");
        QList<QVariant> args;
        args.push_back(id);
        message.setArguments(args);
        auto pending = QDBusConnection::sessionBus().asyncCall(message);
        // 用户关闭了通知，如果没有正在安装，清除 installer 指针
        // 这样可以允许下次热插拔时重新创建 installer 和显示通知
        if (oninstall.isEmpty() && m_installer != nullptr) {
            qDebug() << "[SessionClient] User closed notification and no installation in progress, clearing installer";
            disconnect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
            disconnect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
            m_installer = nullptr;
        }
    }

    if (actionkey == "kylin-os-manager-device-manager-install"){
        // 防止同一逻辑点击触发两次：通知服务有时会对同一操作发送两次 ActionInvoked（装系统后首次常见），导致“开始安装驱动”弹窗出现两次
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (m_lastActionInvokedId == id && m_lastActionInvokedKey == actionkey && (now - m_lastActionInvokedTime) < 1000) {
            qDebug() << "[SessionClient] Ignoring duplicate ActionInvoked (id=" << id << ") within 1s";
            return;
        }
        m_lastActionInvokedId = id;
        m_lastActionInvokedKey = actionkey;
        m_lastActionInvokedTime = now;

        qDebug() << "[SessionClient] Install button clicked, checking installation status...";
        qDebug() << "[SessionClient] Current oninstall=" << oninstall;
        qDebug() << "[SessionClient] Current m_driverInfo=" << m_driverInfo;
        qDebug() << "[SessionClient] m_installer pointer=" << (void*)m_installer;
        
        // 防止重复点击：如果已经在安装中，直接返回
        QString currentDriverInfo = m_driverInfo.join(",");
        qDebug() << "[SessionClient] currentDriverInfo=" << currentDriverInfo;
        
        if (!oninstall.isEmpty() && oninstall == currentDriverInfo) {
            qDebug() << "[SessionClient] Driver is already installing, skip duplicate request. oninstall=" << oninstall;
            return;
        }
        
        if (m_installer == nullptr) {
            qDebug() << "[SessionClient] Installer is null, cannot start installation";
            return;
        }
        
        qDebug() << "[SessionClient] Starting installation process...";
        qDebug() << "[SessionClient] Connecting signals to installer...";
        // 断开之前的连接（如果存在），避免重复连接
        disconnect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
        disconnect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
        // 重新连接信号
        bool connect1 = connect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
        bool connect2 = connect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
        qDebug() << "[SessionClient] Signal connections: succeed=" << connect1 << "failed=" << connect2;
        
        qDebug() << "[SessionClient] Starting installer worker...";
        m_installer->startWorker();
        
        qDebug() << "[SessionClient] Setting oninstall to:" << currentDriverInfo;
        oninstall = m_driverInfo.join(",");
        
        qDebug() << "[SessionClient] Sending 'Start installing the corresponding driver' notification...";
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
                qCritical() << "[SessionClient] Notify D-Bus share object is error.";
                return;
            }
            QDBusPendingReply<quint32> reply = *watcher;
            if (!reply.isValid() || reply.isError()) {
                qCritical() << "[SessionClient] Notify D-Bus return vaule is not valid.";
                return;
            }
            qDebug() << "[SessionClient] Notification sent successfully, notification id=" << reply.value();
        });
        qDebug() << "[SessionClient] Installation process initiated, notification message sent";
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
    for (int i = 0; i < m_driverInfo.length(); i++)
    {
        if(DATABASE_INSTANCE.initDatabase() == DB_OP_SUCC){
            QStringList data;
            data.append(m_devType);
            data.append(m_devModel);
            data.append(m_driverInfo[i].split("=")[0]);
            data.append(m_driverInfo[i].split("=")[1]);
            data.append(m_driverSize);
            DATABASE_INSTANCE.insertDataSlot(data);
        }
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
    m_installer = nullptr;  // 安装失败后，清除 installer 指针，让旧的 installer 自然清理
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
    if (!IsPopup()) {
        return;
    }

    qInfo() << "run service support mini";

    QProcess process;
    process.setProgram("/usr/lib/kylin-os-manager/bin/service-support-mini");
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
    if (!IsPopup()) {
        return;
    }
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
    if (!IsPopup()) {
        return;
    }

    if (service_support_popup_.isNull()) {
        service_support_popup_ = new ServiceSupportPopup;
        service_support_popup_->show();
    } else {
        service_support_popup_->show();
    }
}

void SessionClient::on_NotifyPopupDiskUsageRateToplimit() {
    if (!IsPopup()) {
        return;
    }
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

bool SessionClient::IsPopup() {
    // 判断配置文件中的配置
    kom::Configure conf;
    auto allow_popup = conf.value(CONFIG_SETUP, CONFIG_SETUP_ALLOW_POPUP, true).toBool();
    if (!allow_popup) {
        qInfo() << "[SessionClient][IsPopup] Popup are currently not allowed, configuration option is false";
        return false;
    }

    // 判断是否有全屏窗口
    if (kom::KomUtils::ExistsFullScreenWindow()) {
        qInfo() << "[SessionClient][IsPopup] Popup are currently not allowed, exists full screen window";
        return false;
    }

    // 判断是否为 live 环境
    if (kom::KomUtils::LiveEnv()) {
        qInfo() << "[SessionClient][IsPopup] Popup are currently not allowed, in a live environment";
        return false;
    }

    return true;
}

void SessionClient::StartRootService() {
    qInfo() << "Start the root service by calling the interface...";
    QDBusMessage msg = QDBusMessage::createMethodCall("com.kylin-os-manager.startup", "/com/KylinOsManager/Startup", "com.KylinOsManager.Startup", "Start");
    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qWarning() << "The call to start the root service failed: " << reply.errorMessage();
        return;
    }
    qInfo() << "Call interface to start root service successfully";
}
