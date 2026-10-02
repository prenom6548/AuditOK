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
#include <dlfcn.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <dbus-1.0/dbus/dbus.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>

typedef enum _kdk_boot_mode
{
    MODE_NONE = 0,          // 系统类型未知
    MODE_MAINTAIN,          // 运维模式
    MODE_NORMAL             // 用户模式
} kdk_boot_mode;

typedef unsigned long long ns_mode_session_id_t;

#define OSTREE_MODE_LIB                "/usr/lib/ostreemode/libostreemode.so"
#define NS_MODE_ENTER_HANDLE         "ns_mode_enter"
#define NS_MODE_EXIT_HANDLE        "ns_mode_exit"
#define KDK_CMDLINE_MAX 8192

typedef ns_mode_session_id_t (*handle_ns_mode_enter)(void);
typedef int (*handle_ns_mode_exit)(ns_mode_session_id_t session_id);

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
        return mode;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        return mode;
    }

    if (sendMsgPending == NULL)
    {
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
        return mode;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        dbus_message_unref(replyMsg);
        return mode;
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
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

static bool path_is_trusted_package_wrapper(const char *path)
{
    if (!path || path[0] == '\0') {
        return false;
    }

    char real_path[PATH_MAX] = "\0";
    if (!realpath(path, real_path)) {
        return false;
    }

    if (strcmp(real_path, "/usr/bin/apt_wrapper") != 0 &&
        strcmp(real_path, "/usr/bin/apt") != 0 &&
        strcmp(real_path, "/usr/bin/apt-get") != 0 &&
        strcmp(real_path, "/usr/bin/dpkg_wrapper") != 0 &&
        strcmp(real_path, "/usr/bin/dpkg") != 0) {
        return false;
    }

    struct stat st;
    if (stat(real_path, &st) != 0) {
        return false;
    }

    if (!S_ISREG(st.st_mode) || st.st_uid != 0 ||
        (st.st_mode & (S_IWGRP | S_IWOTH))) {
        return false;
    }

    FILE *fp = fopen(real_path, "r");
    if (!fp) {
        return false;
    }

    char content[4096 + 1] = "\0";
    size_t nread = fread(content, 1, sizeof(content) - 1, fp);
    fclose(fp);

    content[nread] = '\0';
    bool apt_wrapper = strstr(content, "WRAPPER_TAG=\"apt_wrapper\"") ||
                       strstr(content, "WRAPPER_TAG=apt_wrapper");
    bool dpkg_wrapper = strstr(content, "WRAPPER_TAG=\"dpkg_wrapper\"") ||
                        strstr(content, "WRAPPER_TAG=dpkg_wrapper");

    if (apt_wrapper || dpkg_wrapper) {
        return true;
    }

    return false;
}

static bool is_script_interpreter(const char *path)
{
    if (!path) {
        return false;
    }

    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;

    return strcmp(base, "bash") == 0 ||
           strcmp(base, "sh") == 0 ||
           strcmp(base, "dash") == 0;
}

static bool is_called_by_package_wrapper(void)
{
    pid_t ppid = getppid();
    if (ppid <= 1) {
        return false;
    }

    char proc_path[64] = "\0";
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", ppid);

    int fd = open(proc_path, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    char cmdline[KDK_CMDLINE_MAX] = "\0";
    ssize_t nread = read(fd, cmdline, sizeof(cmdline) - 1);
    close(fd);

    if (nread <= 0) {
        return false;
    }
    cmdline[nread] = '\0';

    char *argv0 = cmdline;
    char *argv1 = NULL;
    size_t argv0_len = strnlen(argv0, (size_t)nread);
    if (argv0_len + 1 < (size_t)nread) {
        argv1 = argv0 + argv0_len + 1;
    }

    if (path_is_trusted_package_wrapper(argv0)) {
        return true;
    }

    /*
     * apt_wrapper/dpkg_wrapper are shell scripts. Whether they are started by a terminal,
     * pkexec, or another application, the direct parent of kypackage_general
     * should normally be the script interpreter running a trusted wrapper path.
     */
    if (is_script_interpreter(argv0) && argv1 && path_is_trusted_package_wrapper(argv1)) {
        return true;
    }

    return false;
}

// 检测是否是包管理命令
static const char *get_package_manager_command_path(const char *cmd) {
    if (cmd == NULL) return NULL;
    
    if (strcmp(cmd, "apt.orig") == 0) {
        return "/usr/bin/apt.orig";
    }
    if (strcmp(cmd, "apt-get.orig") == 0) {
        return "/usr/bin/apt-get.orig";
    }
    if (strcmp(cmd, "dpkg.orig") == 0) {
        return "/usr/bin/dpkg.orig";
    }
    return NULL;
}

static int kdk_general(int argc, char *parameter[])
{
    if (argc < 2) {
        return -1;
    }

    const char *command_path = get_package_manager_command_path(parameter[1]);
    if (!command_path) {
        return -1;
    }

    int flag_enter = 0;
    int session_id = 0;
    int ret = -1;
    if (kdk_get_boot_mode() == MODE_NORMAL)
    {
        session_id = kdk_ns_mode_enter();
        if (session_id != 0)
        {
            flag_enter = 1;
        }
        else
        {
            return -1;
        }
    }

    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) == -1) {
        goto out;
    }

    pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        goto out;
    }

    if (pid == 0) { // 子进程
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);
        
        char *new_argv[argc + 1];  // +1 用于NULL
    
        // 复制原始参数（可以修改）
        new_argv[0] = (char *)command_path;
        for (int i = 2; i < argc; i++) {
            new_argv[i - 1] = parameter[i];
        }
        new_argv[argc-1] = NULL;  // execvp需要NULL结尾
        
        execv(new_argv[0], new_argv);
        
        // 如果 execvp 失败
        syslog(LOG_ERR, "Failed to execute dpkg: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else { // 父进程
        close(pipe_fd[1]);
        
        FILE *fp = fdopen(pipe_fd[0], "r");
        if (!fp) {
            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
            goto out;
        }

        char line[1024] = "\0";
        char *val = NULL;
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
            fflush(stdout);
        }
        
        fclose(fp);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code == 0) {
                ret = 0;   // ✅ 成功
            } else {
                ret = -1;  // ❌ dpkg/apt失败
            }
        } else {
            ret = -1;      // ❌ 被信号杀死等异常
        }
    }

out:
    if (session_id != 0 && flag_enter) {
        kdk_ns_mode_exit((ns_mode_session_id_t)session_id);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = -1;
    if (geteuid() == 0) // root 用户，可以调用接口
    {
        if (!is_called_by_package_wrapper())
        {
            printf("拒绝执行：kypackage_general\n");
            return -1;
        }
        ret = kdk_general(argc, argv);
    }
    else
    {
        printf("所请求的操作需要超级用户权限\n");
    }
    return ret;
}
