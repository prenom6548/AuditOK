//
// Created by sqp on 2022/4/14.
//

#include "searchdatabase.h"

#include <iostream>
#include <QSqlQuery>
#include <QMessageBox>
#include <QApplication>
#include <QSqlError>
#include <QFileInfo>
#include <QDebug>
#include <QSqlRecord>
#include <QTime>
#include <sys/time.h>
const QString VIRTUAL_TABLE_NAME = "virtualSearchInfo";
const QString DATEBASE_NAME = "searchbase";

SearchDataBase::SearchDataBase(QObject *parent) : QObject(parent)
{
    qInfo() << QSqlDatabase::drivers();  // 当前环境支持哪些数据库
    QMutexLocker lockData(&m_mutex);     // 加锁，函数执行完后自动解锁
    m_database = QSqlDatabase::addDatabase("QSQLITE", DATEBASE_NAME);
}

SearchDataBase::~SearchDataBase()
{
    if (true == m_databaseOpenFlag) {
        sqlite3_shutdown();
        m_database.close();
    }
}

SearchDataBase *SearchDataBase::getInstance()
{
    static SearchDataBase instance;
    return &instance;
}

int SearchDataBase::initDataBase(const QString &dbPath)
{
    qInfo() << "init database";
    if (!dbPath.isEmpty()) {
        m_database.setDatabaseName(dbPath);
    } else {
        qWarning() << "can not find searchInfo.db";
        return DB_UNCONNECT;
    }

    if (!m_database.open()) {
        m_databaseOpenFlag = true;

        QMessageBox::warning(0, QObject::tr("Database Error"), m_database.lastError().text());
        return DB_UNCONNECT;
    }

    QVariant v = m_database.driver()->handle();  // 获得低级handle包
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0) {
        sqlite3_initialize();
        // 显式地初始化一下。
        sqlite3 *m_handle = *static_cast<sqlite3 **>(v.data());
        if (m_handle) {
            sqlite3_enable_load_extension(m_handle, 1);                               // 允许加载扩展
            QSqlQuery loadExtension(m_database);
            bool loadRes = loadExtension.exec("SELECT load_extension('libsimple')");  // 使用sql函数加载libsimple
            if (!loadRes) {
                qWarning() << "load extension failed:" << loadExtension.lastError().text();
                return -1;
            } else {
                qInfo() << "load extension success!";
                return DB_OP_SUCC;
            }
        }
    }
}

int SearchDataBase::creatVirtualDBTable()
{
    bool queryRes = true;
    QSqlQuery creatVTable(m_database);
    queryRes &= creatVTable.exec(
        QString("create virtual table if not exists %1"
                " using fts5(id UNINDEXED, name, path, size, date, tokenize='simple', prefix='1 2 3 4')")
            .arg(VIRTUAL_TABLE_NAME));

    queryRes &= creatVTable.exec("insert into virtualSearchInfo select * from search");

    if (queryRes) {
        qInfo() << "creat virtual table success！";
        return DB_OP_SUCC;
    } else {
        qInfo() << "creat virtual table error：" << creatVTable.lastError().text();
        return CREATE_TABLE_FAILED;
    }
}

int SearchDataBase::searchFromDB(QMap<QString, FileInfo> &resultMap, const QString &keyword)
{
    if (keyword.isEmpty()) {
        // 空keyword不处理
        return INVALID_INPUT;
    }
    if (m_database.isValid()) {
        bool getRes = true;

        QSqlQuery searchInit(m_database);
        // 直接在虚表中存取，不再根据id回表。
        QString searchSql = QString("select * from virtualSearchInfo"
                                    " where name match simple_query('%1') order by rank")
                                .arg(keyword);

        getRes = searchInit.exec(searchSql);

        if (getRes) {
            while (searchInit.next()) {
                FileInfo temp;
                temp.setName(searchInit.value(1).toString());
                temp.setPath(searchInit.value(2).toString());
                temp.setSize(searchInit.value(3).toInt());
                temp.setDateTime(searchInit.value(4).toString());
                temp.setType(FileInfo::judgmentType(temp.name()));
                resultMap.insert(temp.name(), temp);
            }
            qInfo() << "query success";
            return DB_OP_SUCC;
        } else {
            qWarning() << "search sql error：" << searchInit.lastError().text();
            return DB_OP_GET_FAILED;
        }
    } else {
        return DB_DISORDERD;
    }
}
