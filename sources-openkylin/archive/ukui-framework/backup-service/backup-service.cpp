/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "backup-service.h"
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDBusMessage>
#include <QJsonObject>
#include <QJsonValue>
#include <QDomDocument>

#include "backup-service-private.h"
#include "configbackupserviceadaptor.h"
#include "backup-service-config.h"
using namespace UkuiConfigBackup;
BackupServicePrivate::BackupServicePrivate(QObject* parent)
    : QObject(parent)
{
    new ConfigBackupServiceAdaptor(this);
    QDBusConnection conn = QDBusConnection::sessionBus();
    if(!conn.registerObject(backupServicePath(), this)) {
        qCritical() << "Failed to register BackupService DBus object!" << conn.lastError().message();
        QCoreApplication::quit();
    }
    auto registration = conn.interface()->registerService(backupServiceName(),
                                                          QDBusConnectionInterface::ReplaceExistingService,
                                                          QDBusConnectionInterface::DontAllowReplacement);
    if (registration.value() != QDBusConnectionInterface::ServiceRegistered) {
        qCritical() << "Failed to register BackupService service on DBus!" << conn.lastError().message();
        QCoreApplication::quit();
    }
}

QString BackupServicePrivate::GetBackupConfig(const QStringList &modules, const QString &name)
{
    qDebug() << "Backup service begin backup modules:" << modules << "name:" << name;
    auto templateJson = loadConfig(QString(TEMPLATE_INSTALL_PATH) + "/ukui-backup-config-template.json");
    if (templateJson.isEmpty()) {
        return {};
    }
    QJsonObject resultJson;
    for (auto &module: modules) {
        if (templateJson.contains(module)) {
            // 得到该模块的配置模板
            auto moduleJson = templateJson[module].toObject();
            // 遍历该模块的所有dbus路径
            for (auto &path: moduleJson.keys()) {
                // 得到该路径下的所有dbus接口
                auto interfaceObject = moduleJson[path].toObject();
                // 遍历该路径下的所有dbus接口
                for (auto &interface: interfaceObject.keys()) {
                    // 得到该接口的配置列表
                    auto configMap = interfaceObject[interface].toObject().toVariantMap();
                    if (!configMap.keys().isEmpty()) {
                        // 调用dbus接口获取当前配置
                        QDBusMessage msg = QDBusMessage::createMethodCall(
                            QStringLiteral("org.ukui.Framework"), path,
                            QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("GetAll"));
                        msg.setArguments({interface});
                        QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(msg);
                        if (reply.isValid()) {
                            auto properties = reply.value();
                            qDebug() << "Backup Config Properties: " << properties;
                            // 遍历该接口的所有配置项，将当前存在的配置添加到配置列表中，不存在的配置项从配置列表中删除
                            for (auto &key: configMap.keys()) {
                                if (properties.contains(key)) {
                                    configMap.insert(key, properties[key]);
                                } else if (key != QStringLiteral("Res")) {
                                    qWarning() << "BackupService: Config item" << key << "is not exist on interface" << interface << "on path" << path;
                                    configMap.remove(key);
                                }
                            }

                            // 添加资源文件到配置列表中
                            if (configMap.contains(QStringLiteral("Res"))) {
                                auto res = configMap.value(QStringLiteral("Res")).toMap();
                                for (auto &key: res.keys()) {
                                    qDebug() << "Backup Config Res Key:" << key << " Value:" << configMap.value(key);
                                    res.insert(key, configMap.value(key));
                                }
                                configMap.insert(QStringLiteral("Res"), res);
                            }
                        } else {
                            qCritical() << "BackupService: Get config failed for interface" << interface << "on path" << path << "with error:" << reply.error();
                            configMap.clear();
                        }
                    }
                    interfaceObject.insert(interface, QJsonObject::fromVariantMap(configMap));
                }
                moduleJson.insert(path, interfaceObject);
            }
            resultJson.insert(module, moduleJson);
        }
    }

    return saveCache(resultJson, name);
}

QStringList BackupServicePrivate::SyncConfig(const QString &config)
{
    qDebug() << "Backup service begin sync config:" << config;
    auto configObject = loadConfig(config);
    if (configObject.isEmpty()) {
        return {};
    }
    QStringList successList;
    // 遍历配置对象的所有模块
    for (auto &module: configObject.keys()) {
        bool success = true;
        // 得到该模块的配置
        auto moduleJson = configObject[module].toObject();
        // 遍历该模块的所有dbus路径
        for (auto &path: moduleJson.keys()) {
            // 得到该路径下的所有dbus接口
            auto interfaceObject = moduleJson[path].toObject();
            // 遍历该路径下的所有dbus接口
            for (auto &interface: interfaceObject.keys()) {
                // 得到该接口的配置列表
                auto configMap = interfaceObject[interface].toObject().toVariantMap();
                for (auto &key: configMap.keys()) {
                    if (key != QStringLiteral("Res")) {
                        QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.Framework"), path,QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("Set"));
                        QVariant value;
                        if (configMap[key].type() == QVariant::List) {
                            auto listType = propertyTypeOfList(path, interface, key);
                            if (listType == QMetaType::QString) {
                                value = QVariant(convertVariantListToQList<QString>(configMap[key].toList()));
                            } else if (listType == QMetaType::Bool) {
                                value = QVariant::fromValue(convertVariantListToQList<bool>(configMap[key].toList()));
                            } else if (listType == QMetaType::Double) {
                                value = QVariant::fromValue(convertVariantListToQList<double>(configMap[key].toList()));
                            } else if (listType == QMetaType::Int) {
                                value = QVariant::fromValue(convertVariantListToQList<int>(configMap[key].toList()));
                            } else {
                                qWarning() << "BackupService: Config item" << key << "is not support list type" << listType << "on interface" << interface << "on path" << path;
                                success = false;
                                continue;
                            }
                            value = QVariant::fromValue(QDBusVariant(value));
                        } else {
                            value = QVariant::fromValue(QDBusVariant(configMap[key]));
                        }
                        msg.setArguments({interface, key, value});
                        QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
                        if (reply.type() != QDBusMessage::ReplyMessage) {
                            qCritical() << "BackupService: Set config failed for interface" << interface << "on path" << path << "with error:" << reply.errorMessage();
                            success = false;
                        }
                    }
                }
            }
        }
        if (success) {
            successList.append(module);
        }
    }
    return successList;
}

QString BackupServicePrivate::saveCache(const QJsonObject &config, QString pathPrefix)
{
    if (pathPrefix.isEmpty()) {
        pathPrefix = QStringLiteral("default");
    }

    QDir dir;
    QString configFileDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" + pathPrefix);
    if (!dir.exists(configFileDir)) {
        if (!dir.mkpath(configFileDir)) {
            qCritical() << "BackupService: Unable to create cache file directory" << configFileDir;
            return {};
        }
    }

    // 保存配置到缓存文件
    QFile cacheFile(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" + pathPrefix + "/ukui-backup-config-cache.json");
    if (!cacheFile.open(QFile::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCritical() << "BackupService: save cache file " << cacheFile.fileName() << "failed !" << cacheFile.errorString();
        return {};
    }
    if (cacheFile.write(QJsonDocument(config).toJson()) == -1) {
        qCritical() << "BackupService: save cache file " << cacheFile.fileName() << "failed !" << cacheFile.errorString();
        return {};
    }
    cacheFile.flush();
    cacheFile.close();

    return cacheFile.fileName();
}

QJsonObject BackupServicePrivate::loadConfig(const QString &config)
{
    QFile configFile(config);
    if (!configFile.exists()) {
        qCritical() << "BackupService: loading Config file" << config << " is not exist!";
        return {};
    }

    if (!configFile.open(QFile::ReadOnly)) {
        qCritical() << "BackupService: load config file " << configFile.fileName() << "failed !";
        return {};
    }
    // 读取json数据
    QByteArray byteArray = configFile.readAll();
    configFile.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qCritical() << "BackupService: Incorrect config files. JSON parse error"  << configFile.fileName();;
        return {};
    }

    return jsonDocument.object();
}

QMetaType::Type BackupServicePrivate::propertyTypeOfList(const QString &path, const QString &interface, const QString &property)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.Framework"), path,QStringLiteral("org.freedesktop.DBus.Introspectable"), QStringLiteral("Introspect"));

    QDBusReply<QString> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCritical() << "BackupService: Unable to call Introspect of " << path << "with error:" << reply.error();
        return QMetaType::UnknownType;
    }
    QDomDocument doc;
    if (!doc.setContent(reply.value())) {
        qCritical() << "BackupService: Unable to parse Introspect XML of " << path;
        return QMetaType::UnknownType;
    }

    QDomElement root = doc.documentElement();
    QDomNodeList interfaceList = root.elementsByTagName("interface");
    for (int i = 0; i < interfaceList.count(); ++i) {
        QDomElement interfaceElem = interfaceList.at(i).toElement();
        if (interfaceElem.attribute("name") == interface) {
            QDomNodeList propertyList = interfaceElem.elementsByTagName("property");
            for (int j = 0; j < propertyList.count(); ++j) {
                QDomElement propertyElem = propertyList.at(j).toElement();
                if (propertyElem.attribute("name") == property) {
                    QString typeSignature = propertyElem.attribute("type");
                    if (typeSignature == "as") {
                        return QMetaType::QString;
                    }
                    if (typeSignature == "ad") {
                        return QMetaType::Double;
                    }
                    if (typeSignature == "ab") {
                        return QMetaType::Bool;
                    }
                    if (typeSignature == "ai") {
                        return QMetaType::Int;
                    }
                    qWarning() << "BackupService: Unsupported signature" << typeSignature << "for property" << property << "on interface" << interface << "on path" << path;
                    break;
                }
            }
        }
    }

    return QMetaType::UnknownType;
}


BackupServicePrivate::~BackupServicePrivate()
{
}

static BackupService *g_service = nullptr;
static std::once_flag flag;

BackupService* BackupService::self()
{
    std::call_once(flag, [ & ] {
        g_service = new BackupService();
    });
    return g_service;
}

BackupService::BackupService() : d(new BackupServicePrivate())
{
}
