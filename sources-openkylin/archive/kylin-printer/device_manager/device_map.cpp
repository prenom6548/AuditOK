/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

DeviceMapOnce::DeviceMapOnce()
{

}

DeviceMapOnce::~DeviceMapOnce()
{

}

void DeviceMapOnce::dowork()
{
    if(!GsettingsMonitor::getInstance()->gsettingsCheck()) {
        qDebug() << "GSettings init failed!";
        emit done();
        return;
    }
    DeviceMapUtils::generateMap(m_devMp);
    DeviceMapUtils::jsonWrite(m_devMp);

    emit done();
}

QJsonArray DeviceMapUtils::qStringListToQJsonArray(const QStringList &list)
{
    return QJsonArray::fromStringList(list);
}

QStringList DeviceMapUtils::qJsonArrayToQStringList(const QJsonArray &array)
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
    // 只在为空的时候更新gsetting
    QByteArray s = GsettingsMonitor::getInstance()->printerDevicesGet();
    if (!s.isEmpty()) {
        DeviceMapUtils::generateMap(m_devMp);
        DeviceMapUtils::jsonWrite(m_devMp);
    } else {
        DeviceMapUtils::jsonRead(m_devMp);
    }
    // checkPathAdd("/devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8.4");
}

DeviceMap::~DeviceMap()
{

}

bool DeviceMapUtils::generateMap(QMap<QString, DevInfo> &devmap)
{
    devmap.clear();
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumPrinters(printers);

    for (const auto& prt : printers) {
        if (devmap.contains(prt.device_uri)) {
            devmap[prt.device_uri].printerNames.append(prt.name);
        } else {
            DevInfo info;
            info.uri = prt.device_uri;
            info.printerNames.append(prt.name);
            devmap.insert(info.uri, info);
        }
    }
    auto mp = LpinfoBrowser::findDirectPrinter();
    QStringList paths = list_printers_dev_path();
    for (const auto& path : paths) {
        DevInfo info;
        info.path = path;
        info.deviceId = get_device_id_from_path(path);
        if (info.deviceId.isEmpty()) {
            continue;
        }

        for (auto dev : mp) {
            if (dev.ieee1284ID == info.deviceId) {
                info.uri = dev.uri;
                break;
            }
        }

        if (info.uri.isEmpty()) {
            continue;
        }

        if (devmap.contains(info.uri)) {
            devmap[info.uri].path = info.path;
            devmap[info.uri].deviceId = info.deviceId;
        } else {
            devmap.insert(info.uri, info);
        }
    }
    return true;
}

QString DeviceMapUtils::getJsonVerison(const QJsonObject &jsonObject)
{
    QString jsonVersion{QString()};
    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (it.key().isEmpty()) {
            continue;
        }
        if (it.key() == "version" && it.value().isString()) {
            jsonVersion = it.value().toString();
            break;
        }
    }
    return jsonVersion;
}

bool DeviceMapUtils::jsonReadFromOldGsettings(const QJsonObject &jsonObject, QMap<QString, DevInfo> &devmap)
{
    devmap.clear();
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
        if (devmap.contains(info.uri)) {
            devmap[info.uri].printerNames.append(info.printerNames);
        }
        else {
            devmap.insert(info.uri, info);
        }
    }

    if (devmap.isEmpty()) {
        return false;
    }
    return true;
}

bool DeviceMapUtils::jsonRead(QMap<QString, DevInfo> &devmap)
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

    QString jsonVersion = getJsonVerison(jsonObject);
    if (jsonVersion.isEmpty()) {
        return jsonReadFromOldGsettings(jsonObject, devmap);
    }

    devmap.clear();

    for (auto it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        if (it.key() == "uris" && it.value().isObject()) {
            updateDevMpFromJson(it.value().toObject(), devmap);
        }
    }

    if (devmap.isEmpty()) {
        return false;
    }
    return true;
}

void DeviceMapUtils::updateDevMpFromJson(const QJsonObject &jsonObject, QMap<QString, DevInfo> &devmap)
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
        devmap.insert(info.uri, info);
    }
}

QJsonObject DeviceMapUtils::writeDevInfoToJson(const DevInfo &info)
{
    QJsonObject jsonObject;
    jsonObject.insert("deviceId", info.deviceId);
    jsonObject.insert("path", info.path);
    jsonObject.insert("printers",qStringListToQJsonArray(info.printerNames));
    return jsonObject;
}

int DeviceMapUtils::jsonWrite(const QMap<QString, DevInfo> &devmap)
{
    QJsonObject jsonObject;
    jsonObject.insert("version", "0.0.1");
    QJsonObject subObj;

    for (auto it = devmap.begin(); it != devmap.end(); it++) {
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
    DeviceMapUtils::jsonRead(m_devMp);
    QStringList prts;
    QMap<QString,DevInfo>::iterator it;

    bool configNeedUpdate = false;
    // 插入usb的时候需要使用device_id检测
    // 拔出usb的时候使用path检测
    if (handle == "add") {
        QString deviceId = get_device_id_from_path_retry(path);
        QString uri = find_matching_device_uri(deviceId);
        if (uri.isEmpty()) {
            return false;
        }

        for (it = this->m_devMp.begin(); it != this->m_devMp.end(); it++) {
            if (it.value().uri == uri) {
                prts.append(it.value().printerNames);
                if (it.value().path != path) {
                    configNeedUpdate = true;
                    it.value().path = path;
                    it.value().deviceId = deviceId;
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
        DeviceMapUtils::jsonWrite(m_devMp);
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
    DeviceMapUtils::jsonRead(m_devMp);
    for (auto it = m_devMp.begin(); it != m_devMp.end();) {
        it.value().printerNames.removeAll(prtName);
        if (it.value().path.isEmpty() && it.value().printerNames.isEmpty()) {
            it = m_devMp.erase(it);
        } else {
            it++;
        }
    }

    DeviceMapUtils::jsonWrite(m_devMp);
    return ;
}

void DeviceMap::doPrinterAdd(const QString &prtName)
{
    m_devMp.clear();
    DeviceMapUtils::generateMap(m_devMp);
    DeviceMapUtils::jsonWrite(m_devMp);
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
