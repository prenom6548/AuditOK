#include "recorderdatabase.h"
#include "file-utils.h"

RecorderDataBase::RecorderDataBase(QObject *parent) : QObject(parent)
{
    qDebug() << QSqlDatabase::drivers(); //当前环境支持哪些数据库
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    QString dirPath = QString(getenv("HOME")) + "/.config/.kylin_recorder_Flag.db";
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale()); //设置显示中文
    m_dataBase.setDatabaseName(dirPath);
    connect(this, &RecorderDataBase::insertDataSignal, this, &RecorderDataBase::insertDataSlot);
    connect(this, &RecorderDataBase::deleteDataSignal, this, &RecorderDataBase::deleteDataSlot);
    connect(this, &RecorderDataBase::searchDataSignal, this, &RecorderDataBase::searchDataSlot);
    connect(this, &RecorderDataBase::addRecordSignal, this, &RecorderDataBase::addNewFile);
    connect(this, &RecorderDataBase::deleteRecordSignal,this, &RecorderDataBase::deleteFile);
    connect(this, &RecorderDataBase::updateNameSignal, this, &RecorderDataBase::updateNameSlot);

}

int RecorderDataBase::getStatus()
{
    return m_dbStatus;
}

QStringList RecorderDataBase::getDataList()
{
    return searchDataList;
}

int RecorderDataBase::initDataBase()
{
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    } else {
        QSqlQuery query;
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS Flag  "
                                "(file_id     integer   PRIMARY KEY AUTOINCREMENT,  "
                                "file_name    char(50)  NOT NULL,  "
                                "flag_time    char(50)  NOT NULL); ";
        query.prepare(creatTableStr);
        if (!query.exec()) {
            qDebug() << "表Flag建表失败!";
            m_dbStatus = CREATE_TABLE_FAILED;
            return CREATE_TABLE_FAILED;
        } else {
            qDebug() << "表Flag建表成功!";
            m_dbStatus = DB_OP_SUCC;
        }

        //新建第二个表
        QSqlQuery query2;
        QString creatTableStr2 = "CREATE TABLE IF NOT EXISTS File  "
                                "(file_id     integer   PRIMARY KEY AUTOINCREMENT,  "
                                "file_name    char(50)  NOT NULL,  "
                                "create_time  char(50)  NOT NULL); ";
        query2.prepare(creatTableStr2);
        if (!query2.exec()) {
            qDebug() << "表File建表失败!";
            m_dbStatus = CREATE_TABLE_FAILED;
            return CREATE_TABLE_FAILED;
        } else {
            qDebug() << "表File建表成功!";
            m_dbStatus = DB_OP_SUCC;
        }
    }
    return m_dbStatus;
}

int RecorderDataBase::updateDatabaseSlot(QString filePath, QString suffix)
{
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    } else {
        QString searchCmd = "select file_name_id from Flag";
        QSqlQuery searchQuery;
        searchQuery.exec(searchCmd);
        if(!searchQuery.exec(searchCmd)){
            QSqlQuery query;
            QString updateTableStr = "alter table Flag add file_name_id integer";
            if (!query.exec(updateTableStr)) {
                qDebug() << "表Flag添加字段file_name_id失败!";
            } else {
                qDebug() << "表Flag添加字段file_name_id成功!";
                m_dbStatus = DB_OP_SUCC;
            }
        }else{
            qDebug() <<"file_name_id字段在Flag中已存在";
            m_dbStatus = DB_OP_ADD_REPEAT;
        }
    }
    QSqlQuery query;
    QString searchCmd = "select file_id, file_name from Flag";
    query.exec(searchCmd);
    while(query.next()){
        int file_id = query.value(0).toInt();
        QString file_name = query.value(1).toString();
        QString full_path = filePath + "/" + file_name + suffix;      //获取当前根目录
        QSqlQuery query2;
        QString searchCmd2 = QString("select file_id from File where file_name='%1'").arg(full_path);
        query2.exec(searchCmd2);
        if(query2.next()){
            int id = query2.value(0).toInt();
            QString updateCmd = QString("update Flag set file_name_id=%1, file_name='%2' where file_id=%3").arg(id).arg(full_path).arg(file_id);
            QSqlQuery query3;
            query3.exec(updateCmd);
            if(query3.exec(updateCmd)){
                qDebug() << "表Flag更新file_name_id成功!";
            }
        }else{
            qDebug() << searchCmd2 << "文件路径不存在";
        }
    }
    return m_dbStatus;
}

int RecorderDataBase::addNewFile(QString name,QString time)
{
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    } else {

        int num = 0;
        QString searchCmd = QString("select count(file_name) from File where file_name = '%1' AND create_time = '%2'").arg(name).arg(time);
        QSqlQuery searchQuery;
        searchQuery.exec(searchCmd);
        if (!searchQuery.exec(searchCmd)) {
            qDebug() << "query error :" << searchQuery.lastError();
            m_dbStatus = DB_OP_GET_FAILED;
            return DB_OP_GET_FAILED;
        } else {
            while (searchQuery.next()) {
                num = searchQuery.value(0).toInt();
            }
        }

        if (num > 0) {
            qDebug() << "不可重复添加:" << DB_OP_ADD_REPEAT;
            m_dbStatus = DB_OP_ADD_REPEAT;
            return DB_OP_ADD_REPEAT;
        } else {
            QString cmd =QString("insert into File (%1) values('%2','%3')").arg(ITEM_FIELD2).arg(name).arg(time);
            QSqlQuery query;
            if (!query.exec(cmd)) {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_ADD_FAILED;
                return DB_OP_ADD_FAILED;
            } else {
                qDebug() << "新文件插入表File成功!";
                QString cmd2 = QString("select file_id from File where file_name='%1'").arg(name);
                QSqlQuery query2;
                query2.exec(cmd2);
                if(query2.next()){
                    int file_name_id = query2.value(0).toInt();
                    QString cmd3 = QString("update Flag set file_name_id=%1 where file_name='%2'").arg(file_name_id).arg(name);
                    QSqlQuery query3;
                    query3.exec(cmd3);
                    if(query3.exec(cmd3)){
                        qDebug() << "新文件存在Flag标识，更新Flag标识file_name_id成功!";
                    }
                }
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
        }
    }
}
int RecorderDataBase::deleteFile(QString name)
{
    QString existBaseName;
     if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
         QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
         m_dbStatus = DB_UNCONNECT;
         return DB_UNCONNECT;
     } else {
         QString searchCmd = QString("select file_name from File where file_name = '%1'").arg(name);
         QSqlQuery searchQuery;
         searchQuery.exec(searchCmd);
         if (!searchQuery.exec(searchCmd)) {
             qDebug() << "query error :" << searchQuery.lastError();
             m_dbStatus = DB_OP_GET_FAILED;
             return DB_OP_GET_FAILED;
         } else {
             while (searchQuery.next()) {
                 existBaseName = searchQuery.value(0).toString();
             }
         }
         if (existBaseName == name) {
             QString cmd = QString("delete from Flag where file_name = '%1'").arg(name);
             QSqlQuery query;
             if (!query.exec(cmd)) {
                 qDebug() << "query error :" << query.lastError();
                 m_dbStatus = DB_OP_DEL_FAILED;
                 return DB_OP_DEL_FAILED;
             } else {
                 qDebug() << "删除成功!";
                 m_dbStatus = DB_OP_SUCC;
                 return DB_OP_SUCC;
             }

         } else {
             m_dbStatus = FLAG_UNFOUND;
             return FLAG_UNFOUND;
         }
     }
}

void RecorderDataBase::getAllFiles(QStringList &fileNames, QStringList &fileTimes)
{
    fileNames.clear();
    fileTimes.clear();
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return ;
    }

    QString query = QString("select %1,%2 from File").arg("file_name").arg("create_time");
    QSqlQuery searchQuery;
    searchQuery.exec(query);
    if (!searchQuery.exec(query)) {
        qDebug() << "query error :" << searchQuery.lastError();
        m_dbStatus = DB_OP_GET_FAILED;
        return;
    }

    while (searchQuery.next()) {
        QString name = searchQuery.value(0).toString();
        QString time = searchQuery.value(1).toString();
        fileNames.append(name);
        fileTimes.append(time);
    }
    m_dbStatus = DB_OP_SUCC;
    return ;
}

int RecorderDataBase::insertDataSlot(QFileInfo file_info, QString flagTime)
{
    QString fileName = file_info.baseName();
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    } else {

        int num = 0;
        QString searchCmd = QString("select count(flag_time) from Flag where file_name = '%1' AND flag_time = '%2'").arg(file_info.absoluteFilePath()).arg(flagTime);
        QSqlQuery searchQuery;
        searchQuery.exec(searchCmd);
        if (!searchQuery.exec(searchCmd)) {
            qDebug() << "query error :" << searchQuery.lastError();
            m_dbStatus = DB_OP_GET_FAILED;
            return DB_OP_GET_FAILED;

        } else {

            while (searchQuery.next()) {

                num = searchQuery.value(0).toInt();
            }
        }
        if (num > 0) {
            qDebug() << "不可重复添加:" << DB_OP_ADD_REPEAT;
            m_dbStatus = DB_OP_ADD_REPEAT;
            return DB_OP_ADD_REPEAT;

        } else {
            QString cmd2 = QString("select file_id from File where file_name='%1'").arg(file_info.filePath());
            QSqlQuery query2;
            query2.exec(cmd2);
            if(query2.next()){
                int file_name_id = query2.value(0).toInt();
                QString cmd = QString("insert into Flag (file_name,flag_time) values('%1','%2')").arg(file_info.absoluteFilePath()).arg(flagTime);
                QSqlQuery query;
                if (!query.exec(cmd)) {
                    qDebug() << "query error :" << query.lastError();
                    m_dbStatus = DB_OP_ADD_FAILED;
                    return DB_OP_ADD_FAILED;
                } else {
                    qDebug() << "插入成功!";
                    m_dbStatus = DB_OP_SUCC;
                    return DB_OP_SUCC;
                }
            }else{
                QString cmd = QString("insert into Flag (%1) values('%2','%3')").arg(ITEM_FIELD).arg(file_info.filePath()).arg(flagTime);
                QSqlQuery query;
                if (!query.exec(cmd)) {
                    qDebug() << "query error :" << query.lastError();
                    m_dbStatus = DB_OP_ADD_FAILED;
                    return DB_OP_ADD_FAILED;
                } else {
                    qDebug() << "插入成功!";
                    m_dbStatus = DB_OP_SUCC;
                    return DB_OP_SUCC;
                }

            }
        }
    }
}
int RecorderDataBase::deleteDataSlot(QString fileBaseName)
{   // 删除File和Flag中的数据(目前用于裁剪后覆盖原文件功能）
    qDebug() << "删除数据库中此文件的标记" << fileBaseName;
    QString existBaseName;
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    } else {

        QFileInfo file(fileBaseName);
        QString cmd = QString("delete from Flag where file_name = '%1'").arg(file.baseName());
        QSqlQuery query;
        if (!query.exec(cmd)) {
            qDebug() << "query error :" << query.lastError();
            m_dbStatus = DB_OP_DEL_FAILED;
        } else {
            qDebug() << "由于裁剪覆盖录音，Flag数据库删除成功!";
            m_dbStatus = DB_OP_SUCC;
        }
        QString cmd2 = QString("delete from File where file_name = '%1'").arg(fileBaseName);
        QSqlQuery query2;
        if (!query2.exec(cmd2)) {
            qDebug() << "query error :" << query2.lastError();
            m_dbStatus = DB_OP_DEL_FAILED;
            return DB_OP_DEL_FAILED;
        } else {
            qDebug() << "由于裁剪覆盖录音，File数据库删除成功!";
            m_dbStatus = DB_OP_SUCC;
            return DB_OP_SUCC;
        }
    }
}
int RecorderDataBase::searchDataSlot(QFileInfo file_info)
{
    QString fileBaseName = file_info.baseName();
    qDebug() << "查找标记的文件 :" << fileBaseName;
    QString flagTime;
    if (!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    } else {
        QString searchCmd2 = QString("select file_id from File where file_name='%1'").arg(file_info.absoluteFilePath());
        QSqlQuery searchQuery2;
        searchQuery2.exec(searchCmd2);
        if(searchQuery2.next()){
            QString searchCmd = QString("select flag_time from Flag where file_name='%1'").arg(file_info.absoluteFilePath());
            QSqlQuery searchQuery;
            searchQuery.exec(searchCmd);
            if (!searchQuery.exec(searchCmd)) {
                qDebug() << "query error :" << searchQuery.lastError();
                m_dbStatus = DB_OP_GET_FAILED;
                return DB_OP_GET_FAILED;
            } else {
                searchDataList.clear();
                while (searchQuery.next()) {
                    flagTime = searchQuery.value(0).toString();
                    searchDataList.append(flagTime);
                }
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
        }else{
            m_dbStatus = DB_OP_SUCC;
            return DB_OP_SUCC;
        }
    }
}

int RecorderDataBase::updateNameSlot(QFileInfo file_info, QString new_name)
{
#ifndef WHITE_TEST
    QString fileBaseName = file_info.baseName();
    QString fileFullPath = file_info.filePath();
    qDebug()<<"改名这个文件 :"<<fileBaseName;
    if(!m_dataBase.open()) { //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0,QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    }else{    

        /*
          查询新名字在数据库中是否残存相关数据，如存在则要先删除数据库中的相关数据，再改名
        */
        QSqlQuery checkQuery;
        QString checkCmd = QString("select file_id from Flag where file_name='%1'").arg(file_info.path() + '/' + new_name + '.' + file_info.completeSuffix());
        checkQuery.exec(checkCmd);

        QSqlQuery checkQuery2;
        QString checkCmd2 = QString("select file_id from File where file_name='%1'").arg(file_info.path() + '/' + new_name + '.' + file_info.completeSuffix());
        checkQuery2.exec(checkCmd2);

        // 删除File 和 Flag两表中的new_name对于的数据
        while(checkQuery.next()){
            int id = checkQuery.value(0).toInt();
            QSqlQuery deleteQuery;
            QString deleteCmd = QString("delete from Flag where file_id=%1").arg(id);
            deleteQuery.exec(deleteCmd);
        }

        if(checkQuery2.next()){
            int id =checkQuery2.value(0).toInt();
            QSqlQuery deleteQuery2;
            QString deleteCmd2 = QString("delete from File where file_id=%1").arg(id);
            deleteQuery2.exec(deleteCmd2);
        }

        // 接下来为更改数据库中的名字信息操作
        // 1、修改Flag数据库中的数据
        Q_EMIT DATABASE_INSTANCE.searchDataSignal(file_info);
        if(!DATABASE_INSTANCE.getDataList().isEmpty()){
            QSqlQuery searchQuery;
            QString searchCmd = QString("select file_id from Flag where file_name='%1'").arg(fileFullPath);
            searchQuery.exec(searchCmd);
            while(searchQuery.next()){
                int id = searchQuery.value(0).toInt();
                QSqlQuery updateQuery;
                QString updateCmd = QString("update Flag set file_name='%1' where file_id=%2").arg(new_name).arg(id);
                updateQuery.exec(updateCmd);
                if(!updateQuery.exec(updateCmd)){
                    qDebug()<<"query error :"<<updateQuery.lastError();
                    m_dbStatus = DB_OP_GET_FAILED;
                    return DB_OP_GET_FAILED;
                }
            }
        }
        // 2、修改File数据库中的数据
        QSqlQuery searchQuery2;
        QString searchCmd2 = QString("select file_id from File where file_name='%1'").arg(fileFullPath);
        searchQuery2.exec(searchCmd2);
        if(searchQuery2.next()){
            int id2 = searchQuery2.value(0).toInt();
            QString newFullPath = file_info.path() + '/' + new_name + '.' + file_info.completeSuffix();
            QSqlQuery updateQuery2;
            QString updateCmd2 = QString("update File set file_name ='%1' where file_id=%2").arg(newFullPath).arg(id2);
            updateQuery2.exec(updateCmd2);
            if(!updateQuery2.exec(updateCmd2)){
                qDebug()<<"query error :"<<updateQuery2.lastError();
                m_dbStatus = DB_OP_GET_FAILED;
                return DB_OP_GET_FAILED;
            }
        }
        m_dbStatus = DB_OP_SUCC;
        return DB_OP_SUCC;
    }
#endif
}

RecorderDataBase::~RecorderDataBase()
{
    qDebug() << "析构RecorderDataBase";
    if (m_dataBase.open()) {
        m_dataBase.close();
    }
}
