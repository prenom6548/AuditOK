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
#include "DataBaseInterface.h"

QScopedPointer<DataBaseInterface> DataBaseInterface::ms_instance;
DataBaseInterface::DataBaseInterface(const QString &fileFolderPath, const QString &dataBaseName, const QString &connectName, QObject *parent)
    :QObject(parent),m_fileFolderPath(fileFolderPath),m_dataBaseName(dataBaseName),m_connectName(connectName)
{
    QStringList driverList = QSqlDatabase::drivers();
    foreach(const QString &tempDriver, driverList)
    {
        if(tempDriver == s_sqliteName)
        {
            checkDir();
            m_db = QSqlDatabase::addDatabase(s_sqliteName,connectName);
            m_db.setDatabaseName(fileFolderPath+dataBaseName);
            m_db.open();
            qDebug() << "has feature transactions "<<m_db.driver()->hasFeature(QSqlDriver::DriverFeature::Transactions);
            
        }
    }
}

DataBaseInterface::~DataBaseInterface()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
}

DataBaseInterface *DataBaseInterface::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new DataBaseInterface(KylinDaqCommon::gs_cacheFolderPath,KylinDaqCommon::gs_dataBaseName,KylinDaqCommon::gs_connectionName));
    }
    return ms_instance.data();
}

bool DataBaseInterface::createTable(const QString &createString)
{
    QMutexLocker mutexLocker(&m_mutex);
    QSqlQuery query(m_db);
    bool state = query.exec(createString);
    if(state == false)
    {
       qDebug() << "error :"<<query.lastError();
    }
    return state;
}

bool DataBaseInterface::createTable(const QString &tableName, const QMap<QString,SqliteDataType> &keyType)
{
    QMutexLocker mutexLocker(&m_mutex);
    QList<QString> createStrList;
    QSqlQuery query(m_db);
    QMetaEnum enumState = QMetaEnum::fromType<SqliteDataType>();
    createStrList << QString("Id INTEGER PRIMARY KEY   AUTOINCREMENT");
    for(auto keyTypeIter = keyType.begin(); keyTypeIter != keyType.end(); keyTypeIter++)
    {
        createStrList << QString("%1 %2 NOT NULL").arg(keyTypeIter.key()).arg(QString(enumState.valueToKey(keyTypeIter.value())));
    }
    QString cmdStr = createStrList.join(" , ");
    cmdStr.prepend(QString("CREATE TABLE  IF NOT EXISTS %1 (").arg(tableName));
    cmdStr.append(")");
    bool state = query.exec(cmdStr);
    if(state == false)
    {
       qDebug() << "error :"<<query.lastError();
    }
    return state;
}

bool DataBaseInterface::existTable(const QString &tableName)
{
    QMutexLocker mutexLocker(&m_mutex);
    QList<QString> tableNames = m_db.tables();
    foreach(const QString &tempTable, tableNames)
    {
        if(tempTable == tableName)
        {
            return true;
        }
    }
    return false;
}

bool DataBaseInterface::existDataBase()
{
    if(m_fileFolderPath+m_dataBaseName == m_db.databaseName())
    {
        return true;
    }
    return false;
}

bool DataBaseInterface::writeRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant> > &value)
{
    QMutexLocker mutexLocker(&m_mutex);
    bool openTransactionState = false;
    bool state = true;
    if (value.size() > s_lineNumberDividingLine)
    {
        openTransactionState = m_db.transaction();
        if(openTransactionState == false)
        {
            qDebug() << "transaction state "<< state << " "<<m_db.lastError();
        }
    }
    QStringList placeholderKey(key);
    QSqlQuery query(m_db);
    placeholderKey.replaceInStrings(QRegExp("^(.*)$"),":\\1");
    state = query.prepare(QString("INSERT OR IGNORE INTO  %1 (%2) VALUES (%3)").arg(tableName).arg(key.join(" , ")).arg(placeholderKey.join(", ")));
    if(state == false)
    {
        qDebug()<<"DataBaseInterface::writeRow prepare fail";
        return state;
    }
    QList<QList<QVariant>> insertValue;
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        insertValue << QList<QVariant>();
    }
    for(qlonglong indexNum = 0;indexNum < value.size();indexNum++)
    {
        for(qlonglong indexKey = 0;indexKey < value.at(indexNum).size();indexKey++)
        {
            insertValue[indexKey].append(value.at(indexNum).at(indexKey));
        }
    }
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        query.bindValue(placeholderKey.at(indexKey),QVariantList(insertValue.at(indexKey)));
    }
    state = query.execBatch();
    if(state == false)
    {
        qDebug()<< "writeRow:"<<query.lastError();
    }
    if (openTransactionState == true && state == true) 
    {
        state = m_db.commit();
        if(state == false)
        {
            qDebug() << "commit:"<<m_db.lastError();
        }
    }
    return state;
}

bool DataBaseInterface::writeRow(const QString &tableName, const QMap<QString, QVariant> &keyValue)
{
    QMutexLocker mutexLocker(&m_mutex);
    QList<QString> key;
    QList<QVariant> value;
    QList<QList<QVariant>> valueList;
    for(auto iter=keyValue.begin(); iter != keyValue.end(); iter++)
    {
        key.append(iter.key());
        value.append(iter.value());
    }
    valueList.append(value);
    return writeRow(tableName, key, valueList);
}


bool DataBaseInterface::deleteRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant>> &value)
{
    QMutexLocker mutexLocker(&m_mutex);
    bool openTransactionState = false;
    bool state = true;
    if (value.size() > s_lineNumberDividingLine)
    {
        openTransactionState = m_db.transaction();
        if(openTransactionState == false)
        {
            qDebug() << "transaction state "<< state << " "<<m_db.lastError();
        }
    }
    QStringList placeholderKey(key);
    QStringList placeholderCmdKey(key);
    QSqlQuery query(m_db);
    placeholderKey.replaceInStrings(QRegExp("^(.*)$"),":\\1");
    placeholderCmdKey.replaceInStrings(QRegExp("^(.*)$"),"\\1 = :\\1");
    state = query.prepare(QString("DELETE FROM %1 WHERE (%2)").arg(tableName).arg(placeholderCmdKey.join(" AND ")));
    if(state == false)
    {
        return state;
    }
    QList<QList<QVariant>> insertValue;
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        insertValue << QList<QVariant>();
    }
    for(qlonglong indexNum = 0;indexNum < value.size();indexNum++)
    {
        for(qlonglong indexKey = 0;indexKey < value.at(indexNum).size();indexKey++)
        {
            insertValue[indexKey].append(value.at(indexNum).at(indexKey));
        }
    }
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        query.bindValue(placeholderKey.at(indexKey),QVariantList(insertValue.at(indexKey)));
    }
    state = query.execBatch();
    if(state == false)
    {
        qDebug()<< "delete Row:"<<query.lastError();
    }
    if (openTransactionState == true && state == true) 
    {
        state = m_db.commit();
        if(state == false)
        {
            qDebug() << "commit:"<<m_db.lastError();
        }
    }
    return state;
}


QList<QList<QVariant>> DataBaseInterface::readRow(const QString &tableName, const QList<QString> &key, const int &maxRowNum)
{
    QMutexLocker mutexLocker(&m_mutex);
    QList<QList<QVariant>> retData;
    QSqlQuery query(m_db);
    query.exec(QString("SELECT %1 FROM %2 LIMIT %3").arg(key.join(" , ")).arg(tableName).arg(maxRowNum));
    while(query.next())
    {
        QList<QVariant> tempData;
        for(int32_t index = 0;index < query.record().count();index++)
        {
            tempData.append(query.value(key.at(index)));
        }
        retData.append(tempData);
    }
    return retData;
}


bool DataBaseInterface::updateRow(const QString &tableName, const QList<QString> &constraintkey, const QList<QList<QVariant>> &constraintValue, const QList<QString> &key, const QList<QList<QVariant>> &value)
{
    QMutexLocker mutexLocker(&m_mutex);
    bool openTransactionState = false;
    bool state = true;
    if (value.size() > s_lineNumberDividingLine)
    {
        openTransactionState = m_db.transaction();
        if(openTransactionState == false)
        {
            qDebug() << "transaction state "<< state << " "<<m_db.lastError();
        }
    }
    QStringList constraintPlaceholderKey(constraintkey);
    QStringList constraintPlaceholderCmdKey(constraintkey);
    QStringList placeholderKey(key);
    QStringList placeholderCmdKey(key);
    QSqlQuery query(m_db);
    constraintPlaceholderKey.replaceInStrings(QRegExp("^(.*)$"),":\\1");
    constraintPlaceholderCmdKey.replaceInStrings(QRegExp("^(.*)$"),"\\1 = :\\1");
    placeholderKey.replaceInStrings(QRegExp("^(.*)$"),":\\1");
    placeholderCmdKey.replaceInStrings(QRegExp("^(.*)$"),"\\1 = :\\1");
    state = query.prepare(QString("UPDATE %1 SET %2 WHERE %3").arg(tableName).arg(placeholderCmdKey.join(" , ")).arg(constraintPlaceholderCmdKey.join(" AND ")));
    if(state == false)
    {
        qDebug()<<"DataBaseInterface::writeRow prepare fail";
        return state;
    }
    QList<QList<QVariant>> constraintInsertValue;
    for(qlonglong indexKey = 0; indexKey < constraintkey.size();indexKey++)
    {
        constraintInsertValue << QList<QVariant>();
    }
    for(qlonglong indexNum = 0;indexNum < constraintValue.size();indexNum++)
    {
        for(qlonglong indexKey = 0;indexKey < constraintValue.at(indexNum).size();indexKey++)
        {
            constraintInsertValue[indexKey].append(constraintValue.at(indexNum).at(indexKey));
        }
    }
    for(qlonglong indexKey = 0; indexKey < constraintkey.size();indexKey++)
    {
        query.bindValue(constraintPlaceholderKey.at(indexKey),QVariantList(constraintInsertValue.at(indexKey)));
    }
    QList<QList<QVariant>> insertValue;
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        insertValue << QList<QVariant>();
    }
    for(qlonglong indexNum = 0;indexNum < value.size();indexNum++)
    {
        for(qlonglong indexKey = 0;indexKey < value.at(indexNum).size();indexKey++)
        {
            insertValue[indexKey].append(value.at(indexNum).at(indexKey));
        }
    }
    for(qlonglong indexKey = 0; indexKey < key.size(); indexKey++)
    {
        query.bindValue(placeholderKey.at(indexKey),QVariantList(insertValue.at(indexKey)));
    }
    state = query.execBatch();
    if(state == false)
    {
        qDebug()<< "update Row:"<<query.lastError();
    }
    if (openTransactionState == true && state == true) 
    {
        state = m_db.commit();
        if(state == false)
        {
            qDebug() << "commit:"<<m_db.lastError();
        }
    }
    return state;
}

bool DataBaseInterface::replaceRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant>> &value)
{
    QMutexLocker mutexLocker(&m_mutex);
    // bool state = false;
    // state = updateRow(tableName, constraintkey, constraintValue, key, value);
    // if(state == false)
    // {
    //     qDebug()<<"update Failed";
    //     state = writeRow(tableName,  key, value);
    //     return state;
    // }
    // else
    // {
    //     return state;
    // }
    bool openTransactionState = false;
    bool state = true;
    if (value.size() > s_lineNumberDividingLine)
    {
        openTransactionState = m_db.transaction();
        if(openTransactionState == false)
        {
            qDebug() << "transaction state "<< state << " "<<m_db.lastError();
        }
    }
    QStringList placeholderKey(key);
    QSqlQuery query(m_db);
    placeholderKey.replaceInStrings(QRegExp("^(.*)$"),":\\1");
    state = query.prepare(QString("REPLACE INTO %1 (%2) VALUES (%3)").arg(tableName).arg(key.join(" , ")).arg(placeholderKey.join(", ")));
    if(state == false)
    {
        return state;
    }
    QList<QList<QVariant>> insertValue;
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        insertValue << QList<QVariant>();
    }
    for(qlonglong indexNum = 0;indexNum < value.size();indexNum++)
    {
        for(qlonglong indexKey = 0;indexKey < value.at(indexNum).size();indexKey++)
        {
            insertValue[indexKey].append(value.at(indexNum).at(indexKey));
        }
    }
    for(qlonglong indexKey = 0; indexKey < key.size();indexKey++)
    {
        query.bindValue(placeholderKey.at(indexKey),QVariantList(insertValue.at(indexKey)));
    }
    state = query.execBatch();
    if(state == false)
    {
        qDebug()<< "replace Row:"<<query.lastError();
    }
    if (openTransactionState == true && state == true) 
    {
        state = m_db.commit();
        if(state == false)
        {
            qDebug() << "commit:"<<m_db.lastError();
        }
    }
    return state;
}

bool DataBaseInterface::deleteMaxRow(const QString &tableName, const QString &key, const qlonglong &maxRow)
{
    QMutexLocker mutexLocker(&m_mutex);
    QSqlQuery query(m_db);
    QString cmd=QString("delete from %1 where(select count(%2) from %1) > %3 and %2 in(select %2 from %1 order by %2 desc limit(select count(%2) from %1) offset %3)").arg(tableName).arg(key).arg(maxRow);
    return query.exec(cmd);
}

qlonglong DataBaseInterface::existRowNum(const QString &tableName, const QString &key, const QString &value)
{
    QMutexLocker mutexLocker(&m_mutex);
    bool state = false;
    qlonglong retRows = 0;
    QSqlQuery query(m_db);
    state = query.exec(QString("SELECT count(*) FROM %1 WHERE %2 = '%3' ").arg(tableName).arg(key).arg(value));
    if(state == true)
    {
        while (query.next()) {
            retRows = query.value("count(*)").toLongLong();
        }
    }
    qDebug()<<"retRows="<<retRows;
    return retRows;
}

int DataBaseInterface::RowNum(const QString &tableName)
{
    QMutexLocker mutexLocker(&m_mutex);
    bool state = false;
    int retRows = 0;
    QSqlQuery query(m_db);
    state = query.exec(QString("SELECT count(*) FROM %1 ").arg(tableName));
    if(state == true)
    {
        while (query.next()) {
            retRows = query.value("count(*)").toInt();
        }
    }
    qDebug()<<"rowNum="<<retRows;
    return retRows;
}


void DataBaseInterface::checkDir()
{
    QMutexLocker mutexLocker(&m_mutex);
    QDir tempDir(m_fileFolderPath);
    if(!tempDir.exists())
    {
        tempDir.mkpath(m_fileFolderPath);
    }
}


QList<QList<QVariant>> DataBaseInterface::readRowWhere(const QString &tableName, const QList<QString> condition, const QList<QString> &key, const int &maxRowNum)
{
    QMutexLocker mutexLocker(&m_mutex);
    QList<QList<QVariant>> retData;
    QSqlQuery query(m_db);
    qDebug()<<"DataBaseInterface::readRowWhere sql=" << QString("SELECT %1 FROM %2 WHERE %3 LIMIT %4").arg(key.join(" , ")).arg(tableName).arg(condition.join(" AND ")).arg(maxRowNum);
    query.exec(QString("SELECT %1 FROM %2 WHERE %3 LIMIT %4").arg(key.join(" , ")).arg(tableName).arg(condition.join(" AND ")).arg(maxRowNum));
    while(query.next())
    {
        QList<QVariant> tempData;
        for(int32_t index = 0;index < query.record().count();index++)
        {
            tempData.append(query.value(key.at(index)));
        }
        retData.append(tempData);
    }
    return retData;
}

int DataBaseInterface::userVersion()
{
    QMutexLocker mutexLocker(&m_mutex);
    QSqlQuery query(m_db);
    if (query.exec("PRAGMA user_version") && query.next()) {
        return query.value(0).toInt();
    }
    qWarning() << "DataBaseInterface::userVersion failed to read user_version:" << query.lastError();
    return 0;
}

bool DataBaseInterface::setUserVersion(int version)
{
    QMutexLocker mutexLocker(&m_mutex);
    QSqlQuery query(m_db);
    const bool ok = query.exec(QString("PRAGMA user_version = %1").arg(version));
    if (!ok) {
        qWarning() << "DataBaseInterface::setUserVersion failed to set user_version to" << version << ":" << query.lastError();
    }
    return ok;
}
