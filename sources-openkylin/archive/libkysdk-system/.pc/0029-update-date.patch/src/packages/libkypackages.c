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

#define LIBDPKG_VOLATILE_API
#include "libkypackages.h"
#include <libkylog.h>
#include <kysdk/kysdk-base/cstring-extension.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <ctype.h>
#include <dbus-1.0/dbus/dbus.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <libgen.h>
#include <libkylog.h>
#include <libkysettings.h>
#include <syslog.h>
#include <pwd.h>
#include <pthread.h>
#include <glib.h>
#include <sys/utsname.h>
#include <curl/curl.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/file.h>
#include <poll.h>
#include <openssl/sha.h>
#include "dmvc/libkydmvc.h"
#include <sys/wait.h>

typedef unsigned long long ns_mode_session_id_t;

#define OSTREE_MODE_LIB                "/usr/lib/ostreemode/libostreemode.so"
#define NS_MODE_ENTER_HANDLE         "ns_mode_enter"
#define NS_MODE_EXIT_HANDLE        "ns_mode_exit"

typedef ns_mode_session_id_t (*handle_ns_mode_enter)(void);
typedef int (*handle_ns_mode_exit)(ns_mode_session_id_t session_id);

#define KDK_DEKSTOP_FILE_PATH "/usr/share/applications/"
#define KDK_LOCALE_DEKSTOP_FILE_PATH "/.local/share/applications/"
#define KDK_SNAPD_DEKSTOP_FILE_PATH "/var/lib/snapd/desktop/applications/"
#define KDK_FLATPAK_DEKSTOP_FILE_PATH "/var/lib/flatpak/exports/share/applications/"
#define KAIMING_REPO_CONFIG_FILE "/opt/kaiming/repo/config"
#define PROGRAM_SIZE 1024

#define KDK_KARE_INSTALL_LOCK "/var/lock/kare-install.lock"
#define KDK_KAIMING_INSTALL_LOCK "/opt/kaiming/info/pkgmanager.lock"

#ifdef __linux__
#define DPKG_DB "/var/lib/dpkg/status"
#endif

#define PACKAGE_INCREASE_STEP 50

#ifndef K_MAX_PATH
#define K_MAX_PATH 4096
#endif //K_MAX_PATH

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

#define CHUNK 4096

typedef enum _kdk_boot_mode
{
    MODE_NONE = 0,          // 系统类型未知
    MODE_MAINTAIN,          // 运维模式
    MODE_NORMAL             // 用户模式
} kdk_boot_mode;

#define MAX_PATH_LEN 1024

typedef struct {
    const char *kdk_real_path;  //全路径
    const char *kdk_path;       //转换后路径
} kdk_path_remap_rule;

typedef struct {
    char param_name[128];
    FILE *param_fp;
} ThreadArgs;

#define BUFFER_SIZE 8192

#define MAX_PKGS 1024        //包的最大数量
#define MAX_NAME_LEN 1024    //包名长度

// 结构体存储data.tar信息
typedef struct {
    off_t offset;     // 偏移量
    off_t size;       // 文件大小
    off_t total_size; // 偏移量 + 大小 = 前半段总长度
} DebInfo;

// 函数声明
static int execute_ar_command(const char* deb_file, char* output, size_t output_size);
static DebInfo parse_ar_output(const char* ar_output);
static int calculate_deb_sha512(const char* deb_file, off_t total_size, unsigned char* hash);
static bool key_trust_is_trusted();
static int generate_signature_base64(const char *data, char *sig);

int kdk_package_is_installed(const char *name, const char *version);

static bool kdk_package_is_kaiming_online_installable(const char *name, const char *version);

static bool kaiming_repo_accessible();
static int kdk_package_deb_is_installed(const char *name, const char *version);
static int kdk_package_kare_is_installed(const char *name, const char *version);
static int kdk_package_kaiming_is_installed(const char *name, const char *version);
static int kdk_kare_remove(const char *pkg_name, RemoveCallBack call_back);
static int kdk_kaiming_remove(const char *pkg_name, RemoveCallBack call_back);
static int kdk_debian_remove(const char *pkg_name, RemoveCallBack call_back);
static bool kdk_create_desktop_link(const char *uri);
static int extract_hostname(const char *url, char *hostname, size_t size);
static bool test_connectivity(const char *url);
static void setup_signal_handlers();
static ns_mode_session_id_t kdk_ns_mode_enter();
static int kdk_ns_mode_exit(ns_mode_session_id_t session_id);
static atomic_int g_verify_end_count = 1;
int kdk_ns_mode_session_id = 0;
int kdk_ns_mode_flag_enter = 0;
static char *kdk_get_package_name(char *uri);
static int kdk_get_direct_install_whitelist(char *file_name);
static char* kdk_download_package(char *pkg_name, const char *dir);

static int privilege_tag = 0;           //是否为特权标签软件包，1为是，0为不是
static void kdk_set_privileged_tag_packag(char *pkg_name);
static int kdk_get_privileged_tag_packag(char *pkg_name);

static int kdk_debian_offline_install_in_whitelist(char *uri, char *value, char *file_name, InstallCallBack call_back);       //白名单离线安装验证
static bool kdk_get_whitelist_runtime_check();      //获取白名单是否检查runtime_version开关
static int kdk_is_privilege_label(char *dir, char *uri, char *type);        //是否存在特权标签，返回1是，0为不是
static bool kdk_get_privilege_runtime_check();      //获取特权标签是否检查runtime_version开关
static int kdk_debian_offline_install_in_privilege(char *dir, char *uri, char *value, InstallCallBack call_back);      //特权标签离线安装验证
static int kdk_debian_online_install_in_whitelist(char *uri, char *value, char *file_name, InstallCallBack call_back);     //白名单在线安装验证
static int kdk_debian_online_install_in_privilege(char *dir, char *uri, char *value, char *file_name, InstallCallBack call_back);       //特权标签在线安装验证
static bool kdk_package_is_deb_online_installable(const char *name, const char *version);               //deb包是否可在线安装
static char* get_val_from_file(FILE *fp, const char *key);
static bool kdk_get_pkg_sign_check();               //特权包是否强制开启验证麒麟签名

//软件包检测状态

typedef enum _kdk_deb_verify_mode
{
    VERIFY_MODE_UNKNOWN = -1,     // 未知
    VERIFY_MODE_ON,          // 阻止安装
    VERIFY_MODE_WARNING,     // 警告提示
    VERIFY_MODE_OFF          // 关闭检测项
} kdk_deb_verify_mode;

static kdk_deb_verify_mode s_deb_verify_mode = VERIFY_MODE_UNKNOWN;

typedef enum _kdk_lock_status {
    UNLOCKED,       // 未锁定
    LOCKED,         // 已被锁定
    ERROR           // 检查出错
} kdk_lock_status;

kdk_lock_status check_lock_status(const char* lock_file) {
    int fd = open(lock_file, O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT) {
            return UNLOCKED;  // 文件不存在，肯定没锁
        }
        return ERROR;
    }
    
    // 尝试获取非阻塞共享锁（检查读锁）
    if (flock(fd, LOCK_SH | LOCK_NB) == 0) {
        flock(fd, LOCK_UN);  // 立即释放
        // 再尝试排他锁（检查写锁）
        if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
            flock(fd, LOCK_UN);
            close(fd);
            return UNLOCKED;  // 两种锁都能获取，说明没锁
        } else if (errno == EWOULDBLOCK) {
            close(fd);
            return LOCKED;  // 有写锁
        }
    } else if (errno == EWOULDBLOCK) {
        close(fd);
        return LOCKED;  // 有读锁或写锁
    }
    
    close(fd);
    return ERROR;
}

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

int extract_hostname(const char *url, char *hostname, size_t size) {
    const char *start = url;
    const char *end;
    
    // 检查并跳过http://前缀
    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
    }
    // 检查并跳过https://前缀
    else if (strncmp(url, "https://", 8) == 0) {
        start = url + 8;
    }
    
    // 查找路径分隔符(/)或其他特殊字符
    end = start;
    while (*end != '\0' && *end != '/' && *end != ':' && *end != '?') {
        end++;
    }
    
    // 计算主机名长度
    size_t len = end - start;
    if (len >= size) {
        fprintf(stderr, "Hostname too long\n");
        return -1;
    }
    
    // 复制主机名
    strncpy(hostname, start, len);
    hostname[len] = '\0';
    
    return 0;
}

/**
 * 从 URL 中提取端口号
 * @param url 完整的 URL
 * @return 端口号，如果未指定则返回默认端口 (HTTP:80, HTTPS:443)
 */
static int extract_port_from_url(const char *url) {
    if (!url) return 80;
    
    // 检查是否为 HTTPS
    bool is_https = (strncmp(url, "https://", 8) == 0);
    int default_port = is_https ? 443 : 80;
    
    // 跳过协议部分
    const char *host_start = strstr(url, "://");
    if (!host_start) return default_port;
    host_start += 3;
    
    // 查找端口分隔符 ':'
    const char *port_sep = strchr(host_start, ':');
    if (!port_sep) return default_port;
    
    // 确保冒号在路径 '/' 之前
    const char *path_start = strchr(host_start, '/');
    if (path_start && port_sep > path_start) return default_port;
    
    // 解析端口号
    int port = atoi(port_sep + 1);
    if (port <= 0 || port > 65535) return default_port;
    
    return port;
}

/**
 * 测试主机连通性的函数
 * @param url 要测试的 URL 或主机名
 * @return 连接状态：0 表示成功，非 0 表示失败
 */
bool test_connectivity(const char *url) {
    struct sockaddr_in server_addr;
    struct hostent *host;
    int sock;
    struct timeval start_time, end_time;
    long elapsed_time;
    char hostname[256];
    int port;
    
    // 从 URL 中提取端口号
    port = extract_port_from_url(url);
    
    // 从 URL 中提取主机名
    if (extract_hostname(url, hostname, sizeof(hostname)) != 0) {
        return false;
    }
    
    syslog(LOG_DEBUG, "Testing connectivity to: %s:%d (extracted from %s)\n", hostname, port, url);
    
    // 创建 TCP 套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return false;
    }
    
    // 设置超时时间
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    
    // 解析主机名
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // 判断是 IP 地址还是域名
    if (inet_aton(hostname, &server_addr.sin_addr) == 0) {
        // 是域名，需要解析
        syslog(LOG_DEBUG, "Resolving hostname: %s\n", hostname);
        host = gethostbyname(hostname);
        if (host == NULL) {
            herror("Failed to resolve hostname");
            close(sock);
            return false;
        }
        
        // 显示解析到的 IP 地址
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, host->h_addr_list[0], ip_str, INET_ADDRSTRLEN);
        syslog(LOG_DEBUG, "Resolved to IP: %s\n", ip_str);
        
        memcpy(&server_addr.sin_addr, host->h_addr_list[0], host->h_length);
    } else {
        syslog(LOG_DEBUG, "Using IP address: %s\n", hostname);
    }
    
    // 获取开始时间
    gettimeofday(&start_time, NULL);
    
    // 尝试连接
    syslog(LOG_INFO, "Trying to connect to %s:%d...\n", hostname, port);
    int result = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    // 获取结束时间
    gettimeofday(&end_time, NULL);
    
    // 计算耗时（毫秒）
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000;
    elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000;
    
    if (result == 0) {
        syslog(LOG_INFO, "Success! Connected to %s:%d in %ld ms\n", hostname, port, elapsed_time);
    } else {
        syslog(LOG_ERR, "Failed to connect");
        syslog(LOG_ERR, "Failed to connect to %s:%d (Time elapsed: %ld ms)\n", hostname, port, elapsed_time);
    }
    
    close(sock);
    return result == 0 ? true : false;
}

/*
1. 调用/usr/bin/getsignstatus查取当前的状态，获取终端输出并记录
2. 调用/usr/bin/setsignstatus on 设置非签名包阻止模式
3. 调用/usr/bin/chattr +i /etc/dpkg/dpkg.cfg 锁定文件，防止装包过程被篡改
4. 返回值1代表成功，0代表getsignstatus失败，-1代表/etc/dpkg/dpkg.cfg失败
5. 断电后无法恢复/etc/dpkg/dpkg.cfg文件状态，影响软件包安装，所以去掉/etc/dpkg/dpkg.cfg文件的相关操作
*/
static int kdk_deb_verify_start()
{
    syslog(LOG_DEBUG, "kdk_deb_verify_start calling...");
    
    // 异常判断
    if(access("/usr/bin/getsignstatus", F_OK) != 0)
    {
        syslog(LOG_ERR, "getsignstatus not found!");
        return 0;
    }
    if(access("/usr/bin/setsignstatus", F_OK) != 0)
    {
        syslog(LOG_ERR, "setsignstatus not found!");
        return 0;
    }
    
    // 记录状态
    FILE *fp = popen("LC_ALL=en.US /usr/bin/getsignstatus 2>&1", "r");
    if (NULL != fp)
    {
        char line[1024] = {0};
        while (fgets(line, sizeof(line), fp))
        {
            strstripspace(line);
            if(strcmp(line, "kylin sign status : off") == 0)
            {
                s_deb_verify_mode = VERIFY_MODE_OFF;
                syslog(LOG_DEBUG, "kylin sign status : off");
            }
            else if(strcmp(line, "kylin sign status : warning") == 0)
            {
                s_deb_verify_mode = VERIFY_MODE_WARNING;
                syslog(LOG_DEBUG, "kylin sign status : warning");
            }
            else if(strcmp(line, "kylin sign status : on") == 0)
            {
                s_deb_verify_mode = VERIFY_MODE_ON;
                syslog(LOG_DEBUG, "kylin sign status : on");
            }
            else
            {
                s_deb_verify_mode = VERIFY_MODE_UNKNOWN;
                syslog(LOG_ERR, "kylin sign status : unknown");
            }
        }
        pclose(fp);
    }

    // 2.设置非签名包阻止模式
    system("/usr/bin/setsignstatus on > /dev/null 2>&1");

    syslog(LOG_DEBUG, "kdk_deb_verify_start called...");
    return 1;
}


/*
1. 调用/usr/bin/chattr -i /etc/dpkg/dpkg.cfg 解锁文件
2. 调用/usr/bin/setsignstatus on/off/warninig 恢复原本的状态
3. 断电后无法恢复/etc/dpkg/dpkg.cfg文件状态，影响软件包安装，所以去掉/etc/dpkg/dpkg.cfg文件的相关操作
 */
static void kdk_deb_verify_end()
{
    syslog(LOG_DEBUG, "kdk_deb_verify_end calling...");

    //2. 调用/usr/bin/setsignstatus on/off/warninig 恢复原本的状态
    if(s_deb_verify_mode == VERIFY_MODE_ON)
        system("/usr/bin/setsignstatus on > /dev/null 2>&1");
    else if (s_deb_verify_mode == VERIFY_MODE_OFF)
    {
        system("/usr/bin/setsignstatus off > /dev/null 2>&1");
    }
    else if (s_deb_verify_mode == VERIFY_MODE_WARNING)
    {
        system("/usr/bin/setsignstatus warning > /dev/null 2>&1");
    }
    syslog(LOG_DEBUG, "kdk_deb_verify_end called");
}

// SIGINT 处理器 - Ctrl+C
static void sigint_handler(int sig, siginfo_t *info) {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d, 信号 %d: SIGINT] 收到中断请求，准备优雅退出...\n", current_count, sig);
    
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

// SIGTERM 处理器 - kill 命令默认
static void sigterm_handler(int sig) {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d, 信号 %d: SIGTERM] 收到终止请求，进行清理...\n", g_verify_end_count, sig);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

// SIGQUIT 处理器 - Ctrl+ '\'
static void sigquit_handler(int sig) {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d, 信号 %d: SIGQUIT] 收到退出请求，生成调试信息...\n", g_verify_end_count, sig);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

// SIGHUP 处理器 - 终端断开/配置重载
static void sighup_handler(int sig) {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d, 信号 %d: SIGHUP] 配置重载请求\n", g_verify_end_count, sig);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

// SIGTSTP 处理器 - Ctrl+Z
static void sigtstp_handler(int sig) {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d, 信号 %d: SIGTSTP] 进程被暂停\n", g_verify_end_count, sig);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/", strnlen("/", STRNLEN_MAX));
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

/**
 * 通配符匹配函数
 * @param pattern 模式字符串（可以包含*通配符）
 * @param str 要匹配的字符串
 * @return 匹配成功返回true，失败返回false
 */
bool wildcard_match(const char *pattern, const char *str) {
    const char *pattern_ptr = pattern;
    const char *str_ptr = str;
    const char *pattern_backup = NULL;
    const char *str_backup = NULL;
    
    while (*str_ptr != '\0') {
        if (*pattern_ptr == '*') {
            // 跳过连续的*
            while (*pattern_ptr == '*') {
                pattern_ptr++;
            }
            
            // 如果*是模式串的结尾，匹配所有剩余的字符
            if (*pattern_ptr == '\0') {
                return true;
            }
            
            // 备份当前位置，用于回溯
            pattern_backup = pattern_ptr;
            str_backup = str_ptr;
        } else if (*pattern_ptr == '?' || *pattern_ptr == *str_ptr) {
            // 单个字符匹配或精确匹配
            pattern_ptr++;
            str_ptr++;
        } else if (pattern_backup != NULL) {
            // 不匹配但有*，回溯
            pattern_ptr = pattern_backup;
            str_ptr = ++str_backup;
        } else {
            // 不匹配且没有*可用
            return false;
        }
    }
    
    // 跳过模式串末尾的*
    while (*pattern_ptr == '*') {
        pattern_ptr++;
    }
    
    // 如果模式串也到末尾，匹配成功
    return *pattern_ptr == '\0';
}

static inline char *read_key(char *buffer, const char *key)
{
    if (strstartswith(buffer, key) == 0)
    {
        buffer += strnlen(key, STRNLEN_MAX);
        strstripspace(buffer);
        return buffer;
    }
    return NULL;
}

static int get_desktop_info(char *dir_path, kdk_app_info *programlist, int start_app)
{
    char canonical_filename[4096] = {"\0"};
    if (!realpath(dir_path, canonical_filename) || !verify_file(canonical_filename))
    {
        return 0;
    }
    DIR *dir = opendir(canonical_filename);
    struct dirent *dir_t = NULL;
    if (dir != NULL)
    {
        while ((dir_t = readdir(dir)) != NULL)
        {
            if ((strcmp(dir_t->d_name, ".") == 0) || (strcmp(dir_t->d_name, "..") == 0))
                continue;
            if (DT_DIR == dir_t->d_type)
                continue;

            kdk_app_t *curinfo = NULL;
            if (strstr(dir_t->d_name, ".desktop"))
            {
                char fpath[PROGRAM_SIZE] = {0};
                snprintf(fpath, sizeof(fpath), "%s/%s", canonical_filename, dir_t->d_name);

                int is_start_menu_app = 1;
                char name[128] = {"None"};
                char version[128] = {"None"};
                char company[128] = {"None"};
                char cmd[128] = {"None"};
                char param[128] = {"None"};
                char icon[128] = {"None"};
                char buffer[PROGRAM_SIZE] = {0};

                char filename[4096] = {"\0"};
                if (!realpath(fpath, filename) || !verify_file(filename))
                {
                    return 0;
                }

                FILE *fp = fopen(filename, "r");
                if (NULL == fp)
                {
                    return 0;
                }

                while (fgets(buffer, PROGRAM_SIZE, fp))
                {
                    char *val = NULL;
                    if ((val = read_key(buffer, "Name=")))
                    {
                        strncpy(name, val, sizeof(name) - 1);
                    }

                    if ((val = read_key(buffer, "Version=")))
                    {
                        strncpy(version, val, sizeof(version) - 1);
                    }

                    if ((val = read_key(buffer, "Company=")))
                    {
                        strncpy(company, val, sizeof(company) - 1);
                    }

                    if ((val = read_key(buffer, "Exec=")))
                    {
                        char *p = val;
                        char tmp[128] = "\0";
                        strncpy(tmp, val, sizeof(tmp) - 1);
                        char *token = strsep(&p, " ");
                        if (strcmp(token, "sudo") == 0)
                        {
                            strncpy(cmd, tmp, sizeof(cmd) - 1);
                        }
                        else
                        {
                            strncpy(cmd, token, sizeof(cmd) - 1);
                            if (p != NULL)
                                strncpy(param, p, sizeof(param) - 1);
                        }
                    }

                    if ((val = read_key(buffer, "Icon=")))
                    {
                        strncpy(icon, val, sizeof(icon) - 1);
                    }

                    // 判断是否是显示在开始菜单的应用程序
                    if ((val = read_key(buffer, "NoDisplay=")))
                    {
                        if (0 == strcmp(val, "true") && start_app)
                        {
                            is_start_menu_app = 0;
                        }
                    }
                }
                fclose(fp);

                if (is_start_menu_app)
                {
                    kdk_app_t **tmp = (kdk_app_t **)realloc(programlist->list, PROGRAM_SIZE * sizeof(kdk_app_t *));
                    if (!tmp)
                        return 0;

                    curinfo = (kdk_app_t *)calloc(1, sizeof(kdk_app_t));
                    if (!curinfo)
                        return 0;

                    programlist->list = tmp;
                    programlist->list[programlist->nums] = curinfo;
                    programlist->nums++;

                    curinfo->name = strdup(name);
                    curinfo->version = strdup(version);
                    curinfo->company = strdup(company);
                    curinfo->cmd = strdup(cmd);
                    curinfo->param = strdup(param);
                    curinfo->icon = strdup(icon);
                }
            }
        }
        closedir(dir);
    }
    return 1;
}

static char *read_default_app(const char *filetype)
{
    char filename[K_MAX_PATH] = "\0";
    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    snprintf(filepath, sizeof(filepath), "%s/.config/mimeapps.list", homeDir);
    if(!realpath(filepath, filename) || !verify_file(filename))
    {
        ;
    }
    FILE *fp = fopen(filename, "r");
    char line[1024], *val = NULL;
    if (NULL != fp)
    {
        while (fgets(line, 1024, fp))
        {
            if ((val = lookup(line, filetype)))
                break;
        }
        fclose(fp);
    }

    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/ukui-mimeapps.list", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }
    if (NULL == val)
    {
        fp = fopen("/usr/share/applications/mimeinfo.cache", "r");
        if (NULL != fp)
        {
            while (fgets(line, 1024, fp))
            {
                if ((val = lookup(line, filetype)))
                    break;
            }
            fclose(fp);
        }
    }

    if (NULL == val)
        return strdup("None");

    strstrip(val, ';');
    char desktop_file[1024] = "\0";
    const char *app_dirs[] = { "/usr/share/applications",
                               "/opt/kare/usr/share/applications",
                               "/opt/kaiming/share/applications" };
    char candidate[1024] = {0};
    char canonical_filename[K_MAX_PATH] = {"\0"};
    int found = 0;

    for (size_t i = 0; i < sizeof(app_dirs)/sizeof(app_dirs[0]); i++) {
        snprintf(candidate, sizeof(candidate), "%s/%s", app_dirs[i], val);
        if (access(candidate, F_OK) == 0) {
            if (realpath(candidate, canonical_filename) && verify_file(canonical_filename)) {
                found = 1;
                break;
            }
        }
    }

    if (!found)
        return NULL;

    fp = fopen(canonical_filename, "r");
    if (NULL == fp)
        return NULL;

    while (fgets(line, 1024, fp))
    {
        val = line;
        if (strstartswith(val, "Name=") == 0)
        {
            val += strnlen("Name=", STRNLEN_MAX);
            strstripspace(val);
            break;
        }
    }
    fclose(fp);

    return val ? strdup(val) : NULL;
}

kdk_package_list *kdk_package_get_packagelist()
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    kdk_package_list *packagelist = (kdk_package_list *)calloc(1, sizeof *packagelist);
    if (!packagelist)
        goto err_out;

    char buffer[4097];
    kdk_package_t *curpackage = NULL;
    size_t maxpackage = 0;
    do
    {
        if (!fgets(buffer, 4096, fp))
            continue;
        char *val;
        if ((val = lookup(buffer, "Package:")))
        {
            curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
            if (!curpackage)
                goto err_out;
            curpackage->name = strdup(val);
            if (packagelist->nums == maxpackage)
            {
                maxpackage += PACKAGE_INCREASE_STEP;
                kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
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
            curpackage->status = summarize_pkg_status(val);
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
        else if ((val = lookup(buffer, "Depends")))
        {
        }
    } while (feof(fp) == 0);

    fclose(fp);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_info("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getAllLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        // 释放message
        dbus_message_unref(message);
        message = NULL;

        // 解析回复消息
        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        {
            klog_err("DBus reply parsing failed\n");
            goto out;
        }

        // 释放reply
        dbus_message_unref(reply);
        reply = NULL;

        DBusMessageIter array_iter;
        char *str = NULL;

        // 进入数组
        dbus_message_iter_recurse(&iter, &array_iter);

        // 遍历所有字符串
        while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) {
            dbus_message_iter_get_basic(&array_iter, &str);
            klog_info("Got string: %s\n", str);
            if(str)
            {
                char name[256], id[256], branch[64], module[64], version[64], size[128], description[1024];
                int count = sscanf(line, "%s", name);
                if (count != 1)
                    continue;
                if (!isalpha((unsigned char)name[0])) // 第一行表头
				{
                    continue;
				}

                curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
                if (!curpackage)
                    goto err_out;
                if (packagelist->nums == maxpackage)
                {
                    maxpackage += PACKAGE_INCREASE_STEP;
                    kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                    if (!tmp)
                    {
                        free(curpackage);
                        goto err_out;
                    }
                    packagelist->list = tmp;
                }
                packagelist->list[packagelist->nums] = curpackage;
                packagelist->nums++;

                curpackage->name = strdup(name);
                // curpackage->version = strdup(version);

                // char numericStr[20];
                // int i = 0, j = 0;

                // 从字符串中提取数字部分到 numericStr
                // while ('\0' != size[i])
                // {
                //     if (isdigit(size[i]))
                //         numericStr[j++] = size[i];
                //     i++;
                // }
                // numericStr[j] = '\0';

                // if (0 == strendwith(size, "B"))
                //     curpackage->size_kb = atol(numericStr) / 1024;
                // if (0 == strendwith(size, "MB"))
                //     curpackage->size_kb = atol(numericStr) * 1024;
                // if (0 == strendwith(size, "GB"))
                //     curpackage->size_kb = atol(numericStr) * 1024 * 1024;
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char name[256], version[64];
                int count = sscanf(line, "%s %s", name, version);
                if (count != 2)
                    continue;
                if (0 == strcmp(name, "软件包名")) // 第一行表头
                    continue;
                if (name[0] == '=')
                {
                    continue;
                }

                curpackage = (kdk_package_t *)calloc(1, sizeof(kdk_package_t));
                if (!curpackage)
                    goto err_out;
                if (packagelist->nums == maxpackage)
                {
                    maxpackage += PACKAGE_INCREASE_STEP;
                    kdk_package_t **tmp = (kdk_package_t **)realloc(packagelist->list, maxpackage * sizeof(kdk_package_t *));
                    if (!tmp)
                    {
                        free(curpackage);
                        goto err_out;
                    }
                    packagelist->list = tmp;
                }
                packagelist->list[packagelist->nums] = curpackage;
                packagelist->nums++;

                curpackage->name = strdup(name);
                curpackage->version = strdup(version);
            }
        }
        pclose(fp);
    }

    return packagelist;

err_out:
#ifdef __linux__
    fclose(fp);
#endif
    kdk_package_free_packagelist(packagelist);
    return NULL;
}

static void _free_package_t(kdk_package_t *package)
{
    if (package->name)
        free(package->name);
    if (package->section)
        free(package->section);
    if (package->version)
        free(package->version);
    if (package->status)
        free(package->status);
    if (package->depends)
        free(package->depends);
}

void kdk_package_free_packagelist(kdk_package_list *pl)
{
    if (!pl)
        return;

    if (pl->list)
    {
        for (int i = 0; i < pl->nums; i++)
        {
            _free_package_t(pl->list[i]);
            free(pl->list[i]);
        }
        free(pl->list);
    }
    free(pl);
}

InstallMethod kdk_package_get_installation_method(const char *name, const char *version)
{
    if (!name || !g_utf8_validate(name, -1, NULL))
    {
        syslog(LOG_ERR, "Invalid UTF-8 string: %s", name);
        return NONE_METHOD;
    }

    const char *dpkg = "dpkg -l";
    FILE *fp = popen(dpkg, "r");
    if (NULL != fp)
    {
        char line[1024] = {0};
        while (fgets(line, sizeof(line), fp))
        {
            char tmp_name[256] = {0}, tmp_version[64] = {0}, tmp_arch[64] ={0};
            // 修复：添加缓冲区大小限制
            int count = sscanf(line, "%*s %255s %63s %s %*s", tmp_name, tmp_version, tmp_arch);
            if (count != 3)
                continue;
                
            if (version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
            {
                if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                {
                    pclose(fp);
                    if (strstr(tmp_arch, "kare_"))
                        return KARE_METHOD;
                    if (strstr(name, "kwre"))
                        return KWRE_METHOD;
                    else if (strstr(name, "kmre"))
                        return KMRE_METHOD;
                    else
                        return DPKG_METHOD;
                }
            }
            else
            {
                if (0 == strcmp(tmp_name, name))
                {
                    pclose(fp);
                    if (strstr(tmp_arch, "kare_"))
                        return KARE_METHOD;
                    if (strstr(name, "kwre"))
                        return KWRE_METHOD;
                    else if (strstr(name, "kmre"))
                        return KMRE_METHOD;
                    else
                        return DPKG_METHOD;
                }
            }
        }
        pclose(fp);
    }

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusError error;
        char *str = NULL;
        cJSON *root = NULL;
        InstallMethod result = NONE_METHOD;

        dbus_error_init(&error);

        if (!dbus_validate_utf8(name, &error))
        {
            goto out;
        }

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_connection_flush(connection);

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;
        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }

        // 修复：检查获取基本类型是否成功
        if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
        {
            klog_err("kdk : d-bus reply type mismatch !\n");
            goto out;
        }

        dbus_message_iter_get_basic(&args, &str);
        klog_info("get version Got string: %s\n", str ? str : "NULL");

        if (str && strnlen(str, STRNLEN_MAX) > 0)
        {
            root = cJSON_Parse(str);
            if (root == NULL) 
            {
                klog_err("Failed to parse JSON response\n");
                goto out;
            }

            cJSON *km_name = cJSON_GetObjectItem(root, "name");
            cJSON *km_version = cJSON_GetObjectItem(root, "version");
            cJSON *km_id = cJSON_GetObjectItem(root, "id");
            
            if (km_name && km_version && km_id &&
                km_name->valuestring && km_version->valuestring && km_id->valuestring)
            {
                klog_info("km_name->valuestring = %s\n", km_name->valuestring);
                klog_info("km_version->valuestring = %s\n", km_version->valuestring);
                klog_info("km_id->valuestring = %s\n", km_id->valuestring);

                if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                {
                    if (0 == strcmp(km_name->valuestring, name) && 0 == strcmp(km_version->valuestring, version))
                    {
                        result = KM_METHOD;
                        goto out;
                    }
                    else if(0 == strcmp(km_id->valuestring, name) && 0 == strcmp(km_version->valuestring, version))
                    {
                        result = KM_METHOD;
                        goto out;
                    }
                }
                else
                {
                    if (0 == strcmp(km_name->valuestring, name))
                    {
                        result = KM_METHOD;
                        goto out;
                    }
                    if (0 == strcmp(km_id->valuestring, name))
                    {
                        result = KM_METHOD;
                        goto out;
                    }
                }
            }
        }
out:
        if (root != NULL)
        {
            cJSON_Delete(root);
        }
        if (message != NULL)
        {
            dbus_message_unref(message);
        }
        if (reply != NULL)
        {
            dbus_message_unref(reply);
        }
        if (connection != NULL)
        {
            dbus_connection_unref(connection);
        }
        dbus_error_free(&error);
        
        if (result != NONE_METHOD)
        {
            return result;
        }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024] = {0};
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256] = {0}, tmp_version[64] = {0};
                // 修复：添加缓冲区大小限制
                int count = sscanf(line, "%255s %63s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                    
                if (version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                {
                    if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                    {
                        pclose(fp);
                        return KARE_METHOD;
                    }
                }
                else
                {
                    if (0 == strcmp(tmp_name, name))
                    {
                        pclose(fp);
                        return KARE_METHOD;
                    }
                }
            }
            pclose(fp);
        }
    }

    return NONE_METHOD;
}

void* libHandle = NULL;

static ns_mode_session_id_t kdk_ns_mode_enter()
{
    libHandle = dlopen(OSTREE_MODE_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        syslog(LOG_ERR, "load %s failed\n",OSTREE_MODE_LIB);
        return 0;
    }
    handle_ns_mode_enter enter_handle = (handle_ns_mode_enter)dlsym(libHandle, NS_MODE_ENTER_HANDLE);
    if(!enter_handle)
    {
        syslog(LOG_ERR, "undefined symbol:%s\n", NS_MODE_ENTER_HANDLE);
        return 0;
    }

    ns_mode_session_id_t session_id_t = enter_handle();
    
    if(session_id_t == 0)
    {
        syslog(LOG_ERR, "错误: ns_mode_enter() 失败, %s\n", strerror(errno));
        dlclose(libHandle);
        return 0;
    }
    syslog(LOG_INFO, "ns_mode_enter() 成功，session_id = %llu\n", session_id_t);

    return (int)session_id_t;
}

static int kdk_ns_mode_exit(ns_mode_session_id_t session_id)
{
    libHandle = dlopen(OSTREE_MODE_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        syslog(LOG_ERR, "load %s failed\n",OSTREE_MODE_LIB);
        return 0;
    }
    handle_ns_mode_exit exit_handle = (handle_ns_mode_exit)dlsym(libHandle, NS_MODE_EXIT_HANDLE);
    if(!exit_handle)
    {
        syslog(LOG_ERR, "undefined symbol:%s\n", NS_MODE_EXIT_HANDLE);
        return 0;
    }

    int ret = exit_handle(session_id);
    
    if(ret != 0)
    {
        syslog(LOG_ERR, "错误: ns_mode_exit() 失败, %s\n", strerror(errno));
        dlclose(libHandle);
        return 0;
    }
    dlclose(libHandle);
    syslog(LOG_INFO, "ns_mode_exit() 成功，资源已清理");
    return 1;
}

static kdk_boot_mode kdk_get_boot_mode()
{
    DBusConnection *conn;
    DBusError err;
    kdk_boot_mode mode = MODE_NONE;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        return mode;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    // 创建用户
    info_msg = dbus_message_new_method_call("com.kylin.MaintainMode",  // target for the method call
                                            "/com/kylin/MaintainMode/Object", // object to call on
                                            "com.kylin.MaintainMode.interface",  // interface to call on
                                            "GetBootMode");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return mode;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return mode;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return mode;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        return mode;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return mode;
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return mode;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

   if(strcmp(ret, "maintain") == 0)
   {
        mode = MODE_MAINTAIN;
   }
   else if(strcmp(ret, "normal") == 0)
   {
        mode = MODE_NORMAL;
   }

    return mode;
}

int check_kare_environment(const char *expected_value, char *output_value, size_t output_size) {
    FILE *fp = popen("kare -E", "r");
    if (!fp) return 0;
    
    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        strstripspace(line);
        if (strcmp(line, expected_value) == 0) {
            strncpy(output_value, expected_value, output_size - 1);
            output_value[output_size - 1] = '\0';
            found = 1;
            break;
        }
    }
    pclose(fp);
    return found;
}

static int extract_compat_file(const char *dir, const char *uri, const char *keyword, char *value) {
    char cmd[1024] = "\0";
    char line[1024] = "\0";
    char key[128] = "\0";
    char compat[4096] = "\0";
    snprintf(cmd, sizeof(cmd), "ar -x --output \'%s\' \'%s\' _compat", dir, uri);
    syslog(LOG_INFO, "[%s] ar cmd: %s", __func__, cmd);
    int ret = system(cmd);
    if(ret != 0)
        return 0;

    memset(cmd, 0, 1024);
    snprintf(cmd, sizeof(cmd), "%s/_compat", dir);
    
    FILE *fp = fopen(cmd, "r");
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
        memset(cmd, 0, 1024);
        snprintf(cmd, sizeof(cmd), "%s/_compat", dir);
        snprintf(remove_path, sizeof(remove_path), "rm \'%s\'", cmd);
        int remove = system(remove_path);
        // syslog(LOG_INFO, "value = %s\n", value);
    }
    return 1;
}

static bool kdk_is_panshi_feature()
{
    if (0 == access("/ostree/pkgs/system-ovl.conf", F_OK))
    {
        return true;
    }
    
    return false;
}

static pthread_t thread_install = 0;
static volatile int running_install = 0;
static InstallCallBack _callbackinstall = NULL;

static void *dbus_kaiming_online_install_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    char *info = NULL;

    int timeout_sec = 3600;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='progresschangesignal'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "progresschangesignal")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &info, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "info = %s\n", info);
                    cJSON *json = cJSON_Parse(info);
                    if (json == NULL) {
                        dbus_connection_close(connection);
                        dbus_connection_unref(connection);
                        pthread_exit(NULL);
                    }

                    cJSON *progress = cJSON_GetObjectItem(json, "progress");
                    _callbackinstall(progress->valueint, 1, "");
                    cJSON_Delete(json);
                    errorCode = -1;
                }
            }
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "kaiming online install info name : %s, operateType : %d, errorCode : %d, errinfo : %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0 || errorCode == 5)
                    {
                        _callbackinstall(100, 0, "安装成功，安装的环境为kaiming");
                        errorCode = -1;
                        mark = 1;
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming online install fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackinstall(0, -1, errinfo);
                        errorCode = -1;
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_install_online(const char *pkg_name, InstallCallBack call_back)
{
    int flag = 0;
    int mark = 0;
    bool ret = false;

    DBusConnection *conn;
    DBusError err;

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    running_install = 1;
    _callbackinstall = call_back;

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        syslog(LOG_INFO, "kaiming\n");

        if(strstr(pkg_name, "="))
        {
            char km_name[128] = "\0";
            char km_ver[128] = "\0";
            sscanf(pkg_name, "%[^=]= %s", km_name, km_ver);
            syslog(LOG_DEBUG, "package_name = %s, version = %s", km_name, km_ver);
            ret = kdk_package_is_kaiming_online_installable(km_name, km_ver);
        }
        else
        {
            syslog(LOG_DEBUG, "package_name = %s", pkg_name);
            ret = kdk_package_is_kaiming_online_installable(pkg_name, NULL);
        }
        
        if(!ret)
        {
            return 0;
        }

        kdk_lock_status status = check_lock_status(KDK_KAIMING_INSTALL_LOCK);

        switch (status) {
            case UNLOCKED:
                syslog(LOG_INFO, "[%s], %s File is not locked", __func__, KDK_KAIMING_INSTALL_LOCK);
                break;
            case LOCKED:
                syslog(LOG_INFO, "[%s], %s File is locked", __func__, KDK_KAIMING_INSTALL_LOCK);
                _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
                return 0;
            case ERROR:
                syslog(LOG_INFO, "[%s], %s Failed to check lock status", __func__, KDK_KAIMING_INSTALL_LOCK);
                _callbackinstall(0, -1, "开明锁状态检查出错");
                return 0;
        }

        pthread_create(&thread_install, NULL, dbus_kaiming_online_install_listener_thread, NULL);

        char *branch = "";
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            goto err_out;
        }        

        info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "install");           // method name

        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if(strstr(pkg_name, "="))
        {
            char package_name[128] = "\0";
            char version[128] = "\0";
            char *name = (char *)malloc(sizeof(char) * 512);
            sscanf(pkg_name, "%[^=]= %s", package_name, version);
            snprintf(name, sizeof(char) * 512, "%s/%s", package_name, version);
            syslog(LOG_DEBUG, "package_name = %s, version = %s, name = %s", package_name, version, name);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                free(name);
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }
            free(name);
        }
        else
        {
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, DBUS_TIMEOUT_INFINITE))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);

        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            char errinfo[256] = "\0";
            snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kaiming (%s)", dbus_message_get_error_name(replyMsg));
            _callbackinstall(0, -1, errinfo);
            goto err_out;
        }

        DBusMessageIter args;
        bool *res = false;
                
        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &res);
        }

        dbus_message_unref(info_msg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_message_unref(replyMsg);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);
    }
    else
    {
        return mark;
    }

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
        
   return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_debian_online_install_listener_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    char line[1024] = "\0";
    int success = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int mark = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    int extra_line = 0;
    size_t len = 0;
    int ret = -1;
    if(running_install)
    {
        if (strstr(args->param_name, "kare-v10sp1-base"))
        {
            // kare基础环境包安装时间较长，apt输出无明确阶段关键词
            // 使用poll非阻塞方式：每10秒无新输出则进度加1，最大到95%
            int fd = fileno(args->param_fp);
            while (1)
            {
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLIN;

                int poll_ret = poll(&pfd, 1, 10000);  // 10秒超时
                if (poll_ret > 0 && (pfd.revents & POLLIN))
                {
                    // 有数据可读
                    if (!fgets(line, sizeof(line), args->param_fp))
                        break;  // EOF 或错误
                    syslog(LOG_INFO, "debain online install info : %s\n", line);

                    if (strstr(line, "下列软件包有未满足的依赖关系") || strstr(line, "E: 无法定位软件包") || strstr(line, "Failed to enable unit") || strstr(line, "dpkg: 处理归档"))
                    {
                        capture = 1;
                    }

                    if (capture)
                    {
                        len = strnlen(line, STRNLEN_MAX);
                        if (total_len + len < sizeof(output) - 1)
                        {
                            strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                            total_len += len;
                        }
                    }

                    if (strstr(line, "错误"))
                    {
                        flag = 1;
                    }
                    if (strstr(line, "E: 无法定位软件包"))
                    {
                        flag = 1;
                    }
                    if (strstr(line, "正在处理用于 man-db") || strstr(line, "正在处理用于 libc-bin"))
                    {
                        success = 1;
                    }
                }
                else if (poll_ret == 0)
                {
                    // 10秒超时，无新输出，进度加1，超过99则一直输出99
                    if (progress < 99)
                    {
                        progress += 1;
                    }
                    syslog(LOG_INFO, "kare base env progress: %d\n", progress);
                    _callbackinstall(progress > 99 ? 99 : progress, 2, "kare base env installing...");
                }
                else
                {
                    break;  // poll 出错
                }
            }
            pclose(args->param_fp);
            _callbackinstall(100, 2, "构建成功");
            return (void *)(intptr_t)2;

        }
        else
        {
            while (fgets(line, sizeof(line), args->param_fp))
            {
                syslog(LOG_INFO, "debain online install info : %s\n", line);

                    if (strstr(line, "获取:") && !stage_flags[0]) {
                        progress += 10;  // 下载阶段占 10%
                        stage_flags[0] = 1;
                        _callbackinstall(progress, 1, line);
                    } else if (strstr(line, "准备解压") && !stage_flags[1]) {
                        progress += 20;  // 准备解压占 20%
                        stage_flags[1] = 1;
                        _callbackinstall(progress, 1, line);
                    } else if (strstr(line, "正在解压") && !stage_flags[2]) {
                        progress += 30;  // 解压阶段占 30%
                        stage_flags[2] = 1;
                        _callbackinstall(progress, 1, line);
                    } else if (strstr(line, "正在设置") && !stage_flags[3]) {
                        progress += 30;  // 设置阶段占 30%
                        stage_flags[3] = 1;
                        _callbackinstall(progress, 1, line);
                    } else if (strstr(line, "正在处理用于") && !stage_flags[4]) {
                        progress += 5;  // 触发器处理占 10%
                        stage_flags[4] = 1;
                        _callbackinstall(progress, 1, line);
                    }
                
                
                if (strstr(line, "下列软件包有未满足的依赖关系") || strstr(line, "E: 无法定位软件包") || strstr(line, "Failed to enable unit") || strstr(line, "dpkg: 处理归档"))
                {
                    capture = 1;  // 开始捕获
                }

                if (capture) 
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }

                if (strstr(line, "有未能满足的依赖关系")) 
                {
                    extra_line = 1;  // 让下一行也加入输出
                }
                else if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错"))
                {
                    if(strnlen(output, STRNLEN_MAX) == 0)
                    {
                        len = strnlen(line, STRNLEN_MAX);
                        if (total_len + len < sizeof(output) - 1) 
                        {
                            strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                            total_len += len;
                        } else {
                            break;  // 防止溢出
                        }
                    }
                    capture = 1;
                    extra_line = 1;  // 让下一行也加入输出
                }
                else if (extra_line) 
                {
                    capture = 0;
                    extra_line = 0;  // 只多输出一行
                }

                if(strstr(line, "错误"))
                {
                    flag = 1;
                }
                if(strstr(line, "E: 无法定位软件包"))
                {
                    flag = 1;
                }
                if(strstr(line, "正在处理用于 man-db") || strstr(line, "正在处理用于 libc-bin"))
                {
                    success = 1;
                }
            }
        }
        pclose(args->param_fp);

        if(strstr(args->param_name, "="))
        {
            char package_name[128] = "\0";
            char version[128] = "\0";
            sscanf(args->param_name, "%[^=]= %s", package_name, version);
            ret =  kdk_package_is_installed(package_name, version);
            syslog(LOG_INFO, "%s version: %s package is installed : %d\n", package_name, version, ret);
        }
        else
        {
            ret =  kdk_package_is_installed(args->param_name, NULL);
            syslog(LOG_INFO, "%s package is installed : %d\n", args->param_name, ret);
        }

        if(ret == 0 && flag != 1)
        {
            mark = 1;
            syslog(LOG_INFO, "debian online success");
        }
        else
        {
            _callbackinstall(0, -1, output);
            syslog(LOG_INFO, "debian online fail");
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_debian_install_online(char *uri, const char *pkg_name, InstallCallBack call_back)
{
    int mark = 0;
    
    syslog(LOG_INFO, " %s, pkg_name = %s\n", __func__, pkg_name);
    _callbackinstall = call_back;
    bool check = true;

    if (kdk_get_boot_mode() == MODE_NORMAL) {
        check = kdk_get_pkg_sign_check();
        if (check)
        {
            syslog(LOG_INFO, "Debian package signature verification is enabled.\n");
            int start = kdk_deb_verify_start();
            setup_signal_handlers();
            g_verify_end_count = 0;
        }

        kdk_ns_mode_session_id = kdk_ns_mode_enter();
        if (kdk_ns_mode_session_id != 0)
        {
            kdk_ns_mode_flag_enter = 1;
        }
        else
        {
            _callbackinstall(0, -1, "ns_mode_enter() 失败");
            if (check)
            {
                kdk_deb_verify_end();
                g_verify_end_count = 1;
            }
            return 0;
        }
    }

    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        syslog(LOG_INFO, "Failed to create pipe");
        goto err_out;
    }
    
    pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        syslog(LOG_INFO, "Failed to fork process");
        goto err_out;
    }

    if (pid == 0) { // 子进程
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO); // 将 stderr 也重定向到 stdout
        close(pipe_fd[1]);

        char *args[] = {
            "apt.orig",
            "install",
            "--allow-downgrades",
            "-y",
            "-o",
            "Dpkg::Options::=--force-confnew",
            uri ? (char *)uri : (char *)pkg_name,
            NULL
        };

        execvp("apt.orig", args);
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            syslog(LOG_INFO, "Failed to open pipe stream");
            goto err_out;
        }

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        if (!args) {
            fclose(fp);
            waitpid(pid, NULL, 0);
            syslog(LOG_INFO, "Memory allocation failed");
            goto err_out;
        }
        
        // 安全的字符串复制
        strncpy(args->param_name, pkg_name, sizeof(args->param_name) - 1);
        args->param_name[sizeof(args->param_name) - 1] = '\0';
        args->param_fp = fp;

        running_install = 1;
        if (pthread_create(&thread_install, NULL, dbus_debian_online_install_listener_thread, (void *)args) != 0) {
            fclose(fp);
            free(args);
            waitpid(pid, NULL, 0);
            syslog(LOG_INFO, "Failed to create thread");
            goto err_out;
        }
        
        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if (thread_result) 
        {
            mark = (int)(intptr_t)thread_result;
        }
        free(args);

        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        syslog(LOG_INFO, "debian online mark %d", mark);
        if(kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
    }

    bool result = kdk_create_desktop_link(pkg_name);
    if(!result)
    {
        syslog(LOG_INFO, "Failed to create symlink");
    }

    if (kdk_get_boot_mode() == MODE_NORMAL) 
    {
        if (check)
        {
            kdk_deb_verify_end();
            g_verify_end_count = 1;
        }
    }
    
    if(mark && privilege_tag)
    {
        kdk_set_privileged_tag_packag(pkg_name);
    }

    if(mark == 1)
        _callbackinstall(100, 0, "安装成功，安装的环境为核心系统");

    return mark;
    
err_out:
    if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
    {
        kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
        kdk_ns_mode_flag_enter = 0;
    }
    if (kdk_get_boot_mode() == MODE_NORMAL) {
        if (check)
        {
            kdk_deb_verify_end();
            g_verify_end_count = 1;
        }
    }
    return 0;
}

static void *dbus_kare_build_base_env_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int progress = 0;
    int status = -1;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='buildStatusChanged'",
                    &error);

    while (true)
    {
        if(running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kare", "buildStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &progress, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, progress = %d, status = %d, errinfo = %s\n", name, progress, status, errinfo);

                    if(status == -1 || status == -2 || status == -3 || status == -4)
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                    else
                    {
                        _callbackinstall(progress, 2, errinfo);
                        if(progress == 100 && status == 0)
                        {
                            _callbackinstall(progress, 2, "构建成功");
                            mark = 1;
                            break;
                        }
                        status = -5;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else{
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kare_build_base_env(char *env, InstallCallBack call_back)
{
    char *architecture = NULL;
    char pkg_name[128] = {0};
    bool is_v10sp1_inst = false;
    int mark = 0;
    DBusConnection *conn;
    DBusError err;

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    if(strcmp(env, "v10sp1") == 0)
    {
    
#ifdef __linux__
        FILE *fp = fopen("/proc/osinfo", "r");
        if (!fp)
            return NULL;

        architecture = get_val_from_file(fp, "Architecture");
        if (!architecture)
        {
            fclose(fp);
            return NULL;
        }
        strstrip(architecture, '\n');
        strstrip(architecture, '\t');

        fclose(fp);
#endif
        if (strstr(architecture, "x86_64"))
        {
            strncpy(pkg_name, "kare-v10sp1-base-x86-64", sizeof(pkg_name) - 1);
        }
        else if (strstr(architecture, "aarch64"))
        {
            strncpy(pkg_name, "kare-v10sp1-base-aarch64", sizeof(pkg_name) - 1);
        }
        else if (strstr(architecture, "loongarch64"))
        {
            strncpy(pkg_name, "kare-v10sp1-base-loongarch64", sizeof(pkg_name) - 1);
        }
        else if (strstr(architecture, "sw"))
        {
            strncpy(pkg_name, "kare-v10sp1-base-sw-64", sizeof(pkg_name) - 1);
        }

        if (strlen(pkg_name) != 0)
        {
            bool ret = kdk_package_is_deb_online_installable(pkg_name, NULL);
            if (!ret)
            {
                syslog(LOG_ERR, "kare offline install fail, pkg_name : %s\n", pkg_name);
            }
            else
            {
                is_v10sp1_inst = true;

                kdk_debian_install_online(NULL, pkg_name, call_back);
                mark = 1;
            }
        }
    }

    if(!is_v10sp1_inst)
    {
    
        syslog(LOG_INFO, "build base env\n");
        
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (NULL == conn)
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            goto err_out;
        }

        if (dbus_error_is_set(&err))
        {
            // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        //创建用户
        info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                "/org/kylin/kare", // object to call on
                                                "org.kylin.kare",  // interface to call on
                                                "buildBaseEnv");               // method name
        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            goto err_out;
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);
        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            goto err_out;
        }

        DBusMessageIter args;
        bool *ret = false;

        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &ret);
        }

        dbus_message_unref(info_msg);
        dbus_message_unref(replyMsg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);

        if(ret)
        {
            mark = 1;
        }
        

        if(mark)
        {
            mark = 0;
            running_install = 1;
            _callbackinstall = call_back;
            pthread_create(&thread_install, NULL, dbus_kare_build_base_env_listener_thread, NULL);

            void *thread_result;
            pthread_join(thread_install, &thread_result);

            if(thread_result)
            {
                mark = (int)(intptr_t)thread_result;
            }
        }
    }
    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_kare_install_listener_thread(void *arg)
{
    char *env = (char *)arg;
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int progress = 0;
    int status = -5;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;
    char *info = NULL;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='installStatusChanged'",
                    &error);
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kare", "installStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &progress, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, progress = %d, status = %d, errinfo = %s\n", name, progress, status, errinfo);

                    info = getenv("PACKAGE_KARE_INFO");
                    if(info)
                    {
                        if(strcmp(info, "1") == 0)
                        {
                            printf("name = %s, progress = %d, status = %d, errinfo = %s\n", name, progress, status, errinfo);
                        }
                    }

                    if(status == 1)
                    {
                        syslog(LOG_ERR, "kare offline installing , status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                    }
                    else if(status == 0)
                    {
                        char loginfo[512] = "\0";
                        snprintf(loginfo, sizeof(loginfo), "安装成功，安装的环境为kare %s", env);
                        _callbackinstall(progress, status, loginfo);
                        if(progress == 100)
                        {
                            mark = 1;
                            break;
                        }
                    }
                    else
                    {
                        syslog(LOG_ERR, "kare offline install fail, status : %d, errinfo : %s\n", status, errinfo);
                        _callbackinstall(progress, status, errinfo);
                        status = -5;
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else{
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    syslog(LOG_INFO, "%s ret mark = %d\n", __func__, mark);
    return (void *)(intptr_t)mark;
}

static char *kdk_system_get_eUser()
{
    char *e_user = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        fprintf(stderr, "DBus connection error: %s\n", error.message);
        goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    int element_count = dbus_message_iter_get_element_count(&iter);
    char **users = calloc(element_count + 1, sizeof(char *));
    if (NULL == users)
        goto out;

    dbus_message_iter_recurse(&iter, &iter);

    // 解析返回值
    // 循环中过程中调用新的dbus接口会导致迭代器iter被清空，但是没细查原因
    // 先用一个字符串数组存储所有用户的object path
    for (int i = 0; i < element_count; i++)
    {
        char *path = NULL;

        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&iter, &struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &path);

        users[i] = strdup(path);

        dbus_message_iter_next(&iter);
    }

    for (int i = 0; i < element_count; i++)
    {
        char *interface = "org.freedesktop.login1.User";
        char *state = "State";
        char *property = "Name";

        message = dbus_message_new_method_call("org.freedesktop.login1",
                                               users[i],
                                               "org.freedesktop.DBus.Properties",
                                               "Get");
        if (message == NULL)
        {
            fprintf(stderr, "DBus message allocation failed\n");
            goto out;
        }

        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            fprintf(stderr, "DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_message_unref(message);
        message = NULL;

        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
        {
            fprintf(stderr, "DBus reply parsing failed\n");
            goto out;
        }

        dbus_message_unref(reply);
        reply = NULL;

        char *active = NULL;
        dbus_message_iter_recurse(&iter, &iter);
        dbus_message_iter_get_basic(&iter, &active);

        if (0 == strcmp(active, "active"))
        {
            message = dbus_message_new_method_call("org.freedesktop.login1",
                                                   users[i],
                                                   "org.freedesktop.DBus.Properties",
                                                   "Get");
            if (message == NULL)
            {
                fprintf(stderr, "DBus message allocation failed\n");
                goto out;
            }

            dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
            if (reply == NULL)
            {
                fprintf(stderr, "DBus reply error: %s\n", error.message);
                goto out;
            }

            dbus_message_unref(message);
            message = NULL;

            if (!dbus_message_iter_init(reply, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
            {
                fprintf(stderr, "DBus reply parsing failed\n");
                goto out;
            }

            dbus_message_unref(reply);
            reply = NULL;

            dbus_message_iter_recurse(&iter, &iter);
            dbus_message_iter_get_basic(&iter, &e_user);

            break;
        }
    }

    if (e_user)
        e_user = strdup(e_user);
out:
    if (users)
        g_strfreev(users);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);
    if (connection)
        dbus_connection_unref(connection);
    return e_user;
}

static char* kdk_get_home_dir()
{
    char *username = kdk_system_get_eUser();
    if(!username)
    {
        return NULL;
    }

    char *user = NULL;
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
        free(username);
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            "/org/freedesktop/Accounts",     // object to call on
                                            "org.freedesktop.Accounts", // interface to call on
                                            "FindUserByName");           // method name
    if (!info_msg)
    { // -1 is default timeout
        free(username);
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &username, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        free(username);
        goto err_out;
    }
    free(username);

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);

    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return NULL;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &user);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    DBusMessage *msg = NULL;
    DBusPendingCall *sendPending = NULL;
    DBusMessage *replyLangMsg = NULL;

    //创建用户
    msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            user,     // object to call on
                                            "org.freedesktop.DBus.Properties", // interface to call on
                                            "Get");           // method name
    if (!msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }
    char *interface = "org.freedesktop.Accounts.User";
    char *property = "HomeDirectory";

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
    char *dir = NULL;

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
    dbus_message_iter_get_basic(&iter, &dir);
    dbus_connection_unref(conn);
    return dir;

err_out:
    return NULL;
}

// 从deb行中提取完整的HTTP地址
static char* extract_http_url(const char* line) {
    // 跳过 "deb " 前缀
    const char* url_start = line + 4;
    
    // 跳过开头的空白字符
    while (*url_start && isspace(*url_start)) {
        url_start++;
    }
    
    if (*url_start == '\0') {
        return NULL;
    }
    
    syslog(LOG_INFO, "Original URL Section: %s\n", url_start);
    
    // 检查是否是HTTP或HTTPS协议
    if (strncmp(url_start, "http://", 7) != 0 && 
        strncmp(url_start, "https://", 8) != 0) {
        syslog(LOG_ERR, "Not HTTP/HTTPS protocol \n");
        return NULL;
    }
    
    // 找到URL的结束位置（第一个空格或行尾）
    const char* url_end = url_start;
    while (*url_end && !isspace(*url_end)) {
        url_end++;
    }
    
    // 计算URL长度
    size_t url_len = url_end - url_start;
    if (url_len == 0) {
        return NULL;
    }
    
    // 分配内存并复制URL
    char* http_url = (char*)malloc(url_len + 1);
    if (!http_url) {
        syslog(LOG_ERR, "Memory allocation failed");
        return NULL;
    }
    
    strncpy(http_url, url_start, url_len);
    http_url[url_len] = '\0';
    
    syslog(LOG_INFO, "Extracted URL : %s\n", http_url);
    
    return http_url;
}

// 获取第一个未注释源的完整HTTP地址
static char* get_first_active_repo_url(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        syslog(LOG_ERR, "Cannot open file");
        return NULL;
    }
    
    char line[1024];
    char* http_url = NULL;
    int line_number = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        // 去除行尾的换行符
        strstripspace(line);
        
        // 跳过空行
        if (strnlen(line, STRNLEN_MAX) == 0) {
            continue;
        }
        
        // 查找第一个非空白字符
        const char* first_char = line;
        while (*first_char && isspace(*first_char)) {
            first_char++;
        }
        
        // 检查是否为注释行
        if (*first_char == '#') {
            continue;
        }
        
        if (*first_char == '\0') {
            continue;
        }
        
        // 检查是否是 deb 开头的行
        if (strncmp(first_char, "deb ", 4) == 0) {
            // 提取HTTP地址
            http_url = extract_http_url(first_char);
            if (http_url) {
                syslog(LOG_INFO, "Successfully extracted HTTP address\n");
                break;
            } else {
                syslog(LOG_INFO, "Cannot extract HTTP address\n");
            }
        } else {
            syslog(LOG_INFO, "Not a DEB source, skipping");
        }
    }
    
    fclose(file);
    
    return http_url;
}

static int kdk_kare_install_online(const char *deb_path, char* dir, char* rt_version, InstallCallBack call_back)
{
    syslog(LOG_DEBUG, "kare online");
    char line[1024] = "\0";
    FILE *fp = NULL;
    
    char pkg_name_str[256] = "\0";
    char fileNamePath[512] = "\0";
    
    int mark = 0;
    int flag = 0;
    char key[128] = "\0";
    char value[128] = "\0";
    char *runtime = NULL;
    char typetag[128] = "\0";
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    _callbackinstall = call_back;
    if(rt_version)
        strcpy(value, rt_version);

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "[%s] kare\n", __func__);
        if (!extract_compat_file(dir, deb_path, "sub_system", typetag))
        {
            return 0;
        }

        if(value[0] != '\0')
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
                    
                    if (flag != 1 && order == p->order)
                    {
                        if (kdk_kare_build_base_env(p->value, call_back) == 1)
                        {
                            strncpy(value, p->value, sizeof(value) - 1);
                            flag = 1;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            CheckResult version = _kdk_dmvc_check_os_version(deb_path);

            if(version == CHECK_RESULT_V10SP1)
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

                if(runtime)
                {
                    if (check_kare_environment(runtime, value, sizeof(value)))
                    {
                        syslog(LOG_INFO, "runtime = %s, value = %s\n", runtime, value);
                        flag = 1;
                    }
                    else
                    {
                        int base = kdk_kare_build_base_env(runtime, call_back);
                        if(base == 1)
                        {
                            strncpy(value, runtime, sizeof(value) - 1);
                            flag = 1;
                        }
                    }
                    free(runtime);
                }
            }
        }

        syslog(LOG_INFO, "flag = %d, value = %s, typetag = %s\n", flag, value, typetag);
        
        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            if(typetag[0] != '\0')
            {
                //创建用户
                info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                        "/org/kylin/kare", // object to call on
                                                        "org.kylin.kare",  // interface to call on
                                                        "install_with_subsystem");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    goto err_out;
                }

                char *env = strdup(value);
                char *subsystem = strdup(typetag);
                char *uri = strdup(deb_path);
    
                syslog(LOG_INFO, "kdk : d-bus env : %s, subsystem : %s\n", env, subsystem);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &subsystem, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(env);
                    free(subsystem);
                    free(uri);
                    goto err_out;
                }
                free(env);
                free(subsystem);
                free(uri);
            }
            else
            {
                //创建用户
                info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                        "/org/kylin/kare", // object to call on
                                                        "org.kylin.kare",  // interface to call on
                                                        "install");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    goto err_out;
                }

                char *env = strdup(value);
                char *uri = strdup(deb_path);
                syslog(LOG_INFO, "kdk : d-bus env : %s\n", env);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(env);
                    free(uri);
                    goto err_out;
                }
                free(env);
                free(uri);
            }

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kare (%s)", dbus_message_get_error_name(replyMsg));
                _callbackinstall(0, -1, errinfo);
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);
            
            if(ret)
            {
                mark = 1;
            }
            else
            {
                if(access(KDK_KARE_INSTALL_LOCK, F_OK) == 0)
                {
                    _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
                }
            }
        }
    }

    if(mark)
    {
        mark = 0;
        running_install = 1;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, value);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }
    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);

    return 0;
}

static void *dbus_kaiming_offline_install_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    char *info = NULL;

    int timeout_sec = 1800;
    int poll_interval = 1;
    int elapsed_time = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='progresschangesignal'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_install)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "progresschangesignal")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &info, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "info = %s\n", info);
                    cJSON *json = cJSON_Parse(info);
                    if (json == NULL) {
                        dbus_connection_close(connection);
                        dbus_connection_unref(connection);
                        pthread_exit(NULL);
                    }

                    cJSON *progress = cJSON_GetObjectItem(json, "progress");
                    if (progress != NULL) {
                        _callbackinstall(progress->valueint, 1, "");
                        errorCode = -1;
                    }
                    cJSON_Delete(json);
                }
            }
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "kaiming online install info name : %s, operateType : %d, errorCode : %d, errinfo : %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0)
                    {
                        _callbackinstall(100, 0, "安装成功，安装的环境为kaiming");
                        errorCode = -1;
                        mark = 1;
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming offline install fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackinstall(0, -1, errinfo);
                        errorCode = -1;
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_install_offline(const char *uri, InstallCallBack call_back)
{
    int flag = 0;
    int mark = 0;

    running_install = 1;
    _callbackinstall = call_back;

    kdk_lock_status status = check_lock_status(KDK_KAIMING_INSTALL_LOCK);

    switch (status) {
        case UNLOCKED:
            syslog(LOG_INFO, "[%s], %s File is not locked", __func__, KDK_KAIMING_INSTALL_LOCK);
            break;
        case LOCKED:
            syslog(LOG_INFO, "[%s], %s File is locked", __func__, KDK_KAIMING_INSTALL_LOCK);
            _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
            return 0;
        case ERROR:
            syslog(LOG_INFO, "[%s], %s Failed to check lock status", __func__, KDK_KAIMING_INSTALL_LOCK);
            _callbackinstall(0, -1, "开明锁状态检查出错");
            return 0;
    }

    pthread_create(&thread_install, NULL, dbus_kaiming_offline_install_listener_thread, NULL);

    DBusConnection *conn;
    DBusError err;

    DBusMessage *info_msg = NULL;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessage *replyMsgBranch = NULL;

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        syslog(LOG_ERR, "kaiming\n");
        
        char *branch = "";
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            goto err_out;
        }        

        info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "installOfflinePackage");           // method name

        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            goto err_out;
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, DBUS_TIMEOUT_INFINITE))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);

        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            char errinfo[256] = "\0";
            snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kaiming (%s)", dbus_message_get_error_name(replyMsg));
            _callbackinstall(0, -1, errinfo);
            goto err_out;
        }

        DBusMessageIter args;
        bool *res = false;
                
        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &res);
        }

        dbus_message_unref(info_msg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_message_unref(replyMsg);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);
    }

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }
    
    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_debian_offline_install_listener_thread(void *arg)
{
    FILE *fp = (FILE *)arg;
    char line[1024] = "\0";
    int count = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int cover = 0;
    int mark = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    char errinfo[8192] = "\0";
    int extra_line = 0;
    char *info = NULL;
    if(running_install)
    {
        while (fgets(line, sizeof(line), fp))
        {
            info = getenv("PACKAGE_KARE_INFO");
            if(info)
            {
                if(strcmp(info, "1") == 0)
                {
                    printf("%s", line);
                }
            }
            else
            {
                syslog(LOG_INFO, "debian install offline info : %s\n", line);

                if (strstr(line, "准备解压") && !stage_flags[1]) {
                    progress += 20;  // 准备解压占 20%
                    stage_flags[1] = 1;
                    _callbackinstall(progress, 1, line);
                } else if (strstr(line, "正在解压") && !stage_flags[2]) {
                    progress += 30;  // 解压阶段占 30%
                    stage_flags[2] = 1;
                    _callbackinstall(progress, 1, line);
                } else if (strstr(line, "正在设置") && !stage_flags[3]) {
                    progress += 30;  // 设置阶段占 30%
                    stage_flags[3] = 1;
                    _callbackinstall(progress, 1, line);
                }

                if (strstr(line, "dpkg: 依赖关系问题使得") || strstr(line, "dpkg: 处理归档") || strstr(line, "dpkg: 错误: 另外一个进程已经为 dpkg frontend lock 加锁"))
                {
                    flag = 1;
                    capture = 1;  // 开始捕获
                }

                if (capture) 
                {
                    size_t len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }

                if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错")) 
                {
                    flag = 1;
                    extra_line = 1;  // 让下一行也加入输出
                }
                else if (strstr(line, "dpkg: 处理归档") && strstr(line, "时出错"))
                {
                    flag = 1;
                    extra_line = 1;  // 处理 "dpkg: 处理归档 ..." 也要多输出一行
                } 
                else if (extra_line) 
                {
                    capture = 0;
                    extra_line = 0;  // 只多输出一行
                }

                if(strstr(line, "在处理时有错误发生"))
                {
                    flag = 1;
                    syslog(LOG_ERR, "debian offline install fail\n");
                    break;
                }
            }

            size_t len = strnlen(line, STRNLEN_MAX);
            if (total_len + len < sizeof(errinfo) - 1) 
            {
                strncat(errinfo, line, sizeof(errinfo) - strnlen(errinfo, STRNLEN_MAX));
                total_len += len;
            } else {
                break;  // 防止溢出
            }
            count++;
        }
        pclose(fp);

        if((!flag) && (count > 3))
        {
            mark = 1;
        }
        else
        {
            if(count <= 3)
            {
                _callbackinstall(0, -1, errinfo);
            }
            else
            {
                _callbackinstall(0, -1, output);
            }
        }

    }

    return (void *)(intptr_t)mark;
}

static int kdk_debian_install_offline(const char *uri, InstallCallBack call_back)
{
    syslog(LOG_INFO, "%s debian\n", __func__);
    _callbackinstall = call_back;
    bool check = true;

    if (kdk_get_boot_mode() == MODE_NORMAL)
    {
        check = kdk_get_pkg_sign_check();
        if (check)
        {
            syslog(LOG_INFO, "Debian package signature verification is enabled.\n");
            int start = kdk_deb_verify_start();
            setup_signal_handlers();
            g_verify_end_count = 0;
        }

        kdk_ns_mode_session_id = kdk_ns_mode_enter();
        if (kdk_ns_mode_session_id != 0)
        {
            kdk_ns_mode_flag_enter = 1;
        }
        else
        {
            _callbackinstall(0, -1, "ns_mode_enter() 失败");
            if (check)
            {
                kdk_deb_verify_end();
                g_verify_end_count = 1;
            }
            return 0;
        }
    }

    FILE *fp = NULL;
    char cmd[102400] = "\0";
    int mark = 0;
    char line[10240] = "\0";

    strncpy(cmd, "apt install -y -o Dpkg::Options::=\"--force-confnew\" -f 2>&1", sizeof(cmd));
    fp = popen(cmd, "r");
    if (fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            syslog(LOG_INFO, "%s line = %s\n", __func__, line);
        }

        pclose(fp);
    }

    int ret = 0;

    const char *target_dir = "/tmp/kysdk-depends-deb-install-path";
    if (access(target_dir, F_OK) != 0)
    {
        // 创建目录，包括父目录
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", target_dir);
        ret = system(mkdir_cmd);
        if (ret != 0)
        {
            syslog(LOG_ERR, "Failed to create directory: %s\n", target_dir);
            goto err_out;
        }
    }

    char name[512] = "\0";

    char *file_name = kdk_get_package_name(uri);
    if(!file_name)
    {
        goto err_out;
    }

    strcpy(name, file_name);
    free(file_name);    

    int flag = 0;    // 依赖下载是否失败，flag为1下载失败
    int capture = 0; // 是否开始捕获内容
    size_t total_len = 0;
    char output[102400] = "\0";
    char pkgs[MAX_PKGS][MAX_NAME_LEN];
    int count = 0;
    snprintf(cmd, sizeof(cmd), "apt-get.orig install -s \'%s\' 2>&1", uri);
    syslog(LOG_INFO, "[%s], download depens package cmd: %s\n", __func__, cmd);
    fp = popen(cmd, "r");
    if (fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            syslog(LOG_INFO, "%s download depends info: %s\n", __func__, line);
            if (strncmp(line, "下列软件包有未满足的依赖关系", 42) == 0)
            {
                flag = 1;
                capture = 1; // 开始捕获
            }

            if (strncmp(line, "Inst ", 5) == 0)
            {
                char pkg[MAX_NAME_LEN] = {0};

                // 提取包名（Inst 后第一个字段）
                if (sscanf(line, "Inst %1024s", pkg) == 1)
                {
                    // 去重（可选）
                    int exists = 0;
                    for (int i = 0; i < count; i++)
                    {
                        if (strcmp(pkgs[i], pkg) == 0)
                        {
                            exists = 1;
                            break;
                        }
                    }

                    if (strcmp(pkg, name) != 0 && count < MAX_PKGS -1)
                    {
                        strncpy(pkgs[count], pkg, MAX_NAME_LEN - 1);
                        count++;
                    }
                }
            }

            if (capture)
            {
                size_t len = strnlen(line, STRNLEN_MAX);
                if (total_len + len < sizeof(output) - 1)
                {
                    strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                    total_len += len;
                }
                else
                {
                    break; // 防止溢出
                }
            }
        }

        pclose(fp);
    }

    if (flag)
    {
        syslog(LOG_INFO, "%s download depends failed\n", __func__);
        _callbackinstall(0, -1, output);
        goto err_out;
    }

    if (count != 0)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "apt-get.orig install -y --download-only -o Dir::Cache::archives=\'%s\'", target_dir);

        for (int i = 0; i < count; i++)
        {
            strcat(cmd, " ");
            strcat(cmd, pkgs[i]);
        }

        syslog(LOG_INFO, "[%s], download depens package cmd: %s\n", __func__, cmd);

        fp = popen(cmd, "r");
        if (fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                syslog(LOG_INFO, "%s download depends info: %s\n", __func__, line);
            }

            pclose(fp);
        }

        snprintf(cmd, sizeof(cmd), "apt.orig install -y -o Dpkg::Options::=\"--force-confnew\" %s/*.deb 2>&1", target_dir);
        fp = popen(cmd, "r");
        if (fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                syslog(LOG_INFO, "%s install depends info: %s\n", __func__, line);
            }

            pclose(fp);
        }
    }

    snprintf(cmd, sizeof(cmd), "rm -r %s/* > /dev/null 2>&1", target_dir);
    ret = system(cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "dpkg.orig -i \'%s\' 2>&1", uri);

    fp = popen(cmd, "r");
    if (!fp)
    {
        goto err_out;
    }

    running_install = 1;
    pthread_create(&thread_install, NULL, dbus_debian_offline_install_listener_thread, (void *)fp);

    void *thread_result;
    pthread_join(thread_install, &thread_result);

    if (thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }

    if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
    {
        kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
        kdk_ns_mode_flag_enter = 0;
    }

    bool result = kdk_create_desktop_link(uri);
    if (!result)
    {
        syslog(LOG_INFO, "Failed to create symlink");
    }

    if (kdk_get_boot_mode() == MODE_NORMAL) 
    {
        if (check)
        {
            kdk_deb_verify_end();
            g_verify_end_count = 1;
        }
    }

    if (mark && privilege_tag)
    {
        kdk_set_privileged_tag_packag(name);
    }

    if (mark)
        _callbackinstall(100, 0, "安装成功，安装的环境为核心系统");

    return mark;

err_out:
    if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
    {
        kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
        kdk_ns_mode_flag_enter = 0;
    }
    if (kdk_get_boot_mode() == MODE_NORMAL) {
        if (check)
        {
            kdk_deb_verify_end();
            g_verify_end_count = 1;
        }
    }
    return 0;
}

static int kdk_kare_install_offline(const char *uri, char* dir, char* rt_version, InstallCallBack call_back)
{
    if(!uri)
    {
        syslog(LOG_ERR, "%s: NULL uri parameter\n", __func__);
        return 0;
    }

    _callbackinstall = call_back;
    int mark = 0;
    int flag = 0;
    char cmd[1024]= "\0";

    char line[1024] = "\0";
    char key[128] = "\0";
    char value[128] = "\0";
    char *runtime = NULL;
    char typetag[128] = "\0";
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    if(rt_version)
        strcpy(value, rt_version);

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "[%s] kare\n", __func__);

        if (!extract_compat_file(dir, uri, "sub_system", typetag))
        {
            return 0;
        } 

        if(value[0] != '\0')
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
                    
                    if (flag != 1 && order == p->order)
                    {
                        if (kdk_kare_build_base_env(p->value, call_back) == 1)
                        {
                            strncpy(value, p->value, sizeof(value) - 1);
                            flag = 1;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            CheckResult version = _kdk_dmvc_check_os_version(uri);

            if(version == CHECK_RESULT_V10SP1)
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

                if(runtime)
                {
                    if (check_kare_environment(runtime, value, sizeof(value)))
                    {
                        syslog(LOG_INFO, "runtime = %s, value = %s\n", runtime, value);
                        flag = 1;
                    }
                    else
                    {
                        int base = kdk_kare_build_base_env(runtime, call_back);
                        if(base == 1)
                        {
                            strncpy(value, runtime, sizeof(value) - 1);
                            flag = 1;
                        }
                    }
                    free(runtime);
                }
            }
        }

        syslog(LOG_INFO, "flag = %d, value = %s, typetag = %s\n", flag, value, typetag);
        
        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            if(typetag[0] != '\0')
            {
                //创建用户
                info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                        "/org/kylin/kare", // object to call on
                                                        "org.kylin.kare",  // interface to call on
                                                        "install_with_subsystem");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    goto err_out;
                }

                char *env = strdup(value);
                char *subsystem = strdup(typetag);
    
                syslog(LOG_INFO, "kdk : d-bus env : %s, subsystem : %s\n", env, subsystem);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &subsystem, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(env);
                    free(subsystem);
                    goto err_out;
                }
                free(env);
                free(subsystem);
            }
            else
            {
                //创建用户
                info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                        "/org/kylin/kare", // object to call on
                                                        "org.kylin.kare",  // interface to call on
                                                        "install");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    goto err_out;
                }

                char *env = strdup(value);
                syslog(LOG_INFO, "kdk : d-bus env : %s\n", env);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(env);
                    goto err_out;
                }
                free(env);
            }

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kare (%s)", dbus_message_get_error_name(replyMsg));
                _callbackinstall(0, -1, errinfo);
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

            if(ret)
            {
                mark = 1;
            }
            else
            {
                if(access(KDK_KARE_INSTALL_LOCK, F_OK) == 0)
                {
                    _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
                }
            }

        }
    }

    if(mark)
    {
        mark = 0;
        running_install = 1;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, value);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }

    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

// 检查字符串是否包含通配符
bool contains_wildcard(const char *pattern) {
    if (!pattern) return false;
    const char *wildcards = "*";
    
    for (const char *p = pattern; *p; p++) {
        if (strchr(wildcards, *p)) {
            return true;
        }
    }
    return false;
}

int kdk_package_install_package_offline(const char *uri, int list_mode, InstallCallBack call_back)
{
    char *info = NULL;
    char loginfo[512] = {0};
    info = getenv("PACKAGE_KARE_INFO");
    if (0 != access(uri, F_OK))
    {
        syslog(LOG_INFO, "[%s] 错误: '%s': 没有那个文件或目录\n", __func__, uri);
        if(info)
        {
            if(strcmp(info, "1") == 0)
            {
                snprintf(loginfo, sizeof(loginfo), "错误: '%s': 没有那个文件或目录", uri);
                printf("%s\n", loginfo);
            }
        }
        return 0;
    }
    syslog(LOG_INFO, "offline\n");
    int ret = 0;
    char filename[512] = "\0";
    char value[128] = "\0";
    char *dir = "/tmp/kysdk-package-download";
    _callbackinstall = call_back;
    switch (list_mode)
    {
    case KDK_PACKAGE_KAIMING:
        ret = kdk_kaiming_install_offline(uri, call_back);
        break;
    case KDK_PACKAGE_KARE:
        if (access(dir, F_OK) != 0) {
            // 创建目录，包括父目录
            char mkdir_cmd[512];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir %s", dir);
            int result = system(mkdir_cmd);
            if (result != 0) {
                syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                break;
            }
        }
        
        if (!extract_compat_file(dir, uri, "runtime_version", value))
        {
            break;
        }

        syslog(LOG_INFO, "runtime_version value = %s\n", value);
        if(value != '\0')
        {
            ret = kdk_kare_install_offline(uri, dir, value, call_back);
        }
        else
        {
            ret = kdk_kare_install_offline(uri, dir, NULL, call_back);
        }
        break;
    case KDK_PACKAGE_DEBIAN:
        if (access(dir, F_OK) != 0) {
            // 创建目录，包括父目录
            char mkdir_cmd[512];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir %s", dir);
            int result = system(mkdir_cmd);
            if (result != 0) {
                syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                break;
            }
        }

        memset(filename, 0, 512);
        strncpy(filename, uri, 511);
        
        char *file_name = kdk_get_package_name(filename);
        if(!file_name)
        {
            if(info)
            {
                if(strcmp(info, "1") == 0)
                {
                    snprintf(loginfo, sizeof(loginfo), "错误: %s 并非 Debian 格式的包文件", uri);
                    printf("%s\n", loginfo);
                }
            }
            break;
        }

        char package_name[512] = "\0";
        strcpy(package_name, file_name);
        free(file_name);

        if (kdk_get_boot_mode() == MODE_MAINTAIN)
        {
            char typebm[128] = "\0";
            if (!extract_compat_file(dir, uri, "bare_metal", typebm))
            {
                ;
            }
            //是特权标签，需要验签
            if(typebm[0] != '\0' && (strncmp(typebm, "yes", 3) == 0))
            {
                int result = kdk_verify_signature(uri, dir, typebm, 0);
                if(result)
                {
                    privilege_tag = 1;
                    ret = kdk_debian_install_offline(uri, call_back);
                }
            }
            syslog(LOG_INFO, "Under Debian package installation mode, in maintian mode, directly install the core system.");
            ret = kdk_debian_install_offline(uri, call_back);
            break;
        }

        if (!extract_compat_file(dir, uri, "runtime_version", value))
        {
            break;
        }

        syslog(LOG_INFO, "runtime_version value = %s\n", value);

        char typebm[128] = "\0";

        if (kdk_get_direct_install_whitelist(package_name))
        {
            // 白名单路线
            ret = kdk_debian_offline_install_in_whitelist(uri, value, package_name, call_back);
        }
        else if (kdk_is_privilege_label(dir, uri, typebm))
        {
            // 特权标记路线
            ret = kdk_debian_offline_install_in_privilege(dir, uri, value, call_back);
        }
        break;
    case KDK_PACKAGE_AUTO:
        strncpy(filename, uri, 511); 
        const char *dot = strrchr(filename, '.');
        // 如果没有找到点，或者点是文件名的第一个字符，则返回 false
        if (dot == NULL || dot == filename) {
            return 0;
        }
        // 检查从点开始的子字符串是否与后缀匹配
        if(strcmp(dot, ".ok") == 0)
        {
            ret = kdk_kaiming_install_offline(uri, call_back);
        }
        else
        {
            memset(filename, 0, 512);
            strncpy(filename, uri, 511);
            
            char *file_name = kdk_get_package_name(filename);
            if(!file_name)
            {
                if(info)
                {
                    if(strcmp(info, "1") == 0)
                    {
                        snprintf(loginfo, sizeof(loginfo), "错误: %s 并非 Debian 格式的包文件", uri);
                        printf("%s\n", loginfo);
                    }
                }
                break;
            }

            char package_name[512] = "\0";
            strcpy(package_name, file_name);
            free(file_name);

            bool is_ps = kdk_is_panshi_feature();
            if(!is_ps)
            {
                syslog(LOG_ERR, "%s, is_ps = %d\n", __func__, is_ps);
                ret = kdk_debian_install_offline(uri, call_back);
                break;
            }

            if (access(dir, F_OK) != 0) {
                // 创建目录，包括父目录
                char mkdir_cmd[512];
                snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir %s", dir);
                int result = system(mkdir_cmd);
                if (result != 0) {
                    syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                    break;
                }
            }

            if (!extract_compat_file(dir, uri, "runtime_version", value))
            {
                break;
            }

            syslog(LOG_INFO, "runtime_version value = %s\n", value);

            char typebm[128] = "\0";

            if (kdk_get_direct_install_whitelist(package_name))
            {
                // 白名单路线
                ret = kdk_debian_offline_install_in_whitelist(uri, value, package_name, call_back);
            }
            else if (kdk_is_privilege_label(dir, uri, typebm))
            {
                // 特权标记路线
                ret = kdk_debian_offline_install_in_privilege(dir, uri, value, call_back);
            }
            else
            {
                // 非在白名单中也不是特权标签，kare安装
                ret = kdk_kare_install_offline(uri, dir, value, call_back);
            }
        } 
        
        break;
    default:
        break;
    }
    privilege_tag = 0;

    return ret;
}

int kdk_package_install_package_online(const char *pkg_name, int list_mode, InstallCallBack call_back)
{
    if(!pkg_name)
        return 0;

    int ret = 0;
    syslog(LOG_ERR, "online\n");
    int deb = 0;
    char package_name[128] = "\0";
    char version[128] = "\0";
    char uri[1024] = "\0";
    char value[128] = "\0";
    const char *dir = "/tmp/kysdk-package-download";
    char* deb_path = NULL;
    _callbackinstall = call_back;
    switch (list_mode)
    {
    case KDK_PACKAGE_KAIMING:
        ret = kdk_kaiming_install_online(pkg_name, call_back);
        break;
    case KDK_PACKAGE_DEBIAN:
        if (access(dir, F_OK) != 0) {
            // 创建目录，包括父目录
            char mkdir_cmd[512];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", dir);
            int result = system(mkdir_cmd);
            if (result != 0) {
                syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                break;
            }
        }
        deb_path = kdk_download_package(pkg_name, dir);
        if(!deb_path)
        {
            break;
        }
        strcpy(uri, deb_path);
        free(deb_path);

        char package_name [128] = "\0";

        if(strstr(pkg_name, "="))
        {
            sscanf(pkg_name, "%[^=]= %s", package_name, version);
        }
        else
        {
            strncpy(package_name, pkg_name, sizeof(package_name) -1);
        }

        if (kdk_get_boot_mode() == MODE_MAINTAIN)
        {
            syslog(LOG_INFO, "Under Debian package installation mode, in maintian mode, directly install the core system.");
            kdk_debian_install_online(uri, pkg_name, call_back);
            break;
        }

        if (!extract_compat_file(dir, uri, "runtime_version", value))
        {
            break;
        }

        syslog(LOG_INFO, "runtime_version value = %s\n", value);

        char typebm[128] = "\0";

        if (kdk_get_direct_install_whitelist(package_name))
        {
            // 白名单路线
            ret = kdk_debian_online_install_in_whitelist(uri, value, pkg_name, call_back);
        }
        else if (kdk_is_privilege_label(dir, uri, typebm))
        {
            // 特权标记路线
            ret = kdk_debian_online_install_in_privilege(dir, uri, value, pkg_name, call_back);
        }
        break;
    case KDK_PACKAGE_KARE:
        if (access(dir, F_OK) != 0) {
            // 创建目录，包括父目录
            char mkdir_cmd[512];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", dir);
            int result = system(mkdir_cmd);
            if (result != 0) {
                syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                break;
            }
        }

        deb_path = kdk_download_package(pkg_name, dir);
        if(!deb_path)
        {
            break;
        }
        strcpy(uri, deb_path);
        free(deb_path);
        if (!extract_compat_file(dir, uri, "runtime_version", value))
        {
            break;
        }
        syslog(LOG_INFO, "runtime_version value = %s\n", value);
        if(value != '\0')
        {
            ret = kdk_kare_install_online(uri, dir, value, call_back);
        }
        else
        {
            ret = kdk_kare_install_online(uri, dir, NULL, call_back);
        }
        break;
    case KDK_PACKAGE_AUTO:
        ret = kdk_kaiming_install_online(pkg_name, call_back);
        if(!ret)
        {
            bool is_ps = kdk_is_panshi_feature();
            if(!is_ps)
            {
                syslog(LOG_ERR, "%s, is_ps = %d\n", __func__, is_ps);
                ret = kdk_debian_install_online(NULL, pkg_name, call_back);
                break;
            }

            if (access(dir, F_OK) != 0) {
                // 创建目录，包括父目录
                char mkdir_cmd[512];
                snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", dir);
                int result = system(mkdir_cmd);
                if (result != 0) {
                    syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
                    break;
                }
            }

            deb_path = kdk_download_package(pkg_name, dir);
            if(!deb_path)
            {
                break;
            }
            strcpy(uri, deb_path);
            free(deb_path);

            int flag = 0;
            int app = 0;
            char version[128] = "\0";
            char package_name [128] = "\0";

            if(strstr(pkg_name, "="))
            {
                sscanf(pkg_name, "%[^=]= %s", package_name, version);
            }
            else
            {
                strncpy(package_name, pkg_name, sizeof(package_name) -1);
            }

            if (!extract_compat_file(dir, uri, "runtime_version", value))
            {
                break;
            }

            syslog(LOG_INFO, "runtime_version value = %s\n", value);

            char typebm[128] = "\0";

            if (kdk_get_direct_install_whitelist(package_name))
            {
                // 白名单路线
                ret = kdk_debian_online_install_in_whitelist(uri, value, pkg_name, call_back);
            }
            else if (kdk_is_privilege_label(dir, uri, typebm))
            {
                // 特权标记路线
                ret = kdk_debian_online_install_in_privilege(dir, uri, value, pkg_name, call_back);
            }
            else
            {
                // 非在白名单中也不是特权标签，kare安装
                ret = kdk_kare_install_online(uri, dir, value, call_back);
            }            
        }
        break;
    default:
        break;
    }
    if (0 == access(uri, F_OK))
    {
        char rm_dir[256] = "\n";
        snprintf(rm_dir, sizeof(rm_dir), "rm %s", uri);
        system(rm_dir);
    }
    privilege_tag = 0;
    return ret;   
}

int kdk_package_install_package(const char *uri, int list_mode, InstallCallBack call_back)
{
    if(!uri)
        return 0;
    int ret = 0;
    char *dir = NULL;
    char filepath[256] = "\0";
    strncpy(filepath, uri, 255);
    dir = dirname(filepath);
    if(dir && strcmp(dir, ".") != 0)
    {
        ret = kdk_package_install_package_offline(uri, list_mode, call_back);
    }
    else
        ret = kdk_package_install_package_online(uri, list_mode, call_back);

    return ret;
}

static pthread_t thread_remove = 0;
static volatile int running_remove = 0;
static RemoveCallBack _callbackremove = NULL;

static void *dbus_kare_remove_listener_thread(char *arg)
{
    char *env = (char *)arg;
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int status = -1;
    char *errinfo = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;
    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kare',interface='org.kylin.kare',member='removeStatusChanged'",
                    &error);
    // dbus_connection_flush(connection);
    // dbus_connection_add_filter(connection, logout_signal_handler, NULL, NULL);
    // int32_t ret = -1;
    // 循环监听DBus消息
    while (true)
    {
        if (running_remove)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                continue;
            }

            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kare", "removeStatusChanged")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &status, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, status = %d, errinfo = %s\n", name, status, errinfo);
                    // if (status == 0)
                    // {
                    //     mark = 1;
                    //     break;
                    // }
                    // else if(status == -1)
                    // {
                    //     syslog(LOG_ERR, "kare remove fail! status : %s, errinfo = %s\n", status, errinfo);
                    //     break;
                    // }
                    // else
                    //     continue;

                    if (status != -1)
                    {
                        if(status == 0)
                        {
                            char loginfo[512] = "\0";
                            snprintf(loginfo, sizeof(loginfo), "卸载成功，软件包已从kare %s环境中移除", env);
                            _callbackremove(100, status, loginfo);
                            mark = 1;
                            break;
                        }
                        // status = -1;
                    }
                    else
                    {
                        _callbackremove(0, status, errinfo);
                        status = -1;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kare_remove(const char *pkg_name, RemoveCallBack call_back)
{
    if(!pkg_name)
        return -1;
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;
    // _callbackremove = call_back;

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusConnection *conn;
    DBusError err;
    char tmp_env[64] = {0};

    if (0 == access("/usr/bin/kare", F_OK))
    {
        // syslog(LOG_INFO, "kare\n");
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s %63s", tmp_name, tmp_version, tmp_env);
                if (count != 3)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    // pclose(fp);
                    flag = 1;
                    break;
                }
            }
            pclose(fp);
        }
    }

    if (1 == flag)
    {
        syslog(LOG_INFO, "kare\n");

        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (NULL == conn)
        {
            goto err_out;
        }

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        //创建用户
        info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                "/org/kylin/kare", // object to call on
                                                "org.kylin.kare",  // interface to call on
                                                "remove");               // method name
        if (!info_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
            goto err_out;
        }

        if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            goto err_out;
        }

        if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            goto err_out;
        }

        if (sendMsgPending == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            goto err_out;
        }

        dbus_connection_flush(conn);

        dbus_pending_call_block(sendMsgPending);
        replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
        if (replyMsg == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            goto err_out;
        }

        if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
            goto err_out;
        }

        DBusMessageIter args;
        bool *ret = false;

        if (!dbus_message_iter_init(replyMsg, &args))
        {
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            goto err_out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &ret);
        }

        dbus_message_unref(info_msg);
        dbus_message_unref(replyMsg);
        dbus_pending_call_unref(sendMsgPending);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        dbus_error_free(&err);

        if(ret)
        {
            running_remove = 1;
            pthread_create(&thread_remove, NULL, dbus_kare_remove_listener_thread, tmp_env);

            void *thread_result;
            pthread_join(thread_remove, &thread_result);

            if(thread_result)
            {
                mark = (int)(intptr_t)thread_result;
            }
        }
    }
    else
    {
        mark = -1;
    }

    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

static void *dbus_kaiming_remove_listener_thread()
{
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    int operateType = 0;
    int errorCode = -1;
    char *errinfo = NULL;
    int mark = 0;

    int timeout_sec = 180;
    int poll_interval = 1;
    int elapsed_time = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        return 0;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        return 0;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/org/kylin/kaiming',interface='org.kylin.kaiming.interface',member='finishsignal'",
                    &error);
    // 循环监听DBus消息
    while (true)
    {
        if(running_remove)
        {
            int result = dbus_connection_read_write(connection, poll_interval * 1000);
            
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }

            elapsed_time = 0;
        
            if(dbus_message_is_signal(message, "org.kylin.kaiming.interface", "finishsignal")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INT32, &operateType, DBUS_TYPE_INT32, &errorCode, DBUS_TYPE_STRING, &errinfo, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "name = %s, operateType = %d, errorCode = %d, errinfo = %s\n", name, operateType, errorCode, errinfo);
                    if (errorCode == 0)
                    {
                        mark = 1;
                        _callbackremove(100, 0, "卸载成功，软件包已从kaiming环境中移除");
                        break;
                    }
                    else
                    {
                        syslog(LOG_ERR, "kaiming remove fail, errorCode : %d, errinfo : %s\n", errorCode, errinfo);
                        _callbackremove(0, -1, errinfo);
                        break;
                    }
                }
            }
            dbus_message_unref(message);
        }
        else
        {
            dbus_connection_close(connection);
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_kaiming_remove(const char *pkg_name, RemoveCallBack call_back)
{
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;

    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessage *replyMsgBranch = NULL;

    running_remove = 1;
    // _callbackremove = call_back;
    pthread_create(&thread_remove, NULL, dbus_kaiming_remove_listener_thread, NULL);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        const char *kaiming = "/opt/kaiming-tools/bin/kaiming list";
        fp = popen(kaiming, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %*s %*s %*s %s %*s %*s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    pclose(fp);
                    flag = 1;
                    break;
                }
            }
        }
    }

    if (1 == flag)
    {
        syslog(LOG_INFO, "kaiming\n");
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            return 0;
        }     

        // branch_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
        //                                         "/org/kylin/kaiming",     // object to call on
        //                                         "org.kylin.kaiming.interface", // interface to call on
        //                                         "getLocalInfo");           // method name

        // if (!branch_msg)
        // { // -1 is default timeout
        //     syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
        //     return 0;
        // }

        // if (!dbus_message_append_args(branch_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
        // {
        //     syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
        //     return 0;
        // }

        // if (!dbus_connection_send_with_reply(conn, branch_msg, &sendMsgPendingBranch, -1))
        // {
        //     syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
        //     return 0;
        // }

        // if (sendMsgPendingBranch == NULL)
        // {
        //     syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
        //     return 0;
        // }

        // dbus_connection_flush(conn);

        // if (branch_msg)
        // {
        //     dbus_message_unref(branch_msg);
        // }

        // dbus_pending_call_block(sendMsgPendingBranch);

        // replyMsgBranch = dbus_pending_call_steal_reply(sendMsgPendingBranch);

        // if (replyMsgBranch == NULL)
        // {
        //     syslog(LOG_ERR, "kdk : get reply message fail !\n");
        //     return 0;
        // }

        // if (sendMsgPendingBranch)
        // {
        //     dbus_pending_call_unref(sendMsgPendingBranch);
        // }

        // if (dbus_message_get_type(replyMsgBranch) == DBUS_MESSAGE_TYPE_ERROR) 
        // {
        //     syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        //     dbus_message_unref(replyMsgBranch);
        //     return 0;
        // }

        // DBusMessageIter args_branch;
        // char *branch = NULL;
                
        // if (!dbus_message_iter_init(replyMsgBranch, &args_branch))
        // {
        //     dbus_message_unref(replyMsgBranch);
        //     syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
        //     return 0;
        // }
        // else
        // {
        //     dbus_message_iter_get_basic(&args_branch, &branch);
        // }

        // if (replyMsgBranch)
        // {
        //     dbus_message_unref(replyMsgBranch);
        // }

        // cJSON *json = cJSON_Parse(branch);
        // if (json == NULL) {
        //     return 0;
        // }

        // cJSON *branch_name = cJSON_GetObjectItem(json, "branch");
        // if (branch_name == NULL || !cJSON_IsString(branch_name)) {
        //     cJSON_Delete(json);
        //     return 0;
        // }

        // if(strlen(branch_name->valuestring) != 0)
        // {
            char *branch = "";
            //
            info_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                    "/org/kylin/kaiming",     // object to call on
                                                    "org.kylin.kaiming.interface", // interface to call on
                                                    "uninstall");           // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_STRING, &branch, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                goto err_out;
            }

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);

            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }

            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                goto err_out;
            }

            DBusMessageIter args;
            bool remove = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &remove);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            // dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

    }
    else
    {
        return -1;
    }

    void *thread_result;
    pthread_join(thread_remove, &thread_result);

    if(thread_result)
    {
        mark = (int)(intptr_t)thread_result;
    }

    return (int)(intptr_t)mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

/**
 * 检查软件包是否已被移除
 * @param package_name 包名
 * @return 1=已移除, 0=未移除, -1=错误
 */

int kdk_package_is_removed(const char *name)
{
    if (!g_utf8_validate(name, -1, NULL))
    {
        syslog(LOG_ERR, "Invalid UTF-8 string: %s", name);
        return -1;
    }

    FILE *fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return -1;

    char buffer[4097];
    char *val;
    int flag = 1;
    int mark = 0;
    char *ver = NULL;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
            {
                syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                flag = 0;
                break;
            }
        }
    }
    fclose(fp);
    return flag;
}

static void *dbus_debian_remove_listener_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int mark = 0;
    int stage_flags[5] = {0};
    int progress = 0;
    int flag = 0;
    int capture = 0;  // 是否开始捕获内容
    size_t total_len = 0;
    char output[8192] = "\0";
    int extra_line = 0;
    size_t len = 0;

    if(running_remove)
    {
        char line[1024] = "\0";
        while (fgets(line, sizeof(line), args->param_fp))
        {
            syslog(LOG_INFO, "debian remove info : %s\n", line);

            if (strstr(line, "正在卸载") && !stage_flags[1]) {
                progress += 20;  // 准备解压占 20%
                stage_flags[1] = 1;
                _callbackremove(progress, 1, line);
            } else if (strstr(line, "正在清除") && !stage_flags[2]) {
                progress += 30;  // 解压阶段占 30%
                stage_flags[2] = 1;
                _callbackremove(progress, 1, line);
            } else if (strstr(line, "正在处理用于") && !stage_flags[4]) {
                progress += 30;  // 触发器处理占 10%
                stage_flags[4] = 1;
                _callbackremove(progress, 1, line);
            }

            if (strstr(line, "dpkg: 依赖问题阻止了卸载") || (strstr(line, "dpkg: 警告:") && strstr(line, "不允许的操作")))
            {
                capture = 1;  // 开始捕获
            }

            if (capture) 
            {
                len = strnlen(line, STRNLEN_MAX);
                if (total_len + len < sizeof(output) - 1) 
                {
                    strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                    total_len += len;
                } else {
                    break;  // 防止溢出
                }
            }

            if (strstr(line, "dpkg: 处理软件包") && strstr(line, "时出错")) 
            {
                if(strnlen(output, STRNLEN_MAX) == 0)
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }
                extra_line = 1;  // 让下一行也加入输出
                capture = 1;
            }
            else if (strstr(line, "dpkg: 处理归档") && strstr(line, "时出错"))
            {
                if(strnlen(output, STRNLEN_MAX) == 0)
                {
                    len = strnlen(line, STRNLEN_MAX);
                    if (total_len + len < sizeof(output) - 1) 
                    {
                        strncat(output, line, sizeof(output) - strnlen(output, STRNLEN_MAX));
                        total_len += len;
                    } else {
                        break;  // 防止溢出
                    }
                }
                extra_line = 1;  // 处理 "dpkg: 处理归档 ..." 也要多输出一行
                capture = 1;
            } 
            else if (extra_line) 
            {
                capture = 0;
                extra_line = 0;  // 只多输出一行
            }
        }
        pclose(args->param_fp);

        int ret =  kdk_package_is_removed(args->param_name);
        syslog(LOG_INFO, "%s package is installed : %d\n", args->param_name, ret);

        if(ret == 1)
        {
            _callbackremove(100, 0, "卸载成功，软件包已从核心系统中移除");
            mark = 1;
            syslog(LOG_INFO, "debian remove success");
        }
        else
        {
            _callbackremove(0, -1, output);
            syslog(LOG_INFO, "debian remove fail");
        }
    }
    return (void *)(intptr_t)mark;
}

static int kdk_debian_remove(const char *pkg_name, RemoveCallBack call_back)
{
    int mark = 0;
    FILE *fp = NULL;
    int flag = 0;
    char cmd[1024] = {0};
    char *desktop_files[50];
    int file_count = 0;

    setup_signal_handlers();
    g_verify_end_count = 0;

    fp = fopen(DPKG_DB, "rt");
    if (!fp)
    {
        g_verify_end_count = 1;
        return 0;
    }

    char buffer[4097] = "\0";
    char *val;
    
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, pkg_name) == 0)
            {
                flag = 1;
                break;
            }
        }
    }
    fclose(fp);
    if (1 == flag)
    {
        if (kdk_get_boot_mode() == MODE_NORMAL)
        {
            /*是否存在特权模式白名单中，返回1是，0为不是*/
            int ret = kdk_get_direct_install_whitelist(pkg_name);
            /*是否存在特权标签中，返回1是，0为不是*/
            int tag = kdk_get_privileged_tag_packag(pkg_name);

            if(!ret && !tag)
            {
                syslog(LOG_INFO, "%s is a non-privileged package and cannot be uninstalled\n", pkg_name);
                printf("当前模式下，非特权包不允许卸载\n");
                goto err_out;
            }

            kdk_ns_mode_session_id = kdk_ns_mode_enter();
            if (kdk_ns_mode_session_id != 0)
            {
                kdk_ns_mode_flag_enter = 1; 
            }
            else
            {
                _callbackremove(0, -1, "ns_mode_enter() 失败");
                g_verify_end_count = 1;
                return 0;
            }
        }

        // 使用 dpkg -L 获取包中所有文件，并筛选出 /usr/share/applications/ 下的 desktop 文件
        snprintf(cmd, sizeof(cmd), "dpkg -L '%s' 2>/dev/null | grep \"^/usr/share/applications/.*\\.desktop$\"", pkg_name);

        fp = popen(cmd, "r");
        if (!fp) {
            syslog(LOG_ERR, "Failed to execute command: %s\n", cmd);
            goto err_out;
        }

        char desktop_path[512];

        // 读取每一行输出（每个desktop文件在系统中的路径）
        while (fgets(desktop_path, sizeof(desktop_path), fp)) {
            // 去除换行符
            strstripspace(desktop_path);
            
            // 确保路径有效
            if (strlen(desktop_path) == 0) {
                continue;
            }

            if(access(desktop_path, F_OK) != 0) {
                syslog(LOG_ERR, "uri %s not exits\n", desktop_path);
                continue;
            }

            // 获取文件名
            char *filename = strrchr(desktop_path, '/');
            if (!filename) {
                continue;
            }
            filename++; // 移动到'/'之后的部分

            desktop_files[file_count] = strdup(filename);
            if (!desktop_files[file_count]) {
                syslog(LOG_ERR, "Memory allocation failed for: %s\n", filename);
                continue;
            }
            
            file_count++;
        }
        pclose(fp);

        syslog(LOG_INFO, "debian\n");
        char command[256] = "\0";
        snprintf(command, sizeof(command), "dpkg.orig -P %s 2>&1", pkg_name);
        fp = popen(command, "r");
        if (!fp)
        {
            goto err_out;
        }

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        strncpy(args->param_name, pkg_name, sizeof(args->param_name) - 1);
        args->param_fp = fp;

        running_remove = 1;
        pthread_create(&thread_remove, NULL, dbus_debian_remove_listener_thread, (void *) args);

        void *thread_result;
        pthread_join(thread_remove, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
        free(args);

        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
    }
    else
    {
        mark = -1;
    }    

    if(mark)
    {
        if (file_count > 0) {
            
            for (int i = 0; i < file_count; i++) {
                char link_path[512] = {0};
                char prefixed_filename[512] = {0};
                snprintf(prefixed_filename, sizeof(prefixed_filename), "kylin-redistribution-symbolic-%s", desktop_files[i]);
                snprintf(link_path, sizeof(link_path), "/opt/system/resource/applications/%s", prefixed_filename);

                syslog(LOG_INFO, "desktop files path: %s\n", link_path);
                snprintf(cmd, sizeof(cmd), "rm \"%s\" > /dev/null 2>&1", link_path);
                syslog(LOG_INFO, "remove desktop files path: %s\n", cmd);
                system(cmd);
                free(desktop_files[i]);
                desktop_files[i] = NULL;
            }
        }
    }
    g_verify_end_count = 1;
    return mark;

err_out:
    if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
    {
        kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
        kdk_ns_mode_flag_enter = 0;
    }
    g_verify_end_count = 1;
    return 0;
}

int kdk_package_remove_package(const char *pkg_name, RemoveCallBack call_back)
{
    if(!pkg_name)
        return 0;

    int ret = 0;
    _callbackremove = call_back;
    FILE *rfp = NULL;
    char list[1024] = "\0";
    rfp = fopen("/var/lib/kysdk/kysdk-system/kypackage/unwlist", "r");
    if(!rfp)
        return false;
    while (fgets(list, sizeof(list), rfp))
    {
        strstripspace(list);
        if(strncmp(list, pkg_name, strnlen(list, STRNLEN_MAX)) == 0)
        {
            klog_info("%s list = %s, file_name = %s\n", __func__, list, pkg_name);
            fclose(rfp);
            _callbackremove(0, -1, "核心包不可卸载");
            return ret;
        }
    }
    fclose(rfp);

    ret = kdk_kare_remove(pkg_name, call_back);

    if(ret != -1)
        return ret;
    
    ret = kdk_kaiming_remove(pkg_name, call_back);

    if(ret != -1)
        return ret;

    ret = kdk_debian_remove(pkg_name, call_back);

    if(ret != -1)
        return ret;
    else
    {
        char *info = NULL;
        info = getenv("PACKAGE_KARE_INFO");
        if(info)
        {
            if(strcmp(info, "1") == 0)
            {
                char loginfo[256] = {0};
                snprintf(loginfo, sizeof(loginfo), "%s应用未安装", pkg_name);
                _callbackremove(100, 1, loginfo);
            }
             else
            {
                _callbackremove(100, 1, "应用未安装");
            }
        }
        else
        {
            _callbackremove(100, 1, "应用未安装");
        }
        ret = 0;
    }

    return ret;
}

char *kdk_package_get_version(const char *name)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Status")))
        {
            if (strstr(val, "install ok installed"))
                flag = 1;
            else
                flag = 0;
        }
        else if (flag && (val = lookup(buffer, "Version")))
            break;
    }
    fclose(fp);

    if (val)
        return strdup(val);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        char *str = NULL;

        dbus_connection_flush(connection);

        if (message)
        {
            // 释放message
            dbus_message_unref(message);
            message = NULL;
        }

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;

        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &str);
        }

        if (reply)
        {
            // 释放reply
            dbus_message_unref(reply);
            reply = NULL;
        }
        dbus_connection_unref(connection);
        connection = NULL;

        klog_info("get version Got string: %s\n", str);
        if(str)
        {
            cJSON *root;
            root = cJSON_Parse(str);

            cJSON *km_version = cJSON_GetObjectItem(root, "version");
            if(km_version)
            {
                klog_info("km_version->valuestring = %s\n", km_version->valuestring);
                
                if (km_version->valuestring && strnlen(km_version->valuestring, STRNLEN_MAX) != 0)
                {
                    return strdup(km_version->valuestring);
                }
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if (0 == strcmp(tmp_name, name))
                {
                    pclose(fp);
                    return strdup(tmp_version);
                }
            }
            pclose(fp);
        }
    }

    return NULL;
}

static int kdk_package_deb_is_installed(const char *name, const char *version)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (!fp)
        return -1;

    char buffer[4097];
    char *val;
    int flag = -1;
    int mark = 0;
    char *ver = NULL;
    while (fgets(buffer, 4096, fp))
    {
        if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
        {
            if ((val = lookup(buffer, "Package")))
            {
                if (strcmp(val, name) == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    mark = 1;
                }
            }
            else if(mark && (val = lookup(buffer, "Status")))
            {
                if (strcmp(val, "install ok installed") == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    mark = 1;
                }
                else
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    mark = 0;
                }
            }

            if(mark == 1)
            {
                if ((ver = lookup(buffer, "Version")))
                {
                    mark++;
                    if (strcmp(ver, version) == 0)
                    {
                        syslog(LOG_DEBUG, "ver = %s, version = %s", ver, version);
                        flag = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            if ((val = lookup(buffer, "Package")))
            {
                if (strcmp(val, name) == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    flag = 0;
                }
            }
            else if(!flag && (val = lookup(buffer, "Status")))
            {
                if (strcmp(val, "install ok installed") == 0)
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    flag = 0;
                    break;
                }
                else
                {
                    syslog(LOG_DEBUG, "val = %s, name = %s", val, name);
                    flag = -1;
                    break;
                }
            }
        }
    }
    fclose(fp);
    return flag;
}

static int kdk_package_kare_is_installed(const char *name, const char *version)
{
    FILE *fp = NULL;
    
    int flag = -1;
    if (0 == access("/usr/bin/kare", F_OK))
    {
        DBusConnection *conn;
        DBusError err;
        bool ret = false;
        char *pkgname = NULL;

        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (NULL == conn)
        {
            char line[1024];
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %s", tmp_name, tmp_version);
                if (count != 2)
                    continue;
                if(version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0)
                {
                    if (0 == strcmp(tmp_name, name) && 0 == strcmp(tmp_version, version))
                    {
                        syslog(LOG_DEBUG, "tmp_name = %s, tmp_version = %s", tmp_name, tmp_version);
                        flag = 0;
                        break;
                    }
                }
                else
                {
                    if (0 == strcmp(tmp_name, name))
                    {
                        syslog(LOG_DEBUG, "tmp_name = %s", tmp_name);
                        flag = 0;
                        break;
                    }
                }
            }
            pclose(fp);
        }
    }
    return flag;
}

static int kdk_package_kaiming_is_installed(const char *name, const char *version)
{
    int flag = -1;
    
    // 检查 kaiming 工具是否存在
    if (access("/opt/kaiming-tools/bin/kaiming", F_OK) != 0) {
        return -1;
    }

    DBusConnection *conn = NULL;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    char *pkgname = NULL;
    bool ret = false;

    dbus_error_init(&err);
    
    // 获取 D-Bus 连接
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (conn == NULL) {
        klog_err("D-Bus connection failed: %s\n", err.message);
        dbus_error_free(&err);
        goto cleanup;
    }

    if (dbus_error_is_set(&err)) {
        klog_err("Connection Error: %s\n", err.message);
        dbus_error_free(&err);
        goto cleanup;
    }

    // 构建包名
    if (version != NULL && strnlen(version, STRNLEN_MAX) != 0 && strcmp(version, "NULL") != 0) {
        char res[256] = {0};
        snprintf(res, sizeof(res), "%s/%s", name, version);
        pkgname = strdup(res);
    } else {
        pkgname = strdup(name);
    }

    if (!pkgname) {
        klog_err("Memory allocation failed for package name\n");
        goto cleanup;
    }

    // 创建 D-Bus 方法调用
    info_msg = dbus_message_new_method_call(
        "org.kylin.kaiming",           // target
        "/org/kylin/kaiming",          // object path
        "org.kylin.kaiming.interface", // interface
        "isInstalled"                  // method
    );

    if (!info_msg) {
        klog_err("Failed to create D-Bus message\n");
        goto cleanup;
    }

    // 添加参数
    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &pkgname, DBUS_TYPE_INVALID)) {
        klog_err("Failed to append D-Bus arguments\n");
        goto cleanup;
    }

    // 发送消息并等待回复
    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, DBUS_TIMEOUT_USE_DEFAULT)) {
        klog_err("Failed to send D-Bus message\n");
        goto cleanup;
    }

    if (!sendMsgPending) {
        klog_err("D-Bus pending call is NULL\n");
        goto cleanup;
    }

    dbus_connection_flush(conn);

    // 等待回复
    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    
    if (!replyMsg) {
        klog_err("Failed to get D-Bus reply\n");
        goto cleanup;
    }

    // 检查回复类型
    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) {
        klog_err("D-Bus error: %s\n", dbus_message_get_error_name(replyMsg));
        goto cleanup;
    }

    // 解析回复
    DBusMessageIter args;

    if (dbus_message_iter_init(replyMsg, &args)) {
        dbus_bool_t dbus_ret = FALSE;
        if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_BOOLEAN) {
            dbus_message_iter_get_basic(&args, &dbus_ret);
            ret = (bool)dbus_ret;
        }
    }

    // 设置返回标志
    flag = ret ? 0 : -1;

cleanup:
    // 清理资源
    if (pkgname) {
        free(pkgname);
    }
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return flag;
}

int kdk_package_is_installed(const char *name, const char *version)
{
    if (!g_utf8_validate(name, -1, NULL))
    {
        syslog(LOG_ERR, "Invalid UTF-8 string: %s, %s", name, version);
        return -1;
    }
    int flag = -1;
    flag = kdk_package_deb_is_installed(name, version);

    
    syslog(LOG_DEBUG, "%s flag = %d\n", __func__, flag);

    if (0 == flag)
        return flag;

    flag = kdk_package_kare_is_installed(name, version);
    
    if (0 == flag)
        return flag;

    flag = kdk_package_kaiming_is_installed(name, version);
    return flag;
}

char *kdk_package_get_description(const char *name)
{
    if (NULL == name)
        return NULL;

    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return NULL;

    char buffer[4097], result[4096];
    char *val = NULL;
    int flag = 0;
    while (fgets(buffer, 4096, fp))
    {
        if ((val = lookup(buffer, "Package")))
        {
            if (strcmp(val, name) == 0)
                flag = 1;
        }
        else if (flag && (val = lookup(buffer, "Description")))
        {
            strncat(result, val, sizeof(result) - strnlen(result, STRNLEN_MAX) - 1);
            // strcat(result,"\n");
            // while (fgets(buffer,4096, fp))
            // {
            //     if ((val = lookup(buffer, "Homepage")))
            //         break;
            //     strcat(result, buffer);
            // }
            break;
        }
    }
    fclose(fp);

    if (val)
        return strdup(val);

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        DBusConnection *connection = NULL;
        DBusMessage *message = NULL, *reply = NULL;
        DBusMessageIter iter;

        // 初始化DBus连接
        DBusError error;
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        char *str = NULL;

        dbus_connection_flush(connection);

        if (message)
        {
            // 释放message
            dbus_message_unref(message);
            message = NULL;
        }

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;

        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &str);
        }

        if (reply)
        {
            // 释放reply
            dbus_message_unref(reply);
            reply = NULL;
        }
        dbus_connection_unref(connection);
        connection = NULL;

        klog_info("get version Got string: %s\n", str);
        if(str)
        {
            cJSON *root;
            root = cJSON_Parse(str);

            cJSON *km_des = cJSON_GetObjectItem(root, "description");
            if(km_des)
            {
                klog_info("km_des->valuestring = %s\n", km_des->valuestring);
                
                if (km_des->valuestring && strnlen(km_des->valuestring, STRNLEN_MAX) != 0)
                {
                    strstripspace(km_des->valuestring);
                    return strdup(km_des->valuestring);
                }
            }
        }
        out:
            if(message)
            {
                dbus_message_unref(message);
            }
            if(reply)
            {
                dbus_message_unref(reply);
            }
            if(connection)
            {
                dbus_connection_unref(connection);
            }
    }

    return NULL;
}

char **kdk_package_get_code_path(const char *name)
{
    if (NULL == name)
        return NULL;

    char **result = calloc(1, sizeof(char *));
    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
    {
        free(result);
        return 0;
    }

    int i = 0;
    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            free(result);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
        {
            char *dir_name = dirname(line);
            if ((i > 0) && (0 == strcmp(result[i - 1], dir_name)))
                continue;
            char **tmp = realloc(result, sizeof(char *) * (i + 2));
            if (NULL == tmp)
            {
                for (; i; i--)
                {
                    free(result[i]);
                }
                free(result);
                result = NULL;
                break;
            }
            result = tmp;
            result[i++] = strdup(dir_name);
            result[i] = NULL;
        }
    }
    fclose(fp);
    return result;
}

int kdk_package_get_file_count(const char *name)
{
    if (NULL == name)
        return 0;

    int result = 0;
    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -L %s", name);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        struct stat statbuf;
        if (stat(line, &statbuf) != 0)
        {
            fclose(fp);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode))
            result++;
    }
    fclose(fp);
    return result;
} 

bool kdk_package_is_removable(const char *pkg_name)
{
    FILE *rfp = NULL;
    char list[1024] = "\0";
    rfp = fopen("/var/lib/kysdk/kysdk-system/kypackage/unwlist", "r");
    if(!rfp)
        return false;
    while (fgets(list, sizeof(list), rfp))
    {
        strstripspace(list);
        if(strncmp(list, pkg_name, strnlen(list, STRNLEN_MAX)) == 0)
        {
            klog_info("%s list = %s, file_name = %s\n", __func__, list, pkg_name);
            fclose(rfp);
            return false;
        }
    }
    fclose(rfp);

    const char *dpkg = "dpkg -l";
    FILE *fp = popen(dpkg, "r");
    if (NULL != fp)
    {
        char line[1024] = "\0";
        while (fgets(line, sizeof(line), fp))
        {
            char tmp_name[256], tmp_version[64];
            int count = sscanf(line, "%*s %s %*s %*s %*s", tmp_name);
            if (count != 1)
                continue;
            if (0 == strcmp(tmp_name, pkg_name))
            {
                return true;
            }
        }
    }

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        const char *kaiming = "/opt/kaiming-tools/bin/kaiming list";
        fp = popen(kaiming, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %*s %*s %*s %*s %*s %*s", tmp_name);
                if (count != 1)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    pclose(fp);
                    return true;
                }
            }
            pclose(fp);
        }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        const char *kare = "kare -l";
        fp = popen(kare, "r");
        if (NULL != fp)
        {
            char line[1024] = "\0";
            while (fgets(line, sizeof(line), fp))
            {
                char tmp_name[256], tmp_version[64];
                int count = sscanf(line, "%s %*s", tmp_name);
                if (count != 1)
                    continue;
                if (0 == strcmp(tmp_name, pkg_name))
                {
                    pclose(fp);
                    return true;
                }
            }
            pclose(fp);
        }
    }
    return false;
}

int kdk_package_verify_disk_space(const char *path)
{
    if (NULL == path)
        return 0;

    int result = 0;

    char cmd[128] = "\0";
    snprintf(cmd, sizeof(cmd), "dpkg -I %s", path);
    FILE *fp = popen(cmd, "r");
    if (NULL == fp)
        return 0;

    char line[1024];
    char *val = NULL;
    while (fgets(line, 1024, fp))
    {
        strstripspace(line);
        if ((val = lookup(line, "Installed-Size")))
        {
            break;
        }
    }
    fclose(fp);

    if (NULL == val)
        return 0;

    FILE *mounts = fopen("/proc/mounts", "r");

    char partition[100], mount_point[100];
    while (fscanf(mounts, "%99s %99s", partition, mount_point) != EOF)
    {
        if (strcmp(mount_point, "/") == 0)
        {
            break;
        }
    }
    fclose(mounts);

    struct statfs fs_stat;
    if (statfs(partition, &fs_stat) < 0)
    {
        return 0;
    }
    long install_size = atol(val) * 1024;
    long avail_space = fs_stat.f_bavail * fs_stat.f_bsize;

    return avail_space > install_size;
}

char *kdk_package_get_default_browser(void)
{
    return read_default_app("x-scheme-handler/http");
}

char *kdk_package_get_default_image_viewer(void)
{
    return read_default_app("image/bmp");
}

char *kdk_package_get_default_audio_player(void)
{
    return read_default_app("audio/mpeg");
}

char *kdk_package_get_default_video_player(void)
{
    return read_default_app("video/mp4");
}

char *kdk_package_get_default_pdf_viewer(void)
{
    return read_default_app("application/pdf");
}

char *kdk_package_get_default_word_viewer(void)
{
    return read_default_app("application/wps-office.docx");
}

char *kdk_package_get_default_excel_viewer(void)
{
    return read_default_app("application/wps-office.xls");
}

char *kdk_package_get_default_ppt_viewer(void)
{
    return read_default_app("application/wps-office.ppt");
}

kdk_app_info* kdk_package_get_startmenu_list(void)
{
    kdk_app_info *programlist = (kdk_app_info *)calloc(1, sizeof(kdk_app_info));
    if (!programlist)
        return NULL;

    get_desktop_info(KDK_DEKSTOP_FILE_PATH, programlist, 1);

    char *homeDir = getenv("HOME");
    char filepath[PROGRAM_SIZE] = {0};
    snprintf(filepath, sizeof(filepath), "%s%s", homeDir, KDK_LOCALE_DEKSTOP_FILE_PATH);
    get_desktop_info(filepath, programlist, 1);

    get_desktop_info(KDK_SNAPD_DEKSTOP_FILE_PATH, programlist, 1);

    get_desktop_info(KDK_FLATPAK_DEKSTOP_FILE_PATH, programlist, 1);

    return programlist;
}

kdk_app_info *kdk_package_free_startmenu_list(kdk_app_info *list)
{
    if (NULL == list)
        return;
    for (int i = 0; i < list->nums; i++)
        free(list->list[i]);
    free(list);
}

kdk_app_info *kdk_package_get_application_list(void)
{
    kdk_app_info *programlist = (kdk_app_info *)calloc(1, sizeof(kdk_app_info));
    if (!programlist)
        return NULL;

    get_desktop_info(KDK_DEKSTOP_FILE_PATH, programlist, 0);

    return programlist;
}

void kdk_package_free_app_info(kdk_app_info *list)
{
    if (NULL == list)
        return;
    for (int i = 0; i < list->nums; i++)
        free(list->list[i]);
    free(list);
}

void kdk_package_unregister_install_package_handle()
{
    if (NULL != _callbackinstall)
    {
        running_install = 0;
        _callbackinstall = NULL;
        // pthread_exit(thread_install);
    }
}

void kdk_package_unregister_remove_package_handle()
{
    if (NULL != _callbackremove)
    {
        running_remove = 0;
        _callbackremove = NULL;
        pthread_join(thread_remove, NULL);
    }
}

typedef struct {
    FILE *fp;
    pid_t pid;
} cmd_pipe_t;

static cmd_pipe_t cmd_popen(const char *file, char *const argv[])
{
    cmd_pipe_t result = {NULL, -1};
    int pipefd[2];
    if (pipe(pipefd) < 0)
        return result;

    pid_t pid = fork();
    if (pid < 0)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        return result;
    }

    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execvp(file, argv);
        _exit(127);
    }

    close(pipefd[1]);
    result.fp = fdopen(pipefd[0], "r");
    if (!result.fp)
    {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    }
    result.pid = pid;
    return result;
}

static int cmd_pclose(cmd_pipe_t *cp)
{
    if (!cp || !cp->fp)
        return -1;
    fclose(cp->fp);
    cp->fp = NULL;
    int status = 0;
    if (cp->pid > 0)
    {
        waitpid(cp->pid, &status, 0);
        cp->pid = -1;
    }
    return status;
}

char* kdk_package_get_name(char *desktop)
{
    if(!desktop || desktop[0] == '\0')
        return NULL;

    char package_name[256] = "\0";
    char file_path[256] = "\0";
    char line[1024] = "\0";
    char *name = NULL;
    int found = 0;

    /* 判断入参是绝对路径还是纯文件名，若是绝对路径则提取basename用于回退查询 */
    int is_absolute_path = (desktop[0] == '/');
    char basename_buf[256] = "\0";
    if (is_absolute_path)
    {
        const char *last_slash = strrchr(desktop, '/');
        if (last_slash)
            snprintf(basename_buf, sizeof(basename_buf), "%s", last_slash + 1);
        else
            snprintf(basename_buf, sizeof(basename_buf), "%s", desktop);
    }

    /* ===== 1. dpkg-query 使用原始入参查询 ===== */
    
    char *dpkg_argv[] = {"dpkg-query", "-S", desktop, NULL};
    cmd_pipe_t dpkg_cp = cmd_popen("dpkg-query", dpkg_argv);
    FILE *fp = dpkg_cp.fp;
    if (fp)
    {
        while(fgets(line, sizeof(line), fp) != NULL)
        {
            sscanf(line, "%255[^:]: %255s", package_name, file_path);
            if(strcmp(desktop, file_path) == 0)
            {
                found = 1;
                break;
            }
            package_name[0] = '\0';
        }
        cmd_pclose(&dpkg_cp);
    }

    /* dpkg 用绝对路径未找到时，使用basename重试 */
    if(!found && is_absolute_path && basename_buf[0] != '\0')
    {
        memset(line, 0, sizeof(line));
        char *dpkg_argv[] = {"dpkg-query", "-S", basename_buf, NULL};
        cmd_pipe_t dpkg_cp = cmd_popen("dpkg-query", dpkg_argv);
        fp = dpkg_cp.fp;
        if (fp)
        {
            size_t blen = strlen(basename_buf);
            while(fgets(line, sizeof(line), fp) != NULL)
            {
                char tmp_pkg[256] = "\0";
                char tmp_path[256] = "\0";
                sscanf(line, "%255[^:]: %255s", tmp_pkg, tmp_path);
                size_t plen = strlen(tmp_path);
                if (plen >= blen && strcmp(tmp_path + plen - blen, basename_buf) == 0)
                {
                    strncpy(package_name, tmp_pkg, sizeof(package_name) - 1);
                    found = 1;
                    break;
                }
            }
            cmd_pclose(&dpkg_cp);
        }
    }

    if(found && package_name[0] != '\0')
    {
        name = (char *)malloc(strlen(package_name) + 1);
        if(!name)
            return NULL;
        strcpy(name, package_name);
        return name;
    }

    /* ===== 2. kare 使用原始入参查询 ===== */
    found = 0;
    memset(line, 0, sizeof(line));
    memset(package_name, 0, sizeof(package_name));
    memset(file_path, 0, sizeof(file_path));

    char *kare_argv[] = {"kare", "-S", desktop, NULL};
    cmd_pipe_t kare_cp = cmd_popen("kare", kare_argv);
    fp = kare_cp.fp;
    if (fp)
    {
        while(fgets(line, sizeof(line), fp) != NULL)
        {
            sscanf(line, "%255[^:]: %255s", package_name, file_path);
            if(strcmp(desktop, file_path) == 0)
            {
                found = 1;
                break;
            }
            package_name[0] = '\0';
        }
        cmd_pclose(&kare_cp);
    }

    /* kare 用绝对路径未找到时，使用basename重试 */
    if(!found && is_absolute_path && basename_buf[0] != '\0')
    {
        memset(line, 0, sizeof(line));
        char *kare_argv[] = {"kare", "-S", basename_buf, NULL};
        cmd_pipe_t kare_cp = cmd_popen("kare", kare_argv);
        FILE *fp = kare_cp.fp;
        if (fp)
        {
            size_t blen = strlen(basename_buf);
            while(fgets(line, sizeof(line), fp) != NULL)
            {
                char tmp_pkg[256] = "\0";
                char tmp_path[256] = "\0";
                sscanf(line, "%255[^:]: %255s", tmp_pkg, tmp_path);
                size_t plen = strlen(tmp_path);
                if (plen >= blen && strcmp(tmp_path + plen - blen, basename_buf) == 0)
                {
                    strncpy(package_name, tmp_pkg, sizeof(package_name) - 1);
                    found = 1;
                    break;
                }
            }
            cmd_pclose(&kare_cp);
        }
    }

    if(found && package_name[0] != '\0')
    {
        name = (char *)malloc(strlen(package_name) + 1);
        if(!name)
            return NULL;
        strcpy(name, package_name);
        return name;
    }

    /* ===== 3. 开明：读取desktop文件查找X-Kaiming字段 ===== */
    memset(package_name, 0, sizeof(package_name));
    memset(line, 0, sizeof(line));

    /* 绝对路径：直接读取文件 */
    if(is_absolute_path && 0 == access(desktop, F_OK))
    {
        FILE *fp = fopen(desktop, "r");
        if(fp)
        {
            while (fgets(line, sizeof(line), fp))
            {
                if(strstr(line, "X-Kaiming"))
                {
                    sscanf(line, "%*[^=]=%255s", package_name);
                }
            }
            fclose(fp);
        }
    }

    if(package_name[0] != '\0')
    {
        name = (char *)malloc(strlen(package_name) + 1);
        if(!name)
            return NULL;
        strcpy(name, package_name);
        return name;
    }

    return NULL;
}

bool kdk_package_is_removable_by_desktop(const char *desktop)
{
    klog_info("desktop = %s\n", desktop);
    if (!desktop)
    {
        return false;
    }

    if (access(desktop, F_OK) != 0)
    {
        klog_info("desktop = %s failed\n", desktop);
        return false;
    }

    FILE *fp = NULL;
    char line[1024] = "\0";

    char *file_name = basename(desktop);
    klog_info("MODE_MAINTAIN file_name = %s\n", file_name);
    fp = fopen("/var/lib/kysdk/kysdk-system/kypackage/udeblist", "r");
    if(!fp)
        return false;
    while (fgets(line, sizeof(line), fp))
    {
        strstripspace(line);
        if(strncmp(line, file_name, strnlen(file_name, STRNLEN_MAX)) == 0)
        {
            klog_info("MODE_MAINTAIN line = %s, file_name = %s\n", line, file_name);
            fclose(fp);
            return false;
        }
    }
    fclose(fp);

    /* 判断desktop文件Categories字段是否包含Android，若包含则返回true */
    fp = fopen(desktop, "r");
    if (fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            strstripspace(line);
            if (strncmp(line, "Categories=", 11) == 0)
            {
                if (strstr(line + 11, "Android"))
                {
                    klog_info("Categories contains Android, removable by desktop\n");
                    fclose(fp);
                    return true;
                }
                break;
            }
        }
        fclose(fp);
    }

    char* pkg_name = kdk_package_get_name(desktop);
    if(!pkg_name)
    {
        klog_info("The package name where the desktop file is located has not been found\n");
        return false;
    }
    free(pkg_name);
    return true;
}

kdk_cmd_node* kdk_package_cmd_init(const char *cmd)
{
    if(!cmd)
    {
        return NULL;
    }
    kdk_cmd_node *cmd_node = NULL;
    cmd_node = (kdk_cmd_node *)calloc(1, sizeof(kdk_cmd_node));
    if (cmd_node == NULL) 
    {
        return NULL;
    }
    strncpy(cmd_node->cmd, cmd, sizeof(cmd_node->cmd) - 1); 
    cmd_node->finished = NULL;
    cmd_node->update = NULL;
    cmd_node->stat = -1;
    return cmd_node;
}

static pthread_t thread_cmd = 0;
static volatile int running_cmd = 0;
static FinishedCallBack _finished = NULL;
static UpdateCallBack _update = NULL;

static void dbus_debian_cmd_listener_thread(void *arg)
{
    cmd_pipe_t *cp = (cmd_pipe_t *)arg;
    FILE *fp = cp->fp;
    char cmd_line[512] = "\0";

    char *home_dir = kdk_get_home_dir();
    char log_path[512] = "\0";
    snprintf(log_path, sizeof(log_path), "%s/.log/kysdk_redirect.log", home_dir);
    syslog(LOG_DEBUG, "log_path = %s\n", log_path);
    int fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        klog_err("open\n");
        cmd_pclose(cp);
        free(cp);
        return ;
    }
    while (fgets(cmd_line, 512, fp))
    {
        // 写入内容
        if (write(fd, cmd_line, strnlen(cmd_line, STRNLEN_MAX)) < 0)
        {
            klog_err("write\n");
        }

        strskipspace(cmd_line);
        _update(cmd_line);
    }
    cmd_pclose(cp);
    free(cp);
    close(fd);

    fp = fopen(log_path, "r");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        if (file_size == -1)
        {
            fclose(fp);
            return ;
        }

        rewind(fp);
        char *content = (char *)malloc(file_size + 1);
        if (!content)
        {
            klog_err("Failed to allocate memory\n");
            fclose(fp);
            return ;
        }

        size_t read_size = fread(content, 1, file_size, fp);
        if (read_size != file_size)
        {
            free(content);
            fclose(fp);
            return ;
        }

        content[read_size] = '\0'; // Ensure null-termination
        syslog(LOG_DEBUG, "File content: %s", content);
        _finished(0, content, NULL);
        free(content);
        fclose(fp);
    }
}

static void dbus_kare_cmd_listener_thread(void *arg)
{
    char *package_pid = (char *)arg;
    DBusConnection *connection;
    DBusError error;
    char *name = NULL;
    char *standard_err = NULL;
    char *pkg_pid = NULL;
    char *standard_log = NULL;

    int timeout_sec = 10;
    int poll_interval = 1;
    int elapsed_time = 0;
    int flag = 0;

    int mark = 0;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        syslog(LOG_ERR, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        free(package_pid);
        return ;
    }
    if (connection == NULL)
    {
        syslog(LOG_ERR, "Connection Null\n");
        free(package_pid);
        return ;
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/kareAgent',interface='com.kylin.kareAgent',member='kareStandardLogUpdate'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/kareAgent',interface='com.kylin.kareAgent',member='kareErrorLogUpdate'",
                    &error);
    dbus_bus_add_match(connection,
                    "type='signal',path='/com/kylin/kareAgent',interface='com.kylin.kareAgent',member='kareLogFinished'",
                    &error);
    // 循环监听DBus消息
    while (true)
    {
        if (running_cmd)
        {
            dbus_connection_read_write(connection, poll_interval * 1000);
            DBusMessage *message = dbus_connection_pop_message(connection);
            if(message == NULL){
                elapsed_time += poll_interval;
                if(elapsed_time >= timeout_sec)
                    break;
                
                continue;
            }
            elapsed_time = 0;

            if(dbus_message_is_signal(message, "com.kylin.kareAgent", "kareStandardLogUpdate")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_log, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_log = %s", package_pid, pkg_pid, standard_log);
                    
                    _update(standard_log);
                }
            }

            if(dbus_message_is_signal(message, "com.kylin.kareAgent", "kareErrorLogUpdate")){
                if(dbus_message_get_args(message, &error,  DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_err, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_err = %s", package_pid, pkg_pid, standard_err);
                    
                    _update(standard_err);
                }
            }
        
            if(dbus_message_is_signal(message, "com.kylin.kareAgent", "kareLogFinished")){
                if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &pkg_pid, DBUS_TYPE_STRING, &standard_log, DBUS_TYPE_STRING, &standard_err, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_INFO, "package_pid = %s, pkg_pid = %s, standard_log = %s, standard_err = %s", package_pid, pkg_pid, standard_log, standard_err);
                    flag = 1;

                    if(standard_err && standard_log)
                    {
                        _finished(2, standard_log, standard_err);
                    }
                    else if(standard_log)
                    {
                        _finished(0, standard_log, NULL);
                    }
                    else if (standard_err)
                    {
                        _finished(1, NULL, standard_err);
                    }
                }
            }
            dbus_message_unref(message);
            if(flag)
            {
                running_cmd = 0;
                break;
            }
        }
    }
    dbus_connection_unref(connection);
    free(package_pid);
    syslog(LOG_DEBUG, "dbus_kare_cmd_listener_thread exit\n");
    pthread_exit(NULL);
}

int kdk_package_launch_cmd(kdk_cmd_node* node)
{
    if(!node)
    {
        return -1;
    }

    FILE *fp = NULL;
    char command[64] = "\0";
    if(strncmp(node->cmd, "pkexec", 6) == 0)
    {
        sscanf(node->cmd, "pkexec %s %*s", command);
        syslog(LOG_DEBUG, "node->cmd = %s, command = %s\n", node->cmd, command);
    }
    else
    {
        sscanf(node->cmd, "%s %*s", command);
        syslog(LOG_DEBUG, "command = %s\n", command);
    }

    char line[256] = "\0";
    char path_cmd[256] = "\0";
    char pkg_name[256] = "\0";
    char deb_cmd[256] = "\0";
    char resolved_path[4096] = "\0";
    char *basename_cmd = NULL;

    char *type_argv[] = {"which", "-a", command, NULL};
    syslog(LOG_DEBUG, "which -a %s\n", command);
    cmd_pipe_t type_cp = cmd_popen("which", type_argv);
    fp = type_cp.fp;
    if(fp)
    {
        while (fgets(line, sizeof(line), fp))
        {
            syslog(LOG_DEBUG, "line = %s\n", line);
            sscanf(line, "%255s", path_cmd);
            syslog(LOG_DEBUG, "path_cmd = %s\n", path_cmd);

            basename_cmd = strrchr(path_cmd, '/');
            basename_cmd = basename_cmd ? basename_cmd + 1 : path_cmd;

            if(strcmp(command, basename_cmd) == 0)
            {
                char *dpkg_argv[] = {"dpkg", "-S", path_cmd, NULL};
                syslog(LOG_DEBUG, "dpkg -S %s\n", path_cmd);
                cmd_pipe_t dpkg_cp = cmd_popen("dpkg", dpkg_argv);
                FILE *path_fp = dpkg_cp.fp;
                if(path_fp)
                {
                    char pkg_line[256] = "\0";
                    fgets(pkg_line, sizeof(line), path_fp);
                    if(pkg_line[0] != '\0')
                    {
                        sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                        syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);

                        if(pkg_name[0] != 0 && strstr(pkg_name, "dpkg-query"))
                        {
                            cmd_pclose(&dpkg_cp);
                            break;
                        }
                        else
                        {
                            realpath(path_cmd, resolved_path);
                            if (0 == access(resolved_path, F_OK))
                            {
                                char *dpkg_argv2[] = {"dpkg", "-S", resolved_path, NULL};
                                syslog(LOG_DEBUG, "dpkg -S %s\n", resolved_path);
                                cmd_pipe_t dpkg_cp2 = cmd_popen("dpkg", dpkg_argv2);
                                FILE *dpkg_fp = dpkg_cp2.fp;
                                if(dpkg_fp)
                                {
                                    char pkg_line2[256] = "\0";
                                    fgets(pkg_line2, sizeof(line), dpkg_fp);
                                    if(pkg_line2[0] != '\0')
                                    {
                                        sscanf(pkg_line2, "%127[^:]: %*s",pkg_name);
                                        syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
                                        if(pkg_name[0] != 0 && strcmp(pkg_name, "dpkg-query") != 0)
                                        {
                                            cmd_pclose(&dpkg_cp2);
                                            cmd_pclose(&dpkg_cp);
                                            break;
                                        }
                                    }
                                    cmd_pclose(&dpkg_cp2);
                                }
                            }
                        }
                    }
                    cmd_pclose(&dpkg_cp);
                }
            }
        }
        cmd_pclose(&type_cp);
    }

    if(pkg_name[0] != 0 && strcmp(pkg_name, "dpkg-query") != 0)
    {
        cmd_pipe_t *debian_cp = (cmd_pipe_t *)malloc(sizeof(cmd_pipe_t));
        if (!debian_cp)
            return -1;
        char *sh_argv[] = {"sh", "-c", node->cmd, NULL};
        *debian_cp = cmd_popen("sh", sh_argv);
        fp = debian_cp->fp;
        if(fp)
        {
            node->stat = 1;
            _update = node->update;
            _finished = node->finished;
            pthread_create(&thread_cmd, NULL, dbus_debian_cmd_listener_thread, (void *) debian_cp);
            return 0;
        }
        cmd_pclose(debian_cp);
        free(debian_cp);
    }
    else
    {
        syslog(LOG_DEBUG, "kare\n");
        char *kare_argv[] = {"kare", "-S", command, NULL};
        syslog(LOG_DEBUG, "kare -S %s\n", command);
        cmd_pipe_t kare_cp = cmd_popen("kare", kare_argv);
        FILE *path_fp = kare_cp.fp;
        if(path_fp)
        {
            char pkg_line[256] = "\0";
            fgets(pkg_line, sizeof(line), path_fp);
            if(pkg_line[0] != '\0')
            {
                sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
            }
            cmd_pclose(&kare_cp);
        }
    }

    if(pkg_name[0] != 0 && strcmp(pkg_name, "kare-query") != 0)
    {
        char pkg_pid_buf[256] = "\0";
        pid_t pid = getpid();
        snprintf(pkg_pid_buf, sizeof(pkg_pid_buf), "%s_%d", pkg_name, pid);
        char *pkg_pid = strdup(pkg_pid_buf);
        if (!pkg_pid)
            return -1;
        node->stat = 0;
        _finished = node->finished;
        _update= node->update;
        running_cmd = 1;
        pthread_create(&thread_cmd, NULL, dbus_kare_cmd_listener_thread, (void*)pkg_pid);

        pid_t kare_pid = fork();
        if (kare_pid == 0)
        {
            execl("/bin/sh", "sh", "-c",
                  "exec kare -p \"$1\" run \"$2\" $3",
                  "sh", pkg_pid, pkg_name, node->cmd, NULL);
            _exit(127);
        }
        else if (kare_pid > 0)
        {
            int status;
            waitpid(kare_pid, &status, 0);
            if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
            {
                return 0;
            }
        }
    }
    return -1;
}

void kdk_package_free_cmd(kdk_cmd_node* node)
{
    if(node)
    {
        free(node);
    }
}

void kdk_package_cmd_close(kdk_cmd_node* node)
{
    if(node)
    {
        if(node->stat == 1)
        {
            syslog(LOG_DEBUG, "node->cmd = %s\n", node->cmd);
            char line[115200] = "\0";
            char pid_cmd[512] = "\0";
            char pid[32] = "\0";
            char command[11520] = "\0";
            snprintf(pid_cmd, sizeof(pid_cmd), "ps -ef | grep \"%s\"", node->cmd);
            FILE *fp = popen(pid_cmd, "r");
            if(fp)
            {
                while (fgets(line, sizeof(line), fp))
                {
                    sscanf(line, "%*s %s %*s %*s %*s %*s %*s %11520[^\n]", pid, command);
                    syslog(LOG_DEBUG, "command = %s, pid = %s\n", command, pid);
                    if(strcmp(command, node->cmd) == 0)
                    {
                        break;
                    }
                    
                }
                pclose(fp);
            }
            char kill_cmd[512] = "\0";
            snprintf(kill_cmd, sizeof(kill_cmd), "kill -9 %s", pid);
            syslog(LOG_DEBUG, "kill_cmd = %s\n", kill_cmd);
            system(kill_cmd);
        }
        else
        {
            syslog(LOG_DEBUG, "kare\n");
            char command[64] = "\0";

            if(strncmp(node->cmd, "pkexec", 6) == 0)
            {
                sscanf(node->cmd, "pkexec %s %*s", command);
                syslog(LOG_DEBUG, "node->cmd = %s, command = %s\n", node->cmd, command);
            }
            else
            {
                sscanf(node->cmd, "%s %*s", command);
                syslog(LOG_DEBUG, "command = %s\n", command);
            }

            char pkg_name[128] = "\0";
            char dpkg_cmd[128] = "\0";
            char line[512] = "\0";
            snprintf(dpkg_cmd, sizeof(dpkg_cmd), "kare -S %s", command);
            FILE *path_fp = popen(dpkg_cmd, "r");
            if(path_fp)
            {
                char pkg_line[512] = "\0";
                fgets(pkg_line, sizeof(line), path_fp);
                if(pkg_line[0] != "\0")
                {
                    sscanf(pkg_line, "%[^:]: %*s",pkg_name);
                    syslog(LOG_DEBUG, "pkg_name = %s\n", pkg_name);
                    pclose(path_fp);
                }
                else
                {
                    pclose(path_fp);
                }
            }

            if(pkg_name[0] != 0 && strcmp(pkg_name, "kare-query") != 0)
            {
                char cmd_all[1024] = "\0";
                pid_t pid = getpid();

                char pkg_pid[256] = "\0";
                snprintf(pkg_pid, sizeof(pkg_pid), "%s_%d", pkg_name, pid);

                DBusMessage *info_msg = NULL;
                DBusPendingCall *sendMsgPending = NULL;
                DBusMessage *replyMsg = NULL;
                DBusConnection *conn;
                DBusError err;
                dbus_error_init(&err);
                conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
                if (NULL == conn)
                {
                    syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                    return ;
                }

                if (dbus_error_is_set(&err))
                {
                    // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                    dbus_error_free(&err);
                }

                //创建用户
                info_msg = dbus_message_new_method_call("com.kylin.kareAgent",  // target for the method call
                                                        "/com/kylin/kareAgent", // object to call on
                                                        "com.kylin.kareAgent",  // interface to call on
                                                        "killApplication");               // method name
                if (!info_msg)
                { // -1 is default timeout
                    syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                    return ;
                }

                char *package_pid = strdup(pkg_pid);
                syslog(LOG_INFO, "package_pid : %s\n", package_pid);
                if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &package_pid, DBUS_TYPE_INVALID))
                {
                    syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                    free(package_pid);
                    return ;
                }
                free(package_pid);

                if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
                {
                    syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                    return ;
                }

                if (sendMsgPending == NULL)
                {
                    syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                    return ;
                }

                dbus_connection_flush(conn);

                dbus_pending_call_block(sendMsgPending);
                replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
                if (replyMsg == NULL)
                {
                    syslog(LOG_ERR, "kdk : get reply message fail !\n");
                    return ;
                }


                if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
                {
                    syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                    return ;
                }

                DBusMessageIter args;
                bool *ret = false;

                if (!dbus_message_iter_init(replyMsg, &args))
                {
                    syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                    return ;
                }
                else
                {
                    dbus_message_iter_get_basic(&args, &ret);
                }

                dbus_message_unref(info_msg);
                dbus_message_unref(replyMsg);
                dbus_pending_call_unref(sendMsgPending);
                dbus_connection_close(conn);
                dbus_connection_unref(conn);
                dbus_error_free(&err);
            }
        }
    }
}

char **kdk_package_get_standard_path(const char *name)
{
    if (!name) 
        return NULL;

    FILE *fp = fopen("/etc/ostree/compat-paths.conf", "r");
    if (!fp) 
        return NULL;

    char line[1024] = "\0";
    char **res = NULL;
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char *eq_pos = strchr(line, '=');
        if (!eq_pos) 
            continue;

        *eq_pos = '\0'; // 分割键值
        char *current_key = line;
        char *current_value = eq_pos + 1;

        strstripspace(current_value);

        if (strcmp(name, current_key) != 0)
            continue;

        // 分割路径
        char *saveptr;
        char *token = strtok_r(current_value, ":", &saveptr);
        while (token) 
        {
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) 
                goto err_out;
            res = tmp;
            res[counts] = strdup(token);
            if (!res[counts]) 
                goto err_out;
            counts++;
            token = strtok_r(NULL, ":", &saveptr);
        }

        // 添加 NULL 结尾
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        res[counts] = NULL;
        fclose(fp);
        return res;
    }
    fclose(fp);
    return NULL;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

char **kdk_package_get_standard_path_lists()
{
    FILE *fp = fopen("/etc/ostree/compat-paths.conf", "r");
    if (!fp) 
        return NULL;

    char line[1024] = "\0";
    char **res = NULL;
    int counts = 0;

    while (fgets(line, sizeof(line), fp)) 
    {
        char *eq_pos = strchr(line, '=');
        if (!eq_pos) 
            continue;

        *eq_pos = '\0'; // 分割键值
        char *current_key = line;
        strstripspace(current_key);

        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        if (!tmp) 
            goto err_out;
        res = tmp;
        res[counts] = strdup(current_key);
        if (!res[counts]) 
            goto err_out;
        counts++;
    }
    // 添加 NULL 结尾
    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    if (!tmp) 
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    fclose(fp);
    return res;

err_out:
    fclose(fp);
    if (res) {
        for (int i = 0; i < counts; i++)
        {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_dpkg_list_files(const char *pkg_name)
{
    if (!pkg_name) {
        return NULL;
    }

    char **res = NULL;
    int counts = 0;
    int pipe_fd[2];
    pid_t pid;
    int status;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        klog_err("pipe\n");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        klog_err("fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            klog_err("dup2\n");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null（可选，根据需求）
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数
        char *args[] = {"dpkg", "-L", (char *)pkg_name, NULL};
        
        // 执行 dpkg 命令（完全绕过 shell）
        execvp("dpkg", args);
        
        // 如果 execvp 失败
        klog_err("execvp\n");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, &status, 0);
            return NULL;
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            
            // 跳过空行
            if (line[0] == '\0') {
                continue;
            }

            // 分配内存
            char **tmp = realloc(res, (counts + 2) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            
            // 复制字符串
            res[counts] = strdup(line);
            if (!res[counts]) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            counts++;
        }
        
        // 添加 NULL 结尾
        if (res) {
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            res[counts] = NULL;
        }
        
        fclose(fp);
        waitpid(pid, &status, 0);
        
        // 检查命令执行是否成功
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // dpkg 命令执行失败，清理资源
            goto cleanup;
        }
        
        return res;
    }

cleanup:
    if (res) {
        for (int i = 0; i < counts; i++) {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_kare_list_files(const char *pkg_name)
{
    if (!pkg_name) {
        return NULL;
    }

    char **res = NULL;
    int counts = 0;
    int pipe_fd[2];
    pid_t pid;
    int status;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        klog_err("pipe\n");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        klog_err("fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            klog_err("dup2\n");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数
        char *args[] = {"kare", "-L", (char *)pkg_name, NULL};
        
        // 执行 kare 命令（完全绕过 shell）
        execvp("kare", args);
        
        // 如果 execvp 失败
        klog_err("execvp\n");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, &status, 0);
            return NULL;
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            
            // 跳过空行
            if (line[0] == '\0') {
                continue;
            }

            // 分配内存
            char **tmp = realloc(res, (counts + 2) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            
            // 复制字符串
            res[counts] = strdup(line);
            if (!res[counts]) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            counts++;
        }
        
        // 添加 NULL 结尾
        if (counts > 0) {
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            res[counts] = NULL;
        }
        
        fclose(fp);
        waitpid(pid, &status, 0);
        
        // 检查命令执行是否成功
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // kare 命令执行失败，清理资源
            goto cleanup;
        }
        
        return res;
    }

cleanup:
    if (res) {
        for (int i = 0; i < counts; i++) {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

static char** kdk_package_kaiming_list_files(const char *pkg_name)
{
    if (!pkg_name) {
        return NULL;
    }

    char **res = NULL;
    int counts = 0;
    int pipe_fd[2];
    pid_t pid;
    int status;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数 - 使用绝对路径和明确参数
        char *args[] = {
            "/opt/kaiming-tools/bin/kaiming", 
            "info", 
            "-L", 
            (char *)pkg_name, 
            NULL
        };
        
        // 执行 kaiming 命令（完全绕过 shell）
        execv(args[0], args);
        
        // 如果 execv 失败
        perror("execv");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, &status, 0);
            return NULL;
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            
            // 跳过空行
            if (line[0] == '\0') {
                continue;
            }

            // 分配内存
            char **tmp = realloc(res, (counts + 2) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            
            // 复制字符串
            res[counts] = strdup(line);
            if (!res[counts]) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            counts++;
        }
        
        // 添加 NULL 结尾
        if (counts > 0) {
            char **tmp = realloc(res, (counts + 1) * sizeof(char *));
            if (!tmp) {
                fclose(fp);
                waitpid(pid, &status, 0);
                goto cleanup;
            }
            res = tmp;
            res[counts] = NULL;
        }
        
        fclose(fp);
        waitpid(pid, &status, 0);
        
        // 检查命令执行是否成功
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            // kaiming 命令执行失败，清理资源
            goto cleanup;
        }
        
        return res;
    }

cleanup:
    if (res) {
        for (int i = 0; i < counts; i++) {
            free(res[i]);
        }
        free(res);
    }
    return NULL;
}

char **kdk_package_list_files(const char *pkg_name)
{
    if(!pkg_name)
        return NULL;
    char **res = NULL;

    InstallMethod ret = kdk_package_get_installation_method(pkg_name, NULL);
    switch (ret)
    {
    case DPKG_METHOD:
    case KWRE_METHOD:
    case KMRE_METHOD:
        res = kdk_package_dpkg_list_files(pkg_name);
        break;
    case KM_METHOD:
        res = kdk_package_kaiming_list_files(pkg_name);
        break;
    case KARE_METHOD:
        res = kdk_package_kare_list_files(pkg_name);
        break;
    default:
        break;
    }
    
   return res;
}

static char* kdk_package_dpkg_search_by_file(const char *file_name)
{
    if (!file_name) {
        return NULL;
    }

    char *res = NULL;
    int pipe_fd[2];
    pid_t pid;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        klog_err("pipe\n");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        klog_err("fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null（相当于 2>/dev/null）
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数
        char *args[] = {"dpkg", "-S", (char *)file_name, NULL};
        
        // 执行 dpkg 命令
        execvp("dpkg", args);
        
        // 如果 execvp 失败
        klog_err("execvp");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return NULL;
        }

        char line[1024] = "\0";
        if (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            if (line[0] != '\0') {
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "没有找到与 %s 相匹配的路径", file_name);
                
                if (strstr(line, errinfo)) {
                    fclose(fp);
                    waitpid(pid, NULL, 0);
                    return NULL;
                }

                char *eq_pos = strchr(line, ':');
                if (eq_pos) {
                    *eq_pos = '\0';
                    res = strdup(line);
                }
            }
        }
        
        fclose(fp);
        waitpid(pid, NULL, 0);
    }

    return res;
}

static char* kdk_package_kare_search_by_file(const char *file_name)
{
    if (!file_name) {
        return NULL;
    }

    char *res = NULL;
    int pipe_fd[2];
    pid_t pid;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        klog_err("pipe");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        klog_err("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null（相当于 2>/dev/null）
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数（安全地执行命令）
        char *args[] = {"kare", "-S", (char *)file_name, NULL};
        
        // 执行 kare 命令（完全绕过 shell）
        execvp("kare", args);
        
        // 如果 execvp 失败
        klog_err("execvp");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return NULL;
        }

        char line[1024] = "\0";
        if (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            if (line[0] != '\0') {
                // 检查错误信息（中文错误信息处理）
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "没有找到与 *%s* 相匹配的路径", file_name);
                
                if (strstr(line, errinfo)) {
                    fclose(fp);
                    waitpid(pid, NULL, 0);
                    return NULL;
                }
                
                char *eq_pos = strchr(line, ':');
                if (eq_pos) {
                    *eq_pos = '\0';
                    res = strdup(line);
                }
            }
        }
        
        fclose(fp);
        waitpid(pid, NULL, 0);
    }

    return res;
}

static char* kdk_package_kaiming_search_by_file(const char *file_name)
{
    if (!file_name) {
        return NULL;
    }

    char *res = NULL;
    int pipe_fd[2];
    pid_t pid;

    // 创建管道用于读取子进程输出
    if (pipe(pipe_fd) == -1) {
        klog_err("pipe\n");
        return NULL;
    }

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        klog_err("fork\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return NULL;
    }

    if (pid == 0) { // 子进程
        // 关闭管道的读端
        close(pipe_fd[0]);
        
        // 将标准输出重定向到管道的写端
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        // 重定向标准错误到 /dev/null（相当于 2>/dev/null）
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        // 准备参数 - 注意命令路径和参数要分开
        char *args[] = {
            "/opt/kaiming-tools/bin/kaiming", 
            "which", 
            (char *)file_name, 
            NULL
        };
        
        // 执行 kaiming 命令（完全绕过 shell）
        execv(args[0], args);
        
        // 如果 execv 失败
        klog_err("execv\n");
        exit(EXIT_FAILURE);
    } else { // 父进程
        // 关闭管道的写端
        close(pipe_fd[1]);
        
        // 读取子进程输出
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return NULL;
        }

        char line[1024] = "\0";
        if (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            if (line[0] != '\0') {
                // 检查错误信息
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "No package contains the file: %s", file_name);
                
                if (strstr(line, errinfo)) {
                    fclose(fp);
                    waitpid(pid, NULL, 0);
                    return NULL;
                }
                
                char *eq_pos = strchr(line, ':');
                if (eq_pos) {
                    *eq_pos = '\0';
                    res = strdup(line);
                }
            }
        }
        
        fclose(fp);
        waitpid(pid, NULL, 0);
    }

    return res;
}

// 路径替换规则列表
static const kdk_path_remap_rule remap_rules[] = {
    {"/var/opt", "/opt"},
    {"/var/mnt", "/mnt"},
    {"/var/srv", "/srv"},
    {"/var/tmp", "/tmp"},
    {"/run/media", "/media"}
    // 可添加更多规则
};

// 替换路径前缀：in_path 是输入路径，out_path 是输出路径缓冲区，必须至少 MAX_PATH_LEN 大
// 返回值：0 成功，-1 无需替换或错误
static int remap_path_prefix(const char *in_path, char *out_path, size_t out_size)
{
    if (!in_path || !out_path || out_size == 0)
        return -1;

    size_t rule_count = sizeof(remap_rules) / sizeof(remap_rules[0]);

    for (size_t i = 0; i < rule_count; ++i) {
        const char *real_path = remap_rules[i].kdk_real_path;
        const char *path = remap_rules[i].kdk_path;
        size_t len = strnlen(real_path, STRNLEN_MAX);

        // 匹配前缀，并保证 "/" 或结束符之后不误匹配
        if (strncmp(in_path, real_path, len) == 0 &&
            (in_path[len] == '/' || in_path[len] == '\0')) {

            if (snprintf(out_path, out_size, "%s%s", path, in_path + len) >= out_size)
                return -1;  // buffer too small

            return 0; // 替换成功
        }
    }

    // 没有任何替换规则匹配
    strncpy(out_path, in_path, out_size - 1);
    out_path[out_size - 1] = '\0';
    return -1;
}

char *kdk_package_search_by_file(const char *file_name)
{
    if(!file_name)
        return NULL;
    char *res = NULL;
    char new_path[MAX_PATH_LEN] = "\0";
    remap_path_prefix(file_name, new_path, sizeof(new_path));
    klog_info("new_path = %s\n", new_path);

    res = kdk_package_dpkg_search_by_file(new_path);
    if(res)
    {
        return res;
    }

    res = kdk_package_kare_search_by_file(new_path);
    if(res)
    {
        return res;
    }

    res = kdk_package_kaiming_search_by_file(new_path);
    if(res)
    {
        return res;
    }
    
    return NULL;
}

void kdk_package_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}

static bool kaiming_online_installable_cmd(const char *name, const char *version)
{
    int pipe_fd[2];
    pid_t pid;
    bool installable = false;

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


        char package_spec[512];
        snprintf(package_spec, sizeof(package_spec), "%s=%s", name, version);
        
        char *args[] = {
            "kaiming", "search", name, NULL
        };
        execvp("kaiming", args); 
        
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return false;
        }

        char line[1024];
        char kaiming_name[256] = "\0";
        char kaiming_version[256] = "\0";
        int flag = 0;
        while (fgets(line, sizeof(line), fp)) {

            sscanf(line,"%s %*s %s", kaiming_name, kaiming_version);

            syslog(LOG_INFO, "%s, kaiming_name = %s, kaiming_version = %s\n", __func__, kaiming_name, kaiming_version);
            
            if (kaiming_name[0] != '\0' && strcmp(kaiming_name, name) == 0) {
                flag = 1;
            }

            if(version)
            {
                if(flag && kaiming_version[0] != '\0')
                {
                    if(strcmp(kaiming_version, version) == 0)
                    {
                        installable = true;
                        break;
                    }
                }
            }
            else
            {
                if(flag)
                {
                    installable = true;
                    break;
                }
            }
        }
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        // 检查命令执行状态
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            installable = false;
        }
        
        return installable;
    }
    return installable;
}

static size_t curl_output(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    return nmemb;
}

static bool url_accessible(const char *url)
{
    bool ret_access = test_connectivity(url);
    return ret_access;

}

static char* get_val_from_file(FILE *fp, const char *key)
{
    if (! fp)
        return NULL;
    char *val = NULL;
    char buf[2048] = {0};
    while (fgets(buf, 2048, fp))
    {
        if (strncmp(buf, key, strnlen(key, STRNLEN_MAX)) == 0)
        {
            val = strdup((char*)buf + strnlen(key, STRNLEN_MAX) +1);
            break;
        }
    }

    return val;
}

static bool kaiming_repo_accessible()
{
    FILE *fp;
    int len;
    static char *repo_url = NULL;
    char version[32] = "\0";

    if ((fp = fopen(KAIMING_REPO_CONFIG_FILE, "r")) == NULL)
    {
        syslog(LOG_ERR, "/opt/kaiming/repo/config does not exist or is not accessible!\n");
        return false;
    }

    repo_url = get_val_from_file(fp, "url");
    if (!repo_url)
    {
        fclose(fp);
        return false;
    }
    strstrip(repo_url, '\"');
    strstripspace(repo_url);
    fclose(fp);

    if(url_accessible(repo_url))
        return true;
    else
        return false;
}

static bool kdk_package_is_kaiming_online_installable(const char *name, const char *version)
{
    DBusConnection *conn;
    DBusError err;
    DBusMessage *branch_msg = NULL;
    DBusPendingCall *sendMsgPendingBranch = NULL;
    DBusMessage *replyMsgBranch = NULL;

    cJSON *branch_version = NULL;
    cJSON *json = NULL;

    bool installable = false;
    
    // 检查kaiming repo是否可访问,repo的url在/opt/kaiming/repo/config文件中查询
    if(!kaiming_repo_accessible())
    {
        syslog(LOG_INFO, "[%s] kaiming repo cannot be accessed", __func__);
        return false;
    }

    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        syslog(LOG_INFO, "kaiming\n");
        dbus_error_init(&err);
        conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

        if (dbus_error_is_set(&err))
        {
            syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
            dbus_error_free(&err);
        }

        if (NULL == conn)
        {
            installable = false;
            goto out;
        }        

        branch_msg = dbus_message_new_method_call("org.kylin.kaiming",   // target for the method call
                                                "/org/kylin/kaiming",     // object to call on
                                                "org.kylin.kaiming.interface", // interface to call on
                                                "getRemoteInfo");           // method name

        if (!branch_msg)
        { // -1 is default timeout
            syslog(LOG_ERR, "status_msg: dbus_message_new_method_call调用失败\n");
            installable = false;
            goto out;
        }
        
        if (!dbus_message_append_args(branch_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
        {
            syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
            installable = false;
            goto out;
        }
           

        if (!dbus_connection_send_with_reply(conn, branch_msg, &sendMsgPendingBranch, -1))
        {
            syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
            installable = false;
            goto out;
        }

        if (sendMsgPendingBranch == NULL)
        {
            syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
            installable = false;
            goto out;
        }

        dbus_connection_flush(conn);

        if (branch_msg)
        {
            dbus_message_unref(branch_msg);
        }

        dbus_pending_call_block(sendMsgPendingBranch);

        replyMsgBranch = dbus_pending_call_steal_reply(sendMsgPendingBranch);

        if (replyMsgBranch == NULL)
        {
            syslog(LOG_ERR, "kdk : get reply message fail !\n");
            installable = false;
            goto out;
        }

        if (sendMsgPendingBranch)
        {
            dbus_pending_call_unref(sendMsgPendingBranch);
        }

        if (dbus_message_get_type(replyMsgBranch) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsgBranch));
            dbus_message_unref(replyMsgBranch);
            installable = false;
            goto out;
        }

        DBusMessageIter args_branch;
        char *branch_res;
                
        if (!dbus_message_iter_init(replyMsgBranch, &args_branch))
        {
            dbus_message_unref(replyMsgBranch);
            syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
            installable = false;
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args_branch, &branch_res);
        }

        if (replyMsgBranch)
        {
            dbus_message_unref(replyMsgBranch);
        }
        // dbus_connection_close(conn);

        syslog(LOG_INFO, "kaiming online branch : %s\n", branch_res);

        json = cJSON_Parse(branch_res);
        if (json == NULL) {
            installable = false;
            goto out;
        }

        branch_version = cJSON_GetObjectItem(json, "version");
        if (branch_version == NULL || !cJSON_IsString(branch_version)) {
            cJSON_Delete(json);
            installable = false;
            goto out;
        }

        syslog(LOG_INFO, "%s, strlen branch_version->valuestring = %d\n", __func__,  strnlen(branch_version->valuestring, STRNLEN_MAX));

        if(version && strnlen(version, STRNLEN_MAX)) {
            if(strcmp(branch_version->valuestring, version) == 0)
            {
                installable = true;
            }
        }
        else {
            if(strnlen(branch_version->valuestring, STRNLEN_MAX) != 0)
            {
                installable = true;
            }
        }
    }

    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

out:
    if(!installable) {
        installable = kaiming_online_installable_cmd(name, version);
    }
    return installable;
}

static int extract_deb_uri(const char* input, char* output, size_t out_len) {
    if (!input || !output || out_len == 0)
        return -1;

    const char *start = strchr(input, '\'');
    if (!start)
        return -1; // 没有找到第一个单引号

    const char *end = strchr(start + 1, '\'');
    if (!end)
        return -1; // 没有找到第二个单引号

    size_t len = end - start - 1;
    if (len >= out_len)
        return -1; // 缓冲区太小

    strncpy(output, start + 1, len);
    output[len] = '\0';
    return 0;
}

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strnlen(s, STRNLEN_MAX) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
        *end-- = '\0';
    return s;
}

static bool kdk_package_is_deb_online_installable(const char *name, const char *version)
{
    int pipe_fd[2];
    pid_t pid;
    bool installable = false;

#ifdef __loongarch64

    char cmd[512] = "";
    snprintf(cmd, sizeof(cmd), "dpkg --add-architecture loongarch64 && apt.orig update");
    syslog(LOG_DEBUG, "cmd = %s\n", cmd);
    system(cmd);
#endif

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

        char *args[] = {
            "apt-cache", "policy", (char *)name, NULL
        };
        execvp("apt-cache", args);
        
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return false;
        }

        char line[1024] = {0};
        bool in_version_list = false;  // 标记是否在版本列表区域
        char *colon = NULL;
        char candidate_version[256] = {0};
        while (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            
            char *p = line;

            // 去除前导空格
            p = trim(p);
           
            if (strstr(p, "候选")) {
                colon = strchr(p, '：');
                if (colon) {
                    char *ver_start = colon + 1;
                    strstripspace(ver_start);
                    strcpy(candidate_version, ver_start);
                    syslog(LOG_INFO, "[%s] Found candidate version: %s\n", __func__, candidate_version);
                }
                continue;
            }

            // *** 开头，跳过星号
            if (strncmp(p, "***", 3) == 0)
                p = trim(p + 3);

            // 提取版本号（取第一个空格前的部分）
            char ver[256] = {0};
            sscanf(p, "%255s", ver);

            // 防止错误行：版本号必须包含数字
            if (!strpbrk(ver, "0123456789"))
                continue;

            syslog(LOG_INFO, "[%s] version = %s, ver = %s, candidate_version = %s\n", __func__, version, ver, candidate_version);
            if(version) {
                if(strcmp(version, ver) == 0) {
                    // 需要获取这个版本对应的URL
                    // 继续读取下一行获取URL信息
                    if (fgets(line, sizeof(line), fp)) {
                        strstripspace(line);
                        // URL行通常以数字开头，后面跟着URL
                        char url_temp[1024] = {0};
                        int priority;

                        if (sscanf(line, "%d %1023s", &priority, url_temp) == 2) {
                            if ((strncmp(url_temp, "http://", 7) == 0 || strncmp(url_temp, "https://", 8) == 0)) {
                                syslog(LOG_INFO, "[%s] found URL: %s\n", __func__, url_temp);
                                bool ret_access = test_connectivity(url_temp);
                                if(!ret_access)
                                {
                                    syslog(LOG_DEBUG, "[%s] Weak network environment detected, please check your network connectivity.", __func__);
                                    installable = false;
                                    break;
                                } else {
                                    installable = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else{
                if(candidate_version[0] != '\0') {
                    if(strcmp(candidate_version, ver) == 0) {
                        // 需要获取这个版本对应的URL
                        // 继续读取下一行获取URL信息
                        if (fgets(line, sizeof(line), fp)) {
                            strstripspace(line);
                            // URL行通常以数字开头，后面跟着URL
                            char url_temp[1024] = {0};
                            int priority;

                            if (sscanf(line, "%d %1023s", &priority, url_temp) == 2) {
                                if ((strncmp(url_temp, "http://", 7) == 0 || strncmp(url_temp, "https://", 8) == 0)) {
                                    syslog(LOG_INFO, "[%s] found URL: %s\n", __func__, url_temp);
                                    bool ret_access = test_connectivity(url_temp);
                                    if(!ret_access)
                                    {
                                        installable = false;
                                        syslog(LOG_DEBUG, "[%s] Weak network environment detected, please check your network connectivity.", __func__);
                                        break;
                                    } else {
                                        installable = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);

        // 检查命令执行状态
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            installable = false;
        }
    }

#ifdef __loongarch64
    snprintf(cmd, sizeof(cmd), "dpkg --remove-architecture loongarch64 && apt.orig update");
    syslog(LOG_DEBUG, "cmd = %s\n", cmd);
    system(cmd);
#endif
    return installable;
}

bool kdk_package_is_online_installable(const char *name, const char *version)
{
    if (!name) {
        return false;
    }

    bool ret = kdk_package_is_kaiming_online_installable(name, version);

    if(!ret)
    {
        ret = kdk_package_is_deb_online_installable(name, version);
    }

    return ret;
}

static int kdk_kare_tag_install_online(const char *pkg_name, const char *version, const char *type, const char *compat, InstallCallBack call_back)
{
    syslog(LOG_DEBUG, "%s", __func__);

    char cmd[1024]= "\0";
    char line[1024] = "\0";
    FILE *fp = NULL;
    char deb_path[1024] = "\0";
    char pkg_name_str[256] = "\0";
    char filename[512] = "\0";
    char fileNamePath[512] = "\0";

    const char *dir = "/tmp/kysdk-package-download";
    if (mkdir(dir, 0755) != 0) {
        if (errno != EEXIST) {
            syslog(LOG_ERR, "Failed to create directory: %s, error: %s\n", dir, strerror(errno));
            return false;
        }
        // 如果是EEXIST错误（目录已存在），则继续执行下面的chdir操作
    }

    char cwd[1024];  // 存储当前目录路径
    if (getcwd(cwd, sizeof(cwd)) != NULL) 
    {
        syslog(LOG_DEBUG, "Current directory: %s\n", cwd);

        if (chdir(dir) == 0) 
        {
            syslog(LOG_DEBUG, "Switched to directory: %s\n", dir);
            memset(cmd, 0, 1024);

#ifdef __loongarch64
            const char* sources_name = "/etc/apt/sources.list";
            char* http_url = get_first_active_repo_url(sources_name);
            if (http_url) {
                syslog(LOG_DEBUG, "[%s] http url = %s\n", __func__, http_url);
                bool ret_access = test_connectivity(http_url);
                free(http_url);
                if(!ret_access)
                {
                    _callbackinstall(0, -1, "弱网环境，请检查网络连通性");
                    return 0;
                }
            } else {
                syslog(LOG_DEBUG, "[%s] No uncommented sources found or file read failed", __func__);
                return 0;
            }
            snprintf(cmd, sizeof(cmd), "dpkg --add-architecture loongarch64 && apt.orig update");
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);

            if(version)
            {
                snprintf(cmd, sizeof(cmd), "apt.orig download %s=%s", pkg_name, version);
                syslog(LOG_DEBUG, "cmd = %s\n", cmd);
                system(cmd);
            }
            else
            {
                snprintf(cmd, sizeof(cmd), "apt.orig download %s", pkg_name);
                syslog(LOG_DEBUG, "cmd = %s\n", cmd);
                system(cmd);
            }

            snprintf(cmd, sizeof(cmd), "dpkg --remove-architecture loongarch64 && apt.orig update");
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);

#else
            if(version)
            {
                snprintf(cmd, sizeof(cmd), "apt.orig download %s=%s", pkg_name, version);
                syslog(LOG_DEBUG, "cmd = %s\n", cmd);
                system(cmd);
            }
            else
            {
                snprintf(cmd, sizeof(cmd), "apt.orig download %s", pkg_name);
                syslog(LOG_DEBUG, "cmd = %s\n", cmd);
                system(cmd);
            }
#endif
            if (chdir(cwd) != 0) 
            {
                syslog(LOG_ERR, "Failed to change directory");
                return 0;
            }
        } 
        else 
        {
            syslog(LOG_ERR, "Failed to change directory");
            return 0;
        }
    } else {
        syslog(LOG_ERR, "getcwd failed");
        return 0;
    }

    DIR *dp;
    struct dirent *entry;
    dp = opendir(dir);
    if (dp == NULL) 
    {
        return 0;
    }
    while ((entry = readdir(dp)) != NULL) 
    {
    // 跳过当前目录和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }

        if(strstr(entry->d_name, pkg_name))
        {
            strncpy(filename, entry->d_name, sizeof(filename) - 1);
        }
    }
    closedir(dp);

    memset(deb_path, 0, 1024);
    snprintf(deb_path, sizeof(deb_path), "%s/%s", dir, filename);
    syslog(LOG_DEBUG, "deb_path: %s\n", deb_path);

    char rm_dir[256] = "\n";
    snprintf(rm_dir, sizeof(rm_dir), "rm %s", deb_path);

    if (filename[0] == '\0')
    {
        _callbackinstall(0, -1, "软件包不存在");
        return 0;
    }
    
    int mark = 0;
    int flag = 0;
    char key[128] = "\0";
    char value[128] = "\0";
    char typetag[128] = "\0";
    char *runtime = NULL;
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    _callbackinstall = call_back;

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "dkms kare\n");

        if(compat)
        {
            strncpy(value, compat, sizeof(value) - 1);
        }
        else
        {
            if (!extract_compat_file(dir, deb_path, "runtime_version", value))
            {

                return 0;
            }
        }

        if(type)
        {
            strncpy(typetag, type, sizeof(typetag) - 1);
        }
        else
        {
            if (!extract_compat_file(dir, deb_path, "sub_system", typetag))
            {

                return 0;
            }
        }

        if(typetag[0] != '\0')
        {

            char name[256] = "\0";
            if(version)
            {
                snprintf(name, sizeof(name), "%s=%s", pkg_name, version);
            }
            else
            {
                strncpy(name, pkg_name, sizeof(name));
            }
            
            if(strncmp(typetag, "inputMethod", 11) == 0  || strncmp(typetag, "maintainMode", 12) == 0)
            {
                // int ret = kdk_debian_install_online(name, call_back);
                return 0;
            }
        }

        if(value[0] != '\0')
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
                    
                    if (flag != 1 && order == p->order)
                    {
                        if (kdk_kare_build_base_env(p->value, call_back) == 1)
                        {
                            strncpy(value, p->value, sizeof(value) - 1);
                            flag = 1;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
            if (NULL == setting)
            {
                syslog(LOG_ERR, "get handle failed\n");
                system(rm_dir);
                return 0;
            }

            runtime = kdk_conf2_get_string(setting, "kare-runtime");
            if (NULL != runtime)
            {
                syslog(LOG_INFO, "kare-runtime value: %s\n", runtime);
                kdk_conf2_ksettings_destroy(setting);
            }

            if(runtime)
            {
                if (check_kare_environment(runtime, value, sizeof(value)))
                {
                    syslog(LOG_INFO, "runtime = %s, value = %s\n", runtime, value);
                    flag = 1;
                }
                else
                {
                    int base = kdk_kare_build_base_env(runtime, call_back);
                    if(base == 1)
                    {
                        strncpy(value, runtime, sizeof(value) - 1);
                        flag = 1;
                    }
                }
                free(runtime);
            }
        }

        syslog(LOG_INFO, "flag = %d, value = %s, typetag = %s\n", flag, value, typetag);
        
        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            //创建用户
            info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                    "/org/kylin/kare", // object to call on
                                                    "org.kylin.kare",  // interface to call on
                                                    "install_with_subsystem");               // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            char *env = strdup(value);
			char *uri = strdup(deb_path);
            char *subsystem = NULL;
            
            if(typetag[0] != '\0')
            {
                subsystem = strdup(typetag);
            }
            else
            {
                subsystem = strdup("");
            }
            
            syslog(LOG_INFO, "kdk : d-bus env : %s, subsystem : %s\n", env, subsystem);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &subsystem, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                free(env);
                free(uri);
                free(subsystem);
                goto err_out;
            }
            free(env);
            free(uri);
            free(subsystem);

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kare (%s)", dbus_message_get_error_name(replyMsg));
                _callbackinstall(0, -1, errinfo);
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

            if(ret)
            {
                mark = 1;
            }
            else
            {
                if(access(KDK_KARE_INSTALL_LOCK, F_OK) == 0)
                {
                    _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
                }
            }
        }
    }

    if(mark)
    {
        mark = 0;
        running_install = 1;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, value);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
            syslog(LOG_INFO, "%s mark = %d\n", __func__, mark);
        }
    }
    system(rm_dir);
    syslog(LOG_INFO, "%s ret mark = %d\n", __func__, mark);
    return mark;
err_out:
    system(rm_dir);
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);

    return 0;
}


int kdk_package_install_tag_package_online(const char *pkg_name, const char *version, const char *type, const char *compat, InstallCallBack call_back)
{
    if(!pkg_name)
        return 0;

    int ret = 0;
    syslog(LOG_ERR, "%s online\n", __func__);
    char name[256] = "\0";
    if(version)
    {
        snprintf(name, sizeof(name), "%s=%s", pkg_name, version);
    }
    else
    {
        strncpy(name, pkg_name, sizeof(name));
    }

    ret = kdk_kaiming_install_online(name, call_back);
    
    if(!ret)
    {
        bool is_ps = kdk_is_panshi_feature();
        if(!is_ps)
        {
            syslog(LOG_ERR, "%s, is_ps = %d\n", __func__, is_ps);
            // ret = kdk_debian_install_online(name, call_back);
            return ret;
        }

        int flag = 0;
        int app = 0;
        char file_name[512] = "\0";

        KSettings *setting = kdk_conf2_new("libkysdk-package.direct_install_whitelist", NULL);
        if (NULL == setting)
        {
            syslog(LOG_ERR, "get handle failed\n");
            return 0;
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
                        syslog(LOG_INFO, "[%s] List value : %s, file_name : %s", __func__, value[j], pkg_name);
                        if (contains_wildcard(value[j]))
                        {
                            bool match = wildcard_match(value[j], pkg_name);
                            syslog(LOG_INFO, "[%s] List value : %s, file_name : %s, match = %d\n", __func__, value[j], pkg_name, match);
                            if(match)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        else
                        {
                            if(strcmp(value[j], pkg_name) == 0)
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

        syslog(LOG_ERR, "%s, flag = %d, name = %s\n", __func__, flag, name);

        if(flag)
        {
            // ret = kdk_debian_install_online(name, call_back);
            return ret;
        }
        else
            ret = kdk_kare_tag_install_online(pkg_name, version, type, compat, call_back);
    }
    return ret;
}

static int kdk_kare_tag_install_offline(const char *uri, const char *type, const char *compat, InstallCallBack call_back)
{
    syslog(LOG_DEBUG, "dkms kare offline");

    if(!uri)
    {
        syslog(LOG_ERR, "%s: NULL uri parameter\n", __func__);
        return 0;
    }

    int mark = 0;
    int flag = 0;
    char cmd[1024]= "\0";

    char line[1024] = "\0";
    char key[128] = "\0";
    char value[128] = "\0";
    char *runtime = NULL;
    char typetag[128] = "\0";
    int env = 0;
    int order = 0;
    DBusConnection *conn;
    DBusError err;
    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    _callbackinstall = call_back;

    const char *dir = "/tmp/kysdk-package-download";
    if (access(dir, F_OK) != 0) {
        // 创建目录，包括父目录
        char mkdir_cmd[512];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", dir);
        int result = system(mkdir_cmd);
        if (result != 0) {
            syslog(LOG_ERR, "Failed to create directory: %s\n", dir);
            return false;
        }
    }

    if (0 == access("/usr/bin/kare", F_OK))
    {
        syslog(LOG_ERR, "dkms kare\n");

        if(compat)
        {
            strncpy(value, compat, sizeof(value) - 1);
        }
        else
        {
            if (!extract_compat_file(dir, uri, "runtime_version", value))
            {

                return 0;
            }
        }

        if(type)
        {
            strncpy(typetag, type, sizeof(typetag) - 1);
        }
        else
        {
            if (!extract_compat_file(dir, uri, "sub_system", typetag))
            {

                return 0;
            }
        }

        if(typetag[0] != '\0')
        {
            if(strncmp(typetag, "inputMethod", 11) == 0 || strncmp(typetag, "maintainMode", 12) == 0)
            {
                // int ret = kdk_debian_install_offline(uri, call_back);
                return 0;
            }
        }

        if(value[0] != '\0')
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
                    
                    if (flag != 1 && order == p->order)
                    {
                        if (kdk_kare_build_base_env(p->value, call_back) == 1)
                        {
                            strncpy(value, p->value, sizeof(value) - 1);
                            flag = 1;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
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

            if(runtime)
            {
                if (check_kare_environment(runtime, value, sizeof(value)))
                {
                    syslog(LOG_INFO, "runtime = %s, value = %s\n", runtime, value);
                    flag = 1;
                }
                else
                {
                    int base = kdk_kare_build_base_env(runtime, call_back);
                    if(base == 1)
                    {
                        strncpy(value, runtime, sizeof(value) - 1);
                        flag = 1;
                    }
                }
                free(runtime);
            }
        }

        syslog(LOG_INFO, "flag = %d, value = %s, typetag = %s\n", flag, value, typetag);

        if(flag)
        {
            dbus_error_init(&err);
            conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
            if (NULL == conn)
            {
                syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
                goto err_out;
            }

            if (dbus_error_is_set(&err))
            {
                // syslog(LOG_ERR, stderr, "Connection Error (%s)\n", err.message);
                dbus_error_free(&err);
            }

            //创建用户
            info_msg = dbus_message_new_method_call("org.kylin.kare",  // target for the method call
                                                    "/org/kylin/kare", // object to call on
                                                    "org.kylin.kare",  // interface to call on
                                                    "install_with_subsystem");               // method name
            if (!info_msg)
            { // -1 is default timeout
                syslog(LOG_ERR, "status_msg：dbus_message_new_method_call调用失败\n");
                goto err_out;
            }

            char *env = strdup(value);
            char *subsystem = NULL;
            
            if(typetag[0] != '\0')
            {
                subsystem = strdup(typetag);
            }
            else
            {
                subsystem = strdup("");
            }
 
            syslog(LOG_INFO, "kdk : d-bus env : %s, subsystem : %s\n", env, subsystem);
            if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &env, DBUS_TYPE_STRING, &uri, DBUS_TYPE_STRING, &subsystem, DBUS_TYPE_INVALID))
            {
                syslog(LOG_ERR, "kdk : d-bus append args fail !\n");
                free(env);
                free(subsystem);
                goto err_out;
            }
            free(env);
            free(subsystem);

            if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
            {
                syslog(LOG_ERR, "kdk : d-bus send message fail !\n");
                goto err_out;
            }

            if (sendMsgPending == NULL)
            {
                syslog(LOG_ERR, "kdk : d-bus pending message is NULL !\n");
                goto err_out;
            }

            dbus_connection_flush(conn);

            dbus_pending_call_block(sendMsgPending);
            replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
            if (replyMsg == NULL)
            {
                syslog(LOG_ERR, "kdk : get reply message fail !\n");
                goto err_out;
            }


            if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
            {
                syslog(LOG_ERR, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
                char errinfo[256] = "\0";
                snprintf(errinfo, sizeof(errinfo), "DBus service unavailable: org.kylin.kare (%s)", dbus_message_get_error_name(replyMsg));
                _callbackinstall(0, -1, errinfo);
                goto err_out;
            }

            DBusMessageIter args;
            bool *ret = false;

            if (!dbus_message_iter_init(replyMsg, &args))
            {
                syslog(LOG_ERR, "kdk : d-bus reply message fail !\n");
                goto err_out;
            }
            else
            {
                dbus_message_iter_get_basic(&args, &ret);
            }

            dbus_message_unref(info_msg);
            dbus_message_unref(replyMsg);
            dbus_pending_call_unref(sendMsgPending);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            dbus_error_free(&err);

            if(ret)
            {
                mark = 1;
            }
            else
            {
                if(access(KDK_KARE_INSTALL_LOCK, F_OK) == 0)
                {
                    _callbackinstall(0, -1, "安装、卸载过程中，请勿重复执行");
                }
            }

        }
    }
        
    if(mark)
    {
        mark = 0;
        running_install = 1;
        pthread_create(&thread_install, NULL, dbus_kare_install_listener_thread, value);

        void *thread_result;
        pthread_join(thread_install, &thread_result);

        if(thread_result)
        {
            mark = (int)(intptr_t)thread_result;
        }
    }

    return mark;
err_out:
    if (info_msg) {
        dbus_message_unref(info_msg);
    }
    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }
    if (conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
    dbus_error_free(&err);
    return 0;
}

int kdk_package_install_tag_package_offline(const char *uri, const char *type, const char *compat, InstallCallBack call_back)
{
    if (0 != access(uri, F_OK))
    {
        return 0;
    }
    syslog(LOG_INFO, "%s offline\n", __func__);
    int ret = 0;
    char filename[512] = "\0";  
    strncpy(filename, uri, 511); 
    const char *dot = strrchr(filename, '.');
    // 如果没有找到点，或者点是文件名的第一个字符，则返回 false
    if (dot == NULL || dot == filename) {
        return 0;
    }
    // 检查从点开始的子字符串是否与后缀匹配
    if(strcmp(dot, ".ok") == 0)
    {
        ret = kdk_kaiming_install_offline(uri, call_back);
    }
    else if(strcmp(dot, ".deb") == 0)
    {
        bool is_ps = kdk_is_panshi_feature();
        if(!is_ps)
        {
            syslog(LOG_ERR, "%s, is_ps = %d\n", __func__, is_ps);
            // ret = kdk_debian_install_offline(uri, call_back);
            return ret;
        }
        
        int flag = 0;
        int app = 0;
        memset(filename, 0, 512);
        strncpy(filename, uri, 511);

        char cmd[1024] = "\0";
        char file_name[512] = "\0";
        snprintf(cmd, sizeof(cmd), "dpkg-deb -f '%s' Package", filename);
        syslog(LOG_ERR, "cmd = %s\n", cmd);

        FILE *fd = popen(cmd, "r");
        if(!fd)
            return 0;
        
        fgets(file_name, sizeof(file_name), fd);
        strstripspace(file_name);
        syslog(LOG_ERR, "file_name = %s\n", file_name);
        pclose(fd);

        KSettings *setting = kdk_conf2_new("libkysdk-package.direct_install_whitelist", NULL);
        if (NULL == setting)
        {
            syslog(LOG_ERR, "get handle failed\n");
            return 0;
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
                            if(match)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        else
                        {
                            if(strcmp(value[j], file_name) == 0)
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

        syslog(LOG_ERR, "%s, flag = %d\n", __func__, flag);

        if(flag)
        {
            // ret = kdk_debian_install_offline(uri, call_back);
            return 0;
        }
        else
            ret = kdk_kare_tag_install_offline(uri, type, compat, call_back);
    }

    return ret;
}

static unsigned long kdk_package_get_debian_package_size(const char *pkg_name)
{
    FILE *fp = fopen(DPKG_DB, "rt");
    if (fp <= 0)
        return 0;

    char buffer[4097] = "\0";
    unsigned long size = 0;
    int flag = 0;

    while (fgets(buffer, 4096, fp))
    {
        char *val;
        val = lookup(buffer, "Package");
        // printf("val = %s, pkg_name = %s\n", val, pkg_name);

        if(val && strcmp(val, pkg_name) == 0)
        {
            flag = 1;
        }

        if (flag && (val = lookup(buffer, "Installed-Size")))
        {
            size = atol(val);
            break;
        }
    }
    fclose(fp);

    return size;
}

static unsigned long kdk_package_get_kare_package_size(const char *pkg_name)
{
    unsigned long size = 0;

    int pipe_fd[2];
    pid_t pid;
    bool installable = false;

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

        char *args[] = {"kare", "-s", (char *)pkg_name, NULL};

        execvp("kare", args);
        
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            return false;
        }

        char line[1024] = "\0";
        char *val = NULL;
        while (fgets(line, sizeof(line), fp)) {
            strstripspace(line);
            
            if (strncmp(line, "Installed-Size:", 15) == 0) {
                if ((val = lookup(line, "Installed-Size")))
                {
                    size = atol(val);
                }
            }
        }
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        // 检查命令执行状态
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            installable = false;
        }
    }
    return size;
}

static unsigned long kdk_package_get_kaiming_package_size(const char *pkg_name)
{
    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;
    DBusError error;
    unsigned long size = 0;
    char *str = NULL;
    cJSON *root = NULL;
    
    if (0 == access("/opt/kaiming-tools/bin/kaiming", F_OK))
    {
        // 初始化DBus连接
        dbus_error_init(&error);

        connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        if (connection == NULL)
        {
            klog_err("DBus connection error: %s\n", error.message);
            goto out;
        }

        // 创建DBus方法调用消息
        message = dbus_message_new_method_call("org.kylin.kaiming",
                                            "/org/kylin/kaiming",
                                            "org.kylin.kaiming.interface",
                                            "getLocalInfo");
        if (message == NULL)
        {
            klog_err("DBus message allocation failed\n");
            goto out;
        }

        if (!dbus_message_append_args(message, DBUS_TYPE_STRING, &pkg_name, DBUS_TYPE_INVALID))
        {
            klog_err("kdk : d-bus append args fail !\n");
            goto out;
        }

        // 发送DBus消息并等待回复
        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            klog_err("DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_connection_flush(connection);

        if (message)
        {
            // 释放message
            dbus_message_unref(message);
            message = NULL;
        }

        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) 
        {
            klog_err("DBus error: %s\n", dbus_message_get_error_name(reply));
            goto out;
        }

        DBusMessageIter args;

        if (!dbus_message_iter_init(reply, &args))
        {
            klog_err("kdk : d-bus reply message fail !\n");
            goto out;
        }
        else
        {
            dbus_message_iter_get_basic(&args, &str);
        }

        if (reply)
        {
            // 释放reply
            dbus_message_unref(reply);
            reply = NULL;
        }
        dbus_connection_unref(connection);
        connection = NULL;

        klog_info("get version Got string: %s\n", str);
        if(str)
        {
            root = cJSON_Parse(str);
            if (root == NULL) 
            {
                klog_err("Failed to parse JSON response\n");
                goto out;
            }

            cJSON *km_size = cJSON_GetObjectItem(root, "size");
            if(km_size)
            {
                klog_info("km_size->valuestring = %s\n", km_size->valuestring);
                
                if (km_size->valuestring && strnlen(km_size->valuestring, STRNLEN_MAX) != 0)
                {
                    size = atol(km_size->valuestring);
                }
            }
        }
        
    }
out:
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
    if(message)
    {
        dbus_message_unref(message);
    }
    if(reply)
    {
        dbus_message_unref(reply);
    }
    if(connection)
    {
        dbus_connection_unref(connection);
    }
    dbus_error_free(&error);

    return size;
}

unsigned long kdk_package_get_package_size(const char *pkg_name)
{
    unsigned long size = 0;
    size = kdk_package_get_debian_package_size(pkg_name);
    if(size != 0)
        return size;
    
    size = kdk_package_get_kare_package_size(pkg_name);
    if(size != 0)
        return size;

    size = kdk_package_get_kaiming_package_size(pkg_name);
    return size;
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
    bool is_deb_file = (strstr(uri, ".deb") != NULL) && (strlen(uri) > 4) && (strcmp(uri + strlen(uri) - 4, ".deb") == 0);

    if (is_deb_file) {
        // 使用 dpkg-deb -f 获取 .deb 文件的包名
        snprintf(cmd, sizeof(cmd), "dpkg-deb -f '%s' Package 2>/dev/null", uri);
        syslog(LOG_INFO, "[%s], cmd : %s\n", __func__, cmd);
        FILE *fp_pkg = popen(cmd, "r");
        if (!fp_pkg) {
            syslog(LOG_ERR, "Failed to execute command: %s\n", cmd);
            return false;
        }

        if (!fgets(package_name, sizeof(package_name), fp_pkg)) {
            syslog(LOG_ERR, "Failed to get package name from deb file: %s\n", uri);
            pclose(fp_pkg);
            return false;
        }
        pclose(fp_pkg);

        // 去除包名末尾的换行符
        strstripspace(package_name);
    } else {
        // 直接使用传入的包名
        strncpy(package_name, uri, sizeof(package_name) - 1);
        package_name[sizeof(package_name) - 1] = '\0';
    }
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
            char target_link[512];
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


// 设置所有信号处理器
static void setup_signal_handlers() {
    
    // SIGINT Ctrl+C
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // 重启被中断的系统调用
    sa.sa_handler = sigint_handler;

    // 注册SIGINT处理函数，覆盖默认行为
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        syslog(LOG_ERR, "注册SIGINT信号处理函数失败");
        exit(1);
    }
    
    // SIGTERM kill 命令默认
    sa.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &sa, NULL);
    
    // SIGQUIT Ctrl+ '\'
    sa.sa_handler = sigquit_handler;
    sigaction(SIGQUIT, &sa, NULL);
    
    // SIGHUP 终端关闭
    sa.sa_handler = sighup_handler;
    sigaction(SIGHUP, &sa, NULL);
    
    // SIGTSTP Ctrl+Z
    sa.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &sa, NULL);
}

// 执行 ar 命令获取 data.tar 信息
static int execute_ar_command(const char* deb_file, char* output, size_t output_size) {
    char command[512];
    snprintf(command, sizeof(command), "ar -tvO '%s' | grep data.tar", deb_file);
    
    FILE* fp = popen(command, "r");
    if (!fp) {
        syslog(LOG_ERR, "错误: 无法执行 ar 命令\n");
        return -1;
    }

    // 读取命令输出
    if (fgets(output, output_size, fp) == NULL) {
        syslog(LOG_ERR, "错误: 找不到 data.tar 文件\n");
        pclose(fp);
        return -1;
    }
    
    // 移除换行符
    size_t len = strlen(output);
    if (len > 0 && output[len-1] == '\n') {
        output[len-1] = '\0';
    }

    pclose(fp);
    return 0;
}

// 解析 ar 命令输出
static DebInfo parse_ar_output(const char* ar_output) {
    DebInfo info = {0, 0, 0};
    
    // 示例输出可能包含十六进制:
    // "rw-r--r-- 1000/1000  12345678  Dec 31 23:59 data.tar 0x3d0"
    
    char buffer[1024];
    strncpy(buffer, ar_output, sizeof(buffer));
    buffer[sizeof(buffer)-1] = '\0';
    
    // 分割字符串
    char* tokens[32];
    int token_count = 0;
    
    char* token = strtok(buffer, " \t");
    while (token && token_count < 32) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t");
    }
    
    if (token_count < 7) {
        syslog(LOG_ERR, "警告: ar 输出格式异常\n");
        return info;
    }
    
    // 第3列是大小（可能是十进制或十六进制）
    char* endptr;
    info.size = strtol(tokens[2], &endptr, 0);  // 0表示自动识别进制
    if (endptr == tokens[2]) {
        syslog(LOG_ERR, "错误: 无法解析大小 '%s'\n", tokens[2]);
        return info;
    }
    
    // 最后一列是偏移量（可能是十进制或十六进制）
    info.offset = strtol(tokens[token_count - 1], &endptr, 0);
    if (endptr == tokens[token_count - 1]) {
        syslog(LOG_ERR, "错误: 无法解析偏移量 '%s'\n", tokens[token_count - 1]);
        return info;
    }
    
    info.total_size = info.size + info.offset;
    
    return info;
}

// 计算 deb 文件前半部分的 SHA512（分段读取）
static int calculate_deb_sha512(const char* deb_file, off_t total_size, unsigned char* hash) {
    int fd = open(deb_file, O_RDONLY);
    if (fd < 0) {
        syslog(LOG_ERR, "打开文件失败");
        return -1;
    }

    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    
    unsigned char buffer[BUFFER_SIZE];
    off_t bytes_read = 0;
    ssize_t n;
    
    // 分段读取并计算哈希
    while (bytes_read < total_size) {
        size_t to_read = BUFFER_SIZE;
        if (bytes_read + to_read > total_size) {
            to_read = total_size - bytes_read;
        }
        
        n = read(fd, buffer, to_read);
        if (n < 0) {
            syslog(LOG_ERR, "读取文件失败");
            close(fd);
            return -1;
        }
        if (n == 0) {
            break;  // EOF
        }
        
        SHA512_Update(&ctx, buffer, n);
        bytes_read += n;
    }
    
    SHA512_Final(hash, &ctx);
    close(fd);
    
    return 0;
}

static bool key_trust_is_trusted()
{
    char command[1024];
    char *key_path = "/usr/share/keyrings/deb-sign/";
    char mkdir_cmd[512];
    int result = -1;

    if (access(key_path, F_OK) != 0) {
        // 创建目录，包括父目录
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", key_path);
        result = system(mkdir_cmd);
        if (result != 0) {
            syslog(LOG_ERR, "Failed to create directory: %s\n", key_path);
            return false;
        }
    }

    char *sig_path = "/tmp/kysdk-package-sig/";

    if (access(sig_path, F_OK) != 0) {
        // 创建目录，包括父目录
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", sig_path);
        result = system(mkdir_cmd);
        if (result != 0) {
            syslog(LOG_ERR, "Failed to create directory: %s\n", sig_path);
            return false;
        }
    }
    
    // 构建 gpg 导入命令
    snprintf(command, sizeof(command), "gpg --import /usr/share/keyrings/deb-sign/* > /dev/null 2>&1");
    
    // 执行命令
    result = system(command);
    
    if (result == 0) {
        syslog(LOG_INFO, "Successfully imported\n");
        return true;
    } else {
        syslog(LOG_ERR, "导入失败\n");
        return false;
    }
}

static int generate_signature_base64(const char *data, char *sig) {
    char temp_file[64];
    char sig_file[128];
    char command[4096];
    FILE *fp;
    
    // 生成随机临时文件名
    snprintf(temp_file, sizeof(temp_file), "/tmp/kysdk-package-plain-%d.txt", getpid());
    snprintf(sig_file, sizeof(sig_file), "%s.sig", temp_file);
    
    // 1. 创建数据文件 (相当于 echo -e)
    fp = fopen(temp_file, "w");
    if (!fp) {
        syslog(LOG_ERR, "ERROR: Cannot create temp file\n");
        return NULL;
    }
    fprintf(fp, "%s", data);
    fclose(fp);
    
    snprintf(command, sizeof(command),
                "echo \"%s\" | base64 -d > \"%s\" 2>/dev/null",
                sig, sig_file);
    
    if (system(command) != 0) {
        syslog(LOG_ERR, "ERROR: GPG signature failed\n");
        unlink(temp_file);
        return NULL;
    }

    snprintf(command, sizeof(command),
             "gpg --verify \"%s\" \"%s\" 2>/dev/null", sig_file, temp_file);
    
    int result = system(command);
    
    // 清理临时文件
    unlink(temp_file);
    unlink(sig_file);
    
    return result;
}

static char *kdk_get_package_name(char *uri)
{
    char cmd[1024] = "\0";
    char file_name[512] = "\0";
    char *package_name = NULL;
    snprintf(cmd, sizeof(cmd), "dpkg-deb -f '%s' Package", uri);
    syslog(LOG_ERR, "cmd = %s\n", cmd);

    FILE *fd = popen(cmd, "r");
    if(!fd)
        return package_name;
    
    fgets(file_name, sizeof(file_name), fd);
    strstripspace(file_name);
    syslog(LOG_ERR, "file_name = %s\n", file_name);
    pclose(fd);

    if(file_name[0] != '\0')
    {
        package_name = strdup(file_name);
    }
    return package_name;
}

/*是否存在特权模式白名单中，返回1是，0为不是*/
static int kdk_get_direct_install_whitelist(char *file_name)
{
    int flag = 0;

    KSettings *setting = kdk_conf2_new("libkysdk-package.direct_install_whitelist", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }

    char **keys = kdk_conf2_list_keys(setting);
    if (keys)
    {
        for (int i = 0; keys[i]; i++)
        {
            char **value = kdk_conf2_get_strv(setting, keys[i]);
            if (value)
            {
                for (int j = 0; value[j]; j++)
                {
                    if (contains_wildcard(value[j]))
                    {
                        bool match = wildcard_match(value[j], file_name);
                        if(match)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    else
                    {
                        if(strcmp(value[j], file_name) == 0)
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
    return flag;
}

int kdk_verify_signature(char *uri, char *dir, char *typebm, int callflag)
{
    char typesig[4096] = "\0";
    if (!extract_compat_file(dir, uri, "bare_metal_sig", typesig))
    {
        return 0;
    }
    if(typesig[0] != '\0')
    {
        bool trusted = key_trust_is_trusted();
        if(!trusted)
        {
            syslog(LOG_ERR, "Failed to determine whether the signature public key is trusted\n");
            if(callflag)
                _callbackinstall(0, -1, "无法确定签名公钥是否可信");
            return 0;
        }

        // 执行 ar 命令获取 data.tar 信息
        char ar_output[1024];
        if (execute_ar_command(uri, ar_output, sizeof(ar_output)) != 0) {
            return 0;
        }

        // 解析 ar 输出
        DebInfo info = parse_ar_output(ar_output);
        
        if (info.size == 0 || info.offset == 0) {
            syslog(LOG_ERR, "错误: 无法解析 data.tar 信息\n");
            return 0;
        }

        unsigned char hash[SHA512_DIGEST_LENGTH];
        
        if (calculate_deb_sha512(uri, info.total_size, hash) != 0) {
            syslog(LOG_ERR, "错误: 计算 SHA512 失败\n");
            if(callflag)
                _callbackinstall(0, -1, "错误: 计算 SHA512 失败");
            return 0;
        }

        char hex_str[SHA512_DIGEST_LENGTH * 2] = "\0";

        for (size_t i = 0; i < SHA512_DIGEST_LENGTH; i++) {
            snprintf(hex_str + (i * 2), 3, "%02x", hash[i]);
        }
        hex_str[SHA512_DIGEST_LENGTH * 2] = '\0';

        char signature[256] = "\0";
        snprintf(signature, sizeof(signature), "%s%s", hex_str, typebm);
        
        int base64 = generate_signature_base64(signature, typesig);
        syslog(LOG_INFO, "base64 = %d\n", base64);

        if(base64 != 0)
        {
            syslog(LOG_ERR, "Signature verification failed\n");
            if(callflag)
                _callbackinstall(0, -1, "签名验证失败");
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        syslog(LOG_ERR, "no signature field\n");
        if(callflag)
            _callbackinstall(0, -1, "特权标记签名获取失败");
        return 0;
    }
}

static char* kdk_download_package(char *pkg_name, const char *dir)
{    
    char cwd[1024];  // 存储当前目录路径
    char cmd[1024]= "\0";
    if (getcwd(cwd, sizeof(cwd)) != NULL) 
    {
        syslog(LOG_DEBUG, "Current directory: %s\n", cwd);

        if (chdir(dir) == 0) 
        {
            syslog(LOG_DEBUG, "Switched to directory: %s\n", dir);
            memset(cmd, 0, 1024);
            
#ifdef __loongarch64
            const char* sources_name = "/etc/apt/sources.list";
            char* http_url = get_first_active_repo_url(sources_name);
            if (http_url) {
                syslog(LOG_DEBUG, "[%s] http url = %s\n", __func__, http_url);
                bool ret_access = test_connectivity(http_url);
                free(http_url);
                if(!ret_access)
                {
                    _callbackinstall(0, -1, "弱网环境，请检查网络连通性");
                    return NULL;
                }
            } else {
                syslog(LOG_DEBUG, "[%s] No uncommented sources found or file read failed", __func__);
                return NULL;
            }
            snprintf(cmd, sizeof(cmd), "dpkg --add-architecture loongarch64 && apt.orig update");
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);

            snprintf(cmd, sizeof(cmd), "apt.orig download %s", pkg_name);
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);

            snprintf(cmd, sizeof(cmd), "dpkg --remove-architecture loongarch64 && apt.orig update");
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);
#else
            snprintf(cmd, sizeof(cmd), "apt.orig download %s", pkg_name);
            syslog(LOG_DEBUG, "cmd = %s\n", cmd);
            system(cmd);
#endif
            if (chdir(cwd) != 0) 
            {
                syslog(LOG_ERR, "Failed to change directory");
                return NULL;
            }
        } 
        else 
        {
            syslog(LOG_ERR, "Failed to change directory");
            return NULL;
        }
    } else {
        syslog(LOG_ERR, "getcwd failed");
        return NULL;
    }
    char filename[512] = "\0";
    char deb_path[1024] = "\0";
    char version[128] = "\0";
    char package_name [128] = "\0";
    sscanf(pkg_name, "%[^=]= %s", package_name, version);
    syslog(LOG_DEBUG, "package_name = %s, version = %s", package_name, version);
    DIR *dp;
    struct dirent *entry;
    dp = opendir(dir);
    if (dp == NULL) 
    {
        return NULL;
    }
    while ((entry = readdir(dp)) != NULL) 
    {
    // 跳过当前目录和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }

        if(strstr(entry->d_name, package_name))
        {
            strncpy(filename, entry->d_name, sizeof(filename) - 1);
        }
    }
    closedir(dp);

    memset(deb_path, 0, 1024);
    snprintf(deb_path, sizeof(deb_path), "%s/%s", dir, filename);
    syslog(LOG_DEBUG, "deb_path: %s\n", deb_path);

    if (filename[0] == '\0')
    {
        _callbackinstall(0, -1, "软件包不存在");
        return NULL;
    }

    return strdup(deb_path);
}

void interrupt_signal_handler() {
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d] 收到退出请求，生成调试信息...\n", g_verify_end_count);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

static void kdk_set_privileged_tag_packag(char *pkg_name)
{
    KSettings *setting = kdk_conf2_new("libkysdk-package.bare_metal", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }
    
    char **value = kdk_conf2_get_strv(setting, "pkg-list");
    int j = 0;
    int flag = 0;
    if (value)
    {
        for (j; value[j]; j++)
        {
            syslog(LOG_INFO, "[%s] List value : %s, file_name : %s", __func__, value[j], pkg_name);
            
            if(strcmp(value[j], pkg_name) == 0)
            {
                flag = 1;
            }
        }

        if (!flag)  // 没找到，追加
        {
            // 创建临时数组，包含原有所有元素 + 新元素
            int old_size = j;
            const char *temp_array[old_size + 2];
            
            // 复制原有元素
            for (int k = 0; k < old_size; k++) {
                temp_array[k] = value[k];  // 注意：这只是指针，不能直接free value
            }
            
            // 追加新元素
            temp_array[old_size] = pkg_name;
            temp_array[old_size + 1] = NULL;
            
            // 写入配置
            if (kdk_conf2_set_strv(setting, "pkg-list", temp_array) == 0) {
                syslog(LOG_INFO, "[%s] Appended %s to list", __func__, pkg_name);
            }
        }
        g_strfreev(value);
    }
    kdk_conf2_ksettings_destroy(setting);
}

static int kdk_get_privileged_tag_packag(char *pkg_name)
{
    KSettings *setting = kdk_conf2_new("libkysdk-package.bare_metal", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }
    
    char **value = kdk_conf2_get_strv(setting, "pkg-list");
    int j = 0;
    int flag = 0;
    if (value)
    {
        for (j; value[j]; j++)
        {
            syslog(LOG_INFO, "[%s] List value : %s, file_name : %s", __func__, value[j], pkg_name);
            
            if(strcmp(value[j], pkg_name) == 0)
            {
                flag = 1;
            }
        }
        g_strfreev(value);
    }
    kdk_conf2_ksettings_destroy(setting);
    return flag;
}
void kdk_package_interrupt_signal_handler() 
{
    int current_count = atomic_load(&g_verify_end_count);
    syslog(LOG_INFO, "[%d] 收到退出请求，生成调试信息...\n", g_verify_end_count);
    if (current_count == 0) {
        atomic_store(&g_verify_end_count, 1); // 标记已触发退出
        kdk_deb_verify_end();
        syslog(LOG_INFO, "kdk_ns_mode_flag_enter = %d, kdk_ns_mode_session_id = %d\n", kdk_ns_mode_flag_enter, kdk_ns_mode_session_id);
        if (kdk_ns_mode_session_id != 0 && kdk_ns_mode_flag_enter)
        {
            syslog(LOG_INFO, "kdk_ns_mode_session_id = %d\n", kdk_ns_mode_session_id);
            kdk_ns_mode_exit((ns_mode_session_id_t)kdk_ns_mode_session_id);
            kdk_ns_mode_flag_enter = 0;
        }
        exit(EXIT_FAILURE);
    }
}

static int kdk_debian_offline_install_in_whitelist(char *uri, char *value, char *file_name, InstallCallBack call_back)
{
    int ret = 0;
    int order = 0;
    int flag_version = 0;
    char runtime_version[64] = "\0";
    bool flag = kdk_get_whitelist_runtime_check(); // 获取白名单是否检查runtime_version开关
    if (flag)
    {
        for (const VersionPattern *p = version_patterns; p->pattern; p++)
        {
            if (strstr(value, p->pattern) && flag_version != 1)
            {
                if (order == 0)
                    order = p->order;
                flag_version = 1;
                syslog(LOG_INFO, "value = %s, order = %d, p->value = %s\n", value, order, p->value);
                strcpy(runtime_version, p->value);
            }
        }

        if (runtime_version[0] != '\0' && strcmp(runtime_version, "shadow") == 0)
        {
            if (kdk_get_direct_install_whitelist(file_name))
            {
                ret = kdk_debian_install_offline(uri, call_back);
            }
        }
        else
        {
            _callbackinstall(0, -1, "软件包标签匹配失败");
        }
    }
    else
    {
        if (kdk_get_direct_install_whitelist(file_name))
        {
            ret = kdk_debian_install_offline(uri, call_back);
        }
    }
    return ret;
}

static bool kdk_get_whitelist_runtime_check()
{
    int check = 1;
    KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }

    check = kdk_conf2_get_boolean(setting, "whitelist_runtime_check");
    
    syslog(LOG_INFO, "whitelist_runtime_check value: %d\n", check);
    kdk_conf2_ksettings_destroy(setting);
    
    return check;
}

/*是否存在特权标签，返回1是，0为不是*/
static int kdk_is_privilege_label(char *dir, char *uri, char *type)
{
    char typebm[128] = "\0";

    if (!extract_compat_file(dir, uri, "bare_metal", typebm))
    {
        return 0;
    }
    // 是特权标签，需要验签
    if (typebm[0] != '\0' && (strncmp(typebm, "yes", 3) == 0))
    {
        strcpy(type, typebm);
        return 1;
    }
    return 0;
}

static int kdk_debian_offline_install_in_privilege(char *dir, char *uri, char *value, InstallCallBack call_back)
{
    int ret = 0;
    char typebm[128] = "\0";
    int order = 0;
    int flag_version = 0;
    char runtime_version[64] = "\0";
    bool flag = kdk_get_privilege_runtime_check(); // 获取特权标签是否检查runtime_version开关
    if (flag)
    {
        for (const VersionPattern *p = version_patterns; p->pattern; p++)
        {
            if (strstr(value, p->pattern) && flag_version != 1)
            {
                if (order == 0)
                    order = p->order;
                flag_version = 1;
                syslog(LOG_INFO, "value = %s, order = %d, p->value = %s\n", value, order, p->value);
                strcpy(runtime_version, p->value);
            }
        }
        if (runtime_version[0] != '\0' && (strcmp(runtime_version, "shadow") == 0))
        {
            if (kdk_is_privilege_label(dir, uri, typebm))
            {
                int result = kdk_verify_signature(uri, dir, typebm, 1);
                if (result)
                {
                    privilege_tag = 1;
                    ret = kdk_debian_install_offline(uri, call_back);
                }
            }
        }
        else
        {
            _callbackinstall(0, -1, "软件包标签匹配失败");
        }
    }
    else
    {
        if (kdk_is_privilege_label(dir, uri, typebm))
        {
            int result = kdk_verify_signature(uri, dir, typebm, 1);
            if (result)
            {
                privilege_tag = 1;
                ret = kdk_debian_install_offline(uri, call_back);
            }
        }
    }
}

static bool kdk_get_privilege_runtime_check()
{
    int check = 1;
    KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }

    check = kdk_conf2_get_boolean(setting, "privilege_runtime_check");

    syslog(LOG_INFO, "privilege_runtime_check value: %d\n", check);
    kdk_conf2_ksettings_destroy(setting);

    return check;
}

static int kdk_debian_online_install_in_whitelist(char *uri, char *value, char *file_name, InstallCallBack call_back)
{
    int ret = 0;
    int order = 0;
    int flag_version = 0;
    char runtime_version[64] = "\0";
    char package_name [128] = "\0";
    char version[128] = "\0";

    if(strstr(file_name, "="))
    {
        sscanf(file_name, "%[^=]= %s", package_name, version);
    }
    else
    {
        strncpy(package_name, file_name, sizeof(package_name) -1);
    }
    bool flag = kdk_get_whitelist_runtime_check(); // 获取白名单是否检查runtime_version开关
    if (flag)
    {
        for (const VersionPattern *p = version_patterns; p->pattern; p++)
        {
            if (strstr(value, p->pattern) && flag_version != 1)
            {
                if (order == 0)
                    order = p->order;
                flag_version = 1;
                syslog(LOG_INFO, "value = %s, order = %d, p->value = %s\n", value, order, p->value);
                strcpy(runtime_version, p->value);
            }
        }

        if (runtime_version[0] != '\0' && strcmp(runtime_version, "shadow") == 0)
        {
            if (kdk_get_direct_install_whitelist(package_name))
            {
                ret = kdk_debian_install_online(uri, file_name, call_back);
            }
        }
        else
        {
            _callbackinstall(0, -1, "软件包标签匹配失败");
        }
    }
    else
    {
        if (kdk_get_direct_install_whitelist(package_name))
        {
            ret = kdk_debian_install_online(uri, file_name, call_back);
        }
    }
    return ret;
}

static int kdk_debian_online_install_in_privilege(char *dir, char *uri, char *value, char *file_name, InstallCallBack call_back)
{
    int ret = 0;
    char typebm[128] = "\0";
    int order = 0;
    int flag_version = 0;
    char runtime_version[64] = "\0";
    bool flag = kdk_get_privilege_runtime_check(); // 获取特权标签是否检查runtime_version开关
    if (flag)
    {
        for (const VersionPattern *p = version_patterns; p->pattern; p++)
        {
            if (strstr(value, p->pattern) && flag_version != 1)
            {
                if (order == 0)
                    order = p->order;
                flag_version = 1;
                syslog(LOG_INFO, "value = %s, order = %d, p->value = %s\n", value, order, p->value);
                strcpy(runtime_version, p->value);
            }
        }
        if (runtime_version[0] != '\0' && (strcmp(runtime_version, "shadow") == 0))
        {
            if (kdk_is_privilege_label(dir, uri, typebm))
            {
                int result = kdk_verify_signature(uri, dir, typebm, 1);
                if (result)
                {
                    privilege_tag = 1;
                    ret = kdk_debian_install_online(uri, file_name, call_back);
                }
            }
        }
        else
        {
            _callbackinstall(0, -1, "软件包标签匹配失败");
        }
    }
    else
    {
        if (kdk_is_privilege_label(dir, uri, typebm))
        {
            int result = kdk_verify_signature(uri, dir, typebm, 1);
            if (result)
            {
                privilege_tag = 1;
                ret = kdk_debian_install_online(uri, file_name, call_back);
            }
        }
    }
}

static bool kdk_get_pkg_sign_check()
{
    int check = 1;
    KSettings *setting = kdk_conf2_new("libkysdk-package.installation", NULL);
    if (NULL == setting)
    {
        syslog(LOG_ERR, "get handle failed\n");
        return 0;
    }

    check = kdk_conf2_get_boolean(setting, "pkg_sign_check");
    
    syslog(LOG_INFO, "pkg_sign_check value: %d\n", check);
    kdk_conf2_ksettings_destroy(setting);
    
    return check;
}