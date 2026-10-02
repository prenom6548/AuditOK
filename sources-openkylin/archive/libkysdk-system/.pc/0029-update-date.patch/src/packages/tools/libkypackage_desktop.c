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
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include "../dmvc/libkydmvc.h"
#include <fcntl.h>

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strlen("/"));
}

static bool kdk_create_desktop_link(const char *uri)
{
    if (!uri) {
        return false;
    }

    char canonical_filename[4096] = {"\0"};

    // 确保目标目录存在
    const char *target_dir = "/opt/system/resource/applications";
    if (access(target_dir, F_OK) != 0) {
        // 创建目录，包括父目录
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", target_dir);
        int result = system(mkdir_cmd);
        if (result != 0) {
            syslog(LOG_ERR, "Failed to create directory: %s\n", target_dir);
            return false;
        }
    }

    if (access(target_dir, F_OK) != 0) {
        syslog(LOG_INFO, "[%s], not exits : %s\n", __func__, target_dir);
    }

    char cmd[1024];
    char package_name[256];

    syslog(LOG_INFO, "[%s], uri : %s\n", __func__, uri);

    strncpy(package_name, uri, sizeof(package_name) - 1);
    package_name[sizeof(package_name) - 1] = '\0';
    syslog(LOG_ERR, "[%s], package_name : %s\n", __func__, package_name);

    // // 使用 dpkg -L 获取包中所有文件，并筛选出 /usr/share/applications/ 下的 desktop 文件
    bool success = true;

    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        return false;
    }

    pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return false;
    }

    if (pid == 0) { // 子进程
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        char *args[] = {"dpkg", "-L", (char *)package_name, NULL};

        execvp("dpkg", args);
        
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return false;
        }

        char desktop_path[512];

        // 读取每一行输出（每个desktop文件在系统中的路径）
        while (fgets(desktop_path, sizeof(desktop_path), fp)) {
            // 去除换行符
            strstripspace(desktop_path);

            if (!strstr(desktop_path, "/usr/share/applications/") || !strstr(desktop_path, ".desktop")) {
                continue;
            }

            if (!realpath(desktop_path, canonical_filename) || !verify_file(canonical_filename))
            {
                syslog(LOG_ERR, "[%s], %s realpath false\n", __func__, desktop_path);
                continue;
            }
            
            // 确保路径有效
            if (strlen(canonical_filename) == 0) {
                syslog(LOG_ERR, "[%s], Invalid path\n", __func__);
                continue;
            }

            if(access(canonical_filename, F_OK) != 0) {
                syslog(LOG_ERR, "uri %s not exits\n", canonical_filename);
                continue;
            }

            // 获取文件名
            char *filename = strrchr(canonical_filename, '/');
            if (!filename) {
                continue;
            }
            filename++; // 移动到'/'之后的部分

            syslog(LOG_INFO, "[%s], filename : %s\n", __func__, filename);

            // 构造带前缀的目标文件名
            char prefixed_filename[512];
            snprintf(prefixed_filename, sizeof(prefixed_filename), "kylin-redistribution-symbolic-%s", filename);

            syslog(LOG_INFO, "[%s], prefixed_filename : %s\n", __func__, prefixed_filename);

            // 构造目标链接路径
            char target_link[1024];
            snprintf(target_link, sizeof(target_link), "%s/%s", target_dir, prefixed_filename);

            // 如果软链接已存在，则先删除它
            if (access(target_link, F_OK) == 0) {
                unlink(target_link);
            }

            // 创建软链接，直接链接到实际的desktop文件
            if (symlink(canonical_filename, target_link) != 0) {
                syslog(LOG_WARNING, "Failed to create symlink for %s\n", canonical_filename);
                success = false;
            }
        }

        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
    }
    return success;
}

// 修正的 execvp_system 函数 - 执行 rm 命令
static bool execvp_system(char *link_path) 
{
    pid_t pid = fork();
    
    if (pid == -1) {
        syslog(LOG_ERR, "Failed to fork for rm command\n");
        return false;
    }

    if (pid == 0) { // 子进程
        // 重定向输出到 /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }
        
        char *args[] = {"rm", link_path, NULL};
        execvp("rm", args);
        
        // 如果 execvp 失败
        syslog(LOG_ERR, "Failed to execute rm %s\n", link_path);
        exit(EXIT_FAILURE);
    } else { // 父进程
        int status;
        waitpid(pid, &status, 0);
        return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
}

// 修正的 kdk_create_remove_link 函数
static bool kdk_create_remove_link(char *pkg_name)
{
    char *desktop_files[50] = {0};
    int file_count = 0;
    int pipe_fd[2];
    pid_t pid;

    if (pkg_name == NULL) {
        syslog(LOG_ERR, "Package name is NULL\n");
        return false;
    }

    // 创建管道
    if (pipe(pipe_fd) == -1) {
        syslog(LOG_ERR, "Failed to create pipe\n");
        return false;
    }

    pid = fork();
    if (pid == -1) {
        syslog(LOG_ERR, "Failed to fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return false;
    }

    if (pid == 0) { // 子进程 - 执行 dpkg -L
        close(pipe_fd[0]); // 关闭读端
        
        // 重定向 stdout 和 stderr 到管道
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        // 注意：不能直接在 execvp 中使用管道和 grep
        // 这里只执行 dpkg -L，过滤在父进程进行
        char *args[] = {"dpkg", "-L", pkg_name, NULL};
        execvp("dpkg", args);
        
        // 如果 execvp 失败
        syslog(LOG_ERR, "Failed to execute dpkg -L %s\n", pkg_name);
        exit(EXIT_FAILURE);
    } else { // 父进程 - 读取并过滤输出
        close(pipe_fd[1]); // 关闭写端
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            syslog(LOG_ERR, "Failed to fdopen pipe\n");
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return false;
        }

        char desktop_path[512];

        // 读取每一行输出
        while (fgets(desktop_path, sizeof(desktop_path), fp) && file_count < 50) {
            // 去除换行符和首尾空格
            strstripspace(desktop_path);
            
            // 确保路径有效
            if (strlen(desktop_path) == 0) {
                continue;
            }

            // 手动过滤：只处理 /usr/share/applications/ 下的 .desktop 文件
            if (strstr(desktop_path, "/usr/share/applications/") == desktop_path && 
                strstr(desktop_path, ".desktop") != NULL) {
                
                if (access(desktop_path, F_OK) != 0) {
                    syslog(LOG_ERR, "File %s does not exist\n", desktop_path);
                    continue;
                }

                // 获取文件名
                char *filename = strrchr(desktop_path, '/');
                if (!filename) {
                    continue;
                }
                filename++; // 移动到 '/' 之后的部分

                desktop_files[file_count] = strdup(filename);
                if (!desktop_files[file_count]) {
                    syslog(LOG_ERR, "Memory allocation failed for: %s\n", filename);
                    continue;
                }
                
                file_count++;
            }
        }
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
    }

    // 删除链接文件
    if (file_count > 0) {
        for (int i = 0; i < file_count; i++) {
            char link_path[1024] = {0};
            char prefixed_filename[512] = {0};
            
            snprintf(prefixed_filename, sizeof(prefixed_filename), 
                     "kylin-redistribution-symbolic-%s", desktop_files[i]);
            snprintf(link_path, sizeof(link_path), 
                     "/opt/system/resource/applications/%s", prefixed_filename);

            syslog(LOG_INFO, "Removing desktop link: %s\n", link_path);
            
            if (!execvp_system(link_path)) {
                syslog(LOG_ERR, "Failed to remove: %s\n", link_path);
            }
            
            free(desktop_files[i]);
            desktop_files[i] = NULL;
        }
    } else {
        syslog(LOG_INFO, "No desktop files found for package: %s\n", pkg_name);
    }
    
    return true;
}

static char *kdk_get_package_name(char *uri)
{
    char cmd[1024] = "\0";
    char file_name[512] = "\0";
    char *package_name = NULL;

    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        return NULL;
    }

    pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        char *args[] = {"dpkg-deb", "-f", uri, "Package", NULL};

        execvp("dpkg-deb", args);
        
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return NULL;
        }

        fgets(file_name, sizeof(file_name), fp);
        strstripspace(file_name);
        syslog(LOG_ERR, "file_name = %s\n", file_name);
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
    }

    if(file_name[0] != '\0')
    {
        package_name = strdup(file_name);
    }
    return package_name;
}

int main(int argc, char *argv[])
{
    char *pkg_name = NULL;
    if(argv[2])
    {
        bool is_deb_file = (strstr(argv[2], ".deb") != NULL) && (strlen(argv[2]) > 4) && (strcmp(argv[2] + strlen(argv[2]) - 4, ".deb") == 0);
        if (is_deb_file) 
        {
            pkg_name = kdk_get_package_name(argv[2]);
            if(!pkg_name)
            {
                return 0;
            }
        }
        else
        {
            pkg_name = strdup(argv[2]);
        }
    }
    
    if(strcmp(argv[1], "--create") == 0)
    {
        bool result = kdk_create_desktop_link(pkg_name);
        if(!result)
        {
            syslog(LOG_INFO, "Failed to create symlink");
        }
    }
    else if(strcmp(argv[1], "--remove") == 0)
    {
        bool result = kdk_create_remove_link(pkg_name);
        if(!result)
        {
            syslog(LOG_INFO, "Failed to remove symlink");
        }
    }
    if(pkg_name)
    {
        free(pkg_name);
    }

    return 0;
}