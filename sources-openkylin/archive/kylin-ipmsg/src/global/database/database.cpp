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

#include <QFile>
#include <QFileInfo>

#include "database.h"
#include "global/utils/inisetting.h"
#include <QDir>

DataBase::DataBase(QObject *parent) : QObject(parent)
{
    m_dbName = "kylin-ipmsg.db";

#ifdef NOTKAIMING
    QString dbFileSourPath = "/usr/share/kylin-ipmsg/data/database/" + m_dbName;
#else
    QString dbFileSourPath = "/opt/apps/top.openkylin.kylin-ipmsg/files/share/kylin-ipmsg/data/database/" + m_dbName;
#endif
    QString dbFileTargPath = QString(getenv("HOME")) + "/.config/ipmsg/" + m_dbName;

    QFile dbFileSour(dbFileSourPath);
    QFile dbFileTarg(dbFileTargPath);

    QString confDirPath = QString(getenv("HOME")) + "/.config/ipmsg/";
    QDir confDir = QDir(confDirPath);
    if (!confDir.exists()) {
        confDir.mkdir(confDirPath);
    }

    if (!(dbFileTarg.exists())) {
        if (dbFileSour.exists()) {
            dbFileSour.copy(dbFileTargPath);
        }
    }

    // 打开数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    if (dbFileTarg.exists()) {
        m_db.setDatabaseName(dbFileTargPath);
    } else {
        m_db.setDatabaseName(QApplication::applicationDirPath() + "/../../data/database/" + m_dbName + ".test");
    }

    // m_db.setDatabaseName(QApplication::applicationDirPath() + "/../../data/database/" + m_dbName);

    // qDebug() << m_db.databaseName();

    if (!m_db.open()) {
        // QMessageBox::warning(0, QObject::tr("Database Error"),
        //                              m_db.lastError().text());
        qDebug() << "db open error";
    }
    qDebug() << "db open success";
}

DataBase::~DataBase() {}

/**************************************************************************
 * 函数名称： getInstance
 * 函数功能： 单例，初始化返回指针，完成数据库文件建立
 * 输入参数： 无
 * 返回数值： DataBase *
 * 创建人员：
 * 创建时间： 2021-05-18
 * 修改人员：
 * 修改时间：
 **************************************************************************/
DataBase *DataBase::getInstance()
{
    static DataBase *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new DataBase();
            instance->setObjectName("kylin-ipmsg_DataBase_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new DataBase();
        instance->setObjectName("kylin-ipmsg_DataBase_instance");
#endif
    }

    return instance;
}

// 根据表名打开数据库
QSqlDatabase DataBase::openDatabse(QString tableName)
{
    Q_UNUSED(tableName);
    return m_db;
}
