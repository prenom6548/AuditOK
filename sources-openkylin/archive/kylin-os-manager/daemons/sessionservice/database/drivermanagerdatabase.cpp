#include "drivermanagerdatabase.h"
#include <QDebug>
#include <QTextCodec>

const QString ITEM_FIELD = "devtype, devname, drivername, driverversion, driversize";

DriverManagerDatabase::DriverManagerDatabase(QObject *parent) : QObject(parent) {
    qDebug() << "支持的数据库类型：" << QSqlDatabase::drivers();
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    QString dirPath = QString(getenv("HOME")) + "/.config/.kylin_os_manager_driver_manager_Flag.db";
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    m_dataBase.setDatabaseName(dirPath);
}

DriverManagerDatabase::~DriverManagerDatabase() {

}

int DriverManagerDatabase::initDatabase() {
    if (!m_dataBase.open()) {
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    } else {
        QSqlQuery query;
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS Device  "
                                "(device_id     integer   PRIMARY KEY AUTOINCREMENT,  "
                                "devtype    char(50)  NOT NULL,  "
                                "devname    char(50)  NOT NULL,  "
                                "drivername    char(50)  NOT NULL,  "
                                "driverversion     char(50)  NOT NULL,  "
                                "driversize    char(50)  NOT NULL); ";
        query.prepare(creatTableStr);
        if (!query.exec()) {
            qDebug() << "表Device建表失败!";
            m_dbStatus = CREATE_TABLE_FAILED;
            return m_dbStatus;
        } else {
            qDebug() << "表Device建表成功!";
            m_dbStatus = DB_OP_SUCC;
            return m_dbStatus;
        }
    }
}

void DriverManagerDatabase::insertDataSlot(QStringList data) {
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        m_dbStatus = DB_UNCONNECT;
        return;
    } else {
        // 先看旧的有没有，再插入新的，如果有旧的，那就更新版本号，没有则插入新的一条
        int num = 0;
        QString searchCmd = QString("select count(devname) from Device where devname = '%1' ").arg(data[1]);
        QSqlQuery searchQuery;
        searchQuery.exec(searchCmd);
        if (!searchQuery.exec(searchCmd)) {
            qDebug() << "query error :" << searchQuery.lastError();
            m_dbStatus = DB_OP_GET_FAILED;
            return;
        } else {
            while (searchQuery.next()) {
                num = searchQuery.value(0).toInt();
            }
        }
        if (num > 0) {
            QString updateCmd = QString("update Device set drivername = '%1', driverversion = '%2' where devname = '%3'").arg(data[2]).arg(data[3]).arg(data[1]);
            searchQuery.exec(searchCmd);
            if (!searchQuery.exec(searchCmd)) {
                qDebug() << "query error :" << searchQuery.lastError();
                m_dbStatus = DB_OP_GET_FAILED;
                return;
            }
            qDebug() << "不可重复添加:" << DB_OP_ADD_REPEAT;
            m_dbStatus = DB_OP_ADD_REPEAT;
            return;
        } else {
            QString cmd = QString("insert into Device (%1) values('%2','%3','%4','%5','%6')").arg(ITEM_FIELD).arg(
                    data[0]).arg(data[1]).arg(data[2]).arg(data[3]).arg(data[4]);
            QSqlQuery query;
            if (!query.exec(cmd)) {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_ADD_FAILED;
                return;
            } else {
                qDebug() << "插入成功!";
                m_dbStatus = DB_OP_SUCC;
                return;
            }
        }
    }
}

