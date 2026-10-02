//
// Created by sqp on 2022/4/14.
//

#ifndef KYLIN_CONNECTIVITY_SEARCHDATABASE_H
#define KYLIN_CONNECTIVITY_SEARCHDATABASE_H
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QObject>
#include <QFile>
#include <QList>
#include <QString>
#include <QThread>
#include <QMutexLocker>
#include <sqlite3.h>
#include "fileinfo.h"

enum DB_RETURN_STATUS {         //数据库操作结果返回表
    DB_OP_SUCC = (0),           //数据库操作成功
    DB_UNCONNECT = (-1),        //数据库连接失败导致的错误
    INVALID_INPUT = (-2),       //无效的输入值
    DB_DISORDERD = (-3),        //数据库与外界顺序不符导致的失败
    DB_OP_GET_FAILED = (-4),    //数据库查询操作失败
    CREATE_TABLE_FAILED = (-5), //数据库创表失败
};


class SearchDataBase : public QObject
{
    Q_OBJECT
public:
    ~SearchDataBase();
    //单例，初始化返回指针，完成数据库文件建立
    static SearchDataBase *getInstance();
    // open数据库，建立本地，历史，我喜欢及三个列表
    int initDataBase(const QString &dbPath);

    int creatDBTable(const QString &tablename);

    int creatVirtualDBTable();

    int searchFromDB(QMap<QString, FileInfo> &resultMap, const QString &keyword);

Q_SIGNALS:

protected:
    explicit SearchDataBase(QObject *parent = nullptr);

private:
    // 对包含'的字符串做预处理，避免获取SQL语句时语法出错
    QString preHandle(const QString &input);

    QString inPutStringHandle(const QString &input);

    int checkTableExist(const QString &tablename);

    QString outPutStringHandle(const QString &output);

private:
    QSqlDatabase m_database; //数据库
    QMutex m_mutex;
    bool m_databaseOpenFlag = false;
};
#endif // KYLIN_CONNECTIVITY_SEARCHDATABASE_H
