/*
 * @Author: caoxiansheng caoxiansheng@kylinos.cn
 * @Date: 2025-06-19 17:49:11
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-06-23 09:50:49
 * @FilePath: /kaiming/dbus/gdbus/KMSystemServerAdaptor.h
 * @Description: 
 */
#pragma once

#include "dbus/kmsysteminterface.h"
#include <memory>
#include <string>
#include <map>
#include <atomic>
#include <functional>

#include "app/info/KMInfoCommand.h"
#include "app/list/KMList.h"
#include "common/KMPackageManager.h"
#include "common/KMError.h"

// 通用的模板函数，用于在主线程中执行操作并自动清理资源
template<typename Func>
void postSignalToMainThread(KaimingInterface *service, Func &&func)
{
    if (!service)
        return;

    using FuncType = std::function<void(KaimingInterface*)>;

    struct CallbackData {
        GWeakRef service_ref;
        FuncType func;

        CallbackData(KaimingInterface* service, FuncType f) : func(std::move(f))
        {
            g_weak_ref_init(&service_ref, G_OBJECT(service));
        }

        ~CallbackData()
        {
            g_weak_ref_clear(&service_ref);
        }

        CallbackData(const CallbackData&) = delete;
        CallbackData& operator=(const CallbackData&) = delete;
    };

    auto *raw = new CallbackData(service, FuncType(std::forward<Func>(func)));

    g_main_context_invoke(
        nullptr,
        [](gpointer user_data) -> gboolean
        {
            std::unique_ptr<CallbackData> data(static_cast<CallbackData*>(user_data));

            if (auto *svc = KAIMING_INTERFACE(g_weak_ref_get(&data->service_ref)))
            {
                data->func(svc);
                g_object_unref(svc);
            }

            return G_SOURCE_REMOVE;
        },
        raw
    );
}

class KMSystemServerAdaptor
{
public:
    KMSystemServerAdaptor();
    ~KMSystemServerAdaptor();

    gboolean install(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch = "");
    gboolean installWithSpeedLimit(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, guint maxSpeedLimitKilobytes);
    gboolean downloadBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch, guint maxSpeedLimitKilobytes);
    gboolean deployBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch);
    gboolean installOfflinePackage(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch = "");
    gboolean update(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch = "");
    gboolean uninstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch = "");
    gboolean getLocalInfomation(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name);
    gboolean getLocalDebInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name);
    gboolean getAllLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation);
    gboolean getRemoteInfomation(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name = "");
    gboolean getDebInfoFromOK(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name);
    gboolean getRemotePackageStatus(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *version, const gchar *branch);
    gboolean getAllRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation);
    gboolean getAPPsRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation);
    gboolean isInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name);
    gboolean getAllIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names);
    gboolean isUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name);
    gboolean getAllUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names);
    gboolean getRemoteSummary(KaimingInterface *object, GDBusMethodInvocation *invocation);
    gboolean checkAndCleanupUninstalledDrivers(KaimingInterface *object, GDBusMethodInvocation *invocation);
    gboolean cancelInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskId);
    gboolean pauseInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName);
    gboolean resumeInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName);

    static gboolean handleInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, gpointer user_data);
    static gboolean handleInstallOfflinePackage(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, gpointer user_data);
    static gboolean handleDownloadBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch, guint maxSpeedLimitKilobytes, gpointer user_data);
    static gboolean handleDeployBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch, gpointer user_data);
    static gboolean handleInstallWithSpeedLimit(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, guint maxSpeedLimitKilobytes, gpointer user_data);
    static gboolean handleUpdate(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, gpointer user_data);
    static gboolean handleUninstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, gpointer user_data);
    static gboolean handleGetLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetLocalDebInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetAllLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data);
    static gboolean handleGetAllRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data);
    static gboolean handleGetAPPsRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data);
    static gboolean handleGetRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetDebInfoFromOK(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetRemotePackageStatus(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *version, const gchar *branch, gpointer user_data);
    static gboolean handleIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetAllIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, gpointer user_data);
    static gboolean handleIsUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data);
    static gboolean handleGetAllUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, gpointer user_data);
    static gboolean handleGetRemoteSummary(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data);
    static gboolean handleCheckAndCleanupUninstalledDrivers(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data);
    static gboolean handleCancelInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskId, gpointer user_data);
    static gboolean handlePauseInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName, gpointer user_data);
    static gboolean handleResumeInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName, gpointer user_data);

private:
    bool validateInputName(const gchar *name);
    bool checkOperationRunning(std::atomic<bool> &flag);

    std::unique_ptr<KMPackageManager> m_pkgManager; // 开明包管理
    std::unique_ptr<KMInfoCommand> m_kmLocalInfo;   // 获取本地信息
    std::unique_ptr<KMList> m_kmList;               // 获取列表信息

    KMPackageManager *getPackageManager();
    KMInfoCommand *getKMInfoCommand();
    KMList *getKMList();
    
    // DBus service 对象，在 on_bus_acquired 中创建，用于发送信号
    // 必须保持引用，确保在整个生命周期内都存在
    KaimingInterface *m_service = nullptr;
    
public:
    // 设置 service 对象（在 on_bus_acquired 中调用）
    void setService(KaimingInterface *service);
    
    // 获取 service 对象（用于发送信号）
    KaimingInterface *getService() const { return m_service; }

private:

    std::atomic<bool> m_installRunningFlag = FALSE;
    std::atomic<bool> m_updateRunningFlag = FALSE;
    std::atomic<bool> m_uninstallRunningFlag = FALSE;
    std::atomic<bool> m_downloadRunningFlag = FALSE;
    std::atomic<bool> m_deployRunningFlag = FALSE;
};