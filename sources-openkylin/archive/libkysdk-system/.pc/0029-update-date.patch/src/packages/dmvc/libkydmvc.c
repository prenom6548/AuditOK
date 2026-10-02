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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <regex.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <gelf.h>
#include <libelf.h>
#include <syslog.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define KYSDK_DMVC_LOG_LEN PATH_MAX * 2

typedef enum
{
    CHECK_RESULT_UNDEF,
    CHECK_RESULT_V10SP1,
    CHECK_RESULT_V11
} CheckResult;

typedef struct
{
    bool check_deps;
    bool check_symbols;
    bool check_builder;
    bool check_install;
    const char *temp_dir;
    const char *glibc_target;
    char result_log[KYSDK_DMVC_LOG_LEN];
} DmvcConfig;

typedef struct
{
    int sp1_score;
    int v11_score;
} Score;

static void _init_config(DmvcConfig *config)
{
    config->check_deps = true;
    config->check_symbols = true;
    config->check_builder = true;
    config->check_install = true;
    config->temp_dir = "/tmp";
    config->glibc_target = "2.31";
    memset(config->result_log, 0, KYSDK_DMVC_LOG_LEN);
}

static void _init_version_score(Score *score)
{
    score->sp1_score = 0;
    score->v11_score = 0;
}

static CheckResult _get_result(Score *score)
{
    if (score->sp1_score > score->v11_score)
    {
        return CHECK_RESULT_V10SP1;
    }
    else if (score->v11_score > score->sp1_score)
    {
        return CHECK_RESULT_V11;
    }
    else
    {
        return CHECK_RESULT_UNDEF;
    }
}

static bool _is_elf_arch_compatible(Elf *elf)
{
    if (!elf)
        return false;

    GElf_Ehdr ehdr;
    if (gelf_getehdr(elf, &ehdr) == NULL)
    {
        return false;
    }

#ifdef __x86_64__
#define HOST_ARCH EM_X86_64
#elif __aarch64__
#define HOST_ARCH EM_AARCH64
#elif __i386__
#define HOST_ARCH EM_386
#elif __loongarch64
#define HOST_ARCH EM_LOONGARCH
#elif __mips__
#define HOST_ARCH EM_MIPS
#elif __sw_64__
#define HOST_ARCH 0x9916
#else
#define HOST_ARCH 0
#endif

    return (ehdr.e_machine == HOST_ARCH);
}

static int _open_elf_file(const char *file, Elf **elf)
{
    int fd = -1;
    Elf *tmp = NULL;

    // 初始化 libelf 库
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        return -1;
    }

    // 打开文件
    fd = open(file, O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }

    // 获取 ELF 描述符
    tmp = elf_begin(fd, ELF_C_READ, NULL);
    if (!tmp)
    {
        close(fd);
        return -1;
    }

    // 确保是 ELF 文件
    if (elf_kind(tmp) != ELF_K_ELF)
    {
        elf_end(tmp);
        close(fd);
        return -1;
    }

    if (!_is_elf_arch_compatible(tmp))
    {
        elf_end(tmp);
        close(fd);
        return -1;
    }

    *elf = tmp;
    return fd;
}

static void _close_elf_file(int *fd, Elf **elf)
{
    if (*elf)
    {
        elf_end(*elf);
        *elf = NULL;
    }
    if (*fd >= 0)
    {
        close(*fd);
        *fd = -1;
    }
}

static bool _check_is_elf_binary(const char *file_path)
{
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    // ELF 文件的魔数是 0x7F 'E' 'L' 'F'
    unsigned char magic[4];
    if (read(fd, magic, 4) != 4)
    {
        close(fd);
        return false;
    }

    bool is_elf = (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');

    close(fd);
    return is_elf;
}

static bool _parse_glibc_version(const char *ver, int *major, int *minor)
{
    char *temp = strdup(ver);
    char *underscore = strchr(temp, '_');

    if (underscore)
    {
        // 处理带下划线的情况，例如 "GLIBC_2.38"
        char *version_part = underscore + 1;
        char *dot = strchr(version_part, '.');
        if (dot)
        {
            *dot = '\0';
            *major = atoi(version_part);
            *minor = atoi(dot + 1);
        }
        else
        {
            *major = atoi(version_part);
            *minor = 0;
        }
    }
    else
    {
        char *dot = strchr(temp, '.');
        if (dot)
        {
            *dot = '\0';
            *major = atoi(temp);
            *minor = atoi(dot + 1);
        }
        else
        {
            *major = atoi(temp);
            *minor = 0;
        }
    }

    free(temp);
    return true;
}

static int _compare_version(const char *ver1, const char *ver2)
{
    if (!ver1 && !ver2)
        return 0;

    int major1, minor1, major2, minor2;

    _parse_glibc_version(ver1, &major1, &minor1);
    _parse_glibc_version(ver2, &major2, &minor2);

    if (major1 != major2)
    {
        return major1 - major2;
    }

    return minor1 - minor2;
}

static bool _depend_higher_than_sp1(const char *depends, const char *depend, DmvcConfig *config)
{
    int len = 0;
    const char *start = NULL, *end = NULL;

    start = strstr(depends, depend);
    if (!start)
    {
        return false;
    }

    start += 5; // "libc6"的长度
    while (*start && isspace((unsigned char)*start))
    {
        start++;
    }

    if (*start != '(')
    {
        return false;
    }

    start++; // 跳过 '('

    // 解析约束符号
    if (strncmp(start, ">=", 2) == 0)
    {
        start += 2;
    }
    else if (strncmp(start, ">>", 2) == 0)
    {
        start += 2;
    }
    else if (*start == '=')
    {
        start += 1;
    }
    else
    {
        return false; // 未知约束格式
    }

    // 跳过空格
    while (*start && isspace((unsigned char)*start))
    {
        start++;
    }

    // 提取版本号，直到遇到 ')'
    end = start;
    while (*end && *end != ')')
    {
        end++;
    }

    len = end - start;
    if (len <= 0)
        return false;

    char libc6_version[len + 1];
    strncpy(libc6_version, start, len);
    libc6_version[len] = '\0';

    if (0 > _compare_version(config->glibc_target, libc6_version))
    {
        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        char message[KYSDK_DMVC_LOG_LEN];
        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, "_check_depends Higher LIBC6_%s", libc6_version);
        strncat(config->result_log, message, log_avail_len);
        return true;
    }

    return false;
}

static void _free_elf_files(char **elf_files, int count)
{
    if (elf_files)
    {
        for (int i = 0; i < count; i++)
        {
            free(elf_files[i]);
        }
        free(elf_files);
    }
}

static bool _check_glibc_version(Elf *elf, Elf_Scn *scn, GElf_Shdr *shdr, DmvcConfig *config)
{
    Elf_Data *data = elf_getdata(scn, NULL);
    if (!data || data->d_size == 0)
    {
        return false;
    }

    // 解析版本需求信息
    size_t offset = 0;

    while (offset < data->d_size)
    {
        // 读取 Elfxx_Verneed 结构
        GElf_Verneed verneed;
        if (!gelf_getverneed(data, offset, &verneed))
        {
            break;
        }

        // 获取库名
        const char *libname = elf_strptr(elf, shdr->sh_link, verneed.vn_file);

        // 检查是否是 libc 相关的库
        if (libname && (strstr(libname, "libc.so") || strstr(libname, "GLIBC_")))
        {
            // 遍历该库的所有版本需求
            size_t aux_offset = offset + verneed.vn_aux;

            for (unsigned int i = 0; i < verneed.vn_cnt; i++)
            {
                GElf_Vernaux vernaux;
                if (!gelf_getvernaux(data, aux_offset, &vernaux))
                {
                    break;
                }

                // 获取版本名
                const char *vername = elf_strptr(elf, shdr->sh_link, vernaux.vna_name);
                if (vername && strncmp(vername, "GLIBC_", 6) == 0)
                {
                    // 提取版本号
                    const char *version_str = vername + 6; // 跳过 "GLIBC_"

                    // 创建符号并比较
                    if (0 > _compare_version(config->glibc_target, version_str))
                    {
                        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
                        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
                        char message[KYSDK_DMVC_LOG_LEN];
                        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, " _check_glibc_version %s", vername);
                        strncat(config->result_log, message, log_avail_len);
                        return true;
                    }
                }

                // 移动到下一个 vernaux 结构
                aux_offset += vernaux.vna_next;
                if (aux_offset == 0 || aux_offset >= data->d_size)
                {
                    break;
                }
            }
        }

        // 移动到下一个 verneed 结构
        offset += verneed.vn_next;
        if (offset == 0 || offset >= data->d_size || verneed.vn_next == 0)
        {
            return false;
        }
    }

    return false;
}

static bool _check_kylin_note_value(Elf *elf, Elf_Scn *scn, GElf_Shdr *shdr, DmvcConfig *config)
{
    const char *value = "v11";
    size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
    size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
    char message[KYSDK_DMVC_LOG_LEN];
    snprintf(message, KYSDK_DMVC_LOG_LEN - 1, " _check_kylin_note_value %s", value);
    strncat(config->result_log, message, log_avail_len);

    return true;
}

static bool _kare_try_install(const char *deb_path, const char *env)
{
    char cmd[PATH_MAX + 32];
    snprintf(cmd, sizeof(cmd) - 1, "kare try-install --env %s %s", env, deb_path);
    FILE *fp = popen(cmd, "r");
    if (fp)
    {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL)
        {
        }

        int state = pclose(fp);
        if ((-1 != state) && WIFEXITED(state))
        {
            int exit_code = WEXITSTATUS(state);
            if (exit_code == 0)
            {
                return true;
            }
        }
    }

    return false;
}

// 检查是否为标准的deb包
static bool _check_is_regular_deb(const char *deb_path)
{
    int fd = open(deb_path, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    // 检查 ar magic header: "!<arch>\n"
    unsigned char header[8];
    if (read(fd, header, 8) != 8)
    {
        close(fd);
        return false;
    }

    // 检查是否为 ar 格式的魔数
    // "!<arch>" 后面跟着换行符 \n，总共8个字节
    unsigned char expected_header[8] = {'!', '<', 'a', 'r', 'c', 'h', '>', '\n'};
    bool result = (memcmp(header, expected_header, 8) == 0);

    close(fd);
    return result;
}

static char *_extract_deb(const char *deb_path, DmvcConfig *config)
{
    // 创建临时目录
    char *extract_path = (char *)calloc(PATH_MAX, sizeof(char));
    if (!extract_path)
    {
        return NULL;
    }

    // 构造解压路径
    snprintf(extract_path, PATH_MAX, "%s/extracted_packageXXXXXX", config->temp_dir);

    // 创建临时目录
    if (mkdtemp(extract_path) == NULL)
    {
        free(extract_path);
        return NULL;
    }

    // 使用dpkg-deb解包
    pid_t pid = fork();
    if (pid == 0)
    {
        // 子进程执行dpkg-deb命令
        execlp("dpkg-deb", "dpkg-deb", "-R", deb_path, extract_path, NULL);
        exit(1);
    }
    else if (pid > 0)
    {
        // 父进程等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            // 命令执行失败
            char cmd[4096] = "\0";
            snprintf(cmd, sizeof(cmd), "rm -rf %s", extract_path);
            system(cmd);
            free(extract_path);
            return NULL;
        }
    }
    else
    {
        // fork失败
        free(extract_path);
        return NULL;
    }

    return extract_path;
}

static int _get_all_elf_files_recursive(const char *path, char ***elf_files, int *count)
{
    DIR *dir;
    struct dirent *entry;

    // 初始化
    *count = 0;
    *elf_files = NULL;
    int capacity = 0;

    // 打开目录
    dir = opendir(path);
    if (!dir)
    {
        return -1;
    }

    // 遍历目录
    while ((entry = readdir(dir)) != NULL)
    {
        // 跳过.和..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // 构造完整路径
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1)
        {
            continue;
        }

        // 如果是目录，递归处理
        if (S_ISDIR(statbuf.st_mode))
        {
            char **sub_elf_files = NULL;
            int sub_count = 0;

            if (_get_all_elf_files_recursive(full_path, &sub_elf_files, &sub_count) == 0)
            {
                // 合并结果
                for (int i = 0; i < sub_count; i++)
                {
                    // 扩容检查
                    if (*count >= capacity)
                    {
                        capacity += 20;
                        char **temp = (char **)realloc(*elf_files, capacity * sizeof(char *));
                        if (!temp)
                        {
                            closedir(dir);
                            _free_elf_files(sub_elf_files, sub_count);
                            return -1;
                        }
                        *elf_files = temp;
                    }

                    (*elf_files)[*count] = sub_elf_files[i];
                    sub_elf_files[i] = NULL; // 避免被释放
                    (*count)++;
                }

                free(sub_elf_files);
            }
        }
        // 如果是普通文件且是ELF文件
        else if (!S_ISLNK(statbuf.st_mode) && S_ISREG(statbuf.st_mode) && _check_is_elf_binary(full_path))
        {
            // 扩容检查
            if (*count >= capacity)
            {
                capacity += 20;
                char **temp = (char **)realloc(*elf_files, capacity * sizeof(char *));
                if (!temp)
                {
                    closedir(dir);
                    return -1;
                }
                *elf_files = temp;
            }

            // 添加文件路径
            (*elf_files)[*count] = (char *)malloc(strlen(full_path) + 1);
            if (!(*elf_files)[*count])
            {
                closedir(dir);
                return -1;
            }
            strcpy((*elf_files)[*count], full_path);
            (*count)++;
        }
    }

    closedir(dir);
    return 0;
}

static bool _check_depends(const char *deb_path, DmvcConfig *config)
{
    char cmd[PATH_MAX + 12], line[4096];
    snprintf(cmd, strnlen(deb_path, PATH_MAX) + 12 + 1, "dpkg-deb -I %s", deb_path);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
    {
        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        char message[KYSDK_DMVC_LOG_LEN];
        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, "Run [%s] fail", cmd);
        strncat(config->result_log, message, log_avail_len);
        syslog(LOG_ERR, "%s", config->result_log);
        return false;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (0 == strncmp(line + 1, "Depends:", 8))
        {
            break;
        }
    }
    pclose(fp);

    // 检查特征依赖
    return _depend_higher_than_sp1(line, "libc6", config);
}

static bool _check_elf_files(const char *const *files, int count, DmvcConfig *config)
{
    int fd = -1;
    Elf *elf = NULL;

    for (int i = 0; i < count; i++)
    {
        fd = _open_elf_file(files[i], &elf);
        if (fd < 0)
        {
            return false;
        }

        size_t shstrndx;
        if (elf_getshdrstrndx(elf, &shstrndx) != 0)
        {
            _close_elf_file(&fd, &elf);
            return false;
        }

        // 遍历所有段
        Elf_Scn *scn = NULL;
        while ((scn = elf_nextscn(elf, scn)) != NULL)
        {
            GElf_Shdr shdr;
            gelf_getshdr(scn, &shdr);

            const char *name = elf_strptr(elf, shstrndx, shdr.sh_name);

            if (NULL == name)
                continue;

            // 判断GLIBC版本
            if (0 == strcmp(name, ".gnu.version_r"))
            {
                if (_check_glibc_version(elf, scn, &shdr, config))
                {
                    _close_elf_file(&fd, &elf);
                    return true;
                }
            }

            // 判断麒麟写入的版本
            if (0 == strcmp(name, ".note.kylin"))
            {
                if (_check_kylin_note_value(elf, scn, &shdr, config))
                {
                    _close_elf_file(&fd, &elf);
                    return true;
                }
            }
        }

        _close_elf_file(&fd, &elf);
    }

    return false;
}

static CheckResult _check_kare_try_install(const char *deb_path, DmvcConfig *config, Score *score)
{
    if (_kare_try_install(deb_path, "shadow"))
    {
        score->v11_score++;
    }

    if (_kare_try_install(deb_path, "v10sp1"))
    {
        score->sp1_score++;
    }

    int ret = _get_result(score);
    switch (ret)
    {
    case CHECK_RESULT_V11:
    {
        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        char message[KYSDK_DMVC_LOG_LEN];
        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, "_check_kare_try_install result V11(Sp1 score %d; v11 score %d)", score->sp1_score, score->v11_score);
        strncat(config->result_log, message, log_avail_len);
    }
    break;
    case CHECK_RESULT_V10SP1:
    {
        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        char message[KYSDK_DMVC_LOG_LEN];
        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, "_check_kare_try_install result Sp1(Sp1 score %d; v11 score %d)", score->sp1_score, score->v11_score);
        strncat(config->result_log, message, log_avail_len);
    }
    break;
    case CHECK_RESULT_UNDEF:
    default:
    {
        size_t log_used_len = strnlen(config->result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        char message[KYSDK_DMVC_LOG_LEN];
        snprintf(message, KYSDK_DMVC_LOG_LEN - 1, "_check_kare_try_install result Unknow(Sp1 score %d; v11 score %d)", score->sp1_score, score->v11_score);
        strncat(config->result_log, message, log_avail_len);
    }
    break;
    }

    return _get_result(score);
}

static char *_kdk_dmvc_get_package_name(const char *path)
{
    if (!path)
        return NULL;

    char cmd[1024] = {0};
    int ret = snprintf(cmd, sizeof(cmd), "dpkg-deb -f %s Package 2>/dev/null", path);
    if (ret < 0 || (size_t)ret >= sizeof(cmd))
        return NULL;

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return NULL;

    char buf[256];
    memset(buf, 0, sizeof(buf));
    if (fgets(buf, sizeof(buf), fp) == NULL)
    {
        pclose(fp);
        return NULL;
    }
    pclose(fp);

    // 剔除换行符
    size_t n = strnlen(buf, sizeof(buf));
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
        buf[--n] = '\0';

    return strdup(buf);
}

CheckResult _kdk_dmvc_check_special_package(const char *deb_path)
{
    if (!deb_path)
        return CHECK_RESULT_UNDEF;

    static const char *sp1_list[] = {
        "stuscreen",
        NULL};
    static const char *v11_list[] = {
        NULL};

    char *package_name = _kdk_dmvc_get_package_name(deb_path);
    if (!package_name)
        return CHECK_RESULT_UNDEF;

    for (int i = 0; sp1_list[i]; i++)
    {
        if (0 == strcmp(package_name, sp1_list[i]))
        {
            free(package_name);
            return CHECK_RESULT_V10SP1;
        }
    }

    for (int i = 0; v11_list[i]; i++)
    {
        if (0 == strcmp(package_name, v11_list[i]))
        {
            free(package_name);
            return CHECK_RESULT_V11;
        }
    }

    free(package_name);
    return CHECK_RESULT_UNDEF;
}

CheckResult _kdk_dmvc_check_os_version(const char *deb_path)
{
    char real_path[PATH_MAX];
    if (!realpath(deb_path, real_path))
    {
        syslog(LOG_ERR, "%s not exists", deb_path);
        return CHECK_RESULT_UNDEF;
    }

    CheckResult special_pkg_result = _kdk_dmvc_check_special_package(real_path);
    if (special_pkg_result != CHECK_RESULT_UNDEF)
    {
        return special_pkg_result;
    }

    pid_t pid = 0;
    int elf_count = 0;

    char *extract_path = NULL;
    char **elf_files = NULL;
    char *debug_env = getenv("KYSDK_DEBUG");

    // bool debug = (debug_env && (0 == strncmp(debug_env, "1", 1))) ? true : false;
    bool debug = true;

    DmvcConfig config;
    _init_config(&config);
    snprintf(config.result_log, KYSDK_DMVC_LOG_LEN, "[kysdk-package] %s:", real_path);

    Score score;
    _init_version_score(&score);

    if (!_check_is_regular_deb(real_path))
    {
        size_t log_used_len = strnlen(config.result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        strncat(config.result_log, "Not a regular deb package", log_avail_len);
        syslog(LOG_ERR, "%s", config.result_log);
        return CHECK_RESULT_UNDEF;
    }

    if (_check_depends(real_path, &config))
    {
        if (debug)
            syslog(LOG_DEBUG, "%s", config.result_log);
        score.v11_score++;
        goto out;
    }

    extract_path = _extract_deb(real_path, &config);
    if (!extract_path)
    {
        size_t log_used_len = strnlen(config.result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        strncat(config.result_log, "Extarct deb package fail", log_avail_len);
        syslog(LOG_ERR, "%s", config.result_log);
        goto out;
    }

    if (_get_all_elf_files_recursive(extract_path, &elf_files, &elf_count) != 0)
    {
        size_t log_used_len = strnlen(config.result_log, KYSDK_DMVC_LOG_LEN - 1);
        size_t log_avail_len = KYSDK_DMVC_LOG_LEN - 1 - log_used_len;
        strncat(config.result_log, "Get ELF file fail", log_avail_len);
        syslog(LOG_ERR, "%s", config.result_log);
        goto out;
    }

    if (NULL != elf_files)
    {
        if (_check_elf_files(elf_files, elf_count, &config))
        {
            if (debug)
                syslog(LOG_DEBUG, "%s", config.result_log);
            score.v11_score++;
            goto out;
        }
    }

    _check_kare_try_install(real_path, &config, &score);

    if (debug)
        syslog(LOG_DEBUG, "%s", config.result_log);

out:
    printf("%s\n", config.result_log);

    if (extract_path)
    {
        pid = fork();
        if (pid == 0)
        {
            execlp("rm", "rm", "-rf", extract_path, NULL);
            exit(1);
        }
        else if (pid > 0)
        {
            wait(NULL);
        }
        free(extract_path);
    }

    for (int i = 0; i < elf_count; i++)
    {
        if (elf_files[i])
            free(elf_files[i]);
    }
    free(elf_files);
    return _get_result(&score);
}
