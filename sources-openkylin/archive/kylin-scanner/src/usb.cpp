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

#include "usb.h"

int create_uevent_socket(void)
{
    const int buf_size = 16 << 20;  // 16MB in bytes
    int sock_fd;
    struct sockaddr_nl addr;
    int result;

    // Initialize netlink address structure
    bzero(&addr, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0x1;  // Bitmask for group 1

    // Create netlink socket
    sock_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (sock_fd < 0) {
        fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
        return -1;
    }

    // Set receive buffer size
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF,
                  &buf_size, sizeof(buf_size)) < 0) {
        fprintf(stderr, "Failed to set socket options: %s\n", strerror(errno));
    }

    // Bind socket to address
    result = bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (result != 0) {
        fprintf(stderr, "Socket binding failed: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}
