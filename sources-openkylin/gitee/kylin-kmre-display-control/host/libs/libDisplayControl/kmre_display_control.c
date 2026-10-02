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

#include <stdint.h>
#include <errno.h>

#include "kmre_socket.h"


static int conn_fd = -1;

struct bo_info
{
    uint32_t name;
    int32_t width;
    int32_t height;
    int32_t stride;
    int32_t bpp;
};

typedef struct bo_info bo_info_t;


inline static int get_conn_fd()
{
    if(conn_fd < 0)
        conn_fd = client_connect();

    return conn_fd;
}

inline static void clear_conn_fd()
{
    close(conn_fd);
    conn_fd = -1;
}

inline static int try_read_fully(void *buf, size_t len)
{
    int fd, ret;

    fd = get_conn_fd();
    ret = read_fully(fd, buf, len);

    if(ret < 0)
        clear_conn_fd();

    return ret;
}
inline static int try_write_fully(const void *buffer, size_t size)
{
    int fd, ret;
    fd = get_conn_fd();
    ret = write_fully(fd, buffer, size);

    if(ret < 0)
        clear_conn_fd();
    return ret;
}

int32_t do_FBPost(uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t bpp)
{
    int ret;
    int32_t result;

    bo_info_t b_info;
    b_info.name = name;
    b_info.width = width;
    b_info.height = height;
    b_info.stride = stride;
    b_info.bpp = bpp;

    ret = try_write_fully(&b_info, sizeof(b_info));
    if(ret < 0)
        return -errno;

    ret = try_read_fully(&result, sizeof(result));
    if(ret < 0)
        return -errno;

    return result;
}
