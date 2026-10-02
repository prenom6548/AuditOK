#ifndef SERVERSTATUS_H
#define SERVERSTATUS_H

#include <QObject>
#include <QUrl>

class ServerStatus : public QObject
{
    Q_OBJECT
public:
    ServerStatus(QUrl url);

    void startServer();

private:
    bool isConnect();

private:
    QUrl m_url;
Q_SIGNALS:
    void sigServerState(bool);
};

#endif // SERVERSTATUS_H
