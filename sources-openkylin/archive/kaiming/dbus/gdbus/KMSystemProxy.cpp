/*
 * Copyright (c) KylinSoft Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

/**
 * @brief 开明穿透系统代理
 */

#include "dbus/kmsystemproxy.h"
#include <sys/wait.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <unistd.h>
#include "common/KMCommonUtils.h"
#include "common/KMProcessUtils.h"
#include "common/KMLogger.h"

namespace fs = std::filesystem;

#define KAIMING_DBUS_SERVICE "org.kaiming.proxy"
#define KAIMING_DBUS_PATH "/org/kaiming/proxy"
#define KAIMING_DBUS_INTERFACE "org.kaiming.proxy.system"

struct StartBinaryTask
{
    ProxySystem *conn;
    int pid;
    std::string command;
    std::string pwd;
};

struct TaskResult
{
    int ret;
    std::string msg;
};

static void taskReply(void *taskData, const char *reply)
{
    kmlogger.debug("reply: %s", reply);
    auto *data = static_cast<StartBinaryTask *>(taskData);
    proxy_system_emit_binary_reply(data->conn, data->pid, reply);
}

static bool checkKaimingNamespace(pid_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/ns/mnt";
    std::error_code ec;
    fs::path symlink_path(path);

    if (!fs::is_symlink(symlink_path, ec)) {
        return false;
    }
    fs::path target = fs::read_symlink(symlink_path, ec);
    if (ec) {
        return false;
    }
    std::string ns = target.string();
    size_t l = ns.find('[');
    size_t r = ns.find(']');
    if (l == std::string::npos || r == std::string::npos)
        return false;
    std::string mntId = ns.substr(l + 1, r - l - 1);

    //获取 namespace PID 列表
    std::string cmd1 = "lsns -t mnt | awk '$1==" + mntId + " {print $4}'";
    std::string nsPid = KMProcessUtils::execCmd(cmd1.c_str()).first;
    if (nsPid.empty())
        return false;

    //获取 kaiming ps PID 列表
    std::string cmd2 = "kaiming ps | tail -n +2 | awk '{print $3}' | grep -w " + nsPid;
    std::string kmPid = KMProcessUtils::execCmd(cmd2.c_str()).first;

    return !kmPid.empty();
}

static void onStartBinaryAsync(GTask *task, gpointer, gpointer taskData, GCancellable *)
{
    auto *data = static_cast<StartBinaryTask *>(taskData);

    if (data->pid <= 0)
    {
        kmlogger.error("invalid pid");
        g_task_return_pointer(task, g_strdup("exit=-1"), g_free);
        return;
    }

    if (!checkKaimingNamespace(data->pid))
    {
        kmlogger.info("not allowed");
        g_task_return_pointer(task, g_strdup("exit=-1"), g_free);
        return;
    }

    kmlogger.debug("[DBus] system pid: %d, command: %s", data->pid, data->command.c_str());
    KMCommonUtils::runGtaskCommand(task, data->command, data->pwd, taskReply, taskData);
}

static void onStartBinaryDone(GObject *, GAsyncResult *res, gpointer)
{
    std::string output;
    GError *error = nullptr;
    GTask *task = G_TASK(res);
    auto *data = static_cast<StartBinaryTask *>(g_task_get_task_data(task));
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(g_task_get_source_object(task));
    gchar *result = static_cast<gchar *>(g_task_propagate_pointer(task, &error));
    if (error)
    {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
        output = "start binary done|exit=-1";
    }
    else
    {
        proxy_system_complete_start_binary_as_user(PROXY_SYSTEM(data->conn), invocation);
        output = std::string("start binary done|") + (result ? result : "exit=-1");
        g_free(result);
    }
    taskReply(data, output.c_str());
}

static gboolean onHandleStartBinaryAsUser(ProxySystem *object, GDBusMethodInvocation *invocation, gint uid, const gchar *binary, const gchar *pwd, gpointer userData)
{
    auto *data = new StartBinaryTask{ object, uid, binary, pwd };
    GTask *task = g_task_new(invocation, nullptr, onStartBinaryDone, data);
    g_task_set_task_data(task, data, [](gpointer d) {
        delete static_cast<StartBinaryTask *>(d);
    });
    g_task_run_in_thread(task, onStartBinaryAsync);
    g_object_unref(task);
    return true;
}

static void onCommandAsync(GTask *task, gpointer, gpointer taskData, GCancellable *)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "fork failed");
        return;
    }

    if (pid == 0)
    {
        auto data = static_cast<gchar **>(taskData);
        execv(KAIMING_BOX, const_cast<char *const *>(data));
        exit(EXIT_FAILURE);
    }
    else
    {
        // 父进程
        int status = 0;
        std::string output, log;
        std::error_code ec;
        waitpid(pid, &status, 0);
        int ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        auto data = static_cast<gchar **>(taskData);
        if (data[2] != nullptr)
        {
            log = std::string(data[2]);
        }

        if (fs::exists(log, ec))
        {
            if (ret != 0)
            {
                std::ifstream ifs(log);
                if (ifs)
                {
                    std::getline(ifs, output);
                }
            }
            // 删除日志文件
            fs::remove(log, ec);
        }

        TaskResult *result = new TaskResult{ ret, output };
        g_task_return_pointer(task, result, [](gpointer d) {
            delete static_cast<TaskResult *>(d);
        });
    }
}

static void onCommandDone(GObject *, GAsyncResult *res, gpointer userData)
{
    GError *error = nullptr;
    GTask *task = G_TASK(res);
    GDBusMethodInvocation *invocation = G_DBUS_METHOD_INVOCATION(g_task_get_source_object(task));
    TaskResult *result = static_cast<TaskResult *>(g_task_propagate_pointer(task, &error));
    if (error)
    {
        g_dbus_method_invocation_return_gerror(invocation, error);
        g_error_free(error);
    }
    else
    {
        proxy_system_complete_run(PROXY_SYSTEM(userData), invocation, result->ret, result->msg.c_str());
        delete result;
    }

    KMDebug("onCommandDone finish .");
}

static gboolean asyncTask(GDBusMethodInvocation *invocation, bool isrun, const gchar *const *options, const gchar *const *soptions, const gchar *cid, const gchar *command, gpointer userData)
{
    if (soptions == nullptr || cid == nullptr || (!isrun && command == nullptr))
    {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "args cannot be empty");
        return false;
    }

    gchar **argv = KMCommonUtils::makeArgv(isrun, options, soptions, cid, command);
    if (argv == nullptr)
    {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, "make argv failed");
        return false;
    }

    auto *task = g_task_new(invocation, nullptr, onCommandDone, userData);
    g_task_set_task_data(task, argv, (GDestroyNotify)g_strfreev);
    g_task_run_in_thread(task, onCommandAsync);
    g_object_unref(task);
    return true;
}

static gboolean onHandleRun(ProxySystem *object, GDBusMethodInvocation *invocation, const gchar *const *options, const gchar *const *soptions, const gchar *cid, gpointer userData)
{
    return asyncTask(invocation, true, options, soptions, cid, "", userData);
}

static gboolean onHandleEnter(ProxySystem *object, GDBusMethodInvocation *invocation, const gchar *const *options, const gchar *const *soptions, const gchar *cid, const gchar *command, gpointer userData)
{
    return asyncTask(invocation, false, options, soptions, cid, command, userData);
}

static gboolean onHandleKill(ProxySystem *object, GDBusMethodInvocation *invocation, const gchar *id, gpointer userData)
{
    int ret = 0;
    std::string msg;
    KMCommonUtils::kmkill(ret, msg, id);
    proxy_system_complete_kill(PROXY_SYSTEM(userData), invocation, ret, msg.c_str());
    return true;
}

static gboolean onHandlePs(ProxySystem *object, GDBusMethodInvocation *invocation, gpointer userData)
{
    int ret = 0;
    std::string msg;
    KMCommonUtils::ps(ret, msg);
    proxy_system_complete_ps(PROXY_SYSTEM(userData), invocation, ret, msg.c_str());
    return true;
}

static gboolean onHandleGetRunStatus(ProxySystem *object, GDBusMethodInvocation *invocation, const gchar *appId, const gchar *uid, gpointer userData)
{
    bool ret;
    std::string containerid;
    ret = KMCommonUtils::getRunStatus(appId, uid, containerid);
    proxy_system_complete_get_run_status(PROXY_SYSTEM(userData), invocation, ret, containerid.c_str());
    return true;
}

static void onBusAcquired(GDBusConnection *connection, const gchar *, gpointer)
{
    GError *error = nullptr;
    auto *service = proxy_system_skeleton_new();
    g_signal_connect(service, "handle-start-binary-as-user", G_CALLBACK(onHandleStartBinaryAsUser), nullptr);
    g_signal_connect(service, "handle-run", G_CALLBACK(onHandleRun), nullptr);
    g_signal_connect(service, "handle-enter", G_CALLBACK(onHandleEnter), nullptr);
    g_signal_connect(service, "handle-ps", G_CALLBACK(onHandlePs), nullptr);
    g_signal_connect(service, "handle-kill", G_CALLBACK(onHandleKill), nullptr);
    g_signal_connect(service, "handle-get-run-status", G_CALLBACK(onHandleGetRunStatus), nullptr);

    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(service), connection, KAIMING_DBUS_PATH, &error);
    if (error)
    {
        g_clear_error(&error);
        g_object_unref(service);
        KMError("Failed to export kaiming system proxy interface .");
    }
}

int main()
{
    auto ownerId = g_bus_own_name(G_BUS_TYPE_SYSTEM, KAIMING_DBUS_SERVICE, G_BUS_NAME_OWNER_FLAGS_NONE, onBusAcquired, nullptr, nullptr, nullptr, nullptr);
    if (ownerId == 0)
    {
        KMError("Failed to acquire system dbus name .");
        return EXIT_FAILURE;
    }

    auto *loop = g_main_loop_new(nullptr, false);
    if (!loop)
    {
        KMError("Failed to create GMainLoop .");
        g_bus_unown_name(ownerId);
        return EXIT_FAILURE;
    }

    // 重定向标准输入输出到 /dev/null
    KMCommonUtils::redirectToNull();

    g_main_loop_run(loop);
    g_bus_unown_name(ownerId);
    g_main_loop_unref(loop);
    return 0;
}