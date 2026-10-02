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
#ifndef DATABASEINTERFACE_H
#define DATABASEINTERFACE_H

#include <QObject>
#include <QList>
#include <QCoreApplication>
#include <QString>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QMap>
#include <QMetaEnum>
#include <QString>
#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlRecord>
#include <QDir>
#include "KylinDaqCommon.h"
#include <QMutexLocker>
#include <QMutex>

class DataBaseInterface:public QObject
{
    Q_OBJECT
public:
    enum SqliteDataType
    {
        INTEGER,
        REAL,
        TEXT,
        BLOB 
    };
    Q_ENUM(SqliteDataType)
    /**
     * @brief 使用的数据库驱动名 
     */
    static constexpr const char* s_sqliteName = "QSQLITE";
    static const int32_t s_lineNumberDividingLine = 100;
private:
    /**
     * @brief 数据库文件夹路径
     * 
     */
    QString m_fileFolderPath;
    /**
     * @brief 数据库名
     * 
     */
    QString m_dataBaseName;
    /**
     * @brief 连接名
     * 
     */
    QString m_connectName;
    /**
     * @brief 数据库对象
     * 
     */
    QSqlDatabase  m_db;
    /**
     * @brief 单例
     * 
     */
    static QScopedPointer<DataBaseInterface> ms_instance;
    QMutex m_mutex{QMutex::Recursive};
public:
    /**
     * @brief 单例函数
     * 
     * @return DataBaseInterface* 
     */
    static DataBaseInterface *instance(void);
    /**
     * @brief 创建数据表
     * 
     * @param createString 创建命令
     * @return true 创建成功
     * @return false 创建失败
     */
    virtual bool createTable(const QString &createString);
    /**
     * @brief 创建数据表
     * 
     * @param tableName 表名
     * @param keyType 表中字段
     * @return true 创建成功
     * @return false 创建失败
     */
    virtual bool createTable(const QString &tableName, const QMap<QString,SqliteDataType> &keyType);
    /**
     * @brief 检测表是否存在
     * 
     * @param tableName 表名
     * @return true 表存在
     * @return false 表不存在
     */
    virtual bool existTable(const QString &tableName);
    /**
     * @brief 检测数据库是否存在
     * 
     * @return true 数据库存在
     * @return false 数据库不存在
     */
    virtual bool existDataBase();
    /**
     * @brief 向表中写入数据
     * 
     * @param tableName 表名
     * @param key 键
     * @param value 值
     * @return true 写入成功
     * @return false 写入失败
     */
    virtual bool writeRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant>> &value);
    /**
     * @brief 向表中写入数据
     * 
     * @param tableName 表名
     * @param keyValue 键值对
     * @return true 写入成功
     * @return false 写入失败
     */
    virtual bool writeRow(const QString &tableName, const QMap<QString,QVariant> &keyValue);
    /**
     * @brief 从表中读取数据
     * 
     * @param tableName 表名
     * @param key 键
     * @param maxRowNum 最大读取行数
     * @return QList<QList<QVariant>> 读取结果
     */
    virtual QList<QList<QVariant>> readRow(const QString &tableName ,const QList<QString> &key,const int &maxRowNum);
    /**
     * @brief 从表中删除数据
     * 
     * @param tableName 表名
     * @param key 键
     * @param value 值
     * @return true 删除成功
     * @return false 删除失败
     */
    virtual bool deleteRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant>> &value);
    /**
     * @brief 更新表中数据
     * 
     * @param tableName 表名
     * @param constraintkey 更新的约束键
     * @param constraintValue 更新的约束值
     * @param key 更新键
     * @param value 更新值
     * @return true 更新成功
     * @return false 更新失败
     */
    virtual bool updateRow(const QString &tableName, const QList<QString> &constraintkey, const QList<QList<QVariant>> &constraintValue, const QList<QString> &key, const QList<QList<QVariant>> &value);
    /**
     * @brief 替换表中数据
     * 
     * @param tableName 表名
     * @param key 键
     * @param value 值
     * @return true 替换成功
     * @return false 替换失败
     */
    virtual bool replaceRow(const QString &tableName, const QList<QString> &key, const QList<QList<QVariant>> &value);
    /**
     * @brief 删除表中数据
     * 
     * @param tableName 表名
     * @param key 键
     * @param maxRow 最大保留行数
     * @return true 删除成功
     * @return false 删除失败
     */
    virtual bool deleteMaxRow(const QString &tableName, const QString &key, const qlonglong &maxRow);
    /**
     * @brief 检测表中存在行数
     * 
     * @param tableName 表名
     * @param key 键
     * @param value 值
     * @return qlonglong 存在的行数
     */
    virtual qlonglong existRowNum(const QString &tableName, const QString &key, const QString &value);


    /**
     * @brief 检测表中存在行数
     *
     * @param tableName 表名
     * @param key 键
     * @param value 值
     * @return qlonglong 存在的行数
     */
    virtual int RowNum(const QString &tableName);

    QList<QList<QVariant>> readRowWhere(const QString &tableName, const QList<QString> condition, const QList<QString> &key, const int &maxRowNum);
    /**
     * @brief 读取 SQLite PRAGMA user_version
     *
     * @return int 当前 user_version 值，读取失败返回 0
     */
    int userVersion();
    /**
     * @brief 设置 SQLite PRAGMA user_version
     *
     * @param version 版本号
     * @return true 成功
     * @return false 失败
     */
    bool setUserVersion(int version);
    ~DataBaseInterface();
    DataBaseInterface(const QString &fileFolderPath, const QString &dataBaseName,const QString &connectName,QObject *parent=nullptr);
private:
    void checkDir();
};
#endif
