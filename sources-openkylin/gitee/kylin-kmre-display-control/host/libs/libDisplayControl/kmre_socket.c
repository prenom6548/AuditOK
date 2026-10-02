/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Ma Chao    machao@kylinos.cn
 * Alan Xie   xiehuijun@kylinos.cn
 * Clom       huangcailong@kylinos.cn
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kmre_socket.h"


static void name_unix_path(char* path, size_t pathlen)
{
    char tmp[PATH_MAX];
    int ret = 0;

    struct stat st;

    snprintf(tmp, sizeof(tmp), "/var/run/user/%d/kmre", getuid());

    do {
        ret = lstat(tmp, &st);
    } while(ret < 0 && errno == EINTR);

    if (ret < 0 && errno == ENOENT) {
        do {
            ret = mkdir(tmp, 0766);
        } while (ret < 0 && errno == EINTR);
    }

   snprintf(path, pathlen, "%s/display-control", tmp);

}

int
server_listen(int num_conn)
{
    int fd, len, err, rval;

    struct sockaddr_un un;

    /* create a UNIX domain stream socket */

    if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;

    name_unix_path(un.sun_path, sizeof(un.sun_path));
    if (access(un.sun_path, F_OK) == 0) {
        unlink(un.sun_path);
    }

    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    if(bind(fd, (struct sockaddr *)&un, len) < 0)
    {
        rval = -1;
        goto errout;
    }

    chmod(un.sun_path, S_IRWXU | S_IRWXG | S_IRWXO);

    if(listen(fd, num_conn) < 0)
    {
        rval = -1;
        goto errout;
    }

    return fd;

errout:
    err = errno;
    close(fd);
    errno = err;

    return rval;
}

int server_accept(int listen_fd)
{
    int client_fd = -1;

    while (1) {
        struct sockaddr_un addr;
        socklen_t len = sizeof(addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&addr, &len);

        if (client_fd < 0 && errno == EINTR) {
            continue;
        }
        break;
    }

    return client_fd;
}

int client_connect()
{
    int fd, len, err, rval;
    struct sockaddr_un un;

    if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;

    name_unix_path(un.sun_path, sizeof(un.sun_path));

    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    if(connect(fd, (struct sockaddr*)&un, len) < 0)
    {
        rval = -1;
        goto errout;
    }

    return fd;

errout:
    err = errno;
    close(fd);
    errno = err;

    return rval;

}


int read_fully(int fd, void *buf, size_t len)
{
    if (!buf) {
      return -1;  // do not allow NULL buf in that implementation
    }
    size_t res = len;
    while (res > 0) {
        ssize_t stat = recv(fd, (char *)(buf) + len - res, res, 0);
        if (stat > 0) {
            res -= stat;
            continue;
        }
        if (stat == 0 || errno != EINTR) { // client shutdown or error
            return -1;
        }
    }
    return 0;
}

int write_fully(int fd, const void *buffer, size_t size)
{

    size_t res = size;
    int retval = 0;

    while (res > 0) {
        ssize_t stat = send(fd, (const char *)buffer + (size - res), res, MSG_NOSIGNAL);
        if (stat < 0) {
            if (errno != EINTR) {
                retval =  stat;
                break;
            }
        } else {
            res -= stat;
        }
    }
    return retval;
}

