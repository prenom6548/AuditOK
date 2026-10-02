#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QObject>
#include <QRunnable>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

class DownloadTask : public QObject
{
    Q_OBJECT
public:
    DownloadTask(QObject *parent = nullptr);
    ~DownloadTask();

public Q_SLOTS:
    void readContent();
    void replyFinished(QNetworkReply *e);
    void slotDownloadFile(QString filePath, QString downPath, QUrl url, QString flag);
    void slotAbort();

private:
    void abortDown();
    void deleteDownload();
    // 开始下载文件
    void downloadFile();

private:
    QFile *m_file = nullptr;
    QNetworkAccessManager *m_accessManager = nullptr;
    QNetworkReply *m_reply = nullptr;
    QTimer *m_timer = nullptr; // 下载超时
    QUrl m_url;                // 服务器地址
    QString m_downPath = "";   // 下载路径
    QString m_fileName = "";   // 文件名
    QString m_fileFlag = "";   // 文件前缀
    QString m_filePath = "";   // 文件路径
    int m_errorNum = 1;        // 下载失败次数

Q_SIGNALS:
    void sigTaskFinish(QString, QString);
    void sigTaskFail(QString, QString);
    void sigDownFile(QString filePath, QString downPath, QUrl url, QString flag);
    void sigAbort();
};

#endif // DOWNLOADTASK_H
