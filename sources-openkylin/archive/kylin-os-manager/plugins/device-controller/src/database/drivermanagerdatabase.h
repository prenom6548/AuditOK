#ifndef DRIVERMANAGERDATABASE_H
#define DRIVERMANAGERDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "../hardwareinfo/hardwareinfo.h"

enum DB_RETURN_STATUS {          //数据库操作结果返回表
    DB_OP_SUCC = (0),            //数据库操作成功
    DB_UNCONNECT = (-1),         //数据库连接失败导致的错误
    INVALID_INPUT = (-2),        //无效的输入值
    DB_DISORDERD = (-3),         //数据库与外界顺序不符导致的失败
    DB_EMPTY_TABLE = (-4),       //数据库中的表格内容为空
    DB_OP_ADD_FAILED = (-5),     //数据库添加操作失败
    DB_OP_DEL_FAILED = (-6),     //数据库删除操作失败
    DB_OP_GET_FAILED = (-7),     //数据库查询操作失败
    DB_OP_ADD_REPEAT = (-8),     //数据库添加失败，重复添加
    DB_INSIDE_FAILED = (-9),     //数据库内部错误
    FLAG_UNFOUND = (-10),        //数据库中标记未找到
    CREATE_TABLE_FAILED = (-11), //数据库创表失败
    DEL_TABLE_FAILED = (-12),    //数据库删表失败
    OUT_OF_RESULT,               //结果边界
};

#define DATABASE_INSTANCE (DriverManagerDatabase::getInstance())

class DriverManagerDatabase : public QObject {
Q_OBJECT
public:
    DriverManagerDatabase(QObject *parent = nullptr);

    ~DriverManagerDatabase();

    int initDatabase();

    QList<HardWareInfo> getDataList();

    QSqlDatabase m_dataBase;

    static DriverManagerDatabase &getInstance(){
        static DriverManagerDatabase m_instance;
        return m_instance;
    }

private:
    DB_RETURN_STATUS m_dbStatus;
    QList<HardWareInfo> m_searchDataList;

private slots:
    void insertDataSlot(QStringList data);

    void deleteDataSlot(QString devname);

    void deleteDataFromDriverSlot(QString drivername);
};


#endif //DRIVERMANAGERDATABASE_H
