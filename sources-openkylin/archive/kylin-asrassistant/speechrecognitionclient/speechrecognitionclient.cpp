#include "speechrecognitionclient.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDebug>

const QString URL_FILE_PATH = "/usr/share/kylin-asrassistant/speechrecognitionusr.conf";

SpeechRecognitionClient::SpeechRecognitionClient(QObject *parent) : QObject(parent)
{
    m_clientSocket = new QWebSocket;
    connect(m_clientSocket, &QWebSocket::stateChanged, this, &SpeechRecognitionClient::onStateChanged);
    connect(m_clientSocket, &QWebSocket::textMessageReceived, this, &SpeechRecognitionClient::onRecvMsg);
}

SpeechRecognitionClient::~SpeechRecognitionClient()
{
    if (m_clientSocket != nullptr) {
        m_clientSocket->close();
        delete m_clientSocket;
    }
}

void SpeechRecognitionClient::start()
{
    qInfo() << "client start ...";
    QFile *file = new QFile(URL_FILE_PATH);
    if (!file->open(QIODevice::ReadOnly)) {
        // 打开文件失败
        qCritical() << "fail to open file " << URL_FILE_PATH;
        delete file;
        return;
    }
    QByteArray data = file->readAll();
    if (data.isEmpty()) {
        qCritical() << "Error in getting the server address!";
        return;
    }
    delete file;
    m_clientSocket->open(QUrl(QString(data)));
}

bool SpeechRecognitionClient::senMsg(const QByteArray &msg)
{
    if (m_clientSocket->sendBinaryMessage(msg)) {
        return true;
    } else {
        return false;
    }
}

void SpeechRecognitionClient::stop()
{
    qInfo() << "client stop ...";
    QJsonObject jsonObj;
    jsonObj.insert("name", "test.wav");
    jsonObj.insert("signal", "end");
    jsonObj.insert("nbest", 5);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray msg = jsonDoc.toJson();
    m_msgType = MsgType::END;
    sendTextMsg(msg);
}

void SpeechRecognitionClient::onStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
        case QAbstractSocket::UnconnectedState: {
            if (m_msgType != MsgType::END) {
                qInfo() << "Unconnecte server";
                Q_EMIT sigServerStatus(ServerState::UNCONNECT);
            }
        } break;
        case QAbstractSocket::ConnectedState: {
            QJsonObject jsonObj;
            jsonObj.insert("name", "test.wav");
            jsonObj.insert("signal", "start");
            jsonObj.insert("nbest", 1);
            QJsonDocument jsonDoc(jsonObj);
            QByteArray msg = jsonDoc.toJson();
            m_msgType = MsgType::START;
            sendTextMsg(msg);
            qInfo() << "connection open";
        } break;
        case QAbstractSocket::HostLookupState: {
            qInfo() << "Looking for host ...";
        } break;
        case QAbstractSocket::ConnectingState: {
            qInfo() << "The connection has been established";
        } break;
        case QAbstractSocket::BoundState: {
            qInfo() << "Bbound to address and port";
        } break;
        case QAbstractSocket::ClosingState: {
            qInfo() << "Is about to close";
        } break;
        default: {
            qInfo() << "Unknown error";
        } break;
    }
}

void SpeechRecognitionClient::onRecvMsg(const QString &msg)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(msg.toUtf8());
    if (!jsonDoc.isObject()) {
        qCritical() << "Error in message format!";
        return;
    }
    QJsonObject jsonObj = jsonDoc.object();  //得到Json对象
    switch (m_msgType) {
        case MsgType::START: {
            QString res = jsonObj.value("status").toString();
            qInfo() << "server status:" << res;
            if (res == "ok") {
                m_msgType = MsgType::DATA;
                Q_EMIT sigServerStatus(ServerState::CONNECTED);
            }
        } break;
        case MsgType::DATA: {
            Q_EMIT sigNewMsg(jsonObj.value("result").toString());
        } break;
        case MsgType::END: {
            m_clientSocket->close();
            // Q_EMIT sigNewMsg(jsonObj.value("result").toString());
            // todo
        } break;
        default:
            break;
    }
}

bool SpeechRecognitionClient::sendTextMsg(const QString &msg)
{
    if (m_clientSocket->sendTextMessage(msg)) {
        qInfo() << "The message was sent successfully!";
        return true;
    } else {
        qCritical() << "Message sent failed!";
        return false;
    }
}
