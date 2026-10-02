#include "filesyncmanage.h"

#include <QDir>
#include <QThread>
#include <QUrl>
#include <QDirIterator>

const QStringList CONFIG_FILE_LIST = {"pictureInfo",       "videoInfo",       "musicInfo",       "documentInfo",
                                      "wechatPictureInfo", "wechatVideoInfo", "wechatMusicInfo", "wechatDocumentInfo",
                                      "qqPictureInfo",     "qqVideoInfo",     "qqMusicInfo",     "qqDocumentInfo",
                                      "fileNum",           "searchInfo.db"};
const QString ANDROID_FILE_PATH = "/Android/data/com.kylin.mobileassistant/cache/";
const QString FILE_PATH_FILENUM = ANDROID_FILE_PATH + QString("fileNum");
const QString FILE_PATH_SEARCH = ANDROID_FILE_PATH + QString("searchInfo.db");
const QStringList FILE_PATH_LIST = {"picture/", "video/", "music/", "document/", "", "", "wechatFiles/", "qqFiles/"};
const QString FILE_DOWN_PATH = getenv("HOME") + QString("/.connectivitycache/cache/");
const QString FILE_HOME_PATH = QString("/home/");
const QString FILE_PATH_PCSEARCH = QString("/.connectivitycache/pcsearchInfo.db");
const QString THUMB_DOWN_PATH = "/thumbnail/";
const QString TEMP_DOWN_PATH = "/temp/";
const QStringList FLAGE_LIST = {"", "wechat_", "qq_"};
const QString PICTURE_FLAGE = "picture_";
const QString VIDEO_FLAGE = "video_";

FileSyncManage::FileSyncManage(QString uuid, QUrl url, QObject *parent) : QObject(parent)
{
    m_uuid = uuid;
    m_url = url;
    m_fileParser = new FileParSer(m_url, this);
    m_task = new DownloadTask;
    m_thread = new QThread;
    m_task->moveToThread(m_thread);
    connect(m_task, &DownloadTask::sigTaskFinish, this, &FileSyncManage::slotTaskFinsh);
    connect(m_task, &DownloadTask::sigTaskFail, this, &FileSyncManage::slotTaskFail);
    m_thread->start();
}
FileSyncManage::~FileSyncManage()
{
    if (m_task != nullptr) {
        m_task->deleteLater();
        m_task = nullptr;
    }
    if (m_thread != nullptr) {
        m_thread->quit();
        m_thread->wait();
        m_thread->deleteLater();
        m_thread = nullptr;
    }
    clearTempFiles(FILE_DOWN_PATH + m_uuid + THUMB_DOWN_PATH);
    clearTempFiles(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH);
}

void FileSyncManage::downFile(const QStringList &fileList, QString downloadPath, QString flag)
{
    QDir downDir(downloadPath);
    if (!downDir.exists(downloadPath)) {
        // 文件夹不存在，创建文件夹
        downDir.mkpath(downloadPath);
    }
    m_fileList = fileList;
    m_downPath = downloadPath;
    m_fileFlag = flag;

    Q_EMIT m_task->sigDownFile(m_fileList.value(0), m_downPath, m_url, m_fileFlag);
    m_fileList.removeAt(0);
}

void FileSyncManage::abortDown()
{
    if (m_task != nullptr) {
        Q_EMIT m_task->sigAbort();
    }
}

void FileSyncManage::getItemCount(FileParSer::KeyType type, QMap<int, int> &map)
{
    map.clear();
    qInfo() << "Set classification list information.";
    m_fileParser->getItemCountList(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(FileType::FileNum), type,
                                   map);
}

void FileSyncManage::getClassifiedFileList(FileSyncManage::FileKey key, FileSyncManage::FileType type,
                                           QList<FileInfo> &list)
{
    qInfo() << "Get classification file list...";
    int index = getIndex(key, type);
    switch (type) {
        case FileType::Picture:
            m_fileParser->getFileInfoList(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index), list,
                                          FLAGE_LIST.value(key) + PICTURE_FLAGE);
            break;
        case FileType::Video:
            m_fileParser->getFileInfoList(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index), list,
                                          FLAGE_LIST.value(key) + VIDEO_FLAGE);
            break;
        case FileType::Music:
            m_fileParser->getFileInfoList(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index), list);
            break;
        case FileType::Doc:
            m_fileParser->getFileInfoList(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index), list);
            break;
    }
}

void FileSyncManage::updateFileInfo(FileSyncManage::FileKey key, FileSyncManage::FileType type)
{
    QString keyStr = "";
    int index = getIndex(key, type);
    if (type != FileType::FileNum) {
        keyStr = FILENUM_KEY_LIST.value(index);
    }
    qInfo() << "Start getting update files...";
    DownloadTask *updateTask = new DownloadTask;
    QThread *thread = new QThread;
    updateTask->moveToThread(thread);
    connect(updateTask, &DownloadTask::sigTaskFail, this, [=](QString fileName, QString flag) {
        qInfo() << "Update file synchronization failures.";
        Q_EMIT sigDownFileFail(fileName, "");
        if (updateTask != nullptr) {
            updateTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    connect(updateTask, &DownloadTask::sigTaskFinish, this, [=](QString flag, QString fileName) {
        qInfo() << "Update file synchronization completed.";
        QString targetData = getFileNumInfo(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH + fileName);
        if (targetData.isEmpty()) {
            qInfo() << "Get version information is null.";
            Q_EMIT sigDownAllFileFinish("");
            return;
        }
        // 判断文件是否需要更新
        bool isUpdate = m_fileParser->isNeedUpdate(
            FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(FileType::FileNum), targetData, keyStr);
        if (type == FileType::FileNum) {
            Q_EMIT sigDownAllFileFinish("");
            if (isUpdate) {
                downloadSearch();
            } else {
                if (m_isFirst) {
                    m_isFirst = false;
                    Q_EMIT sigSearchInfoFinsh(FILE_DOWN_PATH + m_uuid + "/" +
                                              CONFIG_FILE_LIST.value(FileType::SearchFile));
                }
            }
            return;
        }

        if (isUpdate) {
            downClassifiedFile(key, type);
            downloadSearch();
        } else {
            // 获取需要更新的文件路径
            QString filePath = FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index);
            QFileInfo fileInfo(filePath);
            if (!fileInfo.exists()) {
                // 文件不存在，下载更新
                qInfo() << filePath << "The file does not exist and needs to be updated.";
                bool flag = m_fileParser->setKeyItemInfo(
                    FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(FileType::FileNum), keyStr);
                if (!flag) {
                    Q_EMIT sigDownFileFail(FILE_PATH_FILENUM, "");
                    return;
                }
                downClassifiedFile(key, type);
                downloadSearch();
            } else {
                Q_EMIT sigDownAllFileFinish("");
                if (m_isFirst) {
                    m_isFirst = false;
                    Q_EMIT sigSearchInfoFinsh(FILE_DOWN_PATH + m_uuid + "/" +
                                              CONFIG_FILE_LIST.value(FileType::SearchFile));
                }
            }
        }
        if (updateTask != nullptr) {
            updateTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    thread->start();

    QString version = QString::number(QDateTime::currentSecsSinceEpoch());
    Q_EMIT updateTask->sigDownFile(FILE_PATH_FILENUM, FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH, m_url, version);
}

void FileSyncManage::downThumb(FileSyncManage::FileKey key, FileSyncManage::FileType type)
{
    abortDown();
    qInfo() << "Load thumbnail...";
    QString flag = "";
    int index = getIndex(key, type);
    QString path = FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(index);
    switch (type) {
        case FileType::Picture:
            flag = FLAGE_LIST.value(key) + PICTURE_FLAGE;
            break;
        case FileType::Video:
            flag = FLAGE_LIST.value(key) + VIDEO_FLAGE;
            break;
    }
    QStringList list;
    m_fileParser->getThumbList(path, list);
    if (list.isEmpty()) {
        qWarning() << "The obtained thumbnail list is empty";
        return;
    }
    downFile(list, FILE_DOWN_PATH + m_uuid + THUMB_DOWN_PATH, flag);
}

void FileSyncManage::downTempFile(QString filePath)
{
    qInfo() << "Download temporary file for opening.";
    QDir downDir(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH);
    if (!downDir.exists(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH)) {
        // 文件夹不存在，创建文件夹
        downDir.mkpath(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH);
    }
    DownloadTask *tempTask = new DownloadTask;
    QThread *thread = new QThread;
    tempTask->moveToThread(thread);
    connect(tempTask, &DownloadTask::sigTaskFail, this, [=](QString fileName, QString flag) {
        qInfo() << fileName << " file download failures.";
        Q_EMIT sigDownFileFail(fileName, flag);
        if (tempTask != nullptr) {
            tempTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    connect(tempTask, &DownloadTask::sigTaskFinish, this, [=](QString flag, QString fileName) {
        qInfo() << fileName << " file download complete.";
        Q_EMIT sigTempFileDownFinish(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH +
                                     filePath.mid(filePath.lastIndexOf("/") + 1, filePath.size()));
        if (tempTask != nullptr) {
            tempTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    thread->start();

    Q_EMIT tempTask->sigDownFile(filePath, FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH, m_url, "");
}

void FileSyncManage::abortDownThumb()
{
    qInfo() << "Stop loading thumbnails...";
    abortDown();
}

void FileSyncManage::clearFile()
{
    clearTempFiles(FILE_DOWN_PATH + m_uuid + THUMB_DOWN_PATH);
    clearTempFiles(FILE_DOWN_PATH + m_uuid + TEMP_DOWN_PATH);
}

void FileSyncManage::nextTask()
{
    if (!m_fileFlag.isEmpty()) {
        while (!m_fileList.isEmpty()) {
            QString filePath = m_fileList.value(0);
            QString fileName = m_fileFlag + filePath.mid(filePath.lastIndexOf("/") + 1, filePath.size());
            QFileInfo fileInfo(m_downPath + fileName);
            if (fileInfo.exists()) {
                // 下载文件本地已存在
                Q_EMIT sigDownFileFinish(m_fileFlag, fileName);
                m_fileList.removeAt(0);
            } else {
                break;
            }
        }
    }
    if (!m_fileList.isEmpty()) {
        Q_EMIT m_task->sigDownFile(m_fileList.value(0), m_downPath, m_url, m_fileFlag);
        m_fileList.removeAt(0);
    } else {
        qInfo() << "All download tasks completed.";
        Q_EMIT sigDownAllFileFinish(m_fileFlag);
    }
}
void FileSyncManage::downClassifiedFile(FileSyncManage::FileKey key, FileSyncManage::FileType type)
{
    abortDown();
    QStringList list;
    qInfo() << "Download information...";
    if (key == FileKey::Default) {
        list << ANDROID_FILE_PATH + FILE_PATH_LIST.value(type) + CONFIG_FILE_LIST.value(type);
    } else {
        int index = int(key) + 5;
        int configIndex = getIndex(key, type);
        list << ANDROID_FILE_PATH + FILE_PATH_LIST.value(index) + CONFIG_FILE_LIST.value(configIndex);
    }
    downFile(list, FILE_DOWN_PATH + m_uuid + "/");
}

void FileSyncManage::clearTempFiles(const QString path)
{
    QDir dir(path);
    if (dir.isEmpty()) {
        qInfo() << "Cache is empty!";
        return;
    }
    // 第三个参数是QDir的过滤参数，这三个表示收集所有文件和目录，且不包含"."和".."目录。
    // 因为只需要遍历第一层即可，所以第四个参数填QDirIterator::NoIteratorFlags
    QDirIterator dirsIterator(path, QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (dirsIterator.hasNext()) {
        if (!dir.remove(dirsIterator.next()))  // 删除文件操作如果返回否，那它就是目录
        {
            QDir(dirsIterator.filePath()).removeRecursively();  // 删除目录本身以及它下属所有的文件及目录
        }
    }
}

void FileSyncManage::setSearchPath(FileSyncManage::SearchFileType type, QString homePath)
{
    switch (type) {
        case SearchFileType::Android:
            m_searchfilePath = FILE_PATH_SEARCH;
            break;
        case SearchFileType::Pc:
            m_searchfilePath = FILE_HOME_PATH + homePath + FILE_PATH_PCSEARCH;
            break;
    }
}

void FileSyncManage::downloadSearch()
{
    DownloadTask *searchTask = new DownloadTask;
    QThread *thread = new QThread;
    searchTask->moveToThread(thread);
    connect(searchTask, &DownloadTask::sigTaskFail, this, [=](QString name, QString flag) {
        qInfo() << "Search file download failures.";
        Q_EMIT sigDownFileFail(name, flag);
        if (searchTask != nullptr) {
            searchTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    connect(searchTask, &DownloadTask::sigTaskFinish, this, [=](QString, QString) {
        qInfo() << "Search file download complete.";
        Q_EMIT sigSearchInfoFinsh(FILE_DOWN_PATH + m_uuid + "/" + CONFIG_FILE_LIST.value(FileType::SearchFile));
        if (searchTask != nullptr) {
            searchTask->deleteLater();
        }
        if (thread != nullptr) {
            thread->quit();
            thread->wait();
            thread->deleteLater();
        }
    });
    thread->start();

    Q_EMIT searchTask->sigDownFile(m_searchfilePath, FILE_DOWN_PATH + m_uuid + "/", m_url, "");
}

QString FileSyncManage::getFileNumInfo(QString filePath)
{
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        // 打开文件失败
        qCritical() << "fail to open file" << filePath;
        delete file;
        file = nullptr;
        return QString("");
    }

    QByteArray data = file->readAll();
    delete file;
    file = nullptr;
    QString targetData = data;
    return targetData;
}

int FileSyncManage::getIndex(FileSyncManage::FileKey key, FileSyncManage::FileType type)
{
    return int(key) * 4 + int(type);
}

void FileSyncManage::slotTaskFinsh(QString flag, QString name)
{
    if (!flag.isEmpty()) {
        Q_EMIT sigDownFileFinish(flag, FILE_DOWN_PATH + m_uuid + THUMB_DOWN_PATH + name);
    } else if (name == CONFIG_FILE_LIST.value(FileType::FileNum)) {
    }
    nextTask();
}

void FileSyncManage::slotTaskFail(QString name, QString flag)
{
    Q_EMIT sigDownFileFail(name, flag);
    nextTask();
}
