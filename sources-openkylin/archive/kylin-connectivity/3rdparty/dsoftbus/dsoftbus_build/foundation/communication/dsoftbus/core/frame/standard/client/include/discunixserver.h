//
// Created by sqp on 2022/6/23.
//

#ifndef UNIX_SOCKET_DEMO_DISCUNIXSERVER_H
#define UNIX_SOCKET_DEMO_DISCUNIXSERVER_H

class DiscUnixServer
{

public:
    DiscUnixServer();
    ~DiscUnixServer();
    void startServer();
    void stopServer();
    int sendDevInfo(char *uuid, const int devType, char *deName, char *devIp, int devPort);

private:
    int serverListen();
    int serverAccept();
    void getRequest(char *buf, int len);

private:
    int m_serverFd;
    int m_clientFd;
};

#endif // UNIX_SOCKET_DEMO_DISCUNIXSERVER_H
