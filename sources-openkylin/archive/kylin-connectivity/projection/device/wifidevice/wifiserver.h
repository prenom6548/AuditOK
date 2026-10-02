#ifndef WIFISERVER_H
#define WIFISERVER_H

#include "tcpserver.h"
#include "videosocket.h"
#include "baseserver.h"

class WifiServer : public BaseServer
{
    Q_OBJECT

public:
    WifiServer(QObject *parent = nullptr, QString ip = "", int port = 0);

    virtual ~WifiServer();

    void stop() override;

private Q_SLOTS:
    void onConnected();

private:
    QString m_clientIP;
    QString m_ip;
    int m_port;
};

#endif // WIFISERVER_H
