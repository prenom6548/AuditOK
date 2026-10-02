#ifndef SERVER_H
#define SERVER_H

#include "adbprocess.h"
#include "tcpserver.h"
#include "videosocket.h"

#include "baseserver.h"

class UsbServer : public BaseServer
{
    Q_OBJECT

    enum SERVER_START_STEP {
        SSS_NULL,
        SSS_PUSH,
        SSS_ENABLE_TUNNEL_REVERSE,
        SSS_ENABLE_TUNNEL_FORWARD,
        SSS_EXECUTE_SERVER,
        SSS_RUNNING,
    };

public:
    explicit UsbServer(QObject *parent = nullptr);

    virtual ~UsbServer();

    bool start(BaseServer::ServerParams params) override;

    bool connectTo() override;

    void stop() override;

private Q_SLOTS:

    void onWorkProcessResult(AdbProcess::ADB_EXEC_RESULT processResult);



private:
    bool startServerByStep();

    const QString &getServerPath();

    bool pushServer();

    bool enableTunnelReverse();

    bool disableTunnelReverse();

    bool enableTunnelForward();

    bool disableTunnelForward();

    bool execute();

    void onConnectTimer() override;

private:
    QString m_serverPath = "";
    AdbProcess m_workProcess;
    AdbProcess m_serverProcess;
    SERVER_START_STEP m_serverStartStep = SSS_NULL;
};

#endif // SERVER_H
