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

#include "dbus/kmsessionproxy.h"
#include <filesystem>
#include <sys/wait.h>
#include <string>
#include <unistd.h>
#include "common/KMCommonUtils.h"
#include "common/KMProcessUtils.h"
#include "common/KMLogger.h"

namespace fs = std::filesystem;

// D-Bus 名称和路径定义
#define KAIMING_DBUS_SERVICE "org.kaiming.proxy"
#define KAIMING_DBUS_PATH "/org/kaiming/proxy"
#define KAIMING_DBUS_INTERFACE "org.kaiming.proxy.session"

struct StartBinaryTask
{
    ProxySession *conn;
    int pid;
    std::string command;
    std::string pwd;
};

static void taskReply(void *taskData, const char *reply)
{
    kmlogger.info("reply: %s", reply);
    auto *data = static_cast<StartBinaryTask *>(taskData);
    proxy_session_emit_binary_reply(data->conn, data->pid, reply);
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

    kmlogger.info("[DBus] session pid: %d, command: %s", data->pid, data->command.c_str());
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
        proxy_session_complete_start_binary_as_user(PROXY_SESSION(data->conn), invocation);
        output = std::string("start binary done|") + (result ? result : "exit=-1");
        g_free(result);
    }
    taskReply(data, output.c_str());
}

static gboolean onHandleStartBinaryAsUser(ProxySession *object, GDBusMethodInvocation *invocation, gint pid, const gchar *binary, const gchar *pwd, gpointer userData)
{
    auto *data = new StartBinaryTask{ object, pid, binary, pwd };
    GTask *task = g_task_new(invocation, nullptr, onStartBinaryDone, data);
    g_task_set_task_data(task, data, [](gpointer d) {
        delete static_cast<StartBinaryTask *>(d);
    });
    g_task_run_in_thread(task, onStartBinaryAsync);
    g_object_unref(task);
    return true;
}

// 当总线获取成功时的回调
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    GError *error = nullptr;
    auto *service = proxy_session_skeleton_new();
    g_signal_connect(service, "handle-start-binary-as-user", G_CALLBACK(onHandleStartBinaryAsUser), nullptr);
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(service), connection, KAIMING_DBUS_PATH, &error);
    if (error)
    {
        KMError("Failed to export kaiming session proxy interface .");
        g_clear_error(&error);
        g_object_unref(service);
    }
}

int main(int argc, char **argv)
{
    auto ownerId = g_bus_own_name(G_BUS_TYPE_SESSION, KAIMING_DBUS_SERVICE, G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired, nullptr, nullptr, nullptr, nullptr);
    if (ownerId == 0)
    {
        KMError("Failed to acquire system dbus name .");
        return EXIT_FAILURE;
    }

    auto *loop = g_main_loop_new(nullptr, FALSE);
    if (!loop)
    {
        KMError("Failed to create GMainLoop .");
        g_bus_unown_name(ownerId);
        return EXIT_FAILURE;
    }

    g_main_loop_run(loop);
    g_bus_unown_name(ownerId);
    g_main_loop_unref(loop);
    return 0;
}