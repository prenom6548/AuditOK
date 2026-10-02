#ifndef FTPMANAGER_H
#define FTPMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QMap>
#include <QStack>
#include <QFile>
#include <QString>
#include <QUrl>
#include "qftp.h"
#include "fileinfo.h"

class FtpManager : public QObject
{
    Q_OBJECT
public:
    explicit FtpManager(QObject *parent = nullptr);

    ~FtpManager();

    // 连接ftp服务器
    void connectFtpServer();

    // 切换目录
    void cdToDir(QString dirName = "");

    // 下载文件
    void downAllFiles(const QList<FileInfo> &fileList, QString downloadPath);

    // 设置临时路径
    void setTempPath(QString path);

    // 下载临时文件
    void downFile(QString filepath);

    // 上传文件
    void uploadAllFiles(const QStringList &fileList, QString uploadPath);

    // 获取path下文件列表
    void list(QString path);

    // 中止当前操作
    void abort();

    // 切换连接模式 QFtp::Passive[默认]（被动） | QFtp::Active（主动）
    void setTransferMode(int mode);

    // 设置服务器地址
    void setUrl(QUrl url);

    enum ErrorType { Put = 0, Get };

Q_SIGNALS:
    void sigCurrentDirectoryList(const QMap<QString, FileInfo> &);
    void sigUpdateTransferProgress(qint64, qint64);
    void sigFtpServerConnected();
    void sigFtpConnectFailed();
    void sigDownFileFail(QString);
    void sigFtpReconnected();
    void sigDownFileFinish(QString);
    void sigRejectTran(ErrorType);

public Q_SLOTS:
    void ftpCommandFinished(int commandId, bool error);
    void ftpCommandStarted(int commandId);
    void ftpStateChanged(int state);
    void addToList(const QVector<QUrlInfo> &urlInfos);
    void updateDataTransferProgress(qint64 readBytes, qint64 totalBytes);

private:
    void initFtp();
    void handleUploadDir(QString dirPath, QString uploadPath);
    void handleDownloadDir(QString dirPath, QString downloadPath);
    void upload(const QStringList &fileList, QString uploadPath);
    void download(const QList<FileInfo> &fileList, QString downloadPath);
    void handleDownTask(QString key);
    void clearErrorFiles();
    // 重新连接服务器
    void connectFtp();

private:
    struct DownloadTask {
        QString localPath = "";
        QString currentPath = "";
        QList<FileInfo> fileList;
    };

private:
    QFtp *m_ftp = nullptr;                          // 客户端
    QThread *m_ftpThread = nullptr;                 // 另起线程，尝试连接ftp服务器
    QUrl m_url;                                     // 主机地址
    QString m_currentPath;                          // 当前所在路径
    QString m_downBeforePath = "";                  // 进行下载之前的路径
    QMap<QString, FileInfo> m_currentAllFiles;      // 当前路径下文件列表
    QMap<int, QFile *> m_downFiles;                 // 所有下载文件
    QMap<int, QFile *> m_uploadFiles;               // 所有上传文件
    QMap<int, QStringList> m_taskLocalMap;          // 用于记录上传文件夹中文件的本地路径
    QMap<int, QString> m_taskPathMap;               // 用于记录每一次任务的上传|下载路径
    QMap<QString, DownloadTask> m_taskDownloadMap;  // 记录当前下载信息
    QStringList m_taskDownload;                     // 当前的下载队列
    QMap<QString, QList<FileInfo>> m_downloadMap;   // 用于记录当前需要下载的文件和路径
    qint64 m_updateAllFileSize = 0;                 // 传输所有文件的大小
    qint64 m_readProgress = 0;                      // 当前文件的传输进度
    qint64 m_readIncrement = 0;                     // 传输速度
    qint64 m_beforeReadSize = 0;                    // 上一次传输的大小
    bool m_isFree = true;                           // 当前是否正在进行下载
    bool m_isFirstConnect = true;                   // 第一次连接服务器
    int m_reconnectNum = 0;
    bool m_isNoProgresss = false;                   // 是否需要进度
    QString m_tempPath = "";                        // 临时下载路径
    bool m_isInternal = false;                      // 是否需要发出重连信号
};

#endif  // FTPMANAGER_H
