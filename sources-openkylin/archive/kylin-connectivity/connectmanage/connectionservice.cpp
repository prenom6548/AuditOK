#include "connectionservice.h"

#include <QDebug>
#include <QHostInfo>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include "config.h"
#include "generatetools.h"

const QString SOCKET_NAME = "KYLIN";
const int LISTEN_PORT = 27188;
const int FTPPORT = 27184;
const int FTPSERVERPORT = 27189;
const int FORWARD_PORT = 2300;
const int FORWARD_PORT1 = 2301;
const int HEARTBEAT_PORT = 27181;
const int HEART_RATE = 10000;
const QString FILE_SAVE_PATH = getenv("HOME") + QString("/下载/");
const QString UUID_FILE_LOACLPATH = getenv("HOME") + QString("/.connectivitycache/UUID");
const QString UUID_FILE_REMOTEPATH = QString("/storage/emulated/0/Android/data/com.kylin.mobileassistant/cache/UUID");
const QString DEVICEID_FILE_LOACLPATH = getenv("HOME") + QString("/.connectivitycache/DeviceId");

ConnectionService::ConnectionService(QObject *parent) : QObject(parent)
{
    m_fileSavePath = FILE_SAVE_PATH;
    m_adb = new AdbProcess(this);
    connect(m_adb, &AdbProcess::adbProcessResult, this, &ConnectionService::onAdbExec);

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ConnectionService::slotServerNewConnection);
    m_tcpServer->listen(QHostAddress::Any, LISTEN_PORT);
}

ConnectionService::~ConnectionService()
{
    deleteSocket();
    if (m_serverStatus != nullptr) {
        m_serverStatus->deleteLater();
        m_serverStatus = nullptr;
    }
    if (m_statusThread != nullptr) {
        m_statusThread->quit();
        m_statusThread->wait();
        m_statusThread->deleteLater();
        m_statusThread = nullptr;
    }
}

bool ConnectionService::startClient(const QString host)
{
    if (m_messageTcpSocket != nullptr) {
        qInfo() << "There is currently a connection in progress!";
        return false;
    }
    m_isClient = true;
    m_serverIp = host;
    m_connectInfo.connectType = ConnectType::WIFI;
    m_messageTcpSocket = new QTcpSocket;
    connect(m_messageTcpSocket, &QTcpSocket::readyRead, this, &ConnectionService::onMessageReadyRead);
    connect(m_messageTcpSocket, &QTcpSocket::connected, this, [=]() {
        m_isMsgSocketConnect = true;
        // 客户端启动心跳服务监听
        if (m_heartTcpServer == nullptr) {
            m_heartTcpServer = new QTcpServer;
            connect(m_heartTcpServer, &QTcpServer::newConnection, this, &ConnectionService::slotHeartbeatConnection);
        }
        int sendRe = m_heartTcpServer->listen(QHostAddress::Any, HEARTBEAT_PORT);
        if (sendRe == -1) {
            qInfo() << "Tcpserver listening port " + QString::number(HEARTBEAT_PORT) + " failed!";
        } else {
            qInfo() << "Tcpserver listening port " + QString::number(HEARTBEAT_PORT) + " succeeded!";
        }
    });
    m_messageTcpSocket->connectToHost(host, LISTEN_PORT);
    return true;
}

void ConnectionService::setConnectionCallback(ConnectSuccessCallBack connectSuccessCallBack,
                                              ConnectFailCallBack connectFailCallBack)
{
    m_connectSuccessCallBack = connectSuccessCallBack;
    m_connectFailCallBack = connectFailCallBack;
}

void ConnectionService::connectRemoteHost(const QString address)
{
    qInfo() << "Attempting to connect to 1722 host.";
    QUrl url;
    url.setHost(address);
    url.setPort(LISTEN_PORT);
    m_serverStatus = new ServerStatus(url);
    connect(m_serverStatus, &ServerStatus::sigServerState, this, &ConnectionService::slotServerState);
    m_statusThread = new QThread;
    m_serverStatus->moveToThread(m_statusThread);
    connect(m_statusThread, &QThread::started, m_serverStatus, &ServerStatus::startServer);
    connect(m_statusThread, &QThread::finished, m_statusThread, &QThread::deleteLater);
    m_statusThread->start();
}

void ConnectionService::setConnectionRespond(bool isAgree)
{
    if (isAgree) {
        if (m_connectInfo.deviceType == ConnectionService::DeviceType::ANDROID) {
            // 同意请求
            if (m_connectInfo.connectType == ConnectType::WIFI) {
                // 当前为wifi连接，尝试连接adb
                m_adb->connectAdbd(m_connectInfo.address);
            } else {
                if (m_connectInfo.connectType == ConnectType::USB) {
                    QFile file(DEVICEID_FILE_LOACLPATH);
                    // 打开文件，不存在则创建
                    file.open(QIODevice::ReadWrite | QIODevice::Text);
                    file.write(m_deviceSerialList.value(0).toUtf8());
                    file.close();
                }
                m_androidConnType = AndroidConnType::ADB;
                handleConnected();
            }
        } else {
            // 已同意连接，向客户端发送设备信息
            handleConnected();
        }
    } else {
        // 拒绝
        ReplyMessage replyMessage;
        replyMessage.set_status(ReplyMessage::REFUSE);
        sendMessage(replyMessage);
        m_messageTcpSocket->waitForBytesWritten();
        deleteSocket();
    }
}

bool ConnectionService::abortService()
{
    qInfo() << "The machine actively disconnects.";
    bool sendRe = true;
    if (m_messageTcpSocket != nullptr) {
        if (!m_isRemoteDisconnect) {
            // 不是远程主机主动断开，发送断开消息
            ReplyMessage replyMessage;
            replyMessage.set_status(ReplyMessage::DISCONNECTED);
            sendRe = sendMessage(replyMessage);
            m_messageTcpSocket->waitForBytesWritten();
        }
        deleteSocket();
    }
    return sendRe;
}

bool ConnectionService::sendByteData(const QString data)
{
    std::string str = data.toStdString();
    const char *ch = str.c_str();
    bool sendRe = sendByteData(ch, str.length());
    return sendRe;
}

bool ConnectionService::sendByteData(const char *buffer, const qint64 bufSize)
{
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::DATA);
    replyMessage.set_datasize(bufSize);
    m_data = QByteArray::fromRawData(buffer, bufSize);
    m_dataSize = bufSize;
    bool sendRe = sendMessage(replyMessage);
    return sendRe;
}

void ConnectionService::setFileSavePath(const QString path)
{
    m_fileSavePath = path;
}

bool ConnectionService::sendFileData(const QString path)
{
    QStringList list;
    list.clear();
    list.append(path);
    bool sendRe = sendFileData(list);
    return sendRe;
}

bool ConnectionService::sendFileData(const QStringList pathList)
{
    m_filePathList = pathList;
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::FILEINFO);
    ProtobufFilesInfo *filesInfo = replyMessage.mutable_files_info();
    for (int i = 0; i < m_filePathList.size(); i++) {
        QFileInfo fileInfo(m_filePathList.value(i));
        ProtobufFileInfo *protoFileInfo;
        protoFileInfo = filesInfo->add_file_info();
        protoFileInfo->set_id(i);
        protoFileInfo->set_name(fileInfo.fileName().toStdString().c_str());
        protoFileInfo->set_size(fileInfo.size());
        protoFileInfo->set_path(fileInfo.filePath().toStdString().c_str());
        protoFileInfo->set_date(fileInfo.metadataChangeTime().toString("yyyy-MM-dd").toStdString().c_str());
    }
    bool sendRe = sendMessage(replyMessage);
    return sendRe;
}

bool ConnectionService::openStream(bool flag)
{
    // todo
    return true;
}

bool ConnectionService::sendStreamData(char *buffer, qint32 bufSize)
{
    // todo
    return true;
}

void ConnectionService::connectLocDev()
{
    m_connectInfo.connectType = ConnectType::USB;
    updateDevice();
}

bool ConnectionService::isSupportADB()
{
    // todo
    // 应该还需要判断当前是否有设备序列号，以及与当前设备序列号是否相同
    return m_isSupportADB;
}

QString ConnectionService::getDeviceSerial()
{
    return m_deviceSerialList.value(0);
}

bool ConnectionService::closeAbility()
{
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::CANCELSCREEN);
    bool sendRe = sendMessage(replyMessage);
    return sendRe;
}

void ConnectionService::slotServerNewConnection()
{
    qInfo() << "There is a new connection.";
    if (!m_isMsgSocketConnect) {
        m_isMsgSocketConnect = true;
        if (m_messageTcpSocket != nullptr) {
            qWarning() << "Currently connected:" << m_connectInfo.deviceName;
            return;
        }
        // 建立消息通道
        m_messageTcpSocket = m_tcpServer->nextPendingConnection();
        connect(m_messageTcpSocket, &QTcpSocket::readyRead, this, &ConnectionService::onMessageReadyRead);
        if (m_connectInfo.connectType == ConnectType::NOTCONNECT) {
            m_connectInfo.connectType = ConnectType::WIFI;
        }
        // 发送确认信息
        ReplyMessage replyMessage;
        replyMessage.set_status(ReplyMessage::CONNECTED);
        sendMessage(replyMessage);
    } else {
        // 建立数据通道
        qInfo() << "Data socket connected.";
        m_dataTcpSocket = m_tcpServer->nextPendingConnection();
        connect(m_dataTcpSocket, &QTcpSocket::readyRead, this, &ConnectionService::onDataReadyRead);
    }
}

void ConnectionService::onMessageReadyRead()
{
    if (m_messageTcpSocket == nullptr) {
        return;
    }
    // 读取ReplyMessage
    if (m_isHead) {
        // 读取数据头部，获取数据长度
        char sizeBuf[16] = {'\0'};
        qint64 readSize = m_messageTcpSocket->read(sizeBuf, sizeof(sizeBuf));
        m_dataSize = QByteArray(sizeBuf, readSize).toLongLong();
        m_data.clear();
        m_isHead = false;
    }
    char buf[4096] = {'\0'};
    qint64 size = 0;
    while ((size = m_messageTcpSocket->read(buf, sizeof(buf))) > 0) {
        QByteArray item(buf, size);
        m_data += item;
        memset(buf, '\0', sizeof(buf));
    }

    if (m_data.size() < m_dataSize) {
        // 数据接受不完整，等待下一次接收
        return;
    }
    m_isHead = true;

    ReplyMessage reply;
    bool ok = reply.ParseFromString(m_data.toStdString());
    if (!ok) {
        // protobuf反序列化错误
        qInfo() << "Message parsing error!";
    }

    switch (reply.status()) {
        case ReplyMessage::ERROR: {
            // 错误消息
            qInfo() << "ERROR:" << QString::fromStdString(reply.error_string());
        } break;
        case ReplyMessage::CONNECTED: {
            // CONNECTED 发送设备信息
            qInfo() << "Generate device information and send it to remote host.";
            // 发送设备信息
            sendDeviceInfo();
        } break;
        case ReplyMessage::REFUSE: {
            qInfo() << "The remote host refused the connection.";
            Q_EMIT sigConnectDenied();
            deleteSocket();
        } break;
        case ReplyMessage::DISCONNECTED: {
            // DISCONNECTED 断开连接
            m_isRemoteDisconnect = true;
            qInfo() << "Remote host disconnected.";
            disconnectSocket();
        } break;
        case ReplyMessage::DEVICEINFO: {
            // DEVICEINFO 读取设备信息DeviceInfo 记录设备信息
            qInfo() << "Received device sent by remote host.";
            receiveDeviceInfo(reply);
        } break;
        case ReplyMessage::DATA: {
            // DATA 接收字节数据
            qInfo() << "Byte data transfer will be performed.";
            if (m_dataType != DataType::FREE) {
                // 正在进行其他任务
                sendBusyMessage();
                return;
            }
            m_dataType = DataType::BYTE_DATA;
            m_dataSize = reply.datasize();
            ReplyMessage replyMessage;
            replyMessage.set_status(ReplyMessage::DATAREADY);
            sendMessage(replyMessage);
        } break;
        case ReplyMessage::DATAREADY: {
            int sendRe = -1;
            if (m_dataTcpSocket != nullptr) {
                sendRe = m_dataTcpSocket->write(m_data.data(), m_dataSize);
            }
            if (sendRe == -1) {
                qInfo() << "Byte data sending failed!";
            } else {
                qInfo() << "Byte data sent successfully!";
            }
        } break;
        case ReplyMessage::FILEINFO: {
            qInfo() << "File data transfer is about to take place.";
            if (m_dataType != DataType::FREE) {
                sendBusyMessage();
                return;
            }
            m_dataType = DataType::FILE_DATA;
            // FILEINFO 接收文件信息
            m_filesInfo = reply.files_info();
            m_dataSize = 0;
            m_data.clear();
            m_currentFileIndex = 0;
            m_writeSize = 0;
            ReplyMessage replyMessage;
            // 获取所有文件大小，并且计算0kb文件数量
            int zeroFileNum = 0;
            for (int i = 0; i < m_filesInfo.file_info_size(); i++) {
                qint64 size = m_filesInfo.file_info(i).size();
                if (size == 0) {
                    zeroFileNum++;
                }
                m_dataSize += size;
            }
            // 处理0kb文件
            if (zeroFileNum == m_filesInfo.file_info_size()) {
                for (int i = 0; i < m_filesInfo.file_info_size(); i++) {
                    qWarning() << "An empty file was received!";
                    QFile *file = new QFile(m_fileSavePath + QString::fromStdString(m_filesInfo.file_info(i).name()));
                    bool openFlag = file->open(QIODevice::WriteOnly);
                    if (!openFlag) {
                        // 打开文件失败
                        qCritical() << "Fail to open file:" + QString::fromStdString(m_filesInfo.file_info(i).name());
                        delete file;
                        continue;
                    }
                    file->write("", 0);
                    file->flush();
                    file->close();
                    delete file;
                }
                m_dataType = DataType::FREE;
                m_filesInfo.clear_file_info();
                m_filesInfo.Clear();
                replyMessage.set_status(ReplyMessage::FILEREADY);
                sendMessage(replyMessage);
                break;
            }
            // 判断本地空间是否充足
            if (!GenerateTools::isEnough(m_dataSize)) {
                qInfo() << "Insufficient valid local space!";
                m_dataType = DataType::FREE;
                replyMessage.set_status(ReplyMessage::ERROR);
                replyMessage.set_error_string(QString("接收端可用存储空间不足").toStdString().c_str());
            } else {
                replyMessage.set_status(ReplyMessage::FILEREADY);
            }
            sendMessage(replyMessage);
        } break;
        case ReplyMessage::FILEREADY: {
            // FILEREADY 发送文件内容
            QByteArray arr;
            for (int i = 0; i < m_filePathList.size(); i++) {
                QFile *file = new QFile(m_filePathList.value(i));
                bool openFlag = file->open(QIODevice::ReadOnly);
                if (!openFlag) {
                    // 打开文件失败
                    qCritical() << m_filePathList.value(i) << " file open failed";
                    delete file;
                    file = nullptr;
                    return;
                }
                arr.append(file->readAll());
            }
            int sendRe = -1;
            if (m_dataTcpSocket != nullptr) {
                sendRe = m_dataTcpSocket->write(arr.data(), arr.size());
            }
            if (sendRe == -1) {
                qInfo() << "File content sending failed!";
            } else {
                qInfo() << "File content sent successfully!";
            }
        } break;
        case ReplyMessage::STREAM: {
            qInfo() << "Streaming data will be transmitted.";
            if (m_dataType != DataType::FREE) {
                sendBusyMessage();
                return;
            }
            m_dataType = DataType::STREAM_DATA;
            // STREAM 准备接受流数据

        } break;
        case ReplyMessage::STREAMREADY: {
            // STREAMREADY 发送流数据
        } break;
        case ReplyMessage::BUSY: {
            qInfo() << "The remote host is busy.";
            // BUSY 繁忙处理
        } break;
    }
}

void ConnectionService::onDataReadyRead()
{
    if (m_dataTcpSocket == nullptr) {
        return;
    }
    char buf[4096] = {'\0'};
    qint64 size = 0;
    while ((size = m_dataTcpSocket->read(buf, sizeof(buf))) > 0) {
        QByteArray item(buf, size);
        m_data += item;
        memset(buf, '\0', sizeof(buf));
    }
    switch (m_dataType) {
        case DataType::BYTE_DATA: {
            if (m_data.size() >= m_dataSize) {
                qInfo() << "Byte data transmission finish!";
                Q_EMIT sigByteReceived(QString(m_data));
                m_data.clear();
                m_dataType = DataType::FREE;
            }
        } break;
        case DataType::FILE_DATA: {
            if (m_writeFile == nullptr) {
                openWriteFile();
            }
            if (m_writeSize < m_filesInfo.file_info(m_currentFileIndex).size()) {
                if (m_data.size() <= (m_filesInfo.file_info(m_currentFileIndex).size() - m_writeSize)) {
                    m_writeFile->write(m_data.data(), m_data.size());
                    m_writeSize += m_data.size();
                    m_data.clear();
                } else {
                    while (m_data.size() >= (m_filesInfo.file_info(m_currentFileIndex).size() - m_writeSize)) {
                        // 切割单个文件数据
                        int index = m_filesInfo.file_info(m_currentFileIndex).size() - m_writeSize;
                        QByteArray fileDate = m_data.mid(0, index);
                        m_writeFile->write(fileDate.data(), fileDate.size());
                        m_writeSize += fileDate.size();
                        m_data = m_data.mid(index, m_data.size() - index);
                        if (m_writeSize == m_filesInfo.file_info(m_currentFileIndex).size()) {
                            endFile();
                            return;
                        }
                    }
                }
            }

            if (m_writeSize == m_filesInfo.file_info(m_currentFileIndex).size()) {
                endFile();
            }
        } break;
        case DataType::STREAM_DATA: {
        } break;
    }
}

void ConnectionService::onHeartbeatReadyRead()
{
    if (m_heartbeatSocket == nullptr) {
        qInfo() << "Heartbeat channel not established!";
        return;
    }
    QByteArray data = "";
    char buf[4096] = {'\0'};
    qint64 size = 0;
    while ((size = m_heartbeatSocket->read(buf, sizeof(buf))) > 0) {
        QByteArray item(buf, size);
        data += item;
        memset(buf, '\0', sizeof(buf));
    }
    ReplyMessage reply;
    bool ok = reply.ParseFromString(data.toStdString());
    if (!ok) {
        qInfo() << "Message parsing error!";
    }
    if (reply.status() == ReplyMessage::URGENTCLIENT) {
        if (!m_isClient) {
            // 当前为服务端，收到客户端的心跳，需要回复
            sendHeartbeat(false);
        } else {
            // 当前为客户端，收到服务端心跳回复，重置标志位
            m_isAgainSendHeartbeat = true;
            m_isSurvival = true;
        }
    } else if (reply.status() == ReplyMessage::URGENTSERVER) {
        if (m_isClient) {
            // 当前为客户端，收到服务端的心跳，需要回复
            sendHeartbeat(true);
        } else {
            // 当前为服务端端，收到客户端心跳回复，重置标志位
#ifdef DEBUG
            qDebug() << "receive heartbeat<<<<<<<<<<<<<<<<<<<<<<<<<<<";
#endif  // !DEBUG
            m_isAgainSendHeartbeat = true;
            m_isSurvival = true;
        }
    }
}

void ConnectionService::slotServerState(bool state)
{
    if (state) {
        m_connectSuccessCallBack();
    } else {
        m_connectFailCallBack();
    }
    m_statusThread->quit();
    if (m_serverStatus != nullptr) {
        m_serverStatus->deleteLater();
        m_serverStatus = nullptr;
    }
}

void ConnectionService::disconnectSocket()
{
    Q_EMIT sigDisconnect();
}

void ConnectionService::onAdbExec(AdbProcess::ADB_EXEC_RESULT processResult)
{
    QStringList args = m_adb->arguments();

    switch (processResult) {
        case AdbProcess::AER_ERROR_START: {
            qWarning() << "adb run error";
            handleWifiConnection();
            break;
        }
        case AdbProcess::AER_SUCCESS_START: {
            break;
        }
        case AdbProcess::AER_ERROR_EXEC: {
            qWarning() << "adb exec error";
            handleWifiConnection();
            if (args.contains("com.kylin.mobileassistant")) {
                qWarning() << "Not Found Apk";
                Q_EMIT sigNotFountApk();
                break;
            }
            break;
        }
        case AdbProcess::AER_ERROR_MISSING_BINARY: {
            qInfo() << "adb not find";
            handleWifiConnection();
            break;
        }
        case AdbProcess::AER_SUCCESS_EXEC: {
            if (args.contains("devices")) {
                // 当前执行为updatedevices
                qInfo() << "adb devices";
                // 获取设备列表
                QStringList devices = m_adb->getDevicesSerialFromStdOut();
                m_deviceSerialList.clear();
                for (auto &item : devices) {
                    m_deviceSerialList.append(item);
                }
                // 多个adb设备
                if (m_deviceSerialList.size() > 1) {
                    qInfo() << "There are currently multiple wired devices!";
                    if (m_messageTcpSocket != nullptr) {
                        QString serials = "";
                        for (auto &item : m_deviceSerialList) {
                            if (item.contains(":5555")) {
                                serials = item;
                                break;
                            }
                        }
                        if (!serials.isEmpty()) {
                            qInfo() << "There are currently WiFi adb devices present!";
                            // 存在wifiadb
                            if (isSameDevice(serials)) {
                                // 同一设备
                                m_isSupportADB = true;
                                m_androidConnType = AndroidConnType::WIFIADB;
                            } else {
                                m_isSupportADB = false;
                                m_androidConnType = AndroidConnType::CONNECTWIFI;
                            }
                        } else {
                            // 已存在连接，说明为WiFi连接
                            qInfo()
                                << "Currently, there are multiple wired devices, and WiFi anti control is not supported "
                                   "temporarily!";
                            m_androidConnType = AndroidConnType::CONNECTWIFI;
                        }
                        handleConnected();
                    } else {
                        // 不存在连接，当前有多个adb设备连接，暂时不处理
                        Q_EMIT sigDeviceRedundancy();
                    }
                    break;
                }
                if (!m_deviceSerialList.isEmpty()) {
                    if (m_messageTcpSocket != nullptr) {
                        // 当前为无线连接
                        // 判断当前有线设备和扫码设备是否是同一设备 uuid是否一致
                        if (isSameDevice(m_deviceSerialList.at(0))) {
                            // 同一设备
                            m_isSupportADB = true;
                            m_androidConnType = AndroidConnType::WIFIADB;
                        } else {
                            m_isSupportADB = false;
                            m_androidConnType = AndroidConnType::CONNECTWIFI;
                        }
                        handleConnected();
                    } else {
                        // 当前为有线设备连接
                        m_connectInfo.connectType = ConnectType::USB;
                        m_isSupportADB = true;
                        m_adb->reverse(m_deviceSerialList.at(0), SOCKET_NAME, LISTEN_PORT);
                        GenerateTools::delayMs(500);
                        m_adb->reverse(m_deviceSerialList.at(0), FTPSERVERPORT, FTPSERVERPORT);
                        GenerateTools::delayMs(500);
                        checkApk(m_deviceSerialList.at(0));
                    }
                } else {
                    // 当前没有usb连接
                    if (m_messageTcpSocket != nullptr) {
                        // 当前为无线连接
                        handleWifiConnection();
                    } else {
                        // 当前没有设备连接
                        qInfo() << "There are currently no wired devices!";
                        Q_EMIT sigNoUsbDevice();
                    }
                }
            }
            if (args.contains("com.kylin.mobileassistant")) {
                // 当前为调起pp命令
                QString checkApkOut = m_adb->getStdOut();
                if (!checkApkOut.isEmpty()) {
                    setMapPort();
                    startApp(m_deviceSerialList.at(0));
                }
            }
            if (args.contains("connect")) {
                // 当前为adbconnect命令
                QString connectResult = m_adb->getStdOut();
                QString testResult = "connected to " + m_connectInfo.address + ":5555";
                if (connectResult.contains(testResult)) {
                    updateDevice();
                } else {
                    // 当前没有usb连接
                    if (m_messageTcpSocket != nullptr) {
                        // 当前为无线连接
                        handleWifiConnection();
                    } else {
                        // 当前没有设备连接
                        qInfo() << "There are currently no wired devices!";
                    }
                }
            }
            break;
        }
    }
}

void ConnectionService::setUserPassword(QString userName, QString password)
{
    m_userName = userName;
    m_password = password;
}

void ConnectionService::deleteSocket()
{
    m_isMsgSocketConnect = false;
    m_isSupportADB = false;
    m_isClient = false;
    m_isHead = true;
    m_isAgainSendHeartbeat = true;
    m_isRemoteDisconnect = false;
    m_dataType = DataType::FREE;
    clearConnectionInfo();
    if (m_timer != nullptr) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (m_messageTcpSocket != nullptr) {
        m_messageTcpSocket->deleteLater();
        m_messageTcpSocket = nullptr;
    }
    if (m_dataTcpSocket != nullptr) {
        m_dataTcpSocket->deleteLater();
        m_dataTcpSocket = nullptr;
    }
    if (m_heartTcpServer != nullptr) {
        m_heartTcpServer->close();
        m_heartTcpServer->deleteLater();
        m_heartTcpServer = nullptr;
    }
    if (m_heartbeatSocket != nullptr) {
        m_heartbeatSocket->deleteLater();
        m_heartbeatSocket = nullptr;
    }
    if (m_adb != nullptr && m_androidConnType != AndroidConnType::NOTANDROID) {
        // 断开连接时处理adb连接
        if (m_androidConnType == AndroidConnType::ADB) {
            // 当前未有线连接，需要开通手机端口，实现wifi反控
            m_adb->startAdbTcpip(m_deviceSerialList.value(0));
        } else {
            // 当前为wifi反控，断开adb连接
            m_adb->disconnectAdbd("");
        }
        m_androidConnType = AndroidConnType::NOTANDROID;
    }
}

bool ConnectionService::sendMessage(const ReplyMessage &message)
{
    if (m_messageTcpSocket == nullptr) {
        qWarning() << "Message channel disconnected!";
        return -1;
    }
    std::string str = "";
    message.SerializeToString(&str);
    long long msgSize = str.length();
    char buf[17] = {'\0'};
    sprintf(buf, "%016lld", msgSize);
    std::string sizeStr(buf);
    std::string msg = sizeStr + str;
    int send = -1;
    send = m_messageTcpSocket->write(msg.c_str(), msg.length());
    m_messageTcpSocket->flush();
    bool sendRe = true;
    if (send == -1) {
        sendRe = false;
    }
    if (!sendRe) {
        qInfo() << "Message sending failed!";
    } else {
        qInfo() << "Message sent successfully!";
    }
    return sendRe;
}

bool ConnectionService::sendBusyMessage()
{
    qInfo() << "The client has transmitted new data, but is currently busy!";
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::BUSY);
    bool sendRe = sendMessage(replyMessage);
    return sendRe;
}

void ConnectionService::openWriteFile()
{
#ifdef DEBUG
    qDebug() << "write file " << QString::fromStdString(m_filesInfo.file_info(m_currentFileIndex).name())
             << m_filesInfo.file_info(m_currentFileIndex).size();
#endif  // !DEBUG
    m_writeSize = 0;
    m_writeFile = new QFile(m_fileSavePath + QString::fromStdString(m_filesInfo.file_info(m_currentFileIndex).name()));
    bool openFlag = m_writeFile->open(QIODevice::WriteOnly);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file:" + QString::fromStdString(m_filesInfo.file_info(m_currentFileIndex).name());
        delete m_writeFile;
        m_writeFile = nullptr;
        return;
    }
}

void ConnectionService::endFile()
{
    m_currentFileIndex++;
    m_writeFile->flush();
    m_writeFile->close();
    delete m_writeFile;
    m_writeFile = nullptr;
    if (m_currentFileIndex >= m_filesInfo.file_info_size()) {
        qInfo() << "File transfer finish!";
        Q_EMIT sigFileReceived(m_fileSavePath);
        m_data.clear();
        m_dataType = DataType::FREE;
        m_currentFileIndex = 0;
        m_filesInfo.clear_file_info();
        m_filesInfo.Clear();
        return;
    } else {
        openWriteFile();
    }
}

bool ConnectionService::sendStartAudioMessage()
{
    qInfo() << "Notify the Android terminal to start the sound service!";
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::STARTAUDIOSERVER);
    switch (m_androidConnType) {
        case AndroidConnType::ADB:
            replyMessage.set_conn_type(ReplyMessage::ADB);
            break;
        case AndroidConnType::CONNECTWIFI:
            replyMessage.set_conn_type(ReplyMessage::WIFI);
            break;
        case AndroidConnType::WIFIADB:
            replyMessage.set_conn_type(ReplyMessage::WIFIADB);
            break;
    }
    bool sendRe = sendMessage(replyMessage);
    return sendRe;
}

void ConnectionService::sendDeviceInfo()
{
    ReplyMessage replyMessage;
    replyMessage.set_status(ReplyMessage::DEVICEINFO);
    switch (m_androidConnType) {
        case AndroidConnType::ADB:
            replyMessage.set_conn_type(ReplyMessage::ADB);
            break;
        case AndroidConnType::CONNECTWIFI:
            replyMessage.set_conn_type(ReplyMessage::WIFI);
            break;
        case AndroidConnType::WIFIADB:
            replyMessage.set_conn_type(ReplyMessage::WIFIADB);
            break;
        case AndroidConnType::NOTANDROID:
            // 不是安卓设备默认为WIFI连接
            replyMessage.set_conn_type(ReplyMessage::WIFI);
            break;
    }
    DeviceInfo *deviceInfo = replyMessage.mutable_device_info();
    deviceInfo->set_uuid(GenerateTools::getUuid());
    deviceInfo->set_device_name(getenv("LOGNAME"));
    deviceInfo->set_device_type(DeviceInfo::PC);
    if (m_isClient) {
        deviceInfo->set_device_role(DeviceInfo::INITIATOR);
    } else {
        deviceInfo->set_device_role(DeviceInfo::RECIPIENT);
    }
    FtpUserPwd *userpwd = deviceInfo->mutable_ftp_userpwd();
    userpwd->set_username(m_userName.toStdString().c_str());
    userpwd->set_pwd(m_password.toStdString().c_str());
    sendMessage(replyMessage);
}

void ConnectionService::receiveDeviceInfo(const ReplyMessage &reply)
{
    DeviceInfo deviceInfo = reply.device_info();
    FtpUserPwd userpwd = deviceInfo.ftp_userpwd();
    m_connectInfo.address = getSocketAddress();
    m_connectInfo.uuid = QString::fromStdString(deviceInfo.uuid());
    m_connectInfo.deviceName = QString::fromStdString(deviceInfo.device_name());
    m_connectInfo.usernamePwd.username = QString::fromStdString(userpwd.username());
    m_connectInfo.usernamePwd.pwd = QString::fromStdString(userpwd.pwd());
    switch (deviceInfo.device_role()) {
        case DeviceInfo::INITIATOR:
            m_connectInfo.deviceRole = DeviceRole::INITIATOR;
            break;
        case DeviceInfo::RECIPIENT:
            m_connectInfo.deviceRole = DeviceRole::RECIPIENT;
            break;
    }
    switch (deviceInfo.device_type()) {
        case DeviceInfo::PC:
            m_connectInfo.deviceType = DeviceType::PC;
            break;
        case DeviceInfo::ANDROID:
            m_connectInfo.deviceType = DeviceType::ANDROID;
            break;
        default:
            qWarning() << "";
            m_connectInfo.deviceType = DeviceType::UNKNOWN;
            break;
    }
    if (!m_isClient) {
        // 服务端, 收到客户端请求，发送信号，让使用者验证
        Q_EMIT sigConnectRequest(m_connectInfo.deviceName);
    } else {
        connectDataSocket();
        // 客户端, 服务端同意请求，通知使用者
        Q_EMIT sigConnectInfo(m_connectInfo);
    }
}

void ConnectionService::connectDataSocket()
{
    // 客户端建立数据通道
    if (m_dataTcpSocket == nullptr) {
        m_dataTcpSocket = new QTcpSocket;
        connect(m_dataTcpSocket, &QTcpSocket::readyRead, this, &ConnectionService::onDataReadyRead);
    }
    m_dataTcpSocket->connectToHost(m_serverIp, LISTEN_PORT);
}

void ConnectionService::startHeartbeat()
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateSocketState()));
    m_timer->start(HEART_RATE);
}

void ConnectionService::connectHeartbeat()
{
    qInfo() << "Connect heartbeat socket...";
    if (m_heartbeatSocket == nullptr) {
        m_heartbeatSocket = new QTcpSocket;
        connect(m_heartbeatSocket, &QTcpSocket::readyRead, this, &ConnectionService::onHeartbeatReadyRead);
        connect(m_heartbeatSocket, &QTcpSocket::connected, this, [=]() {
            qInfo() << "Heartbeat socket connected.";
            sendHeartbeat(true);
            startHeartbeat();
        });
    }
    QString address = getSocketAddress();
    m_heartbeatSocket->connectToHost(address, HEARTBEAT_PORT);
}

QString ConnectionService::getSocketAddress()
{
    if (m_messageTcpSocket != nullptr) {
        quint32 addrOrigin = m_messageTcpSocket->peerAddress().toIPv4Address();
        QHostAddress addrHost = QHostAddress(addrOrigin);
        qInfo() << "The current connection IP is: " << addrHost.toString();
        return addrHost.toString();
    } else {
        qWarning() << "Connection exception currently occurred!";
        return "";
    }
}

void ConnectionService::sendHeartbeat(bool isServer)
{
    ReplyMessage replyMessage;
    std::string str = "";
    if (isServer) {
#ifdef DEBUG
        qDebug() << "URGENTSERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
#endif  // !DEBUG
        replyMessage.set_status(ReplyMessage::URGENTSERVER);
        replyMessage.SerializeToString(&str);
    } else {
#ifdef DEBUG
        qDebug() << "URGENTCLIENT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
#endif  // !DEBUG
        replyMessage.set_status(ReplyMessage::URGENTCLIENT);
        replyMessage.SerializeToString(&str);
    }
    if (m_heartbeatSocket != nullptr) {
        m_heartbeatSocket->write(str.c_str(), str.length());
        m_heartbeatSocket->flush();
    }
}

void ConnectionService::updateSocketState()
{
    // 定时检查更新设备的网络状态
    if (!m_isSurvival) {
        // 是否收到心跳回复
        if (m_isAgainSendHeartbeat) {
            // 第一次未收到重发一次
            m_isAgainSendHeartbeat = false;
            qInfo() << "No heartbeat was received, and the resend";
            // 重发一次
            if (m_heartbeatSocket != nullptr) {
                sendHeartbeat(!m_isClient);
            }
            return;
        }
        qInfo() << "Listen to socket disconnection.";
        disconnectSocket();
        return;
    }

    m_isSurvival = false;

    if (m_heartbeatSocket != nullptr) {
        sendHeartbeat(!m_isClient);
    }
}

void ConnectionService::slotHeartbeatConnection()
{
    qInfo() << "The heartbeat channel is already connected";
    m_heartbeatSocket = m_heartTcpServer->nextPendingConnection();
    connect(m_heartbeatSocket, &QTcpSocket::readyRead, this, &ConnectionService::onHeartbeatReadyRead);
    sendHeartbeat(false);
    startHeartbeat();
}

void ConnectionService::clearConnectionInfo()
{
    m_connectInfo.uuid.clear();
    m_connectInfo.deviceName.clear();
    m_connectInfo.address.clear();
    m_connectInfo.deviceType = DeviceType::UNKNOWN;
    m_connectInfo.connectType = ConnectType::NOTCONNECT;
    m_connectInfo.usernamePwd.username.clear();
    m_connectInfo.usernamePwd.pwd.clear();
    if (QFileInfo().exists(DEVICEID_FILE_LOACLPATH)) {
        QFile::remove(DEVICEID_FILE_LOACLPATH);
    }
}

void ConnectionService::updateDevice()
{
    if (m_adb->isRuning()) {
        qInfo() << "adb is running";
        return;
    }
    qInfo() << "update devices...";
    m_adb->execute("", QStringList() << "devices");
}

void ConnectionService::checkApk(QString serials)
{
    qInfo() << "check apk";
    if (m_adb->isRuning()) {
        qInfo() << "adb is running";
        return;
    }
    QStringList adbArgs;
    adbArgs << "shell";
    adbArgs << "pm";
    adbArgs << "list";
    adbArgs << "packages";
    adbArgs << "|";
    adbArgs << "grep";
    adbArgs << "com.kylin.mobileassistant";
    m_adb->execute(serials, adbArgs);
}

void ConnectionService::setMapPort()
{
    m_adb->portForward("", FTPPORT, FTPPORT);
    GenerateTools::delayMs(50);
    m_adb->portForward("", FORWARD_PORT, FORWARD_PORT);
    GenerateTools::delayMs(50);
    m_adb->portForward("", FORWARD_PORT1, FORWARD_PORT1);
    GenerateTools::delayMs(50);
}

void ConnectionService::startApp(QString serials)
{
    qInfo() << "start android apk";
    if (m_adb->isRuning()) {
        qInfo() << "adb is running";
        return;
    }
    QStringList adbArgs;
    adbArgs << "shell"
            << "am"
            << "start"
            << "-n"
            << "com.kylin.mobileassistant/.ui.activity.remote.HomepageActivity"
            << "-f"
            << "0x04000000"
            << "--es"
            << "type"
            << "USB"
            << "--es"
            << "version" << qApp->applicationVersion();
    m_adb->execute(serials, adbArgs);
}

bool ConnectionService::isSameDevice(QString serials)
{
    // 获取手机设备UUID
    m_adb->pull(serials, UUID_FILE_REMOTEPATH, UUID_FILE_LOACLPATH);
    if (m_adb->getStdOut().contains("error")) {
        qInfo() << "The device did not generate a UUID file!";
        return false;
    }
    QFileInfo uuidFileInfo;
    int i = 0;
    while (!uuidFileInfo.exists(UUID_FILE_LOACLPATH)) {
        if (i >= 9) {
            qCritical() << "The UUID file downloaded by ADB does not exist!";
            return false;
        }
        QThread::msleep(50);
        i++;
    }
    QFile *file = new QFile(UUID_FILE_LOACLPATH);
    bool openFlag = file->open(QIODevice::ReadOnly);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file:" + UUID_FILE_LOACLPATH;
        delete file;
        file = nullptr;
        return false;
    }
    QString uuid = file->readAll();
    uuid.remove('\n');
    if (m_connectInfo.uuid == uuid) {
        qInfo() << "The uid is the same as that of the wireless device.";
        delete file;
        file = nullptr;
        return true;
    } else {
        qInfo() << "The UUID of wired connection device is different from that of wireless device.";
        delete file;
        file = nullptr;
        return false;
    }
}

void ConnectionService::handleWifiConnection()
{
    if (m_connectInfo.connectType == ConnectType::WIFI) {
        qInfo() << "Currently, there is no wired device, and WiFi anti control is not supported temporarily!";
        m_isSupportADB = false;
        m_androidConnType = AndroidConnType::CONNECTWIFI;
        handleConnected();
    }
}

void ConnectionService::handleConnected()
{
    sendDeviceInfo();
    Q_EMIT sigConnectInfo(m_connectInfo);
    connectHeartbeat();
}
