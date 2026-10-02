#include "pcscreenmanage.h"
#include <QByteArray>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QFileInfo>

// #define KYLIN_CONNECTIVITY_TOOLS_PATH "/"
// #define KYLIN_CONNECTIVITY_TOOLS_SERVICE "com.kylinos.connectivity.tools"
// #define KYLIN_CONNECTIVITY_TOOLS_INTERFACE "com.kylinos.connectivity.tools"

const int LISTEN_PORT = 27192;
QString LOAD_URL = "http://%1:27190/%2";

PcScreenManage::PcScreenManage(QObject *parent) : QObject(parent)
{
    m_tcpServer = new QTcpServer;
    connect(m_tcpServer, &QTcpServer::newConnection, this, &PcScreenManage::slotServerNewConnection);

    int sendRe = m_tcpServer->listen(QHostAddress::AnyIPv4, LISTEN_PORT);
    if (sendRe == -1) {
        qInfo() << "PcScreenManage listening port " + QString::number(LISTEN_PORT) + " failed!";
    } else {
        qInfo() << "PcScreenManage listening port " + QString::number(LISTEN_PORT) + " succeeded!";
    }
}

PcScreenManage::~PcScreenManage()
{
    disconnected();
    if (m_tcpServer != nullptr) {
        m_tcpServer->close();
        m_tcpServer->deleteLater();
        m_tcpServer = nullptr;
    }
}

void PcScreenManage::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    if (m_curtain != nullptr) {
        m_curtain->setTheme(m_theme);
    }
}

bool PcScreenManage::isSupport()
{
    QFileInfo screencast("/usr/bin/kylin-screencast.AppImage");
    QFileInfo controlClient("/usr/bin/anti-control-client");
    QFileInfo controlServer("/usr/bin/anti-control-server");

    return screencast.isExecutable() && controlClient.isExecutable() && controlServer.isExecutable();
}

void PcScreenManage::disconnected()
{
    m_isService = true;
    closeControl();
    closeCurtain();
    closePcScreenService();
    if (m_messageTcpSocket != nullptr) {
        m_messageTcpSocket->deleteLater();
        m_messageTcpSocket = nullptr;
    }
}

void PcScreenManage::connectService(const QString host, PcScreenManage::DeviceType deviceType)
{
    m_isService = false;
    m_deviceType = deviceType;
    if (m_messageTcpSocket == nullptr) {
        m_messageTcpSocket = new QTcpSocket;
        connect(m_messageTcpSocket, &QTcpSocket::stateChanged, this, &PcScreenManage::handleStateChange);
        connect(m_messageTcpSocket, &QTcpSocket::readyRead, this, &PcScreenManage::onMessageReadyRead);
        connect(m_messageTcpSocket, &QTcpSocket::connected, this, [=]() {
            quint32 addrOrigin = m_messageTcpSocket->peerAddress().toIPv4Address();
            QHostAddress addrHost = QHostAddress(addrOrigin);
            m_address = addrHost.toString();
        });
        connect(m_messageTcpSocket, &QTcpSocket::disconnected, this, &PcScreenManage::slotSocketDisconnect);
    }
    m_messageTcpSocket->connectToHost(host, LISTEN_PORT);
}

void PcScreenManage::setConnectionRespond(bool isAgree)
{
    if (isAgree) {
        quint32 addrOrigin = m_messageTcpSocket->peerAddress().toIPv4Address();
        QHostAddress addrHost = QHostAddress(addrOrigin);
        m_address = addrHost.toString();
        startControl();
        qInfo() << "Remote host " + m_address + " connected!";
    } else {
        // 拒绝
        qInfo() << "The current connection request has been rejected!";
        ScreenManageMsg message;
        message.set_msg(ScreenManageMsg::REFUSE);
        sendMessage(message);
        disconnected();
    }
}

void PcScreenManage::startPcService()
{
    qInfo() << "Getting opposite device screen...";
    if (m_pcScreen == nullptr) {
        m_pcScreen = new PCScreen;
        connect(m_pcScreen, &PCScreen::sigStartedScreen, this, [=](QString roomid) {
            ScreenManageMsg message;
            message.set_msg(ScreenManageMsg::SCREENREADY);
            message.set_roomid(roomid.toStdString().c_str());
            sendMessage(message);
            Q_EMIT sigRequestReceived(ScreenMsg::StartedScreen);
        });
        connect(m_pcScreen, &PCScreen::sigStartError, this, [=]() {
            Q_EMIT sigRequestReceived(ScreenMsg::ScreenError);
            disconnected();
        });
        m_pcScreenThread = new QThread;
        m_pcScreen->moveToThread(m_pcScreenThread);
        connect(m_pcScreenThread, &QThread::started, m_pcScreen, &PCScreen::startService);
        m_pcScreenThread->start();
    }
}

void PcScreenManage::startCurtain(QString roomid)
{
    // 服务端
    // m_dbusInterface = new QDBusInterface(KYLIN_CONNECTIVITY_TOOLS_SERVICE, KYLIN_CONNECTIVITY_TOOLS_PATH,
    //                                      KYLIN_CONNECTIVITY_TOOLS_INTERFACE, QDBusConnection::sessionBus());
    // QObject::connect(m_dbusInterface, SIGNAL(enterClient(bool)), this, SLOT(slotEnterControl(bool)));

    if (m_curtain == nullptr) {
        m_curtain = new Curtain;
        connect(m_curtain, &Curtain::sigStateChange, this, &PcScreenManage::slotStateChange);
    }
    QString targetUrl;
    if (!roomid.isEmpty()) {
        targetUrl = LOAD_URL.arg(m_address).arg(roomid);
        qInfo() << "targetUrl = " << targetUrl;
    }
    m_curtain->showFullScreen();
    m_curtain->showMaximized();
    m_curtain->loadUrl(targetUrl);
    m_curtain->setTheme(m_theme);
}

void PcScreenManage::startControl()
{
    m_control = new Control(this);
    connect(m_control, &Control::sigControlState, this, [=](bool state) {
        if (state && !m_isAtiveStarted) {
            ScreenManageMsg message;
            message.set_msg(ScreenManageMsg::STARTEDCONTROL);
            sendMessage(message);
            Q_EMIT sigRequestReceived(ScreenMsg::PassiveControl);
            showView();
            m_curtain->hideTabBar();
        } else if (!state && !m_isAtiveEnd) {
            ScreenManageMsg message;
            message.set_msg(ScreenManageMsg::EXITCONTROL);
            sendMessage(message);
            showView();
            m_curtain->showTabBar();
        }
    });
    connect(m_control, &Control::sigServerState, this, [=](bool isSuccess) {
        if (isSuccess) {
            ScreenManageMsg message;
            message.set_msg(ScreenManageMsg::CURTAINREADY);
            sendMessage(message);
        } else {
            Q_EMIT sigRequestReceived(ScreenMsg::ScreenError);
            disconnected();
        }
    });
    connect(m_control, &Control::sigClientState, this, [=](bool isSuccess) {
        if (isSuccess) {
            startPcService();
        } else {
            Q_EMIT sigRequestReceived(ScreenMsg::ScreenError);
            disconnected();
        }
    });
    if (m_isService) {
        // 服务端
        m_control->startServer();
    } else {
        // 客户端
        m_control->startClient(m_address);
    }
}

void PcScreenManage::startedControl()
{
    qInfo() << "Enter anti control.";
    ScreenManageMsg message;
    message.set_msg(ScreenManageMsg::STARTEDCONTROL);
    sendMessage(message);
    showView();
    m_curtain->hideTabBar();
    m_isAtiveStarted = true;
    m_isAtiveEnd = false;
    QString common = "xdotool key ctrl+h";
    system(common.toUtf8().constData());
}

void PcScreenManage::exitControl()
{
    ScreenManageMsg message;
    message.set_msg(ScreenManageMsg::EXITCONTROL);
    sendMessage(message);
}

void PcScreenManage::showView()
{
    if (m_curtain != nullptr) {
        m_curtain->showView();
    }
}

void PcScreenManage::closePcScreenService()
{
    qInfo() << "Closing the projection service...";
    if (m_pcScreen != nullptr) {
        ScreenManageMsg message;
        message.set_msg(ScreenManageMsg::SCREENEXIT);
        sendMessage(message);
        m_messageTcpSocket->waitForBytesWritten();
        m_pcScreen->deleteLater();
        m_pcScreen = nullptr;
    }
    if (m_pcScreenThread != nullptr) {
        m_pcScreenThread->quit();
        m_pcScreenThread->wait();
        m_pcScreenThread->deleteLater();
        m_pcScreenThread = nullptr;
    }
}
void PcScreenManage::closeCurtain()
{
    qInfo() << "Closing curtain...";
    if (m_curtain != nullptr) {
        ScreenManageMsg message;
        message.set_msg(ScreenManageMsg::CURTAINEXIT);
        sendMessage(message);
        m_messageTcpSocket->waitForBytesWritten();
        m_curtain->hide();
        delete m_curtain;
        m_curtain = nullptr;
    }
}

void PcScreenManage::closeControl()
{
    m_isAtiveEnd = false;
    m_isAtiveStarted = false;
    if (m_control != nullptr) {
        delete m_control;
        m_control = nullptr;
    }
}

void PcScreenManage::slotServerNewConnection()
{
    if (!isSupport()) {
        Q_EMIT sigRequestReceived(ScreenMsg::ScreenError);
        disconnected();
        return;
    }
    qInfo() << "The projection service has a new connection.";
    if (m_messageTcpSocket != nullptr) {
        qWarning() << "The current screen projection service is already connected!";
        return;
    }
    m_messageTcpSocket = m_tcpServer->nextPendingConnection();
    connect(m_messageTcpSocket, &QTcpSocket::readyRead, this, &PcScreenManage::onMessageReadyRead);
    connect(m_messageTcpSocket, &QTcpSocket::disconnected, this, &PcScreenManage::slotSocketDisconnect);
    Q_EMIT sigRequestReceived(ScreenMsg::SharingRequest);
}

void PcScreenManage::onMessageReadyRead()
{
    if (m_messageTcpSocket == nullptr) {
        return;
    }
    // 读取Message
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

    ScreenManageMsg message;
    bool ok = message.ParseFromString(m_data.toStdString());
    if (!ok) {
        // protobuf反序列化错误
        qInfo() << "Message parsing error!";
    }

    switch (message.msg()) {
        case ScreenManageMsg::REFUSE: {
            qInfo() << "Remote rejection of screen projection request.";
            Q_EMIT sigRequestReceived(ScreenMsg::RemoteRejection);
        } break;
        case ScreenManageMsg::CURTAINREADY: {
            qInfo() << "Remote consent to screen projection request.";
            Q_EMIT sigRequestReceived(ScreenMsg::Successfully);
            if (m_deviceType == PcScreenManage::DeviceType::PC) {
                startControl();
            } else {
                startPcService();
            }
        } break;
        case ScreenManageMsg::SCREENREADY: {
            qInfo() << "Receive remote screen projection information.";
            Q_EMIT sigRequestReceived(ScreenMsg::StartedScreen);
            QString roomid = QString::fromStdString(message.roomid());
            startCurtain(roomid);
        } break;
        case ScreenManageMsg::CURTAINEXIT: {
            qInfo() << "The screen receiver has disconnected the screen projection.";
            Q_EMIT sigRequestReceived(ScreenMsg::ScreenExit);
            disconnected();
        } break;
        case ScreenManageMsg::SCREENEXIT: {
            qInfo() << "The screen sharing party quit the service.";
            Q_EMIT sigRequestReceived(ScreenMsg::ScreenExit);
            disconnected();
        } break;
        case ScreenManageMsg::STARTEDCONTROL: {
            Q_EMIT sigRequestReceived(ScreenMsg::StartedControl);
        } break;
        case ScreenManageMsg::EXITCONTROL: {
            qInfo() << "Exit anti control.";
            if (m_isService) {
                m_isAtiveEnd = true;
                m_isAtiveStarted = false;
                QString common = "xdotool key ctrl+g";
                system(common.toUtf8().constData());
                m_curtain->showTabBar();
            } else {
                Q_EMIT sigRequestReceived(ScreenMsg::ExitControl);
            }
        } break;
    }
}

void PcScreenManage::handleStateChange(QAbstractSocket::SocketState state)
{
    switch (state) {
        case QAbstractSocket::UnconnectedState:
            // 套接字未连接。
            qInfo() << "The socket is not connected.";
            break;
        case QAbstractSocket::HostLookupState:
            // 套接字正在查询主机。
            qInfo() << "The socket is performing a host name lookup.";
            break;
        case QAbstractSocket::ConnectingState:
            // 套接字开始建立连接。
            qInfo() << "The socket has started establishing a connection.";
            break;
        case QAbstractSocket::ConnectedState:
            // 新的连接已建立。
            qInfo() << "A connection is established.";
            break;
        case QAbstractSocket::BoundState:
            // 套接字已绑定到一个地址和端口。
            qInfo() << "The socket is bound to an address and port.";
            break;
        case QAbstractSocket::ClosingState:
            // 套接字即将关闭（数据可能仍在等待写入）。
            qInfo() << "The socket is about to close (data may still be waiting to be written).";
            break;
        case QAbstractSocket::ListeningState:
            // 套接字仅限内部使用。
            qInfo() << "For internal use only.";
            break;
    }
}

void PcScreenManage::slotSocketDisconnect()
{
    qInfo() << "PcScreenManage socket disconnect.";
    // Q_EMIT sigRequestReceived(ScreenMsg::Disconnect);
}

void PcScreenManage::slotStateChange(Curtain::States state)
{
    switch (state) {
        case Curtain::States::Control: {
            // 进入反控
            startedControl();
        } break;
        case Curtain::States::ReturnDesktop: {
            Q_EMIT sigRequestReceived(ScreenMsg::HideView);
        } break;
        case Curtain::States::ScreenShare: {
            Q_EMIT sigRequestReceived(ScreenMsg::ShowView);
        } break;
        case Curtain::States::Exit: {
            disconnected();
            Q_EMIT sigRequestReceived(ScreenMsg::Exit);
        } break;
    }
}

// void PcScreenManage::slotEnterControl(bool isControl)
// {
//     qInfo() << "Receive anti control message, current status: " << isControl;
//     if (isControl) {
//         m_curtain->showView();
//         m_curtain->hideTabBar();
//         startedControl();
//     } else {
//         exitControl();
//     }
// }

int PcScreenManage::sendMessage(const ScreenManageMsg &message)
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
    int sendRe = m_messageTcpSocket->write(msg.c_str(), msg.length());
    m_messageTcpSocket->flush();

    if (sendRe == -1) {
        qInfo() << "Message sending failed!";
    } else {
        qInfo() << "Message sent successfully!";
    }
    return sendRe;
}
