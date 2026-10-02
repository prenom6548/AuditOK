#define _GNU_SOURCE

#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *hostPluginRoot = "/run/host/unionRootfs";

/* 原始函数 */
static DIR *(*realOpendir)(const char *) = NULL;
static int (*realAccess)(const char *, int) = NULL;
static void *(*realDlopen)(const char *, int) = NULL;
static int (*realStatx)(int, const char *, int, unsigned int, struct statx *) = NULL;

static pthread_once_t initOnce = PTHREAD_ONCE_INIT;

static void initRealFuctions()
{
    realOpendir = dlsym(RTLD_NEXT, "opendir");
    realAccess = dlsym(RTLD_NEXT, "access");
    realDlopen = dlsym(RTLD_NEXT, "dlopen");
    realStatx = dlsym(RTLD_NEXT, "statx");
}

static void ensureInit()
{
    pthread_once(&initOnce, initRealFuctions);
}

/**
 * @brief 重定向路径
 * @param path 原始绝对路径
 * @param buf 输出缓冲区，用于存放重定向后的路径
 * @return 1 成功 0 失败
 */
static int redirectPath(const char *path, char *buf)
{
    if (!path || path[0] != '/')
        return 0;

    int len = snprintf(buf, PATH_MAX, "%s%s", hostPluginRoot, path);
    if (len < 0 || len >= PATH_MAX)
        return 0;

    return 1;
}

/**
 * @brief 检查指定路径的文件或目录是否存在
 * @param path 待检查的文件路径
 * @return 1 存在 0 不存在
 */
static int fileExists(const char *path)
{
    if (!realAccess)
    {
        return 0;
    }
    return realAccess(path, F_OK) == 0;
}

/**
 * @brief 拦截 statx 系统调用
 * @param dirfd 目录文件描述符
 * @param path 文件路径
 * @param flags 标志位
 * @param mask 需要查询的文件属性掩码
 * @param statxbuf 输出文件属性结构体
 * @return 0 成功 非0 失败
 */
int statx(int dirfd, const char *path, int flags, unsigned int mask, struct statx *statxbuf)
{
    ensureInit();

    if (!realStatx)
    {
        errno = EINVAL;
        return -1;
    }

    int ret = realStatx(dirfd, path, flags, mask, statxbuf);
    if (ret == 0)
        return 0;

    char rebuildPath[PATH_MAX];
    if (redirectPath(path, rebuildPath))
    {
        return realStatx(dirfd, rebuildPath, flags, mask, statxbuf);
    }
    return ret;
}

/**
 * @brief 拦截 dlopen 调用
 * @param path 动态库文件路径
 * @param flags 加载标志位
 */
void *dlopen(const char *path, int flags)
{
    ensureInit();

    if (!realDlopen)
    {
        errno = EINVAL;
        return NULL;
    }
    if (!path)
        return realDlopen(path, flags);

    if (fileExists(path))
        return realDlopen(path, flags);

    char rebuildPath[PATH_MAX];

    if (redirectPath(path, rebuildPath) && fileExists(rebuildPath))
        return realDlopen(rebuildPath, flags);

    return realDlopen(path, flags);
}

/**
 * @brief 拦截 opendir 调用
 * @param path 待打开的目录路径
 * @return 成功返回目录流指针，失败返回 NULL
 */
DIR *opendir(const char *path)
{
    ensureInit();

    if (!realOpendir)
    {
        errno = EINVAL;
        return NULL;
    }
    if (!path)
    {
        errno = EINVAL;
        return NULL;
    }

    if (strstr(path, hostPluginRoot) != NULL)
    {
        return realOpendir(path);
    }

    // 定义需要重定向的系统目录前缀
    static const char *const prefixes[] = {
        "/usr", "/lib", "/lib32", "/lib64", "/bin", "/sbin", "/opt", "/etc"
    };
    const size_t numPrefixes = sizeof(prefixes) / sizeof(prefixes[0]);

    int needRedirect = 0;
    for (size_t i = 0; i < numPrefixes; i++)
    {
        size_t len = strlen(prefixes[i]);
        if (strncmp(path, prefixes[i], len) == 0 && (path[len] == '\0' || path[len] == '/'))
        {
            needRedirect = 1;
            break;
        }
    }

    if (needRedirect)
    {
        char rebuildPath[PATH_MAX];
        if (redirectPath(path, rebuildPath))
        {
            DIR *dir = realOpendir(rebuildPath);
            if (dir != NULL)
            {
                return dir;
            }
        }
    }
    return realOpendir(path);
}
