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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyprocess.h"
#include <cstring-extension.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <linux/param.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <libkylog.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#define PRG_SOCKET_PFX "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2 "[0000]:"
#define PRG_SOCKET_PFX2l (strlen(PRG_SOCKET_PFX2))

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

#define LINE_MAX 4096

#define STAT_FILE    "/proc/stat"
#define COLWID 240
#define UPTIME_FILE "/proc/uptime"
#define MEMINFO "/proc/meminfo"
#define MAX_PF_NAME 1024
#define PROC_DIR "/proc"
#define MAX_PROC_NUM 1024
#define PROC_NUM 1024 * 5
#define BUF_SIZE 1024
static int uptime_fd = -1;
static int stat_fd = -1;
#define LC_NUMERIC __LC_NUMERIC
#define __LC_NUMERIC 1
static char buf[8192];
#define O_RDONLY 00
#define BAD_OPEN_MESSAGE                                           \
    "Error: /proc must be mounted\n"                               \
    "  To mount /proc at boot you need an /etc/fstab line like:\n" \
    "      proc   /proc   proc    defaults\n"                      \
    "  In the meantime, run \"mount proc /proc -t proc\"\n"

#define FILE_TO_BUF(filename, fd)                              \
    do                                                         \
    {                                                          \
        static int local_n;                                    \
        if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) \
        {                                                      \
            fputs(BAD_OPEN_MESSAGE, stderr);                   \
            fflush(NULL);                                      \
            _exit(102);                                        \
        }                                                      \
        lseek(fd, 0L, SEEK_SET);                               \
        if ((local_n = read(fd, buf, sizeof buf - 1)) < 0)     \
        {                                                      \
            perror(filename);                                  \
            fflush(NULL);                                      \
            _exit(103);                                        \
        }                                                      \
        buf[local_n] = '\0';                                   \
    } while (0)

#define SET_IF_DESIRED(x, y) \
    do                       \
    {                        \
        if (x)               \
            *(x) = (y);      \
    } while (0)

struct pstat
{
    long unsigned int utime_ticks;
    long unsigned int stime_ticks;
    long unsigned int cpu_total_time;
};

/* 进程信息 */
typedef struct process_info
{
    long proc_pid;          // 进程id
    char proc_name[128];    // 进程名
    char proc_command[1024]; // 进程启动的命令行,包含启动参数
    char proc_uid[128];     // 进程所属用户　用户uid
    char start_time[128];   // 进程启动的时间 (char * ctime(time_t time) 函数返回值Tue Jul 28 08:50:26 2020)
    long utime;             // 进程cpu使用时间,单位s
    double proc_cpu;        // 进程cpu使用率 %
    double proc_mem;        // 进程mem使用量 %
    char proc_state[20];    // 进程状态
    char proc_time[50];     // 进程运行时间
    char proc_port[20];     // 进程端口号

} process_info;

#define PROCESS_ITEM 14
#define PROC_COMD_NUM 11520
#define PATH_MAX_PROC 4096
#define FREE(ptr)       \
    {                   \
        if(ptr)         \
            free(ptr);  \
        ptr = NULL;     \
    }

static unsigned long lookup_num(char *buffer, const char *key, unsigned long *res)
{
    if (strstartswith(buffer, key) == 0)
    {
        char *val = buffer;
        val += strnlen(key, STRNLEN_MAX) + 1;
        *res = atol(strskipblank(val));
        return 1;
    }
    return 0;
}

static int lookup(char *line, char *pattern, char **value)
{
    char *p, *v;
    int len = strnlen(pattern, STRNLEN_MAX);

    /* pattern */
    if (strncmp(line, pattern, len))
        return 0;

    /* white spaces */
    for (p = line + len; isspace(*p); p++)
        ;

    /* separator */
    if (*p != ':')
        return 0;

    /* white spaces */
    for (++p; isspace(*p); p++)
        ;

    /* value */
    if (!*p)
        return 0;
    v = p;

    /* end of value */
    len = strnlen(line, STRNLEN_MAX) - 1;
    for (p = line + len; isspace(*(p - 1)); p--)
        ;
    *p = '\0';

    *value = strdup(v);
    return 1;
}

static int isnumber(char *c)
{
    char *i = c;

    while (*i != '\0')
    {
        if (!isdigit(*i))
            return 0;
        i++;
    }
    return 1;
}

// static size_t strlcpy(char *dst, const char *src, size_t siz)
// {
//     char *d = dst;
//     const char *s = src;
//     size_t n = siz;

//     /* Copy as many bytes as will fit */
//     if (n != 0 && --n != 0)
//     {
//         do
//         {
//             if ((*d++ = *s++) == 0)
//                 break;
//         } while (--n != 0);
//     }

//     /* Not enough room in dst, add NUL and traverse rest of src */
//     if (n == 0)
//     {
//         if (siz != 0)
//             *d = '\0'; /* NUL-terminate dst */
//         while (*s++)
//             ;
//     }

//     return (s - src - 1); /* count does not include NUL */
// }

// 获取第N项开始的指针
// const char *get_items(const char *buffer, unsigned int item)
// {

//     const char *p = buffer;

//     int len = strlen(buffer);
//     int count = 0;

//     for (int i = 0; i < len; i++)
//     {
//         if (' ' == *p)
//         {
//             count++;
//             if (count == item - 1)
//             {
//                 p++;
//                 break;
//             }
//         }
//         p++;
//     }

//     return p;
// }

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/proc", strnlen("/proc", STRNLEN_MAX));
}

char *kdk_get_process_name(int proc_num)
{
    char path[100] = "\0";
    char *name = NULL;
    char link[64] = "\0";
    char buffer[1025] = "\0";
    snprintf(path, sizeof(path), "/proc/%d/exe", proc_num);

    int count = readlink(path, link, sizeof(link));
    if (count != -1)
    {
        name = calloc(1, strnlen(basename(link), STRNLEN_MAX) + 1);
        if(NULL == name)
            return NULL;
        strncpy(name, basename(link), strnlen(basename(link), STRNLEN_MAX) + 1);
    }
    else
    {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "/proc/%d/status", proc_num);
        FILE *fp = fopen(path, "rt");
        if (!fp)
        {
            free(name);
            return NULL;
        }

        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (lookup(buffer, "Name", &name))
                break;
        }

        fclose(fp);
    }
    return name;
}

int kdk_get_process_id(char *proc_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char path[512] = "\0";
    int i = 0;
    char *realpath_res = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;
    char line[1024] = {0};
    char *name = NULL;
    char link[128] = "\0";
    int res = 0;

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            snprintf(path, sizeof(path), "/proc/%s/exe", ptr->d_name); // 生成要读取的文件的路径
            memset(link, 0, sizeof(link));
            int count = readlink(path, link, sizeof(link));
            if (count != -1)
            {
                if ((strcmp(basename(link), proc_name)) == 0)
                {
                    sscanf(ptr->d_name, "%d", &res);
                }
            }
            else
            {
                memset(path, 0, sizeof(path));
                snprintf(path, sizeof(path), "/proc/%s/status", ptr->d_name);
                if (path_size > 0)
                {
                    canonical_filename = malloc(path_size);
                    if (canonical_filename == NULL)
                    {
                        FREE(canonical_filename);
                        continue;
                    }
                    realpath_res = realpath(path, canonical_filename);
                }
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }

                fp = fopen(canonical_filename, "r");
                if (fp == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                else
                {
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (lookup(line, "Name", &name))
                            break;
                    }
                    FREE(canonical_filename);
                    fclose(fp);
                }

                if ((strcmp(name, proc_name)) == 0)
                {
                    sscanf(ptr->d_name, "%d", &res);
                }
                FREE(name);
            }
        }
        closedir(dir);
    }
    return res;
}

int* kdk_get_process_id_nums(char *proc_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char path[512] = "\0";
    int i = 0;
    char *realpath_res = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;
    char line[1024] = {0};
    char *name = NULL;
    char link[128] = "\0";
    int *res = NULL;

    dir = opendir("/proc");
    if (NULL != dir)
    {
        int *tmp = realloc(res, sizeof(int) * 128);
        if (!tmp)
        {
            closedir(dir);
        }
        res = tmp;
        memset(res, 0, sizeof(int) * 128);

        while ((ptr = readdir(dir)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            snprintf(path, sizeof(path), "/proc/%s/exe", ptr->d_name); // 生成要读取的文件的路径
            memset(link, 0, sizeof(link));
            int count = readlink(path, link, sizeof(link));
            if (count != -1)
            {
                if ((strcmp(basename(link), proc_name)) == 0)
                {
                    int id = 0;
                    sscanf(ptr->d_name, "%d", &id);
                    res[i] = id;
                    i++;
                }
            }
            else
            {
                memset(path, 0, sizeof(path));
                snprintf(path, sizeof(path), "/proc/%s/status", ptr->d_name);
                if (path_size > 0)
                {
                    canonical_filename = malloc(path_size);
                    if (canonical_filename == NULL)
                    {
                        FREE(canonical_filename);
                        continue;
                    }
                    realpath_res = realpath(path, canonical_filename);
                }
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }

                fp = fopen(canonical_filename, "r");
                if (fp == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                else
                {
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (lookup(line, "Name", &name))
                            break;
                    }
                    FREE(canonical_filename);
                    fclose(fp);
                }

                if ((strcmp(name, proc_name)) == 0)
                {
                    int id = 0;
                    sscanf(ptr->d_name, "%d", &id);
                    res[i] = id;
                    i++;
                }
                FREE(name);
            }
            
        }
        closedir(dir);
    }
    return res;
}

unsigned long long Hertz;
long smp_num_cpus;     /* number of CPUs */

static void old_Hertz_hack(void){
  unsigned long long user_j, nice_j, sys_j, other_j, wait_j, hirq_j, sirq_j, stol_j;  /* jiffies (clock ticks) */
  double up_1, up_2, seconds;
  unsigned long long jiffies;
  unsigned h;
  char *savelocale;
  long hz;

#ifdef _SC_CLK_TCK
  if((hz = sysconf(_SC_CLK_TCK)) > 0){
    Hertz = hz;
    return;
  }
#endif

  wait_j = hirq_j = sirq_j = stol_j = 0;
  savelocale = strdup(setlocale(LC_NUMERIC, NULL));
  setlocale(LC_NUMERIC, "C");
  do{
    FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_1);
    /* uptime(&up_1, NULL); */
    FILE_TO_BUF(STAT_FILE,stat_fd);
    sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user_j, &nice_j, &sys_j, &other_j, &wait_j, &hirq_j, &sirq_j, &stol_j);
    FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_2);
    /* uptime(&up_2, NULL); */
  } while((long long)( (up_2-up_1)*1000.0/up_1 )); /* want under 0.1% error */
  setlocale(LC_NUMERIC, savelocale);
  free(savelocale);
  jiffies = user_j + nice_j + sys_j + other_j + wait_j + hirq_j + sirq_j + stol_j ;
  seconds = (up_1 + up_2) / 2;
  h = (unsigned)( (double)jiffies/seconds/smp_num_cpus );
  /* actual values used by 2.4 kernels: 32 64 100 128 1000 1024 1200 */
  switch(h){
  case    9 ...   11 :  Hertz =   10; break; /* S/390 (sometimes) */
  case   18 ...   22 :  Hertz =   20; break; /* user-mode Linux */
  case   30 ...   34 :  Hertz =   32; break; /* ia64 emulator */
  case   48 ...   52 :  Hertz =   50; break;
  case   58 ...   61 :  Hertz =   60; break;
  case   62 ...   65 :  Hertz =   64; break; /* StrongARM /Shark */
  case   95 ...  105 :  Hertz =  100; break; /* normal Linux */
  case  124 ...  132 :  Hertz =  128; break; /* MIPS, ARM */
  case  195 ...  204 :  Hertz =  200; break; /* normal << 1 */
  case  247 ...  252 :  Hertz =  250; break;
  case  253 ...  260 :  Hertz =  256; break;
  case  393 ...  408 :  Hertz =  400; break; /* normal << 2 */
  case  410 ...  600 :  Hertz =  500; break; /* SMP WinNT */
  case  790 ...  808 :  Hertz =  800; break; /* normal << 3 */
  case  990 ... 1010 :  Hertz = 1000; break; /* ARM */
  case 1015 ... 1035 :  Hertz = 1024; break; /* Alpha, ia64 */
  case 1180 ... 1220 :  Hertz = 1200; break; /* Alpha */
  default:
#ifdef HZ
    Hertz = (unsigned long long)HZ;    /* <asm/param.h> */
#else
    /* If 32-bit or big-endian (not Alpha or ia64), assume HZ is 100. */
    Hertz = (sizeof(long)==sizeof(int) || htons(999)==999) ? 100UL : 1024UL;
#endif
    fprintf(stderr, "Unknown HZ value! (%d) Assume %Ld.\n", h, Hertz);
  }
}

int uptime(double *restrict uptime_secs, double *restrict idle_secs)
{
    double up = 0, idle = 0;
    char *savelocale;

    FILE_TO_BUF(UPTIME_FILE, uptime_fd);
    savelocale = strdup(setlocale(LC_NUMERIC, NULL));
    setlocale(LC_NUMERIC, "C");
    if (sscanf(buf, "%lf %lf", &up, &idle) < 2)
    {
        setlocale(LC_NUMERIC, savelocale);
        free(savelocale);
        fputs("bad data in " UPTIME_FILE "\n", stderr);
        return 0;
    }
    setlocale(LC_NUMERIC, savelocale);
    free(savelocale);
    SET_IF_DESIRED(uptime_secs, up);
    SET_IF_DESIRED(idle_secs, idle);
    return up; /* assume never be zero seconds in practice */
}

float kdk_get_process_cpu_usage_percent(int proc_num)
{
    FILE *file = NULL;
    char path[1024] = "\0";
    char buf[1024] = "\0";
    float cpu_usage = 0.0;
     unsigned long long total_time;   /* jiffies used by this process */
    unsigned pcpu = 0;               /* scaled %cpu, 999 means 99.9% */
    unsigned long long seconds;      /* seconds of process life */
    unsigned long long utime;  // user time
    unsigned long long stime;  // kernel time
    unsigned long long cutime; // all user time
    unsigned long long cstime; // all dead time
    unsigned long long start_time;
    char canonical_filename[4096] = "\0";

    time_t seconds_since_boot;
    seconds_since_boot = uptime(0,0);

    // Read process stats
    snprintf(path, sizeof(path), "/proc/%d/stat", proc_num);
    if(!realpath(path, canonical_filename) || !verify_file(canonical_filename))
    {
        return cpu_usage;
    }
    file = fopen(canonical_filename, "r");
    if (file == NULL) {
        return cpu_usage;
    }
    if (fgets(buf, sizeof(buf) - 1, file) == NULL) {
        fclose(file);
        return cpu_usage;
    }
    fclose(file);
    strstripspace(buf);
    
    // Parse the required fields
    sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %llu %llu %llu %llu %*ld %*ld %*d %*ld %llu", &utime, &stime, &cutime, &cstime, &start_time);

    total_time = utime + stime;

    old_Hertz_hack();

    char outbuf[240] = "\0";
    seconds = (((unsigned long long)seconds_since_boot >= (start_time / Hertz)) ? ((unsigned long long)seconds_since_boot - (start_time / Hertz)) : 0);
    if(seconds) pcpu = (total_time * 1000ULL / Hertz) / seconds;
    if (pcpu > 999U)
    {
        snprintf(outbuf, COLWID, "%u", pcpu/10U);
        cpu_usage = atof(outbuf);
    }
    else
    {
        snprintf(outbuf, COLWID, "%u.%u", pcpu/10U, pcpu%10U);
        cpu_usage = atof(outbuf);
    }

    return cpu_usage;
}

float kdk_get_process_mem_usage_percent(int proc_num)
{
    char path[100] = {0};
    float percent = 0.00;
    unsigned long memTotal = 0, memRSS = 0;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/status", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return 0.00;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return 0.00;
        }
    }

    FILE *fp = fopen(canonical_filename, "r");
    if (!fp)
        goto err;

    char buffer[1025];
    char buffers[1025];

    while (fgets(buffer, 1024, fp))
    {
        if (lookup_num(buffer, "VmRSS", &memRSS))
            break;
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);

    FILE *fb = fopen(MEMINFO, "rt");
    if (!fb)
        goto err;
    while (fgets(buffers, 1024, fb))
    {
        if (lookup_num(buffers, "MemTotal", &memTotal))
            break;
    }
    if (memTotal)
        percent = ((float)memRSS / (float)memTotal) * 100;

    fseek(fb, 0, SEEK_SET);
    fclose(fb);
    FREE(canonical_filename);
    return percent;

err:
    FREE(canonical_filename);
    return 0.00;
}

char *kdk_get_process_status(int proc_num)
{
    char path[100] = {0};
    char *name = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/status", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fp = fopen(canonical_filename, "r");
    if (!fp)
        goto err;

    char buffer[1025] = {0};

    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "State", &name))
            break;
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    FREE(canonical_filename);
    return name;

err:
    FREE(canonical_filename);
    return NULL;
}

static int extract_type_1_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (lname[strnlen(lname, STRNLEN_MAX) - 1] != ']')
        return -1;

    {
        char inode_str[strnlen(lname + 1, STRNLEN_MAX)]; /* e.g. "12345" */
        const int inode_str_len = strnlen(lname, STRNLEN_MAX) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname + PRG_SOCKET_PFXl, sizeof(inode_str));
        inode_str[inode_str_len] = '\0';
        *inode_p = strtoul(inode_str, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return -1;
    }
    return 0;
}

static int extract_type_2_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strnlen(lname, STRNLEN_MAX) < PRG_SOCKET_PFX2l + 1)
        return -1;
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l))
        return -1;
    {
        char *serr;
        *inode_p = strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return -1;
    }
    return 0;
}

int kdk_get_process_port(int proc_num)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/fd", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return 0;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return 0;
        }
    }

    dirfd = opendir(canonical_filename);

    if (!dirfd)
    {
        goto err;
    }

    while ((direfd = readdir(dirfd)))
    {
        snprintf(line, sizeof(line), "/proc/%d/fd/%s", proc_num, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;
        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                // sprintf(proc_prot, "%d", local_port);
                fclose(fd);
                closedir(dirfd);
                FREE(canonical_filename);
                return local_port;
            }
            else
            {
                continue;
            }
        }
        fclose(fd);
        FREE(canonical_filename);
    }
    FREE(canonical_filename);
    closedir(dirfd);
    return 0;

err:
    FREE(canonical_filename);
    return 0;
}

int* kdk_get_process_port_nums(int proc_num)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;
    int i = 0;
    int *res = NULL;

    snprintf(path, sizeof(path), "/proc/%d/fd", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    dirfd = opendir(canonical_filename);

    if (!dirfd)
    {
        goto err;
    }

    int *tmp = realloc(res, sizeof(int) * 128);
    if (!tmp)
    {
        closedir(dirfd);
        goto err;
    }
    res = tmp;
    memset(res, 0, sizeof(int) * 128);
    
    while ((direfd = readdir(dirfd)))
    {
        snprintf(line, sizeof(line), "/proc/%d/fd/%s", proc_num, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;

        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                res[i] = local_port;
                i++;
            }
        }
        fclose(fd);
    }
    FREE(canonical_filename);
    closedir(dirfd);

    return res;

err:
    FREE(canonical_filename);
    if(res)
        free(res);
    return NULL;
}

char *kdk_get_process_start_time(int proc_num)
{
    char path[100] = {0};
    char *name = (char *)malloc(sizeof(char) * 128);
    char st[128] = {0};
    if (!name)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1, starttime;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/stat", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(name);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            free(name);
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);

    time_t now;
    now = time(NULL);
    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    starttime = (unsigned long long)(time1 / HZ) + (now - time2.tv_sec) - 0.1;

    time_t time3 = starttime;

    strncpy(st, ctime(&time3), sizeof(st) - 1);
    strstripspace(st);
    strncpy(name, st, 128);
    return name;

err:
    FREE(canonical_filename);
    free(name);
    return NULL;
}

char *kdk_get_process_running_time(int proc_num)
{
    char path[1024] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1;
    unsigned long long seconds;
    char *cp = (char *)calloc(240, sizeof(char));
    if (!cp)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/stat", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(cp);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            free(cp);
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);

    time_t seconds_since_boot = uptime(0, 0);
    seconds = seconds_since_boot - (time1 / 100);

    unsigned dd, hh, mm, ss;
    char buff[256];
    ss = seconds % 60;
    seconds /= 60;
    mm = seconds % 60;
    seconds /= 60;
    hh = seconds % 24;
    seconds /= 24;
    dd = seconds;
    if (dd)
    {
        snprintf(cp, COLWID, "%u-", dd);
    }
    else
    {
        snprintf(cp, COLWID, "%u-", 0);
    }
    if (dd || hh)
    {
        snprintf(buff, COLWID, "%02u:", hh);
        strncat(cp, buff, 240 - strnlen(cp, STRNLEN_MAX) - 1);
    }
    else
    {
        snprintf(buff, COLWID, "%02u:", 0);
        strncat(cp, buff, 240 - strnlen(cp, STRNLEN_MAX) - 1);
    }
    memset(buff, '\0', sizeof(buff));
    snprintf(buff, COLWID, "%02u:%02u", mm, ss);
    strncat(cp, buff, 240 - strnlen(cp, STRNLEN_MAX) - 1);
    return cp;

err:
    free(cp);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_cpu_time(int proc_num)
{
    char path[100] = {0};
    struct pstat result;
    unsigned long utime = 0;
    char *name = (char *)malloc(128 * sizeof(char));
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/stat", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(name);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            free(name);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    // read values from /proc/pid/stat
    bzero(&result, sizeof(struct pstat));

    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*s",
               &result.utime_ticks, &result.stime_ticks) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    utime = (long)((result.utime_ticks + result.stime_ticks) / HZ);
    snprintf(name, 128, "%ld秒", utime);
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);
    return name;

err:
    free(name);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_command(int proc_num)
{
    FILE *fd;
    char path[128] = {0};
    char *command = (char *)malloc(1024 * sizeof(char));
    if (!command)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char line[1024] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (proc_num < 0)
    {
        free(command);
        return NULL;
    }
    else
    {
        snprintf(path, sizeof(path), "/proc/%d/cmdline", proc_num);
    }

    if (strstr(path, "../"))
    {
        free(command);
        return NULL;
    }
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(command);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            free(command);
            return NULL;
        }
    }

    if ((fd = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }

    int c;
    int i = 0;
    while (1)
    {
        char a = ' ';

        c = fgetc(fd);
        if (feof(fd))
            break;
        if (c == '\0')
            line[i] = a;
        else
            line[i] = c;
        i++;
    }
    strncpy(command, line, 1024);
    fseek(fd, 0, SEEK_SET);
    fclose(fd);
    FREE(canonical_filename);
    return command;

err:
    free(command);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_user(int proc_num)
{
    FILE *fpprocstat, *fp;
    char procstatf[128] = {0};
    char line[128] = {0};
    char buff[128] = {0};
    char *name = NULL;
    int j = 0, i = 0;
    char *bufs[30], *q = NULL;
    char *buf[5], *p = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(procstatf, sizeof(procstatf), "/proc/%d/status", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
        goto err;

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "Uid", &name))
            break;
    }
    fseek(fpprocstat, 0, SEEK_SET);
    fclose(fpprocstat);

    char tmp[256] = "";
    strncpy(tmp, name, sizeof(tmp) - 1);
    FREE(name);
    p = strtok(tmp, "	");
    while (p)
    {
        buf[i] = p;
        i++;
        p = strtok(NULL, "	");
    }
    if ((fp = fopen("/etc/passwd", "r")) == NULL)
        goto err;

    while (fgets(buff, sizeof(buff), fp) != NULL)
    {
        if (strstr(buff, buf[1]))
        {
            q = strtok(buff, ":");
            while (q)
            {
                bufs[j] = q;
                j++;
                q = strtok(NULL, ":");
            }

            if (!strcmp(buf[1], bufs[2]))
            {
                name = strdup(bufs[0]);
                break;
            }
        }
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    FREE(canonical_filename);
    return name;
err:
    FREE(canonical_filename);
    return NULL;
}

int *getPidByName(char *task_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char path[512] = "\0";
    char cur_task_name[50];
    char buf[BUF_SIZE];
    int i = 0;
    char *realpath_res = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;
    char *name = NULL;
    char link[128] = "\0";

    int *res = NULL;
    int max_count = 0;

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            if (((max_count - 1)  == i) || (max_count == 0))
            {
                max_count += 5;
                int *tmp = realloc(res, sizeof(int) * max_count);
                if (!tmp)
                {
                    klog_err("内存申请失败:%s\n", strerror(errno));
                    goto err;
                }
                res = tmp;
                memset(res + i, 0, sizeof(int) * (max_count - i));
            }

            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;
            snprintf(path, sizeof(path), "/proc/%s/exe", ptr->d_name); // 生成要读取的文件的路径
            int count = readlink(path, link, sizeof(link));
            if (count != -1)
            {
                if (strstr(basename(link), task_name))
                {
                    sscanf(ptr->d_name, "%d", res + i);
                    i++;
                }
            }
            else
            {
                memset(path, 0, sizeof(path));
                snprintf(path, sizeof(path), "/proc/%s/status", ptr->d_name);
                if (path_size > 0)
                {
                    canonical_filename = malloc(path_size);
                    if (canonical_filename == NULL)
                    {
                        FREE(canonical_filename);
                        continue;
                    }
                    realpath_res = realpath(path, canonical_filename);
                }
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }

                fp = fopen(canonical_filename, "r");
                if (fp == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                else
                {
                    while (fgets(buf, sizeof(buf), fp))
                    {
                        if (lookup(buf, "Name", &name))
                            break;
                    }
                    FREE(canonical_filename);
                }

                if (strstr(name, task_name))
                {
                    sscanf(ptr->d_name, "%d", res + i);
                    i++;
                }
                FREE(name);
            }
        }
        closedir(dir);
    }
    return res;
err:
    free(res);
    closedir(dir);
    free(res);
    return NULL;
}

char **kdk_procname_get_process_infomation(char *proc_name)
{
    int *pid = getPidByName(proc_name);
    if (!pid)
    {
        return NULL;
    }
    char **res = NULL;
    
    char result[PROC_COMD_NUM] = "\0";
    char tmp[PROC_COMD_NUM] = "\0";
    char proc[PROC_COMD_NUM] = "\0";
    float percent = 0.00;
    int num = 0;

    int max_count = 0;
    int index = 0;
    while (pid[index])
    {
        if ((max_count - 1 == index) || (max_count == 0))
        {
            max_count += 5;
            char **tmp = realloc(res, sizeof(char *) * max_count);
            if (!tmp)
            {
                klog_err("内存申请失败:%s\n", strerror(errno));
                goto err;
            }
            res = tmp;
            memset(res + index, 0, sizeof(char *) * (max_count - index));
        }

        memset(result, 0, PROC_COMD_NUM * sizeof(char));
        memset(proc, 0, PROC_COMD_NUM * sizeof(char));

        strncat(result, "proc_pid:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        snprintf(tmp, sizeof(tmp), "%d", pid[index]);
        strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);

        percent = kdk_get_process_cpu_usage_percent(pid[index]);
        strncat(result, "proc_cpu:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        snprintf(tmp, sizeof(tmp), "%0.1f", percent);
        strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);

        percent = kdk_get_process_mem_usage_percent(pid[index]);
        strncat(result, "proc_mem:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        snprintf(tmp, sizeof(tmp), "%0.1f", percent);
        strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);

        char *proc_state = kdk_get_process_status(pid[index]);
        strncat(result, "proc_state:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, proc_state, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        FREE(proc_state);

        num = kdk_get_process_port(pid[index]);
        strncat(result, "proc_port:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        snprintf(tmp, sizeof(tmp), "%d", num);
        strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);

        char *start_time = kdk_get_process_start_time(pid[index]);
        strncat(result, "start_time:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, start_time, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        free(start_time);

        char *proc_time = kdk_get_process_running_time(pid[index]);
        strncat(result, "proc_time:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, proc_time, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        free(proc_time);

        char *cpu_time = kdk_get_process_cpu_time(pid[index]);
        strncat(result, "utime:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, cpu_time, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        free(cpu_time);

        char *proc_command = kdk_get_process_command(pid[index]);
        strncat(result, "proc_command:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, proc_command, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
        free(proc_command);

        char *proc_uid = kdk_get_process_user(pid[index]);
        // strcat(result, "proc_uid:");
        // strcat(result, proc_uid);
        FREE(proc_uid);

        res[index] = strdup(result);
        index++;
    }
    free(pid);
    return res;
err:
    free(pid);
    kdk_proc_freeall(res);
    return NULL;
}

static int get_proc_name(pid_t pid, char *name)
{
    char path[100] = "\0";
    char link[256] = "\0";
    char *tmp = NULL;
    char buffer[1025] = "\0";
    char canonical_filename[PATH_MAX_PROC] = "\0";
    if (pid < 0)
        return -1;
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    int count = readlink(path, link, sizeof(link));
    if (count != -1)
    {
        strncpy(name, basename(link), 128);
    }
    else
    {
        memset(path, 0, sizeof(path)); 
        snprintf(path, sizeof(path), "/proc/%d/status", pid);

        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            return -1;
        }
        FILE *fp = fopen(canonical_filename, "rt");
        if (!fp)
        {
            // free(name);
            return -1;
        }

        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (lookup(buffer, "Name", &tmp))
                break;
        }
        strncpy(name, tmp, 128);
        FREE(tmp);

        fclose(fp);
    }
    return 0;
}

int get_proc_state(pid_t pid, char *state)
{
    FILE *fpprocstat = NULL;
    char procstatf[128] = {0};
    char line[128] = {0};
    char *name = NULL;
    char canonical_filename[PATH_MAX_PROC] = "\0";
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid < 0)
        return -1;
    else
        snprintf(procstatf, sizeof(procstatf), "/proc/%d/status", pid);

    if (strstr(procstatf, "../"))
        return -1;

    if (path_size > 0)
    {
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            return -1;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }
    fseek(fpprocstat, 0, SEEK_SET);

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "State", &name))
            break;
    }
    fclose(fpprocstat);
    strncpy(state, name, 20);
    FREE(name);
    return 0;

err:
    return -1;
}

static int get_proc_command(pid_t pid, char *command)
{
    FILE *fd = NULL;
    char path[128] = {0};
    char canonical_filename[PATH_MAX_PROC] = "\0";
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid < 0)
        return -1;
    else
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

    if (strstr(path, "../"))
        return -1;

    if (path_size > 0)
    {
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            return -1;
        }
    }

    if ((fd = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }
    fseek(fd, 0, SEEK_SET);
    int c;
    int i = 0;
    while (1)
    {
        char a = ' ';

        c = fgetc(fd);
        if (feof(fd))
            break;
        if (c == '\0')
            command[i] = a;
        else
            command[i] = c;
        i++;
    }

    fclose(fd);
    return 0;

err:
    return -1;
}

int get_proc_uid(pid_t pid, char *proc_uid)
{
    FILE *fpprocstat = NULL, *fp = NULL;
    char procstatf[128] = {0};
    char line[128] = {0};
    char buff[128] = {0};
    char *name = NULL;
    char cpname[20] = "\0";
    int j = 0, i = 0;
    char cpbuf[20] = "\0";
    char *bufs[20], *q = NULL;
    char *buf[5], *p = NULL;
    char canonical_filename[PATH_MAX_PROC] = "\0";
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(procstatf, sizeof(procstatf), "/proc/%d/status", pid);
    if (path_size > 0)
    {
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            return -1;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
        goto err;

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "Uid", &name))
            break;
    }
    fseek(fpprocstat, 0, SEEK_SET);
    fclose(fpprocstat);

    p = strtok(name, "	");
    while (p)
    {
        buf[i] = p;
        i++;
        p = strtok(NULL, "	");
    }
    memset(cpname, 0, 20);
    strncpy(cpname, buf[1], 19); 
    if ((fp = fopen("/etc/passwd", "r")) == NULL)
    {
        FREE(name);
        goto err;
    }

    while (fgets(buff, sizeof(buff), fp) != NULL)
    {
        if (strstr(buff, cpname))
        {
            q = strtok(buff, ":");
            while (q)
            {
                bufs[j] = q;
                j++;
                q = strtok(NULL, ":");
            }
            memset(cpbuf, 0, sizeof(cpbuf));
            strncpy(cpbuf, bufs[2], 19);

            if (!strcmp(cpname, cpbuf))
            {
                strncpy(name, bufs[0], 127);
                break;
            }
        }
    }
    fseek(fp, 0, SEEK_SET);
    strncpy(proc_uid, name, 127);
    fclose(fp);
    FREE(name);
    return 0;

err:
    return -1;
}

int get_proc_mem_usage_pct(const pid_t pid, double *pct)
{
    FILE *fpmeminfo = NULL;
    FILE *fpprocstat = NULL;
    double memtotal = 1.0;
    double memproc = 0.0;
    char procstatf[128] = {0};
    char line[128] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid <= 0)
        return -1;

    if ((fpmeminfo = fopen(MEMINFO, "r")) == NULL)
        return -1;
    fseek(fpmeminfo, 0, SEEK_SET);
    while (fgets(line, sizeof(line), fpmeminfo) != NULL)
    {
        if (!strncmp(line, "MemTotal:", 9))
        {
            sscanf(line + 9, "%lf", &memtotal);
            break;
        }
    }
    fclose(fpmeminfo);
    snprintf(procstatf, sizeof(procstatf), "/proc/%d/status", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    fpprocstat = fopen(canonical_filename, "r");
    if (fpprocstat == NULL)
        goto err;
    fseek(fpprocstat, 0, SEEK_SET);
    memset(line, 0, 128);
    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (!strncmp(line, "VmRSS:", 6))
        {
            sscanf(line + 6, "%lf", &memproc);
            break;
        }
    }
    fclose(fpprocstat);
    FREE(canonical_filename);
    *pct = 100 * memproc / memtotal;
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_starttime(pid_t pid, char *start_time)
{
    char stat_filepath[30] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1, starttime;
    char st[128] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid < 0)
    {
        return -1;
    }
    else
        snprintf(stat_filepath, sizeof(stat_filepath), "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
    {
        return -1;
    }

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fclose(fpstat);
    FREE(canonical_filename);

    time_t now;
    now = time(NULL);
    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    starttime = (unsigned long long)(time1 / HZ) + (now - time2.tv_sec);
    time_t time3 = starttime;

    strncpy(st, ctime(&time3), 127);
    strstripspace(st);
    strncpy(start_time, st, 128);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

static long get_proc_cpu_time(pid_t pid)
{
    char stat_filepath[30] = {0};
    struct pstat result;
    long utime = 0;

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid < 0)
        return -1;
    else
        snprintf(stat_filepath, sizeof(stat_filepath), "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
        return -1;
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    bzero(&result, sizeof(struct pstat));

    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*s",
               &result.utime_ticks, &result.stime_ticks) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    utime = (long)((result.utime_ticks + result.stime_ticks) / HZ);
    fclose(fpstat);
    FREE(canonical_filename);
    return utime;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_cpu_pcpu(pid_t pid, double *pcpu)
{
    char stat_filepath[30] = {0};
    struct pstat result;
    unsigned long long time1;
    long cutime, cstime;
    *pcpu = 0;

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    if (pid < 0)
        return -1;
    else
        snprintf(stat_filepath, sizeof(stat_filepath), "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
        return -1;

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    bzero(&result, sizeof(struct pstat));
    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %lu %lu %*d %*d %*d %*d %llu %*u",
               &result.utime_ticks, &result.stime_ticks, &cutime, &cstime, &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    long long t1, t2;
    t1 = (result.utime_ticks + result.stime_ticks) / HZ;
    t2 = time2.tv_sec - time1 / HZ;

    if (t2)
        *pcpu = (double)(100 * t1 / t2);
    fclose(fpstat);
    FREE(canonical_filename);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_running_time(pid_t pid, char *proc_time)
{
    char path[1024] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1;
    unsigned long long seconds;
    char cp[240] = {0};
    char buff[240] = {0};
    if (!buff)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return -1;
    }

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fclose(fpstat);
    FREE(canonical_filename);

    time_t seconds_since_boot = uptime(0, 0);
    seconds = seconds_since_boot - (time1 / 100);

    unsigned dd, hh, mm, ss;
    ss = seconds % 60;
    seconds /= 60;
    mm = seconds % 60;
    seconds /= 60;
    hh = seconds % 24;
    seconds /= 24;
    dd = seconds;
    if (dd)
    {
        snprintf(cp, COLWID, "%u-", dd);
    }
    else
    {
        snprintf(cp, COLWID, "%u-", 0);
    }
    if (dd || hh)
    {
        snprintf(buff, COLWID, "%02u:", hh);
        strncat(cp, buff, sizeof(cp) - strnlen(cp, STRNLEN_MAX) - 1);
    }
    else
    {
        snprintf(buff, COLWID, "%02u:", 0);
        strncat(cp, buff, sizeof(cp) - strnlen(cp, STRNLEN_MAX) - 1);
    }
    memset(buff, 0, sizeof(buff));
    snprintf(buff, COLWID, "%02u:%02u", mm, ss);
    strncat(cp, buff, sizeof(cp) - strnlen(cp, STRNLEN_MAX) - 1);
    strncpy(proc_time, cp, 50);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_port(pid_t pid, char *proc_prot)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    snprintf(path, sizeof(path), "/proc/%d/fd", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    dirfd = opendir(canonical_filename);
    if (!dirfd)
    {
        goto err;
    }

    while ((direfd = readdir(dirfd)))
    {
        snprintf(line, sizeof(line), "/proc/%d/fd/%s", pid, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;
        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                snprintf(proc_prot, 20, "%d", local_port);
            }
            else
            {
                continue;
            }
        }
        fclose(fd);
    }
    FREE(canonical_filename);
    closedir(dirfd);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_process_info(process_info *info, unsigned int *proc_num)
{
    DIR *dir;
    struct dirent *drd;
    struct stat buf;
    process_info *proc_info = info;
    char dirname[MAX_PF_NAME] = {0};

    char canonical_filename[PATH_MAX_PROC] = "\0";
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX_PROC;

    int count = 0;
    *proc_num = 0;
    int i = 0;
    if ((dir = opendir(PROC_DIR)) == NULL)
        return -1;

    while ((drd = readdir(dir)) != NULL)
    {
        if (*proc_num >= MAX_PROC_NUM)
            break;
        if (isnumber(drd->d_name))
        {
            snprintf(dirname, sizeof(dirname), "%s/%s", PROC_DIR, drd->d_name);
            if (path_size > 0)
            {
                if (!realpath(dirname, canonical_filename) || !verify_file(canonical_filename))
                {
                    continue;
                }
            }

            if (stat(canonical_filename, &buf) < 0)
            {
                continue;
            }
            if (!S_ISDIR(buf.st_mode))
            {
                continue;
            }
            i++;
            memset(proc_info, 0, sizeof(process_info));
            sscanf(drd->d_name, "%ld", &(proc_info->proc_pid));
            get_proc_name(proc_info->proc_pid, proc_info->proc_name);

            get_proc_state(proc_info->proc_pid, &(proc_info->proc_state));

            get_proc_command(proc_info->proc_pid, proc_info->proc_command);

            get_proc_uid(proc_info->proc_pid, proc_info->proc_uid);

            get_proc_mem_usage_pct(proc_info->proc_pid, &(proc_info->proc_mem));

            get_proc_starttime(proc_info->proc_pid, proc_info->start_time);

            proc_info->utime = get_proc_cpu_time(proc_info->proc_pid);

            get_proc_cpu_pcpu(proc_info->proc_pid, &(proc_info->proc_cpu));

            get_proc_running_time(proc_info->proc_pid, proc_info->proc_time);

            get_proc_port(proc_info->proc_pid, proc_info->proc_port);

            proc_info++;
            ++count;
        }
    }
    *proc_num = i;
    closedir(dir);

    return 0;
}

char **kdk_get_process_all_information()
{
    process_info pp[PROC_NUM];
    unsigned int proc_num;
    char result[115200] = "\0";
    char tmp[115200] = "\0";
    size_t counts = 0;
    char **res = NULL;
    char **tmpres = NULL;
    int ret = get_process_info(pp, &proc_num);
    if (ret == 0)
    {
        for (int i = 0; i < proc_num; i++)
        {
            memset(result, 0, 115200 * sizeof(char));
            memset(tmp, 0, 115200 * sizeof(char));
            strncat(result, "process_id:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%ld", pp[i].proc_pid);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_name:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, pp[i].proc_name, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_command:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, pp[i].proc_command, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_uid:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%s", pp[i].proc_uid);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "start_time:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, pp[i].start_time, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "utime:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%ld", pp[i].utime);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_cpu:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%.2f", (double)pp[i].proc_cpu);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_mem:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%.2f", (double)pp[i].proc_mem);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_state:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%s", pp[i].proc_state);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_time:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%s", pp[i].proc_time);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, ", ", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            strncat(result, "proc_port:", sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            snprintf(tmp, sizeof(tmp), "%s", pp[i].proc_port);
            strncat(result, tmp, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            tmpres = realloc(res, (proc_num + 3) * sizeof(char *));
            if (!tmpres)
            {
                klog_err("内存申请失败:%s\n", strerror(errno));
                free(res);
                return NULL;
            }
            res = tmpres;
            res[counts] = malloc(115200 * sizeof(char) + 1);
            if (!res[counts])
            {
                free(tmpres);
                klog_err("内存申请失败:%s\n", strerror(errno));
                return NULL;
            }
            strncpy(res[counts], result, 115200);
            counts++;
        }
    }
    res[counts] = NULL;
    return res;
}

inline void kdk_proc_freeall(char **list)
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

inline void kdk_proc_freeid(int *list)
{
    if (!list)
        return;
    free(list);
}

float kdk_get_process_kernel_modes_per(int pid)
{
    FILE *file = NULL;
    char path[128] = "\0";
    char buf[1024] = "\0";
    float ratio = 0.0;
    unsigned long long total_time;   /* jiffies used by this process */
    unsigned long long utime;  // user time
    unsigned long long stime;  // kernel time

    // Read process stats
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file == NULL) {
        return ratio;
    }
    if (fgets(buf, sizeof(buf) - 1, file) == NULL) {
        fclose(file);
        return ratio;
    }
    fclose(file);
    strstripspace(buf);
    
    // Parse the required fields
    sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %llu %llu", &utime, &stime);
    total_time = utime + stime;
    ratio = (float) stime / total_time;
    return ratio;
}

float kdk_get_process_user_modes_per(int pid)
{
    FILE *file = NULL;
    char path[128] = "\0";
    char buf[1024] = "\0";
    float ratio = 0.0;
    unsigned long long total_time;   /* jiffies used by this process */
    unsigned long long utime;  // user time
    unsigned long long stime;  // kernel time

    // Read process stats
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file == NULL) {
        return ratio;
    }
    if (fgets(buf, sizeof(buf) - 1, file) == NULL) {
        fclose(file);
        return ratio;
    }
    fclose(file);
    strstripspace(buf);
    
    // Parse the required fields
    sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %llu %llu", &utime, &stime);
    total_time = utime + stime;
    ratio = (float) utime / total_time;
    return ratio;
}