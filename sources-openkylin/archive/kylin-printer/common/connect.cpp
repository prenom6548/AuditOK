/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <QDebug>
#include <QProcess>

#include "connect.h"


// return : - 0 on success or -1 on error
// addrname: - Hostname or IP address
// port - Port number
int try_connect(const char *addrname, int port, const char *data)
{
    if (addrname == nullptr || port == 0) {
        return -1;
    }
    int	fd;				/* Socket */
    int	status;			/* Connection status */
    struct sockaddr_in ipv4;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        qDebug("ERROR: Unable to create socket: %s\n",
                strerror(errno));
        return (-1);
    }
    bzero(&ipv4, sizeof(ipv4));
    ipv4.sin_family = AF_INET;
    ipv4.sin_addr.s_addr = inet_addr(addrname);
    ipv4.sin_port = htons(port);

    struct timeval timeout={5,0};
    
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

    status = connect(fd, (struct sockaddr *)&ipv4, (socklen_t)sizeof(ipv4));
    if (status < 0) {
        qDebug("ERROR connecting");
        close(fd);
        return -1;
    }

    if (data == nullptr) {
        close(fd);
        return (status);
    }

    int n = write(fd, data, strlen(data));
    if (n < 0) {
        qDebug("ERROR writing to socket");
        close(fd);
        return -1;
    }
    char buffer[256];
    memset(buffer, 0 , sizeof(buffer));
    n = read(fd, buffer, 255);
    if (n < 0) {
         qDebug("ERROR reading from socket");
         close(fd);
         return -1;
    }
    // printf("%d %d\n", strlen(buffer), buffer[0]);
    if (n > 0 && buffer[0] == 0) {
        close(fd);
        return 0;
    }
    close(fd);
    return -1;
}


bool pingCheck(const QString &destination)
{
    QProcess process;
    process.setProgram("ping");
    process.setArguments(QStringList() << "-c" << "5" << "-q" << destination);
    process.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    process.start();
    process.waitForFinished(-1);

    QByteArray outPut;
    bool isAnalysis {false};
    double packetLoss {0.0};
    while (!(outPut = process.readLine()).isEmpty()) {
        if (outPut.contains("packets transmitted")
            && outPut.contains("received")
            && outPut.contains("packet loss"))
        {
            isAnalysis = true;
            QStringList items = QString(outPut.trimmed()).split(",");
            for(auto &item : items) {
                if (item.contains("packet loss")) {
                    item.remove("packet loss");
                    item.remove("%");
                    item = item.trimmed();
                    packetLoss = item.toDouble();
                    break;
                }
            }
        }
    }

    if (!isAnalysis || std::abs(packetLoss - 100.00) < 0.01)
        return false;

    return true;
}

bool checkLocalhostPort(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return false;
    }

    // 设置 sockaddr_in 结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 使用本地回环地址
    server_addr.sin_port = htons(port);

    // 尝试连接
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        return false;
    } else {
        qDebug("Port %d is open\n", port);
        close(sockfd);
        return true;
    }
}