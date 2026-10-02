/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QString>
#include <QChar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QProcess>
#include <QCoreApplication>

#include <QDebug>

static const char letters[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
QString createRandomString(int length) {
    QString result;

    for (int i = 0; i < length; ++i) {
        int randomIndex = QRandomGenerator::global()->bounded(61);
        result.append(letters[randomIndex]);
    }

    return result;
}

class DatabaseManagerPrivate
{
public:
    DatabaseManagerPrivate(DatabaseManager *manager, const QString &dbname,const QString &filedbname) {
        QString connectionName = QString("sqlite_con_thread_%1").arg(QString::number(reinterpret_cast<ulong>(QThread::currentThreadId())));
        QString filedbconnectionName = QString("filesqlite_con_thread_%1").arg(QString::number(reinterpret_cast<ulong>(QThread::currentThreadId())));
        if (QSqlDatabase::contains(connectionName) && QSqlDatabase::contains(filedbconnectionName)) {
            db = QSqlDatabase::database(connectionName);
            db.open();
            filedb = QSqlDatabase::database(filedbconnectionName);
            filedb.open();
            return;
        }
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        filedb = QSqlDatabase::addDatabase("QSQLITE", filedbconnectionName);

        QObject::connect(QThread::currentThread(), &QThread::destroyed, qApp, [=]{
            QSqlDatabase::removeDatabase(connectionName);
        });
        QString dbDirectory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QDir d;
        d.mkpath(dbDirectory);
        db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + dbname);
        filedb.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + filedbname);
        if (!db.open()) {
            qCritical() << "can not open database" << db.lastError().text();
        } else {
            // 如果没有创建空间，创建默认空间
            auto tables = db.tables();
            if (tables.isEmpty()) {
                // 创建空间表格，包含Name、ID、空间条件
                QString createSpacesTableCommand =
                        "CREATE TABLE \"Spaces\" ("
                            "\"Name\" TEXT NOT NULL UNIQUE,"
                            "\"id\"	INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "\"Conditions\" BLOB,"
                            "\"Active\" BOOL NOT NULL,"
                            "\"AutoUpdate\" BOOL NOT NULL,"
                            "\"lastAddedFileCount\"	DEFAULT 0,"
                            "\"lastAddedFileTimeMsec\"	DEFAULT 0"
                        ")";
                db.exec(createSpacesTableCommand);
                createInternalSpaces();
            } else {
                // 将spaces和id的表格进行缓存
                NeedAddintroFile = false;
                QSqlQuery query(db);
                query.prepare("SELECT (id, Name) FROM Spaces WHERE Active=true");
                bool ok = query.exec();
                if (!ok) {
                    qWarning() << query.lastError().text();
                } else {
                    while (query.next()) {
                        int id = query.value(0).toInt();
                        QString name = query.value(1).toString();
                        spaces.insert(id, name);
                    }
                }

                bool flag = false;
                QString queryStr = QString("PRAGMA table_info(Spaces);");
                 if(query.exec(queryStr))
                 {
                    while(query.next())
                    {
                         QString colname = query.value(1).toString();
                         if(colname == "lastAddedFileCount" || colname == "lastAddedFileTimeMsec")
                         {
                             flag =true;
                             break;
                         }
                    }
                 }
                 if(!flag)
                 {
                     QString addfilecount = QString("ALTER TABLE Spaces ADD COLUMN lastAddedFileCount INTEGER DEFAULT 0");
                     if (!query.exec(addfilecount)) {
                         qCritical() << "failed to add col" <<query.lastError().text();
                     }
                     else
                     {
                         qDebug()<<"add col sucess";
                     }
                     QString addfiletimeMsec = QString("ALTER TABLE Spaces ADD COLUMN lastAddedFileTimeMsec INTEGER DEFAULT 0");
                     if (!query.exec(addfiletimeMsec)) {
                         qCritical() << "failed to add col" <<query.lastError().text();
                     }
                     else
                     {
                         qDebug()<<"add col sucess";
                     }
                 }
            }
        }
        if (!filedb.open()) {
            qCritical() << "can not open fileinfo database" << filedb.lastError().text();
        }else{
            auto tables = filedb.tables();
            if (tables.isEmpty()) {
                // 创建自动添加文件表格，包含文件路径、ID、所在空间、对应版本控制路径、过时时间
                QString createAutoAddedFilesTableCommand =
                        "CREATE TABLE \"AutoAddedFiles\" ("
                            "\"Path\" TEXT NOT NULL UNIQUE,"
                            "\"id\"	INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "\"SpaceIds\" TEXT,"
                            "\"VCPath\"	TEXT,"
                            "\"OutdateTime\" BIGINT"
                        ")";
                filedb.exec(createAutoAddedFilesTableCommand);

                // 创建手动添加文件表格，包含文件路径、ID、所在空间、对应版本控制路径、过时时间
                QString createUserAddedFilesTableCommand =
                        "CREATE TABLE \"UserAddedFiles\" ("
                            "\"Path\" TEXT NOT NULL UNIQUE,"
                            "\"id\"	INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "\"SpaceIds\" TEXT,"
                            "\"VCPath\"	TEXT,"
                            "\"OutdateTime\" BIGINT"
                        ")";
                filedb.exec(createUserAddedFilesTableCommand);

                // 将介绍文件加入默认空间
                //manager->addFileToSpace("/usr/share/1", 1, true);
                //manager->addFileToSpace("/usr/share/2", 1, true);
                NeedAddintroFile = true;
            }
        }
    }
    void createIntroSpace()
    {
        QString workSpaceName = "Intelligent Data Manager Introduction";

        // 创建默认空间
        QString createWorkCommand = QString("INSERT INTO Spaces (ID, Name, Active,AutoUpdate,lastAddedFileCount, lastAddedFileTimeMsec) VALUES (1, '%1', true,true, 0 , 0)").arg(workSpaceName);
        db.exec(createWorkCommand);
        spaces.insert(0, workSpaceName);

        // 默认空间条件初始化
        QJsonObject obj;
        obj["conditions count"] = 0;
        obj["labels"] = QJsonArray::fromStringList(QStringList());
        QJsonDocument doc(obj);
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir d;
        d.mkpath(configPath + "/spaces");
        QString jsonPath = QString("%1/spaces/1.json").arg(configPath);
        QFile file(jsonPath);
        if (file.open(QFile::WriteOnly|QFile::Text)) {
            file.write(doc.toJson());
            file.close();
        }
    }
    void createInternalSpaces()
    {
        ensureXdgUserDirs();
        createIntroSpace();
        //createDocSpace();
        //createDownloadSpace();
    }

    void ensureXdgUserDirs()
    {
        QString xdgUserDirsConfigFilePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/user-dirs.dirs";
        QFile xdgUserDirsConfigFile(xdgUserDirsConfigFilePath);
        if (!xdgUserDirsConfigFile.exists()) {
            QProcess p;
            p.setProgram("/usr/bin/xdg-user-dirs-update");
            p.start();
            p.waitForFinished();
        }
    }
    QSqlDatabase db;
    QSqlDatabase filedb;
    QMap<int, QString> spaces;
    bool NeedAddintroFile = false;

};

DatabaseManager::DatabaseManager(const QString &dbname, const QString &filedbname,QObject *parent) : QObject(parent)
{
    priv = new DatabaseManagerPrivate(this, dbname,filedbname);
    if(priv->NeedAddintroFile){
        bool ok = addFileToSpace("/usr/share/peony-intelligent-data-management-service/智能空间使用手册.pdf",1,true);
        priv->NeedAddintroFile = false;
    }
}

DatabaseManager::~DatabaseManager()
{
    priv->db.close();
    priv->filedb.close();
    delete priv;
}

QStringList DatabaseManager::getAllSpaceNames(QString *errorMessage)
{
    if (!priv->spaces.isEmpty()) {
        return priv->spaces.values();
    }

    QStringList l;
    QSqlQuery query(priv->db);
    query.prepare("SELECT Name FROM Spaces WHERE Active=true ORDER BY id ASC");
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return l;
    } else {
        while (query.next()) {
            l << query.value(0).toString();
        }
    }

    return l;
}

bool DatabaseManager::createSpace(const QString &spaceName, const QStringList &conditions, QString *errorMessage, QString *resultName)
{
    QSqlQuery query(priv->db);
    QString realSpaceName = spaceName;
    if (spaceName.isEmpty()) {
        auto currentSpaceNames = getAllSpaceNames();
        int count = 1;
        while (true) {
            realSpaceName = tr("New Space %1").arg(count);
            if (currentSpaceNames.contains(realSpaceName)) {
                count++;
            } else {
                break;
            }
        }
    }
    query.prepare("INSERT INTO Spaces (Name, Conditions, Active,AutoUpdate,lastAddedFileCount, lastAddedFileTimeMsec) VALUES (:n, :c,true,true,0 ,0)");
    query.bindValue(":n", realSpaceName);
    query.bindValue(":c", conditions.join('\n'));
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        int newSpaceId = getSpaceIdByName(spaceName);
        priv->spaces.insert(newSpaceId, spaceName);
        if (resultName) {
            *resultName = realSpaceName;
        }
        return true;
    }
}

bool DatabaseManager::renameSpace(const QString &currentSpaceName, const QString &newSpaceName, QString *errorMessage)
{
    QSqlQuery query(priv->db);
    query.prepare("UPDATE Spaces SET Name=:n WHERE Name=:c");
    query.bindValue(":c", currentSpaceName);
    query.bindValue(":n", newSpaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        return true;
    }
}

bool DatabaseManager::updateSpaceStatusDescription(int id,quint64 count,quint64 time)
{
    QSqlQuery query(priv->db);
    query.prepare("UPDATE Spaces SET lastAddedFileCount=:count ,lastAddedFileTimeMsec=:time WHERE id=:n");
    query.bindValue(":count", count);
    query.bindValue(":time", time);
    query.bindValue(":n", id);
    bool ok = query.exec();
    return ok;
}
QVector<quint64> DatabaseManager::getSpaceStatusDescription(int id)
{
    QSqlQuery query(priv->db);
    QVector<quint64>vec;
    query.prepare("SELECT lastAddedFileCount , lastAddedFileTimeMsec FROM Spaces WHERE id=:n");
    query.bindValue(":n", id);
    bool ok = query.exec();
    quint64 count = 0;
    quint64 msectime = 0;
    //QString queryStr = QString("SELECT lastAddedFileCount , lastAddedFileTimeMsec FROM Spaces WHERE id=64;");
    if(ok)
    {
        while(query.next())
        {
            count  = query.value(0).toULongLong();
            vec.push_back(count);
            msectime = query.value(1).toULongLong();
            vec.push_back(msectime);
        }
    }
    return vec;

}
bool DatabaseManager::removeSpace(const QString &spaceName, QString *errorMessage)
{
    QSqlQuery query(priv->db);
    query.prepare("UPDATE Spaces SET Name=:randomName,Active=false,AutoUpdate=false WHERE Name=:n");
    QString randomString = createRandomString(64);
    query.bindValue(":randomName", randomString);
    query.bindValue(":n", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        priv->spaces.remove(priv->spaces.key(spaceName));
        return true;
    }
}

bool DatabaseManager::getSpacestatus(const QString &spaceName){
    QString errorMessage;
    QSqlQuery query(priv->db);
    query.prepare("SELECT AutoUpdate FROM Spaces WHERE Name=:n");
    query.bindValue(":n", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), &errorMessage);
        return false;
    } else {
        if (query.next()) {
            bool status = query.value(0).toBool();
            return status;
        }
    }
    return true;
}

bool DatabaseManager::updateSpacestatus(const QString &spaceName,bool status){
    QString errorMessage;
    QSqlQuery query(priv->db);
    query.prepare("UPDATE Spaces SET AutoUpdate=:s WHERE Name=:n");
    query.bindValue(":s", status);
    query.bindValue(":n", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), &errorMessage);
        return false;
    } else {
         return true;
    }
}
QStringList DatabaseManager::getSpaceCondition(const QString &spaceName, QString *errorMessage)
{
    QStringList l;

    QSqlQuery query(priv->db);
    query.prepare("SELECT Conditions from Spaces WHERE name=:n");
    query.bindValue(":n", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return l;
    } else {
        if (query.next()) {
            QString conditions = query.value(0).toString();
            l = conditions.split('\n');
            return l;
        }
    }

    return l;
}

bool DatabaseManager::changeSpaceConditions(const QString &spaceName, const QStringList &conditions, QString *errorMessage)
{
    QSqlQuery query(priv->db);
    query.prepare("UPDATE Spaces SET Conditions=:c WHERE Name=:n");
    query.bindValue(":c", conditions.join('\n'));
    query.bindValue(":n", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        return true;
    }
}

bool DatabaseManager::addFileToSpace(const QString &filePath, const QString &spaceName, bool isUserAdded, QString *errorMessage)
{
    int spaceId = getSpaceIdByName(spaceName);

    return addFileToSpace(filePath, spaceId, isUserAdded, errorMessage);
}

bool DatabaseManager::addFileToSpace(const QString &filePath, int spaceId, bool isUserAdded, QString *errorMessage)
{
    // 更新或者插入文件空间信息
    // fixme: 使用文件id信息进行空间管理
    QString targetTable = isUserAdded? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    QString cmd = QString("SELECT Path,id,SpaceIds FROM %1 WHERE Path=:path").arg(targetTable);
    query.prepare(cmd);
    query.bindValue(":path", filePath);

    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        QString path;
        QString oldSpaces;
        int id;
        if (query.next()) {
            // 已经加入至少一个空间，更新信息
            path = query.value("path").toString();
            oldSpaces = query.value("SpaceIds").toString();
            id = query.value("id").toInt();

            QString spaceName = priv->spaces.value(spaceId);

            QStringList spaces = oldSpaces.split('\n');
            if (spaces.contains(QString::number(spaceId))) {
                setErrorString(tr("File %1 has areally been added to %2").arg(filePath).arg(spaceName), errorMessage);
                return false;
            } else {
                spaces << QString::number(spaceId);
                QString targetString = spaces.join('\n');
                QSqlQuery updateQuery(priv->filedb);
                updateQuery.prepare(QString("UPDATE %1 SET SpaceIds=:SpaceIds WHERE Path=:path").arg(targetTable));
                updateQuery.bindValue(":SpaceIds", targetString);
                updateQuery.bindValue(":path", filePath);
                bool ok = updateQuery.exec();
                if (!ok) {
                    setErrorString(updateQuery.lastError().text(), errorMessage);
                    return false;
                } else {
                    return true;
                }
            }
        } else {
            // 文件未在任何空间中，添加数据
            int spaceIdToBeAdded = spaceId;

            QSqlQuery insertQuery(priv->filedb);
            insertQuery.prepare(QString("INSERT INTO %1 (Path,SpaceIds) VALUES (:path,:space)").arg(targetTable));
            insertQuery.bindValue(":path", filePath);
            insertQuery.bindValue(":space", QString::number(spaceIdToBeAdded));
            bool ok = insertQuery.exec();
            if (!ok) {
                setErrorString(insertQuery.lastError().text(), errorMessage);
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}

bool DatabaseManager::removeFileFromSpace(const QString &filePath, const QString &spaceName, bool isUserRemove, QString *errorMessage)
{
    int spaceId = getSpaceIdByName(spaceName);
    if (spaceId < 0) {
        qCritical() << "invalid space id" << spaceId << "from" << spaceName;
        return false;
    }

    return removeFileFromSpace(filePath, spaceId, isUserRemove, errorMessage);
}

bool DatabaseManager::removeFileFromSpace(const QString &filePath, int spaceId, bool isUserRemove, QString *errorMessage)
{
    // 更新文件空间信息，如果文件不存在，返回false
    // fixme: 使用文件id信息进行空间管理
    QString targetTable = isUserRemove? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    query.prepare(QString("SELECT Path,id,SpaceIds FROM %1 WHERE Path=:path").arg(targetTable));
    query.bindValue(":path", filePath);

    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        if (query.next()) {
            QString path;
            QString oldSpaces;
            int id;
            // 更新信息
            path = query.value("path").toString();
            oldSpaces = query.value("SpaceIds").toString();
            id = query.value("id").toInt();

            QStringList spaces = oldSpaces.split('\n');
            spaces.removeAll(QString::number(spaceId));
            QString targetString = spaces.join('\n');

            if (spaces.isEmpty()) {
                // 如果删除此文件后没有数据则清空此记录
                QSqlQuery updateQuery(priv->filedb);
                QString cmd = QString("DELETE FROM %1 WHERE Path=:path").arg(targetTable);
                updateQuery.prepare(cmd);
                updateQuery.bindValue(":path", filePath);
                bool ok = updateQuery.exec();
                if (!ok) {
                    setErrorString(updateQuery.lastError().text(), errorMessage);
                    return false;
                } else {
                    return true;
                }
            } else {
                QSqlQuery updateQuery(priv->filedb);
                QString cmd = QString("UPDATE %1 SET SpaceIds=:SpaceIds WHERE Path=:path").arg(targetTable);
                updateQuery.prepare(cmd);
                updateQuery.bindValue(":SpaceIds", targetString);
                updateQuery.bindValue(":path", filePath);
                bool ok = updateQuery.exec();
                if (!ok) {
                    setErrorString(updateQuery.lastError().text(), errorMessage);
                    return false;
                } else {
                    return true;
                }
            }
        } else {
            setErrorString(tr("File %1 doesn't exsit in any space.").arg(filePath), errorMessage);
            return false;
        }
    }

    return false;
}

bool DatabaseManager::removeFileFromAllSpace(const QString &filePath, QString *errorMessage)
{
    QSqlQuery query(priv->filedb);
    query.prepare("UPDATE UserAddedFiles SET SpaceIds=NULL WHERE Path=:path");
    query.bindValue(":path", filePath);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    }

    query.prepare("UPDATE AutoAddedFiles SET SpaceIds=NULL WHERE Path=:path");
    query.bindValue(":path", filePath);
    ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return false;
    } else {
        return true;
    }

    return false;
}

QStringList DatabaseManager::getAllFilesFromSpaces(bool isUserAdded, QString *errorMessage)
{
    QStringList l;

    QString targetTable = isUserAdded? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    QString cmd = QString("SELECT Path FROM %1 WHERE SpaceIds IS NOT NULL").arg(targetTable);
    query.prepare(cmd);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return l;
    } else {
        while (query.next()) {
            l << query.value(0).toString();
        }
    }

    return l;
}

QList<QPair<QString, QList<int> > > DatabaseManager::getAllFilesWithSpaceIds(bool isUserAdded, QString *errorMessage)
{
    QList<QPair<QString, QList<int>>> l;

    QString targetTable = isUserAdded? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    QString cmd = QString("SELECT Path,SpaceIds FROM %1 WHERE SpaceIds IS NOT NULL").arg(targetTable);
    query.prepare(cmd);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return l;
    } else {
        while (query.next()) {
            auto path = query.value(0).toString();
            QString spaceIds = query.value(1).toString();

            if (spaceIds.isEmpty() || spaceIds == "") {
                qWarning() << "file" << path << "has no spaceIds value";
                continue;
            }

            QStringList ids = spaceIds.split('\n');
            QList<int> idsList;
            for (QString idString : ids) {
                bool ok = false;
                int id = idString.toInt(&ok);
                if (!ok) {
                    qWarning()<< QString("Parse file %3 space ids %2 failed, current string: %1.").arg(idString).arg(spaceIds).arg(path);
                } else {
                    idsList << id;
                }
            }
            QPair<QString, QList<int>> pathWithSpaceIds;
            pathWithSpaceIds.first = path;
            pathWithSpaceIds.second = idsList;
            l << pathWithSpaceIds;
        }
    }


    return l;
}

QMap<int, QStringList> DatabaseManager::getAllFilesFromSpacesBySpaceIds(bool isUserAdded, QString *errorMessage)
{
    QMap<int, QStringList> l;

    QString targetTable = isUserAdded? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    QString cmd = QString("SELECT Path,SpaceIds FROM %1 WHERE SpaceIds IS NOT NULL").arg(targetTable);
    query.prepare(cmd);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return l;
    } else {
        while (query.next()) {
            auto path = query.value(0).toString();
            QString spaceIds = query.value(1).toString();
            QStringList ids = spaceIds.split('\n');
            for (QString idString : ids) {
                bool ok = false;
                int id = idString.toInt(&ok);
                if (!ok) {
                    qWarning()<< QString("Parse file %3 space ids %2 failed, current string: %1.").arg(idString).arg(spaceIds).arg(path);
                } else {
                    if (l.keys().contains(id)) {
                        QStringList newList = l.value(id);
                        l.insert(id, newList<<path);
                    } else {
                        l.insert(id, QStringList()<<path);
                    }
                }
            }
        }
    }

    return l;
}

int DatabaseManager::getSpaceIdByName(const QString &spaceName, QString *errorMessage)
{
    int cachedId = priv->spaces.key(spaceName, -1);
    if (cachedId > 0) {
        return cachedId;
    }

    QSqlQuery query(priv->db);
    query.prepare("SELECT id FROM Spaces WHERE name=:name");
    query.bindValue(":name", spaceName);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return -1;
    } else {
        if (query.next()) {
            int id = query.value(0).toInt(&ok);
            if (!ok) {
                setErrorString(tr("Failed to get space %1's id from query").arg(spaceName), errorMessage);
                return -1;
            }
            return id;
        }
    }
    return -1;
}

int DatabaseManager::getFileIdByPath(const QString &filePath, bool isUserAdded, QString *errorMessage)
{
    // fixme: 使用缓存管理
    QString targetTable = isUserAdded? "UserAddedFiles": "AutoAddedFiles";
    QSqlQuery query(priv->filedb);
    // 表名不能作为绑定变量
    //query.prepare("SELECT id FROM :table WHERE Path=:path");
    //query.bindValue(":table", targetTable);
    QString cmd = QString("SELECT id FROM %1 WHERE Path=:path").arg(targetTable);
    query.prepare(cmd);
    query.bindValue(":path", filePath);
    bool ok = query.exec();
    if (!ok) {
        setErrorString(query.lastError().text(), errorMessage);
        return -1;
    } else {
        if (query.next()) {
            int id = query.value(0).toInt(&ok);
            if (!ok) {
                setErrorString(tr("Failed to get %1's id from query").arg(filePath), errorMessage);
                return -1;
            }
            return id;
        }
    }
    return -1;
}

void DatabaseManager::setErrorString(const QString &errorMessage, QString *targetString)
{
    if (targetString)
        *targetString = errorMessage;
}
