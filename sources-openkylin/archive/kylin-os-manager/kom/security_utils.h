#ifndef __KOM_SECURITY_UTILS_H__
#define __KOM_SECURITY_UTILS_H__

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
namespace kom
{
extern int verify_file(char *const filename, const char *dirname);
extern FILE *security_fopen(const char *filename, const char *modes, const char *dirname);
extern int security_chmod(const char *file, __mode_t mode, const char *dirname);
extern int security_chown(const char *file, __uid_t owner, __gid_t group, const char *dirname);
extern int security_rename(const char *oldname, const char *newname, const char *dirname);
extern int security_mkdir(const char *path, __mode_t mode, const char *dirname);
extern int security_unlink(const char *name, const char *dirname);
extern int security_system(const char *com, const char *input);
} // namespace kom
#endif
