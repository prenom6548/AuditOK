#include "baseserver.h"
#include <QTimer>
#include <QTimerEvent>
#include <inputchinese.pb.h>

const int DEVICE_NAME_FIELD_LENGTH = 64;

BaseServer::BaseServer(QObject *parent) : QObject(parent)
{
    connect(&m_serverSocket, &TcpServer::sigSocketDisconnect, this, &BaseServer::sigSocketDisconnect);
}

BaseServer::~BaseServer()
{
}

void BaseServer::onInputChinese(const QString &input)
{
    if (m_inputSocket && m_inputSocket->isValid()) {
        qInfo() << "input" << input;
        std::string inputData = input.toStdString();
        std::string data = "";
        ChineseInputMessage message;
        message.set_status(ChineseInputMessage_StatusType_CONNECTED);
        message.set_committext(inputData);
        message.SerializeToString(&data);
        int ret = m_inputSocket->write(data.c_str(), data.length());
        if (ret == -1) {
            qWarning() << "Chinese Input Fail";
        }
    }
}

bool BaseServer::isReverse()
{
    return !m_tunnelForward;
}

BaseServer::ServerParams BaseServer::getParams()
{
    return m_params;
}

bool BaseServer::start(BaseServer::ServerParams params)
{
    return true;
}

bool BaseServer::connectTo()
{
    return true;
}

void BaseServer::onConnectTimer()
{
}

void BaseServer::timerEvent(QTimerEvent *event)
{
    if (event && m_acceptTimeoutTimer == event->timerId()) {
        stopAcceptTimeoutTimer();
        Q_EMIT connectToResult(false, "", QSize(), "");
    } else if (event && m_connectTimeoutTimer == event->timerId()) {
        onConnectTimer();
    }
}

VideoSocket *BaseServer::getVideoSocket()
{
    return m_videoSocket;
}

QTcpSocket *BaseServer::getControlSocket()
{
    return m_controlSocket;
}

InputSocket *BaseServer::getInputSocket()
{
    return m_inputSocket;
}

bool BaseServer::readInfo(VideoSocket *videoSocket, QString &deviceName, QSize &size)
{
    unsigned char buf[DEVICE_NAME_FIELD_LENGTH + 4];
    if (videoSocket->bytesAvailable() <= (DEVICE_NAME_FIELD_LENGTH + 4)) {
        videoSocket->waitForReadyRead(300);
    }

    qint64 len = videoSocket->read((char *)buf, sizeof(buf));
    if (len < DEVICE_NAME_FIELD_LENGTH + 4) {
        qInfo("Could not retrieve device information");
        return false;
    }
    buf[DEVICE_NAME_FIELD_LENGTH - 1] = '\0';  // in case the client sends garbage
    // strcpy is safe here, since name contains at least DEVICE_NAME_FIELD_LENGTH bytes
    // and strlen(buf) < DEVICE_NAME_FIELD_LENGTH
    deviceName = (char *)buf;
    size.setWidth((buf[DEVICE_NAME_FIELD_LENGTH] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 1]);
    size.setHeight((buf[DEVICE_NAME_FIELD_LENGTH + 2] << 8) | buf[DEVICE_NAME_FIELD_LENGTH + 3]);
    return true;
}

void BaseServer::startAcceptTimeoutTimer()
{
    stopAcceptTimeoutTimer();
    m_acceptTimeoutTimer = startTimer(1000);
}

void BaseServer::stopAcceptTimeoutTimer()
{
    if (m_acceptTimeoutTimer) {
        killTimer(m_acceptTimeoutTimer);
        m_acceptTimeoutTimer = 0;
    }
}

void BaseServer::startConnectTimeoutTimer()
{
    stopConnectTimeoutTimer();
    m_connectTimeoutTimer = startTimer(100);
}

void BaseServer::stopConnectTimeoutTimer()
{
    if (m_connectTimeoutTimer) {
        killTimer(m_connectTimeoutTimer);
        m_connectTimeoutTimer = 0;
    }
    m_connectCount = 0;
}
