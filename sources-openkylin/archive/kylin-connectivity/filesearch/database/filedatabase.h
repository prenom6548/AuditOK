#ifndef FILEDATABASE_H
#define FILEDATABASE_H

#include <QObject>
#include <QString>
#include <QtSql/QSqlDatabase>
#include <QList>
#include <QMutexLocker>

#include "fileinfo.h"

class FileDataBase : public QObject
{
public:
    FileDataBase(QObject *parent = nullptr);
    FileDataBase(QString name, QObject *parent);
    ~FileDataBase();

    // 插入多条数据
    int insertData(const QList<FileInfo> &fileinfoList);
    // 删除数据库中filePathList没有的数据， 返回删除条数
    int removeData(const QStringList &filePathList);

    void stopRun();

private:
    enum DataStatus {
        NEEDUPDATE = 0,  // 该行数据需要更新
        NEEDINSTER = 1,  // 需要插入该数据
        NOUPDATE = 2,    // 不需要更新
    };
    Q_ENUM(DataStatus)

private:
    // 初始化表
    bool initTable();
    // 查询表是否存在
    bool isExistTable();
    // 创建表
    bool createTable();
    // 插入单条数据
    bool insertData(const FileInfo &fileinfo);
    // 获取要插入的数据在数据库中的状态
    FileDataBase::DataStatus getDataStatus(const FileInfo &fileinfo);
    // 获取数据库中所有文件路径
    QStringList getDataPathList();
    // 插入数据
    bool insert(const FileInfo &fileinfo);
    bool update(const FileInfo &fileinfo);

private:
    QSqlDatabase m_DataBase;  // 数据库对象
    QMutex m_mutex;
    bool m_isStop = false;
};

#endif  // FILEDATABASE_H