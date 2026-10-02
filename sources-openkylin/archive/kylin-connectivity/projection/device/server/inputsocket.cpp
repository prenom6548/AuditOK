//
// Created by sqp on 2022/5/31.
//

#include "inputsocket.h"
#include "inputchinese.pb.h"
InputSocket::InputSocket(QObject *parent) : QTcpSocket(parent)
{
    connect(this, &InputSocket::readyRead, this, &InputSocket::onReadyRead);
    connect(this, &InputSocket::disconnected, this, &InputSocket::sigLostInputState);
}

InputSocket::~InputSocket() noexcept {}

void InputSocket::onReadyRead()
{
    QByteArray data = this->readAll();
    ChineseInputMessage reply;
    reply.ParseFromString(data.toStdString());
    switch (reply.status()) {
    case ChineseInputMessage_StatusType_START: {
        qInfo() << "chinese input start";
        std::string x = reply.x();
        std::string y = reply.y();
        Q_EMIT sigGetInputState();
        Q_EMIT sigGetInputPos(atoi(x.c_str()), atoi(y.c_str()));
        break;
    }
    case ChineseInputMessage_StatusType_CONNECTED: {
        std::string x = reply.x();
        std::string y = reply.y();
        Q_EMIT sigGetInputPos(atoi(x.c_str()), atoi(y.c_str()));
        break;
    }
    case ChineseInputMessage_StatusType_END: {
        qInfo() << "chinese input end";
        Q_EMIT sigLostInputState();
        break;
    }
    }
}