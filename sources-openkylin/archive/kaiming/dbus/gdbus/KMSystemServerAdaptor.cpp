/*
 * @Author: yushuoqi yushuoqi@kylinos.cn
 * @Date: 2024-04-11 20:24:44
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-05-01 10:25:13
 * @FilePath: /kaiming/app/dbus/KMSystemServerAdaptor.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * 
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSystemServerAdaptor.h"
#include <gio/gio.h>
#include <unistd.h>
#include <iostream>
#include <malloc.h>
#include <atomic>
#include <filesystem>

#include "common/KMLogger.h"
#include "common/KMError.h"
#include "common/KMFileUtils.h"
#include "common/KMFolder.h"
#include "common/KMOverlay.h"
#include "common/KMBrowserUtils.h"
#include "common/KMHostAppDeploy.h"

#define KAIMING_DBUS_SERVICE "org.kylin.kaiming"
#define KAIMING_DBUS_PATH "/org/kylin/kaiming"
#define KAIMING_DBUS_INTERFACE "org.kylin.kaiming.interface"

namespace {

constexpr const char *kKaimingTmpRoot = "/var/tmp/kaiming";

void cleanupStaleInstallTempRepos()
{
    namespace fs = std::filesystem;
    const fs::path root(kKaimingTmpRoot);

    std::error_code existsEc;
    if (!fs::exists(root, existsEc)) {
        if (existsEc) {
            kmlogger.warn("cleanupStaleInstallTempRepos: cannot access %s: %s", root.c_str(), existsEc.message().c_str());
        }
        return;
    }

    try {
        for (const auto &entry : fs::directory_iterator(root)) {
            if (!entry.is_directory()) {
                continue;
            }
            const std::string name = entry.path().filename().string();
            if (name.size() < 4 || name.compare(0, 4, "job-") != 0) {
                continue;
            }
            std::error_code rmEc;
            fs::remove_all(entry.path(), rmEc);
            if (rmEc) {
                kmlogger.warn("cleanupStaleInstallTempRepos: failed to remove %s: %s", entry.path().c_str(), rmEc.message().c_str());
            } else {
                kmlogger.info("cleanupStaleInstallTempRepos: removed %s", entry.path().c_str());
            }
        }
    } catch (const fs::filesystem_error &e) {
        kmlogger.warn("cleanupStaleInstallTempRepos: %s", e.what());
    }
}

}

KMSystemServerAdaptor::KMSystemServerAdaptor()
{
	cleanupStaleInstallTempRepos();
	getPackageManager()->processDeferredOldRefsCleanupOnBootIfNeeded();
	getPackageManager()->refreshSummaryCache();
}

KMSystemServerAdaptor::~KMSystemServerAdaptor()
{
    // 确保先 unexport，再释放
    if (m_service) {
        g_dbus_interface_skeleton_unexport(
            G_DBUS_INTERFACE_SKELETON(m_service));
        g_object_unref(m_service);
        m_service = nullptr;
    }
}

void KMSystemServerAdaptor::setService(KaimingInterface *service)
{
    if (m_service) {
        g_object_unref(m_service);
    }
    m_service = service;
    if (m_service) {
        g_object_ref(m_service);  // 增加引用计数，确保不被销毁
    }
}

KMPackageManager *KMSystemServerAdaptor::getPackageManager()
{
    if (!m_pkgManager)
    {
        m_pkgManager = std::make_unique<KMPackageManager>();
    }
    return m_pkgManager.get();
}

KMInfoCommand *KMSystemServerAdaptor::getKMInfoCommand()
{
    if (!m_kmLocalInfo)
    {
        m_kmLocalInfo = std::make_unique<KMInfoCommand>();
    }
    return m_kmLocalInfo.get();
}

KMList *KMSystemServerAdaptor::getKMList()
{
    if (!m_kmList)
    {
        m_kmList = std::make_unique<KMList>();
    }
    return m_kmList.get();
}

bool KMSystemServerAdaptor::validateInputName(const gchar *name)
{
    return name && *name;
}

bool KMSystemServerAdaptor::checkOperationRunning(std::atomic<bool> &flag)
{
    bool expected = false;
    return !flag.compare_exchange_strong(expected, true);
}

// 保留原有接口，添加默认限速参数
gboolean KMSystemServerAdaptor::install(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch)
{
    return installWithSpeedLimit(object, invocation, name, branch, 0);
}

// 新的重载方法，支持限速参数
gboolean KMSystemServerAdaptor::installWithSpeedLimit(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch, guint maxSpeedLimitKilobytes)
{
    kmlogger.info("begin call install api, m_installRunningFlag = %d, maxSpeedLimitKilobytes = %u", m_installRunningFlag.load(), maxSpeedLimitKilobytes);

    if (!validateInputName(name))
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Package name must be specified");
        return TRUE;
    }

    // 创建回调函数，负责发送信号
    // 注意：libostree 的进度回调可能在任意线程中执行，需要使用 g_idle_add_full 切换到主线程
    // 使用全局、稳定的 service 对象（在 on_bus_acquired 中创建）
    KaimingInterface *service = getService();
    if (!service)
    {
        kmlogger.error("Service object not available, cannot send progress signal");
        kaiming_interface_complete_install(object, invocation, false);
        return TRUE;
    }

    // 创建download进度回调
    auto downloadCallback = [service](const std::string &progressJson)
    {
        // 复制字符串数据（需要在主线程中使用）
        char *json = g_strdup(progressJson.c_str());

        // 使用模板函数切换到主线程并发送信号
        postSignalToMainThread(service, [json](KaimingInterface *svc)
                               {
            if (svc && json) {
                kmlogger.info("Sending download progress signal from main thread: %s", json);
                g_signal_emit_by_name(svc, "progresschangesignal", json);
            }

            if (json) {
                g_free(json);
            } });
    };

    // 创建deploy回调
    auto deployCallback = [service](const std::string &progressJson)
    {
        char *json = g_strdup(progressJson.c_str());

        postSignalToMainThread(service, [json](KaimingInterface *svc)
                               {
            if (svc && json) {
                kmlogger.info("Sending deploy progress signal from main thread: %s", json);
                g_signal_emit_by_name(svc, "deployprogresssignal", json);
            }

            if (json) {
                g_free(json);
            } });
    };

    // 创建完成回调，用于在任务完成时发送finishsignal信号
    auto completionCallback = [service, name](const std::string &refId, int operateType, int errorCode, const std::string &errorMessage)
    {
        char *refIdCopy = g_strdup(refId.c_str());
        char *errorMessageCopy = g_strdup(errorMessage.c_str());

        postSignalToMainThread(service, [refIdCopy, operateType, errorCode, errorMessageCopy](KaimingInterface *svc)
                               {
            if (svc && refIdCopy && errorMessageCopy) {
                kmlogger.info("Sending finish signal from main thread: refId=%s, operateType=%d, errorCode=%d, errorMessage=%s", refIdCopy, operateType, errorCode, errorMessageCopy);
                g_signal_emit_by_name(svc, "finishsignal", refIdCopy, operateType, errorCode, errorMessageCopy);
            }

            if (refIdCopy) {
                g_free(refIdCopy);
            }

            if (errorMessageCopy) {
                g_free(errorMessageCopy);
            } });
    };

    std::vector<std::string> vectorIds{name};
    PackageManagerOptions pmOption;

    // 调用异步安装函数，立即返回 taskId（不阻塞）
    std::string taskId = getPackageManager()->installSoftwareCenter(vectorIds, branch ? branch : "", downloadCallback, deployCallback, pmOption, maxSpeedLimitKilobytes, completionCallback);

    kmlogger.info("Install task enqueued with taskId: %s", taskId.c_str());

    // 立即返回 taskId（通过 DBus 返回值或信号）
    g_signal_emit_by_name(object, "taskstartedsignal", taskId.c_str(), name);

    // 返回成功（任务已加入队列）
    kaiming_interface_complete_install(object, invocation, true);
    return TRUE;
}

// 批量安装接口
gboolean KMSystemServerAdaptor::deployBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch)
{
    // 计算数组长度
    int names_count = 0;
    while (names && names[names_count])
    {
        names_count++;
    }

    kmlogger.info("begin call deployBatch api, m_deployRunningFlag = %d, names count = %d", m_deployRunningFlag.load(), names_count);

    if (checkOperationRunning(m_deployRunningFlag))
    {
        kaiming_interface_complete_deploy_batch(object, invocation, false);
        return TRUE;
    }

    m_deployRunningFlag.store(TRUE);

    // 验证输入
    if (names_count == 0)
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "No package names specified");
        return TRUE;
    }

    for (int i = 0; i < names_count; i++)
    {
        if (!validateInputName(names[i]))
        {
            g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Invalid package name: %s", names[i]);
            return TRUE;
        }
    }

    const gchar *name = names[0];

    // 创建deploy进度回调
    std::function<void(const std::string &)> deployCallback = [object](const std::string &progressJson)
    {
        kmlogger.info("deploy progress json: %s", progressJson.c_str());
        g_signal_emit_by_name(object, "deployprogresssignal", progressJson.c_str());
    };

    // 准备参数
    std::vector<std::string> packageNames;
    for (int i = 0; i < names_count; i++)
    {
        packageNames.push_back(names[i]);
    }
    
    PackageManagerOptions pmOptions;
    pmOptions.isDeployOnly = true;
    
    int ret = getPackageManager()->deploySoftwareCenter(packageNames, pmOptions, deployCallback);

    g_signal_emit_by_name(object, "finishsignal", name, KM_DEPLOY_OPERATION, ret, KMErrorMsg::getErrorMessage(ret).c_str());
    m_deployRunningFlag.store(FALSE);
    
    kmlogger.info("end emit deploy signal, m_deployRunningFlag = %d, result is %s", m_deployRunningFlag.load(), KMErrorMsg::getErrorMessage(ret).c_str());

    // 返回结果
    kaiming_interface_complete_deploy_batch(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

// 批量下载接口
gboolean KMSystemServerAdaptor::downloadBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch, guint maxSpeedLimitKilobytes)
{
    kmlogger.info("begin call downloadOKPackage api, m_downloadOKRunningFlag = %d", m_downloadRunningFlag.load());
    
    // 将 const gchar** 转换为 vector<string>
    std::vector<std::string> packageNames;
    for (int i = 0; names[i] != NULL; i++) 
    {
        if (names[i] == NULL || strlen(names[i]) == 0)
        {
            kmlogger.error("package name at index %d is empty", i);
        }
        if (!validateInputName(names[i]))
        {
            g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Package name must be specified");
            return TRUE;
        }
        packageNames.push_back(std::string(names[i]));
        kmlogger.info("package[%d]: %s", i, names[i]);
    }

    // 创建回调函数，负责发送信号
    KaimingInterface *service = getService();
    if (!service)
    {
        kmlogger.error("Service object not available, cannot send progress signal");
        kaiming_interface_complete_download_batch(object, invocation, false);
        return TRUE;
    }

    // 创建download进度回调
    auto downloadCallback = [service](const std::string &progressJson)
    {
        char *json = g_strdup(progressJson.c_str());

        postSignalToMainThread(service, [json](KaimingInterface *svc)
                                {
            if (svc && json) {
                kmlogger.info("Sending download progress signal from main thread: data=%s", json);
                g_signal_emit_by_name(svc, "progresschangesignal", json);
            }

            if (json) {
                g_free(json);
            } });
    };

    // deploy回调为空函数（不需要deploy进度）
    auto deployCallback = [](const std::string &progressJson)
    {
        kmlogger.debug("Deploy progress ignored for download only operation: %s", progressJson.c_str());
    };

    // 创建完成回调
    auto completionCallback = [service](const std::string &refId, int operateType,
                                                    int errorCode, const std::string &errorMessage)
    {
        char *refIdCopy = g_strdup(refId.c_str());
        char *errorMessageCopy = g_strdup(errorMessage.c_str());

        postSignalToMainThread(service, [refIdCopy, operateType, errorCode, errorMessageCopy](KaimingInterface *svc)
                                {
            if (svc && refIdCopy && errorMessageCopy) {
                kmlogger.info("Sending finish signal from main thread: refId=%s, operateType=%d, errorCode=%d, errorMessage=%s", 
                                refIdCopy, operateType, errorCode, errorMessageCopy);
                g_signal_emit_by_name(svc, "finishsignal", refIdCopy, operateType, errorCode, errorMessageCopy);
            }

            if (refIdCopy) {
                g_free(refIdCopy);
            }

            if (errorMessageCopy) {
                g_free(errorMessageCopy);
            } });
    };

    PackageManagerOptions pmOptions;
    pmOptions.isDownloadOnly = true;

    kmlogger.info("downloading packages");

    // 调用异步安装函数，立即返回 taskId（不阻塞）
    std::string taskId = getPackageManager()->installSoftwareCenter(packageNames, branch ? branch : "", downloadCallback, deployCallback, pmOptions, maxSpeedLimitKilobytes, completionCallback);

    kmlogger.info("DownloadOnly task enqueued with taskId: %s", taskId.c_str());

    // 立即返回 taskId（通过 DBus 返回值或信号）
    g_signal_emit_by_name(object, "taskstartedsignal", taskId.c_str(), packageNames[0].c_str());

    kaiming_interface_complete_download_batch(object, invocation, TRUE);
    return TRUE;
}

gboolean KMSystemServerAdaptor::installOfflinePackage(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch)
{
    kmlogger.info("begin call install offline package api, m_installRunningFlag = %d", m_installRunningFlag.load());

    if (!validateInputName(name))
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Package name must be specified");
        return TRUE;
    }

    if (checkOperationRunning(m_installRunningFlag))
    {
        kaiming_interface_complete_install_offline_package(object, invocation, false);
        return TRUE;
    }

    m_installRunningFlag.store(TRUE);

    // 下载阶段回调
    auto downloadCallback = [this, object](const std::string &progressJson)
    {
        kmlogger.info("download progress json: %s", progressJson.c_str());
        g_signal_emit_by_name(object, "progresschangesignal", progressJson.c_str());
    };

    // 部署阶段回调
    auto deployCallback = [this, object](const std::string &progressJson)
    {
        kmlogger.info("deploy progress json: %s", progressJson.c_str());
        g_signal_emit_by_name(object, "deployprogresssignal", progressJson.c_str());
    };

    std::vector<std::string> vectorIds;
    vectorIds.push_back(name);

    PackageManagerOptions pmOptions;
    pmOptions.assumeYes = true;
    pmOptions.skipSignatureVerification = false;

    int ret = getPackageManager()->installOfflinePackage(vectorIds, pmOptions, downloadCallback, deployCallback);
    std::string errorInfo = KMErrorMsg::getErrorMessage(ret);

    g_signal_emit_by_name(object, "finishsignal", name, KM_INSTALL_OPERATION, ret, errorInfo.c_str());
    m_installRunningFlag.store(FALSE);
    kmlogger.info("end emit install offline package signal, errorInfo = %s, m_installRunningFlag = %d", errorInfo.c_str(), m_installRunningFlag.load());

    kaiming_interface_complete_install_offline_package(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::update(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch)
{
    kmlogger.info("begin call update api, m_updateRunningFlag = %d", m_updateRunningFlag.load());

    if (checkOperationRunning(m_updateRunningFlag))
    {
        kaiming_interface_complete_install(object, invocation, false);
        return TRUE;
    }

    m_updateRunningFlag.store(TRUE);

    std::function<void(const std::string &)> updatecallback = [this, object](const std::string &progressJson)
    {
        kmlogger.info("download progress json: %s", progressJson.c_str());
        g_signal_emit_by_name(object, "progresschangesignal", progressJson.c_str());
    };

    std::function<void(const std::string &)> deploycallback = [this, object](const std::string &progressJson)
    {
        kmlogger.info("deploy progress json: %s", progressJson.c_str());
        g_signal_emit_by_name(object, "deployprogresssignal", progressJson.c_str());
    };

    int ret = getPackageManager()->updateSoftwareCenter(name, branch, updatecallback, deploycallback);

    g_signal_emit_by_name(object, "finishsignal", name, KM_UPDATE_OPERATION, ret, KMErrorMsg::getErrorMessage(ret).c_str());
    m_updateRunningFlag.store(FALSE);
    kmlogger.info("end emit update signal, m_updateRunningFlag = %d", m_updateRunningFlag.load());

    kaiming_interface_complete_update(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::uninstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *branch)
{
    kmlogger.info("begin call uninstall api, m_uninstallRunningFlag = %d", m_uninstallRunningFlag.load());

    if (checkOperationRunning(m_uninstallRunningFlag))
    {
        kaiming_interface_complete_install(object, invocation, false);
        return TRUE;
    }

    m_uninstallRunningFlag.store(TRUE);
    int ret = getPackageManager()->uninstallSoftwareCenter(name);

    g_signal_emit_by_name(object, "finishsignal", name, KM_UNINSTALL_OPERATION, ret, KMErrorMsg::getErrorMessage(ret).c_str());
    m_uninstallRunningFlag.store(FALSE);
    kmlogger.info("end emit uninstall signal, m_uninstallRunningFlag = %d", m_uninstallRunningFlag.load());

    kaiming_interface_complete_uninstall(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::cancelInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskId)
{
    kmlogger.info("begin call cancelInstall api, taskIdOrName: %s", taskId ? taskId : "null");

    if (!taskId || !*taskId) {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Task ID or package name must be specified");
        return TRUE;
    }

    // cancelInstall 本身是轻量级的（只是设置标志和调用 cancel）
    // 如果其他操作正在持有锁，这里会短暂等待，但操作本身很快
    bool success = getPackageManager()->cancelInstall(std::string(taskId));
    kmlogger.info("cancelInstall result for %s: %s", taskId, success ? "success" : "failed");

    kaiming_interface_complete_cancel_install(object, invocation, success);
    return TRUE;
}

gboolean KMSystemServerAdaptor::pauseInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName)
{
    kmlogger.info("begin call pauseInstall api, taskIdOrName: %s", taskIdOrName ? taskIdOrName : "null");

    if (!taskIdOrName || !*taskIdOrName) {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Task ID or package name must be specified");
        return TRUE;
    }

    bool success = getPackageManager()->pauseInstall(std::string(taskIdOrName));
    kmlogger.info("pauseInstall result for %s: %s", taskIdOrName, success ? "success" : "failed");

    kaiming_interface_complete_pause_install(object, invocation, success);
    return TRUE;
}

gboolean KMSystemServerAdaptor::resumeInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName)
{
    kmlogger.info("begin call resumeInstall api, taskIdOrName: %s", taskIdOrName ? taskIdOrName : "null");

    if (!taskIdOrName || !*taskIdOrName) {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Task ID or package name must be specified");
        return TRUE;
    }

    bool success = getPackageManager()->resumeInstall(std::string(taskIdOrName));
    kmlogger.info("resumeInstall result for %s: %s", taskIdOrName, success ? "success" : "failed");

    kaiming_interface_complete_resume_install(object, invocation, success);
    return TRUE;
}

gboolean KMSystemServerAdaptor::handlePauseInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->pauseInstall(object, invocation, taskIdOrName);
}

gboolean KMSystemServerAdaptor::handleResumeInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskIdOrName, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->resumeInstall(object, invocation, taskIdOrName);
}

gboolean KMSystemServerAdaptor::getLocalInfomation(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call getLocalInfo api");

    int errorCode;
    string localInfoJson = getKMInfoCommand()->getLocalInfo(name, errorCode);

    kmlogger.info("before emit getLocalInfo signal");
    g_signal_emit_by_name(object, "getlocalinfosignal", name, localInfoJson.c_str(), errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getLocalInfo signal");

    // 创建一个 GVariant 来封装字符串
    GVariant *result = g_variant_new("(s)", localInfoJson.c_str());

    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getLocalDebInfo(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call getLocalDebInfo api");

    int errorCode;
    string localDebInfoJson = getKMInfoCommand()->getLocalDebInfo(name, errorCode);

    kmlogger.info("before emit getLocalDebInfo signal");
    g_signal_emit_by_name(object, "getlocaldebinfosignal", name, localDebInfoJson.c_str(), errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getLocalDebInfo signal");

    // 创建一个 GVariant 来封装字符串
    GVariant *result = g_variant_new("(s)", localDebInfoJson.c_str());

    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getDebInfoFromOK(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call getDebInfoFromOK api");

    int errorCode = 0;
    string OKDebInfoJson = getPackageManager()->getDebInfoFromOK(name, errorCode);

    kmlogger.info("before emit getDebInfoFromOK signal");
    g_signal_emit_by_name(object, "getdebinfofromoksignal", name, OKDebInfoJson.c_str(), errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getdebinfofromok signal");

    // 创建一个 GVariant 来封装字符串
    GVariant *result = g_variant_new("(s)", OKDebInfoJson.c_str());

    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getAllLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation)
{
    kmlogger.info("begin call getAllLocalInfo api");

    std::vector<std::string> localInfoList = getKMList()->getAllLocalInfo();

    // for (const auto &item : localInfoList) {
    //     kmlogger.info("%s", item.c_str());
    // }

    kmlogger.info("before emit getAllLocalInfo signal");
    int errorCode = 0;
    // 转换 std::vector<std::string> 为 GStrv（信号与返回值都用这批数据）
    gchar **strv = g_new0(gchar *, localInfoList.size() + 1);
    for (size_t i = 0; i < localInfoList.size(); ++i)
    {
        strv[i] = g_strdup(localInfoList[i].c_str());
    }
    // 发送 getalllocalinfosignal 信号
    g_signal_emit_by_name(object, "getalllocalinfosignal", "", strv, errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getalllocalinfosignal signal");

    // 使用 g_variant_new_strv 生成数组，并尽早释放 GStrv
    GVariant *array_variant = g_variant_new_strv((const gchar * const *)strv, (gssize)localInfoList.size());
    g_strfreev(strv);

    // 用完源数据后，主动释放局部容器内存
    std::vector<std::string>().swap(localInfoList);
    malloc_trim(0);

    // 打包为元组返回（零拷贝封装已有变体）
    GVariant *result = g_variant_new("(@as)", array_variant);

    // 返回给调用方
    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

/*
     * 软件商店通过直接返回字符串中branch是否为空判断查询结果是否正确，而不是通过发出的信号
     *
     * branch为空： 返回信息失败
     * branch不为空： 返回信息正确后，另外商店也能获取到发布平台发布的应用信息，商店就可以正确显示该应用
     */
gboolean KMSystemServerAdaptor::getRemoteInfomation(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call getRemoteInfo api, ID: %s", name);

    string remoteInfoJson;
    std::vector<std::string> allRemoteInfo;
    std::string remoteKind = "";
    bool isNameNullable = false;
    int errorCode = getPackageManager()->getRemoteInfo(name, remoteInfoJson, allRemoteInfo, remoteKind, isNameNullable);

    kmlogger.info("before emit getRemoteInfo signal");
    // 发送 getremoteinfosignal 信号
    g_signal_emit_by_name(object, "getremoteinfosignal", name, remoteInfoJson.c_str(), errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getRemoteInfo signal");

    // 创建一个 GVariant 来封装字符串
    GVariant *result = g_variant_new("(s)", remoteInfoJson.c_str());

    // 返回结果给调用方
    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getRemotePackageStatus(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name, const gchar *version, const gchar *branch)
{
    kmlogger.info("begin call getRemotePackageStatus api, ID: %s, version is %s, branch is %s", name, version, branch);

    std::vector<std::string> completeRef;
    if (std::string(branch).empty())
    {
        branch = "stable";
    }
    int errorCode;
    std::string remoteStatusInfo;
    errorCode = getPackageManager()->getRemoteStatusInfo(name, version, branch, remoteStatusInfo, errorCode);

    kmlogger.info("before emit getRemotePackageStatus signal");
    // 发送 getremotepackagestatussignal 信号
    g_signal_emit_by_name(object, "getremotepackagestatussignal", name, remoteStatusInfo.c_str(), errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getRemotePackageStatus signal");

    // 创建一个 GVariant 来封装字符串
    GVariant *result = g_variant_new("(s)", remoteStatusInfo.c_str());

    // 返回结果给调用方
    g_dbus_method_invocation_return_value(invocation, result);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getAllRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation)
{
    kmlogger.info("begin call getAllRemoteInfo api");

    string remoteInfoJson;
    std::vector<std::string> allRemoteInfo;
    int errorCode;
    std::string remoteKind = "";
    bool isNameNullable = true;
    try
    {   
        errorCode = getPackageManager()->getRemoteInfo("", remoteInfoJson, allRemoteInfo, remoteKind, isNameNullable);
    }
    catch (const std::exception &e)
    {
        kmlogger.info("getAllRemoteInfo error: %s", e.what());
    }

    kmlogger.info("before emit getAllRemoteInfo signal");

    // 转换 std::vector<std::string> 为 GStrv（信号与返回值都用这批数据）
    gchar **strv = g_new0(gchar *, allRemoteInfo.size() + 1);
    for (size_t i = 0; i < allRemoteInfo.size(); ++i)
    {
        strv[i] = g_strdup(allRemoteInfo[i].c_str());
    }
    // 发送 getallremoteinfosignal 信号
    g_signal_emit_by_name(object, "getallremoteinfosignal", "", strv, errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getallremoteinfosignal signal");

    // 使用 g_variant_new_strv 生成数组，并尽早释放 GStrv
    GVariant *array_variant = g_variant_new_strv((const gchar * const *)strv, (gssize)allRemoteInfo.size());
    g_strfreev(strv);

    // 用完源数据后，主动释放局部容器内存
    std::vector<std::string>().swap(allRemoteInfo);
    malloc_trim(0);

    // 封装返回值并返回给调用方
    GVariant *result = g_variant_new("(@as)", array_variant);
    g_dbus_method_invocation_return_value(invocation, result);

    return TRUE;
}

gboolean KMSystemServerAdaptor::getAPPsRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation)
{
    kmlogger.info("begin call getAPPsRemoteInfo api");

    string remoteInfoJson;
    std::vector<std::string> allRemoteInfo;
    std::string remoteKind = "app";
    int errorCode;
    bool isNameNullable = true;
    try
    {   
        errorCode = getPackageManager()->getRemoteInfo("", remoteInfoJson, allRemoteInfo, remoteKind, isNameNullable);
    }
    catch (const std::exception &e)
    {
        kmlogger.info("getAPPsRemoteInfo error: %s", e.what());
    }

    kmlogger.info("before emit getAPPsRemoteInfo signal");

    // 转换 std::vector<std::string> 为 GStrv（信号与返回值都用这批数据）
    gchar **strv = g_new0(gchar *, allRemoteInfo.size() + 1);
    for (size_t i = 0; i < allRemoteInfo.size(); ++i)
    {
        strv[i] = g_strdup(allRemoteInfo[i].c_str());
    }
    // 发送 getappsremoteinfosignal 信号
    g_signal_emit_by_name(object, "getappsremoteinfosignal", "", strv, errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getappsremoteinfosignal signal");

    // 使用 g_variant_new_strv 生成数组，并尽早释放 GStrv
    GVariant *array_variant = g_variant_new_strv((const gchar * const *)strv, (gssize)allRemoteInfo.size());
    g_strfreev(strv);

    // 用完源数据后，主动释放局部容器内存
    std::vector<std::string>().swap(allRemoteInfo);
    malloc_trim(0);

    // 封装返回值并返回给调用方
    GVariant *result = g_variant_new("(@as)", array_variant);
    g_dbus_method_invocation_return_value(invocation, result);

    return TRUE;
}

gboolean KMSystemServerAdaptor::isInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call isInstalled api, name = %s", name);

    if (!validateInputName(name))
    {
        kmlogger.error("Package name must be specified");
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Package name must be specified");
        return TRUE;
    }

    int ret = getPackageManager()->isInstalled(name);
    g_signal_emit_by_name(object, "isinstalledsignal", name, ret, 0, KMErrorMsg::getErrorMessage(ret).c_str());
    kmlogger.info("end emit isInstalled signal");

    kaiming_interface_complete_is_installed(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getAllIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names)
{
    kmlogger.info("begin call getAllIsInstalled api");
    
    if (!names) 
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Input name list is null");
        return TRUE;
    }

    std::vector<std::string> pkgNames;
    int length = 0;
    for (const gchar **p = names; *p; ++p) 
    {
        if (validateInputName(*p))
        {
            pkgNames.emplace_back(*p);
            length++;
        }
    }

    if (pkgNames.empty()) 
    {
        g_dbus_method_invocation_return_error( invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Empty input name list");
        return TRUE;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
    
    for (int i = 0; i < length; i++) 
    {   
        const gchar *packageName = pkgNames[i].c_str();
        int ret = getPackageManager()->isInstalled(packageName);
        gboolean isInstalled = (ret == KM_ERROR_NO);
        
        GVariant *entry = g_variant_new("(sb)", packageName, isInstalled);
        g_variant_builder_add_value(&builder, entry);
        
        GVariantBuilder singleBuilder;
        g_variant_builder_init(&singleBuilder, G_VARIANT_TYPE_ARRAY);
        g_variant_builder_add_value(&singleBuilder, entry);
        GVariant *singleResult = g_variant_builder_end(&singleBuilder);
        
        g_signal_emit_by_name(object, "getallisinstalledsignal", packageName, singleResult, ret, KMErrorMsg::getErrorMessage(ret).c_str());
    }
    
    GVariant *result = g_variant_builder_end(&builder);
    
    kmlogger.info("end emit getallisinstalledsignal for all packages");

    kaiming_interface_complete_get_all_is_installed(object, invocation, result);
    return TRUE;
}


gboolean KMSystemServerAdaptor::isUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *name)
{
    kmlogger.info("begin call isUpgradable api, name = %s", name);

    if (!validateInputName(name))
    {
        kmlogger.error("Package name must be specified");
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Package name must be specified");
        return TRUE;
    }

    int ret = getPackageManager()->isUpgradabled(name);
    g_signal_emit_by_name(object, "isupgradablesignal", name, ret, 0, KMErrorMsg::getErrorMessage(ret).c_str());
    kmlogger.info("end emit isUpgradable signal");

    kaiming_interface_complete_is_upgradable(object, invocation, ret == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getAllUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names)
{
    kmlogger.info("begin call getAllUpgradable api");

    if (!names) {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Input name list is null");
        return TRUE;
    }

    std::vector<std::string> pkgNames;
    for (const gchar **p = names; *p; ++p) {
        if (validateInputName(*p))
            pkgNames.emplace_back(*p);
    }

    if (pkgNames.empty()) {
        g_dbus_method_invocation_return_error( invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Empty input name list");
        return TRUE;
    }

    std::vector<std::pair<std::string, bool>> results;
    int errorCode = KM_ERROR_NO;

    try {
        errorCode = getPackageManager()->getAllUpgradableStatus(pkgNames, results);
    } catch (const std::exception &e) {
        kmlogger.error("Exception in getAllUpgradable: %s", e.what());
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Exception: %s", e.what());
        return TRUE;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sb)"));
    for (const auto &item : results)
    {
        g_variant_builder_add(&builder, "(sb)", item.first.c_str(), item.second);
    }
    GVariant *array_variant = g_variant_builder_end(&builder);

    if (!array_variant)
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Failed to build result variant");
        return TRUE;
    }

    g_signal_emit_by_name(object, "getallupgradablesignal", "", array_variant, errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());

    // 返回结果
    kaiming_interface_complete_get_all_upgradable(object, invocation, array_variant);

    gchar *variant_str = g_variant_print(array_variant, TRUE);
    kmlogger.info("Return array_variant: %s", variant_str);
    g_free(variant_str);

    kmlogger.info("getAllUpgradable completed, total=%zu, errorCode=%d", results.size(), errorCode);
    return TRUE;
}

gboolean KMSystemServerAdaptor::getRemoteSummary(KaimingInterface *object, GDBusMethodInvocation *invocation)
{
    kmlogger.info("begin call getRemoteSummary api");

    KMFolder folder;
    folder.ensureRepoForSudo(FALSE);

    // 下载和解析清单依赖文件 summary.json
    // string ostreeRepoUrl = m_pkgManager->getRemoteUrl(REPO_NAME);
    // std::string summaryUrl = ostreeRepoUrl.append("/").append("summary.json");

    // fs::path basePath = folder.getPath();
    // fs::path summaryJsonPath = basePath / "summary.json";
    // bool isSummaryJsonExpired = KMFileUtils::isFileExpired(summaryJsonPath, 3600);
    int errorCode = getPackageManager()->downloadAllRemoteSummaries();

    // 发送 getremotesummarysignal 信号
    g_signal_emit_by_name(object, "getremotesummarysignal", "summary.json", errorCode, KMErrorMsg::getErrorMessage(errorCode).c_str());
    kmlogger.info("end emit getRemoteSummary signal");

    // 返回生成的信息字符串
    kaiming_interface_complete_is_upgradable(object, invocation, errorCode == KM_ERROR_NO);
    return TRUE;
}

gboolean KMSystemServerAdaptor::checkAndCleanupUninstalledDrivers(KaimingInterface *object, GDBusMethodInvocation *invocation)
{
    try
    {
        KMFolder folder(KAIMING_SYSTEM_PATH, false);
        KMHostAppDeploy deploy(folder.getInfoPath());
        int cleaned = deploy.checkAndCleanupUninstalledDrivers();
        // 返回清理数量
        kaiming_interface_complete_check_and_cleanup_uninstalled_drivers(object, invocation, cleaned);
        return TRUE;
    }
    catch (const std::exception &e)
    {
        g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "cleanup failed: %s", e.what());
        return TRUE;
    }
}

// 静态适配器方法，用于连接 GDBus 信号   KaimingInterface需与xml自动生成的.c中的一致
gboolean KMSystemServerAdaptor::handleInstall(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                              const gchar *name, const gchar *branch, gpointer user_data)
{
    // 将 user_data 转换为 KMSystemServerAdaptor 实例
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->install(object, invocation, name, branch);
}

gboolean KMSystemServerAdaptor::handleInstallOfflinePackage(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                            const gchar *name, const gchar *branch, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->installOfflinePackage(object, invocation, name, branch);
}

gboolean KMSystemServerAdaptor::handleInstallWithSpeedLimit(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                           const gchar *name, const gchar *branch, 
                                                           guint maxSpeedLimitKilobytes, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->installWithSpeedLimit(object, invocation, name, branch, maxSpeedLimitKilobytes);
}

gboolean KMSystemServerAdaptor::handleDownloadBatch(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                   const gchar **names, const gchar *branch,
                                                   guint maxSpeedLimitKilobytes, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->downloadBatch(object, invocation, names, branch, maxSpeedLimitKilobytes);
}

gboolean KMSystemServerAdaptor::handleDeployBatch(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar **names, const gchar *branch, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->deployBatch(object, invocation, names, branch);
}

gboolean KMSystemServerAdaptor::handleUpdate(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                             const gchar *name, const gchar *branch, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->update(object, invocation, name, branch);
}

gboolean KMSystemServerAdaptor::handleUninstall(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                const gchar *name, const gchar *branch, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->uninstall(object, invocation, name, branch);
}

gboolean KMSystemServerAdaptor::handleGetLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                   const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getLocalInfomation(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetLocalDebInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                      const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getLocalDebInfo(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetDebInfoFromOK(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                      const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getDebInfoFromOK(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetAllLocalInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                      gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getAllLocalInfo(object, invocation);
}

gboolean KMSystemServerAdaptor::handleGetAllRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                       gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getAllRemoteInfo(object, invocation);
}

gboolean KMSystemServerAdaptor::handleGetAPPsRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                        gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getAPPsRemoteInfo(object, invocation);
}

gboolean KMSystemServerAdaptor::handleGetRemoteInfo(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                    const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getRemoteInfomation(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetRemotePackageStatus(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                    const gchar *name, const gchar *version, const gchar *branch, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getRemotePackageStatus(object, invocation, name, version, branch);
}

gboolean KMSystemServerAdaptor::handleIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                  const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->isInstalled(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetAllIsInstalled(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                  const gchar **names, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getAllIsInstalled(object, invocation, names);
}

gboolean KMSystemServerAdaptor::handleIsUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                   const gchar *name, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->isUpgradable(object, invocation, name);
}

gboolean KMSystemServerAdaptor::handleGetAllUpgradable(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                      const gchar **names, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getAllUpgradable(object, invocation, names);
}

gboolean KMSystemServerAdaptor::handleGetRemoteSummary(KaimingInterface *object, GDBusMethodInvocation *invocation,
                                                       gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->getRemoteSummary(object, invocation);
}

gboolean KMSystemServerAdaptor::handleCheckAndCleanupUninstalledDrivers(KaimingInterface *object, GDBusMethodInvocation *invocation, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->checkAndCleanupUninstalledDrivers(object, invocation);
}

gboolean KMSystemServerAdaptor::handleCancelInstall(KaimingInterface *object, GDBusMethodInvocation *invocation, const gchar *taskId, gpointer user_data)
{
    KMSystemServerAdaptor *instance = static_cast<KMSystemServerAdaptor *>(user_data);
    return instance->cancelInstall(object, invocation, taskId);
}

// 当总线获取成功时的回调
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    GError *error = NULL;
    // 获取 KMSystemServerAdaptor 实例
    KMSystemServerAdaptor *handler = static_cast<KMSystemServerAdaptor *>(user_data);
    // 创建一个新的 Service 对象  需与xml自动生成的.c中的一致
    KaimingInterface *service = kaiming_interface_skeleton_new();
    // 将 Service 对象的方法调用与 KMSystemServerAdaptor 的方法连接起来   "handle-install-offline-package"需与xml自动生成的.c中的一致
    g_signal_connect(service, "handle-install", G_CALLBACK(KMSystemServerAdaptor::handleInstall), handler);
    g_signal_connect(service, "handle-install-offline-package", G_CALLBACK(KMSystemServerAdaptor::handleInstallOfflinePackage), handler);
    g_signal_connect(service, "handle-install-with-speed-limit", G_CALLBACK(KMSystemServerAdaptor::handleInstallWithSpeedLimit), handler);
    g_signal_connect(service, "handle-download-batch", G_CALLBACK(KMSystemServerAdaptor::handleDownloadBatch), handler);
    g_signal_connect(service, "handle-deploy-batch", G_CALLBACK(KMSystemServerAdaptor::handleDeployBatch), handler);
    g_signal_connect(service, "handle-update", G_CALLBACK(KMSystemServerAdaptor::handleUpdate), handler);
    g_signal_connect(service, "handle-uninstall", G_CALLBACK(KMSystemServerAdaptor::handleUninstall), handler);
    g_signal_connect(service, "handle-get-local-info", G_CALLBACK(KMSystemServerAdaptor::handleGetLocalInfo), handler);
    g_signal_connect(service, "handle-get-local-deb-info", G_CALLBACK(KMSystemServerAdaptor::handleGetLocalDebInfo), handler);
    g_signal_connect(service, "handle-get-deb-info-from-ok", G_CALLBACK(KMSystemServerAdaptor::handleGetDebInfoFromOK), handler);
    g_signal_connect(service, "handle_get_all_local_info", G_CALLBACK(KMSystemServerAdaptor::handleGetAllLocalInfo), handler);
    g_signal_connect(service, "handle_get_all_remote_info", G_CALLBACK(KMSystemServerAdaptor::handleGetAllRemoteInfo), handler);
    g_signal_connect(service, "handle_get_apps_remote_info", G_CALLBACK(KMSystemServerAdaptor::handleGetAPPsRemoteInfo), handler);
    g_signal_connect(service, "handle-get-remote-info", G_CALLBACK(KMSystemServerAdaptor::handleGetRemoteInfo), handler);
    g_signal_connect(service, "handle-get-remote-package-status", G_CALLBACK(KMSystemServerAdaptor::handleGetRemotePackageStatus), handler);
    g_signal_connect(service, "handle-is-installed", G_CALLBACK(KMSystemServerAdaptor::handleIsInstalled), handler);
    g_signal_connect(service, "handle-get-all-is-installed", G_CALLBACK(KMSystemServerAdaptor::handleGetAllIsInstalled), handler);
    g_signal_connect(service, "handle-is-upgradable", G_CALLBACK(KMSystemServerAdaptor::handleIsUpgradable), handler);
    g_signal_connect(service, "handle-get-all-upgradable", G_CALLBACK(KMSystemServerAdaptor::handleGetAllUpgradable), handler);
    g_signal_connect(service, "handle-get-remote-summary", G_CALLBACK(KMSystemServerAdaptor::handleGetRemoteSummary), handler);
    g_signal_connect(service, "handle-check-and-cleanup-uninstalled-drivers", G_CALLBACK(KMSystemServerAdaptor::handleCheckAndCleanupUninstalledDrivers), handler);
    g_signal_connect(service, "handle-cancel-install", G_CALLBACK(KMSystemServerAdaptor::handleCancelInstall), handler);
    g_signal_connect(service, "handle-pause-install", G_CALLBACK(KMSystemServerAdaptor::handlePauseInstall), handler);
    g_signal_connect(service, "handle-resume-install", G_CALLBACK(KMSystemServerAdaptor::handleResumeInstall), handler);

    // 将 Service 对象导出到 D-Bus 连接上
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(service), connection, KAIMING_DBUS_PATH, &error);
    if (error)
    {
        std::cerr << "Failed to export interface: " << error->message << std::endl;
        g_clear_error(&error);
        g_object_unref(service);
        return;
    }
    
    // 将 service 对象保存到 handler 中，确保在整个生命周期内都存在
    // 这样进度回调就可以安全地使用它来发送信号
    handler->setService(service);
    
    kmlogger.info("DBus service exported successfully, service object saved");
}

int main(int argc, char *argv[])
{
    // 检查 overlay 配置文件
    KMOverlay::checkOverlayConfig();

    // 获取系统上已注册的浏览器二进制文件
    KMBrowserUtils::getAllBrowserExecutables();

    // 创建主循环对象
    GMainLoop *loop;
    guint owner_id;

    // 创建 KMSystemServerAdaptor 实例
    KMSystemServerAdaptor handler;

    // 将 handler 作为 user_data 传递
    owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, KAIMING_DBUS_SERVICE, G_BUS_NAME_OWNER_FLAGS_NONE,
                              on_bus_acquired, NULL, NULL, &handler, NULL);

    // 创建主循环
    loop = g_main_loop_new(NULL, FALSE);

    kmlogger.info("org.kylin.kaiming service started successfully, waiting for D-Bus requests...");

    // 进入循环
    g_main_loop_run(loop);

    // 取消 D-Bus 名称的所有权 释放资源
    g_bus_unown_name(owner_id);
    // 释放 GMainLoop 对象
    g_main_loop_unref(loop);

    return 0;
}
