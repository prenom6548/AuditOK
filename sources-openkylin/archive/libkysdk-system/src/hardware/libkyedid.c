/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
#include <X11/extensions/Xrandr.h>
#include <dirent.h>
#include <errno.h>
#include <libkylog.h>
#include <fcntl.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <kysdk/kysdk-base/sdkmarcos.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include "libkyedid.h"
#include "hd.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <kysdk/kysdk-base/sdkmarcos.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <gio/gio.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

typedef struct _kdk_position
{
    int x;
    int y;
}kdk_position;

typedef struct _kdk_resolution
{
    int x_res;
    int y_res;
}kdk_resolution;

static int edid_lines = 0;

// static int claims_one_point_oh = 0;
static int claims_one_point_two = 0;
static int claims_one_point_three = 0;
static int claims_one_point_four = 0;

static char* get_val_from_file(FILE *fp, const char *key)
{
    if (! fp)
        return NULL;
    char *val = NULL;
    char buf[2048] = {0};
    while (fgets(buf, 2048, fp))
    {
        strstripspace(buf);
        if (strncmp(buf, key, strnlen(key, STRNLEN_MAX)) == 0)
        {
            val = strdup((char*)buf + strnlen(key, STRNLEN_MAX) +1);
            break;
        }
    }

    return val;
}

static int verify_file(char *pFileName)
{
    if(!strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX)))
    {
        return 1;
    }
    else{
        return 0;
    }
}

// 辅助函数：去掉单引号
static void strip_quotes(char *str) {
    if (!str) return;
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '\'') *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

static char *cmd_exec(char *file)
{
    char *res = NULL;
    int fd[2];
    if (pipe(fd) == -1) {
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        return NULL;
    }

    if (pid == 0) {
        close(fd[0]); // 关闭读取端
        dup2(fd[1], STDOUT_FILENO); // 将管道写入端连接到标准输出

        // 使用execve执行所需的命令，例如：
        char* args[] = {"edid-decode", file, NULL};
        execve("/bin/edid-decode", args, NULL);

        // 如果execve执行失败，需要打印错误信息并退出
        perror("Failed to execute command");
        exit(1);
    } else {
        close(fd[1]); // 关闭写入端

        const char* targetLine1 = "Display Product Name"; // 要匹配的目标行
        const char* targetLine2 = "Alphanumeric Data String"; // 要匹配的目标行
        
        res = (char *)malloc(sizeof(char) * 128);
        FILE* fp = fdopen(fd[0], "r"); // 将读取端转换为文件指针
        char *val = get_val_from_file(fp, targetLine1);
        if(!val)
        {
            char *value = get_val_from_file(fp, targetLine2);
            if(!value)
            {
                free(res);
                return NULL;
            }
            strip_quotes(value);
            strncpy(res, value, 127);
            free(value);
        }
        else{
            strip_quotes(val);
            strncpy(res, val, 127);
            free(val);
        }

        fclose(fp); // 关闭文件指针
    }
    
    return res;
}

static void kdk_edid(char *name)
{
    char edid_text[2048] = "\0";
    char *edid_character = (char *)malloc(sizeof(char) * 1024);
    if(!edid_character)
    {
        return;
    }
    FILE *pipeLine = popen("xrandr --prop", "r");  // 建立流管道
    if (!pipeLine)
    {
        free(edid_character);
        return;
    }
    int i = 0;
    int isEdid = 0;
    memset(edid_character, 0, 1024);
    while(fgets(edid_text, sizeof(edid_text), pipeLine))
    {
        if(i != 1)
        {
            if(strstr(edid_text, name))
            {
                i = 1;
                continue;
            }
            else{
                continue;
            }
        }
        else{
            if(isEdid != 1)
            {
                if(strstr(edid_text, "EDID"))
                {
                    isEdid = 1;
                    continue;
                }
                else
                    continue;
            }
            
            if(strstr(edid_text, "TearFree") || strstr(edid_text, "GAMMA_LUT_SIZE:") || strstr(edid_text, "non-desktop:") || strstr(edid_text, "BorderDimensions"))
            {
                break;
            }
        
            strstripspace(edid_text);
            strncat(edid_character, edid_text, 1024 - strnlen(edid_character, STRNLEN_MAX));
        }
    }
    pclose(pipeLine);

    FILE *fp = fopen("/tmp/sdk-edid", "w");
    if(!fp)
    {
        free(edid_character);
        return;
    }
    fprintf(fp, "%s", edid_character);
    
    fclose(fp);
    free(edid_character);
    return;

}

char **kdk_edid_get_interface()
{
    Display *disp;
    XRRScreenResources *screen;
    XRROutputInfo *info;
    int iscres;
    int icrtc;
    char tmp[128] = "\0";
    char **res = NULL;
    int count = 0;
    char test[100] = {0};
    
    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources(disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                memset(tmp, 0, sizeof(tmp));

                if (strstr(test, info->name))
                {
                    break;
                }
                else
                {
                    strncpy(tmp, info->name, sizeof(tmp) - 1);
                    strncpy(test, tmp, sizeof(test) - 1);
                    --icrtc;
                }

                res = realloc(res, (count + 2) * sizeof(char *));
                if (!res)
                {
                    klog_err("内存申请失败：%s\n", strerror(errno));
                    goto err_out;
                }
                res[count] = calloc(strnlen(tmp, STRNLEN_MAX) + 1, sizeof(char));
                if (!res[count])
                {
                    klog_err("内存申请失败：%s\n", strerror(errno));
                    goto err_out;
                }
                strncpy(res[count], tmp, strnlen(tmp, STRNLEN_MAX) + 1);
                count++;
            }
        }      
        XRRFreeOutputInfo(info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);
    res[count] = NULL;
    goto out;

err_out:
    while (count)
    {
        free(res[count - 1]);
        count--;
    }
    SAFE_FREE(res);
    return NULL;
out:
    return res;
}

static unsigned char *extract_edid(int fd)
{
    char *ret = NULL;
    char *start, *c;
    unsigned char *out = NULL;
    int state = 0;
    int lines = 0;
    int i;
    int out_index = 0;
    int len, size;

    size = 1 << 10;
    ret = malloc(size);
    len = 0;

    if (ret == NULL)
        return NULL;

    for (;;)
    {
        i = read(fd, ret + len, size - len);
        if (i < 0)
        {
            free(ret);
            return NULL;
        }
        if (i == 0)
            break;
        len += i;
        if (len == size)
        {
            char *t;
            size <<= 1;
            t = realloc(ret, size);
            if (t == NULL)
            {
                free(ret);
                return NULL;
            }
            ret = t;
        }
    }

    start = strstr(ret, "EDID_DATA:");
    if (start == NULL)
        start = strstr(ret, "EDID:");
    /* Look for xrandr --verbose output (lines of 16 hex bytes) */
    if (start != NULL)
    {
        const char indentation1[] = "                ";
        const char indentation2[] = "\t\t";
        /* Used to detect that we've gone past the EDID property */
        const char half_indentation1[] = "        ";
        const char half_indentation2[] = "\t";
        const char *indentation;
        char *s;

        lines = 0;
        for (i = 0;; i++)
        {
            int j;

            /* Get the next start of the line of EDID hex, assuming spaces for indentation */
            s = strstr(start, indentation = indentation1);
            /* Did we skip the start of another property? */
            if (s && s > strstr(start, half_indentation1))
                break;

            /* If we failed, retry assuming tabs for indentation */
            if (!s)
            {
                s = strstr(start, indentation = indentation2);
                /* Did we skip the start of another property? */
                if (s && s > strstr(start, half_indentation2))
                    break;
            }

            if (!s)
                break;

            lines++;
            start = s + strnlen(indentation, STRNLEN_MAX);

            s = realloc(out, lines * 16);
            if (!s)
            {
                free(ret);
                free(out);
                return NULL;
            }
            out = (unsigned char *)s;
            c = start;
            for (j = 0; j < 16; j++)
            {
                char buf[3];
                /* Read a %02x from the log */
                if (!isxdigit(c[0]) || !isxdigit(c[1]))
                {
                    if (j != 0)
                    {
                        lines--;
                        break;
                    }
                    free(ret);
                    free(out);
                    return NULL;
                }
                buf[0] = c[0];
                buf[1] = c[1];
                buf[2] = 0;
                out[out_index++] = strtol(buf, NULL, 16);
                c += 2;
            }
        }

        free(ret);
        edid_lines = lines;
        return out;
    }

    /* Is the EDID provided in hex? */
    for (i = 0; i < 32 && isxdigit(ret[i]); i++)
        ;
    if (i == 32)
    {
        out = malloc(size >> 1);
        if (out == NULL)
        {
            free(ret);
            return NULL;
        }

        for (c = ret; *c; c++)
        {
            char buf[3];

            if (*c == '\n')
                continue;

            /* Read a %02x from the log */
            if (!isxdigit(c[0]) || !isxdigit(c[1]))
            {
                free(ret);
                free(out);
                return NULL;
            }

            buf[0] = c[0];
            buf[1] = c[1];
            buf[2] = 0;

            out[out_index++] = strtol(buf, NULL, 16);
            c++;
        }

        free(ret);
        edid_lines = out_index >> 4;
        return out;
    }

    /* wait, is this a log file? */
    for (i = 0; i < 8; i++)
    {
        if (!isascii(ret[i]))
        {
            edid_lines = len / 16;
            return (unsigned char *)ret;
        }
    }

    /* I think it is, let's go scanning */
    if (!(start = strstr(ret, "EDID (in hex):")))
        return (unsigned char *)ret;
    if (!(start = strstr(start, "(II)")))
        return (unsigned char *)ret;

    for (c = start; *c; c++)
    {
        if (state == 0)
        {
            char *s;
            /* skip ahead to the : */
            s = strstr(c, ": \t");
            if (!s)
                s = strstr(c, ":     ");
            if (!s)
                break;
            c = s;
            /* and find the first number */
            while (!isxdigit(c[1]))
                c++;
            state = 1;
            lines++;
            s = realloc(out, lines * 16);
            if (!s)
            {
                free(ret);
                free(out);
                return NULL;
            }
            out = (unsigned char *)s;
        }
        else if (state == 1)
        {
            char buf[3];
            /* Read a %02x from the log */
            if (!isxdigit(*c))
            {
                state = 0;
                continue;
            }
            buf[0] = c[0];
            buf[1] = c[1];
            buf[2] = 0;
            out[out_index++] = strtol(buf, NULL, 16);
            c++;
        }
    }

    edid_lines = lines;

    free(ret);

    return out;
}

const float kdk_edid_get_gamma(char *name)
{
    if (!name) {
        return 0.00f;
    }

    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    float ga = 0.00f;
    strncpy(str_name, name, sizeof(str_name) - 1);

    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return 0.00f;
    }

    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return 0.00f;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg); 
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }
    
    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        // 子进程
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO); 
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        
        exit(1);
    } else {
        // 父进程
        close(fd_pipe[1]); 
        
        FILE *fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            return 0.00f;
        }
        
        char *gamma = get_val_from_file(fp, "Gamma");
        if (gamma) {
            strstripspace(gamma);
            ga = atof(gamma);
            free(gamma);  // 释放 gamma 字符串
        }
        
        fclose(fp);  // 这会自动关闭 fd_pipe[0]
        
        // 等待子进程结束，避免僵尸进程
        int status;
        waitpid(pid, &status, 0);
        
        if (ga > 0.0f) {
            return ga;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return 0.00f;
    }
    
    if (access(filename, F_OK) != 0) {
        return 0.00f;
    }
    
    FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return 0.00f;
    }
    
    char *gamma = get_val_from_file(fp, "Gamma");
    if (gamma) {
        strstripspace(gamma);
        ga = atof(gamma);
        free(gamma);
    }
    
    pclose(fp);
    
    return ga;
}

static float parse_edid_size(FILE *fp) {
    if (!fp) return 0.00f;
    
    float size = 0.00f;
    char xsize[16] = {0};
    char ysize[16] = {0};
    
    char *tmp = get_val_from_file(fp, "Maximum image size");
    if (tmp) {
        sscanf(tmp, "%s %*s %*s %s", xsize, ysize);
        int num1 = atoi(xsize);
        int num2 = atoi(ysize);
        if (num1 > 0 && num2 > 0) {
            size = sqrt(pow(num1, 2) + pow(num2, 2)) / 2.54;
        }
        free(tmp);
    }
    
    if (size == 0.00f) {        
        char *msize = get_val_from_file(fp, "Detailed mode");
        if (msize) {
            sscanf(msize, "%*s %*s %*s %s %*s %*s %s", xsize, ysize);
            int num1 = atoi(xsize);
            int num2 = atoi(ysize);
            if (num1 > 0 && num2 > 0) {
                size = sqrt(pow(num1, 2) + pow(num2, 2)) / 25.4;
            }
            free(msize);
        }
    }
    
    return size;
}

float kdk_edid_get_size(char *name)
{
    if (!name) {
        return 0.00f;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    float size = 0.00f;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return 0.00f;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return 0.00f;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    // 使用 pipe + fork 方式调用 edid-decode
    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        // 子进程
        close(fd_pipe[0]);  // 关闭读取端
        dup2(fd_pipe[1], STDOUT_FILENO);  // 标准输出重定向到管道
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);

        exit(1);
    } else {
        // 父进程
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        size = parse_edid_size(fp);
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        if (size > 0.0f) {
            goto adjust_by_model;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return 0.00f;
    }
    
    if (access(filename, F_OK) != 0) {
        return 0.00f;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return 0.00f;
    }
    
    size = parse_edid_size(fp);
    pclose(fp);
    
adjust_by_model:

    if (size > 0.0f) {
        char *model = kdk_edid_get_model(name);

        if (model) {
            // greatwall
            if (strcmp(model, "24AL60") == 0) {
                size = 23.8f;
            }
            // X100
            if (strcmp(model, "LM133LF-8L02") == 0) {
                size = 13.3f;
            }
            free(model);
        }
    }
    
    return size;
}

char *kdk_edid_get_max_resolution(char *name)
{
    if(!name)
    {
        return NULL;
    }

    char *res = NULL;

    if (getenv("WAYLAND_DISPLAY")) {
        FILE *fp = popen("kscreen-doctor --json", "r");
        if(!fp)
        {
            return NULL;
        }
        char *data = (char *)malloc(sizeof(char) * 230400);
        fread(data, 1, 230400, fp);
        fclose(fp);

        cJSON *json = cJSON_Parse(data);
        if (json == NULL) {
            return NULL;
        }
        // 获取 outputs 数组
        cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
        if (outputs == NULL || !cJSON_IsArray(outputs)) {
            cJSON_Delete(json);
            return NULL;
        }

        // 遍历 outputs 数组
        cJSON *output, *currentModeId;
        cJSON_ArrayForEach(output, outputs) {
            // 获取当前对象的 currentModeId
            currentModeId = cJSON_GetObjectItem(output, "currentModeId");
            if (strnlen(currentModeId->valuestring, STRNLEN_MAX) != 0) {
                cJSON *modes = cJSON_GetObjectItem(output, "modes");
                if (modes == NULL || !cJSON_IsArray(modes)) {
                    cJSON_Delete(json);
                    return NULL;
                }

                // 遍历 modes 数组
                cJSON *mode;
                int resolv_height = 0;
                int resolv_width = 0;
                cJSON_ArrayForEach(mode, modes) {
                    // 获取当前 mode 对象的 id
                    cJSON *id = cJSON_GetObjectItem(mode, "id");
                    cJSON *size = cJSON_GetObjectItem(mode, "size");
                    // 获取 size 对象中的 height 和 width
                    cJSON *height = cJSON_GetObjectItem(size, "height");
                    cJSON *width = cJSON_GetObjectItem(size, "width");

                    if((height->valueint > resolv_height) && (width->valueint > resolv_height))
                    {
                        resolv_height = height->valueint;
                        resolv_width = width->valueint;
                    }
                }

                cJSON *resolv_name = cJSON_GetObjectItem(output, "name");
                if (resolv_name == NULL || !cJSON_IsString(resolv_name)) {
                    cJSON_Delete(json);
                    return NULL;
                }

                if(strstr(resolv_name->valuestring, name))
                {
                    res = (char *)malloc(sizeof(char) * 64);
                    if(!res)
                    {
                        return NULL;
                    }
                    snprintf(res, 64, "%dx%d", resolv_width, resolv_height);
                    break;
                }
            }
        }

        cJSON_Delete(json);       

    } else if (getenv("DISPLAY")) {
        char edid_text[2048] = "\0";
        char tmp[64] = "\0";
        res = (char *)malloc(sizeof(char) * 64);
        if(!res)
        {
            return NULL;
        }
        memset(res, 0, 64);
        FILE *pipeLine = popen("xrandr --prop", "r");  // 建立流管道
        if (!pipeLine)
        {
            free(res);
            return NULL;
        }
        int i = 0;
        while(fgets(edid_text, sizeof(edid_text), pipeLine))
        {
            if(i != 1)
            {
                if(strstr(edid_text, name))
                {
                    i = 1;
                }
                continue;
            }
            if(strstr(edid_text, "+"))
            {
                strncpy(tmp, edid_text, sizeof(tmp) - 1);
            }

        }
        pclose(pipeLine);
        sscanf(tmp, "%s", res);
        if(strnlen(res, STRNLEN_MAX) == 0)
        {
            free(res);
            return NULL;
        }
    }
    
    return res;
}

char *kdk_edid_get_model(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[4096] = "\0";
    char filename[4096] = "\0";
    char *edid_text = NULL;
    char str_name[32] = "\0";
    strncpy(str_name, name, sizeof(str_name) - 1);
    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);

    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
    {
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            goto err_out;
        }

        if (0 != access(filename, F_OK))
            goto err_out;
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            goto err_out;
        }
        edid_text = get_val_from_file(fp, "Display Product Name");
        if(!edid_text)
        {
            pclose(fp);  // 关闭第一个fp
            fp = popen("edid-decode /tmp/sdk-edid", "r");
            if(!fp)
            {
                goto err_out;
            }
            char *str_model = get_val_from_file(fp, "Alphanumeric Data String");
            if(!str_model)
            {
                pclose(fp);
                goto err_out;
            }
            pclose(fp);
            strip_quotes(str_model);
            strstripspace(str_model);
            return str_model;
        }
        else{
            pclose(fp);
            strip_quotes(edid_text);
            strstripspace(edid_text);
            return edid_text;
        }
    }

    char *cmd_val = cmd_exec(path);
    if(!cmd_val)
    {
        hd_data_t *hd_data;
        hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
        if(!hd_data)
        {
            goto err_out;
        }
        char src_model[128] = "\0";
        memset(src_model, 0, 128);

        hd_data->progress = NULL;
        hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

        hd_t *hd, *hd0;
        hd0 = hd_list(hd_data, hw_monitor, 1, NULL);
        for (hd = hd0; hd; hd = hd->next)
        {
            if(hd->model)
            {
                strncpy(src_model, hd->model, sizeof(src_model) - 1); 
            }
        }
        
        hd_free_hd_data(hd_data);
        hd_free_hd_list(hd0);
        if(strnlen(src_model, STRNLEN_MAX) == 0)
        {
            goto err_out;
        }
        else{
            char *model = (char *)malloc(sizeof(char) * 128);
            strncpy(model, src_model, 127);
            return model;
        }
    }
    else{
        strstripspace(cmd_val);
        return cmd_val;
    }
err_out:
    return NULL;
}

static char* get_val_from_fp(FILE *fp, const char *key)
{
    if (! fp)
        return NULL;
    char *val = NULL;
    char buf[2048] = {0};
    while (fgets(buf, 2048, fp))
    {
        strstripspace(buf);
        if (strncmp(buf, key, strnlen(key, STRNLEN_MAX)) == 0)
        {
            val = strdup((char*)buf + strnlen(key, STRNLEN_MAX) +1);
            break;
        }
    }

    return val;
}

static char* parse_visible_area(FILE *fp) {
    if (!fp) return NULL;
    
    float num1 = 0.0f;
    float num2 = 0.0f;
    char *area = NULL;
    char xsize[16] = "\0";
    char ysize[16] = "\0";
    
    char *tmp = get_val_from_fp(fp, "Maximum image size");
    if (tmp) {
        sscanf(tmp, "%s %*s %*s %s", xsize, ysize);
        num1 = atoi(xsize);
        num2 = atoi(ysize);
        if (num1 > 0 && num2 > 0) {
            area = (char *)malloc(64);
            if (area) {
                snprintf(area, 32, "%0.1f X %0.1f cm", num1, num2);
            }
        }
        free(tmp);
    }
    
    if (!area) {        
        char *msize = get_val_from_fp(fp, "Detailed mode");
        if (msize) {
            sscanf(msize, "%*s %*s %*s %s %*s %*s %s", xsize, ysize);
            int num1 = atoi(xsize);
            int num2 = atoi(ysize);
            if (num1 > 0 && num2 > 0) {
                area = (char *)malloc(64);
                if (area) {
                    snprintf(area, 64, "%0.1f X %0.1f mm", num1, num2);
                }
            }
            free(msize);
        }
    }
    
    return area;
}

char *kdk_edid_get_visible_area(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    char *area = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }
    
    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        area = parse_visible_area(fp);
        
        fclose(fp);
        
        int status;
        waitpid(pid, &status, 0);
        
        if (area) {
            return area;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return NULL;
    }
    
    area = parse_visible_area(fp);
    pclose(fp);
    
    return area;
}

static char *manufacturer_name(const unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7c) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xe0) >> 5) + '@';
    name[2] = (x[1] & 0x1f) + '@';
    name[3] = 0;

    if (!isupper(name[0]) || !isupper(name[1]) || !isupper(name[2]))
        ;

    return name;
}

char *kdk_edid_get_manufacturer(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    char *manufacturer = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);

    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        manufacturer = get_val_from_file(fp, "Manufacturer");
        fclose(fp);
             
        int status;
        waitpid(pid, &status, 0);
        
        if (manufacturer) {
            strstripspace(manufacturer);
            return manufacturer;
        }
    }
    
fallback_to_xrandr:
    
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return NULL;
    }
    
    manufacturer = get_val_from_file(fp, "Manufacturer");
    pclose(fp);
    
    if (!manufacturer) {
        return NULL;
    }
    strstripspace(manufacturer);
    return manufacturer;
}

int kdk_edid_get_week(char *name)
{
    if (!name) {
        return -1;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    unsigned char *edid = NULL;
    int wek = -1;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return -1;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return -1;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }
    
    fd = open(canonical_filename, O_RDONLY);
    if (fd == -1) {
        goto fallback_to_xrandr;
    }
    
    edid = extract_edid(fd);
    close(fd);
    
    if (!edid) {
        goto err_out;
    }
    
    if (memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8) != 0) {
        goto err_out;
    }
    
    unsigned char week_edid = edid[0x10];
    unsigned char edid_minor = 0;
    
    if (edid[0x12] == 1) {
        edid_minor = edid[0x13];
    }
    
    if (week_edid != 0 && week_edid != 0xff) {
        if (week_edid <= 54) {
            wek = (int)week_edid;
        }
    } else if (week_edid == 0xff && edid_minor <= 3) {
        wek = -1;
    } else if (week_edid == 0) {
        wek = -1;
    }
    
    free(edid);
    return wek;
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return -1;
    }
    
    if (access(filename, F_OK) != 0) {
        return -1;
    }
    
    FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return -1;
    }
    
    char edid_line[1024] = {0};
    char edid_week[64] = {0};
    int week = -1;
    int year = 0;
    
    while (fgets(edid_line, sizeof(edid_line), fp)) {
        if (strstr(edid_line, "Made in:")) {
            if (sscanf(edid_line, "%*s %*s %*s %d %*s %d", &week, &year) == 2) {
                wek = week;
                break;
            }
            else if (sscanf(edid_line, "%*s %*s %*s %d", &week) == 1) {
                wek = week;
                break;
            }
        }
        else if (strstr(edid_line, "Made in week")) {
            if (sscanf(edid_line, "%*s %*s %*s %d %*s %d", &week, &year) == 2) {
                wek = week;
                break;
            }
            else if (sscanf(edid_line, "%*s %*s %*s %d", &week) == 1) {
                wek = week;
                break;
            }
        }
    }
    
    pclose(fp);
    return wek;
    
err_out:
    if (edid) free(edid);
    return -1;
}

int kdk_edid_get_year(char *name)
{
    if (!name) {
        return -1;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    unsigned char *edid = NULL;
    int year = -1;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return -1;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return -1;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }
    
    fd = open(canonical_filename, O_RDONLY);
    if (fd == -1) {
        goto fallback_to_xrandr;
    }
    
    edid = extract_edid(fd);
    close(fd);
    
    if (!edid) {
        goto err_out;
    }
    
    if (memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8) != 0) {
        goto err_out;
    }

    unsigned char year_byte = edid[0x11];
    
    if (year_byte > 0 && year_byte <= 255) {
        year = 1990 + year_byte;
    } else {
        year = -1;
    }
    
    free(edid);
    return year;
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return -1;
    }
    
    if (access(filename, F_OK) != 0) {
        return -1;
    }
    
    FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return -1;
    }
    
    char edid_line[1024] = {0};
    int week = 0;
    int parsed_year = -1;

    while (fgets(edid_line, sizeof(edid_line), fp)) {
        if (strstr(edid_line, "Made in:")) {
            if (sscanf(edid_line, "%*s %*s %*s %d %*s %d", &week, &parsed_year) == 2) {
                year = parsed_year;
                break;
            }
        }

        else if (strstr(edid_line, "Made in week")) {
            if (sscanf(edid_line, "%*s %*s %*s %d %*s %d", &week, &parsed_year) == 2) {
                year = parsed_year;
                break;
            }
        }
    }
    
    pclose(fp);
    return year;
    
err_out:
    if (edid) free(edid);
    return -1;
}

int kdk_edid_get_primary(char *name)
{
    if(!name)
    {
        return -1;
    }
    int primary = 0;

    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        primary = 0;
        goto out;
    }

    Window root = DefaultRootWindow(display);
    RROutput primary_output = XRRGetOutputPrimary(display, root);

    XRRScreenResources *screen = XRRGetScreenResources(display, root);
    assert(screen);

    for (int i = 0; i < screen->noutput; i++)
    {
        XRROutputInfo *output_info = XRRGetOutputInfo(
            display, screen, screen->outputs[i]);
        assert(output_info);

        if (output_info->connection == RR_Connected)
        {
            XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(
                display, screen, output_info->crtc);
            assert(crtc_info);

            for (int j = 0; j < crtc_info->noutput; j++)
            {
                if (crtc_info->outputs[j] == primary_output)
                {
                    if (strstr(output_info->name, name))
                        primary = 1;
                }
            }
            XRRFreeCrtcInfo(crtc_info);
        }
        XRRFreeOutputInfo(output_info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(display);

out:
    return primary;
}

char *kdk_edid_get_resolution(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char *resolution = NULL;

    if (getenv("WAYLAND_DISPLAY")) {
        char resolv[32] = "\0";
        FILE *fp = popen("kscreen-doctor --json", "r");
        if (!fp) {
            return NULL;
        }
        char *data = (char *)malloc(115200);
        if (!data) {
            pclose(fp);
            return NULL;
        }
        size_t read_len = fread(data, 1, 115199, fp);
        data[read_len] = '\0';
        pclose(fp);

        cJSON *json = cJSON_Parse(data);
        if (json == NULL) {
            free(data);
            return NULL;
        }

        // 获取 outputs 数组
        cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
        if (outputs == NULL || !cJSON_IsArray(outputs)) {
            cJSON_Delete(json);
            return NULL;
        }
        
        /* 遍历 outputs 数组，先检查 name 匹配，只有匹配的 output 才解析 modes */
        cJSON *output, *currentModeId;
        cJSON_ArrayForEach(output, outputs) {
            cJSON *resolv_name = cJSON_GetObjectItem(output, "name");
            if (resolv_name == NULL || !cJSON_IsString(resolv_name))
                continue;
            if (strstr(resolv_name->valuestring, name) == NULL)
                continue;

            /* 找到目标输出，解析其 currentModeId 与 modes */
            currentModeId = cJSON_GetObjectItem(output, "currentModeId");
            if (currentModeId == NULL || !cJSON_IsString(currentModeId) || strnlen(currentModeId->valuestring, STRNLEN_MAX) == 0)
                break;

            cJSON *modes = cJSON_GetObjectItem(output, "modes");
            if (modes == NULL || !cJSON_IsArray(modes))
                break;

            cJSON *mode;
            cJSON_ArrayForEach(mode, modes) {
                cJSON *id = cJSON_GetObjectItem(mode, "id");
                cJSON *size = cJSON_GetObjectItem(mode, "size");
                if (!id || !cJSON_IsString(id) || !size)
                    continue;
                cJSON *height = cJSON_GetObjectItem(size, "height");
                cJSON *width = cJSON_GetObjectItem(size, "width");
                if (!height || !width)
                    continue;

                if (strcmp(id->valuestring, currentModeId->valuestring) == 0) {
                    snprintf(resolv, sizeof(resolv), "%dx%d", width->valueint, height->valueint);
                    break;
                }
            }
            if (strnlen(resolv, STRNLEN_MAX) != 0)
                break;
        }

        cJSON_Delete(json);
        free(data);

        if (strnlen(resolv, STRNLEN_MAX) == 0) {
            /* 未找到匹配显示器，按要求返回 NULL */
            return NULL;
        }

        resolution = (char *)calloc(strnlen(resolv, STRNLEN_MAX) + 1, sizeof(char));
        if (!resolution) {
            return NULL;
        }
        strncpy(resolution, resolv, strnlen(resolv, STRNLEN_MAX) + 1);
        return resolution;
    } else if (getenv("DISPLAY")) {
        char tmp[128];
        XRRCrtcInfo *crtcInfo;
        XRROutputInfo *outputInfo;
        Display *disp = XOpenDisplay(NULL);
        if (!disp) {
            klog_err("Unable to open display\n");
            return NULL;
        }

        Window root = DefaultRootWindow(disp);
        XRRScreenResources *screen = XRRGetScreenResources(disp, root);

        if (!screen) {
            klog_err("Unable to get screen resources\n");
            return NULL;
        }

        for (int i = 0; i < screen->noutput; ++i) {
            outputInfo = XRRGetOutputInfo(disp, screen, screen->outputs[i]);

            if (outputInfo->connection == RR_Connected) {
                if (outputInfo->crtc) {
                    crtcInfo = XRRGetCrtcInfo(disp, screen, outputInfo->crtc);

                    if (crtcInfo && crtcInfo->mode != None) {

                        if (strstr(outputInfo->name, name))
                        {
                            snprintf(tmp, sizeof(tmp), "%dx%d", crtcInfo->width, crtcInfo->height);
                            resolution = (char *)calloc(strnlen(tmp, STRNLEN_MAX) + 1, sizeof(char));
                            if (!resolution)
                            {
                                XRRFreeCrtcInfo(crtcInfo);
                                goto err_out;
                            }
                            strncpy(resolution, tmp, strnlen(tmp, STRNLEN_MAX) + 1);
                        }

                        XRRFreeCrtcInfo(crtcInfo);
                    }
                }
            }
            XRRFreeOutputInfo(outputInfo);
        }

        XRRFreeScreenResources(screen);
        XCloseDisplay(disp);

        if (resolution == NULL)
        {
            free(resolution);
            return NULL;
        }
        return resolution;

    err_out:
        XRRFreeOutputInfo(outputInfo);
        XRRFreeScreenResources(screen);
        XCloseDisplay(disp);
        return NULL;
    }
}

char *kdk_edid_get_ratio(char *name)
{
    if (!name)
    {
        return NULL;
    }
    Display *disp;
    XRRScreenResources *screen;
    XRROutputInfo *info;
    XRRCrtcInfo *crtc_info;
    int iscres;
    int icrtc;
    char tmp[32] = "\0";
    char *ratio = (char *)malloc(sizeof(char) * 32);
    if (!ratio)
    {
        return NULL;
    }

    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources(disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                --icrtc;

                crtc_info = XRRGetCrtcInfo(disp, screen, screen->crtcs[icrtc]);
                if (crtc_info->width != 0 || crtc_info->height != 0)
                {
                    if (strstr(info->name, name))
                    {
                        int ret = (crtc_info->width * 100 / crtc_info->height);
                        if (ret == 125)
                            strncpy(tmp, "5:4", sizeof(tmp) - 1);
                        else if (ret == 133)
                            strncpy(tmp, "4:3", sizeof(tmp) - 1);
                        else if (ret == 160)
                            strncpy(tmp, "16:10", sizeof(tmp) - 1);
                        else if (ret == 177)
                            strncpy(tmp, "16:9", sizeof(tmp) - 1);
                    }
                }
                XRRFreeCrtcInfo(crtc_info);
            }
        }
        XRRFreeOutputInfo(info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);

    if (!tmp[0])
    {
        free(ratio);
        return NULL;
    }
    strncpy(ratio, tmp, 31);
    return ratio;
}

char *kdk_edid_get_character(char *name)
{
    if (!name) return NULL;
    
    char str_name[64] = {0};    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    // 1. 优先尝试 sysfs
    DIR *dirfd = opendir("/sys/class/drm");
    if (dirfd) {
        char **fg = strsplit(str_name, '-');
        if (fg) {
            struct dirent *direfd;
            while ((direfd = readdir(dirfd)) != NULL) {
                if (strstr(direfd->d_name, fg[0])) {
                    char path[512];
                    snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                    
                    FILE *fd = fopen(path, "rb");
                    if (fd) {
                        unsigned char buf[256];
                        size_t size = fread(buf, 1, sizeof(buf), fd);
                        fclose(fd);
                        
                        if (size > 0) {
                            char *hex = malloc(2048);
                            if (hex) {
                                char *ptr = hex;
                                for (size_t i = 0; i < size && ptr - hex < 2048 - 4; i++) {
                                    ptr += snprintf(ptr, 2048 - (ptr - hex), "%02X ", buf[i]);
                                }
                                free(fg);
                                closedir(dirfd);
                                return hex;
                            }
                        }
                    }
                }
            }
            free(fg);
        }
        closedir(dirfd);
    }
    
    // 2. 降级使用 xrandr
    FILE *pipeLine = popen("xrandr --prop 2>/dev/null", "r");
    if (!pipeLine) return NULL;
    
    char buffer[2048] = {0};
    char line[2048];
    int found = 0;
    
    while (fgets(line, sizeof(line), pipeLine)) {
        if (!found) {
            if (strstr(line, name)) found = 1;
            continue;
        }
        
        if (strstr(line, "EDID")) continue;
        
        if (strstr(line, "TearFree") || strstr(line, "GAMMA_LUT_SIZE:") || 
            strstr(line, "non-desktop:")) break;
        
        strstripspace(line);
        strncat(buffer, line, sizeof(buffer) - strlen(buffer) - 1);
    }
    pclose(pipeLine);
    
    if (strlen(buffer) == 0) return NULL;
    
    char *result = malloc(strlen(buffer) + 1);
    if (result) strcpy(result, buffer);
    return result;
}

inline void kdk_edid_freeall(char **list)
{
    if (!list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index++;
    }
    free(list);
}

char* kdk_edid_get_serialNumber(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[4096] = {0};
    char canonical_filename[4096] = {0};
    char str_name[64] = {0};
    char *serial = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        char *serial = get_val_from_file(fp, "Display Product Serial Number");
        fclose(fp);
             
        int status;
        waitpid(pid, &status, 0);
        
        if (serial) {
            strstripspace(serial);
            strip_quotes(serial);
            return serial;
        }
    }
    
fallback_to_xrandr:
    // 降级方案：使用 kdk_edid 和临时文件
    kdk_edid(name);
    
    char filename[4096] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid", "r");
    if (!fp) {
        return NULL;
    }
    
    serial = get_val_from_file(fp, "Display Product Serial Number");
    pclose(fp);
    
    if (!serial) {
        return NULL;
    }
    
    strstripspace(serial);
    strip_quotes(serial);
    return serial;
}

static bool kdk_is_battery()
{
    DBusConnection *conn = NULL;
    DBusError err;

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }


    DBusMessage *msg = NULL;
    DBusPendingCall *sendPending = NULL;
    DBusMessage *replyLangMsg = NULL;

    //创建用户
    msg = dbus_message_new_method_call("org.freedesktop.UPower",   // target for the method call
                                        "/org/freedesktop/UPower",     // object to call on
                                        "org.freedesktop.DBus.Properties", // interface to call on
                                        "Get");           // method name
    if (!msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }
    char *interface = "org.freedesktop.UPower";
    char *property = "OnBattery";

    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, msg, &sendPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    if (msg)
    {
        dbus_message_unref(msg);
    }

    dbus_pending_call_block(sendPending);

    replyLangMsg = dbus_pending_call_steal_reply(sendPending);

    if (replyLangMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendPending)
    {
        dbus_pending_call_unref(sendPending);
    }

    if (dbus_message_get_type(replyLangMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyLangMsg));
        dbus_message_unref(replyLangMsg);
        goto err_out;
    }

    DBusMessageIter iter;
    bool onbattery = FALSE;

    // 解析回复消息
    if (!dbus_message_iter_init(replyLangMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("DBus reply parsing failed\n");
        dbus_message_unref(replyLangMsg);
        dbus_connection_unref(conn);
        goto err_out;
    }

    // 释放reply
    dbus_message_unref(replyLangMsg);

    dbus_message_iter_recurse(&iter, &iter);
    dbus_message_iter_get_basic(&iter, &onbattery);
    dbus_connection_unref(conn);
    
    return onbattery;

err_out:
    return FALSE;
}

static int init_brightness_gamma(const char *screen_name)
{
    DBusConnection *conn = NULL;
    DBusError err;
    int brightness_value = 0;  // 默认返回-1表示失败

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        goto cleanup;
    }

    if (NULL == conn) {
        klog_err("Failed to get DBus connection\n");
        goto cleanup;
    }

    DBusMessage *msg = NULL;
    DBusPendingCall *pending_call = NULL;
    DBusMessage *reply_msg = NULL;

    // 创建方法调用消息
    msg = dbus_message_new_method_call("org.ukui.SettingsDaemon",                    // 目标服务
                                        "/org/ukui/SettingsDaemon/GammaManager",      // 对象路径
                                        "org.ukui.SettingsDaemon.GammaManager",       // 接口名
                                        "getScreensGammaList");                       // 方法名

    if (!msg) {
        klog_err("Failed to create DBus message\n");
        goto cleanup;
    }

    // 添加参数
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &screen_name, DBUS_TYPE_INVALID)) {
        klog_err("Failed to append arguments\n");
        goto cleanup;
    }

    // 发送消息并等待回复
    if (!dbus_connection_send_with_reply(conn, msg, &pending_call, -1)) {
        klog_err("Failed to send message\n");
        goto cleanup;
    }

    if (pending_call == NULL) {
        klog_err("Pending call is NULL\n");
        goto cleanup;
    }

    dbus_connection_flush(conn);
    dbus_pending_call_block(pending_call);

    reply_msg = dbus_pending_call_steal_reply(pending_call);
    if (reply_msg == NULL) {
        klog_err("Failed to get reply message\n");
        goto cleanup;
    }

    // 检查回复类型
    if (dbus_message_get_type(reply_msg) == DBUS_MESSAGE_TYPE_ERROR) {
        const char *error_name = dbus_message_get_error_name(reply_msg);
        klog_err("DBus error: %s\n", error_name ? error_name : "Unknown error");
        goto cleanup;
    }

    // 解析回复消息
    DBusMessageIter args;
    if (!dbus_message_iter_init(reply_msg, &args)) {
        klog_err("Failed to initialize message iterator\n");
        goto cleanup;
    }

    if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) {
        klog_err("Expected array type in reply\n");
        goto cleanup;
    }

    // 遍历数组
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(&args, &array_iter);

    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRUCT) {
        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&array_iter, &struct_iter);

        const char *name = NULL;
        int val1 = 0, val2 = 0, val3 = 0;

        // 解析结构体中的字段
        if (dbus_message_iter_get_arg_type(&struct_iter) == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&struct_iter, &name);
        }
        dbus_message_iter_next(&struct_iter);

        
        dbus_message_iter_get_basic(&struct_iter, &val1);
        dbus_message_iter_next(&struct_iter);

        dbus_message_iter_get_basic(&struct_iter, &val2);
        brightness_value = val2;
        dbus_message_iter_next(&struct_iter);

        dbus_message_iter_get_basic(&struct_iter, &val3);
        dbus_message_iter_next(&array_iter);
    }

cleanup:
    // 清理资源
    if (reply_msg) dbus_message_unref(reply_msg);
    if (pending_call) dbus_pending_call_unref(pending_call);
    if (msg) dbus_message_unref(msg);
    if (conn) dbus_connection_unref(conn);

    return brightness_value;
}

static int get_brightness_from_gsettings() 
{
    GSettings *settings = NULL;
    double brightness = 0.0;

    // 创建GSetting对象
    settings = g_settings_new("org.ukui.power-manager");
    if (!settings) {
        klog_err("Failed to create GSettings object\n");
        return 0;
    }

    // 获取亮度值
    brightness = g_settings_get_double(settings, "brightness-ac");
    
    g_object_unref(settings);
    return (int)brightness;
}

BrightnessInfo* kdk_edid_get_max_brightness(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char** edid_name = kdk_edid_get_interface();
    if(NULL == edid_name)
        return NULL;
    
    for(size_t count = 0; edid_name[count]; count++)
    {
        if(strcmp(edid_name[count], name) != 0)
        {
            kdk_edid_freeall(edid_name);
            return NULL;
        }
    }
    kdk_edid_freeall(edid_name);
    
    BrightnessInfo *info = NULL;
    char line[256] = {0};
    bool onbattery = kdk_is_battery();
    if(!onbattery)
    {
        info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));
        info->brightness_value = 100;
        info->brightness_percentage = 100;
    }
    else
    {
        DIR *dirfd = opendir("/sys/class/backlight");
        if (!dirfd)
        {
            return NULL;
        }
        struct dirent *direfd = NULL;
        char path[256] = {0};
        int brightness_value = 0;

        while ((direfd = readdir(dirfd)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(direfd->d_name, ".") == 0) || (strcmp(direfd->d_name, "..") == 0))
                continue;

            snprintf(path, sizeof(path), "/sys/class/backlight/%s/max_brightness", direfd->d_name);
            
        }
        closedir(dirfd);

        FILE *fp = fopen(path, "r");
        if(fp)
        {
            fgets(line, 256, fp);
            brightness_value = atoi(line);
            fclose(fp);
        }
        info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));
        info->brightness_value = brightness_value;
        info->brightness_percentage = 100;
    }
    
    return info;
}

BrightnessInfo* kdk_edid_get_current_brightness(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char** edid_name = kdk_edid_get_interface();
    if(NULL == edid_name)
        return NULL;
    
    for(size_t count = 0; edid_name[count]; count++)
    {
        if(strcmp(edid_name[count], name) != 0)
        {
            kdk_edid_freeall(edid_name);
            return NULL;
        }
    }
    kdk_edid_freeall(edid_name);

    BrightnessInfo *info = NULL;
    FILE *fp = NULL;
    int brightness = 0;
    char line[256] = {0};
    
    bool onbattery = kdk_is_battery();
    if(!onbattery)
    {
        brightness = init_brightness_gamma(name);
        info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));

        info->brightness_value = brightness;
        info->brightness_percentage = brightness;
    }
    else
    {
        brightness = get_brightness_from_gsettings();

        DIR *dirfd = opendir("/sys/class/backlight");
        if (!dirfd)
        {
            return NULL;
        }
        struct dirent *direfd = NULL;
        char path[256] = {0};
        int brightness_value = 0;

        while ((direfd = readdir(dirfd)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(direfd->d_name, ".") == 0) || (strcmp(direfd->d_name, "..") == 0))
                continue;

            snprintf(path, sizeof(path), "/sys/class/backlight/%s/brightness", direfd->d_name);
            
        }
        closedir(dirfd);

        fp = fopen(path, "r");
        if(fp)
        {
            fgets(line, 256, fp);
            brightness_value = atoi(line);
            fclose(fp);
        }

        info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));

        info->brightness_value = brightness_value;
        info->brightness_percentage = brightness;
    }

    
    return info;
}

void remove_char(char *str, char ch)
{
    char *read = str, *write = str;
    while(*read)
    {
        if(*read != ch)
        {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';
}

static ChromaticityCoordinates* parse_chromaticity_coordinates(const char *input) {
    if (!input || strlen(input) == 0) {
        return NULL;
    }
    
    char *buffer = strdup(input);
    if (!buffer) {
        return NULL;
    }
    
    char *x_str = NULL;
    char *y_str = NULL;
    
    char *comma = strchr(buffer, ',');
    if (comma) {
        *comma = '\0';
        x_str = buffer;
        y_str = comma + 1;
        
        strstripspace(x_str);
        strstripspace(y_str);
    } else {
        char *space = strchr(buffer, ' ');
        if (space) {
            *space = '\0';
            x_str = buffer;
            y_str = space + 1;
            strstripspace(x_str);
            strstripspace(y_str);
        }
    }
    
    if (!x_str || !y_str || strlen(x_str) == 0 || strlen(y_str) == 0) {
        free(buffer);
        return NULL;
    }
    
    ChromaticityCoordinates *info = (ChromaticityCoordinates *)calloc(1, sizeof(ChromaticityCoordinates));
    if (!info) {
        free(buffer);
        return NULL;
    }
    
    info->xCoordinate = strdup(x_str);
    if (!info->xCoordinate) {
        free(info);
        free(buffer);
        return NULL;
    }
    
    info->yCoordinate = strdup(y_str);
    if (!info->yCoordinate) {
        free(info->xCoordinate);
        free(info);
        free(buffer);
        return NULL;
    }
    
    free(buffer);
    return info;
}

ChromaticityCoordinates* kdk_edid_get_red_primary(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[PATH_MAX] = {0};
    char canonical_filename[PATH_MAX] = {0};
    char str_name[64] = {0};
    char tmp[256] = {0}; 
    ChromaticityCoordinates *info = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        char *msize = get_val_from_file(fp, "Red");
        fclose(fp);

        if (msize) {
            remove_char(msize, ':');
            strstripspace(msize);
            strncpy(tmp, msize, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            free(msize);
            
            if (strlen(tmp) > 0) {
                info = parse_chromaticity_coordinates(tmp);
            }
        }
             
        int status;
        waitpid(pid, &status, 0);
        
        if (info) {
            return info;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[PATH_MAX] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid 2>/dev/null", "r");
    if (!fp) {
        return NULL;
    }
    
    char *msize = get_val_from_file(fp, "Red");
    pclose(fp);
    
    if (!msize) {
        return NULL;
    }
    
    remove_char(msize, ':');
    strstripspace(msize);
    strncpy(tmp, msize, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    free(msize);
    
    if (strlen(tmp) == 0) {
        return NULL;
    }
    
    info = parse_chromaticity_coordinates(tmp);
    return info;
}

ChromaticityCoordinates* kdk_edid_get_green_primary(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[PATH_MAX] = {0};
    char canonical_filename[PATH_MAX] = {0};
    char str_name[64] = {0};
    char tmp[256] = {0}; 
    ChromaticityCoordinates *info = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        char *msize = get_val_from_file(fp, "Green");
        fclose(fp);

        if (msize) {
            remove_char(msize, ':');
            strstripspace(msize);
            strncpy(tmp, msize, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            free(msize);
            
            if (strlen(tmp) > 0) {
                info = parse_chromaticity_coordinates(tmp);
            }
        }
             
        int status;
        waitpid(pid, &status, 0);
        
        if (info) {
            return info;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[PATH_MAX] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid 2>/dev/null", "r");
    if (!fp) {
        return NULL;
    }
    
    char *msize = get_val_from_file(fp, "Green");
    pclose(fp);
    
    if (!msize) {
        return NULL;
    }
    
    remove_char(msize, ':');
    strstripspace(msize);
    strncpy(tmp, msize, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    free(msize);
    
    if (strlen(tmp) == 0) {
        return NULL;
    }
    
    info = parse_chromaticity_coordinates(tmp);
    return info;
}

ChromaticityCoordinates* kdk_edid_get_blue_primary(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[PATH_MAX] = {0};
    char canonical_filename[PATH_MAX] = {0};
    char str_name[64] = {0};
    char tmp[256] = {0}; 
    ChromaticityCoordinates *info = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        char *msize = get_val_from_file(fp, "Blue");
        fclose(fp);

        if (msize) {
            remove_char(msize, ':');
            strstripspace(msize);
            strncpy(tmp, msize, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            free(msize);
            
            if (strlen(tmp) > 0) {
                info = parse_chromaticity_coordinates(tmp);
            }
        }
             
        int status;
        waitpid(pid, &status, 0);
        
        if (info) {
            return info;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[PATH_MAX] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid 2>/dev/null", "r");
    if (!fp) {
        return NULL;
    }
    
    char *msize = get_val_from_file(fp, "Blue");
    pclose(fp);
    
    if (!msize) {
        return NULL;
    }
    
    remove_char(msize, ':');
    strstripspace(msize);
    strncpy(tmp, msize, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    free(msize);
    
    if (strlen(tmp) == 0) {
        return NULL;
    }
    
    info = parse_chromaticity_coordinates(tmp);
    return info;
}

ChromaticityCoordinates* kdk_edid_get_white_primary(char *name)
{
    if (!name) {
        return NULL;
    }
    
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char path[PATH_MAX] = {0};
    char canonical_filename[PATH_MAX] = {0};
    char str_name[64] = {0};
    char tmp[256] = {0}; 
    ChromaticityCoordinates *info = NULL;
    FILE *fp = NULL;
    
    strncpy(str_name, name, sizeof(str_name) - 1);
    
    dirfd = opendir("/sys/class/drm");
    if (!dirfd) {
        return NULL;
    }
    
    char **fg = strsplit(str_name, '-');
    if (!fg) {
        closedir(dirfd);
        return NULL;
    }
    
    // 第一次遍历：优先精确匹配完整的显示器名称
    rewinddir(dirfd);
    int exact_match_found = 0;
    while ((direfd = readdir(dirfd)) != NULL) {
        if (strstr(direfd->d_name, name)) {
            snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
            exact_match_found = 1;
            break;
        }
    }
    
    // 如果没有精确匹配，再进行模糊匹配
    if (!exact_match_found) {
        rewinddir(dirfd);
        while ((direfd = readdir(dirfd)) != NULL) {
            if (strstr(direfd->d_name, fg[0])) {
                snprintf(path, sizeof(path), "/sys/class/drm/%s/edid", direfd->d_name);
                break;
            }
        }
    }
    
    free(fg);
    closedir(dirfd);
    
    int fd = -1;
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)) {
        goto fallback_to_xrandr;
    }

    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        goto fallback_to_xrandr;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        goto fallback_to_xrandr;
    }
    
    if (pid == 0) {
        close(fd_pipe[0]);
        dup2(fd_pipe[1], STDOUT_FILENO);
        
        char* args[] = {"edid-decode", path, NULL};
        execve("/bin/edid-decode", args, NULL);
        exit(1);
    } else {
        close(fd_pipe[1]);
        
        fp = fdopen(fd_pipe[0], "r");
        if (!fp) {
            close(fd_pipe[0]);
            goto fallback_to_xrandr;
        }
        
        char *msize = get_val_from_file(fp, "White");
        fclose(fp);

        if (msize) {
            remove_char(msize, ':');
            strstripspace(msize);
            strncpy(tmp, msize, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            free(msize);
            
            if (strlen(tmp) > 0) {
                info = parse_chromaticity_coordinates(tmp);
            }
        }
             
        int status;
        waitpid(pid, &status, 0);
        
        if (info) {
            return info;
        }
    }
    
fallback_to_xrandr:
    kdk_edid(name);
    
    char filename[PATH_MAX] = {0};
    if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename)) {
        return NULL;
    }
    
    if (access(filename, F_OK) != 0) {
        return NULL;
    }
    
    fp = popen("edid-decode /tmp/sdk-edid 2>/dev/null", "r");
    if (!fp) {
        return NULL;
    }
    
    char *msize = get_val_from_file(fp, "White");
    pclose(fp);
    
    if (!msize) {
        return NULL;
    }
    
    remove_char(msize, ':');
    strstripspace(msize);
    strncpy(tmp, msize, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    free(msize);
    
    if (strlen(tmp) == 0) {
        return NULL;
    }
    
    info = parse_chromaticity_coordinates(tmp);
    return info;
}

float kdk_edid_get_rawDpiX(char *name)
{
    if(!name)
    {
        return 0.0;
    }
    char *area = kdk_edid_get_visible_area(name);
    if(!area)
    {
        return 0.0;
    }

    char vertical[16] = {0};
    char unit[16] = {0};
    sscanf(area,"%s %*s %*s %s", vertical, unit);
    free(area);

    float inch = 0.0;
    if(strstr(unit, "mm"))
    {
        inch = atof(vertical) / 25.4;
    }
    else if(strstr(unit, "cm"))
    {
        inch = atof(vertical) / 2.54;
    }

    char *max = kdk_edid_get_max_resolution(name);
    if(!max)
    {
        return 0.0;
    }
    char *p = NULL;
    char *buff[3];
    int i = 0;
    p = strtok(max, "x");
    while(p)
    {
        buff[i] = p;
        i++;
        p = strtok(NULL,"");
    }

    float reslou = atof(buff[0]);
    float rawDpiX = reslou / inch;
    return rawDpiX;
}

float kdk_edid_get_rawDpiY(char *name)
{
    if(!name)
    {
        return 0.0;
    }
    char *area = kdk_edid_get_visible_area(name);
    if(!area)
    {
        return 0.0;
    }

    char horization[16] = {0};
    char unit[16] = {0};
    sscanf(area,"%*s %*s %s %s", horization, unit);

    float inch = 0.0;
    if(strstr(unit, "mm"))
    {
        inch = atof(horization) / 25.4;
    }
    else if(strstr(unit, "cm"))
    {
        inch = atof(horization) / 2.54;
    }
    free(area);

    char *max = kdk_edid_get_max_resolution(name);
    if(!max)
    {
        return 0.0;
    }
    char *p = NULL;
    char *buff[3];
    int i = 0;
    p = strtok(max, "x");
    while(p)
    {
        buff[i] = p;
        i++;
        p = strtok(NULL,"");
    }

    int reslou = atoi(buff[1]);
    float rawDpiY = reslou / inch;
    return rawDpiY;
}

char* kdk_edid_get_refreshRate(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char *rate = NULL;

    if (getenv("WAYLAND_DISPLAY")) {
        FILE *fp = popen("kscreen-doctor --json", "r");
        if(!fp)
        {
            return NULL;
        }
        char *data = (char *)malloc(sizeof(char) * 230400);
        if (!data) {
            pclose(fp);
            return NULL;
        }
        size_t read_len = fread(data, 1, 230399, fp);
        data[read_len] = '\0';
        pclose(fp);

        cJSON *json = cJSON_Parse(data);
        if (json == NULL) {
            free(data);
            return NULL;
        }
        
        cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
        if (outputs == NULL || !cJSON_IsArray(outputs)) {
            cJSON_Delete(json);
            free(data);
            return NULL;
        }

        cJSON *output, *currentModeId;
        cJSON_ArrayForEach(output, outputs) {
            cJSON *resolv_name = cJSON_GetObjectItem(output, "name");
            if (resolv_name == NULL || !cJSON_IsString(resolv_name)) {
                continue;
            }

            if (strstr(resolv_name->valuestring, name) == NULL) {
                continue;
            }

            currentModeId = cJSON_GetObjectItem(output, "currentModeId");
            if (currentModeId == NULL || !cJSON_IsString(currentModeId)) {
                break;
            }

            if (strnlen(currentModeId->valuestring, STRNLEN_MAX) == 0) {
                break;
            }

            cJSON *modes = cJSON_GetObjectItem(output, "modes");
            if (modes == NULL || !cJSON_IsArray(modes)) {
                break;
            }

            cJSON *mode;
            cJSON_ArrayForEach(mode, modes) {
                cJSON *id = cJSON_GetObjectItem(mode, "id");
                cJSON *resolv_rate = cJSON_GetObjectItem(mode, "refreshRate");
                if (!id || !cJSON_IsString(id) || !resolv_rate) {
                    continue;
                }

                if (strcmp(id->valuestring, currentModeId->valuestring) == 0) {
                    if (resolv_rate->valuedouble != 0) {
                        rate = (char *)malloc(32 * sizeof(char));
                        if (!rate) {
                            cJSON_Delete(json);
                            free(data);
                            return NULL;
                        }
                        snprintf(rate, 32, "%.2f", resolv_rate->valuedouble);
                    }
                    break;
                }
            }

            break;
        }

        cJSON_Delete(json);
        free(data);

    } else if (getenv("DISPLAY")) {
        char line[512] = {0};

        FILE *fp = popen("xrandr", "r");
        if(!fp)
        {
            return NULL;
        }

        char **buffer = NULL;
        int i = 0;

        while (fgets(line, 512, fp))
        {
            if(i != 1)
            {
                if(strstr(line, name))
                {
                    i = 1;
                    continue;
                }
                else{
                    continue;
                }
            }
            else{
                if(strstr(line, "*"))
                {
                    char* ptr = strchr(line, '*');
                    if (ptr != NULL) {
                        char num[10];
                        char* start = ptr;
                        while (start >= line && *start != ' ') {
                            start--;
                        }
                        sscanf(start + 1, "%s", num);

                        char **buffer = strsplit(num, '*');
                        if (!buffer)
                        {
                            pclose(fp);
                            return NULL;
                        }

                        rate = (char *)malloc(32 * sizeof(char));
                        if(!rate)
                        {
                            pclose(fp);
                            free(buffer);
                            return NULL;
                        }

                        if(strnlen(buffer[0], STRNLEN_MAX) == 0)
                        {
                            pclose(fp);
                            free(buffer);
                            free(rate);
                            return NULL;
                        }
                        strncpy(rate, buffer[0], 31);
                        free(buffer);
                    }
                    break;
                }
            }
        }  
        pclose(fp);
    }

    return rate;
}

char* kdk_edid_get_rotation(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char line[512] = {0};

    FILE *fp = popen("xrandr", "r");
    if(!fp)
    {
        return NULL;
    }

    char **buffer = NULL;
    char *rate = NULL;
    char res[32] = {0};
    int i = 0;

    while (fgets(line, 512, fp))
    {
        if(strstr(line, name))
        {
            char *p = NULL;
            char *buffer[3];
            int i = 0;
            p = strtok(line, "(");
            while(p)
            {
                buffer[i] = p;
                i++;
                p = strtok(NULL,"");
            }

            if(strstr(buffer[0], "right"))
            {
                strncpy(res, "right", sizeof(res) - 1);
                break;
            }
            else if(strstr(buffer[0], "left"))
            {
                strncpy(res, "left", sizeof(res) - 1);
                break;
            }
            else if(strstr(buffer[0], "inverted"))
            {
                strncpy(res, "inverted", sizeof(res) - 1);
                break;
            }
            else
            {
                strncpy(res, "normal", sizeof(res) - 1);
                break;
            }
        }
    }
    if(strnlen(res, STRNLEN_MAX) != 0)
    {
        rate = (char *)malloc(32 * sizeof(char));
        if (!rate)
        {
            return NULL;
        }
        strncpy(rate, res, 31);
    }

    return rate;
}

void kdk_free_brightnessInfo(BrightnessInfo *info)
{
    SAFE_FREE(info);
}

void kdk_free_chromaticityCoordinates(ChromaticityCoordinates *info)
{
    SAFE_FREE(info->xCoordinate);
    SAFE_FREE(info->yCoordinate);
    SAFE_FREE(info);
}

static int conversion_type_int(char *str)
{
    char outputId[48] = "\0";
    strncpy(outputId, str, sizeof(outputId) - 1); 
    char cleanOutputId[10];
    int j = 0;
    for (int i = 0; outputId[i] != '\0'; ++i) {
        if (outputId[i] >= '0' && outputId[i] <= '9') {
            cleanOutputId[j++] = outputId[i];
        }
    }
    cleanOutputId[j] = '\0';
    int id = atoi(cleanOutputId);
    return id;
}

static kdk_position kdk_edid_get_position(char *name)
{
    if(!name)
    {
        return (kdk_position){0, 0};
    }
    kdk_position position = {0, 0};

    char resolv[32] = "\0";
    FILE *fp = popen("kscreen-doctor --json", "r");
    if(!fp)
    {
        return (kdk_position){0, 0};
    }
    char *data = (char *)malloc(115200);
    fread(data, 1, 115200, fp);
    fclose(fp);

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        free(data);
        return (kdk_position){0, 0};
    }

    // 获取 outputs 数组
    cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
    if (outputs == NULL || !cJSON_IsArray(outputs)) {
        cJSON_Delete(json);
        free(data);
        return (kdk_position){0, 0};
    }
    
    // 遍历 outputs 数组
    cJSON *output, *currentModeId;
    cJSON_ArrayForEach(output, outputs) {
        // 获取当前对象的 currentModeId
        currentModeId = cJSON_GetObjectItem(output, "currentModeId");
        if (strnlen(currentModeId->valuestring, STRNLEN_MAX) != 0) {
            cJSON *modes = cJSON_GetObjectItem(output, "modes");
            if (modes == NULL || !cJSON_IsArray(modes)) {
                cJSON_Delete(json);
                free(data);
                return (kdk_position){0, 0};
            }

            // 获取当前 mode 对象的 id
            cJSON *resolv_name = cJSON_GetObjectItem(output, "name");
            if (resolv_name == NULL || !cJSON_IsString(resolv_name)) {
                cJSON_Delete(json);
                free(data);
                return (kdk_position){0, 0};
            }
            int key = 0;
            
            if(strstr(resolv_name->valuestring, name))
            {
                key = 1;
            }

            if(key == 1)
            {
                cJSON *pos = cJSON_GetObjectItem(output, "pos");

                cJSON *x = cJSON_GetObjectItem(pos, "x");
                cJSON *y = cJSON_GetObjectItem(pos, "y");

                position.x = x->valueint;
                position.y = y->valueint;
                break;
            }
        }
    }
    return position;
}

static kdk_position kdk_edid_get_id_position(int id)
{
    kdk_position position = {0, 0};

    char resolv[32] = "\0";
    FILE *fp = popen("kscreen-doctor --json", "r");
    if(!fp)
    {
        return (kdk_position){0, 0};
    }
    char *data = (char *)malloc(115200);
    fread(data, 1, 115200, fp);
    fclose(fp);

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        free(data);
        return (kdk_position){0, 0};
    }

    // 获取 outputs 数组
    cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
    if (outputs == NULL || !cJSON_IsArray(outputs)) {
        cJSON_Delete(json);
        free(data);
        return (kdk_position){0, 0};
    }
    
    // 遍历 outputs 数组
    cJSON *output, *currentId;
    cJSON_ArrayForEach(output, outputs) {
        // 获取当前对象的 currentModeId
        currentId = cJSON_GetObjectItem(output, "id");
        if (currentId->valueint != 0) {
            cJSON *modes = cJSON_GetObjectItem(output, "modes");
            if (modes == NULL || !cJSON_IsArray(modes)) {
                cJSON_Delete(json);
                free(data);
                return (kdk_position){0, 0};
            }
            int key = 0;
            
            if(currentId->valueint == id)
            {
                key = 1;
            }

            if(key == 1)
            {
                cJSON *pos = cJSON_GetObjectItem(output, "pos");

                cJSON *x = cJSON_GetObjectItem(pos, "x");
                cJSON *y = cJSON_GetObjectItem(pos, "y");

                position.x = x->valueint;
                position.y = y->valueint;
                break;
            }
        }
    }
    return position;
}

static kdk_resolution kdk_edid_get_id_resolution(int inputId)
{
    kdk_resolution res = {0, 0};
    FILE *fp = popen("kscreen-doctor --json", "r");
    if(!fp)
    {
        return (kdk_resolution){0, 0};
    }
    char *data = (char *)malloc(115200);
    fread(data, 1, 115200, fp);
    fclose(fp);

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        free(data);
        return (kdk_resolution){0, 0};
    }

    // 获取 outputs 数组
    cJSON *outputs = cJSON_GetObjectItem(json, "outputs");
    if (outputs == NULL || !cJSON_IsArray(outputs)) {
        cJSON_Delete(json);
        free(data);
        return (kdk_resolution){0, 0};
    }
    
    // 遍历 outputs 数组
    cJSON *output, *currentModeId, *currentId;
    cJSON_ArrayForEach(output, outputs) {
        // 获取当前对象的 currentModeId
        currentModeId = cJSON_GetObjectItem(output, "currentModeId");
        if (strnlen(currentModeId->valuestring, STRNLEN_MAX) == 0) 
        {
            cJSON_Delete(json);
            free(data);
            return (kdk_resolution){0, 0};
        }

        currentId = cJSON_GetObjectItem(output, "id");
        if (currentModeId->valueint != 0) 
        {
            cJSON_Delete(json);
            free(data);
            return (kdk_resolution){0, 0};
        }

        if(currentId->valueint == inputId)
        {
            cJSON *modes = cJSON_GetObjectItem(output, "modes");
            if (modes == NULL || !cJSON_IsArray(modes)) 
            {
                cJSON_Delete(json);
                free(data);
                return (kdk_resolution){0, 0};
            }

            // 遍历 modes 数组
            cJSON *mode;
            cJSON_ArrayForEach(mode, modes) {
                // 获取当前 mode 对象的 id
                cJSON *id = cJSON_GetObjectItem(mode, "id");
                cJSON *size = cJSON_GetObjectItem(mode, "size");
                // 获取 size 对象中的 height 和 width
                cJSON *height = cJSON_GetObjectItem(size, "height");
                cJSON *width = cJSON_GetObjectItem(size, "width");

                if(strcmp(id->valuestring, currentModeId->valuestring) == 0)
                {
                    res.x_res = width->valueint;
                    res.y_res = height->valueint;
                }
            }
        }
    }
    cJSON_Delete(json);
    return res;
}

int kdk_edid_set_resolution(char *name, char *xresolv, char *yresolv)
{
    if(!name || !xresolv || !yresolv)
    {
        return 0;
    }

    char resolv[48] = "\0";
    snprintf(resolv, sizeof(resolv), "%sx%s", xresolv, yresolv);
    FILE *fp = popen("kscreen-doctor output -o", "r");
    if(!fp)
        return 0;

    char line[11520] = "\0";
    char outline[11520] = "\0";
    char **outid = NULL;
    char *buffer[518];
    int j = 0;
    int num = 0;
    char *splid[48];
    while (fgets(line, sizeof(line), fp))
    {
        if(strstr(line, name))
        {
            strncpy(outline, line, sizeof(outline) - 1);
            continue;
        }

        outid = realloc(outid, (num + 2) * sizeof(char *));
        if (!outid)
        {
            klog_err("内存申请失败：%s\n", strerror(errno));
            return 0;
        }
        outid[num] = calloc(strnlen(line, STRNLEN_MAX) + 1, sizeof(char));
        if (!outid[num])
        {
            klog_err("内存申请失败：%s\n", strerror(errno));
            return 0;
        }
        strncpy(outid[num], line, strnlen(line, STRNLEN_MAX) + 1);
        num++;
    }

    if(outid)
    {
        outid[num] = NULL;
        num = 0;
        while(outid[num])
        {
            char *q = strtok(outid[num], " ");
            while(q)
            {
                buffer[j] = q;
                j++;
                q = strtok(NULL," ");
            }
            splid[num] = buffer[1];
            num++;
        }
        splid[num] = NULL;
        free(outid);
    }
    pclose(fp);

    if (strnlen(outline, STRNLEN_MAX) == 0) {
        klog_err("The interface name %s does not exist\n", name);
        return 0;
    }

    char modeid[48] = "\0";
    char *buff[48];
    int i = 0;
    char *p = strtok(outline, " ");
    while(p)
    {
        buff[i] = p;
        if(strstr(buff[i], resolv))
            break;
        i++;
        p = strtok(NULL," ");
    }

    if(buff[i])
    {
        sscanf(buff[i], "%[^:]", modeid);
    }
    else{
        klog_err("Resolution %s does not exist\n", resolv);
        return 0;
    }

    char outputId[48] = "\0";
    sscanf(buff[1], "%s", outputId);

    int currentId = conversion_type_int(outputId);

    kdk_resolution currentRes = kdk_edid_get_id_resolution(currentId);

    int mid = conversion_type_int(modeid);

    char cmd[256] = "\0";
    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.mode.%d", currentId, mid);
    strstripspace(cmd);

    int ret = system(cmd);
    if(ret != 0)
    {
        klog_err("Resolution setting failed\n");
        return 0;
    }
    kdk_position position = kdk_edid_get_position(name);

    int tmp = 0;
    tmp = num;
    
    num = 0;
    while(splid[num])
    {
        int id = conversion_type_int(splid[num]);
        kdk_position id_pos = kdk_edid_get_id_position(id);
        if(id_pos.y == 0 && id_pos.x == 0)
        {
            break;
        }
        else{
            kdk_resolution res = kdk_edid_get_id_resolution(id);
            if(id_pos.x - position.x == currentRes.x_res && (position.x == 0 || id_pos.y == 0) && id_pos.y - position.y != currentRes.y_res && position.y != res.y_res)
            {
                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, position.x, id_pos.y);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }

                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%d", id, xresolv, id_pos.y);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }
            }
            else if(id_pos.x - position.x < 0 && id_pos.y - position.y != currentRes.y_res)
            {
                if(tmp == 3)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%d", id, xresolv, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, id_pos.x, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, res.x_res, position.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }
            else if(id_pos.y - position.y == currentRes.y_res && (id_pos.x == 0 || position.y == 0) && id_pos.x - position.x != currentRes.x_res)
            {
                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, position.x, id_pos.y);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }

                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%s", id, id_pos.x, yresolv);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }
            }
            else if(id_pos.y - position.y < 0 && id_pos.x - position.x != currentRes.x_res)
            {
                 if(tmp == 3)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%s", id, id_pos.x, yresolv);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, id_pos.x, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, position.x, res.y_res);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }

            else if(position.x - id_pos.x == res.x_res && position.y - id_pos.y != res.y_res)
            {
                if(tmp == 1)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%s", id, id_pos.x, yresolv);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%d", currentId, xresolv, position.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else{
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, id_pos.x, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, res.x_res, 0);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }

            else if(position.y - id_pos.y == res.y_res && position.x - id_pos.x != res.x_res)
            {
                if(tmp == 1)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, position.x, res.y_res);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%d", id, xresolv, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else{
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, id_pos.x, id_pos.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, 0, res.y_res);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }

            else if(id_pos.y - position.y == currentRes.y_res && id_pos.x - position.x == currentRes.x_res)
            {
                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", currentId, position.x, position.y);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }

                if(tmp == 1)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%s", id, xresolv, yresolv);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else{
                    kdk_resolution res = kdk_edid_get_id_resolution(id);

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, res.x_res, res.y_res);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }
            else if(position.y - id_pos.y == id_pos.y && id_pos.x - position.x == currentRes.x_res)
            {
                snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%s", currentId, position.x, yresolv);
                ret = system(cmd);
                if(ret != 0)
                {
                    klog_err("Position setting failed\n");
                    return 0;
                }

                if(tmp == 1)
                {
                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%s,%d", id, xresolv, position.y);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
                else{
                    kdk_resolution res = kdk_edid_get_id_resolution(id);

                    snprintf(cmd, sizeof(cmd), "kscreen-doctor output.%d.position.%d,%d", id, res.x_res, res.y_res);
                    ret = system(cmd);
                    if(ret != 0)
                    {
                        klog_err("Position setting failed\n");
                        return 0;
                    }
                }
            }
        }
        num++;
    }
    return 1;
}
