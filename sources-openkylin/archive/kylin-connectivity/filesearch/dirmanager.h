#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QList>

#include "filedatabase.h"
#include "fileinfo.h"

class DirManager : public QThread
{
    Q_OBJECT
public:
    DirManager();
    ~DirManager();

    void stopRun();

protected:
    void run();

private:
    void handleDir(const QString &path, QStringList &dirList);
    void handleFile(const QString &path, QStringList &fileList);

private:
    FileDataBase *m_dataBase = nullptr;
    QList<FileInfo> m_fileinfoList;  // 文件信息列表，保存遍历的文件信息
    QStringList m_filePathList;  // 单独保存所以文件路径，用于对比删除数据库中不存在的文件信息
Q_SIGNALS:
    void finish();               // 处理完成信号

public Q_SLOTS:
};

#endif  // DIRMANAGER_H