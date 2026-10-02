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
#include <unistd.h>
#include <syslog.h>
#include <libkysettings.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <ctype.h>
#include "../dmvc/libkydmvc.h"
#include <errno.h>
#include <sys/wait.h>
#include <glib.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

typedef enum _kdk_pre_inst_method
{
    KDK_PRE_INST_NONE_METHOD = -1,
    KDK_PRE_INST_DPKG_METHOD = 1,
    KDK_PRE_INST_KM_METHOD,
    KDK_PRE_INST_KARE_V10SP1_METHOD,
    KDK_PRE_INST_KARE_V11_METHOD
} kdk_pre_inst_method;

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
}

static int extract_compat_file(const char *dir, const char *uri, const char *keyword, char *value) {
    char cmd[1024] = "\0";
    char line[1024] = "\0";
    char key[128] = "\0";
    char compat[4096] = "\0";

    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == -1) {
        syslog(LOG_ERR, "[%s] fork failed: %s", __func__, strerror(errno));
        return 0;
    }
    
    if (pid == 0) {

        // 直接调用execvp，避免shell注入
        char* argv[] = {"ar", "-x", "--output", (char*)dir, (char*)uri, "_compat", NULL};
        // 子进程
        execvp("ar", argv);
        
        // 如果execvp失败
        syslog(LOG_ERR, "[%s] execvp ar failed: %s", __func__, strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        // 父进程
        waitpid(pid, &status, 0);
    }

    memset(cmd, 0, 1024);
    snprintf(cmd, sizeof(cmd), "%s/_compat", dir);
    char canonical_filename[4096] = {"\0"};
    realpath(cmd, canonical_filename);
    verify_file(canonical_filename);
    
    FILE *fp = fopen(canonical_filename, "r");
    if(fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            strstripspace(line);
            sscanf(line, "%[^:]:%s", key, compat);
            // syslog(LOG_INFO, "key = %s, value = %s\n", key, compat);
            if(strcmp(key, keyword) == 0)
            {
                strcpy(value, compat);
                break;
            }
        }
        fclose(fp);
        char remove_path[1024] = "\0";
        snprintf(remove_path, sizeof(remove_path), "%s/_compat", dir);
        unlink(remove_path);
        // syslog(LOG_INFO, "value = %s\n", value);
    }
    return 1;
}

static kdk_pre_inst_method kdk_check_kare_method(char *uri, char *dir, char *file_name)
{
    char value[128] = "\0";
    int order = 0;
    int flag = 0;
    char *runtime = NULL;

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "[%s] kare\n", __func__);

        if (!extract_compat_file(dir, uri, "runtime_version", value))
        {
            return KDK_PRE_INST_NONE_METHOD;
        }

        if (value[0] != '\0')
        {
            for (const VersionPattern *p = version_patterns; p->pattern; p++)
            {
                if (strstr(value, p->pattern) && flag != 1)
                {
                    if (order == 0)
                        order = p->order;
                    syslog(LOG_INFO, "value = %s, order = %d\n", value, order);

                    if (check_kare_environment(p->value, value, sizeof(value)))
                    {
                        flag = 1;
                        break;
                    }
                }
            }
        }
        else
        {
            CheckResult version = _kdk_dmvc_check_os_version(uri);

            if (version == CHECK_RESULT_V10SP1)
            {
                syslog(LOG_INFO, "CHECK_RESULT_V10SP1");
                strncpy(value, "v10sp1", sizeof(value) - 1);
                flag = 1;
            }
            else if (version == CHECK_RESULT_V11)
            {
                syslog(LOG_INFO, "CHECK_RESULT_V11");
                strncpy(value, "shadow", sizeof(value) - 1);
                flag = 1;
            }
            else
            {
                syslog(LOG_INFO, "CHECK_RESULT_UNDEF");
                KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
                if (NULL == setting)
                {
                    syslog(LOG_ERR, "get handle failed\n");
                    return 0;
                }

                runtime = kdk_conf2_get_string(setting, "kare-runtime");
                if (NULL != runtime)
                {
                    syslog(LOG_INFO, "kare-runtime value: %s\n", runtime);
                    kdk_conf2_ksettings_destroy(setting);
                }

                if (runtime)
                {
                    if (check_kare_environment(runtime, value, sizeof(value)))
                    {
                        syslog(LOG_INFO, "runtime = %s, value = %s\n", runtime, value);
                        flag = 1;
                    }
                    free(runtime);
                }
            }
        }

        if (flag && value[0] != '\0')
        {
            if (strncmp(value, "shadow", 6) == 0)
            {
                return KDK_PRE_INST_KARE_V11_METHOD;
            }
            else if (strncmp(value, "v10sp1", 6) == 0)
            {
                return KDK_PRE_INST_KARE_V10SP1_METHOD;
            }
        }
    }
    return KDK_PRE_INST_NONE_METHOD;
}

static kdk_pre_inst_method kdk_check_kare_or_dpkg(char *uri, char *file_name)
{
    kdk_pre_inst_method method = KDK_PRE_INST_NONE_METHOD;
    int flag = 0;
    KSettings *setting = kdk_conf2_new("libkysdk-package.direct_install_whitelist", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return KDK_PRE_INST_NONE_METHOD;
    }

    char **keys = kdk_conf2_list_keys(setting);
    if (keys)
    {
        for (int i = 0; keys[i]; i++)
        {
            syslog(LOG_INFO, "[%s] keys: %s", __func__, keys[i]);
            char **value = kdk_conf2_get_strv(setting, keys[i]);
            if (value)
            {
                for (int j = 0; value[j]; j++)
                {
                    syslog(LOG_INFO, "[%s] List value : %s, file_name : %s", __func__, value[j], file_name);
                    if (contains_wildcard(value[j]))
                    {
                        bool match = wildcard_match(value[j], file_name);
                        syslog(LOG_INFO, "[%s] List value : %s, file_name : %s, match = %d\n", __func__, value[j], file_name, match);
                        if (match)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    else
                    {
                        if (strcmp(value[j], file_name) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                }
                g_strfreev(value);

                if (flag)
                {
                    break;
                }
            }
            free(keys[i]);
        }
        free(keys);
        kdk_conf2_ksettings_destroy(setting);
    }

    if (flag)
        return KDK_PRE_INST_DPKG_METHOD;
    else
    {
        char *dir = "/tmp/kysdk-package-download";

        if (access(dir, F_OK) != 0)
        {
            // 创建目录，包括父目录
            char mkdir_cmd[512];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir %s", dir);
            int result = system(mkdir_cmd);
            if (result != 0)
            {
                syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                return KDK_PRE_INST_NONE_METHOD;
            }
        }

        //不在白名单里判断是都是特权标签
        char typebm[128] = "\0";

        if (!extract_compat_file(dir, uri, "bare_metal", typebm))
        {
            return 0;
        }
        //是特权标签，需要验签
        if(typebm[0] != '\0' && (strncmp(typebm, "yes", 3) == 0))
        {
            int result = kdk_verify_signature(uri, dir, typebm, 0);
            if(result)
            {
                return KDK_PRE_INST_DPKG_METHOD;
            }
        }
        else
        {
            method = kdk_check_kare_method(uri, dir, file_name);
        }
    }
    return method;
}

static kdk_pre_inst_method kdk_check_km_or_deb(char *uri)
{
    kdk_pre_inst_method method = KDK_PRE_INST_NONE_METHOD;
    char filename[512] = {0};
    strncpy(filename, uri, 511);
    const char *dot = strrchr(filename, '.');
    // 如果没有找到点，或者点是文件名的第一个字符，则返回 false
    if (dot == NULL || dot == filename)
    {
        return 0;
    }
    if (strcmp(dot, ".ok") == 0)
    {
        return KDK_PRE_INST_KM_METHOD;
    }
    else
    {
        memset(filename, 0, 512);
        strncpy(filename, uri, 511);

        char file_name[512] = "\0";

        int pipe_fd[2];
        pid_t pid;

        if (pipe(pipe_fd) == -1) {
            return KDK_PRE_INST_NONE_METHOD;
        }

        pid = fork();
        if (pid == -1) {
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            return KDK_PRE_INST_NONE_METHOD;
        }

        if (pid == 0) { // 子进程
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            dup2(pipe_fd[1], STDERR_FILENO);
            close(pipe_fd[1]);

            char *args[] = {"dpkg-deb", "-f", (char*)filename, "Package", NULL};

            execvp("dpkg", args);
            
            exit(EXIT_FAILURE);
        } else { // 父进程
            close(pipe_fd[1]);
            
            FILE *fp = fdopen(pipe_fd[0], "r");
            if (!fp) {
                close(pipe_fd[0]);
                waitpid(pid, NULL, 0);
                return KDK_PRE_INST_NONE_METHOD;
            }

            char line[1024] = "\0";
            char *val = NULL;
            fgets(file_name, sizeof(file_name), fp);
            strstripspace(file_name);
            syslog(LOG_INFO, "file_name = %s\n", file_name);
            
            fclose(fp);
            if (file_name[0] != '\0')
            {
                method = kdk_check_kare_or_dpkg(uri, file_name);
            }
            
            // 等待子进程结束
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return method;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || 0 == strcmp(argv[1], "--help") || 0 == strcmp(argv[1], "-h"))
    {
        printf("用法:\n"
               "    kypackage_check [选项] 软件包文件\n"
               "\n"
               "选项:\n"
               "    --help, -h              显示此帮助信息\n"
               "    --version               显示程序版本信息\n"
               "\n"
               "功能描述:\n"
               "    此工具用于检查软件包文件的安装环境。\n"
               "    它能确定软件包应使用 dpkg、km 或 kare 环境进行安装。\n"
               "\n"
               "支持的软件包格式:\n"
               "    - deb 文件 (Debian 软件包、KARE 软件包)\n"
               "    - ok 文件 (开明软件包)\n"
               "\n"
               "返回值含义:\n"
               "    1    软件包将在 dpkg 环境中安装\n"
               "    2    软件包将在 kaiming 环境中安装\n"
               "    3    软件包将在 kare v10sp1 环境中安装\n"
               "    4    软件包将在 kare v11 环境中安装\n"
               "    0    无法确定软件包安装环境\n"
               "\n"
               "使用示例:\n"
               "    sudo kypackage_check mypackage.deb\n"
               "\n");
    }
    else if (geteuid() == 0) // root 用户，可以调用接口
    {
        if (access(argv[1], F_OK) != 0)
        {
            char loginfo[512] = {0};
            snprintf(loginfo, sizeof(loginfo), "错误: '%s': 没有那个文件或目录", argv[1]);
            printf("%s\n", loginfo);
            return -1;
        }

        kdk_pre_inst_method method = kdk_check_km_or_deb(argv[1]);
        switch (method)
        {
        case KDK_PRE_INST_DPKG_METHOD:
            printf("软件包%s将要安装的环境为dpkg\n", argv[1]);
            break;
        case KDK_PRE_INST_KM_METHOD:
            printf("软件包%s将要安装的环境为kaiming\n", argv[1]);
            break;
        case KDK_PRE_INST_KARE_V10SP1_METHOD:
            printf("软件包%s将要安装的环境为kare v10sp1\n", argv[1]);
            break;
        case KDK_PRE_INST_KARE_V11_METHOD:
            printf("软件包%s将要安装的环境为kare v11\n", argv[1]);
            break;
        case KDK_PRE_INST_NONE_METHOD:
            printf("无法判断软件包%s将要安装的环境\n", argv[1]);
            break;
        default:
            printf("无法判断软件包%s将要安装的环境\n", argv[1]);
            break;
        }
        return method;
    }
    else
    {
        printf("所请求的操作需要超级用户权限\n");
        return -1;
    }

    return 0;
}