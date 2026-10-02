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
#include "dbus.h"
#include "feedbackmanager.h"
#include "driver-manager-service/drivermanagerservicedbus.h"
#include "junk_clean_interface.h"
#include "tool_box_interface.h"
#include "kom-service.h"

using namespace dbus;

/*====================dbus接口======================*/

void Tool::creatBugForZentao(const QString &title, const QString &type, const QString &name, const QString &number,
                             const QString &key)
{
    if (!isAllowedCaller()) {
        return;
    }
    QString tmpTitle = title;
    QString tmpType = type;
    QString tmpKey = key;
    QString tmpName = name;
    QString tmpNumber = number;
    if (tmpTitle.isEmpty()) {
        qDebug() << "标题为空！";
        tmpTitle = "未传入标题";
    }
    if (tmpType.isEmpty()) {
        qDebug() << "类别为空！";
        tmpType = "1-0";
    }
    if (tmpKey.isEmpty()) {
        qDebug() << "标志code为空！";
        tmpKey = "11223344";
    }
    if (tmpName.isEmpty()) {
        qDebug() << "姓名为空！";
        tmpName = "姓名为空";
    }
    if (tmpNumber.isEmpty()) {
        qDebug() << "工号为空！";
        tmpNumber = "工号为空";
    }
    if (!type.contains("-")) {
        tmpType += "-0";
    }
    InformationUserStruct info;
    info.title = tmpTitle.mid(0, 50);
    info.priviteType = tmpType;
    info.details = tmpTitle;
    info.userName = tmpName;
    info.jobNumber = tmpNumber;
    FeedbackManager::getInstance()->setBeFrom(FromDomain, tmpKey);
    FeedbackManager::getInstance()->setSavePath("/tmp");
    FeedbackManager::getInstance()->setUserData(info);
    FeedbackManager::getInstance()->startCollecting();
}

void Tool::collectionMessage(int type , const QString &path,int detailed)
{
    if (!isAllowedCaller()) {
        return;
    }

    InformationItem *it = Settings::getInfoItemMap().value(type);
    switch (it->collectionType()) {
    case InformationItem::File:
        collectingFile(path, it->getMessage(), detailed);
        break;
    case InformationItem::CMD:
        collectingCmd(path, it->getMessage());
        break;
    case InformationItem::Others:
        collectingOther(path, it->getMessage(), it->getItemName());
        break;
    default:
        break;
    }
}

/*====================非dbus接口======================*/

void Dbus::creatDbusServer()
{
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

DbusManager::DbusManager()
{
    QThread *thread = new QThread(this);
    Dbus *dbus = new Dbus();
    connect(this, &DbusManager::creatDbusServer, dbus, &Dbus::creatDbusServer);
    connect(thread, &QThread::finished, dbus, &Dbus::deleteLater);
    dbus->moveToThread(thread);
    thread->start();
    emit creatDbusServer();
}
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
        QFileInfo info(nowFile);
        if (info.isSymLink()) {
            nowFile = info.canonicalFilePath();
            if (nowFile.isEmpty()) {
                error += list.at(i) + " SymLink error ! \n";
                continue;
            }
        }
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

    if (!error.isEmpty()) {
        QFile e(saveDir + "error");
        if (e.open(QIODevice::NewOnly)) {
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
    proc.start(message);
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
    const auto size = file.write(command_output.toLocal8Bit());
    if (size != command_output.size()) {
        qWarning() << "Collect command output annex write file exception";
    }
    file.close();
}

void Tool::collectingOther(const QString &saveDir, const QString &message, const QString &name)
{
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
