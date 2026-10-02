/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#include "app-info-table.h"
#include "app-info-table-private.h"
#include "app-info-dbus-argument.h"
#include "app-data-utils.h"

#include <QDebug>
#include <QtGlobal>
#include <string>
#include <QStandardPaths>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QTime>
#include <QSqlQuery>
#include <QSqlError>
#include <mutex>
#include <QFile>
#include <application-property-helper.h>
#include <dbus-connector.h>
#include <QMutex>
#include <QWaitCondition>
#include "sqlite-database-utils.h"

using namespace UkuiSearch;

namespace {
const QStringList &desktopPaths()
{
    static const QStringList paths = AppDataUtils::getDesktopPaths();
    return paths;
}
}

QMutex AppInfoTablePrivate::s_mutex;

AppInfoTablePrivate::AppInfoTablePrivate(AppInfoTable *parent) : QObject(parent), q(parent), m_database(new QSqlDatabase())
{
    //dbus接收数据库信号
    qRegisterMetaType<UkuiSearch::ApplicationProperty::Property>("ApplicationProperty::Property");
    qRegisterMetaType<UkuiSearch::ApplicationInfoMap>("ApplicationInfoMap");
    qRegisterMetaType<UkuiSearch::ApplicationPropertyMap>("ApplicationPropertyMap");
    qDBusRegisterMetaType<UkuiSearch::ApplicationProperty::Property>();
    qDBusRegisterMetaType<ApplicationPropertyMap>();
    qDBusRegisterMetaType<ApplicationInfoMap>();

    s_mutex.lock();
    auto sigTransConnector = new UkuiQuick::DbusConnector(QStringLiteral("com.ukui.search.appdb.service"),
                                                          QStringLiteral("/org/ukui/search/appDataBase/signalTransformer"),
                                                          QStringLiteral("org.ukui.search.signalTransformer"),
                                                          QDBusConnection::sessionBus());

    connect(sigTransConnector, &UkuiQuick::DbusConnector::ready, this, [ this, sigTransConnector ](QDBusInterface *dbusInterface) {
        QMutexLocker locker(&s_mutex);
        m_isReady = true;
        if (dbusInterface->isValid()) {
            m_signalTransInterface = dbusInterface;
            connect(m_signalTransInterface, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)), this, SLOT(sendAppDBItemsUpdate(ApplicationInfoMap)));
            connect(m_signalTransInterface, SIGNAL(appDBItemsUpdateAll(QStringList)), this, SLOT(sendAppDBItemsUpdateAll(QStringList)));
            connect(m_signalTransInterface, SIGNAL(appDBItemsAdd(QStringList)), this, SLOT(sendAppDBItemsAdd(QStringList)));
            connect(m_signalTransInterface, SIGNAL(appDBItemsDelete(QStringList)), this, SLOT(sendAppDBItemsDelete(QStringList)));
        }
        if (sigTransConnector) {
            sigTransConnector->deleteLater();
        }
        m_waitCondition.wakeAll();
    }, Qt::DirectConnection);
    sigTransConnector->start();

    // 使用 while 而非 if：防止条件变量伪唤醒(spurious wakeup)导致在 m_isReady 仍为 false 时继续执行
    while (!m_isReady) {
        qDebug() << "waitCondition is going to wait";
        if (!m_waitCondition.wait(&s_mutex, 300000)) {
            qDebug() << "SignalTransformer interface timeout";
            break;
        }
    }

    // 在锁保护下检查 m_signalTransInterface，避免超时后与晚到的 ready 回调产生竞争
    bool interfaceValid = m_signalTransInterface && m_signalTransInterface->isValid();
    s_mutex.unlock();

    if (!interfaceValid) {
        qCritical() << "Create signalTransformer Interface Failed Because: " << QDBusConnection::sessionBus().lastError();
    }

    //使用dbus操作数据库获取数据
    m_appDBInterface = new QDBusInterface("com.ukui.search.appdb.service",
                                          "/org/ukui/search/appDataBase/dbManager",
                                          "org.ukui.search.appDBManager");

    while(1) {
        srand(QTime(0,0,0).secsTo(QTime::currentTime()));
        m_ConnectionName = QString::fromStdString(std::to_string(rand()));//随机生产链接
        if (!QSqlDatabase::contains(m_ConnectionName))
            break;
    }
    qDebug() << "App info database currunt connection name:" << m_ConnectionName;
    if (!this->openDataBase()) {
        Q_EMIT q->DBOpenFailed();
        qWarning() << "Fail to open App DataBase, because:" << m_database->lastError();
    }
}

void AppInfoTablePrivate::setAppFavoritesState(const QString &desktopfp)
{
    m_appDBInterface->call("updateFavoritesState", desktopfp);
}

void AppInfoTablePrivate::setAppFavoritesState(const QString &desktopfp, uint num)
{
    m_appDBInterface->call("updateFavoritesState", desktopfp, num);
}

void AppInfoTablePrivate::setAppTopState(const QString &desktopfp)
{
    m_appDBInterface->call("updateTopState", desktopfp);
}

void AppInfoTablePrivate::setAppTopState(const QString &desktopfp, uint num)
{
    m_appDBInterface->call("updateTopState", desktopfp, num);
}

void AppInfoTablePrivate::setAppLaunchedState(const QString &desktopFilePath, bool launched)
{
    ApplicationInfoMap infoMap;
    infoMap[desktopFilePath].insert(ApplicationProperty::Launched, launched);
    QVariant var;
    var.setValue(infoMap);
    m_appDBInterface->call("setValue", var);
}

bool AppInfoTablePrivate::searchInstallApp(QString &keyWord, QStringList &installAppInfoRes)
{
    bool res(true);
    QSqlQuery sql(*m_database);
    QString cmd;
    if (keyWord.size() < 2) {
        cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON FROM APPINFO WHERE LOCAL_NAME OR NAME_EN OR NAME_ZH OR FIRST_LETTER_OF_PINYIN LIKE '%%0%' ORDER BY FAVORITES DESC")
                .arg(keyWord);
    } else {
        cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON FROM APPINFO WHERE LOCAL_NAME OR NAME_EN OR NAME_ZH OR PINYIN_NAME OR FIRST_LETTER_OF_PINYIN LIKE '%%0%' ORDER BY FAVORITES DESC")
                .arg(keyWord);
    }

    if (sql.exec(cmd)) {
        while (sql.next()) {
            installAppInfoRes << sql.value(0).toString() << sql.value(1).toString() << sql.value(2).toString();
        }
    } else {
        qWarning() << QString("cmd %0 failed!").arg(cmd) << m_database->lastError();
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes)
{
    if (keyWord.isEmpty()) {
        qWarning() << "searchInstallApp: keyword list is empty";
        return false;
    }

    bool res(true);
    QSqlQuery sql(*m_database);
    QString cmd;
    if (keyWord.at(0).size() < 2) {
        cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON,NAME_EN,NAME_ZH,FIRST_LETTER_OF_PINYIN FROM APPINFO"
                      " WHERE LOCAL_NAME LIKE '%%0%' OR NAME_EN LIKE '%%0%' OR NAME_ZH LIKE '%%0%' OR FIRST_LETTER_OF_PINYIN LIKE '%%0%'")
                .arg(keyWord.at(0));
    } else {
        cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON,NAME_EN,NAME_ZH,FIRST_LETTER_OF_PINYIN FROM APPINFO"
                      " WHERE LOCAL_NAME LIKE '%%0%' OR NAME_EN LIKE '%%0%' OR NAME_ZH LIKE '%%0%' OR PINYIN_NAME LIKE '%%0%' OR FIRST_LETTER_OF_PINYIN LIKE '%%0%'")
                .arg(keyWord.at(0));
    }
    for (int i = 0; ++i<keyWord.size();) {
        if (keyWord.at(i).size() < 2) {
            cmd += QString(" AND (LOCAL_NAME LIKE '%%1%' OR NAME_EN LIKE '%%1%' OR NAME_ZH LIKE '%%1%' OR FIRST_LETTER_OF_PINYIN LIKE '%%1%')")
                    .arg(keyWord.at(i));
        } else {
            cmd += QString(" AND (LOCAL_NAME LIKE '%%1%' OR NAME_EN LIKE '%%1%' OR NAME_ZH LIKE '%%1%' OR PINYIN_NAME LIKE '%%1%' OR FIRST_LETTER_OF_PINYIN LIKE '%%1%')")
                    .arg(keyWord.at(i));
        }
    }
    cmd += QString(" ORDER BY LENGTH(LOCAL_NAME)");
    if (sql.exec(cmd)) {
        while (sql.next()) {
            installAppInfoRes << sql.value(0).toString() << sql.value(1).toString() << sql.value(2).toString();
        }
    } else {
        qWarning() << QString("cmd %0 failed!").arg(cmd) << m_database->lastError();
        res = false;
    }
    return res;
}

QString AppInfoTablePrivate::lastError() const
{
    return m_database->lastError().text();
}

bool AppInfoTablePrivate::tranPidToDesktopFp(uint pid, QString &desktopfp)
{
    QDBusReply<QString> reply = m_appDBInterface->call("tranPidToDesktopFp", pid);
    if (reply.isValid()) {
        desktopfp = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp)
{
    QDBusReply<QString> reply = m_appDBInterface->call("tranWinIdToDesktopFilePath", QVariant::fromValue(QDBusVariant(winId)));
    if (reply.isValid()) {
        desktopfp = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath)
{
    QDBusReply<QString> reply = m_appDBInterface->call("desktopFilePathFromName", desktopFileName);
    if (reply.isValid()) {
        desktopFilePath = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::setAppLaunchTimes(const QString &desktopfp, size_t num)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LAUNCH_TIMES=%1, LAUNCHED=%2 WHERE DESKTOP_FILE_PATH='%3'")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(num)
                .arg(1)
                .arg(desktopfp);
        if (!sql.exec(cmd)) {
            qWarning() << "Set app favorites state failed!" << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::updateAppLaunchTimes(const QString &desktopfp)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("SELECT LAUNCH_TIMES FROM APPINFO WHERE DESKTOP_FILE_PATH='%1'").arg(desktopfp);
        if (sql.exec(cmd)) {
            if (sql.next()) {
                cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LAUNCH_TIMES=%1, LAUNCHED=%2 WHERE DESKTOP_FILE_PATH='%3'")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(sql.value(0).toInt() + 1)
                        .arg(1)
                        .arg(desktopfp);
                if (!sql.exec(cmd)) {
                    qWarning() << "Set app favorites state failed!" << m_database->lastError();
                    res = false;
                }
            } else {
                qWarning() << "Failed to exec next!" << cmd;
                res = false;
            }
        } else {
            qWarning() << "Failed to exec:" << cmd;
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::setAppLockState(const QString &desktopfp, size_t num)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LOCK=%1 WHERE DESKTOP_FILE_PATH='%2'")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(num)
                .arg(desktopfp);
        if (!sql.exec(cmd)) {
            qWarning() << "Set app favorites state failed!" << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

AppInfoTablePrivate::~AppInfoTablePrivate()
{
    this->closeDataBase();
    if (m_signalTransInterface)
        delete m_signalTransInterface;
    m_signalTransInterface = nullptr;
    if (m_appDBInterface)
        delete m_appDBInterface;
    m_appDBInterface = nullptr;
}

bool AppInfoTablePrivate::initDateBaseConnection()
{
    m_database->setDatabaseName(APP_DATABASE_PATH + APP_DATABASE_NAME);
    QString errorMessage;
    if (!configureSqliteDatabase(*m_database, SqliteDatabaseMode::FileBacked, &errorMessage)) {
        qWarning() << errorMessage;
        return false;
    }
    return true;
}

bool AppInfoTablePrivate::openDataBase()
{
    *m_database = QSqlDatabase::addDatabase("QSQLITE", m_ConnectionName);
    m_database->setDatabaseName(APP_DATABASE_PATH + APP_DATABASE_NAME);

    QString errorMessage;
    if (!configureSqliteDatabase(*m_database, SqliteDatabaseMode::FileBacked, &errorMessage)) {
        qWarning() << errorMessage;
        return false;
    }
    return true;
}

void AppInfoTablePrivate::closeDataBase()
{
    m_database->close();
    delete m_database;
    m_database = nullptr;
    QSqlDatabase::removeDatabase(m_ConnectionName);
}

void AppInfoTablePrivate::sendAppDBItemsUpdate(ApplicationInfoMap results)
{
    Q_EMIT q->appDBItems2BUpdate(results);
}

void AppInfoTablePrivate::sendAppDBItemsUpdateAll(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BUpdateAll(desktopFilePaths);
}

void AppInfoTablePrivate::sendAppDBItemsAdd(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BAdd(desktopFilePaths);
}

void AppInfoTablePrivate::sendAppDBItemsDelete(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BDelete(desktopFilePaths);
}

bool AppInfoTablePrivate::shouldInsert(QHash<QString, QString> &nameHash, ApplicationInfoMap &infoMap, const QString &md5, const QString &desktopFilePath) {
    QString desktopFileId = calcDesktopFileId(desktopFilePath);
    bool needInsert = true;
    QString previousFilePath = nameHash.value(md5);
    if (previousFilePath.isEmpty()) {
        previousFilePath = nameHash.value(desktopFileId);
    }

    if (!previousFilePath.isEmpty()) {
        const int currentPriority = desktopPathPriority(desktopFilePath);
        const int previousPriority = desktopPathPriority(previousFilePath);
        // Prefer entries under configured desktop paths. A smaller index means a higher-priority
        // desktop directory. If neither path is under a configured directory, keep the old behavior.
        if (currentPriority >= 0 && (previousPriority < 0 || currentPriority <= previousPriority)) {
            infoMap.remove(previousFilePath);
            for (const auto &key : nameHash.keys(previousFilePath)) {
                nameHash[key] = desktopFilePath;
            }
        } else {
            needInsert = false;
        }
    }

    nameHash[md5] = needInsert ? desktopFilePath : previousFilePath;
    nameHash[desktopFileId] = needInsert ? desktopFilePath : previousFilePath;
    return needInsert;
}

int AppInfoTablePrivate::desktopPathPriority(const QString &desktopFilePath)
{
    const QStringList &paths = desktopPaths();
    for (int i = 0; i < paths.size(); ++i) {
        if (desktopFilePath.startsWith(paths.at(i) + "/")) {
            return i;
        }
    }
    return -1;
}

QString AppInfoTablePrivate::calcDesktopFileId(const QString &desktopFilePath)
{
    for (const auto &parentPath: desktopPaths()) {
        if (desktopFilePath.startsWith(parentPath + "/")) {
            QString desktopFileId = desktopFilePath.mid(parentPath.size() + 1);
            desktopFileId.replace("/", "-");
            return desktopFileId;
        }
    }

    return QFileInfo(desktopFilePath).fileName();
}

AppInfoTable::AppInfoTable(QObject *parent) : QObject(parent), d(new AppInfoTablePrivate(this))
{
}

bool AppInfoTable::query(ApplicationPropertyMap &propertyMap, const QString &desktopFile, const ApplicationProperties& properties)
{
    QString field;
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    field.remove(field.length() - 1, 1);
    if(field.isEmpty()) {
        return true;
    }
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(QString("SELECT %0 FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFile").arg(field));
    query.bindValue(":desktopFile", desktopFile);
    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError();
        return false;
    }
    while (query.next()) {
        for(int i = 0; i< properties.size(); i++) {
            propertyMap.insert(properties.at(i), query.value(i));
        }
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties)
{
    QString field = ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField() + ",";
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField());
    } else if(!field.isEmpty()) {
        field.remove(field.length() - 1, 1);
    }

    QString sql = QString("SELECT %0 FROM APPINFO").arg(field);
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);

    if (!query.exec(sql)) {
        qWarning() << d->m_database->lastError() << sql;
        return false;
    }
    QHash<QString, QString> nameHash;//md5/desktopFileId, desktopFilePath
    while (query.next()) {
        QString desktopFilePath = query.value(ApplicationPropertyHelper(ApplicationProperty::DesktopFilePath).dataBaseField()).toString();
        QString md5 = query.value(ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField()).toString();
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));
        }

        if (d->shouldInsert(nameHash, infoMap, md5, desktopFilePath)) {
            infoMap.insert(desktopFilePath, propertyMap);
        }
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const ApplicationPropertyMap& restrictions)
{
    QString field = ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField() + ",";
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField());
    } else if(!field.isEmpty()) {
        field.remove(field.length() - 1, 1);
    }

    QString condition;
    for (const ApplicationProperty::Property prop: restrictions.keys()) {
        condition.append(ApplicationPropertyHelper(prop).dataBaseField() + "=? AND ");
    }
    condition = condition.left(condition.lastIndexOf(" AND "));

    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(QString("SELECT %0 FROM APPINFO WHERE %1")
                  .arg(field)
                  .arg(condition));
    int i = 0;
    for (const QVariant &conditionValue : restrictions) {
        query.bindValue(i, conditionValue);
        i++;
    }
    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError();
        return false;
    }

    QHash<QString, QString> nameHash;//md5/desktopFileId, desktopFilePath
    while (query.next()) {
        QString desktopFilePath = query.value(ApplicationPropertyHelper(ApplicationProperty::DesktopFilePath).dataBaseField()).toString();
        QString md5 = query.value(ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField()).toString();
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));
        }

        if (d->shouldInsert(nameHash, infoMap, md5, desktopFilePath)) {
            infoMap.insert(desktopFilePath, propertyMap);
        }
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const QStringList &keywords, const ApplicationPropertyMap& restrictions)
{
    QString field = ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField() + ",";
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField() + ",");
    }
    field.append(ApplicationPropertyHelper(ApplicationProperty::Property::LocalKeywords).dataBaseField());
    QString condition;
    for (const ApplicationProperty::Property prop: restrictions.keys()) {
        condition.append(ApplicationPropertyHelper(prop).dataBaseField() + "=? AND ");
    }
    QString keywordCondition;
    QString findWithLocalKeywords =  "((select count(*) from ("
                                     "WITH RECURSIVE split(str, rest) AS "
                                     "("
                                     "SELECT substr(LOCAL_KEYWORDS || ';', 1, instr(LOCAL_KEYWORDS || ';', ';') - 1),"
                                     "substr(LOCAL_KEYWORDS || ';', instr(LOCAL_KEYWORDS || ';', ';') + 1) "
                                     "UNION ALL "
                                     "SELECT substr(rest, 1, instr(rest, ';') - 1), substr(rest, instr(rest, ';') + 1) "
                                     "FROM split WHERE str <> ''"
                                     ")"
                                     "SELECT str FROM split WHERE lower(str)=?)) <> 0)";
    for(const QString& keyword : keywords) {
        if(keyword.size() < 2) {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        } else {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or ifnull(PINYIN_NAME, '') like ? or ifnull(replace(PINYIN_NAME,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        }
    }
    if(!keywordCondition.isEmpty()) {
        keywordCondition.remove(keywordCondition.length() - 3, 3);
    }

    QString sql = QString("SELECT %0 FROM APPINFO WHERE %1 %2 ORDER BY LENGTH(LOCAL_NAME)").arg(field, condition, keywordCondition);
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(sql);

    int count = 0;
    for (const QVariant &conditionValue : restrictions) {
        query.bindValue(count, conditionValue);
        count++;
    }

    for(const QString &keyword : keywords) {
        int i = 11;
        if(keyword.size() < 2) {
            i = i - 2;
        }
        for (int bindCount = 0; bindCount < i; bindCount++) {
            query.bindValue(count, (bindCount - i + 1) ? "%" + keyword + "%" : keyword.toLower());
            count++;
        }
    }

    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError() << query.lastQuery() << query.boundValues();
        return false;
    }

    QHash<QString, QString> nameHash;//md5/desktopFileId, desktopFilePath
    while (query.next()) {
        QString desktopFilePath = query.value(ApplicationPropertyHelper(ApplicationProperty::DesktopFilePath).dataBaseField()).toString();
        QString md5 = query.value(ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField()).toString();
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));
        }

        if (d->shouldInsert(nameHash, infoMap, md5, desktopFilePath)) {
            infoMap.insert(desktopFilePath, propertyMap);
        }
    }
    return true;
}

bool AppInfoTable::query(QList<ApplicationInfoMap> &infoList, const ApplicationProperties& properties, const QStringList &keywords, const ApplicationPropertyMap& restrictions)
{
    QString field = ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField() + ",";
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField() + ",");
    }
    field.append(ApplicationPropertyHelper(ApplicationProperty::Property::LocalKeywords).dataBaseField());
    QString condition;
    for (const ApplicationProperty::Property prop: restrictions.keys()) {
        condition.append(ApplicationPropertyHelper(prop).dataBaseField() + "=? AND ");
    }
    QString keywordCondition;
    QString findWithLocalKeywords =  "((select count(*) from ("
                                     "WITH RECURSIVE split(str, rest) AS "
                                     "("
                                     "SELECT substr(LOCAL_KEYWORDS || ';', 1, instr(LOCAL_KEYWORDS || ';', ';') - 1),"
                                     "substr(LOCAL_KEYWORDS || ';', instr(LOCAL_KEYWORDS || ';', ';') + 1) "
                                     "UNION ALL "
                                     "SELECT substr(rest, 1, instr(rest, ';') - 1), substr(rest, instr(rest, ';') + 1) "
                                     "FROM split WHERE str <> ''"
                                     ")"
                                     "SELECT str FROM split WHERE lower(str)=?)) <> 0)";
    for(const QString& keyword : keywords) {
        if(keyword.size() < 2) {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        } else {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or ifnull(PINYIN_NAME, '') like ? or ifnull(replace(PINYIN_NAME,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        }
    }
    if(!keywordCondition.isEmpty()) {
        keywordCondition.remove(keywordCondition.length() - 3, 3);
    }

    QString sql = QString("SELECT %0, "
                          "(julianday(datetime('now'))-julianday(datetime(INSERT_TIME))) AS installDays "
                          "FROM APPINFO WHERE %1 %2 ORDER BY "
                          "IIF(installDays > 10, (LAUNCH_TIMES * 240 / (installDays - 6)), (LAUNCH_TIMES * (-0.4 * installDays * installDays + 100))) DESC")
            .arg(field, condition, keywordCondition);
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(sql);

    int count = 0;
    for (const QVariant &conditionValue : restrictions) {
        query.bindValue(count, conditionValue);
        count++;
    }

    for(const QString &keyword : keywords) {
        int i = 11;
        if(keyword.size() < 2) {
            i = i - 2;
        }
        for (int bindCount = 0; bindCount < i; bindCount++) {
            query.bindValue(count, (bindCount - i + 1) ? "%" + keyword + "%" : keyword.toLower());
            count++;
        }
    }

    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError() << query.lastQuery() << query.boundValues();
        return false;
    }

    QHash<QString, int> indexHash;//md5/desktopFileId->index

    while (query.next()) {
        QString desktopFilePath = query.value(ApplicationPropertyHelper(ApplicationProperty::DesktopFilePath).dataBaseField()).toString();
        QString md5 = query.value(ApplicationPropertyHelper(ApplicationProperty::Md5).dataBaseField()).toString();
        QString desktopFileId = AppInfoTablePrivate::calcDesktopFileId(desktopFilePath);
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));
        }

        int index = indexHash.value(md5, -1);
        if (index < 0) {
            index = indexHash.value(desktopFileId, -1);
        }

        bool needInsert = true;
        if (index >= 0) {
            const QString previousFilePath = infoList.at(index).firstKey();
            const int currentPriority = AppInfoTablePrivate::desktopPathPriority(desktopFilePath);
            const int previousPriority = AppInfoTablePrivate::desktopPathPriority(previousFilePath);
            // Reuse the same directory-priority rule as shouldInsert() so list and map queries
            // keep the same deduplication behavior.
            if (currentPriority >= 0 && (previousPriority < 0 || currentPriority <= previousPriority)) {
                infoList.replace(index, {{desktopFilePath, propertyMap}});
                needInsert = false;
            } else {
                needInsert = false;
            }
        }

        if (needInsert) {
            infoList.append({{desktopFilePath, propertyMap}});
            index = infoList.size() - 1;
        }
        indexHash[md5] = index;
        indexHash[desktopFileId] = index;
    }

    return true;
}

void AppInfoTable::setAppFavoritesState(const QString &desktopfp)
{
    return d->setAppFavoritesState(desktopfp);
}

void AppInfoTable::setAppFavoritesState(const QString &desktopfp, size_t num)
{
    return d->setAppFavoritesState(desktopfp, num);
}

void AppInfoTable::setAppTopState(const QString &desktopfp, size_t num)
{
    return d->setAppTopState(desktopfp, num);
}

void AppInfoTable::setAppLaunchedState(const QString &desktopFilePath, bool launched)
{
    return d->setAppLaunchedState(desktopFilePath, launched);
}

void AppInfoTable::setAppTopState(const QString &desktopfp)
{
    return d->setAppTopState(desktopfp);
}

bool AppInfoTable::searchInstallApp(QString &keyWord, QStringList &installAppInfoRes)
{
    return d->searchInstallApp(keyWord, installAppInfoRes);
}

bool AppInfoTable::searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes)
{
    return d->searchInstallApp(keyWord, installAppInfoRes);
}

QString AppInfoTable::lastError() const
{
    return d->lastError();
}

bool AppInfoTable::tranPidToDesktopFp(uint pid, QString &desktopfp)
{
    return d->tranPidToDesktopFp(pid, desktopfp);
}

bool AppInfoTable::desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath)
{
    return d->desktopFilePathFromName(desktopFileName, desktopFilePath);
}

bool AppInfoTable::tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp)
{
    return d->tranWinIdToDesktopFilePath(winId, desktopfp);
}

//下面接口暂时没啥用，不外放。
bool AppInfoTable::setAppLaunchTimes(const QString &desktopfp, size_t num)
{
    return d->setAppLaunchTimes(desktopfp, num);
}

bool AppInfoTable::updateAppLaunchTimes(const QString &desktopfp)
{
    return d->updateAppLaunchTimes(desktopfp);
}

bool AppInfoTable::setAppLockState(const QString &desktopfp, size_t num)
{
    return d->setAppLockState(desktopfp, num);
}

AppInfoTable &AppInfoTable::instance() {
    static AppInfoTable instance;
    return instance;
}
