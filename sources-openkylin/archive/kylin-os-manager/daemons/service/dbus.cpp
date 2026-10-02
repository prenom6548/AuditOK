#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include "dbus.h"
#include "feedbackmanager.h"
#include "driver-manager-service/drivermanagerservicedbus.h"
#include "junk_clean_interface.h"
#include "tool_box_interface.h"
#include "kom-service.h"
#include "informationitem.h"
#include "settings.h"
#include "startup_interface.h"

#define CUSTOMIZED_COLLECT_KYLIN_SOFTWATE_PROPERTIES "/var/log/kylin-software-properties/kylin-software-properties.log"

constexpr int ANNEX_FILE_SIZE_TOPLIMIT = 100 * 1024 * 1024;    // 100M

using namespace dbus;

void Tool::collectionMessage(int type , const QString &path,int detailed)
{
    //if (!isAllowedCaller()) {
    //    return;
    //}

    //InformationItem *it = Settings::getInfoItemMap().value(type);
    //switch (it->collectionType()) {
    //case InformationItem::File:
    //    collectingFile(path, it->getMessage(), detailed);
    //    break;
    //case InformationItem::CMD:
    //    collectingCmd(path, it->getMessage());
    //    break;
    //case InformationItem::Others:
    //    collectingOther(path, it->getMessage(), it->getItemName());
    //    break;
    //default:
    //    break;
    //}
}

void Tool::change3rdpartyConf(const QString &group, const QString &key, const QString &value) {
    QSettings setting("/etc/kylin-os-manager/3rdparty.conf", QSettings::IniFormat);
    setting.beginGroup(group);
    setting.setValue(key, value);
    setting.endGroup();
    setting.sync();
}

/*====================非dbus接口======================*/

void Dbus::creatDbusServer() {
    Tool *toolBus = new Tool(this);
    DriverManagerServiceTool *driverManagerServiceToolBus = new DriverManagerServiceTool(this);

    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.registerService(DBUS_SERVICENAME)) {
        qDebug() << "creat dbus service error";
    }
    if (!bus.registerObject(DBUS_PARH_TOOL, DBUS_INTERFACE_TOOL, toolBus,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << "creat dbus interface error" << DBUS_INTERFACE_TOOL;
    }
    if (!bus.registerObject(DBUS_DRIVER_INFO_TOOL, DBUS_DRIVER_INFO_INTERFACE_TOOL, driverManagerServiceToolBus,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << "creat dbus interface error" << DBUS_DRIVER_INFO_INTERFACE_TOOL;
    }
    if (!bus.registerObject(dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, this,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << "creat hotplug dbus object error";
    }

    // 注册垃圾清理接口
    auto *junk_clean_object = new QObject(this);
    auto *junk_clean_adaptor = new junk_clean::JunkCleanAdaptor(junk_clean_object);
    auto ok = bus.registerObject(JUNK_CLEAN_DBUS_PATH,
                                 JUNK_CLEAN_DBUS_INTERFACE,
                                 junk_clean_adaptor,
                                 QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots);
    if (!ok) {
        qCritical() << "Register junk clean dbus interface fail: " << bus.lastError();
    }

    // 注册百宝箱接口
    auto *tool_box_object = new QObject(this);
    auto *tool_box_adaptor = new tool_box::ToolBoxAdaptor(tool_box_object);
    ok = bus.registerObject(TOOL_BOX_DBUS_PATH,
                       TOOL_BOX_DBUS_INTERFACE,
                       tool_box_adaptor,
                       QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots);
    if (!ok) {
        qCritical() << "Register tool box dbus interface fail: " << bus.lastError();
    }

    qInfo() << "complete obtaining system information and registering dbus interface";
}

Tool::Tool(QObject *parent) : QObject(parent),QDBusContext()
{
    connect(FeedbackManager::getInstance(), &FeedbackManager::creatFinish, this,
            [=](FeedBackFinishType type, QString msg) {
                QList<QVariant> varList;
                if (type == Success) {
                    QStringList list = msg.split("$$$");
                    varList.append(list.last());
                    varList.append(list.first());
                } else if (type == Oversize) {
                    varList.append("Oversize");
                    varList.append(msg);
                } else if (type == UploadFail) {
                    varList.append("UploadFail");
                    varList.append(msg);
                } else {
                    varList.append("error code : " + QString::number(type));
                    varList.append(msg);
                }
                QDBusMessage send =
                    QDBusMessage::createSignal(DBUS_PARH_TOOL, DBUS_INTERFACE_TOOL, "creatBugForZentaoResult");
                send.setArguments(varList);
                QDBusConnection::systemBus().send(send);
                //                emit creatBugForZentaoResult(list.last(), list.first());
            });
}

/*
 * 获取目录下以文件名开头的所有文件：
 * /var/log/syslog
 *     |
 *     v
 * 匹配 /var/log/ 目录下所有 syslog 开头的文件名
 *     |
 *     v
 * /var/log/syslog、/var/log/syslog.1、/var/log/syslog.2.gz...
 */
QStringList Tool::getFileNameFromDir(const QString &arg) const
{
    QStringList tmp;
    QFileInfo info(arg);
    QString path = info.absolutePath();
    tmp =
        QDir(path).entryList(QStringList() << QString(info.fileName() + "*"), QDir::Files | QDir::Readable, QDir::Name);
    for (QString &str : tmp) {
        str = path + "/" + str;
    }
    return tmp;
}

DbusManager::DbusManager() {
    RegisterStartupService();

    QThread *thread = new QThread(this);
    Dbus *dbus = new Dbus();
    connect(this, &DbusManager::creatDbusServer, dbus, &Dbus::creatDbusServer);
    connect(thread, &QThread::finished, dbus, &Dbus::deleteLater);
    dbus->moveToThread(thread);
    thread->start();
    emit creatDbusServer();
}

void DbusManager::RegisterStartupService() {
    QDBusConnection conn = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "StartUpSystemBus");
    bool ok = conn.registerObject("/com/KylinOsManager/Startup",
                                  "com.KylinOsManager.Startup",
                                  new startup::Startup(this),
                                  QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots);
    if (!ok) {
        qCritical() << "register startup dbus interface error: " << conn.lastError();
        return;
    }
    if (!conn.registerService("com.kylin-os-manager.startup")) {
        qCritical() << "Register startup service error: " << conn.lastError();
    }
}

/*
 * @param detailed 当匹配到多个文件时，收集几个个文件
 * @example
 * ("/tmp/kom-pfb_snowsi_2025.09.12_15.23.05/kylin-os-manager/系统日志/系统运行日志/", "/var/log/syslog", 1)
 */
void Tool::collectingFile(const QString &saveDir, const QString &message, int detailed)
{
    if (detailed < 1) {
        detailed = 1;
    }
    if (message.isEmpty()) {
        return;
    }
    QString file = message;
    QStringList list= getFileNameFromDir(file);
    if (list.isEmpty()) {
        list.append(file);
    }

    int count = list.length();
    if (file == "/var/log/syslog") {
        count = qMin(list.length(), detailed);
    }

    QString error;
    for (int i = 0; i < count; i++) {
        QString nowFile = list.at(i);

        /*
         * 对 /var/log/kylin-software-properites/kylin-software-properties.log 做处理
         * 该日志实际的名字为 kylin-software-properties-2024-11-29.log
         * 当收集该日志时，只取最后一天的日志
         */
        if (nowFile == CUSTOMIZED_COLLECT_KYLIN_SOFTWATE_PROPERTIES) {
            QFileInfo file_info(CUSTOMIZED_COLLECT_KYLIN_SOFTWATE_PROPERTIES);
            if (!file_info.exists()) {
                const auto dir = file_info.absoluteDir();

                QStringList name_filters;
                name_filters.push_back("kylin-software-properties-*");
                const auto entrys = dir.entryList(name_filters, QDir::Filter::Files | QDir::Filter::NoSymLinks, QDir::SortFlag::Name);
                if (entrys.empty()) {
                    continue;
                }

                nowFile = QString("%1/%2").arg(dir.absolutePath(), entrys.back());
            }
        }

        // 判断要采集的 文件 / 目录 是否合法
        QFileInfo info(nowFile);
        if (info.isSymLink()) {
            nowFile = info.canonicalFilePath();
            if (nowFile.isEmpty()) {
                error += list.at(i) + " SymLink error ! \n";
                continue;
            }
        }
        if (info.isFile()) {
            // 检查文件大小
            const auto size = info.size();
            if (size > ANNEX_FILE_SIZE_TOPLIMIT) {
                error +=  nowFile + ": File size exceeds 100M, skip collection\n";
                continue;
            }
        }

        // 检测通过，开始采集
        QString cmd = "/usr/bin/cp -r \"" + nowFile + "\" \"" + saveDir + "\"";
        QString res = getCmdMessage(cmd,false);
        if (res.isEmpty()) {
            QProcess proc;
            QString chmod = "/usr/bin/chmod -R a+r " + saveDir + QFileInfo(nowFile).fileName();
            proc.start(chmod);
            proc.waitForFinished();
        } else {
            error += res;
        }
    }

    // 采集完成，修改保存目录下所有子目录的权限为 775
    QProcess p;
    QString command = QString("find %1 -type d -exec chmod 775 {} +").arg(saveDir);
    p.start(command);
    if (!p.waitForFinished()) {
        qCritical() << "[Tool][collectingFile] failed to change subdirectories permissions";
    }

    if (!error.isEmpty()) {
        QFile e(saveDir + "error");
        if (e.open(QIODevice::WriteOnly | QIODevice::Append)) {
            e.write(error.toLocal8Bit());
            e.close();
        } else {
            qDebug() << "creat error file faild:" << saveDir + "error";
        }
    }
}

QString Tool::getCmdMessage(const QString &message, bool hasType)
{
    QProcess proc;
    proc.setProgram("bash");
    proc.setArguments({"-c", message});
    proc.start();
    proc.waitForFinished();
    QString res = proc.readAllStandardOutput();
    if (res.isEmpty()) {
        res = "0" + proc.readAllStandardError();
    } else {
        res = "1" + res;
    }
    if (!hasType) {
        if (res.at(0) == '0') {
            return "";
        }
        res = res.mid(1);
    }
    return res;
}

// BUG: 在测试时发现，有些版本的系统上有时有些命令会卡住（lshw -C Display），而且 kill -9 也杀不掉
//      而 QProcess 在析构时，如果命令没有结束，会调用 close 来回收资源，而此时由于命令没法结束， close 函数会阻塞，导致无法析构
//      所以即使给 QProcess 的 waitForFinished 函数设置了超时，超时后调用 kill 函数也无法规避该问题
//      从而导致 getCmdMessage 函数会阻塞，导致 dbus 无法正常返回而超时
void Tool::collectingCmd(const QString &saveDir, const QString &message) {
    QString res = getCmdMessage(message, true);
    if (res.size() < 2) {
        /*
         * getCmdMessage 函数的返回值格式：
         * 0xxxxxx 命令的错误输出
         * 1xxxxxx 命令的正确输出
         */
        return;
    }

    const auto command_output = res.mid(1, -1);
    const auto file_path = saveDir + "message";
    QFile file(file_path);
    if (!file.open(QIODevice::NewOnly)) {
        qCritical() << "Collect command output annex open file fail.[ "<< message << " ]";
        return;
    }
    const auto ba = command_output.toLocal8Bit();
    const auto count = file.write(ba);
    if (count < ba.size()) {
        qWarning() << "[collect command output] incomplete writing to storage file, " << "command: " << "message" << ", written size: " << count << ", total size: " << ba.size();
    }
    file.close();
}

void Tool::collectingOther(const QString &saveDir, const QString &message, const QString &name)
{
    // 采集磁盘 smart 信息
    if (message == "DISK_SMART") {
        collectDiskSmartStatus(saveDir);
        return;
    }

    QString fileName = "/var/log/hedron/policy_cache.json";
    QString key1 = "after";
    QString value1 = "";
    QString key2 = "type_code";
    QString value2 = "";
    QString key3 = "dest";
    QString res;
    if (name == "防火墙脚本") {
        value1 = "script";
        value2 = "8401";
        res = getPathFromJson(fileName, key1, value1, key3, key2, value2);
    } else if (name == "外设策略文件") {
        value2 = "85**";
        res = getPathFromJson(fileName, key2, value2, key3);
    } else if (name == "应用运行黑名单文件") {
        value2 = "8205";
        res = getPathFromJson(fileName, key2, value2, key3);
    } else if (name == "应用防卸载文件") {
        value1 = "softwareUninstallDisable";
        res = getPathFromJson(fileName, key1, value1, key3);
    }
    if (!value2.contains('*')) {
        collectingFile(saveDir, res, 1);
    } else {
        QFile file(saveDir + "message");
        if (!file.open(QIODevice::NewOnly)) {
            qDebug() << "creat user data file error :" << saveDir + fileName;
            return;
        }
        file.write(res.toLocal8Bit());
        file.close();
    }
}

/*
 * 文件格式:
 * ++++++
 * disk: /dev/sda, type: disk
 * standard output
 * standard error
 *
 *
 * ++++++
 * disk: /dev/sdb, type: disk
 * standard output
 * standard error
 */
void Tool::collectDiskSmartStatus(const QString &saveDir) {
    QString content;
    QTextStream ss(&content);

    QProcess p;
    p.setProgram("lsblk");
    p.setArguments({"-d", "-n", "-o", "PATH,TYPE"});
    p.start();
    p.waitForFinished(5000);
    if (p.exitStatus() != QProcess::ExitStatus::NormalExit) {
        qCritical() << "[collect disk smart status] error reading system disk.";
        return;
    }

    auto standardOutput = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
    auto disks = standardOutput.split('\n');
    disks.removeAll("");
    for (const auto &d : disks) {
        auto words = d.split(' ');
        words.removeAll("");
        if (words.size() < 2) {
            continue;
        }

        QString diskPath = words.at(0);
        QString diskType = words.at(1);

        QProcess p1;
        p1.setProgram("smartctl");
        p1.setArguments({"-a", "-H", diskPath});
        p1.start();
        p1.waitForFinished(5000);
        if (p1.exitStatus() != QProcess::ExitStatus::NormalExit) {
            qWarning() << "[collect disk smart status] smartctl command exited abnormally.";
        }
        const auto standardOutput = QString::fromUtf8(p1.readAllStandardOutput()).trimmed();
        const auto standardError = QString::fromUtf8(p1.readAllStandardError()).trimmed();

        ss << "++++++" << "\n";
        ss << "disk: " << diskPath << ", type: " << diskType << "\n";
        if (!standardOutput.isEmpty()) {
            ss << standardOutput << "\n";
        }
        if (!standardError.isEmpty()) {
            ss << standardError << "\n";
        }
        ss << "\n\n";
    }
    content = content.trimmed();

    const auto filePath = QDir(saveDir).absoluteFilePath("message");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "[collect disk smart status] error opening storage file.";
        return;
    }
    const auto ba = content.toLocal8Bit();
    const auto count = file.write(ba);
    if (count < ba.size()) {
        qWarning() << "[collect disk smart status] incomplete writing to storage file, written size: " << count << ", total size: " << ba.size();
    }
    file.close();
}

QString Tool::getPathFromJson(QString fn, QString k1, QString v1, QString keyRes, QString k2,
                                              QString v2)
{
    QString cmd = "/usr/bin/cat " + fn;
    QString dbusRes = getCmdMessage(cmd,true);
    if (dbusRes.length() < 1) {
        qDebug() << "dbus interface return string error : getMessage";
        return "";
    }
    if (dbusRes.at(0) == '0') {
        qDebug() << "cat " + fn + " error : getMessage";
        return "";
    }
    QByteArray json = dbusRes.mid(1).toLocal8Bit();
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isArray() && !doc.isObject()) {
        qDebug() << "json error !";
        return "";
    }
    QJsonArray arr = doc.array();
    QString res;
    for (int i = 0; i < arr.size(); i++) {
        QJsonObject jo = arr[i].toObject();
        if (jo.value(k1).toString() == v1) {
            if (!v2.isEmpty()) {
                if (jo.value(k2).toString() == v2) {
                    return jo.value(keyRes).toString();
                }
            } else {
                return jo.value(keyRes).toString();
            }
        }
        if (v1.contains('*')) {
            QString tmp = v1;
            tmp.remove('*');
            if (jo.value(k1).toString().contains(tmp)) {
                res.append("========" + jo.value(k1).toString() + "========\n");
                for (QString key : jo.keys()) {
                    res.append(key + ":" + jo.value(key).toString() + "\n");
                }
            }
        }
    }
    return res;
}

bool Tool::isAllowedCaller()
{
    return true;
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int pid = intf->servicePid(clientService).value();
    /*check caller environ*/
    if (!kom::KomService::checkEnviron(pid)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QString("[%2] dbus method control,env forbidden").arg((pid)));
        return false;
    }
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath = "";
    if (file.exists()) {
        execPath = file.canonicalFilePath();
        foreach (const QString &whitePath, kom::KomService::programWhiteList) {
            if (execPath == whitePath) {
                return true;
            }
        }
    }
    sendErrorReply(QDBusError::ErrorType::Failed,
                   QString("[%2] dbus method control,operation not permitted").arg((execPath)));
    return false;
}
