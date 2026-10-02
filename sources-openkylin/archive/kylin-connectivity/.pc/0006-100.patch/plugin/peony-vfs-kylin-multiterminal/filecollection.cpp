#include "filecollection.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <sys/syslog.h>

FileCollection::FileCollection(QString path)
{
    m_path = path;
    m_fileinfoList.clear();
}

FileCollection::~FileCollection() {}

void FileCollection::run()
{
    QDir dir(m_path);
    QStringList subDirList = dir.entryList(QDir::AllDirs);
    subDirList.removeOne(".");
    subDirList.removeOne("..");
    handleDir(m_path, subDirList);
    QStringList fileList = dir.entryList(QDir::Files);
    handleFile(m_path, fileList);

    Q_EMIT finish(m_fileinfoList);
}

void FileCollection::handleDir(const QString &path, QStringList &dirList)
{
    if (dirList.isEmpty()) {
        return;
    }
    QString dirName = dirList.value(0);
    QString dirPath = path + "/" + dirName;
    dirList.removeAt(0);
    QFileInfo info(dirPath);
    FileInfo fileinfo;
    fileinfo.setName(info.fileName());
    fileinfo.setPath(dirPath);
    fileinfo.setSize(info.size());
    fileinfo.setDateTime(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    fileinfo.setType(FileInfo::FileType::Dir);
    m_fileinfoList.addFileInfo(fileinfo);
    handleDir(path, dirList);

    return;
}

void FileCollection::handleFile(const QString &path, QStringList &fileList)
{
    if (fileList.isEmpty()) {
        return;
    }
    QString fileName = fileList.value(0);
    QString filePath = path + "/" + fileName;
    fileList.removeAt(0);
    QFileInfo info(filePath);
    FileInfo fileinfo;
    fileinfo.setName(info.fileName());
    fileinfo.setPath(filePath);
    fileinfo.setSize(info.size());
    fileinfo.setDateTime(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    m_fileinfoList.addFileInfo(fileinfo);
    handleFile(path, fileList);

    return;
}
