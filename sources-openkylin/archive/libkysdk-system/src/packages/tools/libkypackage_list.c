/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <kysdk/kysdk-base/cstring-extension.h>

#define DPKG_DB "/var/lib/dpkg/status"

#define PACKAGE_INCREASE_STEP 50

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

typedef struct _kdk_package_info{
    char *name;     // 包名
    char *version;  // 版本号
    char *section;  // 包类型
    char *status;   // 状态
    unsigned long size_kb;   // 包大小
    char *arch;     // 架构
    char * description;     // 描述
}kdk_package_info;

typedef struct {
    unsigned int nums;
    kdk_package_info **list;
}kdk_pkg_list;

/*
根据传入的包状态描述总结包状态标志位，比如传入“install ok installed”，返回值应当为"ii"，
传入值是严格遵守/var/lib/dpkg/status文件的状态描述规则的。
所以这个函数的作用就是获取状态描述的含义，并返回一个字符串，这个字符串的含义是状态描述的含义。
*/
static char* summarize_pkg_status(const char *pkg_status)
{
    if (!pkg_status)
    {
        syslog(LOG_ERR, "pkg status is null");
        return NULL;
    }
        

    // 分配内存存储两个字符的状态码
    char *result = malloc(3 * sizeof(char));
    if (!result)
    {
        syslog(LOG_ERR, "malloc failed");
        return NULL;
    }
        

    memset(result, 0, 3);
    // 初始化为0

    // 解析状态字符串，例如 "install ok installed"
    // 第一部分：期望状态 (Desired-State)
    // 第二部分：实际状态 (Package-Status)
    char *status_copy = strdup(pkg_status);
    if (!status_copy) {
        free(result);
        return NULL;
    }

    char *part1 = NULL;  // 期望状态
    char *part2 = NULL;  // 实际状态
    char *part3 = NULL;  // 子状态

    char *token = strtok(status_copy, " ");
    if (token) {
        part1 = token;
        token = strtok(NULL, " ");
        if (token) {
            part2 = token;
            token = strtok(NULL, " ");
            if (token) {
                part3 = token;
            }
        }
    }

    // 确定期望状态的第一个字符
    if (part1) {
        if (strcmp(part1, "install") == 0) {
            result[0] = 'i';  // install
        } else if (strcmp(part1, "hold") == 0) {
            result[0] = 'h';  // hold
        } else if (strcmp(part1, "deinstall") == 0) {
            result[0] = 'r';  // remove
        } else if (strcmp(part1, "purge") == 0) {
            result[0] = 'p';  // purge
        } else {
            result[0] = 'u';  // unknown
        }
    } else {
        result[0] = 'u';  // unknown
    }

    // 确定实际状态的第二个字符
    if (part2) {
        if (strcmp(part2, "ok") == 0) {
            if (part3 && strcmp(part3, "installed") == 0) {
                result[1] = 'i';  // installed
            } else if (part3 && strcmp(part3, "unpacked") == 0) {
                result[1] = 'U';  // unpacked
            } else if (part3 && strcmp(part3, "half-configured") == 0) {
                result[1] = 'C';  // half-configured
            } else if (part3 && strcmp(part3, "triggers-pending") == 0) {
                result[1] = 't';  // triggers-pending
            } else if (part3 && strcmp(part3, "config-files") == 0) {
                result[1] = 'c';  // config-files
            } else {
                result[1] = 'i';  // default to installed if part2 is "ok"
            }
        } else if (strcmp(part2, "not-installed") == 0) {
            result[1] = 'n';  // not-installed
        } else if (strcmp(part2, "config-files") == 0) {
            result[1] = 'c';  // config-files only
        } else if (strcmp(part2, "half-installed") == 0) {
            result[1] = 'H';  // half-installed
        } else if (strcmp(part2, "unpacked") == 0) {
            result[1] = 'U';  // unpacked
        } else if (strcmp(part2, "failed-config") == 0) {
            result[1] = 'F';  // failed-config
        } else if (strcmp(part2, "missing-ok") == 0) {
            result[1] = 'N';  // obsolete missing-ok
        } else {
            result[1] = 'u';  // unknown
        }
    } else {
        result[1] = 'u';  // unknown
    }

    free(status_copy);
    
    syslog(LOG_DEBUG, "pkg status summary: %s", result);
    return result;
}
static inline char *lookup(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strnlen(key, STRNLEN_MAX) + 1;
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static void _free_package_info(kdk_package_info *package)
{
    if (package->name)
        free(package->name);
    if (package->section)
        free(package->section);
    if (package->version)
        free(package->version);
    if (package->status)
        free(package->status);
    if (package->arch)
        free(package->arch);
    if (package->description)
        free(package->description);
}

void kdk_package_free_list(kdk_pkg_list *pl)
{
    if (!pl)
        return;

    if (pl->list)
    {
        for (int i = 0; i < pl->nums; i++)
        {
            _free_package_info(pl->list[i]);
            free(pl->list[i]);
        }
        free(pl->list);
    }
    free(pl);
}

kdk_pkg_list* kdk_package_get_list()
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    kdk_pkg_list *packagelist = (kdk_pkg_list *)calloc(1, sizeof *packagelist);
    if (!packagelist)
        goto err_out;

    char buffer[4097];
    kdk_package_info *curpackage = NULL;
    size_t maxpackage = 0;
    do
    {
        if (!fgets(buffer, 4096, fp))
            continue;
        char *val;
        if ((val = lookup(buffer, "Package:")))
        {
            curpackage = (kdk_package_info *)calloc(1, sizeof(kdk_package_info));
            if (!curpackage)
                goto err_out;
            curpackage->name = strdup(val);
            if (packagelist->nums == maxpackage)
            {
                maxpackage += PACKAGE_INCREASE_STEP;
                kdk_package_info **tmp = (kdk_package_info **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_info *));
                if (!tmp)
                {
                    free(curpackage);
                    goto err_out;
                }
                packagelist->list = tmp;
            }
            packagelist->list[packagelist->nums] = curpackage;
            packagelist->nums++;
        }
        else if ((val = lookup(buffer, "Status")))
        {
            // syslog(LOG_DEBUG, "package status description: %s", val);
            curpackage->status = summarize_pkg_status(val);
            // syslog(LOG_DEBUG, "package status: %d", curpackage->status);
        }
        else if ((val = lookup(buffer, "Section")))
        {
            curpackage->section = strdup(val);
        }
        else if ((val = lookup(buffer, "Version")))
        {
            curpackage->version = strdup(val);
        }
        else if ((val = lookup(buffer, "Installed-Size")))
        {
            curpackage->size_kb = atol(val);
        }
        else if ((val = lookup(buffer, "Architecture")))
        {
            curpackage->arch = strdup(val);
        }
        else if ((val = lookup(buffer, "Description")))
        {
            curpackage->description = strdup(val);
        }
    } while (feof(fp) == 0);

    fclose(fp);

    if (0 == access("/usr/bin/kare", F_OK))
    {
        char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char name[256] = {0}, version[64] = {0}, env[64] = {0};
                int count = sscanf(line, "%s %s %s", name, version, env);
                if (count != 3)
                    continue;
                if (0 == strcmp(name, "软件包名")) // 第一行表头
                    continue;
                if (name[0] == '=')
                {
                    continue;
                }

                curpackage = (kdk_package_info *)calloc(1, sizeof(kdk_package_info));
                if (!curpackage)
                    goto err_out;
                if (packagelist->nums == maxpackage)
                {
                    maxpackage += PACKAGE_INCREASE_STEP;
                    kdk_package_info **tmp = (kdk_package_info **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_info *));
                    if (!tmp)
                    {
                        free(curpackage);
                        goto err_out;
                    }
                    packagelist->list = tmp;
                }
                packagelist->list[packagelist->nums] = curpackage;
                packagelist->nums++;

                char arch[128] = {0};
                snprintf(arch, sizeof(arch), "kare_%s", env);

                curpackage->name = strdup(name);
                curpackage->version = strdup(version);
                curpackage->status = strdup("ii");
                curpackage->arch = strdup(arch);
                curpackage->description = NULL;
            }
        }
        pclose(fp);
    }

    return packagelist;

err_out:
#ifdef __linux__
    fclose(fp);
#endif
    kdk_package_free_list(packagelist);
    return NULL;
}

// 定义输出字段宽度
#define FIELD_STATUS_WIDTH 4
#define FIELD_NAME_WIDTH 52
#define FIELD_VERSION_WIDTH 39
#define FIELD_ARCH_WIDTH 12

static void print_package_list(kdk_package_info *package)
{
    if (NULL == package)
        return;

    // 第一行：主要信息
    printf("%-*s", FIELD_STATUS_WIDTH, package->status);
    
    // 包名
    if (package->name) {

        printf("%-*s ", FIELD_NAME_WIDTH, package->name);

    } else {
        printf("%-*s ", FIELD_NAME_WIDTH, "");
    }
    
    // 版本号
    if (package->version) {
        printf("%-*s ", FIELD_VERSION_WIDTH, package->version);

    } else {
        printf("%-*s ", FIELD_VERSION_WIDTH, "");
    }
    
    // 架构
    if (package->arch) {
        printf("%-*s ", FIELD_ARCH_WIDTH, package->arch);
    } else {
        printf("%-*s ", FIELD_ARCH_WIDTH, "");
    }
    
    // 描述（使用section或description）
    const char *desc = package->description ? package->description : "";
    if (strlen(desc) > 0) {
        printf("%s", desc);
    }
    
    printf("\n");
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        printf("不需要加其它参数\n");
        return 1;
    }

    kdk_pkg_list *list = kdk_package_get_list();
    for (size_t i = 0; i < list->nums; i ++)
    {
        print_package_list(list->list[i]);
    }
    kdk_package_free_list(list);
    return 0;
}
