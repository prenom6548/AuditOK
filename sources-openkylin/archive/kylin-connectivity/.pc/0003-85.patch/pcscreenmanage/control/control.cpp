#include "control.h"

#include <QDir>
#include <QDebug>

const int CONTROL_PORT = 27191;
const QByteArray CONTROL_START = "control:start";
const QByteArray CONTROL_END = "control:end";

Control::Control(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started, this, [=]() {
        if (m_isServer) {
            qInfo() << "Anti control server started successfully!";
            m_server = new QLocalServer(this);
            connect(m_server, &QLocalServer::newConnection, this, &Control::slotNewConnected);
            // 设置服务器名字
            bool flag = m_server->listen("/tmp/anti_control_server.socket");
            if (!flag) {
                qWarning() << "anti_control_server start listen error!";
            }
            Q_EMIT sigServerState(true);
        } else {
            qInfo() << "Anti control client started successfully!";
            Q_EMIT sigClientState(true);
        }
    });
    // connect(m_process, &QProcess::errorOccurred, [&](QProcess::ProcessError error) {
    //     if (m_isServer) {
    //         qInfo() << "Anti control server startup error: " << error;
    //         Q_EMIT sigServerState(false);
    //     } else {
    //         qInfo() << "Anti control client startup error: " << error;
    //         Q_EMIT sigClientState(false);
    //     }
    // });
}

Control::~Control()
{
    if (m_process != nullptr) {
        if (m_process->state() == QProcess::Running) {
            m_process->close();
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void Control::startServer()
{
    m_isServer = true;
    QString serverpara = "anti-control-server -f --debug DEBUG --name server --disable-crypto --log " +
                         QDir::homePath() +
                         "/.log/kylin-connectivity-control.log --disable-client-cert-checking"
                         " -c " +
                         "/usr/share/kylin-connectivity/control.conf --address :" + QString::number(CONTROL_PORT);
    qInfo() << "Anti control server starting...";
    m_process->start(serverpara);
}

void Control::startClient(QString address)
{
    m_isServer = false;
    QString clientpara = "anti-control-client -f --debug DEBUG --name client --disable-crypto --log " +
                         QDir::homePath() +
                         "/.log/kylin-connectivity-control.log"
                         " [" +
                         address + "]:" + QString::number(CONTROL_PORT);
    qInfo() << "Anti control client starting..";
    m_process->start(clientpara);
}

void Control::slotNewConnected()
{
    qInfo() << "anti_control_server new connection";
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QLocalSocket::readyRead, this, &Control::slotReadyRead);
}

void Control::slotReadyRead()
{
    if (m_socket) {
        // 处理接收到的数据
        QByteArray data = m_socket->readAll();
        qInfo() << "anti_control_server revc msg: " << data;
        if (data.contains(CONTROL_START)) {
            Q_EMIT sigControlState(true);
        }
        if (data.contains(CONTROL_END)) {
            Q_EMIT sigControlState(false);
        }
    }
}