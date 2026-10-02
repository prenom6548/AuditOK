#ifndef FILESYNCMANGE_H
#define FILESYNCMANGE_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QMap>

#include "fileinfo.h"
#include "fileparser.h"
#include "downloadtask.h"

class FileSyncManage : public QObject
{
    Q_OBJECT
public:
    FileSyncManage(QString uuid, QUrl url, QObject *parent = nullptr);
    ~FileSyncManage();

    enum FileKey {
        Default = 0,  // 默认
        WeChat,       // 微信
        QQ,           // QQ
    };

    enum FileType {
        Picture = 0,      // 图片
        Video = 1,        // 视频
        Music = 2,        // 音乐
        Doc = 3,          // 文档
        FileNum = 12,     // 分类文件数量
        SearchFile = 13,  // 搜索索引文件
    };

    enum SearchFileType {
        Android = 0,  // 移动设备搜索文件
        Pc = 1,       // PC设备搜索文件
    };

    // 中止当前下载
    void abortDown();

    // 获取分类列表项数
    void getItemCount(FileParSer::KeyType type, QMap<int, int> &map);

    // 获取分类列表
    void getClassifiedFileList(FileSyncManage::FileKey key, FileSyncManage::FileType type, QList<FileInfo> &lsit);

    void updateFileInfo(FileSyncManage::FileKey key, FileSyncManage::FileType type);

    // 下载缩略图
    void downThumb(FileSyncManage::FileKey key, FileSyncManage::FileType type);

    // 停止缩略图下载
    void abortDownThumb();

    // 下载临时文件
    void downTempFile(QString filePath);

    // 清理文件
    void clearFile();

    void setSearchPath(FileSyncManage::SearchFileType type, QString homePath = "");

    void downloadSearch();

private:
    void downFile(const QStringList &fileList, QString downloadPath, QString flag = "");
    void nextTask();
    void downClassifiedFile(FileSyncManage::FileKey key, FileSyncManage::FileType type);
    void clearTempFiles(const QString path);
    QString getFileNumInfo(QString filePath);
    int getIndex(FileSyncManage::FileKey key, FileSyncManage::FileType type);

private:
    DownloadTask *m_task = nullptr;      // 下载任务
    QThread *m_thread = nullptr;         // 下载线程
    FileParSer *m_fileParser = nullptr;  // 文件解析
    QUrl m_url;                          // 服务器地址
    QString m_uuid = "";                 // 连接设备的uuid
    QStringList m_fileList;              // 下载文件地址列表
    QString m_downPath = "";             // 下载路径
    QString m_fileName = "";             // 文件名
    QString m_fileFlag = "";             // 文件前缀
    QString m_searchfilePath = "";       // 搜索文件下载路径
    bool m_isFirst = true;

Q_SIGNALS:
    void sigDownFileFinish(QString, QString);
    void sigDownAllFileFinish(QString);
    void sigDownFileFail(QString, QString);
    void sigSearchInfoFinsh(QString);
    void sigTempFileDownFinish(QString);

public Q_SLOTS:
    void slotTaskFinsh(QString flag, QString name);
    void slotTaskFail(QString name, QString flag);
};

#endif  // FILESYNCMANGE_H
