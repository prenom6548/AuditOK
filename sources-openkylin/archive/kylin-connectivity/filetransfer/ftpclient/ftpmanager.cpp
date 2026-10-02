#include "ftpmanager.h"

#include <QtNetwork>
#include <QTimer>
#include "serverstatus.h"

const int RECONNECT_MAX_NUM = 1;

FtpManager::FtpManager(QObject *parent) : QObject(parent)
{
    initFtp();
    m_currentPath = "";
}

FtpManager::~FtpManager()
{
    m_isFree = true;
    m_reconnectNum = 0;
    m_isNoProgresss = false;
    clearErrorFiles();
    if (m_ftp != nullptr) {
        m_ftp->disconnect();
        m_ftp->abort();
        m_ftp->close();
        m_ftp->deleteLater();
        m_ftp = nullptr;
    }
}

void FtpManager::connectFtpServer()
{
    // 获取服务器状态
    ServerStatus *serverStatus = new ServerStatus(m_url);
    connect(serverStatus, &ServerStatus::sigServerState, this, [=](bool state) {
        if (state) {
            qInfo() << "Connect FTP server...";
            m_ftp->connectToHost(m_url.host(), m_url.port());
        } else {
            // 连接失败
            qInfo() << "FTP Server startup failed!";
            Q_EMIT sigFtpConnectFailed();
        }
        if (serverStatus != nullptr) {
            serverStatus->deleteLater();
        }
        if (m_ftpThread != nullptr) {
            m_ftpThread->quit();
            m_ftpThread->wait();
            m_ftpThread->deleteLater();
            m_ftpThread = nullptr;
        }
    });
    m_ftpThread = new QThread;
    serverStatus->moveToThread(m_ftpThread);
    connect(m_ftpThread, &QThread::started, serverStatus, &ServerStatus::startServer);
    m_ftpThread->start();
}

void FtpManager::cdToDir(QString dirName)
{
    m_currentPath = dirName;
    m_ftp->cd(dirName);
}

void FtpManager::downAllFiles(const QList<FileInfo> &fileList, QString downloadPath)
{
    m_downBeforePath = m_currentPath;
    if (!m_isFree) {
        // 当前正在下载，无法进行新任务
        qInfo() << "There are tasks in progress, unable to perform new task.";
        return;
    }
    m_isFree = false;
    // 重置用于计算更新传输进度的变量
    m_updateAllFileSize = 0;
    m_readProgress = 0;
    m_readIncrement = 0;
    m_beforeReadSize = 0;
    QList<FileInfo> currentDownloadFiles;
    QStringList dirPathList;
    for (int i = 0; i < fileList.size(); i++) {
        QString fileName = fileList.value(i).name();
        // 需要传输文件的总大小
        if (fileList.value(i).type() == FileInfo::FileType::Dir) {
            // 文件夹
            dirPathList.append(fileList.value(i).getPath());
        } else {
            // 文件
            m_updateAllFileSize += fileList.value(i).size();
            currentDownloadFiles.append(fileList.value(i));
        }
    }
    if (dirPathList.isEmpty()) {
        download(currentDownloadFiles, downloadPath);
        return;
    }
    m_downloadMap.insert(downloadPath, currentDownloadFiles);
    for (int i = 0; i < dirPathList.size(); i++) {
        handleDownloadDir(dirPathList.value(i), downloadPath);
    }
}

void FtpManager::setTempPath(QString path)
{
    m_tempPath = path;
}

void FtpManager::downFile(QString filepath)
{
    m_isNoProgresss = true;
    QString fileName = filepath.mid(filepath.lastIndexOf("/") + 1, filepath.size());
    QString dirPath = m_tempPath;
    QDir downDir(dirPath);
    if (!downDir.exists(dirPath)) {
        // 文件夹不存在，创建文件夹
        downDir.mkpath(dirPath);
    }
    // 创建文件
    QFile *file = new QFile(dirPath + fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        // 打开文件失败
        qCritical() << "Fail to open file!" << fileName;
        delete file;
        file = nullptr;
        return;
    }
    int id = m_ftp->get(filepath, file);
    m_downFiles.insert(id, file);
}

void FtpManager::uploadAllFiles(const QStringList &fileList, QString uploadPath)
{
    if (!m_isFree) {
        // 当前正在下载，无法进行新任务
        qInfo() << "There are tasks in progress, unable to perform new task.";
        return;
    }
    m_ftp->clearPendingCommands();
    m_isFree = false;
    // 重置用于计算更新传输进度的变量
    m_updateAllFileSize = 0;
    m_readProgress = 0;
    m_readIncrement = 0;
    m_beforeReadSize = 0;
    QStringList currentUploadFiles;
    QStringList dirPathList;
    for (int i = 0; i < fileList.size(); i++) {
        QString fileName = fileList.value(i);
        QFileInfo fileInfo(fileName);
        // 需要传输文件的总大小
        if (fileInfo.isDir()) {
            // 文件夹
            dirPathList.append(fileName);
        } else {
            // 文件暂时记录
            m_updateAllFileSize += fileInfo.size();
            currentUploadFiles.append(fileName);
        }
    }
    for (int i = 0; i < dirPathList.size(); i++) {
        handleUploadDir(dirPathList.value(i), uploadPath);
    }
    if (!currentUploadFiles.isEmpty()) {
        upload(currentUploadFiles, uploadPath);
    }
}

void FtpManager::list(QString path)
{
    m_currentAllFiles.clear();
    m_ftp->list(path);
}

void FtpManager::abort()
{
    m_isFree = true;
    m_reconnectNum = 0;
    m_isNoProgresss = false;
    m_ftp->abort();
    clearErrorFiles();
    connectFtp();
}

void FtpManager::ftpCommandFinished(int commandId, bool error)
{
    if (m_ftp->currentCommand() == QFtp::None) {
        if (error) {
            qWarning() << "CommandFinished: None!" << m_ftp->errorString();
        } else {
            qInfo() << "CommandFinished: None!";
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::SetTransferMode) {
        if (error) {
            qWarning() << "CommandFinished: SetTransferMode error!" << m_ftp->errorString();
        } else {
            qInfo() << "CommandFinished: SetTransferMode!";
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error) {
            qWarning() << "CommandFinished: Failed to connect to FTP server!";
            Q_EMIT sigFtpConnectFailed();
            return;
        }
        qInfo() << "CommandFinished: Successfully connected to FTP server!";
        // 连接成功，登录服务器，匿名登录
        m_ftp->login(m_url.userName(), m_url.password());
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Login) {
        if (error) {
            qWarning() << "CommandFinished: Failed to login to FTP server!";
            Q_EMIT sigFtpConnectFailed();
            return;
        }
        qInfo() << "CommandFinished: Login to FTP server!";
        // 登陆成功
        m_reconnectNum = 0;
        if (m_isFirstConnect) {
            m_currentPath = "/";
            Q_EMIT sigFtpServerConnected();
        } else {
            if (!m_isInternal) {
                Q_EMIT sigFtpReconnected();
                m_isInternal = false;
            }
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Remove) {
        if (error) {
            qWarning() << "CommandFinished: Remove error!" << m_ftp->errorString();
        } else {
            qInfo() << "CommandFinished: Remove the file success";
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Close) {
        if (error) {
            qWarning() << "CommandFinished: Close error!" << m_ftp->errorString();
        } else {
            qInfo() << "CommandFinished: Close!";
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Get) {
        QFile *file = m_downFiles.take(commandId);
        if (file == nullptr) {
            return;
        }
        if (error) {
            // 传输错误，删除文件
            file->remove();
            if (m_ftp->error() == QFtp::RejectTran) {
                qWarning() << "Insufficient local storage space!";
                m_isInternal = true;
                Q_EMIT sigRejectTran(ErrorType::Get);
            } else {
                QString fileName = file->fileName();
                qWarning() << "CommandFinished: get file error!" << m_ftp->errorString();
                Q_EMIT sigDownFileFail(fileName.mid(fileName.lastIndexOf("/") + 1, fileName.size()));
            }
            abort();
        } else {
            qInfo() << "CommandFinished: Download the file success: " << file->fileName();
            if (m_isNoProgresss) {
                m_isNoProgresss = false;
                Q_EMIT sigDownFileFinish(file->fileName());
            }
        }
        file->close();
        delete file;
        file = nullptr;
        m_downFiles.remove(commandId);
        if (m_downFiles.isEmpty() && !m_isFree) {
            m_isFree = true;
            Q_EMIT sigUpdateTransferProgress(0, 0);
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Put) {
        QFile *file = m_uploadFiles.take(commandId);
        if (file == nullptr) {
            return;
        }
        if (error) {
            if (m_ftp->error() == QFtp::RejectTran) {
                qWarning() << "Insufficient storage space on the other party's device!";
                m_isInternal = true;
                Q_EMIT sigRejectTran(ErrorType::Put);
            } else {
                QString fileName = file->fileName();
                qWarning() << "CommandFinished: Cancle the file success: " << fileName << m_ftp->errorString();
                Q_EMIT sigDownFileFail(fileName.mid(fileName.lastIndexOf("/") + 1, fileName.size()));
            }
            abort();
        } else {
            QString fileName = file->fileName();
            qInfo() << "CommandFinished: Upload the file success: " << fileName;
        }
        // 将该文件从上传文件中删除
        file->close();
        delete file;
        file = nullptr;
        m_uploadFiles.remove(commandId);
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Cd) {
        if (error) {
            qWarning() << "CommandFinished: Cd error!" << m_ftp->errorString() << m_currentPath;
            connectFtp();
        } else {
            qInfo() << "CommandFinished: Currently in " + m_currentPath + " directory!";
            // 切换目录成功，获取当前目录下的文件列表
            m_currentAllFiles.clear();
            m_ftp->list();
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::List) {
        if (error) {
            qWarning() << "CommandFinished: List error!" << m_ftp->errorString();
            connectFtp();
        } else {
            // 获取当前目录下的文件列表成功，发送信号
            qInfo() << "CommandFinished: Successfully obtained the list under " + m_currentPath + " directory!";
            if (m_isFree) {
                Q_EMIT sigCurrentDirectoryList(m_currentAllFiles);
            } else {
                QString downloadPath = m_taskDownload.value(0);
                qInfo() << "CommandFinished: Successfully obtained the list under " +
                               m_taskDownloadMap.value(downloadPath).currentPath + " directory!";
                QList<FileInfo> currentDownloadFiles;
                QStringList dirPathList;
                if (m_taskDownloadMap.value(downloadPath).fileList.isEmpty()) {
                    qInfo() << m_taskDownloadMap.value(downloadPath).currentPath << " folder is empty!";
                    handleDownTask(downloadPath);
                    return;
                }
                for (int i = 0; i < m_taskDownloadMap.value(downloadPath).fileList.size(); i++) {
                    if (m_taskDownloadMap.value(downloadPath).fileList.value(i).type() == FileInfo::FileType::Dir) {
                        // 文件夹
                        dirPathList.append(m_taskDownloadMap.value(downloadPath).fileList.value(i).getPath());
                    } else {
                        // 文件
                        m_updateAllFileSize += m_taskDownloadMap.value(downloadPath).fileList.value(i).size();
                        currentDownloadFiles.append(m_taskDownloadMap.value(downloadPath).fileList.value(i));
                    }
                }
                m_downloadMap.insert(downloadPath, currentDownloadFiles);
                for (int i = 0; i < dirPathList.size(); i++) {
                    handleDownloadDir(dirPathList.value(i), downloadPath);
                }
                handleDownTask(downloadPath);
            }
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Mkdir) {
        if (error) {
            qWarning() << "CommandFinished: Mkdir error!" << m_ftp->errorString();
            connectFtp();
        } else {
            qInfo() << "CommandFinished: Mkdir !";
            QStringList list = m_taskLocalMap.take(commandId);
            QString uploadPath = m_taskPathMap.take(commandId);
            if (!list.isEmpty()) {
                upload(list, uploadPath);
            }
        }
        m_taskLocalMap.remove(commandId);
        m_taskPathMap.remove(commandId);
        if (m_uploadFiles.isEmpty() && m_taskLocalMap.isEmpty() && m_taskPathMap.isEmpty()) {
            m_isFree = true;
            Q_EMIT sigUpdateTransferProgress(0, 0);
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Rmdir) {
        if (error) {
            qWarning() << "CommandFinished: Rmdir error!" << m_ftp->errorString();
            connectFtp();
        } else {
            qInfo() << "CommandFinished: Rmdir!";
        }
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Rename) {
        if (error) {
            qWarning() << "CommandFinished: Rename error!" << m_ftp->errorString();
            connectFtp();
        } else {
            qInfo() << "CommandFinished: Rename!";
            m_reconnectNum = 0;
        }
        return;
    }
}

void FtpManager::ftpCommandStarted(int commandId)
{
    Q_UNUSED(commandId);

    if (m_ftp->currentCommand() == QFtp::None) {
        qInfo() << "CommandStarted: None...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::SetTransferMode) {
        qInfo() << "CommandStarted: SetTransferMode...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::ConnectToHost) {
        qInfo() << "CommandStarted: Connected to FTP server...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Login) {
        qInfo() << "CommandStarted: Logining to FTP server...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Remove) {
        qInfo() << "CommandStarted: Removing file...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Close) {
        qInfo() << "CommandStarted: Closing...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Get) {
        qInfo() << "CommandStarted: Downloading file...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Put) {
        qInfo() << "CommandStarted: Uploading file...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Cd) {
        qInfo() << "CommandStarted: Switching to " + m_currentPath + " directory...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::List) {
        qInfo() << "CommandStarted: Getting current file list...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Mkdir) {
        qInfo() << "CommandStarted: Mkdir...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Rmdir) {
        qInfo() << "CommandStarted: Rmdir...";
        return;
    }

    if (m_ftp->currentCommand() == QFtp::Rename) {
        qInfo() << "CommandStarted: Rename...";
        return;
    }
}

void FtpManager::ftpStateChanged(int state)
{
    switch (state) {
        case QFtp::Unconnected: {
            qInfo() << "StateChanged: Unconnected.";
            connectFtp();
            break;
        }
        case QFtp::HostLookup: {
            qInfo() << "StateChanged: Host name lookup in progress.";
            break;
        }
        case QFtp::Connecting: {
            qInfo() << "StateChanged: Attempting to connect to host.";
            break;
        }
        case QFtp::Connected: {
            qInfo() << "StateChanged: The connection with the host has been realized.";
            break;
        }
        case QFtp::LoggedIn: {
            qInfo() << "StateChanged: Connection and user login have been realized.";
            break;
        }
        case QFtp::Closing: {
            qInfo() << "StateChanged: Connection closing.";
            break;
        }
        default:
            break;
    }
}

void FtpManager::addToList(const QVector<QUrlInfo> &urlInfos)
{
    for (int i = 0; i < urlInfos.size(); i++) {
        QUrlInfo urlInfo = urlInfos[i];
        if (urlInfo.name().compare(".") != 0) {
            // 获取文件信息
            FileInfo fileInfo;
            QString filePath = urlInfo.name().toUtf8();
            fileInfo.setName(filePath.mid(filePath.lastIndexOf("/") + 1, filePath.size()));
            fileInfo.setSize(urlInfo.size());
            fileInfo.setDateTime(urlInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            if (urlInfo.isDir()) {
                // 文件夹
                fileInfo.setType(FileInfo::FileType::Dir);
            }
            if (!m_isFree) {
                fileInfo.setPath(m_taskDownloadMap.value(m_taskDownload.value(0)).currentPath + "/" + filePath);
                m_taskDownloadMap.find(m_taskDownload.value(0)).value().fileList.append(fileInfo);
            } else {
                QString currentPath = "";
                if (m_currentPath == "/") {
                    currentPath = m_currentPath;
                } else {
                    currentPath = m_currentPath + "/";
                }
                fileInfo.setPath(currentPath + filePath);
                m_currentAllFiles.insert(fileInfo.name(), fileInfo);
            }
        }
    }
}

void FtpManager::updateDataTransferProgress(qint64 readBytes, qint64 totalBytes)
{
    if (!m_isNoProgresss) {
        m_readIncrement = readBytes - m_beforeReadSize;
        m_readProgress += m_readIncrement;
        m_beforeReadSize = readBytes;
        if (readBytes == totalBytes) {
            m_beforeReadSize = 0;
        }
        if (m_readProgress == m_updateAllFileSize) {
            m_isFree = true;
            // this->cdToDir(m_downBeforePath);
        }

        Q_EMIT sigUpdateTransferProgress(m_readProgress, m_updateAllFileSize);
    }
}

void FtpManager::initFtp()
{
    m_isFree = true;
    m_readProgress = 0;
    m_readIncrement = 0;
    m_beforeReadSize = 0;
    m_updateAllFileSize = 0;
    m_currentAllFiles.clear();
    m_downFiles.clear();
    m_uploadFiles.clear();
    m_taskLocalMap.clear();
    m_taskPathMap.clear();
    m_taskDownloadMap.clear();
    m_taskDownload.clear();
    m_downloadMap.clear();
    m_ftp = new QFtp;
    connect(m_ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(ftpCommandFinished(int, bool)));
    connect(m_ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCommandStarted(int)));
    connect(m_ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpStateChanged(int)));
    connect(m_ftp, SIGNAL(listInfos(QVector<QUrlInfo>)), this, SLOT(addToList(QVector<QUrlInfo>)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64, qint64)), this,
            SLOT(updateDataTransferProgress(qint64, qint64)));
}

void FtpManager::handleUploadDir(QString dirPath, QString uploadPath)
{
    QDir dir(dirPath);  // QDir的路径一定要是全路径，相对路径会有错误

    if (!dir.exists()) {
        qCritical() << dirPath << "Folder does not exist!";
        return;
    }

    QString dirName = dirPath.mid(dirPath.lastIndexOf("/") + 1, dirPath.size());
    QString key = dirName;
    QString tag = "";
    int i = 1;
    while (m_currentAllFiles.contains(key) && m_currentAllFiles.value(key).type() == FileInfo::FileType::Dir) {
        tag = QString("(%1)").arg(QString::number(i));
        key = dirName + tag;
        i++;
    }
    dirName = key;
    QString uploadDirPath = uploadPath + "/" + dirName;
    QStringList currentUploadFiles;
    QStringList dirPathList;

    // 取到所有的文件和文件名，去掉.和..文件夹
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    // 将其转化为一个list
    QFileInfoList fileInfoList = dir.entryInfoList();
    if (fileInfoList.size() < 1) {
        qInfo() << dirPath << "Folder is empty!";
        int id = m_ftp->mkdir(uploadDirPath);
        m_taskLocalMap.insert(id, currentUploadFiles);
        m_taskPathMap.insert(id, uploadDirPath);
        return;
    }
    i = 0;
    do {
        QFileInfo fileInfo = fileInfoList.at(i);
        bool isDir = fileInfo.isDir();
        if (isDir) {
            // 文件夹
            dirPathList.append(fileInfo.filePath());
        } else {
            // 文件
            m_updateAllFileSize += fileInfo.size();
            currentUploadFiles.append(fileInfo.filePath());
        }
        ++i;
    } while (i < fileInfoList.size());

    int id = m_ftp->mkdir(uploadDirPath);
    m_taskLocalMap.insert(id, currentUploadFiles);
    m_taskPathMap.insert(id, uploadDirPath);

    for (int i = 0; i < dirPathList.size(); i++) {
        handleUploadDir(dirPathList.value(i), uploadDirPath);
    }

    return;
}

void FtpManager::handleDownloadDir(QString dirPath, QString downloadPath)
{
    QString dirName = dirPath.mid(dirPath.lastIndexOf("/") + 1, dirPath.size());
    QString localDirPath = downloadPath + "/" + dirName;
    QString key = dirName;
    QString tag = "";
    int i = 1;
    while (1) {
        QDir dir(localDirPath);
        if (!dir.exists()) {
            // 文件夹不存在
            dir.mkpath(localDirPath);
            break;
        }
        qInfo() << localDirPath << "file already exist.Add a suffix to it.";
        tag = QString("(%1)").arg(QString::number(i));
        key = dirName + tag;
        localDirPath = downloadPath + "/" + key;
        i++;
    }
    m_taskDownload.append(localDirPath);
    DownloadTask task;
    task.localPath = localDirPath;
    task.currentPath = dirPath;
    m_taskDownloadMap.insert(localDirPath, task);
    m_ftp->list(dirPath);

    return;
}

void FtpManager::upload(const QStringList &fileList, QString uploadPath)
{
    if (fileList.isEmpty()) {
        return;
    }
    for (int i = 0; i < fileList.size(); i++) {
        // 先上传当前目录下的文件
        QString filePath = fileList.value(i);
        QString fileName = filePath.mid(filePath.lastIndexOf('/') + 1, filePath.size());
        QString key = fileName.mid(0, fileName.lastIndexOf("."));
        QString suffix = fileName.mid(fileName.lastIndexOf("."), fileName.size() - key.size());
        // QString
        QString tag = "";
        int num = 1;
        while (m_currentAllFiles.contains(fileName)) {
            tag = QString("(%1)").arg(QString::number(num));
            fileName = key + tag + suffix;
            num++;
        }
        QFile *file = new QFile(filePath);
        if (!file->open(QIODevice::ReadOnly)) {
            // 打开文件失败
            qCritical() << "fail to open file";
            delete file;
            file = nullptr;
            continue;
        }
        int id = 0;
        id = m_ftp->put(file, uploadPath + "/" + fileName);
        m_uploadFiles.insert(id, file);
    }
}

void FtpManager::download(const QList<FileInfo> &fileList, QString downloadPath)
{
    if (fileList.isEmpty()) {
        return;
    }
    for (int i = 0; i < fileList.size(); i++) {
        QString fileName = fileList.value(i).name();
        QString localDirPath = downloadPath + "/" + fileName;
        QString key = fileName.mid(0, fileName.lastIndexOf("."));
        QString suffix = fileName.mid(fileName.lastIndexOf("."), fileName.size() - key.size());
        QString tag = "";
        int num = 1;
        while (1) {
            QFileInfo fileInfo(localDirPath);
            if (!fileInfo.exists()) {
                // 文件不存在
                break;
            }
            qInfo() << localDirPath << "file already exist.Add a suffix to it.";
            tag = QString("(%1)").arg(QString::number(num));
            localDirPath = downloadPath + "/" + key + tag + suffix;
            num++;
        }
        // 创建文件
        QFile *file = new QFile(localDirPath);
        if (!file->open(QIODevice::WriteOnly)) {
            // 打开文件失败
            qCritical() << "Fail to open file!" << localDirPath;
            delete file;
            file = nullptr;
            continue;
        }
        int id = m_ftp->get(fileList.value(i).getPath(), file);
        m_downFiles.insert(id, file);
    }
}

void FtpManager::handleDownTask(QString key)
{
    m_taskDownload.removeAt(0);
    m_taskDownloadMap.remove(key);
    QMap<QString, QList<FileInfo>>::iterator it = m_downloadMap.begin();
    while (it != m_downloadMap.end()) {
        if (it.value().isEmpty()) {
            it = m_downloadMap.erase(it);
        } else {
            it++;
        }
    }
    if (m_taskDownload.isEmpty()) {
        if (m_downloadMap.isEmpty()) {
            qInfo() << "The file does not exist in the download path.";
            Q_EMIT sigUpdateTransferProgress(0, 0);
            m_isFree = true;
        } else {
            QMap<QString, QList<FileInfo>>::iterator it = m_downloadMap.begin();
            while (it != m_downloadMap.end()) {
                download(it.value(), it.key());
                it++;
            }
            m_downloadMap.clear();
        }
    }
}

void FtpManager::clearErrorFiles()
{
    qInfo() << "Cleaning up files for incomplete transfer...";
    // 清理未完成下载的文件
    QMap<int, QFile *>::iterator downIt = m_downFiles.begin();
    while (downIt != m_downFiles.end()) {
        if (downIt.value() == nullptr) {
            downIt++;
            continue;
        } else {
            downIt.value()->close();
            downIt.value()->remove();
            downIt.value()->deleteLater();
            downIt.value() = nullptr;
        }
    }
    // 清理未完成上传的文件
    QMap<int, QFile *>::iterator uploadIt = m_uploadFiles.begin();
    while (uploadIt != m_uploadFiles.end()) {
        if (uploadIt.value() == nullptr) {
            uploadIt++;
            continue;
        } else {
            QString fileName = uploadIt.value()->fileName();
            QString filePath = fileName.mid(0, fileName.lastIndexOf('/') + 1);
            fileName = fileName.mid(fileName.lastIndexOf('/') + 1, fileName.size());
            if (filePath != m_currentPath) {
                m_ftp->cd(filePath);
                m_ftp->remove(fileName);
                m_ftp->cd(m_currentPath);
            } else {
                m_ftp->remove(fileName);
            }
            uploadIt.value()->close();
            uploadIt.value()->deleteLater();
            uploadIt.value() = nullptr;
        }
    }
}

void FtpManager::connectFtp()
{
    if (m_reconnectNum >= RECONNECT_MAX_NUM) {
        return;
    }
    m_reconnectNum++;
    m_isFirstConnect = false;
    ServerStatus *serverStatus = new ServerStatus(m_url);
    connect(serverStatus, &ServerStatus::sigServerState, this, [=](bool state) {
        if (state) {
            qInfo() << "Reconnect to the FTP server...";
            m_ftp->deleteLater();
            m_ftp = nullptr;
            initFtp();
            m_ftp->clearPendingCommands();
            m_ftp->connectToHost(m_url.host(), m_url.port());
        } else {
            // 连接失败
            qInfo() << "FTP Server startup failed!";
            Q_EMIT sigFtpConnectFailed();
        }
        serverStatus->deleteLater();
        if (m_ftpThread != nullptr) {
            m_ftpThread->quit();
            m_ftpThread->wait();
            m_ftpThread->deleteLater();
            m_ftpThread = nullptr;
        }
    });
    m_ftpThread = new QThread;
    serverStatus->moveToThread(m_ftpThread);
    connect(m_ftpThread, &QThread::started, serverStatus, &ServerStatus::startServer);
    m_ftpThread->start();
}

void FtpManager::setTransferMode(int mode)
{
    switch (mode) {
        case QFtp::Active:
            m_ftp->setTransferMode(QFtp::Active);
            break;
        case QFtp::Passive:
            m_ftp->setTransferMode(QFtp::Passive);
            break;
    }
}

void FtpManager::setUrl(QUrl url)
{
    m_url = url;
}
