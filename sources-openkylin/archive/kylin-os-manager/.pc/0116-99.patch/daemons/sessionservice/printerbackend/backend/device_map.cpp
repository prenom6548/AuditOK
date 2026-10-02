#include "device_map.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ukui_printer.h"
#include "udev_devices.h"
#include "common.h"
#include "system_notification.h"
#include "event_notify_monitor.h"
#include "global_variable.h"
#include "gsettings_monitor.h"
#include "lpinfo_browser.h"

QJsonArray qStringListToQJsonArray(const QStringList &list)
{
    return QJsonArray::fromStringList(list);
}

QStringList qJsonArrayToQStringList(const QJsonArray &array)
{
    QStringList list;
    for (auto i : array) {
        if (i.isString()) {
            list.append(i.toString());
        }
    }
    return list;
}

DeviceMap::DeviceMap(/* args */)
{
    if(!GsettingsMonitor::getInstance()->gsettingsCheck()) {
        qDebug() << "GSettings init failed!";
        exit(0);
    }

    connect(EventNotifyMonitor::getInstance(), &EventNotifyMonitor::rhPrinterRemoved,
            this , &DeviceMap::doPrinterRemove);
    connect(EventNotifyMonitor::getInstance(), &EventNotifyMonitor::rhPrinterAdded,
            this , &DeviceMap::doPrinterAdd);


    m_devMp.clear();
    // 开机启动重新扫一遍设备 更新gsetting
    generateMap();
    jsonWrite();
    // checkPathAdd("/devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8.4");
}

DeviceMap::~DeviceMap()
{

}

bool DeviceMap::updateDevMpFromPrinterName(const QString &printerName)
{
    bool res = false;
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumPrinters(printers);
    for (const auto& prt : printers) {
        if (printerName.isEmpty() == false && printerName != prt.name) {
            continue;
        }


        for (auto it = m_devMp.begin(); it != m_devMp.end(); it++) {
            if (it.key() == prt.device_uri) {
                if(it.value().printerNames.contains(prt.name) == false) {
                    res = true;
                    it.value().printerNames.append(prt.name);
                }
                break;
            }
        }
        break;
    }
    return res;
}

bool DeviceMap::generateMap()
{
    QStringList paths = list_printers_dev_path();
    QStringList uris;
    for (const auto& path : paths) {
        DevInfo info;
        info.path = path;
        info.deviceId = get_device_id_from_path(path);
        if (info.deviceId.isEmpty()) {
            continue;
        }
        auto mp = LpinfoBrowser::findDirectPrinter();
        for (auto dev : mp) {
            if (dev.ieee1284ID == info.deviceId) {
                info.uri = dev.uri;
            }
        }
        if (!info.uri.isEmpty()) {
            m_devMp.insert(info.uri, info);
        }
    }

    updateDevMpFromPrinterName();
    return true;
}

void DeviceMap::getJsonVerison(const QJsonObject &jsonObject)
{
    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (it.key().isEmpty()) {
            continue;
        }
        if (it.key() == "version" && it.value().isString()) {
            m_jsonVersion = it.value().toString();
            break;
        }
    }
    return ;
}

bool DeviceMap::jsonReadFromOldGsettings(const QJsonObject &jsonObject)
{
    this->m_devMp.clear();
    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        QJsonObject subObj = it.value().toObject();
        DevInfo info;
        if (subObj.contains("path") && subObj.value("path").isString()
         && subObj.contains("uri")  && subObj.value("uri").isString()
         && subObj.contains("deviceId") && subObj.value("deviceId").isString()) {
            info.path = subObj.value("path").toString();
            info.uri  = subObj.value("uri").toString();
            info.deviceId  = subObj.value("deviceId").toString();
            info.printerNames.append(it.key());
        }
        else {
            continue;
        }
        if (m_devMp.contains(info.uri)) {
            m_devMp[info.uri].printerNames.append(info.printerNames);
        }
        else {
            this->m_devMp.insert(info.uri, info);
        }
    }

    if (this->m_devMp.isEmpty()) {
        return false;
    }
    return true;
}

bool DeviceMap::jsonRead()
{
    QByteArray s = GsettingsMonitor::getInstance()->printerDevicesGet();

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(s, &jsonParserError);

    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug () << "json解析失败";
        return false;
    }
    if (!jsonDocument.isObject()) {
        return false;
    }
    QJsonObject jsonObject = jsonDocument.object();
    qDebug() << jsonObject;
    if (jsonObject.size() == 0) {
        qDebug() << "json is empty!";
        return false;
    }

    getJsonVerison(jsonObject);
    if (m_jsonVersion.isEmpty()) {
        return jsonReadFromOldGsettings(jsonObject);
    }

    this->m_devMp.clear();

    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        if (it.key() == "uris" && it.value().isObject()) {
            updateDevMpFromJson(it.value().toObject());
        }
    }

    if (this->m_devMp.isEmpty()) {
        return false;
    }
    return true;
}

void DeviceMap::updateDevMpFromJson(const QJsonObject &jsonObject)
{
    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        QJsonObject subObj = it.value().toObject();
        DevInfo info;
        info.uri = it.key();
        if (subObj.contains("path") && subObj.value("path").isString()) {
            info.path = subObj.value("path").toString();
        }
        if (subObj.contains("deviceId") && subObj.value("deviceId").isString()) {
            info.deviceId = subObj.value("deviceId").toString();
        }
        if (subObj.contains("printers") && subObj.value("printers").isArray()) {
            info.printerNames = qJsonArrayToQStringList(subObj.value("printers").toArray());
        }
        this->m_devMp.insert(info.uri, info);
    }
}

QJsonObject DeviceMap::writeDevInfoToJson(const DevInfo &info)
{
    QJsonObject jsonObject;
    jsonObject.insert("deviceId", info.deviceId);
    jsonObject.insert("path", info.path);
    jsonObject.insert("printers",qStringListToQJsonArray(info.printerNames));
    return jsonObject;
}

int DeviceMap::jsonWrite()
{
    QJsonObject jsonObject;
    jsonObject.insert("version", "0.0.1");
    QJsonObject subObj;
    QMap<QString,DevInfo>::iterator it;
    for (it = this->m_devMp.begin(); it != this->m_devMp.end(); it++) {
        subObj.insert(it.key(), writeDevInfoToJson(it.value()));
    }
    jsonObject.insert("uris", subObj);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);

    GsettingsMonitor::getInstance()->printerDevicesSet(jsonDoc.toJson());
    return jsonObject.size();
}

bool DeviceMap::checkPathAdd(const QString &path)
{
    return checkPath("add", path);
}

bool DeviceMap::checkPathRemove(const QString &path)
{
    return checkPath("remove", path);
}

bool DeviceMap::checkPath(const QString &handle, const QString &path)
{
    if (handle.isEmpty() || path.isEmpty())
        return false;
    jsonRead();
    QStringList prts;
    QMap<QString,DevInfo>::iterator it;

    bool configNeedUpdate = false;
    // 插入usb的时候需要使用device_id检测
    // 拔出usb的时候使用path检测
    if (handle == "add") {
        QString deviceId = get_device_id_from_path(path);
        if (deviceId.isEmpty()) {
            return false;
        }

        for (it = this->m_devMp.begin(); it != this->m_devMp.end(); it++) {
            if (it.value().deviceId == deviceId) {
                prts.append(it.value().printerNames);
                if (it.value().path != path) {
                    configNeedUpdate = true;
                    it.value().path = path;
                }
            }
        }
    } else if (handle == "remove") {
        for (it = this->m_devMp.begin(); it != this->m_devMp.end(); it++) {
            if (it.value().path == path) {
                prts.append(it.value().printerNames);
                it.value().path.clear();
                configNeedUpdate = true;
            }
        }
    }

    if (configNeedUpdate) {
        jsonWrite();
    }

    QString msg;
    int prtsCnt = prts.size();
    if (prtsCnt == 0)
        return false;
    if (prtsCnt == 1)
        msg = tr("Printer");
    else if (prtsCnt > 1)
        msg = tr("Printers");
    if (handle == "add")
        msg += QString(" ") + tr("plug-in:") + QString(" ") + prts.join(',');
    else if (handle == "remove")
        msg += QString(" ") + tr("unplugged:") + QString(" ") + prts.join(',');

    SystemNotification().sendMessage(msg);
    return true;
}

void DeviceMap::doPrinterRemove(const QString &prtName)
{
    jsonRead();
    for (auto it = m_devMp.begin(); it != m_devMp.end();) {
        it.value().printerNames.removeAll(prtName);
        if (it.value().path.isEmpty() && it.value().printerNames.isEmpty()) {
            it = m_devMp.erase(it);
        } else {
            it++;
        }
    }

    jsonWrite();
    return ;
}

void DeviceMap::doPrinterAdd(const QString &prtName)
{
    m_devMp.clear();
    generateMap();
    jsonWrite();
    return ;
}

DeviceMap *DeviceMap::getInstance()
{
    static DeviceMap *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new DeviceMap();
        thread = new QThread();
        instance->moveToThread(thread);
        thread->start();
    }
    return instance;
}
