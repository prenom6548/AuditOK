/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <QDebug>

#include "connect.h"

#include <QHostInfo>


// return : - 0 on success or -1 on error
// addrname: - Hostname or IP address
// port - Port number
int try_connect(const char *addrname, int port)
{
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

    close(fd);

    return (status);
}


int tryConnectFromHost(const QString &hostname)
{
    QHostInfo host = QHostInfo::fromName(hostname);
    return host.error();
}
