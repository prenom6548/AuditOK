#ifndef RECORDERDATABASE_H
#define RECORDERDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextCodec>
#include <QSqlError>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

const QString ITEM_FIELD = "file_name, flag_time"; //数据库字段
const QString ITEM_FIELD2 = "file_name, create_time";

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

#define DATABASE_INSTANCE (RecorderDataBase::getInstance())

class RecorderDataBase : public QObject
{
    Q_OBJECT
public:
    explicit RecorderDataBase(QObject *parent = nullptr);
    ~RecorderDataBase();

public:
    QSqlDatabase m_dataBase;
    static RecorderDataBase &getInstance()
    {
        static RecorderDataBase m_pInstance;
        return m_pInstance;
    }

    int initDataBase();
    int searchData();
    int updateData();
    int getStatus();
    QStringList getDataList();
    void getAllFiles(QStringList &fileNames, QStringList &fileTimes);
public slots:

    int insertDataSlot(QFileInfo file_info, QString flagTime);
    int deleteDataSlot(QString fileBaseName);
    int searchDataSlot(QFileInfo file_info);
    int addNewFile(QString name,QString time);
    int deleteFile(QString name);
    int updateNameSlot(QFileInfo file_info, QString new_name);
    int updateDatabaseSlot(QString filePath, QString suffix);
private:
    int m_id;
    QString m_fileName;
    QString m_flagTime;

    DB_RETURN_STATUS m_dbStatus;
    QStringList searchDataList;

signals:
    void insertDataSignal(QFileInfo file_info, QString currentTimeStr);
    void deleteDataSignal(QString fileBaseName);
    void searchDataSignal(QFileInfo file_info);
    void addRecordSignal(QString name,QString time);
    void deleteRecordSignal(QString name);
    void updateNameSignal(QFileInfo file_info, QString new_name);
    void updateDatabaseSignal(QString filePath, QString suffix);
};

#endif // RECORDERDATABASE_H
