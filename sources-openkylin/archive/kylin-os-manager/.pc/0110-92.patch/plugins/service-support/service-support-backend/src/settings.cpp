#include "settings.h"
#include <unistd.h>
#include <QFile>
#include <QMap>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <QSettings>
#include <QMessageAuthenticationCode>
#include "kom-configure.h"
#include "informationitem.h"

#define KEY_CLASS "class"
#define KEY_ITEM "item"
#define KEY_NAME "name"
#define KEY_INFO_NAME "itemName"
#define KEY_INFO_FILE "itemFile"
#define KEY_INFO_CMD "itemCmd"
#define KEY_INFO_OTHER "itemOther"
#define KEY_INFO_TYPES "itemTypes"

#define CONFIG_PLUGIN_SYSTEM_PATH "/etc/kylin-os-manager/kylin-os-manager-plugin.ini"
#define CONFIG_PLUGIN_USER_PATH ".kylin-os-manager/kylin-os-manager-plugin.ini"

#define SYSTEM_INFOMATION_FILE_PATH "/etc/kylin-os-manager/systemInformation.txt"

#ifndef DEBUG_MODE
#include "../../conf/log-collection.h"
#endif

QString Settings::m_filePath = Settings::csvFilePath();
QByteArray Settings::m_jsonByte;
QMap<QString, QString> Settings::m_logItemClassMap;

QString Settings::m_machineId = Settings::getMachineId();

InformationClassItemList Settings::getSettings()
{
    creatMap();
#ifdef DEBUG_MODE
    creatJson();
#else
    m_jsonByte = QByteArray::fromHex(SettingsCMD::g_SettingsCMD);
#endif
    InformationClassItemList list;
    if (m_jsonByte.isEmpty()) {
        qCritical() << "Settings CMD isEmpty !";
        return list;
    }
    QJsonDocument doc = QJsonDocument::fromJson(m_jsonByte);
    if (!doc.isObject()) {
        qDebug() << "json error !";
        return list;
    }
    QJsonObject obj = doc.object();
    QJsonArray carr = obj.value(KEY_CLASS).toArray();
    for (int i = 0; i < carr.size(); i++) {
        QJsonObject cjo = carr[i].toObject();
        InformationClassItem *citem = new InformationClassItem(cjo.value(KEY_NAME).toString());
        citem->setItemNameShow(m_logItemClassMap.value(citem->getItemName()));
        QJsonArray arr = cjo.value(KEY_ITEM).toArray();
        for (int j = 0; j < arr.size(); j++) {
            QJsonObject jo = arr[j].toObject();
            QString tmpName = jo.value(KEY_INFO_NAME).toString();
            InformationItem::CollectionType t = InformationItem::File;
            QString tmpMessage = jo.value(KEY_INFO_FILE).toString();
            QString tmpTypes = jo.value(KEY_INFO_TYPES).toString();
            ModuleTypeSet set = ModuleTypeSet::fromList(tmpTypes.split("|"));
            if (tmpMessage.isEmpty()) {
                t = InformationItem::CMD;
                tmpMessage = jo.value(KEY_INFO_CMD).toString();
            }
            if (tmpMessage.isEmpty()) {
                t = InformationItem::Others;
                tmpMessage = jo.value(KEY_INFO_OTHER).toString();
            }
            InformationItem *item = new InformationItem(tmpName, t, tmpMessage, set);
            citem->children()->append(item);
        }
        list.append(citem);
    }
    return list;
}

void Settings::creatJson()
{
    QString csvFile = m_filePath;
    csvFile.chop(1);
    csvFile += "csv";
    if (!QFile::exists(csvFile)) {
        qDebug() << "can not find csv file :" << csvFile;
        return;
    }
    QFile file(csvFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read csv err";
        return;
    }
    bool skipHeader = true;
    QJsonObject root;
    QJsonArray rootArray;
    QJsonObject classItem;
    QJsonArray classArray;
    QString lastClassItemName;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (skipHeader) {
            skipHeader = false;
            continue;
        }
        line.remove('\n');
        QStringList tmpList = line.split(',');
        if (tmpList.length() < 6) {
            continue;
        }
        QString tmpClass = tmpList.first();
        QString tmpName = tmpList.at(1);
        QString tmpFile = tmpList.at(2);
        if (!tmpFile.isEmpty()) {
            //            qDebug() << tmpFile << tmpFile.at(tmpFile.length() - 1);
        }
        QString tmpCmd = tmpList.at(3);
        QString tmpTypes = tmpList.at(4);
        QString tmpOther = tmpList.at(5);
        QJsonObject info;
        info.insert(KEY_INFO_NAME, tmpName);
        info.insert(KEY_INFO_FILE, tmpFile);
        info.insert(KEY_INFO_CMD, tmpCmd);
        info.insert(KEY_INFO_OTHER, tmpOther);
        info.insert(KEY_INFO_TYPES, tmpTypes);
        if (lastClassItemName != tmpClass) {
            lastClassItemName = tmpClass;
            if (!classArray.isEmpty()) {
                classItem.insert(KEY_ITEM, classArray);
                rootArray.append(classItem);
                classItem = QJsonObject();
                classArray = QJsonArray();
            }
            classItem.insert(KEY_NAME, tmpClass);
        }
        classArray.append(info);
    }
    file.close();
    if (!classArray.isEmpty()) {
        classItem.insert(KEY_ITEM, classArray);
        rootArray.append(classItem);
    }
    root.insert(KEY_CLASS, rootArray);
    QJsonDocument jd(root);
    m_jsonByte = jd.toJson(QJsonDocument::Compact);
    QFile jsonFile(m_filePath);
    bool replaceFile = false;
    if (jsonFile.exists(m_filePath)) {
        qDebug() << "json file already exist!";
        if (!replaceFile) {
            return;
        }
        jsonFile.remove();
    }
    saveFile();
}

void Settings::saveFile()
{
    QFile saveFile(m_filePath);
    if (!saveFile.open(QIODevice::NewOnly)) {
        qDebug() << "can not creat json file !";
    }
    QByteArray fileH;
    fileH.append("#ifndef SETTINGS_CMD_H \n");
    fileH.append("#define SETTINGS_CMD_H \n");
    fileH.append("#include <QByteArray> \n");
    fileH.append("namespace SettingsCMD { \n");
    fileH.append("const QByteArray g_SettingsCMD=\"");
    fileH.append(m_jsonByte.toHex());
    fileH.append("\";\n");
    fileH.append("} \n");
    fileH.append("#endif");
    saveFile.write(fileH);
    saveFile.close();
}

void Settings::creatMap()
{
    m_logItemClassMap.insert("系统日志", QObject::tr("System log"));
    m_logItemClassMap.insert("整机信息", QObject::tr("Machine"));
    m_logItemClassMap.insert("硬件参数", QObject::tr("Hardware"));
    m_logItemClassMap.insert("驱动信息", QObject::tr("Drive"));
    m_logItemClassMap.insert("软件列表", QObject::tr("APP list"));
    m_logItemClassMap.insert("终端策略", QObject::tr("Rules"));
    m_logItemClassMap.insert("网络状态", QObject::tr("Network"));
    m_logItemClassMap.insert("系统状态", QObject::tr("System"));
}

QString Settings::csvFilePath()
{
    char path[4096];
    getcwd(path, 4096);
    QString res = path;
    res += "/../../../kylin-os-manager/plugins/service-support/conf/log-collection.h";
    return res;
}

std::tuple<QString, QString, QString> Settings::getUrlInformation(void)
{
    QString protocol;
    QString domain;
    QString port;

    QString path = QString("%1/%2").arg(getenv("HOME")).arg(CONFIG_PLUGIN_USER_PATH);
    QFile userConfigFile(path);
    if (userConfigFile.exists()) {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        protocol = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PROTOCOL).toString();
        domain = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_DOMAIN).toString();
        port = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PORT).toString();
        settings.endGroup();
    }

    if (!protocol.isEmpty() && !domain.isEmpty() && !port.isEmpty())
        return std::make_tuple(protocol, domain, port);

    QFile systemConfigfile(CONFIG_PLUGIN_SYSTEM_PATH);
    if (systemConfigfile.exists()) {
        QSettings settings(CONFIG_PLUGIN_SYSTEM_PATH, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        if (protocol.isEmpty()) {
            protocol = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PROTOCOL).toString();
        }
        if (domain.isEmpty()) {
            domain = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_DOMAIN).toString();
        }
        if (port.isEmpty()) {
            port = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PORT).toString();
        }
        settings.endGroup();
    }

    return std::make_tuple(protocol, domain, port);
}

bool Settings::isUpload(void)
{
    kom::Configure configure;
    return configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_ENABLE_UPLOAD, false)
        .toBool();
}

QString Settings::getExportPath(void)
{
    kom::Configure configure;
    return configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, "").toString();
}

void Settings::setExportPath(const QString &path)
{
    kom::Configure configure;
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, path);
}

void Settings::setHistoryBug(QString bugId)
{
    kom::Configure configure;
    QString bugIdList = getHistoryBug();
    if (!bugIdList.isEmpty()) {
        bugIdList += ',';
    }
    bugIdList += bugId;
    QString key = bugIdList;
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG, key);
}

QString Settings::getHistoryBug()
{
    kom::Configure configure;
    QString key =
        configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG).toString();
    return key;
}

QString Settings::getMachineId()
{
    QFile file("/etc/machine-id");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "打开 /etc/machine-id 失败 ！";
        return "";
    }
    QString machine = file.readAll();
    file.close();
    if (machine.isEmpty()) {
        qDebug() << "读取 /etc/machine-id 失败 ！";
        return "";
    }
    return machine;
}

QString Settings::getSystemDeviceInfo()
{
    QFile file(SYSTEM_INFOMATION_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Settings::getSystemDeviceInfo 打开文件失败";
        return "";
    }
    QString res = file.readAll();
    return res;
}
