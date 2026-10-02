#include "filedatabase.h"

#include <QDebug>
#include <QDir>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

const QString DATABASE_NAME = QString("pcsearchInfo.db");
const QString DATABASE_PATH = getenv("HOME") + QString("/.connectivitycache/");
const QString TABLE_NAME = QString("search");

// 数据项
enum HEADER_NAME {
    ID = 0,  // id
    NAME,    // 文件名
    PATH,    // 文件路径
    SIZE,    // 文件大小
    DATE     // 最后修改时间
};

FileDataBase::FileDataBase(QObject *parent) : QObject(parent)
{
    QMutexLocker lockData(&m_mutex);
    QDir downDir(DATABASE_PATH);
    if (!downDir.exists(DATABASE_PATH)) {
        // 文件夹不存在，创建文件夹
        downDir.mkpath(DATABASE_PATH);
    }
    if (QSqlDatabase::contains(DATABASE_NAME)) {
        // 本地已存在该数据库
        m_DataBase = QSqlDatabase::database(DATABASE_NAME);
    } else {
        // 不存在创建
        m_DataBase = QSqlDatabase::addDatabase("QSQLITE", DATABASE_NAME);
        m_DataBase.setDatabaseName(DATABASE_PATH + DATABASE_NAME);
        initTable();
    }
}

FileDataBase::FileDataBase(QString name, QObject *parent) : QObject(parent)
{
    QMutexLocker lockData(&m_mutex);
    if (QSqlDatabase::contains(name)) {
        m_DataBase = QSqlDatabase::database(name);
    } else {
        m_DataBase = QSqlDatabase::addDatabase("QSQLITE", DATABASE_NAME);
        m_DataBase.setDatabaseName(DATABASE_PATH + name + ".db");
        initTable();
    }
}

FileDataBase::~FileDataBase()
{
    if (m_DataBase.open()) {
        // 关闭数据库
        m_DataBase.close();
    }
}

int FileDataBase::insertData(const QList<FileInfo> &fileinfoList)
{
    QMutexLocker lockData(&m_mutex);
    if (!m_DataBase.open()) {
        qCritical() << "Failed to insert table: data opening error.";
        return -1;
    }

    int count = 0;
    for (FileInfo fileinfo : fileinfoList) {
        if (m_isStop) {
            return count;
        }
        if (insertData(fileinfo)) {
            // 成功更新数据
            count++;
        }
    }

    return count;
}

int FileDataBase::removeData(const QStringList &filePathList)
{
    QMutexLocker lockData(&m_mutex);
    if (!m_DataBase.open()) {
        qCritical() << "Failed to remove data: data opening error.";
        return -1;
    }
    QStringList dataBaseList = getDataPathList();
    QSqlQuery query(m_DataBase);
    int count = 0;
    for (QString path : dataBaseList) {
        if (m_isStop) {
            return count;
        }
        if (!filePathList.contains(path)) {
            // 数据库中的数据本地不能存在，删除该数据
            QString sqlStr = QString("delete from %1 where path = '%2'").arg(TABLE_NAME).arg(path);
            if (!query.prepare(sqlStr)) {
                qWarning() << "Unable to execute statement: " << sqlStr;
                continue;
            }
            if (query.exec()) {
                count++;
            } else {
                QSqlError lastError = query.lastError();
                QString err = lastError.driverText();
                qWarning() << "Failed to delete data: " + err;
            }
        }
    }

    return count;
}

void FileDataBase::stopRun()
{
    m_isStop = true;
}

bool FileDataBase::initTable()
{
    if (!m_DataBase.open()) {
        qCritical() << "data opening error.";
        return false;
    }

    if (!isExistTable()) {
        return createTable();
    }

    return false;
}

bool FileDataBase::isExistTable()
{
    bool bRet = false;
    if (!m_DataBase.open()) {
        qCritical() << "Failed to read the table: data opening error.";
        return bRet;
    }

    QSqlQuery query(m_DataBase);
    QString sqlStr = QString("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(TABLE_NAME);
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        return bRet;
    }
    if (query.exec()) {
        if (query.next()) {
            if (query.value(0).toInt() > 0) {
                bRet = true;
            }
        }
    } else {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qWarning() << "Failed to select table: " + err;
        bRet = false;
    }

    return bRet;
}

bool FileDataBase::createTable()
{
    if (!m_DataBase.open()) {
        qCritical() << "Failed to create table: data opening error.";
        return false;
    }

    QSqlQuery query(m_DataBase);
    QString sqlStr = QString("CREATE TABLE %1 ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "name VARCHAR(40) NOT NULL, "
                             "path VARCHAR(127) NOT NULL, "
                             "size INTEGER NOT NULL, "
                             "date VARCHAR(31) NOT NULL)")
                         .arg(TABLE_NAME);
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        return false;
    }
    if (query.exec()) {
        qInfo() << "Table creation succeeded.";
        return true;  // 新建数据库成功
    } else {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qInfo() << "Failed to create table: " + err;
        return false;
    }
}

bool FileDataBase::insertData(const FileInfo &fileinfo)
{
    bool bRet = false;
    if (!m_DataBase.open()) {
        qCritical() << "Failed to insert table: data opening error.";
        return bRet;
    }

    DataStatus status = getDataStatus(fileinfo);
    switch (status) {
        case DataStatus::NEEDINSTER: {
            if (insert(fileinfo)) {
                bRet = true;
            }
        } break;
        case DataStatus::NEEDUPDATE: {
            if (update(fileinfo)) {
                bRet = true;
            }
        } break;
        case DataStatus::NOUPDATE:
            bRet = false;
            break;
    }

    return bRet;
}

FileDataBase::DataStatus FileDataBase::getDataStatus(const FileInfo &fileinfo)
{
    DataStatus status = DataStatus::NOUPDATE;
    if (!m_DataBase.open()) {
        qCritical() << "data opening error.";
        return status;
    }

    status = DataStatus::NEEDINSTER;
    QSqlQuery query(m_DataBase);
    QString name = fileinfo.name();
    QString sqlStr = QString("SELECT * FROM %1 WHERE name='%2'").arg(TABLE_NAME).arg(name);
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        status = DataStatus::NOUPDATE;
        return status;
    }
    if (query.exec()) {
        while (query.next()) {
            if (m_isStop) {
                return DataStatus::NOUPDATE;
            }
            if (fileinfo.getPath() == query.value(HEADER_NAME::PATH).toString()) {
                if (fileinfo.size() == query.value(HEADER_NAME::SIZE).toInt() &&
                    fileinfo.dateTime() == query.value(HEADER_NAME::DATE).toString()) {
                    // 数据都相同，不需要更新
                    status = DataStatus::NOUPDATE;
                    return status;
                }
                // 数据和路径相同，但是其他信息不同，需要更新
                status = DataStatus::NEEDUPDATE;
            } else {
                // 数据库中不存在该条数据，需要插入
                status = DataStatus::NEEDINSTER;
            }
        }
    } else {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qInfo() << "Failed to get data status: " + err;
        status = DataStatus::NOUPDATE;
        return status;
    }

    return status;
}

QStringList FileDataBase::getDataPathList()
{
    if (!m_DataBase.open()) {
        qCritical() << "data opening error.";
        return QStringList();
    }
    QStringList filePathList;
    QSqlQuery query(m_DataBase);
    QString sqlStr = QString("SELECT * FROM %1").arg(TABLE_NAME);
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        return QStringList();
    }
    if (!query.exec()) {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qInfo() << "Failed to get data: " + err;
        return QStringList();
    }

    while (query.next()) {
        if (m_isStop) {
            return filePathList;
        }
        filePathList.append(query.value(HEADER_NAME::PATH).toString());
    }
    return filePathList;
}

bool FileDataBase::insert(const FileInfo &fileinfo)
{
    if (!m_DataBase.open()) {
        qCritical() << "data opening error.";
        return false;
    }
    QSqlQuery query(m_DataBase);
    QString sqlStr = QString("INSERT INTO %1 (name, path, size, date) "
                             "VALUES ('%2', '%3', %4, '%5')")
                         .arg(TABLE_NAME)
                         .arg(fileinfo.name())
                         .arg(fileinfo.getPath())
                         .arg(QString::number(fileinfo.size()))
                         .arg(fileinfo.dateTime());
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        return false;
    }
    if (!query.exec()) {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qInfo() << "Failed to inster data: " + err;
        return false;
    }

    return true;
}

bool FileDataBase::update(const FileInfo &fileinfo)
{
    if (!m_DataBase.open()) {
        qCritical() << "data opening error.";
        return false;
    }
    QSqlQuery query(m_DataBase);
    QString sqlStr = QString("UPDATE %1 SET size = %2, date = '%3' WHERE name = '%4' AND path = '%5'")
                         .arg(TABLE_NAME)
                         .arg(QString::number(fileinfo.size()))
                         .arg(fileinfo.dateTime())
                         .arg(fileinfo.name())
                         .arg(fileinfo.getPath());
    if (!query.prepare(sqlStr)) {
        qWarning() << "Unable to execute statement: " << sqlStr;
        return false;
    }
    if (!query.exec()) {
        QSqlError lastError = query.lastError();
        QString err = lastError.driverText();
        qInfo() << "Failed to inster data: " + err;
        return false;
    }

    return true;
}