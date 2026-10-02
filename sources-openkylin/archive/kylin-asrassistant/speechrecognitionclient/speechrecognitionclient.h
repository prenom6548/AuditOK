#ifndef SPEECHRECOGNITIONCLIENT_H
#define SPEECHRECOGNITIONCLIENT_H

#include <QObject>
#include <QString>
#include <QtWebSockets/QWebSocket>

class SpeechRecognitionClient : public QObject
{
    Q_OBJECT
public:
    enum ServerState {
        CONNECTED = 0,
        UNCONNECT,
    };

    SpeechRecognitionClient(QObject *parent = nullptr);
    ~SpeechRecognitionClient();

    bool senMsg(const QByteArray &msg);
    void start();
    void stop();

public Q_SLOTS:
    void onStateChanged(QAbstractSocket::SocketState state);
    void onRecvMsg(const QString &msg);

Q_SIGNALS:
    void sigServerStatus(ServerState state);
    void sigNewMsg(const QString &msg);

private:
    bool sendTextMsg(const QString &msg);

private:
    enum MsgType {
        START = 0,
        DATA,
        END,
    };

private:
    QWebSocket *m_clientSocket;
    MsgType m_msgType = MsgType::START;
};

#endif // SPEECHRECOGNITIONCLIENT_H
