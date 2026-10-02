#define HAVE_UNISTD_H
#define HAVE_FCNTL_H

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "blockinput.h"


static ssize_t pread_impl(int fd, void *buf, size_t count, off_t offset)
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    ssize_t result;

    pthread_mutex_lock(&lock);

    if (lseek(fd, offset, SEEK_SET) != offset) {
        result = -1;
    } else {
        result = read(fd, buf, count);
    }

    pthread_mutex_unlock(&lock);
    return result;
}

#define pread(a,b,c,d) pread_impl(a,b,c,d)


typedef struct default_block_input {
    udfread_block_input input;
    int                 fd;
} default_block_input;


static int _def_close(udfread_block_input *p_gen)
{
    default_block_input *p = (default_block_input *)p_gen;
    int result = -1;

    if (p) {
        if (p->fd >= 0) {
            result = close(p->fd);
        }
        free(p);
    }

    return result;
}

static uint32_t _def_size(udfread_block_input *p_gen)
{
    default_block_input *p = (default_block_input *)p_gen;
    off_t pos;

    pos = lseek(p->fd, 0, SEEK_END);
    if (pos < 0) {
        return 0;
    }

    return (uint32_t)(pos / UDF_BLOCK_SIZE);
}

static int _def_read(udfread_block_input *p_gen, uint32_t lba, void *buf, uint32_t nblocks, int flags)
{
    default_block_input *p = (default_block_input *)p_gen;
    size_t bytes, got;
    off_t  pos;

    (void)flags;

    bytes = (size_t)nblocks * UDF_BLOCK_SIZE;
    got   = 0;
    pos   = (off_t)lba * UDF_BLOCK_SIZE;

    while (got < bytes) {
        ssize_t ret = pread(p->fd, ((char*)buf) + got, bytes - got, pos + (off_t)got);

        if (ret <= 0) {
            if (ret < 0 && errno == EINTR) {
                continue;
            }
            if (got < UDF_BLOCK_SIZE) {
                return ret;
            }
            break;
        }
        got += (size_t)ret;
    }

    return got / UDF_BLOCK_SIZE;
}


udfread_block_input *block_input_new(const char *path)
{
    default_block_input *p = (default_block_input*)calloc(1, sizeof(default_block_input));
    if (!p) {
        return NULL;
    }

    p->fd = open(path, O_RDONLY);

    if(p->fd < 0) {
        free(p);
        return NULL;
    }

    p->input.close = _def_close;
    p->input.read  = _def_read;
    p->input.size  = _def_size;

    return &p->input;
}
