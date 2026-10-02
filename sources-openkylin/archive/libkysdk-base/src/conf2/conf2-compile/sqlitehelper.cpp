/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include "sqlitehelper.h"
#include <QFile>
#include <QJsonDocument>
#include "syslog.h"

#include "utils.cpp"

SqliteHelper::SqliteHelper()
{
}

bool SqliteHelper::map2Database(const QMap<QString, QVariant> &map, const QString &databaseFile)
{
    QSqlDatabase db;
    if (!QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(databaseFile);
    }
    else
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }

    if (!db.open())
    {
        syslog(LOG_ERR, "%s: Could not open database.", __func__);
        return false;
    }

    QSqlQuery query(db);

    // 开始事务
    if (!query.exec("BEGIN TRANSACTION"))
    {
        syslog(LOG_ERR, "%s: %s.", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 清空所有表
    query.exec("DELETE FROM app");
    query.exec("update sqlite_sequence SET seq = 0 where name ='app'");

    query.exec("DELETE FROM version");
    query.exec("update sqlite_sequence SET seq = 0 where name ='version'");

    query.exec("DELETE FROM configures");
    query.exec("update sqlite_sequence SET seq = 0 where name ='configures'");

    int appId = 0;
    int versionId = 0;
    QStringList apps = map.keys();
    for (int i = 0; i < apps.size(); ++i)
    {
        QString app = apps[i];
        appId++;

        const QVariant &appValue = map[app];
        QMap<QString, QVariant> versionsMap = appValue.toMap();
        QStringList versions = versionsMap.keys();

        QString defaultVersion = calculateDefaultVersion(versionsMap);

        query.prepare("INSERT INTO app (app_name, default_version) VALUES (?, ?)");
        query.addBindValue(app);
        query.addBindValue(defaultVersion);
        if (!query.exec())
        {
            syslog(LOG_ERR, "%s: Could not insert into app table. %s", __func__, query.lastError().text().toLatin1().data());
            return false;
        }

        for (int j = 0; j < versions.size(); ++j)
        {
            QString version = versions[j];
            if (version == "_default_version")
                continue;

            versionId++;
            query.prepare("INSERT INTO version (app_id, version, compatible) VALUES (?, ?, ?)");
            query.addBindValue(appId);
            query.addBindValue(version);
            query.addBindValue(versionsMap[version].toMap().value("_compatible"));
            if (!query.exec())
            {
                syslog(LOG_ERR, "%s: Could not insert into version table. %s", __func__, query.lastError().text().toLatin1().data());
                return false;
            }

            QVariantMap configures = versionsMap[version].toMap();
            writeGroupInDatabase(configures, configures, query, versionId, 0);
        }
    }

    // 提交事务
    if (!query.exec("COMMIT"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    db.close();
    return true;
}

bool SqliteHelper::createDbFile(const QString &databaseFile)
{
    QSqlDatabase db;
    if (!QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(databaseFile);
    }
    else
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }

    if (!db.open())
    {
        syslog(LOG_ERR, "%s: Could not open databa7se.", __func__);
        return false;
    }

    QSqlQuery query;

    // 开始事务
    if (!query.exec("BEGIN TRANSACTION"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 创建 app 表
    if (!query.exec("CREATE TABLE IF NOT EXISTS app ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "app_name TEXT, "
                    "default_version TEXT)"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 创建 version 表
    if (!query.exec("CREATE TABLE IF NOT EXISTS version ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "app_id INTEGER, "
                    "version TEXT, "
                    "compatible TEXT, "
                    "FOREIGN KEY (app_id) REFERENCES app(id))"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 创建 configures 表
    if (!query.exec("CREATE TABLE IF NOT EXISTS configures ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "version_id INTEGER, "
                    "node_name TEXT, "
                    "node_type TEXT, "
                    "permission TEXT, "
                    "description TEXT, "
                    "summary TEXT, "
                    "parent INTEGER, "
                    "value_type TEXT, "
                    "custom_value TEXT, "
                    "default_value TEXT, "
                    "range TEXT, "
                    "extends TEXT, "
                    "FOREIGN KEY (version_id) REFERENCES version(id), "
                    "FOREIGN KEY (parent) REFERENCES configures(id))"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 提交事务
    if (!query.exec("COMMIT"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    db.close();
    return true;
}

bool SqliteHelper::updateTableStructure(const QString &databaseFile)
{
    if (!QFile::exists(databaseFile))
    {
        createDbFile(databaseFile);
        return true;
    }

    if (databaseFile == "/etc/kylin-config/user.db")
    {
        QFile::remove(databaseFile);
        createDbFile(databaseFile);
        return true;
    }

    QSqlDatabase db;
    if (!QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(databaseFile);
    }
    else
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }

    if (!db.open())
    {
        syslog(LOG_ERR, "%s: Could not open database.", __func__);
        return false;
    }

    QSqlQuery query;

    // 开始事务
    if (!query.exec("BEGIN TRANSACTION"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    // 检查是否是旧的表结构
    bool isOld = false;
    bool hasExtends = false;

    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='configures';");
    if (query.next())
    {
        query.exec("PRAGMA table_info(configures);");
        while (query.next())
        {
            QString columnName = query.value(1).toString();
            if (columnName == "group_name")
            {
                isOld = true;
            }
            if (columnName == "extends")
            {
                hasExtends = true;
            }
        }
    }
    else
    {
        db.close();
        QFile::remove(databaseFile);
        createDbFile(databaseFile);
        syslog(LOG_DEBUG, "%s: Data base table is invalid. Remove file %s", __func__, databaseFile.toLatin1().data());
        return false;
    }

    if (isOld)
    {
        syslog(LOG_DEBUG, "%s: Data base table is old structure. Update structure", __func__);
        // 创建新表
        if (!query.exec("CREATE TABLE IF NOT EXISTS new_configures ("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "version_id INTEGER, "
                        "node_name TEXT, "
                        "node_type TEXT, "
                        "permission TEXT, "
                        "description TEXT, "
                        "summary TEXT, "
                        "parent INTEGER, "
                        "value_type TEXT, "
                        "custom_value TEXT, "
                        "default_value TEXT, "
                        "range TEXT, "
                        "extends TEXT, "
                        "FOREIGN KEY (version_id) REFERENCES version(id), "
                        "FOREIGN KEY (parent) REFERENCES configures(id))"))
        {
            syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
            return false;
        }

        // 迁移数据
        if (hasExtends)
        {
            if (!query.exec("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                            "SELECT "
                            "version_id,"
                            "CASE "
                            "WHEN property IS NOT NULL THEN property "
                            "WHEN group_name IS NOT NULL THEN group_name "
                            "END, "
                            "CASE "
                            "WHEN property IS NOT NULL THEN 'key' "
                            "WHEN group_name IS NOT NULL THEN 'schema' "
                            "END,"
                            "permission,"
                            "description,"
                            "summary,"
                            "parent,"
                            "data_type,"
                            "user_value,"
                            "default_value,"
                            "range,"
                            "CASE "
                            "WHEN 'extends' IN (SELECT name FROM pragma_table_info('configures')) THEN extends "
                            "ELSE NULL "
                            "END "
                            "FROM configures;"))
            {
                syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
                return false;
            }
        }
        else
        {
            if (!query.exec("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                            "SELECT "
                            "version_id,"
                            "CASE "
                            "WHEN property IS NOT NULL THEN property "
                            "WHEN group_name IS NOT NULL THEN group_name "
                            "END, "
                            "CASE "
                            "WHEN property IS NOT NULL THEN 'key' "
                            "WHEN group_name IS NOT NULL THEN 'schema' "
                            "END,"
                            "permission,"
                            "description,"
                            "summary,"
                            "parent,"
                            "data_type,"
                            "user_value,"
                            "default_value,"
                            "range,"
                            "NULL "
                            "FROM configures;"))
            {
                syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
                return false;
            }
        }

        // 删除原表
        if (!query.exec("DROP TABLE configures;"))
        {
            syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
            return false;
        }

        // 重命名新表
        if (!query.exec("ALTER TABLE new_configures RENAME TO configures;"))
        {
            syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
            return false;
        }

        // 检查并修复 app 表的 default_version 列名
        query.exec("PRAGMA table_info(app);");
        bool hasDefaultVersionType = false;
        while (query.next())
        {
            QString columnName = query.value(1).toString();
            if (columnName == "defualt_version")
            {
                hasDefaultVersionType = true;
                break;
            }
        }

        if (hasDefaultVersionType)
        {
            if (!query.exec("ALTER TABLE app RENAME COLUMN defualt_version TO default_version;"))
            {
                syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
                return false;
            }
        }
    }

    // 提交事务
    if (!query.exec("COMMIT"))
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
        return false;
    }

    db.close();
    return true;
}

void SqliteHelper::writeGroupInDatabase(const QMap<QString, QVariant> &versionData, const QMap<QString, QVariant> &data, QSqlQuery &query, int versionId, int parentId)
{
    QStringList groups;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it)
    {
        QString key = it.key();
        QVariant value = it.value();

        if (value.type() == QVariant::Map)
        {
            QMap<QString, QVariant> valueMap = value.toMap();
            if (!valueMap.contains("_default"))
            {
                groups.append(key);
            }
            else
            {
                writeKeyInDatabase(versionData, key, valueMap, query, versionId, parentId, data.value("_permission").toString());
            }
        }
    }

    for (const QString &group : groups)
    {
        QString nodeName = group;
        QString nodeType = "schema";
        QString permission = data[group].toMap().value("_permission", "public").toString();
        QString description = data[group].toMap().value("_description").toString();
        QString summary = data[group].toMap().value("_summary").toString();
        QString extends = data[group].toMap().value("_extends").toString();

        query.prepare("INSERT INTO configures (version_id, node_name, node_type, permission, description, summary, parent, extends) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(versionId);
        query.addBindValue(nodeName);
        query.addBindValue(nodeType);
        query.addBindValue(permission);
        query.addBindValue(description);
        query.addBindValue(summary);
        query.addBindValue(parentId);
        query.addBindValue(extends);
        if (!query.exec())
        {
            syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
            return;
        }

        int groupId = query.lastInsertId().toInt();
        writeGroupInDatabase(versionData, data[group].toMap(), query, versionId, groupId);
    }
}

void SqliteHelper::writeKeyInDatabase(const QMap<QString, QVariant> &versionData, const QString &key, const QMap<QString, QVariant> &value, QSqlQuery &query, int versionId, int parentId, const QString &parentPermission)
{
    QString nodeName = key;
    QString nodeType = "key";
    QString permission = value.value("_permission", parentPermission).toString();
    QString description = value.value("_description").toString();
    QString summary = value.value("_summary").toString();
    QString valueType = value.value("_type").toString();
    QString customValue = value.value("_value").toString();
    QString defaultValue = value.value("_default").toString();
    QString range = value.value("_range").toString();

    if (valueType == "enum" && range.startsWith("@"))
    {
        QVariant enumRange = recursiveSearch(versionData, range.mid(1));
        QMap<QString, QVariant> enumMap;
        for (const QVariant &element : enumRange.toList())
        {
            QMap<QString, QVariant> elementMap = element.toMap();
            QString nick = elementMap.value("_nick").toString();
            QString value = elementMap.value("_value").toString();
            if (value.startsWith("0x"))
            {
                bool ok = false;
                enumMap[nick] = value.mid(2).toInt(&ok, 16);
            }
            else
                enumMap[nick] = value.toInt();
        }
        QVariant rangeValue(enumMap);
        range = QString::fromUtf8(QJsonDocument::fromVariant(rangeValue).toJson());
    }
    else if (valueType == "bool")
    {
        valueType = "b";
    }
    else if (valueType == "int")
    {
        valueType = "i";
    }
    else if (valueType == "int64")
    {
        valueType = "x";
    }
    else if (valueType == "uint")
    {
        valueType = "u";
    }
    else if (valueType == "uint64")
    {
        valueType = "t";
    }
    else if (valueType == "double")
    {
        valueType = "d";
    }
    else if (valueType == "string")
    {
        valueType = "s";
    }

    query.prepare("INSERT INTO configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(versionId);
    query.addBindValue(nodeName);
    query.addBindValue(nodeType);
    query.addBindValue(permission);
    query.addBindValue(description);
    query.addBindValue(summary);
    query.addBindValue(parentId);
    query.addBindValue(valueType);
    query.addBindValue(customValue);
    query.addBindValue(defaultValue);
    query.addBindValue(range);
    if (!query.exec())
    {
        syslog(LOG_ERR, "%s: %s", __func__, query.lastError().text().toLatin1().data());
    }
}

QString SqliteHelper::calculateDefaultVersion(const QMap<QString, QVariant> &data)
{
    QStringList versions = data.keys();
    if (versions.contains("_default_version"))
    {
        return data.value("_default_version").toString();
    }

    versions.removeOne("_default_version");
    std::sort(versions.begin(), versions.end(), compareVersions);

    return versions.last();
}

QVariant SqliteHelper::recursiveSearch(const QMap<QString, QVariant> &dictionary, const QString &key)
{
    if (dictionary.contains(key))
    {
        return dictionary.value(key);
    }
    else
    {
        for (const QVariant &value : dictionary.values())
        {
            if (value.type() == QVariant::Map)
            {
                QVariant result = recursiveSearch(value.toMap(), key);
                if (!result.isNull())
                {
                    return result;
                }
            }
        }
    }
    return QVariant();
}
