#include "dirmanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

const QString PATH = getenv("HOME");
const QString PICTURE = "图片";
const QString VIDEO = "视频";
const QString AUDIO = "音乐";
const QString DOCUMENT = "文档";
const QString DOWNLOAD = "下载";

DirManager::DirManager()
{
    m_dataBase = new FileDataBase;
    m_fileinfoList.clear();
    m_filePathList.clear();
}

DirManager::~DirManager()
{
    if (m_dataBase != nullptr) {
        delete m_dataBase;
        m_dataBase = nullptr;
    }
}

void DirManager::stopRun()
{
    m_dataBase->stopRun();
}

void DirManager::run()
{
    QStringList list;
    list.append(QString(PICTURE));
    list.append(QString(VIDEO));
    list.append(QString(AUDIO));
    list.append(QString(DOCUMENT));
    list.append(QString(DOWNLOAD));

    qInfo() << "Start updating search database ...";
    handleDir(PATH, list);
    int dataNum = m_dataBase->removeData(m_filePathList);
    qInfo() << "Deleted " << dataNum << " pieces of data";

    if (!isInterruptionRequested()) {
        dataNum = m_dataBase->insertData(m_fileinfoList);
        qInfo() << dataNum << " pieces of data updated";
    }

    Q_EMIT finish();
}

void DirManager::handleDir(const QString &path, QStringList &dirList)
{
    if (isInterruptionRequested()) {
        return;
    }
    if (dirList.isEmpty()) {
        return;
    }
    QString dirName = dirList.value(0);
    QString dirPath = path + "/" + dirName;
    dirList.removeAt(0);
    QDir dir(dirPath);
    QStringList fileList = dir.entryList(QDir::Files);
    handleFile(dirPath, fileList);
    // 递归处理目录，不包含隐藏目录
    QStringList subDirList = dir.entryList(QDir::Dirs);
    subDirList.removeOne(".");
    subDirList.removeOne("..");
    handleDir(dirPath, subDirList);
    handleDir(path, dirList);

    return;
}

void DirManager::handleFile(const QString &path, QStringList &fileList)
{
    if (isInterruptionRequested()) {
        return;
    }
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
    m_fileinfoList.append(fileinfo);
    m_filePathList.append(filePath);
    // 递归处理文件信息
    handleFile(path, fileList);

    return;
}