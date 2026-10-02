#ifndef FILECOLLECTION_H
#define FILECOLLECTION_H

#include <QObject>
#include <QThread>
#include <QStringList>

#include "fileinfo.h"
#include "fileinfolist.h"

class FileCollection : public QThread
{
    Q_OBJECT
public:
    FileCollection(QString path);
    ~FileCollection();

protected:
    void run();

private:
    void handleDir(const QString &path, QStringList &dirList);
    void handleFile(const QString &path, QStringList &fileList);

private:
    QString m_path = "";
    FileInfoList m_fileinfoList; // 文件信息列表，保存遍历的文件信息
    QStringList m_filePathList; // 单独保存所以文件路径，用于对比删除数据库中不存在的文件信息
Q_SIGNALS:
    void finish(FileInfoList); // 处理完成信号

public Q_SLOTS:
};

#endif // FILECOLLECTION_H
