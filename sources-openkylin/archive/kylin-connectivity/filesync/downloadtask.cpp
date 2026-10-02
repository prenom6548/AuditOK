#include "downloadtask.h"

#include <QDir>

const int ERROR_MAX_NUM = 5;

DownloadTask::DownloadTask(QObject *parent) : QObject(parent)
{
    connect(this, &DownloadTask::sigDownFile, this, &DownloadTask::slotDownloadFile);
    connect(this, &DownloadTask::sigAbort, this, &DownloadTask::slotAbort);
}

DownloadTask::~DownloadTask()
{
    if (m_timer != nullptr) {
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (m_file != nullptr) {
        m_file->flush();
        m_file->close();
        m_file->deleteLater();
        m_file = nullptr;
    }
    if (m_reply != nullptr) {
        if (m_reply->isRunning()) {
            m_reply->abort();
        } else {
            m_reply->close();
        }
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_accessManager != nullptr) {
        m_accessManager->deleteLater();
        m_accessManager = nullptr;
    }
}

void DownloadTask::slotDownloadFile(QString filePath, QString downPath, QUrl url, QString flag)
{
    m_filePath = filePath;
    m_downPath = downPath;
    m_fileFlag = flag;
    m_url = url;
    m_url.setPath(m_filePath);
    m_errorNum = 1;
    downloadFile();
}

//下载时将FTP服务器上文件内容写入本地文件
void DownloadTask::readContent()
{
    if (m_timer != nullptr) {
        // 有数据读取，停止定时器
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    QByteArray readData = m_reply->readAll();
    m_file->write(readData);
}

void DownloadTask::replyFinished(QNetworkReply *e)
{
    if (e->error() == QNetworkReply::OperationCanceledError) {
        // 取消下载
        qInfo() << m_fileName << ":" << e->errorString();
        deleteDownload();
        return;
    }
    if (e->error() == QNetworkReply::NoError) {
        // 下载成功
        m_file->flush();
        m_file->close();
        m_file->deleteLater();
        m_file = nullptr;
        m_reply->close();
        m_reply->deleteLater();
        m_reply = nullptr;
        m_accessManager->deleteLater();
        m_accessManager = nullptr;
        Q_EMIT sigTaskFinish(m_fileFlag, m_fileName);
        return;
    }
    if (m_errorNum <= ERROR_MAX_NUM) {
        m_errorNum++;
        deleteDownload();
        downloadFile();
        return;
    }
    // 其他错误
    qInfo() << m_fileName << ":" << e->errorString();
    Q_EMIT sigTaskFail(m_fileName, m_fileFlag);
    deleteDownload();
}

void DownloadTask::slotAbort()
{
    abortDown();
}

void DownloadTask::deleteDownload()
{
    if (m_timer != nullptr) {
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (m_file != nullptr) {
        m_file->flush();
        m_file->close();
        m_file->remove();
        m_file->deleteLater();
        m_file = nullptr;
    }
    if (m_reply != nullptr) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_accessManager != nullptr) {
        m_accessManager->deleteLater();
        m_accessManager = nullptr;
    }
}

void DownloadTask::downloadFile()
{
    QDir downDir(m_downPath);
    if (!downDir.exists(m_downPath)) {
        // 文件夹不存在，创建文件夹
        downDir.mkpath(m_downPath);
    }
    m_fileName = m_fileFlag + m_filePath.mid(m_filePath.lastIndexOf("/") + 1, m_filePath.size());
    m_file = new QFile(m_downPath + m_fileName); // 指定文件下载到的本地路径
    bool openFlag = m_file->open(QIODevice::WriteOnly);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file:" + m_downPath + m_fileName;
        delete m_file;
        m_file = nullptr;
        return;
    }
    m_accessManager = new QNetworkAccessManager(this);
    m_accessManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QNetworkRequest request(m_url);
    m_reply = m_accessManager->get(request); // 发送下载请求
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(
        m_timer, &QTimer::timeout, this,
        [=]() {
            qInfo() << m_fileName << " download request timed out trying to download again!";
            m_timer->deleteLater();
            m_timer = nullptr;
            deleteDownload();
            downloadFile();
        },
        Qt::DirectConnection);
    m_timer->start(1000);
    connect((QObject *)m_reply, SIGNAL(readyRead()), this, SLOT(readContent()));
    connect(m_accessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
}

void DownloadTask::abortDown()
{
    if (m_reply != nullptr && m_reply->isRunning()) {
        m_reply->abort();
    }
    deleteDownload();
}
