#include "security_utils.h"
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stddef.h>
#include <pwd.h>
#include <grp.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QDebug>

namespace kom
{
int verify_file(char *const filename, const char *dirname)
{
    const char *base_dir = dirname;
    size_t len = strlen(base_dir);
    if (strncmp(filename, base_dir, len) == 0) {
        return 1;
    }

    return 0;
}

FILE *security_fopen(const char *filename, const char *modes, const char *dirname)
{
    char file_path[PATH_MAX];
    memset(file_path, 0, sizeof(file_path));
    realpath(filename, file_path);

    if (!file_path || !verify_file(file_path, dirname)) {
        return NULL;
    }
    return fopen(file_path, modes);
}


int security_chmod(const char *file, __mode_t mode, const char *dirname)
{
    char file_path[PATH_MAX];
    memset(file_path, 0, sizeof(file_path));
    realpath(file, file_path);

    if (!file_path || !verify_file(file_path, dirname)) {
        return -1;
    }
    return chmod(file_path, mode);
}

int security_chown(const char *file, __uid_t owner, __gid_t group, const char *dirname)
{
    char file_path[PATH_MAX];
    memset(file_path, 0, sizeof(file_path));
    realpath(file, file_path);

    if (!file_path || !verify_file(file_path, dirname)) {
        return -1;
    }
    return chown(file_path, owner, group);
}


int security_rename(const char *oldname, const char *newname, const char *dirname)
{
    char old_path[PATH_MAX];
    memset(old_path, 0, sizeof(old_path));
    realpath(oldname, old_path);

    char new_path[PATH_MAX];
    memset(new_path, 0, sizeof(new_path));
    realpath(newname, new_path);

    if (!old_path || !verify_file(old_path, dirname) || !new_path || !verify_file(new_path, dirname)) {
        return -1;
    }
    return rename(old_path, new_path);
}

int security_mkdir(const char *path, __mode_t mode, const char *dirname)
{
    char file_path[PATH_MAX];
    memset(file_path, 0, sizeof(file_path));
    realpath(path, file_path);

    if (!file_path || !verify_file(file_path, dirname)) {
        return -1;
    }
    return mkdir(file_path, mode);
}

int security_unlink(const char *name, const char *dirname)
{
    char file_path[PATH_MAX];
    memset(file_path, 0, sizeof(file_path));
    realpath(name, file_path);

    if (!file_path || !verify_file(file_path, dirname)) {
        return -1;
    }
    return unlink(file_path);
}


int security_system(const char *path, const char *command)
{
    pid_t pid = -1;
    int status = -1;


    size_t len = strlen(command) + 1;
    char *cmd = (char *)calloc(len, sizeof(char));
    snprintf(cmd, len, "%s", command);
    int argc = 0;
    char *token = NULL;
    const char *s = " ";
    token = strtok(cmd, s);
    while (token != NULL) {
        argc++;
        token = strtok(NULL, s);
    }
    free(cmd);

    char **argv = (char **)calloc(argc + 1, sizeof(char *));
    cmd = (char *)calloc(len, sizeof(char));
    snprintf(cmd, len, "%s", command);
    int i = 0;
    token = strtok(cmd, s);
    while (token != NULL) {
        argv[i] = token;
        token = strtok(NULL, s);
        i++;
    }
    argv[i] = NULL;

    char **env = NULL;

    pid = fork();
    if (pid == (pid_t)0) {
        /* Child side.  */
        (void)execve(path, argv, env);
        _exit(127);
    } else if (pid < (pid_t)0) {
        status = -1;
    } else {
        int n;
        do {
            n = waitpid(pid, &status, 0);
        } while (n == -1 && errno == EINTR);

        if (n != pid)
            status = -1;
    }
    free(argv);
    free(cmd);
    return status;
}
} // namespace kom
