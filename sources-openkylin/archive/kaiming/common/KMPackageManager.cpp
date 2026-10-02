/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <thread>
#include <atomic>
#include <string>
#include <iostream>
#include <sstream>
#include <dlfcn.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <filesystem>
#include <ctime>
#include <fstream>

#include "KMPackageManager.h"
#include "common/KMFileLock.h"

#include "common/KMLogger.h"
#include "common/KMError.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMDbusJson.h"
#include "common/KMVersion.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMLibHandle.h"
#include "common/KMDirMd5sum.h"
#include "common/KMPluginSynchronizer.h"
#include "common/KMDirUtils.h"
#include "common/KMCommonUtils.h"
#include "common/KMJsonHelper.h"
#include "common/KMDeferredCleanupStore.h"
#include "app/list/KMList.h"

namespace fs = std::filesystem;

std::atomic<int> sharedValue(0);             // 定义一个原子类型的共享变量，初始值为0
constexpr int SUMMARY_EXPIRE_SECONDS = 3600; // the expire check time of summary

class KMPackageManager::Private
{
public:
    KMPackageManager* m_packageManager;  // 指向外部 KMPackageManager 的指针，用于调用成员函数

    std::unique_ptr<KMFolder> m_folder;
    std::unique_ptr<KMList> m_list;
    std::unique_ptr<KMUtil> m_util;
    std::unique_ptr<KMEnv> m_env;
    std::unique_ptr<KMParseSummaryJson> m_parseSummaryJson; // 解析 summary.json
    std::unique_ptr<KMFormatter> m_formatter;               // 格式化输出
    bool m_pmIsForceInstall = false;                        // 是否强制安装
    bool m_pmIsThereDbusService = true;                     // 是否有dbus安装卸载检测服务
    bool m_isHostapp = false;                               // 是否是hostapp
    bool m_isOffInstall = false;                            // 是否是离线安装
    int m_currentRefIndex = 0;
    int m_totalRefCount = 0;
    KMPMRef m_pmRef;                                        // 单独ref,用于存储到 d->m_refUserList中
    std::vector<KMPMRef> m_pmRefDownloadList;               // 实际上需要安装的软件列表
    std::vector<KMPMRef> m_pmRefOfflineList;                // 实际上需要安装的软件列表
    std::vector<std::string> m_pmFuzzyMatchRefs;            // 从依赖文件中模糊匹配到的 ref 列表
    std::vector<KMRef> m_pmQueryVector;                     // 本地指定信息安装列表
    std::vector<KMRef> m_pmUninstallVector;                 // 卸载列表
    std::vector<std::string> m_pmOldAVersion;               // 旧版本app
    std::vector<std::string> m_pmSpecialIdVersion;          // 特殊版本app
    std::vector<std::string> m_pmReserveDepend;             // 保留depend依赖
    std::map<std::string, KMPMRef> m_pmRefMap;              // 存储所有ref的map表
    std::map<std::string, std::string> m_installedInfo;     // 存储所有已安装的开明包信息
    std::set<std::string> m_installedFiles;                 // 存储所有已安装的文件列表
    std::vector<std::string> m_pmDownloadRefs;              // 先遍历运行时，再遍历app。因此先压入app,之后提取的时候顺序不会乱
    std::vector<KMfilterInfo> m_upgragableList;             // 可更新的列表
    std::vector<std::string> m_pmCanNotInstallVector;       // 不可以安装的包名
    std::multimap<std::string, std::string> m_pmOkBase;     // 离线包所需的在线依赖
    std::vector<std::string> m_pmInstallId;                 // 安装软件包名
    // std::map<std::string, KMRef> m_mapRefs;                 // ref映射表
    std::vector<std::string> m_pmBinName;
    std::shared_ptr<KMOSTreeHandler> m_ostreeHandler;
    std::unique_ptr<KMOKFile> m_okFile;               // ok 文件解压
    std::shared_ptr<KMOABMetadata> m_okMetadata;      // ok 文件解压中的 metadata 数据
    std::unique_ptr<KMHostAppDeploy> m_hostAppDeploy; // hostapp deploy
    std::unique_ptr<KMDeferredCleanupStore> m_deferredCleanupStore; // 延迟清理旧base/runtime ref

    // 并发安装任务管理
    std::map<std::string, std::shared_ptr<DownloadTaskContext>> m_activeTasks; // 所有活动任务（下载中+等待写入）
    std::queue<std::string> m_downloadQueue;                                   // 下载队列（FIFO）
    std::queue<std::string> m_writeQueue;                                      // 写入队列（串行，FIFO）
    std::mutex m_tasksMutex;                                                   // 保护任务相关数据结构
    std::mutex m_prepareMutex;                                                 // 保护准备阶段的全局状态（m_pmFuzzyMatchRefs 等）
    std::condition_variable m_downloadCondition;                               // 下载队列条件变量
    std::condition_variable m_writeCondition;                                  // 写入队列条件变量
    std::vector<std::thread> m_downloadWorkers;                                // 下载 worker 线程池
    std::thread m_writerThread;                                                // 写入线程（单线程）
    std::atomic<bool> m_shutdown{false};                                       // 关闭标志
    std::atomic<bool> m_workersStarted{false};                                 // 线程是否已启动标志
    std::mutex m_workersStartMutex;                                            // 保护线程启动的互斥锁
    static constexpr size_t MAX_DOWNLOAD_WORKERS = 4;                          // 最大并发下载数

    // 生成唯一的任务ID
    std::string generateTaskId(const std::string &appName)
    {
        static std::atomic<uint64_t> counter{0};
        return appName + "_" + std::to_string(counter.fetch_add(1)) + "_" + std::to_string(std::time(nullptr));
    }

    // Worker 线程函数
    void downloadWorkerThread();
    void writerThread();

    // 确保 worker 线程已启动（延迟启动，只在需要时启动）
    void ensureWorkersStarted();
    
    // 部署任务：将下载的包部署到系统（适配并发安装场景）
    int deployTask(std::shared_ptr<DownloadTaskContext> context);
};

KMPackageManager::KMPackageManager()
    : d(std::make_unique<Private>())
{
    d->m_packageManager = this;  // 设置指向自己的指针
    initializeFixedModifications();
    d->m_folder = std::make_unique<KMFolder>(KAIMING_SYSTEM_PATH, false);
    d->m_util = std::make_unique<KMUtil>();
    d->m_env = std::make_unique<KMEnv>();
    d->m_parseSummaryJson = std::make_unique<KMParseSummaryJson>();
    d->m_formatter = std::make_unique<KMFormatter>();

    d->m_ostreeHandler = std::make_unique<KMOSTreeHandler>(d->m_folder->getRepoPath()); // opt/kaiming/repo
    d->m_okFile = std::make_unique<KMOKFile>();                                         // ok 文件解压
    d->m_okMetadata = std::make_shared<KMOABMetadata>();                                // ok 文件解压中的 metadata 数据
    d->m_hostAppDeploy = std::make_unique<KMHostAppDeploy>(d->m_folder->getInfoPath());
    d->m_list = std::make_unique<KMList>();
    d->m_deferredCleanupStore = std::make_unique<KMDeferredCleanupStore>(d->m_folder->getInfoPath());

    // {
    //     auto allRefs = d->m_ostreeHandler->getAllLocalRefs();
    //     d->m_mapRefs.clear();
    //     for (const auto &entry : allRefs)
    //     {
    //         d->m_mapRefs.insert(entry.second.begin(), entry.second.end());
    //     }
    // }
}

KMPackageManager::~KMPackageManager()
{
    if (d->m_workersStarted.load()) {
    // 停止所有 worker 线程
    {
        std::lock_guard<std::mutex> lock(d->m_tasksMutex);
        d->m_shutdown = true;
    }
    d->m_downloadCondition.notify_all();
    d->m_writeCondition.notify_all();

    // 等待所有下载 worker 线程结束
    for (auto& worker : d->m_downloadWorkers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // 等待写入线程结束
    if (d->m_writerThread.joinable()) {
        d->m_writerThread.join();
        }
    }
}

int KMPackageManager::processTask(TaskType taskType, const vector<string> &vectorIds, PackageManagerOptions pmOptions)
{
    // 环境校验
    if (!d->m_env->passEnvCheck())
    {
        kmlogger.error("Failed to check environments!");
        return KM_ERROR_CHECK_ENV_FAILED;
    }

    d->m_folder->ensureRepoForSudo(false);

    // KMFileLock pkgLock;
    // if (!pkgLock.lock(KMFileLock::LockType::WRITE)) {
    //     std::cerr << "Unable to acquire the kaiming lock, another process is already in progress!" << std::endl;
    //     return KM_ERROR_INSTALLING;
    // }

    int ret = 0;
    // 根据任务类型执行不同的任务
    switch (taskType)
    {
    case TaskType::TASK_TYPE_INSTALL:
        ret = install(vectorIds, pmOptions);
        break;

    case TaskType::TASK_TYPE_UPDATE:
        ret = update(vectorIds, pmOptions);
        break;

    case TaskType::TASK_TYPE_SEARCH:
        ret = search(vectorIds[0], pmOptions);
        break;

    case TaskType::TASK_TYPE_UNINSTALL:
        ret = uninstall(vectorIds, pmOptions);
        break;

    case TaskType::TASK_TYPE_REPAIR:
        ret = repair(vectorIds, pmOptions);
        break;

    case TaskType::TASK_TYPE_UPGRADE:
        ret = install(vectorIds, pmOptions);
        break;
    default:
        break;
    }

    return ret;
}

int KMPackageManager::install(const vector<string> &vectorIds, PackageManagerOptions pmOptions)
{
    if (pmOptions.isPrint)
    {
        std::cout << _("Search matching packages ...") << std::endl;
    }

    d->m_pmOldAVersion.clear();
    d->m_pmSpecialIdVersion.clear();
    d->m_pmInstallId.clear();
    d->m_pmCanNotInstallVector.clear();
    d->m_pmFuzzyMatchRefs.clear();
    d->m_pmDownloadRefs.clear();

    resetInstallState();

    // 驱动清理
    int cleanedCount = d->m_hostAppDeploy->checkAndCleanupUninstalledDrivers();
    if (cleanedCount > 0)
    {
        kmlogger.info("Cleaned up %d uninstalled driver packages before installation", cleanedCount);
    }

    d->m_pmIsForceInstall = pmOptions.force;
    int countOfflinePackages = 0;
    for (auto name : vectorIds)
    {
        kmlogger.info("Install app %s", name.c_str());
        bool isMatchedRefFlag = false;

        KMPMRef pmRef = d->m_util->parseKMPMRef(name);
        d->m_util->printKMPMRef(name, pmRef);

        // 如果用户指定了 arch，则覆盖默认的 arch
        if (!pmOptions.arch.empty())
        {
            pmRef.ref.arch = pmOptions.arch;
        }

        if (!pmOptions.kind.empty())
        {
            pmRef.ref.kind = pmOptions.kind;
        }

        if (pmRef.refSourceType == KMPACKAGE_SOURCE_TYPE_OK)
        {
            // 开始处理离线包
            ++countOfflinePackages;
        }
    }

    if (countOfflinePackages != 0 && countOfflinePackages != vectorIds.size())
    {
        cout << _("Please check the package is all online package or offline package!") << endl;
        return KM_ERROR_INSTALL_FAILED;
    }

    if (countOfflinePackages != 0)
    {
        // // 暂时先只安装一个离线包，之后优化
        // if (vectorIds.size() > 1)
        // {
        //     cout << _("Only support one offline package!") << endl;
        //     return KM_ERROR_INSTALL_FAILED;
        // }
        int ret = installOfflinePackage(vectorIds, pmOptions);
        if (!d->m_pmCanNotInstallVector.empty())
        {
            return KM_ERROR_NOT_INSTALL_ALLPACKAGE;
        }
        else
        {
            return ret;
        }
    }
    else
    {
        pmOptions.isOnlineInstall = true;
    }

    // 下载和解析清单依赖文件 summary.json
    int getSummaryState = downloadAllRemoteSummaries();
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    // 遍历取得所有符合条件的依赖列表
    int getFuzzyMatchState = getAllFuzzyMatchRefVector(vectorIds, pmOptions, TaskType::TASK_TYPE_INSTALL);
    if (getFuzzyMatchState != KM_ERROR_NO)
    {
        kmlogger.error("Get fuzzy match ref error! %d", getFuzzyMatchState);
        return getFuzzyMatchState;
    }

    // 获取必要安装列表
    getUsefulRefList();
    // 获取真正安装列表
    getDownloadRefList(TaskType::TASK_TYPE_INSTALL, pmOptions);

    if (d->m_pmRefDownloadList.empty())
    {
        return KM_ERROR_ALREADY_INSTALLED;
    }

    // 展示下载列表信息
    printDownloadRefList(d->m_pmRefDownloadList);

    // 用户选择是否展示信息
    if (!printAssumeYesChoice(pmOptions))
    {
        return KM_ERROR_NOT_INSTALLED;
    };

    InstallTaskContext ctx;

    unsigned int maxSpeedLimitKilobytes = pmOptions.maxSpeedLimitKilobytes > 0 ? pmOptions.maxSpeedLimitKilobytes : 0;
    auto repoRefMap = d->m_parseSummaryJson->getRepoToRefs();
    reorderRefListByRepoMap(repoRefMap, refDownloadListForProgress);

    auto isLocalRepoUrl = [](const std::string &url) -> bool
    {
        if (url.empty() || url == "-")
        {
            return true;
        }

        if (url.rfind("file://", 0) == 0 || url.rfind("file:", 0) == 0)
        {
            return true;
        }

        if (url.rfind("unix://", 0) == 0 || url.rfind("unix:", 0) == 0)
        {
            return true;
        }

        if (url.find("://") == std::string::npos)
        {
            return true;
        }
        return false;
    };

    bool needNetworkCheck = false;
    if (d->m_ostreeHandler)
    {
        if (!repoRefMap.empty())
        {
            for (const auto &[remote, refs] : repoRefMap)
            {
                (void)refs;
                std::string remoteUrl = d->m_ostreeHandler->getRemoteUrl(remote);
                if (!isLocalRepoUrl(remoteUrl))
                {
                    needNetworkCheck = true;
                    break;
                }
            }
        }
    }

    installing = true;
    if (needNetworkCheck)
    {
        std::thread([this, &ctx]()
                    {
                        while (installing && !ctx.cancelRequested.load() && !ctx.stopNetworkCheck.load())
                        {
                            checkAndHandleNetwork(d->m_ostreeHandler, &ctx, this);
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        } })
            .detach();
    }

    bool pullFlag = true;

    if (ctx.cancelRequested.load())
    {
        installing = false;
        return KM_ERROR_NETWORK_FAILED;
    }

    if (!repoRefMap.empty())
    {
        for (const auto &[remote, refs] : repoRefMap)
        {
            kmlogger.info("Pulling %zu refs from remote: %s", refs.size(), remote.c_str());
            bool result = d->m_ostreeHandler->pull(remote, refs, progressCallback, this, maxSpeedLimitKilobytes);
            if (result)
            {
                ctx.stopNetworkCheck.store(true);
            }
            if (!result)
            {
                kmlogger.error("Pull failed for remote: %s", remote.c_str());
                pullFlag = false;
                break;
            }
        }
    }

    installing = false; // 安装结束，停止监控
    if (ctx.cancelRequested.load())
    {
        return KM_ERROR_NETWORK_FAILED;
    }

    if (!pullFlag)
    {
        return KM_ERROR_PULL_FAILED;
    }

    int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, d->m_pmIsForceInstall);
    if (cleanupRet != KM_ERROR_NO)
    {
        return cleanupRet;
    }
    // 部署包
    int ret = deploy(pmOptions);

    if (!d->m_pmCanNotInstallVector.empty())
    {
        return KM_ERROR_NOT_INSTALL_ALLPACKAGE;
    }
    else
    {
        return ret;
    }
}

int KMPackageManager::installOfflinePackage(const vector<string> &vectorIds, PackageManagerOptions pmOptions, KMPMProgressCallBack downloadCallback, KMPMProgressCallBack deployCallback)
{
    vector<string> onlinePackageIds; // 离线包的 id 列表
    std::map<string, string> mountMap;
    int ret = KM_ERROR_NO;
    bool printLog = false;
    bool canNotInstall = false;
    bool isVerifyFailed = false;

    // 设置进度回调
    globalProgressCallback = downloadCallback;
    deployProgressCallback = deployCallback;
    // 与并发在线安装(enqueueInstall)共用同一把锁，避免共享状态(m_pmFuzzyMatchRefs 等)被并发修改
    {
        std::lock_guard<std::mutex> prepareLock(d->m_prepareMutex);

        d->m_okFile->clearMountPoint();
        d->m_pmRefOfflineList.clear();
        d->m_pmOkBase.clear();
        d->m_pmOldAVersion.clear();
        d->m_pmSpecialIdVersion.clear();
        d->m_pmInstallId.clear();
        d->m_pmCanNotInstallVector.clear();
        d->m_pmFuzzyMatchRefs.clear();
        d->m_pmDownloadRefs.clear();
        d->m_pmRefDownloadList.clear();

        resetInstallState();

        for (auto name : vectorIds)
        {
            kmlogger.debug("Install offline packages %s", name.c_str());
            bool isMatchedRefFlag = false;

            KMPMRef pmRef = d->m_util->parseKMPMRef(name);
            d->m_util->printKMPMRef(name, pmRef);

            // 如果用户指定了 arch，则覆盖默认的 arch
            if (!pmOptions.arch.empty())
            {
                pmRef.ref.arch = pmOptions.arch;
            }

            if (!pmOptions.kind.empty())
            {
                pmRef.ref.kind = pmOptions.kind;
            }

            if (pmRef.refSourceType == KMPACKAGE_SOURCE_TYPE_OK)
            {
                d->m_isOffInstall = true;
                // 开始处理离线包
                ret = installFromOK(name, onlinePackageIds, pmOptions, mountMap, canNotInstall);

                if (ret != KM_ERROR_NO)
                {
                    canNotInstall = true;
                    // d->m_okFile->umountOK();
                    continue;
                }
            }
        }

        if (!d->m_pmFuzzyMatchRefs.empty())
        {
            for (const auto &ref : d->m_pmFuzzyMatchRefs)
            {
                kmlogger.debug("fuzzy match ref : %s", ref.c_str());
            }

            // 依赖列表去重
            KMUtil::removeDuplicateValues(d->m_pmFuzzyMatchRefs);
        }
        else
        {
            if ((printLog || canNotInstall))
            {
                d->m_okFile->umountOK();
                return ret;
            }
            else
            {
                printf("%s\n", _("This app has been installed!"));
                d->m_okFile->umountOK();
                return KM_ERROR_ALREADY_INSTALLED;
            }
        }

        for (auto it = onlinePackageIds.begin(); it != onlinePackageIds.end();)
        {
            string onlinId = *it;
            KMPMRef idRef = d->m_util->parseKMPMRef(onlinId);
            bool isMatchedRefFlag = false;
            for (auto ref : d->m_pmFuzzyMatchRefs)
            {
                KMPMRef fuzzyRef = d->m_util->parseKMPMRef(ref);
                if (idRef.ref.id == fuzzyRef.ref.id && idRef.ref.kind == fuzzyRef.ref.kind && idRef.ref.arch == fuzzyRef.ref.arch && idRef.ref.module == fuzzyRef.ref.module && idRef.ref.channel == fuzzyRef.ref.channel)
                {
                    if (idRef.ref.version[0] == '-')
                    {
                        it = onlinePackageIds.erase(it);
                        isMatchedRefFlag = true;
                        break;
                    }
                    KMVersion idVersion = KMVersion::parse(idRef.ref.version, false);
                    KMVersion fuzzyVersion = KMVersion::parse(fuzzyRef.ref.version, false);
                    if (idVersion.getMajor() == fuzzyVersion.getMajor())
                    {
                        it = onlinePackageIds.erase(it);
                        isMatchedRefFlag = true;
                        break;
                    }
                }
            }
            if (!isMatchedRefFlag)
            {
                it++;
            }
        }

        if (!onlinePackageIds.empty())
        {
            std::cout << _("The following dependency packages need to be downloaded online") << std::endl;

            for (auto packageId : onlinePackageIds)
            {
                std::cout << packageId << std::endl;
            }

            d->m_pmFuzzyMatchRefs.clear();
            int getSummaryState = downloadAllRemoteSummaries();
            if (getSummaryState != KM_ERROR_NO)
            {
                kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
                d->m_okFile->umountOK();
                return getSummaryState;
            }

            pmOptions.compatible = true;
            int getFuzzyMatchState = getAllFuzzyMatchRefVector(onlinePackageIds, pmOptions, TaskType::TASK_TYPE_INSTALL);

            if (!d->m_pmCanNotInstallVector.empty())
            {
                std::cout << _("The dependencies of these software packages cannot be satisfied: ") << std::endl;
            }

            for (auto notInstall : d->m_pmCanNotInstallVector)
            {
                std::unordered_set<std::string> keysToDelete;
                // 找出所有依赖 notInstall 的键
                for (auto it = d->m_pmOkBase.begin(); it != d->m_pmOkBase.end(); ++it)
                {
                    if (it->second == notInstall)
                    {
                        keysToDelete.insert(it->first);
                    }
                }

                // 从 m_pmOkBase 中删除所有相关的键值对
                for (const auto &key : keysToDelete)
                {
                    auto range = d->m_pmOkBase.equal_range(key);
                    for (auto it = range.first; it != range.second; ++it)
                    {
                        std::regex pattern(R"(([^/]+)/([^/]+)/([^/]+)/([^/]+)/([^/]+))");

                        for (auto itRef = d->m_pmFuzzyMatchRefs.begin(); itRef != d->m_pmFuzzyMatchRefs.end();)
                        {
                            KMPMRef pmFuzzyRef = KMUtil::parseSummaryFuzzyKMPMRef(*itRef);
                            std::smatch match;

                            if (std::regex_search(it->second, match, pattern))
                            {
                                std::string id = match[1].str();
                                std::string version = match[2].str();
                                std::string module = match[3].str();
                                std::string channel = match[4].str();
                                std::string arch = match[5].str();

                                if (pmFuzzyRef.ref.id == id && pmFuzzyRef.ref.module == module && pmFuzzyRef.ref.channel == channel && pmFuzzyRef.ref.arch == arch)
                                {
                                    if (version != "-")
                                    {
                                        KMVersion pmVersion = KMVersion::parse(pmFuzzyRef.ref.version, false);
                                        KMVersion reVersion = KMVersion::parse(version, false);
                                        if (pmVersion.getMajor() == reVersion.getMajor())
                                        {
                                            itRef = d->m_pmFuzzyMatchRefs.erase(itRef);
                                            continue;
                                        }
                                    }
                                    else
                                    {
                                        itRef = d->m_pmFuzzyMatchRefs.erase(itRef);
                                        continue;
                                    }
                                }
                            }
                            ++itRef;
                        }
                    }
                    std::cout << key << std::endl;
                    d->m_pmOkBase.erase(key);
                    printLog = true;
                    // 从 m_pmRefOfflineList 中删除相关的引用
                    auto newEnd = std::remove_if(d->m_pmRefOfflineList.begin(), d->m_pmRefOfflineList.end(), [this, key](const KMPMRef &element)
                                                 { return element.ref.id == key; });
                    d->m_pmRefOfflineList.erase(newEnd, d->m_pmRefOfflineList.end());
                }
            }

            if (getFuzzyMatchState != KM_ERROR_NO)
            {
                d->m_okFile->umountOK();
                kmlogger.error("Get fuzzy match ref error, err = %d!", getFuzzyMatchState);
                return getFuzzyMatchState;
            }

            // 获取必要安装列表
            getUsefulRefList();
            // 获取真正安装列表
            getDownloadRefList(TaskType::TASK_TYPE_INSTALL, pmOptions);
        }
    } // prepareLock 作用域结束，此后只读已准备好的列表，不再修改共享状态

    d->m_isOffInstall = false;

    if (d->m_pmRefOfflineList.empty() && d->m_pmRefDownloadList.empty())
    {
        if (printLog || canNotInstall)
        {
            d->m_okFile->umountOK();
            return KM_ERROR_NOT_INSTALLED;
        }
        else
        {
            printf("%s\n", _("This app has been installed!"));
            d->m_okFile->umountOK();
            return KM_ERROR_ALREADY_INSTALLED;
        }
    }
    else if (d->m_pmRefOfflineList.empty() && !d->m_pmRefDownloadList.empty())
    {
        // 展示下载列表信息
        printDownloadRefList(d->m_pmRefDownloadList);

        // 用户选择是否展示信息
        if (!printAssumeYesChoice(pmOptions))
        {
            d->m_okFile->umountOK();
            return KM_ERROR_NOT_INSTALLED;
        }

        InstallTaskContext ctx;
        installing = true; // 先设置安装标志

        std::thread([this, &ctx]()
                    {
                        while (installing && !ctx.cancelRequested.load() && !ctx.stopNetworkCheck.load())
                        {
                            checkAndHandleNetwork(d->m_ostreeHandler, &ctx, this);
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        } })
            .detach();

        // 开始真正的拉取
        auto repoRefMap = d->m_parseSummaryJson->getRepoToRefs();
        bool pullFlag = true;
        if (ctx.cancelRequested.load())
        {
            installing = false;
            d->m_okFile->umountOK();
            return KM_ERROR_NETWORK_FAILED;
        }

        unsigned int maxSpeedLimitKilobytes = pmOptions.maxSpeedLimitKilobytes > 0 ? pmOptions.maxSpeedLimitKilobytes : 0;

        if (!repoRefMap.empty())
        {
            for (const auto &[remote, refs] : repoRefMap)
            {
                kmlogger.info("Pulling %zu refs from remote: %s", refs.size(), remote.c_str());
                // 使用成员函数 progressCallback，而不是参数 progressCallback（避免命名冲突）
                bool result = d->m_ostreeHandler->pull(remote, refs, this->progressCallback, this, maxSpeedLimitKilobytes);
                if (result)
                {
                    ctx.stopNetworkCheck.store(true);
                }
                if (!result)
                {
                    kmlogger.error("Pull failed for remote: %s", remote.c_str());
                    pullFlag = false;
                    break;
                }
            }
        }
        else
        {
            kmlogger.warn("No remote-ref mapping found, using fallback pull");
            pullFlag = d->m_ostreeHandler->pullWithFallback(d->m_pmDownloadRefs, this->progressCallback, this, maxSpeedLimitKilobytes);
            if (pullFlag)
            {
                ctx.stopNetworkCheck.store(true);
            }
        }

        installing = false; // 安装结束，停止监控
        if (ctx.cancelRequested.load())
        {
            d->m_okFile->umountOK();
            return KM_ERROR_NETWORK_FAILED;
        }

        if (!pullFlag)
        {
            d->m_okFile->umountOK();
            return KM_ERROR_PULL_FAILED;
        }

        // 部署在线下载包
        kmlogger.info("Only deploy online package!");

        int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, d->m_pmIsForceInstall);
        if (cleanupRet != KM_ERROR_NO)
        {
            return cleanupRet;
        }

        return deploy(pmOptions);
    }
    else if (!d->m_pmRefOfflineList.empty() && d->m_pmRefDownloadList.empty())
    {
        // 展示下载列表信息
        printDownloadRefList(d->m_pmRefOfflineList);

        // 用户选择是否展示信息
        if (!printAssumeYesChoice(pmOptions))
        {
            d->m_okFile->umountOK();
            return KM_ERROR_NOT_INSTALLED;
        }

        // 部署离线包
        kmlogger.info("Only deploy offline package!");

        int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, d->m_pmIsForceInstall, true);
        if (cleanupRet != KM_ERROR_NO)
        {
            return cleanupRet;
        }

        return deployOfflinePackage(mountMap);
    }
    else
    {
        // 展示下载列表信息
        vector<KMPMRef> allRefList;
        allRefList.insert(allRefList.end(), d->m_pmRefOfflineList.begin(), d->m_pmRefOfflineList.end());
        allRefList.insert(allRefList.end(), d->m_pmRefDownloadList.begin(), d->m_pmRefDownloadList.end());
        printDownloadRefList(allRefList);

        for (auto &offline : d->m_pmRefOfflineList)
        {
            kmlogger.info("offline ref: %s\n", offline.completedRef.c_str());
        }

        for (auto &online : d->m_pmRefDownloadList)
        {
            kmlogger.info("online ref: %s\n", online.completedRef.c_str());
        }

        // 用户选择是否展示信息
        if (!printAssumeYesChoice(pmOptions))
        {
            d->m_okFile->umountOK();
            return KM_ERROR_NOT_INSTALLED;
        }

        // 部署离线包
        kmlogger.info("Deploy offline package!");

        // 下载包
        InstallTaskContext ctx;
        installing = true;
        bool pullFlag = false;

        std::thread([this, &ctx]()
                    {
                        while (installing && !ctx.cancelRequested.load() && !ctx.stopNetworkCheck.load())
                        {
                            checkAndHandleNetwork(d->m_ostreeHandler, &ctx, this);
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        } })
            .detach();

        // 开始真正的拉取
        //  pullFlag = d->m_ostreeHandler->pull(REPO_NAME, d->m_pmDownloadRefs, progressCallback, pmOptions.maxSpeedLimitKilobytes);
        pullFlag = d->m_ostreeHandler->pullWithFallback(d->m_pmDownloadRefs, this->progressCallback, this, pmOptions.maxSpeedLimitKilobytes);
        if (pullFlag)
        {
            ctx.stopNetworkCheck.store(true);
        }

        installing = false; // 安装结束，停止监控

        if (ctx.cancelRequested.load())
        {
            return KM_ERROR_NETWORK_FAILED;
        }

        if (!pullFlag)
        {
            return KM_ERROR_PULL_FAILED;
        }

        // 部署在线下载包
        kmlogger.info("Deploy online package!");
        int retDeploy = deploy(pmOptions);
        if (retDeploy != KM_ERROR_NO)
        {
            return retDeploy;
        }

        int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, d->m_pmIsForceInstall);
        if (cleanupRet != KM_ERROR_NO)
        {
            return cleanupRet;
        }

        return deployOfflinePackage(mountMap);
    }
}

std::string KMPackageManager::installSoftwareCenter(const std::vector<std::string> &vecNames, string version, KMPMProgressCallBack downloadCallback, KMPMProgressCallBack deployCallback, PackageManagerOptions pmOption, unsigned int maxSpeedLimitKilobytes, KMPMCompletionCallBack completionCallback)
{
    kmlogger.debug("begin install (async), maxSpeedLimitKilobytes = %u", maxSpeedLimitKilobytes);

    // PackageManagerOptions pmOption;
    pmOption.assumeYes = true;
    pmOption.skipSignatureVerification = false;
    pmOption.maxSpeedLimitKilobytes = maxSpeedLimitKilobytes;
    
    // 使用异步并发安装
    return enqueueInstall(vecNames, version, downloadCallback, deployCallback, pmOption, completionCallback);
}

std::string KMPackageManager::enqueueInstall(const std::vector<std::string>& vectorIds, std::string version, KMPMProgressCallBack downloadCallback, KMPMProgressCallBack deployCallback, PackageManagerOptions pmOptions, KMPMCompletionCallBack completionCallback)
{
    std::string pausedTaskId;
    {
        std::lock_guard<std::mutex> lock(d->m_tasksMutex);
        for (const auto& [taskId, context] : d->m_activeTasks) {
            if (context->mainAppName == vectorIds[0]) {
                // 检查任务是否已完成（installResult 不为默认值）
                // 如果任务已完成但还在 m_activeTasks 中，可能是等待清理，允许创建新任务
                // 如果任务正在执行（installResult 为默认值），返回已存在的 taskId
                if(context->installResult == KM_ERROR_USER_PAUSED) {
                    pausedTaskId = taskId;
                }

                if (context->installResult == KM_ERROR_NOTHING_TO_DO) {
                    kmlogger.info("Task for package %s already exists (taskId: %s), returning existing taskId", vectorIds[0].c_str(), taskId.c_str());
                    return taskId;
                }
            }
        }
    }

    if (!pausedTaskId.empty()) {
        bool resumed = resumeInstall(pausedTaskId);
        if (resumed) {
            return pausedTaskId;
        }
    }

    // 确保 worker 线程已启动（延迟启动）
    d->ensureWorkersStarted();

    // 创建任务上下文
    auto taskContext = std::make_shared<DownloadTaskContext>();
    taskContext->mainAppName = vectorIds[0];
    taskContext->downloadCallback = downloadCallback;
    taskContext->deployCallback = deployCallback;
    taskContext->completionCallback = completionCallback;  // 设置完成回调
    taskContext->taskId = d->generateTaskId(vectorIds[0]);
    taskContext->refDownloadList.clear();
    taskContext->packageManager = this;

    // 保存安装选项到 context（包含 maxSpeedLimitKilobytes）
    taskContext->pmOptions = pmOptions;

    // 在主线程中完成准备阶段（避免在 worker 线程中调用 DBus 等非线程安全的 API）
    // 使用锁保护准备阶段的全局状态
    {
        std::lock_guard<std::mutex> prepareLock(d->m_prepareMutex);

        try {
            // 清理状态
            d->m_pmOldAVersion.clear();
            d->m_pmSpecialIdVersion.clear();
            d->m_pmInstallId.clear();
            d->m_pmCanNotInstallVector.clear();
            d->m_pmFuzzyMatchRefs.clear();
            d->m_pmDownloadRefs.clear();

            // 下载和解析清单依赖文件 summary.json
            int getSummaryState = downloadAllRemoteSummaries();
            if (getSummaryState != KM_ERROR_NO)
            {
                kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
                taskContext->installResult = getSummaryState;

                // 调用完成回调，发送finishsignal信号
                if (completionCallback) {
                    try {
                        std::string errorInfo = KMErrorMsg::getErrorMessage(getSummaryState);
                        completionCallback(taskContext->mainAppName, KM_INSTALL_OPERATION, getSummaryState, errorInfo);
                    } catch (const std::exception& e) {
                        kmlogger.error("Task %s: Exception in completion callback: %s", taskContext->taskId.c_str(), e.what());
                    } catch (...) {
                        kmlogger.error("Task %s: Unknown exception in completion callback", taskContext->taskId.c_str());
                    }
                }

                // 返回空 taskId 表示失败（调用者可以通过其他方式检查）
                return "";
            }

            // 遍历取得所有符合条件的依赖列表
            int getFuzzyMatchState = getAllFuzzyMatchRefVector(vectorIds, pmOptions, TaskType::TASK_TYPE_INSTALL);
            if (getFuzzyMatchState != KM_ERROR_NO)
            {
                kmlogger.error("Get fuzzy match ref error! %d", getFuzzyMatchState);
                taskContext->installResult = getFuzzyMatchState;

                if (completionCallback) {
                    try {
                        std::string errorInfo = KMErrorMsg::getErrorMessage(getFuzzyMatchState);
                        completionCallback(taskContext->mainAppName, KM_INSTALL_OPERATION, getFuzzyMatchState, errorInfo);
                    } catch (const std::exception& e) {
                        kmlogger.error("Task %s: Exception in completion callback: %s", taskContext->taskId.c_str(), e.what());
                    } catch (...) {
                        kmlogger.error("Task %s: Unknown exception in completion callback", taskContext->taskId.c_str());
                    }
                }

                return "";
            }

            // 获取必要安装列表
            getUsefulRefList();

            // 获取真正安装列表
            getDownloadRefList(TaskType::TASK_TYPE_INSTALL, pmOptions);

            if (d->m_pmRefDownloadList.empty())
            {
                kmlogger.info("Task %s: All packages already installed", taskContext->taskId.c_str());
                taskContext->installResult = KM_ERROR_ALREADY_INSTALLED;

                if (completionCallback) {
                    try {
                        std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_ALREADY_INSTALLED);
                        completionCallback(taskContext->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_ALREADY_INSTALLED, errorInfo);
                    } catch (const std::exception& e) {
                        kmlogger.error("Task %s: Exception in completion callback: %s", taskContext->taskId.c_str(), e.what());
                    } catch (...) {
                        kmlogger.error("Task %s: Unknown exception in completion callback", taskContext->taskId.c_str());
                    }
                }

                return "";
            }

            // 将下载列表保存到 context
            taskContext->refDownloadList = d->m_pmRefDownloadList;
            
            // 获取 remote-ref 映射（用于下载）
            taskContext->repoRefMap = d->m_parseSummaryJson->getRepoToRefs();
            reorderRefListByRepoMap(taskContext->repoRefMap, taskContext->refDownloadList);
            
            // 将无法安装的包保存到 context
            for (const auto& cannotInstallName : d->m_pmCanNotInstallVector) {
                taskContext->cannotInstallRefs.push_back(cannotInstallName);
            }

            kmlogger.info("Task %s: Prepared %zu refs for download", taskContext->taskId.c_str(), taskContext->refDownloadList.size());
        } catch (const std::exception& e) {
            kmlogger.error("Exception in enqueueInstall preparation for task %s: %s", taskContext->taskId.c_str(), e.what());
            taskContext->installResult = KM_ERROR_INSTALL_FAILED;

            if (completionCallback) {
                try {
                    std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_INSTALL_FAILED);
                    completionCallback(taskContext->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_ALREADY_INSTALLED, errorInfo);
                } catch (const std::exception& callbackEx) {
                    kmlogger.error("Task %s: Exception in completion callback: %s", taskContext->taskId.c_str(), callbackEx.what());
                } catch (...) {
                    kmlogger.error("Task %s: Unknown exception in completion callback", taskContext->taskId.c_str());
                }
            }

            return "";
        } catch (...) {
            kmlogger.error("Unknown exception in enqueueInstall preparation for task %s", taskContext->taskId.c_str());
            taskContext->installResult = KM_ERROR_INSTALL_FAILED;

            if (completionCallback) {
                try {
                    std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_INSTALL_FAILED);
                    completionCallback(taskContext->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_ALREADY_INSTALLED, errorInfo);
                } catch (const std::exception& callbackEx) {
                    kmlogger.error("Task %s: Exception in completion callback: %s", taskContext->taskId.c_str(), callbackEx.what());
                } catch (...) {
                    kmlogger.error("Task %s: Unknown exception in completion callback", taskContext->taskId.c_str());
                }
            }

            return "";
        }
    }

    // 将任务加入活动任务表和下载队列
    {
        std::lock_guard<std::mutex> lock(d->m_tasksMutex);
        d->m_activeTasks[taskContext->taskId] = taskContext;
        d->m_downloadQueue.push(taskContext->taskId);
    }

    // 通知下载 worker
    d->m_downloadCondition.notify_one();

    kmlogger.info("Task %s enqueued for installation", taskContext->taskId.c_str());
    return taskContext->taskId;
}

bool KMPackageManager::cancelInstall(const std::string& taskIdOrName)
{
    // 使用 try_lock 避免阻塞，如果锁被占用，短暂重试后返回
    std::unique_lock<std::mutex> lock(d->m_tasksMutex, std::try_to_lock);

    // 如果 try_lock 失败，短暂重试几次（每次等待 10ms，最多重试 10 次，总共最多 100ms）
    if (!lock.owns_lock())
    {
        kmlogger.warn("cancelInstall: m_tasksMutex is locked, retrying...");
        for (int i = 0; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock = std::unique_lock<std::mutex>(d->m_tasksMutex, std::try_to_lock);
            if (lock.owns_lock())
            {
                break;
            }
        }

        if (!lock.owns_lock())
        {
            kmlogger.error("cancelInstall: Failed to acquire lock after retries for %s", taskIdOrName.c_str());
            return false;
        }
    }

    // 首先尝试按 taskId 查找
    auto it = d->m_activeTasks.find(taskIdOrName);
    if (it != d->m_activeTasks.end())
    {
        auto context = it->second;
        TaskState currentState = context->taskState.load();
        if (context->installResult == KM_ERROR_NOTHING_TO_DO || currentState == TaskState::PAUSED)
        {
            context->cancelRequested.store(true);
            context->taskState.store(TaskState::CANCELLED); // 设置状态为已取消

            if (context->tmpHandler)
            {
                kmlogger.info("Cancelling ostree pull operation for task %s", taskIdOrName.c_str());
                context->tmpHandler->cancel();
            }

            if (currentState == TaskState::PAUSED) {
                kmlogger.info("Task %s was paused, cleaning up resources directly", taskIdOrName.c_str());

                int errorCode = KM_ERROR_USER_CANCEL;
                context->installResult = errorCode;

                // 发送取消信号
                if (context->completionCallback) {
                    try {
                        context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, errorCode, KMErrorMsg::getErrorMessage(errorCode));
                    } catch (...) {
                        kmlogger.error("Task %s: Exception in completion callback during cancel", taskIdOrName.c_str());
                    }
                }

                std::string repoPathToRemove = context->tmpRepoPath;
                if (repoPathToRemove.empty()) {
                    // 如果tmpRepoPath为空，使用默认路径格式
                    repoPathToRemove = "/var/tmp/kaiming/job-" + context->taskId;
                }
                if (fs::exists(repoPathToRemove)) {
                    kmlogger.info("Removing temp repo for cancelled task %s: %s", taskIdOrName.c_str(), repoPathToRemove.c_str());
                    fs::remove_all(repoPathToRemove);
                }

                d->m_activeTasks.erase(it);
            }

            kmlogger.info("Task %s cancelled (matched by taskId)", taskIdOrName.c_str());
            return true;
        }
    }

    // 如果按 taskId 找不到，尝试按包名查找
    for (const auto& [taskId, context] : d->m_activeTasks) {
        if (context->mainAppName == taskIdOrName) {
            // 检查任务是否还在执行中
            TaskState currentState = context->taskState.load();
            if (context->installResult == KM_ERROR_NOTHING_TO_DO || currentState == TaskState::PAUSED) {
                context->cancelRequested.store(true);
                context->taskState.store(TaskState::CANCELLED);

                if (context->tmpHandler) {
                    kmlogger.info("Cancelling ostree pull operation for package %s (taskId: %s)", taskIdOrName.c_str(), taskId.c_str());
                    context->tmpHandler->cancel();
                }

                // 如果任务已经暂停，worker线程可能已经退出，需要直接清理资源
                if (currentState == TaskState::PAUSED) {
                    kmlogger.info("Task for package %s (taskId: %s) was paused, cleaning up resources directly", taskIdOrName.c_str(), taskId.c_str());

                    int errorCode = KM_ERROR_USER_CANCEL;
                    context->installResult = errorCode;

                    if (context->completionCallback) {
                        try {
                            context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, errorCode, KMErrorMsg::getErrorMessage(errorCode));
                        } catch (...) {
                            kmlogger.error("Task %s: Exception in completion callback during cancel", taskId.c_str());
                        }
                    }

                    std::string repoPathToRemove = context->tmpRepoPath;
                    if (repoPathToRemove.empty()) {
                        repoPathToRemove = "/var/tmp/kaiming/job-" + taskId;
                    }
                    if (fs::exists(repoPathToRemove)) {
                        kmlogger.info("Removing temp repo for cancelled task %s: %s", taskId.c_str(), repoPathToRemove.c_str());
                        fs::remove_all(repoPathToRemove);
                    }

                    d->m_activeTasks.erase(taskId);
                }

                kmlogger.info("Task for package %s cancelled (taskId: %s, matched by package name)", taskIdOrName.c_str(), taskId.c_str());
                return true;
            }
        }
    }

    kmlogger.warn("No active task found for %s (neither as taskId nor as package name)", taskIdOrName.c_str());
    return false;
}

bool KMPackageManager::pauseInstall(const std::string& taskIdOrName)
{
    // 使用 try_lock 避免阻塞
    std::unique_lock<std::mutex> lock(d->m_tasksMutex, std::try_to_lock);

    if (!lock.owns_lock()) {
        kmlogger.warn("pauseInstall: m_tasksMutex is locked, retrying...");
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock = std::unique_lock<std::mutex>(d->m_tasksMutex, std::try_to_lock);
            if (lock.owns_lock()) {
                break;
            }
        }

        if (!lock.owns_lock()) {
            kmlogger.error("pauseInstall: Failed to acquire lock after retries for %s", taskIdOrName.c_str());
            return false;
        }
    }

    // 首先尝试按 taskId 查找
    auto it = d->m_activeTasks.find(taskIdOrName);
    if (it == d->m_activeTasks.end()) {
        // 如果按 taskId 找不到，尝试按包名查找
        // 查找状态为 RUNNING 或 PAUSED 的任务（RUNNING 可以暂停，PAUSED 可能是之前暂停的）
        for (const auto& [taskId, context] : d->m_activeTasks) {
            if (context->mainAppName == taskIdOrName) {
                TaskState state = context->taskState.load();
                if (state == TaskState::RUNNING || state == TaskState::PAUSED) {
                    it = d->m_activeTasks.find(taskId);
                    break;
                }
            }
        }
    }

    if (it == d->m_activeTasks.end()) {
        kmlogger.warn("No active task found for %s", taskIdOrName.c_str());
        return false;
    }

    auto context = it->second;
    std::lock_guard<std::mutex> pauseLock(context->pauseResumeMutex);

    // 检查任务状态
    TaskState currentState = context->taskState.load();
    if (currentState != TaskState::RUNNING) {
        kmlogger.warn("Task %s is not running (current state: %d), cannot pause", taskIdOrName.c_str(), static_cast<int>(currentState));
        return false;
    }

    // 设置状态为 PAUSED（确保 worker 线程能看到暂停状态）
    // 注意：必须在 cancel 之前设置，这样 worker 线程在检查状态时能看到 PAUSED
    context->taskState.store(TaskState::PAUSED);

    // Cancel 当前的 pull 操作（通过 tmpHandler->cancel() 来中断当前的 pull）
    // 暂停时不设置 cancelRequested，worker 线程通过检查 taskState == PAUSED 来退出
    if (context->tmpHandler) {
        kmlogger.info("Cancelling ostree pull operation for task %s (pausing)", taskIdOrName.c_str());
        context->tmpHandler->cancel();
    }

    // 记录当前状态
    context->currentRef = context->progressStatus.refId;  // 当前正在处理的 ref

    kmlogger.info("Task %s paused at progress %d%%, current ref: %s",
                 taskIdOrName.c_str(), context->totalPercent, context->currentRef.c_str());
    return true;
}

bool KMPackageManager::resumeInstall(const std::string& taskIdOrName)
{
    // 使用 try_lock 避免阻塞
    std::unique_lock<std::mutex> lock(d->m_tasksMutex, std::try_to_lock);

    if (!lock.owns_lock()) {
        kmlogger.warn("resumeInstall: m_tasksMutex is locked, retrying...");
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock = std::unique_lock<std::mutex>(d->m_tasksMutex, std::try_to_lock);
            if (lock.owns_lock()) {
                break;
            }
        }

        if (!lock.owns_lock()) {
            kmlogger.error("resumeInstall: Failed to acquire lock after retries for %s", taskIdOrName.c_str());
            return false;
        }
    }

    // 首先尝试按 taskId 查找
    auto it = d->m_activeTasks.find(taskIdOrName);
    if (it != d->m_activeTasks.end()) {
        // 找到了，检查状态
        TaskState state = it->second->taskState.load();
        if (state != TaskState::PAUSED) {
            kmlogger.warn("Task %s found but not paused (state: %d)", taskIdOrName.c_str(), static_cast<int>(state));
            it = d->m_activeTasks.end(); // 重置，继续按包名查找
        }
    }

    if (it == d->m_activeTasks.end()) {
        // 如果按 taskId 找不到，尝试按包名查找
        kmlogger.debug("Searching for paused task by package name: %s", taskIdOrName.c_str());
        for (const auto& [taskId, context] : d->m_activeTasks) {
            TaskState state = context->taskState.load();
            kmlogger.debug("Checking task %s: mainAppName=%s, state=%d", taskId.c_str(), context->mainAppName.c_str(), static_cast<int>(state));
            if (context->mainAppName == taskIdOrName) {
                if (state == TaskState::PAUSED) {
                    it = d->m_activeTasks.find(taskId);
                    kmlogger.info("Found paused task %s for package %s", taskId.c_str(), taskIdOrName.c_str());
                    break;
                } else {
                    kmlogger.debug("Task %s matches package name but state is %d (not PAUSED)", taskId.c_str(), static_cast<int>(state));
                }
            }
        }
    }

    if (it == d->m_activeTasks.end()) {
        kmlogger.warn("No paused task found for %s (checked %zu active tasks)", taskIdOrName.c_str(), d->m_activeTasks.size());
        // 打印所有活动任务的状态，便于调试
        for (const auto& [taskId, context] : d->m_activeTasks) {
            kmlogger.debug("Active task: %s, package: %s, state: %d", taskId.c_str(), context->mainAppName.c_str(), static_cast<int>(context->taskState.load()));
        }
        return false;
    }

    auto context = it->second;
    std::lock_guard<std::mutex> pauseLock(context->pauseResumeMutex);

    TaskState currentState = context->taskState.load();
    if (currentState != TaskState::PAUSED) {
        kmlogger.warn("Task %s is not paused (current state: %d), cannot resume", taskIdOrName.c_str(), static_cast<int>(currentState));
        return false;
    }

    // 1. 重置取消标志（因为 pause 时设置了 cancelRequested）
    context->cancelRequested.store(false);

    // 2. 重置 installResult（因为 resume 后任务重新开始运行）
    context->installResult = KM_ERROR_NOTHING_TO_DO;

    // 3. 设置状态为 RUNNING
    context->taskState.store(TaskState::RUNNING);

    // 4. 将任务重新加入下载队列（从暂停位置继续）
    // currentPercent 已经保存了暂停时的进度，恢复时会从该进度继续（通过 progressCallbackWithContext 中的检查）
    d->m_downloadQueue.push(context->taskId);
    d->m_downloadCondition.notify_one();

    kmlogger.info("Task %s resumed from progress %d%% (currentPercent: %d), re-queued for download", taskIdOrName.c_str(), context->progressStatus.progress, context->totalPercent);
    return true;
}

KMPMProgressStatus KMPackageManager::getTaskStatus(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(d->m_tasksMutex);
    auto it = d->m_activeTasks.find(taskId);
    if (it == d->m_activeTasks.end()) {
        return {"", 0, "", 5}; // 任务不存在，返回空状态
    }

    return it->second->progressStatus;
}

int KMPackageManager::updateSoftwareCenter(const string &name, string version, KMPMProgressCallBack updateCallback, KMPMProgressCallBack deployCallback)
{
    kmlogger.debug("begin update");
    globalProgressCallback = updateCallback;
    deployProgressCallback = deployCallback;

    int ret = KM_ERROR_NOTHING_TO_DO;

    PackageManagerOptions pmOption;
    vector<string> vecUpdate;

    pmOption.assumeYes = true;
    vecUpdate.push_back(name);

    ret = update(vecUpdate, pmOption);

    kmlogger.debug("end update");
    return ret;
}

int KMPackageManager::uninstallSoftwareCenter(const std::string &name)
{
    kmlogger.debug("begin uninstall");

    int ret = KM_ERROR_NOTHING_TO_DO;

    PackageManagerOptions pmOption;
    vector<string> vecUninstall;

    pmOption.assumeYes = true;
    vecUninstall.push_back(name);

    ret = uninstall(vecUninstall, pmOption);

    kmlogger.debug("end uninstall");
    return ret;
}

int KMPackageManager::deploySoftwareCenter(const std::vector<std::string>& names, PackageManagerOptions pmOptions, KMPMProgressCallBack deployCallback)
{
    kmlogger.debug("begin deploy");
    deployProgressCallback = deployCallback;

    int ret = KM_ERROR_NO;

    // 遍历取得所有符合条件的依赖列表
    int getFuzzyMatchState = getAllFuzzyMatchRefVector(names, pmOptions, TaskType::TASK_TYPE_INSTALL);
    if (getFuzzyMatchState != KM_ERROR_NO)
    {
        kmlogger.error("Get fuzzy match ref error! %d", getFuzzyMatchState);
        return getFuzzyMatchState;
    }

    // 获取必要安装列表
    getUsefulRefList();

    // 获取真正安装列表
    getDownloadRefList(TaskType::TASK_TYPE_INSTALL, pmOptions);

    // 如果部署列表为空，直接返回
    if (d->m_pmRefDownloadList.empty())
    {
        kmlogger.info("All packages are already installed");
        return KM_ERROR_ALREADY_INSTALLED;
    }

    int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, d->m_pmIsForceInstall);
    if (cleanupRet != KM_ERROR_NO)
    {
        return cleanupRet;
    }
    
    // 执行部署
    pmOptions.assumeYes = true;
    ret = deploy(pmOptions);

    kmlogger.debug("end deploy");
    return ret;
}

int KMPackageManager::installFromOK(const string &name, vector<string> &onlinePackageIds, PackageManagerOptions pmOptions, std::map<string, string> &mountMap, bool &canNotInstall)
{
    d->m_okMetadata->layers.clear();
    size_t pos = name.find(".ok");
    string mountPoint = "/tmp/" + name.substr(0, pos) + "_" + std::to_string(getpid());

    bool offlineRun = (name.find(".okinstaller") != std::string::npos) || (name.find(".okx") != std::string::npos);

    d->m_okFile->setMountPoint(mountPoint);
    // d->m_okFile->setOKFilePath(name);

    if (!d->m_okFile->loadOKFile(name))
    {
        std::cout << "Failed to load file: " << name << std::endl;
        return KM_ERROR_LOAD_FILE_FAILED;
    }

    d->m_okFile->mountOK(mountPoint);
    d->m_okMetadata = d->m_okFile->getMetadata();
    std::vector<KMInfoJson> layers = d->m_okMetadata->layers;

    if (!pmOptions.skipSignatureVerification)
    {
        std::string filename = name.substr(0, pos);
        size_t lastSlashPos = filename.find_last_of("/\\");
        filename = (lastSlashPos != std::string::npos) ? filename.substr(lastSlashPos + 1) : filename;
        std::string okId, okKind, okModule;

        if (layers.size() == 1)
        {
            okId = layers[0].id;
            okKind = layers[0].kind;
            okModule = layers[0].module;
        }
        std::string path = mountPoint + "/" + "layers/" + okId + "/" + okModule;
        fs::path modulePath = path;
        int errorCode = verifySignature(okId, okKind, modulePath);

        if (errorCode != KM_ERROR_NO)
        {
            d->m_okFile->umountOK(mountPoint);
            return KM_ERROR_SIGN_FAILED;
        }
    }

    if (layers.empty())
    {
        kmlogger.error("No layers found in OK file!");
        std::cout << "No layers found in OK file! " << name << std::endl;
        d->m_okFile->umountOK(mountPoint);
        return KM_ERROR_NO_LAYERS_FOUND;
    }

    vector<std::string> preVersion;
    if (layers.size() == 1)
    {
        for (auto layer : layers)
        {
            KMPMRef pmRef;
            string base = layer.base;
            string runtime = layer.runtime;
            vector<string> depends = layer.annotations.depends;

            if (layer.annotations.deltas == true)
            {
                preVersion = layer.annotations.preVersion;
            }

            pmRef.ref.preVersion = preVersion;
            pmRef.ref.isDeltas = layer.annotations.deltas;
            pmRef.ref.id = layer.id;
            pmRef.ref.channel = layer.channel;
            pmRef.ref.version = layer.version;
            pmRef.ref.module = layer.module;
            pmRef.ref.kind = layer.kind;
            pmRef.installedSize = layer.size;
            pmRef.name = layer.name;
            pmRef.description = layer.description;
            pmRef.ref.isSysapp = layer.annotations.sysapp;
            pmRef.ref.isHostapp = layer.getAnnotations().hostapp;
            pmRef.ref.vecPlugins = layer.export_plugins;
            pmRef.ref.vecDbusWhitelist = layer.getAnnotations().kylin_dbus_whitelist;
            pmRef.ref.isOfflineRun = offlineRun;

            if (pmRef.ref.isHostapp)
            {
                // std::cout << KMErrorMsg::getErrorMessage(KM_ERROR_NOT_SUPPORT_HOSTAPP) << "\n";
                // return KM_ERROR_NOT_SUPPORT_HOSTAPP;
                d->m_hostAppDeploy->checkAndCleanupUninstalledDrivers();
            }

            if (d->m_pmIsThereDbusService)
            {
                const char *dbusmethod = "GetPackageInstallStatus";
                const char *idChar = pmRef.ref.id.c_str();
                const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiinstall";
                int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

                if (getDbusReturn == -1)
                {
                    d->m_pmIsThereDbusService = false;
                }
                if (getDbusReturn == 1)
                {
                    canNotInstall = true;
                    std::cout << "can not install " << pmRef.ref.id << std::endl;
                    d->m_okFile->umountOK(mountPoint);
                    return KM_ERROR_CANNOT_INSTALL;
                }
            }

            for (auto arch : layer.archs)
            {
                if (arch == KMUtil::kmGetArch())
                {
                    pmRef.ref.arch = arch;
                }
                else if (KMUtil::kmGetArch() == "longarch64" && arch == "loong64")
                {
                    pmRef.ref.arch = arch;
                }
            }

            if (pmRef.ref.arch.empty() && (layer.kind == "base" || layer.kind == "runtime"))
            {
                pmRef.ref.arch = layer.archs[0];
            }

            mountMap[layer.id + pmRef.ref.arch + layer.module] = mountPoint;
            pmRef.completedRef = KMUtil::dupKMPMCompletedRef(pmRef);
            d->m_pmRefMap[layer.id] = pmRef;

            if (pmRef.ref.arch.empty())
            {
                const std::string &filePath = d->m_okFile->getOKFilePath();
                std::cout << "Unsupported architecture for file: " << filePath << std::endl;
                kmlogger.error("Unsupported architecture - File: %s", filePath.c_str());
                d->m_okFile->umountOK(mountPoint);
                return KM_ERROR_NOT_SUPPORT_ARCH;
            }
            pmRef.ref.isSpecialVersion = true;
            d->m_pmSpecialIdVersion.push_back(pmRef.ref.id + pmRef.ref.version);

            if (currentFuzzyRefIsInstalled(pmRef))
            {
                // 不强制安装已经安装的应用
                if (!pmOptions.force)
                {
                    std::cout << _("This package has been installed! ") << pmRef.ref.id << std::endl;
                    d->m_okFile->umountOK(mountPoint);
                    return KM_ERROR_ALREADY_INSTALLED;
                }

                std::string removeStr;

                if (pmRef.ref.module == "binary")
                {
                    fs::path rootPath = d->m_folder->getLayersPath();
                    removeStr = rootPath.string() + "/" + pmRef.completedRef;
                }

                if (pmRef.ref.module == "devel")
                {
                    fs::path develDir = fs::path(KMStorageDir::getUserBaseDirLocation());
                    removeStr = develDir.string() + "/" + pmRef.completedRef;
                }
                removeSpecificDeployFiles(removeStr);
            }

            for (auto version : pmRef.ref.preVersion)
            {
                for (auto it = d->m_pmOldAVersion.begin(); it != d->m_pmOldAVersion.end();)
                {
                    const auto &oldVersion = *it;
                    size_t lastSlashPos = oldVersion.rfind('/');
                    if (lastSlashPos != std::string::npos)
                    {
                        std::string nowOldVersion = oldVersion.substr(lastSlashPos + 1);
                        if (nowOldVersion == version)
                        {
                            pmRef.ref.findPreVersion = version;
                            it = d->m_pmOldAVersion.erase(it);
                            break;
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    else
                    {
                        ++it; // 无效路径，跳过
                    }
                }
                if (!pmRef.ref.findPreVersion.empty())
                {
                    break; // 如果已经找到预版本，跳出外层循环
                }
            }

            if (pmRef.ref.isDeltas)
            {
                if (pmRef.ref.findPreVersion.empty())
                {
                    std::cout << _("Not install preversion of: ") << pmRef.ref.id << std::endl;
                    d->m_okFile->umountOK(mountPoint);
                    return KM_ERROR_INSTALL_DELTA_FAILED;
                }
                else
                {
                    std::string completedRef = pmRef.ref.id + "/" + pmRef.ref.version + "/" + pmRef.ref.module + "/" + pmRef.ref.channel + "/" + pmRef.ref.arch;
                    d->m_pmFuzzyMatchRefs.push_back(completedRef);
                    d->m_pmRefOfflineList.push_back(pmRef);
                    continue;
                }
            }

            std::string completedRef = pmRef.ref.id + "/" + pmRef.ref.version + "/" + pmRef.ref.module + "/" + pmRef.ref.channel + "/" + pmRef.ref.arch;
            d->m_pmFuzzyMatchRefs.push_back(completedRef);
            d->m_pmRefOfflineList.push_back(pmRef);

            if (pmRef.ref.kind != "base" && !pmRef.ref.isOfflineRun)
            {
                if (!runtime.empty())
                {
                    KMPMRef pmRuntimeRef = KMUtil::parseSummaryKMPMRef(runtime);
                    pmRuntimeRef.ref.module = layer.module;
                    std::string runtimeStr = pmRuntimeRef.ref.id + "/" + pmRuntimeRef.ref.version + "/" + layer.module + "/" + layer.channel + "/" + pmRef.ref.arch;

                    if (!d->m_parseSummaryJson->currentFuzzyRefIsInstalled(pmRuntimeRef))
                    {
                        d->m_pmOkBase.insert({pmRef.ref.id, runtimeStr});
                        onlinePackageIds.push_back(runtimeStr);
                    }
                }

                if (!base.empty())
                {
                    KMPMRef pmBaseRef = KMUtil::parseSummaryKMPMRef(base);
                    pmBaseRef.ref.module = layer.module;
                    std::string baseStr = pmBaseRef.ref.id + "/" + pmBaseRef.ref.version + "/" + layer.module + "/" + layer.channel + "/" + pmRef.ref.arch;
                    if (!d->m_parseSummaryJson->currentFuzzyRefIsInstalled(pmBaseRef))
                    {
                        d->m_pmOkBase.insert({pmRef.ref.id, baseStr});
                        onlinePackageIds.push_back(baseStr);
                    }
                }

                if (!depends.empty())
                {
                    for (std::string dependStr : depends)
                    {
                        KMPMRef pmDependRef;
                        std::vector<std::string> tokens;
                        std::stringstream ss(dependStr);
                        std::string token;
                        while (std::getline(ss, token, '/'))
                        {
                            tokens.push_back(token); // 将切分后的部分添加到 tokens 向量中
                        }

                        if (tokens.empty())
                        {
                            kmlogger.debug("Empty dependency string found: : %s", dependStr.c_str());
                            continue;
                        }

                        if (dependStr.find('/') != std::string::npos)
                        {
                            pmDependRef.ref.arch = pmRef.ref.arch;
                            pmDependRef.ref.id = tokens[0];
                            pmDependRef.ref.version = tokens.size() > 1 ? tokens[1] : "";
                            pmDependRef.ref.module = layer.module;
                            pmDependRef.ref.channel = layer.channel;

                            if (!d->m_parseSummaryJson->currentFuzzyRefIsInstalled(pmDependRef))
                            {
                                dependStr = dependStr + "/" + layer.module + "/" + layer.channel + "/" + pmRef.ref.arch;
                                d->m_pmOkBase.insert({pmRef.ref.id, dependStr});
                                onlinePackageIds.push_back(dependStr);
                            }
                        }
                        else
                        {
                            pmDependRef.ref.arch = pmRef.ref.arch;
                            pmDependRef.ref.id = tokens[0];
                            pmDependRef.ref.version = "";
                            pmDependRef.ref.module = layer.module;
                            pmDependRef.ref.channel = layer.channel;
                            if (!currentRefIsUninstalled(pmDependRef))
                            {
                                dependStr = dependStr + "/" + "-" + "/" + layer.module + "/" + layer.channel + "/" + pmRef.ref.arch;
                                d->m_pmOkBase.insert({pmRef.ref.id, dependStr});
                                onlinePackageIds.push_back(dependStr);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (auto layer : layers)
        {
            KMPMRef pmRef;
            string base = layer.base;
            string runtime = layer.runtime;

            pmRef.ref.id = layer.id;
            pmRef.ref.channel = layer.channel;
            pmRef.ref.version = layer.version;
            pmRef.ref.module = layer.module;
            pmRef.ref.kind = layer.kind;
            pmRef.installedSize = layer.size;
            pmRef.name = layer.name;
            pmRef.description = layer.description;
            pmRef.ref.isOfflineRun = offlineRun;

            for (auto arch : layer.archs)
            {
                if (arch == KMUtil::kmGetArch())
                {
                    pmRef.ref.arch = arch;
                }
            }

            if (d->m_pmIsThereDbusService)
            {
                const char *dbusmethod = "GetPackageInstallStatus";
                const char *idChar = pmRef.ref.id.c_str();
                const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiinstall";
                int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

                if (getDbusReturn == -1)
                {
                    d->m_pmIsThereDbusService = false;
                }
                if (getDbusReturn == 1)
                {
                    canNotInstall = true;
                    std::cout << "can not install " << pmRef.ref.id << std::endl;
                    return KM_ERROR_CANNOT_INSTALL;
                }
            }

            pmRef.completedRef = KMUtil::dupKMPMCompletedRef(pmRef);

            if (pmRef.ref.arch.empty())
            {
                const std::string &filePath = d->m_okFile->getOKFilePath();
                std::cout << "Unsupported architecture for file: " << filePath << std::endl;
                kmlogger.error("Unsupported architecture - File: %s", filePath.c_str());
                d->m_okFile->umountOK(mountPoint);
                return KM_ERROR_NOT_SUPPORT_ARCH;
            }

            if (currentRefIsInstalled(pmRef))
            {
                // 不强制安装已经安装的应用
                if (!pmOptions.force)
                {
                    return KM_ERROR_ALREADY_INSTALLED;
                }
            }
            d->m_pmRefOfflineList.push_back(pmRef);
        }
    }

    d->m_okFile->closeOKFile();
    return KM_ERROR_NO;
}

int KMPackageManager::uninstall(const vector<string> &vectorIds, PackageManagerOptions pmOptions)
{
    std::vector<std::string> perUninstallList; // 待删除列表
    bool isUninstallable = true;
    vector<std::string> UninstallVector;
    vector<std::string> dependVector;
    bool needDevelMap = false;
    bool needRootMap = false;
    d->m_pmReserveDepend.clear();

    // 预先确定需要加载哪些 map，避免在循环中重复加载
    for (auto name : vectorIds)
    {
        KMPMRef pmRef = d->m_util->parseKMPMRef(name);
        pmRef.ref.arch = pmOptions.arch;

        if (!currentRefIsUninstalled(pmRef))
        {
            continue;
        }

        // 检查 d->m_pmQueryVector 中的 ref，确定需要加载哪些 map
        for (auto it : d->m_pmQueryVector)
        {
            if (it.kind == "app")
            {
                continue;
            }

            if (needDevelMap && needRootMap)
            {
                break;
            }
            if (it.module == "binary")
            {
                needRootMap = true;
            }
            else
            {
                needDevelMap = true;
            }
        }
    }

    // 在循环外部一次性加载所需的 map
    if (needRootMap)
    {
        fs::path rootPath = d->m_folder->getLayersPath();
        std::unordered_map<std::string, KMInfoJson> rootMap = loadAllInfoJsonFiles(rootPath);
        m_infoMap.insert(rootMap.begin(), rootMap.end());
    }
    if (needDevelMap)
    {
        fs::path develDir = fs::path(KMStorageDir::getUserBaseDirLocation());
        std::unordered_map<std::string, KMInfoJson> develMap = loadAllInfoJsonFiles(develDir);
        m_infoMap.insert(develMap.begin(), develMap.end());
    }

    // 遍历每个待卸载的应用ID
    for (auto name : vectorIds)
    {
        kmlogger.debug("Uninstall app %s", name.c_str());
        bool isMatchedRefFlag = false;

        KMPMRef pmRef = d->m_util->parseKMPMRef(name);
        d->m_util->printKMPMRef(name, pmRef);
        pmRef.ref.arch = pmOptions.arch;

        if (!currentRefIsUninstalled(pmRef))
        {
            kmlogger.pinfo("%s not installed!", name.c_str());
            continue;
        }

        printUninstallList(pmRef);

        // 用户选择是否展示信息
        if (!printAssumeUninstallYesChoice(pmOptions, pmRef))
        {
            return KM_ERROR_USER_CANCEL;
        }

        if (d->m_pmUninstallVector[0].kind == "app")
        {
            if (d->m_pmIsThereDbusService)
            {
                const char *dbusmethod = "GetPackageUninstallStatus";
                const char *idChar = d->m_pmUninstallVector[0].id.c_str();
                const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiuninstall";
                int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);
                if (getDbusReturn == -1)
                {
                    d->m_pmIsThereDbusService = false;
                }

                if (getDbusReturn == 1)
                {
                    isUninstallable = false;
                    kmlogger.error("Can not uninstall %s", idChar);
                    std::cout << "Can not uninstall " << d->m_pmUninstallVector[0].id << std::endl;
                    return KM_ERROR_CANNOT_UNINSTALL;
                }
            }

            if (pmOptions.deletedataYes)
            {
                deleteUserData(name);
            }

            KMRef appRemoveRef = d->m_pmUninstallVector[0];
            removeUserDesktopShortcutsByMd5(appRemoveRef.id);
            int ret = deleteHostAppFiles(); // deleteHostAppFiles before removeDeployFiles
            if (ret != KM_ERROR_NO)
            {
                return ret;
            }

            string removeStr = appRemoveRef.baseInstallDir + "/" + appRemoveRef.channel + "/" + appRemoveRef.arch + "/" + appRemoveRef.kind + "/" + appRemoveRef.id + "/" + appRemoveRef.module + "/" + appRemoveRef.version;
            removeSpecificDeployFiles(removeStr);
            deleteSpecificRefsFromRepo(removeStr);

            killAppid(name);
            removePackageInfoFromList(appRemoveRef.id, appRemoveRef.version, appRemoveRef.arch, appRemoveRef.module);
        }

        vector<KMRef> unRemove = reserveVersion();

        if (d->m_pmUninstallVector[0].kind == "depend")
        {
            if (!unRemove.empty())
            {
                d->m_pmReserveDepend.push_back(unRemove[0].id);
            }

            for (auto &uninstallRef : d->m_pmUninstallVector)
            {
                bool isfound = false;
                for (auto reserveDepend : unRemove)
                {
                    KMVersion pmVersion = KMVersion::parse(reserveDepend.version, false);
                    KMVersion reVersion = KMVersion::parse(uninstallRef.version, false);
                    if (pmVersion.getMajor() == reVersion.getMajor())
                    {
                        isfound = true;
                        break;
                    }
                }

                if (isfound)
                {
                    std::string removeUninstallStr = uninstallRef.baseInstallDir + "/" + uninstallRef.channel + "/" + uninstallRef.arch + "/" + uninstallRef.kind + "/" + uninstallRef.id + "/" + uninstallRef.module + "/" + uninstallRef.version;
                    removeHostAppDeployFiles(removeUninstallStr); // remove hostapp first, need to read info.json
                    removeSpecificDeployFiles(removeUninstallStr);
                    deleteSpecificRefsFromRepo(removeUninstallStr);
                    continue;
                }

                if (!removeDepend(pmRef.ref.id, uninstallRef, UninstallVector))
                {

                    std::cout << "can not uninstall " << d->m_pmUninstallVector[0].id << std::endl;
                    break;
                }
            }

            printUninstallDependsOnList(UninstallVector);

            if (UninstallVector.empty() || isContinueUninstall())
            {
                for (auto &uninstallStr : UninstallVector)
                {
                    removeHostAppDeployFiles(uninstallStr); // remove hostapp first, need to read info.json
                    removeSpecificDeployFiles(uninstallStr);
                    deleteSpecificRefsFromRepo(uninstallStr);
                }
            }
        }

        if (d->m_pmUninstallVector[0].kind == "runtime" || d->m_pmUninstallVector[0].kind == "base")
        {

            for (auto it = d->m_pmUninstallVector.begin(); it != d->m_pmUninstallVector.end(); it++)
            {
                auto &uninstallIt = *it;
                for (auto &queryIt : unRemove)
                {
                    if (uninstallIt.arch == queryIt.arch && uninstallIt.channel == queryIt.channel && uninstallIt.module == queryIt.module)
                    {
                        if (uninstallIt.version != queryIt.version)
                        {
                            KMVersion uninstallVersion = KMVersion::parse(uninstallIt.version, false);
                            KMVersion queryVersion = KMVersion::parse(queryIt.version, false);
                            if (uninstallVersion.getMajor() == queryVersion.getMajor())
                            {
                                // directUninstallVector.push_back(uninstallIt);
                                std::string removeUninstallStr = uninstallIt.baseInstallDir + "/" + uninstallIt.channel + "/" + uninstallIt.arch + "/" + uninstallIt.kind + "/" + uninstallIt.id + "/" + uninstallIt.module + "/" + uninstallIt.version;
                                removeSpecificDeployFiles(removeUninstallStr);
                                deleteSpecificRefsFromRepo(removeUninstallStr);
                                it = d->m_pmUninstallVector.erase(it);
                                it--;
                                break;
                            }
                        }
                    }
                }
            }

            if (!d->m_pmUninstallVector.empty())
            {
                for (auto it = d->m_pmUninstallVector.begin(); it != d->m_pmUninstallVector.end(); it++)
                {
                    auto &uninstallIt = *it;

                    if (d->m_pmIsThereDbusService)
                    {
                        const char *dbusmethod = "GetPackageUninstallStatus";
                        const char *idChar = uninstallIt.id.c_str();
                        const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiuninstall";
                        int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

                        if (getDbusReturn == -1)
                        {
                            d->m_pmIsThereDbusService = false;
                        }
                        if (getDbusReturn == 1)
                        {
                            isUninstallable = false;
                            kmlogger.error("Can not uninstall %s", idChar);
                            std::cout << "can not uninstall " << uninstallIt.id << std::endl;
                            break;
                        }
                    }

                    KMVersion reVersion = KMVersion::parse(uninstallIt.version, false);
                    std::string removeUninstallStr = uninstallIt.baseInstallDir + "/" + uninstallIt.channel + "/" + uninstallIt.arch + "/" + uninstallIt.kind + "/" + uninstallIt.id + "/" + uninstallIt.module + "/" + uninstallIt.version;
                    std::string findStr = uninstallIt.channel + ":" + uninstallIt.id + "/" + std::to_string(reVersion.getMajor());
                    vector<std::string> willUninstallVector;

                    for (auto it = m_infoMap.begin(); it != m_infoMap.end();)
                    {
                        auto pair = *it;
                        KMInfoJson &info = pair.second;

                        if (info[uninstallIt.kind].find(findStr) != std::string::npos && info[uninstallIt.kind].find(uninstallIt.arch) != std::string::npos && info.module == uninstallIt.module)
                        {
                            if (d->m_pmIsThereDbusService)
                            {
                                const char *dbusmethod = "GetPackageUninstallStatus";
                                const char *idChar = info.id.c_str();
                                const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiuninstall";
                                int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

                                if (getDbusReturn == -1)
                                {
                                    d->m_pmIsThereDbusService = false;
                                }

                                if (getDbusReturn == 1)
                                {
                                    isUninstallable = false;
                                    kmlogger.error("Can not uninstall %s", idChar);
                                    std::cout << info.id << " depends on " << name << ",but " << info.id << " can not be uninstalled" << std::endl;
                                    break;
                                }
                            }

                            if (info.kind == "runtime")
                            {
                                KMVersion reVersion = KMVersion::parse(info.version, false);
                                std::string runtimeStr = info.channel + ":" + info.id + "/" + std::to_string(reVersion.getMajor());

                                for (auto secondIt = m_infoMap.begin(); secondIt != m_infoMap.end();)
                                {
                                    auto secondPair = *secondIt;
                                    KMInfoJson &secondInfo = secondPair.second;

                                    if (secondInfo[info.kind].find(runtimeStr) != std::string::npos && secondInfo[info.kind].find(uninstallIt.arch) != std::string::npos && secondInfo.module == uninstallIt.module)
                                    {
                                        if (d->m_pmIsThereDbusService)
                                        {
                                            const char *dbusmethod = "GetPackageUninstallStatus";
                                            const char *idChar = secondInfo.id.c_str();
                                            const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiuninstall";
                                            int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

                                            if (getDbusReturn == -1)
                                            {
                                                d->m_pmIsThereDbusService = false;
                                            }

                                            if (getDbusReturn == 1)
                                            {
                                                isUninstallable = false;
                                                kmlogger.error("Can not uninstall %s", idChar);
                                                std::cout << d->m_pmUninstallVector[0].id << " depends on " << name << ",but " << d->m_pmUninstallVector[0].id << " can not be uninstalled" << std::endl;
                                                break;
                                            }
                                        }

                                        std::string removeSecondStr = secondPair.first;
                                        willUninstallVector.push_back(removeSecondStr);
                                        secondIt = m_infoMap.erase(secondIt);
                                    }
                                    else
                                    {
                                        secondIt++;
                                    }
                                }

                                if (!isUninstallable)
                                {
                                    break;
                                }
                            }
                            if (info.kind == "depend")
                            {
                                KMRef dependRef;
                                dependRef.id = info.id;
                                dependRef.kind = info.kind;
                                dependRef.module = info.module;
                                dependRef.arch = d->m_pmUninstallVector[0].arch;
                                dependRef.version = info.version;
                                dependRef.channel = info.channel;
                                dependRef.baseInstallDir = fs::path(KMStorageDir::getUserBaseDirLocation());
                                isUninstallable = removeDepend(pmRef.ref.id, dependRef, willUninstallVector);

                                if (!isUninstallable)
                                {
                                    break;
                                }
                            }
                            std::string removeStr = pair.first;
                            willUninstallVector.push_back(removeStr);
                            it = m_infoMap.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }

                    if (!isUninstallable)
                    {
                        break;
                    }

                    UninstallVector.push_back(removeUninstallStr);
                    dependVector.insert(dependVector.end(), willUninstallVector.begin(), willUninstallVector.end());
                    UninstallVector.insert(UninstallVector.end(), willUninstallVector.begin(), willUninstallVector.end());
                }

                KMUtil::removeDuplicateValues(dependVector);
                KMUtil::removeDuplicateValues(UninstallVector);
                printUninstallDependsOnList(dependVector);

                if (dependVector.empty() || isContinueUninstall())
                {
                    for (auto &uninstallStr : UninstallVector)
                    {
                        removeHostAppDeployFiles(uninstallStr);
                        removeSpecificDeployFiles(uninstallStr);
                        deleteSpecificRefsFromRepo(uninstallStr);
                    }
                }
            }
        }
    }
    return KM_ERROR_NO;
}

bool KMPackageManager::removeDepend(const std::string &name, const KMRef &uninstallRef, std::vector<std::string> &UninstallVector)
{
    bool isUninstallable = true;
    if (d->m_pmIsThereDbusService)
    {
        const char *dbusmethod = "GetPackageUninstallStatus";
        const char *idChar = uninstallRef.id.c_str();
        const char *dbusInterface = "com.kylin.kysdk.applicationsec.antiuninstall";
        int getDbusReturn = d->m_parseSummaryJson->getDbus(dbusInterface, dbusmethod, idChar);

        if (getDbusReturn == -1)
        {
            d->m_pmIsThereDbusService = false;
        }
        if (getDbusReturn == 1)
        {
            isUninstallable = false;
            if (uninstallRef.id == name)
            {
                kmlogger.error("Can not uninstall %s", idChar);
                std::cout << "can not uninstall " << name << std::endl;
                return isUninstallable;
            }
            else
            {
                kmlogger.error("Can not uninstall %s", idChar);
                std::cout << uninstallRef.id << " depends on " << name << ", but " << uninstallRef.id << " can not be uninstalled" << std::endl;
            }
        }
    }

    std::string removeUninstallStr = uninstallRef.baseInstallDir + "/" + uninstallRef.channel + "/" + uninstallRef.arch + "/" + uninstallRef.kind + "/" + uninstallRef.id + "/" + uninstallRef.module + "/" + uninstallRef.version;
    auto it = std::find(UninstallVector.begin(), UninstallVector.end(), removeUninstallStr);

    if (it != UninstallVector.end())
    {
        return true;
    }
    else
    {
        UninstallVector.push_back(removeUninstallStr);
    }

    for (auto it = m_infoMap.begin(); it != m_infoMap.end();)
    {
        auto pair = *it;
        KMInfoJson &info = pair.second;
        std::string uninstallArch;
        auto uninstallArchIt = std::find(info.archs.begin(), info.archs.end(), uninstallRef.arch);

        if (uninstallArchIt != info.archs.end())
        {
            uninstallArch = uninstallRef.arch;
        }
        else
        {
            uninstallArch = info.archs[0];
        }

        if (uninstallArch != uninstallRef.arch || info.module != uninstallRef.module || info.channel != uninstallRef.channel)
        {
            it++;
            continue;
        }

        for (auto dependstr : info.annotations.depends)
        {
            if (dependstr.find(uninstallRef.id) != std::string::npos)
            {
                KMVersion uninstallVersion = KMVersion::parse(uninstallRef.version, false);
                uint64_t uninstallVersionStr = uninstallVersion.getMajor();
                std::string dependInfo = uninstallRef.id + "/" + std::to_string(uninstallVersionStr);
                KMRef dependRef;
                dependRef.id = info.id;
                dependRef.kind = info.kind;
                dependRef.module = info.module;
                dependRef.channel = info.channel;
                dependRef.arch = uninstallArch;
                dependRef.version = info.version;
                dependRef.baseInstallDir = fs::path(KMStorageDir::getUserBaseDirLocation());

                if (dependstr.find(dependInfo) != std::string::npos)
                {
                    if (!removeDepend(name, dependRef, UninstallVector))
                    {
                        isUninstallable = false;
                        return isUninstallable;
                    }
                }
                else
                {
                    if (!d->m_pmReserveDepend.empty())
                    {
                        for (auto reserver : d->m_pmReserveDepend)
                        {
                            if (reserver == uninstallRef.id)
                            {
                                d->m_pmReserveDepend.push_back(info.id);
                            }
                            else
                            {
                                if (!removeDepend(name, dependRef, UninstallVector))
                                {
                                    isUninstallable = false;
                                    return isUninstallable;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (!removeDepend(name, dependRef, UninstallVector))
                        {
                            isUninstallable = false;
                            return isUninstallable;
                        }
                    }
                }
            }
        }
        it++;
    }

    return isUninstallable;
}

void KMPackageManager::killAppid(const std::string &appId)
{
    std::string loginUser = KMStorageDir::getLoginUser();
    std::string xdgRuntimeDir = KMStorageDir::getXdgRuntimeDir(loginUser);
    if (xdgRuntimeDir.empty())
    {
        kmlogger.error("Invalid parameters user=%s", loginUser.c_str());
        return;
    }

    std::string killCommand = "sudo -u " + loginUser + " XDG_RUNTIME_DIR=" + xdgRuntimeDir + " -- /opt/kaiming-tools/bin/kaiming kill " + appId + " >/dev/null 2>&1";
    FILE *pipe = popen(killCommand.c_str(), "r");
    if (!pipe)
    {
        kmlogger.error("Failed to execute command: %s", killCommand.c_str());
        return;
    }

    char buffer[128];
    std::string commandOutput;
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        commandOutput += buffer;
    }

    int ret = pclose(pipe);
    if (ret != 0)
    {
        kmlogger.debug("kaiming kill failed (ret=%d): %s\nOutput: %s", ret, killCommand.c_str(), commandOutput.c_str());
    }
    else
    {
        kmlogger.info("kaiming kill succeeded: %s", killCommand.c_str());
    }
}

bool KMPackageManager::isContinueUninstall()
{
    bool isContinueUninstall = false;
    while (true)
    {
        std::cout << _("The apps, which depends on those packages, will be uninstalled, do you want to continue (Y or N)?");
        std::string input;
        std::getline(std::cin, input);

        // 检查是否输入 'a'
        if (input == "y" || input == "Y")
        {
            isContinueUninstall = true;
            return isContinueUninstall;
        }
        else if (input == "n" || input == "N")
        {
            return isContinueUninstall;
        }
        else
        {
            continue;
        }
    }
}

void KMPackageManager::printUninstallList(const KMPMRef &pmRef)
{
    // 初始化最大宽度
    size_t maxIdWidth = 0;
    size_t maxChannelWidth = 0;
    size_t maxArchWidth = 0;
    size_t maxModuleWidth = 0;
    size_t maxVersionWidth = 0;

    // 第一次遍历：计算最大宽度
    for (const auto &ref : d->m_pmQueryVector)
    {
        // 如果指定了版本号且不匹配，则跳过
        if (!pmRef.ref.version.empty() && pmRef.ref.version != ref.version)
        {
            continue;
        }

        maxIdWidth = std::max(maxIdWidth, ref.id.length());
        maxChannelWidth = std::max(maxChannelWidth, ref.channel.length());
        maxArchWidth = std::max(maxArchWidth, ref.arch.length());
        maxModuleWidth = std::max(maxModuleWidth, ref.module.length());
        maxVersionWidth = std::max(maxVersionWidth, ref.version.length());
    }

    // 设置最小宽度（确保不小于列标题）
    const size_t minIdWidth = 2;      // "ID" 标题长度
    const size_t minChannelWidth = 7; // "Channel" 标题长度
    const size_t minArchWidth = 4;    // "Arch" 标题长度
    const size_t minModuleWidth = 6;  // "Module" 标题长度
    const size_t minVersionWidth = 7; // "Version" 标题长度

    maxIdWidth = std::max(maxIdWidth, minIdWidth) + 5;
    maxChannelWidth = std::max(maxChannelWidth, minChannelWidth) + 5;
    maxArchWidth = std::max(maxArchWidth, minArchWidth) + 5;
    maxModuleWidth = std::max(maxModuleWidth, minModuleWidth) + 5;
    maxVersionWidth = std::max(maxVersionWidth, minVersionWidth) + 5;

    // 打印表头
    printf("\033[1;36m%-10s %-*s %-*s %-*s %-*s %-*s\033[0m\n",
           _("No."),
           static_cast<int>(maxIdWidth), _("ID"),
           static_cast<int>(maxChannelWidth), _("Channel"),
           static_cast<int>(maxArchWidth), _("Arch"),
           static_cast<int>(maxModuleWidth), _("Module"),
           static_cast<int>(maxVersionWidth), _("Version"));

    // 第二次遍历：打印数据
    int i = 1;
    // printf("\n%-10s %-30s %-15s %-15s %-15s %-15s\n", _(" "), _("ID"), _("Channel"), _("Arch"), _("Module"), _("Version"));
    const bool filterByVersion = !pmRef.ref.version.empty();

    for (const auto &ref : d->m_pmQueryVector)
    {
        // 如果指定了版本号且不匹配，则跳过
        if (filterByVersion && pmRef.ref.version != ref.version)
        {
            continue;
        }

        // 打印行数据
        printf("%-10d %-*s %-*s %-*s %-*s %-*s\n",
               i,
               static_cast<int>(maxIdWidth), ref.id.c_str(),
               static_cast<int>(maxChannelWidth), ref.channel.c_str(),
               static_cast<int>(maxArchWidth), ref.arch.c_str(),
               static_cast<int>(maxModuleWidth), ref.module.c_str(),
               static_cast<int>(maxVersionWidth), ref.version.c_str());
        i++;
    }
}

bool KMPackageManager::removeDeployFiles()
{
    std::set<std::string> allUniqueLines;

    for (auto &it : d->m_pmUninstallVector)
    {
        fs::path versionFile = it.baseInstallDir + "/" + it.channel + "/" + it.arch + "/" + it.kind + "/" + it.id + "/" + it.module + "/" + it.version;
        fs::path deployFile = versionFile / "deploy";

        if (!fs::exists(deployFile))
        {
            // std::cerr << "文件不存在: " << deployFile << std::endl; // 输出调试信息
            continue; // 跳过该文件
        }

        auto lines = KMFolder::readDeployFiles(deployFile);
        allUniqueLines.insert(lines.begin(), lines.end());
    }

    deleteFilesIfExist(allUniqueLines);
    removeEmptyDirs(allUniqueLines);

    return true;
}

int KMPackageManager::deleteHostAppFiles()
{
    for (auto &it : d->m_pmUninstallVector)
    {
        fs::path versionFile = it.baseInstallDir + "/" + it.channel + "/" + it.arch + "/" + it.kind + "/" + it.id + "/" + it.module + "/" + it.version;
        d->m_hostAppDeploy->setHostAppConfig(versionFile, it.id);
        if (d->m_hostAppDeploy->loadHostAppInfo())
        {
            int uninstallResult = d->m_hostAppDeploy->uninstall();
            if (uninstallResult != KM_ERROR_NO)
            {
                kmlogger.error("Uninstall failed for package: %s", it.id.c_str());
                return uninstallResult;
                // Continue with other packages even if one fails
            }
        }
        else
        {
            d->m_hostAppDeploy->processKillList(d->m_hostAppDeploy->getUninstallkills());
            d->m_hostAppDeploy->handleStopServices();
            d->m_hostAppDeploy->handleRmdir();
            d->m_hostAppDeploy->handleUnlink();
        }

        clearCacheFiles(it.id);

        const fs::path serviceDir = versionFile / "entries" / "etc" / "systemd" / "system" / "multi-user.target.wants";
        if (fs::exists(serviceDir))
        {
            for (const auto &entry : fs::directory_iterator(serviceDir))
            {
                if (entry.path().extension() == ".service")
                {
                    std::string serviceName = entry.path().stem().string();

                    std::string serviceCommand = "systemctl stop " + serviceName;
                    int ret = system(serviceCommand.c_str());
                    if (ret == 0)
                    {
                        kmlogger.info("Stopped service: %s", serviceName.c_str());
                    }
                    else
                    {
                        kmlogger.error("Failed to stop service: %s", serviceName.c_str());
                    }
                }
            }
        }
    }

    return KM_ERROR_NO;
}

void KMPackageManager::removeEmptyDirs(const std::set<std::string> &files)
{
    const std::set<std::string> protectedDirs = {
        "/opt/kaiming/share/applications",
        "/opt/kaiming/share/icons",
        "/opt/kaiming/exports/share/applications",
        "/opt/kaiming/exports/share/icons"};

    for (const auto &file : files)
    {
        fs::path dir = fs::path(file).parent_path();
        while (dir != "/" && fs::exists(dir))
        {
            if (protectedDirs.find(dir.string()) != protectedDirs.end())
            {
                break;
            }

            if (fs::is_empty(dir))
            {
                fs::remove(dir);
                KMDebug("Removed empty dir: " + dir.string());
                dir = dir.parent_path();
            }
            else
            {
                break;
            }
        }
    }
}

std::unordered_map<std::string, KMInfoJson> KMPackageManager::loadAllInfoJsonFiles(std::string pathStr)
{
    std::unordered_map<std::string, KMInfoJson> infoMap;
    
    try
    {
        // 第1层：stable等版本目录
        for (const auto& l1 : std::filesystem::directory_iterator(pathStr))
        {
            if (!l1.is_directory()) continue;
            
            // 第2层：架构目录（x86_64等）
            for (const auto& l2 : std::filesystem::directory_iterator(l1.path()))
            {
                if (!l2.is_directory()) continue;
                
                // 第3层：类型目录（app等）
                for (const auto& l3 : std::filesystem::directory_iterator(l2.path()))
                {
                    if (!l3.is_directory()) continue;
                    
                    // 第4层：应用目录（top.openkylin.Clock等）
                    for (const auto& l4 : std::filesystem::directory_iterator(l3.path()))
                    {
                        if (!l4.is_directory()) continue;
                        
                        // 第5层：binary目录
                        for (const auto& l5 : std::filesystem::directory_iterator(l4.path()))
                        {
                            if (!l5.is_directory()) continue;
                            
                            // 第6层：版本目录（4.0.0.0等）
                            for (const auto& l6 : std::filesystem::directory_iterator(l5.path()))
                            {
                                if (!l6.is_directory()) continue;
                                
                                // 在第6层目录中查找info.json
                                std::filesystem::path infoJsonPath = l6.path() / "info.json";
                                
                                if (exists(infoJsonPath) && is_regular_file(infoJsonPath))
                                {
                                    try
                                    {
                                        KMInfoJson infoJson;
                                        infoJson.loadFile(infoJsonPath.string());
                                        
                                        // 使用第6层目录路径作为key
                                        infoMap[l6.path().string()] = infoJson;
                                    }
                                    catch (const KMException& e)
                                    {
                                        kmlogger.perror("Failed to load %s : %s", infoJsonPath.string().c_str(), e.what());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        kmlogger.perror("Filesystem error: %s", e.what());
    }
    
    return infoMap;
}

bool KMPackageManager::printAssumeUninstallYesChoice(const PackageManagerOptions &pmOptions, const KMPMRef &pmRef)
{
    d->m_pmUninstallVector.clear();

    std::vector<KMRef> speIndices;
    if (!pmRef.ref.version.empty())
    {
        for (const auto &ref : d->m_pmQueryVector)
        {
            if (pmRef.ref.version == ref.version)
            {
                speIndices.push_back(ref);
            }
        }

        if (speIndices.empty())
        {
            std::cout << pmRef.ref.id << " " << pmRef.ref.version << " not installed" << std::endl;
            return false;
        }
    }

    const auto &targetRefs = pmRef.ref.version.empty() ? d->m_pmQueryVector : speIndices;

    if (!pmOptions.assumeYes)
    {
        while (true)
        {
            // yesOption = std::cin.get();
            std::cout << _("Enter number to delete (separated by spaces) or 'y' to delete all, enter 'n' to cancel: ");
            d->m_pmUninstallVector.clear();
            std::string input;
            std::getline(std::cin, input);

            // 检查是否输入 'a'
            if (input == "y" || input == "Y")
            {
                d->m_pmUninstallVector = targetRefs;
                break;
            }

            if (input == "n" || input == "N")
            {
                return false;
            }

            std::istringstream iss(input);
            std::vector<int> indices;
            int index;
            bool isValid = true;

            if (input.empty())
            {
                std::cout << "Input is empty" << std::endl;
                continue;
            }

            for (char ch : input)
            {
                if (!std::isdigit(ch) && ch != ' ')
                {
                    std::cout << "Invalid idx: " << input << std::endl;
                    isValid = false;
                    break;
                }
            }

            if (!isValid)
            {
                continue;
            }

            while (iss >> index)
            {
                indices.push_back(index);
            }

            indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

            for (int idx : indices)
            {
                if (idx > 0 && idx <= static_cast<int>(targetRefs.size()))
                {
                    d->m_pmUninstallVector.push_back(targetRefs[idx - 1]);
                }
                else
                {
                    std::cout << "Invalid index: " << idx << std::endl;
                    isValid = false;
                }
            }

            if (!isValid)
            {
                continue;
            }

            break;
        }
    }
    else
    {
        d->m_pmUninstallVector = targetRefs;
    }

    return !d->m_pmUninstallVector.empty();
}

vector<KMRef> KMPackageManager::reserveVersion()
{
    vector<KMRef> unRemove = d->m_pmQueryVector;
    for (auto &it : d->m_pmUninstallVector)
    {
        unRemove.erase(std::remove_if(unRemove.begin(), unRemove.end(), [&](const KMRef &nowIt)
                                      { return it.arch == nowIt.arch && it.module == nowIt.module && it.version == nowIt.version; }),
                       unRemove.end());
    }
    return unRemove;
}

bool KMPackageManager::removeDeployFiles(const std::string &name)
{
    std::vector<fs::path> idPaths;

    fs::path rootDir = d->m_folder->getLayersPath();
    fs::path develDir = fs::path(KMStorageDir::getUserBaseDirLocation());

    KMFolder::findIdPaths(rootDir, name, idPaths);
    if (fs::exists(develDir))
    {
        KMFolder::findIdPaths(develDir, name, idPaths);
    }
    if (idPaths.empty())
    {
        // std::cerr << "No ID paths found for ID: " << name << std::endl;
        return false;
    }

    std::cout << "ID paths found: " << std::endl;
    for (const auto &path : idPaths)
    {
        std::cout << "  " << path << std::endl;
    }

    // Step 2: Find deploy files in each ID path
    std::vector<fs::path> deployFiles;
    for (const auto &idPath : idPaths)
    {
        KMFolder::findDeployFiles(idPath, deployFiles);
    }

    if (deployFiles.empty())
    {
        std::cerr << "No deploy files found for ID: " << name << std::endl;
        return false;
    }

    std::cout << "Deploy files found: " << std::endl;
    for (const auto &file : deployFiles)
    {
        std::cout << "  " << file << std::endl;
    }

    // Step 3: Read deploy files and collect unique lines
    std::set<std::string> allUniqueLines;
    for (const auto &deployFile : deployFiles)
    {
        auto lines = KMFolder::readDeployFiles(deployFile);
        allUniqueLines.insert(lines.begin(), lines.end());
    }

    // Step 4: Output and delete unique lines
    deleteFilesIfExist(allUniqueLines);

    return true;
}

bool KMPackageManager::removeTmpDeployFiles(const fs::path &signPath)
{
    std::error_code ec;

    if (!fs::exists(signPath, ec))
    {
        // 目录不存在，认为清理成功
        return true;
    }

    // 递归删除目录及其所有内容
    fs::remove_all(signPath, ec);
    if (ec)
    {
        kmlogger.error("删除临时部署目录失败: %s, error: %s", signPath.c_str(), ec.message().c_str());
        return false;
    }

    kmlogger.info("已删除临时部署目录: %s", signPath.c_str());
    return true;
}

bool KMPackageManager::removeSpecificDeployFiles(const std::string &str)
{
    std::set<std::string> allUniqueLines;
    fs::path versionFile = str + "/" + "deploy";

    if (!fs::exists(versionFile))
    {
        // std::cerr << "文件不存在: " << deployFile << std::endl; // 输出调试信息
        return false; // 跳过该文件
    }

    auto lines = KMFolder::readDeployFiles(versionFile);
    allUniqueLines.insert(lines.begin(), lines.end());

    deleteFilesIfExist(allUniqueLines);
    removeEmptyDirs(allUniqueLines);
    kmlogger.info("正在卸载 completedRef: %s", str.c_str());

    return true;
}

void KMPackageManager::deleteFilesIfExist(const std::set<std::string> &fileList)
{
    for (const auto &filePath : fileList)
    {
        try
        {
            if (fs::exists(filePath) || fs::is_symlink(filePath))
            {
                std::cout << "File exists: " << filePath << ". Deleting..." << std::endl;
                fs::remove_all(filePath);
            }
            else
            {
                // std::cout << "File does not exist: " << filePath << ". Skipping..." << std::endl;
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Error processing file: " << filePath << ". Error: " << e.what() << std::endl;
        }
    }
}

bool KMPackageManager::deleteRefsFromRepo(const std::string &name)
{
    // 从所有远程仓库中删除对应应用的引用
    auto allRefs = d->m_ostreeHandler->getAllLocalRefs();
    bool deletedAny = false;
    for (const auto &remoteEntry : allRefs)
    {
        const std::string &remoteName = remoteEntry.first;
        const auto &refMap = remoteEntry.second;
        for (const auto &pair : refMap)
        {
            const KMRef &ref = pair.second;
            if (ref.id != name)
            {
                continue;
            }

            std::string refString = ref.branch + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;

            if (!d->m_ostreeHandler->deleteRefTag("", std::string("deploy/").append(refString)))
            {
                return false;
            }

            if (!d->m_ostreeHandler->deleteRefTag(remoteName, refString))
            {
                return false;
            }
            deletedAny = true;
        }
    }

    if (deletedAny)
    {
        if (!d->m_ostreeHandler->repoPrune())
        {
            return false;
        }
    }

    return true;
}

bool KMPackageManager::deleteSpecificRefsFromRepo(const std::string &str)
{
    int count = 0;
    size_t pos = str.length(); // 从末尾开始
    std::string extractedStr;
    // 从后往前找第 6 个 '/'
    while (pos != 0)
    {
        pos--;
        if (str[pos] == '/')
        {
            count++;
            if (count == 6)
            {
                extractedStr = str.substr(pos + 1); // 返回第 6 个 '/' 之后的内容
                break;
            }
        }
    }

    d->m_ostreeHandler->replaceRef(extractedStr);

    // 从所有远程仓库中删除引用
    auto allRefs = d->m_ostreeHandler->getLocalRefs();
    for (auto pair : allRefs)
    {
        std::string refString = pair.second.branch + "/" + pair.second.arch + "/" + pair.second.kind + "/" + pair.second.id + "/" + pair.second.module + "/" + pair.second.version;

        if (refString == extractedStr)
        {
            // 删除 deploy ref
            if (!d->m_ostreeHandler->deleteRefTag("", string("deploy/").append(refString)))
            {
                return false;
            }

            // 删除远程 ref（统一格式：kaiming-repo:stable/...）
            auto remotes = d->m_ostreeHandler->getAllRemotesWithPriority();
            for (const auto &repoName : remotes)
            {
                if (!d->m_ostreeHandler->deleteRefTag(repoName.name, refString)) // file:///opt/kaiming/repo/refs/remotes/kaiming-repo/stable/x86_64/app/top.openkylin.Music
                {
                    return false;
                }
            }

            // 兼容旧数据：如果还有不带 remote 前缀的本地 ref（可能是旧版本安装的），也删除
            if (!d->m_ostreeHandler->deleteRefTag("", refString))
            {
                return false;
            }

            if (!d->m_ostreeHandler->repoPrune())
            {
                return false;
            }
        }
    }

    const bool isImmutableSystem = KMFileUtils::isImmutableSystem();
    const std::string schemasFilePath = isImmutableSystem ? "/opt/system/resource/glib-2.0/schemas/" : "/usr/share/glib-2.0/schemas";
    std::string compileCommand = "glib-compile-schemas \"" + schemasFilePath + "\" > /dev/null 2>&1";
    int result = system(compileCommand.c_str());
    if (result != 0)
    {
        kmlogger.error("Failed to compile schemas in: %s", schemasFilePath.c_str());
    }

    return true;
}

bool KMPackageManager::deleteUserData(const std::string &name)
{
    printf("remove app %s user data dir.\n", name.c_str());

    if (!fs::remove_all(d->m_folder->getAppDataPath(name)))
    {
        kmlogger.error("uninstall %s error, can't delete the directory.", name.c_str());
        return false;
    }
    return true;
}

int KMPackageManager::update(const vector<string> &vectorIds, PackageManagerOptions pmOptions)
{
    // 下载和解析清单依赖文件 summary.json
    std::cout << _("Search matching packages ...") << std::endl;
    d->m_pmDownloadRefs.clear();
    d->m_pmFuzzyMatchRefs.clear();
    d->m_pmOldAVersion.clear();

    resetInstallState();

    refreshRemoteConfiguration();

    if (vectorIds.empty() ||
        std::all_of(vectorIds.begin(), vectorIds.end(), [](const std::string &str)
                    { return str.empty(); }))
    {
        // 仅更新包依赖清单文件
        return KM_ERROR_NO;
    }

    // 遍历取得所有符合条件的依赖列表
    int getFuzzyMatchState = getAllFuzzyMatchRefVector(vectorIds, pmOptions, TaskType::TASK_TYPE_UPDATE);
    if (getFuzzyMatchState != KM_ERROR_NO)
    {
        kmlogger.error("Get fuzzy match ref error, err = %d!", getFuzzyMatchState);
        return getFuzzyMatchState;
    }
    getUsefulRefList();
    // 获取真正更新列表
    getDownloadRefList(TaskType::TASK_TYPE_UPDATE, pmOptions);

    if (d->m_pmRefDownloadList.empty())
    {
        printf("%s\n", _("Not need to update!"));
        return KM_ERROR_NOT_NEED_UPDATE;
    }

    // 展示下载列表信息
    printDownloadRefList(d->m_pmRefDownloadList);

    // 用户选择是否展示信息
    if (!printAssumeYesChoice(pmOptions))
    {
        return KM_ERROR_NOT_INSTALLED;
    };

    // 下载包
    InstallTaskContext ctx;
    installing = true; // 先设置安装标志
    bool pullFlag = false;
    std::thread([this, &ctx]()
                {
                    while (installing && !ctx.cancelRequested.load() && !ctx.stopNetworkCheck.load())
                    {
                        checkAndHandleNetwork(d->m_ostreeHandler, &ctx, this);
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    } })
        .detach();

    // 开始真正的拉取
    // pullFlag = d->m_ostreeHandler->pull(REPO_NAME, d->m_pmDownloadRefs, progressCallback, pmOptions.maxSpeedLimitKilobytes);
    pullFlag = d->m_ostreeHandler->pullWithFallback(d->m_pmDownloadRefs, progressCallback, this, pmOptions.maxSpeedLimitKilobytes);
    if (pullFlag)
    {
        ctx.stopNetworkCheck.store(true);
    }

    installing = false; // 安装结束，停止监控

    if (ctx.cancelRequested.load())
    {
        return KM_ERROR_NETWORK_FAILED;
    }

    if (!pullFlag)
    {
        return KM_ERROR_PULL_FAILED;
    }

    int cleanupRet = cleanupOldRefsWithRuntimeGuard(d->m_pmOldAVersion, false);
    if (cleanupRet != KM_ERROR_NO)
    {
        return cleanupRet;
    }

    // 部署包
    return deploy(pmOptions);
}

int KMPackageManager::filterIdsNotInRemote(PackageManagerOptions pmOptions)
{
    std::vector<KMlocalAppsInfo> localAppsInfo = d->m_list->getLocalAppIds(false);

    pmOptions.arch = KMUtil::kmGetArch();

    int getSummaryState = downloadAllRemoteSummaries(!pmOptions.upgradable);
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    for (const auto &app : localAppsInfo)
    {
        std::vector<std::string> completePath = d->m_parseSummaryJson->printSearchJson(app.id, pmOptions.arch, pmOptions.kind, app.module, pmOptions.isPrint, pmOptions.isGetDevel);

        if (!completePath.empty() && !completePath[0].empty())
        {
            kmlogger.info("Currently installed app %s , completePath is : %s", app.id.c_str(), completePath[0].c_str());

            std::vector<std::string> splitPath;
            std::istringstream iss(completePath[0]);
            std::string token;
            while (std::getline(iss, token, '/'))
            {
                if (!token.empty())
                {
                    splitPath.push_back(token);
                }
            }

            std::string remoteArch, remoteId, remoteVersion, remoteModule;
            if (splitPath.size() >= 8)
            {
                remoteArch = splitPath[1];
                remoteId = splitPath[3];
                remoteModule = splitPath[4];
                remoteVersion = splitPath[5];
            }

            KMVersion parseVersion = KMVersion::parse(remoteVersion, false);
            KMVersion appVersion = KMVersion::parse(app.version, false);
            if (!remoteId.empty() && appVersion < parseVersion)
            {
                d->m_upgragableList.push_back({remoteId, remoteVersion, remoteArch, remoteModule});
            }
        }
    }

    return KM_ERROR_NO;
}

std::vector<std::string> KMPackageManager::getUpgradeInfo(PackageManagerOptions pmOptions)
{
    std::vector<KMlocalAppsInfo> localAppsInfo = d->m_list->getLocalAppIds(false);
    pmOptions.arch = KMUtil::kmGetArch();
    vector<std::string> vectorIds;

    int getSummaryState = downloadAllRemoteSummaries(true);
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
    }

    for (const auto &apps : localAppsInfo)
    {
        // 下载和解析清单依赖文件 summary.json
        pmOptions.isGetDevel = true;
        pmOptions.isPrint = false;
        std::vector<std::string> completePath = d->m_parseSummaryJson->printSearchJson(apps.id, pmOptions.arch, pmOptions.kind, apps.module, pmOptions.isPrint, pmOptions.isGetDevel);

        if (!completePath.empty() && !completePath[0].empty())
        {
            kmlogger.info("Currently installed app %s, completePath is: %s", apps.id.c_str(), completePath[0].c_str());
            std::vector<std::string> splitPath;
            std::istringstream iss(completePath[0]);
            std::string token;
            while (std::getline(iss, token, '/'))
            {
                if (!token.empty())
                {
                    splitPath.push_back(token);
                }
            }

            std::string remoteId, remoteVersion, remoModule;
            if (splitPath.size() >= 8)
            {
                remoteId = splitPath[3];
                remoteVersion = splitPath[5];
                remoModule = splitPath[4];
            }

            if (!remoteVersion.empty() && !apps.version.empty())
            {
                KMVersion parseVersion = KMVersion::parse(remoteVersion, false);
                KMVersion appVersion = KMVersion::parse(apps.version, false);
                if (!remoteId.empty() && appVersion < parseVersion)
                {
                    vectorIds.push_back(remoteId + "/" + remoteVersion + "/" + remoModule);
                }
            }
        }
        else
        {
            kmlogger.info("App %s has no complete path despite success", apps.id.c_str());
        }
    }
    return vectorIds;
}

int KMPackageManager::getUpgradableList(const std::vector<KMlocalAppsInfo> &localAppsInfo, PackageManagerOptions pmOptions)
{
    if (!pmOptions.upgradable)
    {
        // 环境校验
        if (!d->m_env->passEnvCheck())
        {
            kmlogger.error("Failed to check environments!");
            return KM_ERROR_CHECK_ENV_FAILED;
        }

        d->m_folder->ensureRepoForSudo(false);
    }

    std::vector<std::string> vectorIds;
    for (const auto &apps : localAppsInfo)
    {
        vectorIds.push_back(apps.id);
    }

    fs::path basePath = d->m_folder->getPath();

    std::vector<KMlocalAppsInfo> localInfo = localAppsInfo;
    int errorCode = filterIdsNotInRemote(pmOptions);
    if (errorCode != KM_ERROR_NO)
    {
        return errorCode;
    }
    else
    {
        if (!pmOptions.upgradable)
        {
            std::cout << _("There are ") << d->m_upgragableList.size() << _(" packages available for upgrade. Please run 'kaiming update --upgradable' to check them.") << std::endl;

            if (!d->m_upgragableList.empty())
            {
                fs::path cachePath = basePath / "cache";
                if (!fs::exists(cachePath))
                {
                    fs::create_directories(cachePath);
                }

                fs::path upgradablePath = cachePath / "upgradable.list";
                std::ofstream outfile(upgradablePath);

                if (outfile.is_open())
                {
                    for (const auto &vec : d->m_upgragableList)
                    {
                        kmlogger.info("%s is upgradable.", vec.id.c_str());
                        outfile << vec.id << ":" << vec.version << std::endl;
                    }
                    outfile.close();
                }
                else
                {
                    kmlogger.info("Failed to create %s", upgradablePath.string().c_str());
                    return KM_ERROR_CREATE_FILE_FAILED;
                }
            }
        }
        else
        {
            for (const auto &vec : d->m_upgragableList)
            {
                std::string version;

                auto it = std::find_if(localInfo.begin(), localInfo.end(),
                                       [&](const KMlocalAppsInfo &info)
                                       {
                                           return info.id == vec.id && info.module == vec.module;
                                       });

                if (it != localInfo.end())
                {
                    version = it->version;
                }
                std::cout << vec.id << " " << vec.version << " " << vec.arch << " " << vec.module << " [" << _("Can be upgraded from this version: ") << version << "]" << std::endl;
            }
        }
    }
    d->m_upgragableList.clear();
    return KM_ERROR_NO;
}

int KMPackageManager::dbusSearch(const string &strID, PackageManagerOptions pmOptions)
{
    GError *error = NULL;
    // 连接到 D-Bus 系统总线
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!connection)
    { // 连接失败
        kmlogger.error("Failed to connect to D-Bus: %s", error->message);
        g_clear_error(&error);
    }

    // 调用 getRemoteSummary 方法
    GVariant *result = g_dbus_connection_call_sync(
        connection,
        "org.kylin.kaiming",           // 服务名称
        "/org/kylin/kaiming",          // 对象路径
        "org.kylin.kaiming.interface", // 接口名称
        "getRemoteSummary",            // 方法名  与xml中的方法名一致
        NULL,                          // 传递给 getRemoteSummary 方法的参数
        // g_variant_new("(s)", vectorIds.c_str()), // 参数元组
        NULL, // 返回值类型（NULL 表示不检查）
        G_DBUS_CALL_FLAGS_NONE,
        -1,   // 超时时间,-1 表示默认超时
        NULL, // Cancellable
        &error);

    if (error)
    { // 调用失败
        kmlogger.error("Failed to call getRemoteSummary: %s", error->message);
        return KM_ERROR_CURL_DOWNLOAD_FAILED;
        g_clear_error(&error);
    }
    else
    {
        gboolean success;
        g_variant_get(result, "(b)", &success); // 假设返回值是布尔值
        kmlogger.debug("getRemoteSummary method call result: %s", success ? "Success" : "Failure");
        if (success == false)
        {
            std::cout << _("Please check your network connection!") << std::endl;
            return KM_ERROR_CURL_DOWNLOAD_FAILED;
        }
        g_variant_unref(result); // 释放资源
    }

    g_object_unref(connection); // 释放资源

    // fs::path basePath = d->m_folder->getPath();
    // fs::path summaryJsonPath = basePath / "summary.json";
    // d->m_parseSummaryJson->loadFile(summaryJsonPath);
    downloadAllRemoteSummaries();
    d->m_parseSummaryJson->printSearchJson(strID, pmOptions.arch, pmOptions.kind, pmOptions.module, pmOptions.isPrint, pmOptions.isGetDevel);

    return KM_ERROR_NO;
}

int KMPackageManager::search(const string &strID, PackageManagerOptions pmOptions)
{
    std::cout << _("Search matching packages ...") << std::endl;

    int getSummaryState = downloadAllRemoteSummaries();
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    // d->m_parseSummaryJson->loadFile(summaryJsonPath.string());
    d->m_parseSummaryJson->printSearchJson(strID, pmOptions.arch, pmOptions.kind, pmOptions.module, pmOptions.isPrint, pmOptions.isGetDevel);

    return KM_ERROR_NO;
}

int KMPackageManager::searchInfo(const string &strID, PackageManagerOptions pmOptions, std::vector<std::string> &strCompletePath)
{
    if (pmOptions.isPrint)
    {
        std::cout << _("Search matching packages ...") << std::endl;
    }

    int getSummaryState = downloadAllRemoteSummaries();
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    pmOptions.arch = KMUtil::kmGetArch();
    strCompletePath = d->m_parseSummaryJson->printSearchJson(strID, pmOptions.arch, pmOptions.kind, pmOptions.module, pmOptions.isPrint, pmOptions.isGetDevel);

    return KM_ERROR_NO;
}

int KMPackageManager::getRemoteInfo(const std::string &name, std::string &strRemoteInfoJson, std::vector<std::string> &allRemoteInfoJson, std::string &remoteKind, bool &isNameNullable)
{
    PackageManagerOptions pmOptions;
    pmOptions.relativeRepoPath = "";
    pmOptions.isPrint = false;
    pmOptions.kind = remoteKind;
    pmOptions.arch = KMUtil::kmGetArch();

    std::vector<std::string> allRemoteInfo;
    int errorCode = searchInfo(name, pmOptions, allRemoteInfo);

    bool isSearchId = false;
    if (!allRemoteInfo.empty() && !allRemoteInfo[0].empty())
    {
        isSearchId = true;
    }

    if (isSearchId == false || (!isNameNullable && name.empty()))
    {
        // 没有在summary.json中找到可匹配的id
        strRemoteInfoJson = KMDbusJson::errorRemoteJson();
        return KM_ERROR_REMOTE_NOT_FOUND;
    }

    if (errorCode != KM_ERROR_NO)
    {
        strRemoteInfoJson = KMDbusJson::errorRemoteJson();
    }
    else
    {
        if (!allRemoteInfo.empty())
        {
            allRemoteInfoJson.reserve(allRemoteInfo.size());
            for (auto &info : allRemoteInfo)
            {
                strRemoteInfoJson = KMDbusJson::parseRemotePathToJson(info);
                kmlogger.debug("remoteInfoJson: %s", strRemoteInfoJson.c_str());
                allRemoteInfoJson.push_back(strRemoteInfoJson);
            }
        }
    }

    return errorCode;
}

string KMPackageManager::getDebInfoFromOK(const string &name, int &errorCode)
{
    auto createEmptyJson = []()
    {
        nlohmann::json emptyJson = {
            {"branch", ""},
            {"id", ""},
            {"version", ""},
            {"size", ""},
            {"debinfo", {{"package", ""}, {"version", ""}}}};
        return emptyJson.dump();
    };

    d->m_okMetadata->layers.clear();
    size_t pos = name.find(".ok");
    string mountPoint = "/tmp/" + name.substr(0, pos) + "_" + std::to_string(getpid());

    d->m_okFile->setMountPoint(mountPoint);

    if (name.empty())
    {
        kmlogger.info("Name is empty");
        errorCode = KM_ERROR_NO;
        return createEmptyJson();
    }

    if (!d->m_okFile->loadOKFile(name))
    {
        kmlogger.info("Failed to load file: %s", name.c_str());
        errorCode = KM_ERROR_LOAD_FILE_FAILED;
        return createEmptyJson();
    }

    d->m_okFile->mountOK(mountPoint);
    d->m_okMetadata = d->m_okFile->getMetadata();
    std::vector<KMInfoJson> layers = d->m_okMetadata->layers;

    if (layers.empty())
    {
        kmlogger.info("No layers found in OK file!");
        errorCode = KM_ERROR_NO_LAYERS_FOUND;
        return createEmptyJson();
    }

    KMPMRef pmRef;
    string debPackage, debVersion;
    if (layers.size() == 1)
    {
        for (auto layer : layers)
        {
            debPackage = layer.annotations.debinfo.package;
            debVersion = layer.annotations.debinfo.version;

            pmRef.ref.id = layer.id;
            pmRef.ref.channel = layer.channel;
            pmRef.ref.version = layer.version;
            pmRef.installedSize = layer.size;
        }
    }

    nlohmann::json jsonNode;
    jsonNode["branch"] = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    jsonNode["id"] = pmRef.ref.id.empty() ? "" : pmRef.ref.id;
    jsonNode["version"] = pmRef.ref.version.empty() ? "" : pmRef.ref.version;
    if (pmRef.installedSize > 0)
    {
        jsonNode["size"] = std::to_string(pmRef.installedSize);
    }
    else
    {
        jsonNode["size"] = "";
    }

    nlohmann::json debInfoNode;
    debInfoNode["package"] = debPackage.empty() ? "" : debPackage;
    debInfoNode["version"] = debVersion.empty() ? "" : debVersion;
    jsonNode["debinfo"] = debInfoNode;
    string debOKJson = jsonNode.dump();

    kmlogger.info("DebInfoFromOK JSON: %s", debOKJson.c_str());

    return debOKJson;
}

int KMPackageManager::repair(const vector<string> &vectorIds, PackageManagerOptions pmOptions)
{
    vector<KMPMRef> installVector;
    d->m_pmDownloadRefs.clear();
    resetInstallState();

    for (auto name : vectorIds)
    {
        kmlogger.debug("Install app %s", name.c_str());
        bool isMatchedRefFlag = false;

        KMPMRef pmRef = d->m_util->parseKMPMRef(name);
        d->m_util->printKMPMRef(name, pmRef);

        // 如果用户指定了 arch，则覆盖默认的 arch
        if (!pmOptions.arch.empty())
        {
            pmRef.ref.arch = pmOptions.arch;
        }

        if (pmRef.refSourceType == KMPACKAGE_SOURCE_TYPE_OK)
        {
            return KM_ERROR_REPAIR_FAILED;
        }

        if (!currentRefIsUninstalled(pmRef))
        {
            std::cout << _("Not install") << name << std::endl;
        }
    }

    if (d->m_pmQueryVector.empty())
    {
        return KM_ERROR_REPAIR_FAILED;
    }
    // 完整的 ref ，如 "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0"

    // 进行损坏检测，过滤掉未损坏的应用
    auto it = d->m_pmQueryVector.begin();
    while (it != d->m_pmQueryVector.end())
    {
        KMRef ref = *(it);

        // 对修复的应用包files做md5判断，如果计算得到的md5与原md5一致则该包不损坏，跳过修复
        std::string layersPath = ref.baseInstallDir;

        std::string completedRef = ref.channel + '/' + ref.arch + '/' + ref.kind + '/' + ref.id + '/' + ref.module + '/' + ref.version;
        string removeStr = ref.baseInstallDir + "/" + completedRef;
        // 计算出files的md5
        std::string filesDirPath = KMStringUtils::buildFilename(layersPath, completedRef);
        std::string filesMd5 = KMStringUtils::buildFilename(filesDirPath, "files.md5");
        std::string filePrefix; // 读取出本地md5文件的值

        KMPMRef installRef;
        installRef.ref = ref;
        KMVersion reVersion = KMVersion::parse(ref.version, false);
        installRef.ref.version = (ref.kind == "app") ? "" : std::to_string(reVersion.getMajor());

        // 读取md5的文件不存在，该包存在修复列表中
        if (!fs::exists(filesMd5))
        {
            removeSpecificDeployFiles(removeStr);
            deleteSpecificRefsFromRepo(removeStr);
            installVector.push_back(installRef);
            removePackageInfoFromList(ref.id, ref.version, ref.arch, ref.module);
            ++it; // 迭代计算下一个文件
        }
        else
        {
            if (KMFileUtils::isObsolete(filesMd5))
            {
                std::string filesDirFiles = KMStringUtils::buildFilename(filesDirPath, "files");
                std::cout << _("Verify file MD5 checksum ") << ref.id << endl;
                std::string checksum = KMFileUtils::getDirMd5sum(filesDirFiles);

                // 读取之前判断md5是否存在
                KMFileUtils::readFile(filesMd5, filePrefix);

                // 如果本地的Md5的文件与计算出来的Md5文件相同，则该app/base/runtime则不需要修复
                if ((filePrefix == checksum) && (checksum != ""))
                {
                    it = d->m_pmQueryVector.erase(it); // 修复列表中移除当前app/base/runtime
                    std::cout << _("Repair") << ref.id << _("complete") << endl;
                }
                else
                {
                    installVector.push_back(installRef);
                    removeSpecificDeployFiles(removeStr);
                    deleteSpecificRefsFromRepo(removeStr);
                    removePackageInfoFromList(ref.id, ref.version, ref.arch, ref.module);
                    ++it; // 迭代计算下一个文件
                }
            }
            else
            {
                KMDirMd5sum md5sum(filesDirPath);
                if (md5sum.checksum("files"))
                {
                    it = d->m_pmQueryVector.erase(it); // 修复列表中移除当前app/base/runtime
                    std::cout << _("Repair") << ref.id << _("complete") << endl;
                }
                else
                {
                    installVector.push_back(installRef);
                    removeSpecificDeployFiles(removeStr);
                    deleteSpecificRefsFromRepo(removeStr);
                    removePackageInfoFromList(ref.id, ref.version, ref.arch, ref.module);
                    ++it; // 迭代计算下一个文件
                }
            }
        }
    }

    // 如果得到的修复list为空说明不需要修复则直接返回
    if (d->m_pmQueryVector.empty())
    {
        return KM_ERROR_NO;
    }

    int getSummaryState = downloadAllRemoteSummaries();
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    // 下载损坏文件
    for (auto findIt = installVector.begin(); findIt != installVector.end();)
    {
        KMPMRef pmRef = *findIt;
        vector<string> fuzzyMatchRefs;
        // 从远程依赖文件查找对应的包是否存在，并获取所有的依赖列表
        d->m_parseSummaryJson->fuzzyMatchCompletedRef(pmRef, fuzzyMatchRefs);
        string notInstallable = d->m_parseSummaryJson->getNotInstallable();
        vector<std::string> notFoundAppIds = d->m_parseSummaryJson->getNotFoundAppIds();
        d->m_parseSummaryJson->clearNotInstallable();

        if (!notInstallable.empty())
        {
            if (pmRef.ref.id == notInstallable)
            {
                std::cout << _("Can not install this package ") << pmRef.ref.id << std::endl;
            }
            else
            {
                std::cout << pmRef.ref.id << _(" Can not install the dependencies of this package:") << notInstallable << std::endl;
            }
            findIt = installVector.erase(findIt);

            continue;
        }

        if (!notFoundAppIds.empty())
        {
            // notFoundAppIds items may be in form "id/version/module/channel/arch"; compare only id prefix
            auto isSameId = [&](const std::string &entry)
            {
                auto pos = entry.find('/');
                const std::string prefix = (pos == std::string::npos) ? entry : entry.substr(0, pos);
                return prefix == pmRef.ref.id;
            };

            if (isSameId(notFoundAppIds[0]))
            {
                std::cout << _("Not find this online package ") << pmRef.ref.id << std::endl;
                kmlogger.info("Not find this online package %s", pmRef.ref.id.c_str());
            }
            else
            {
                std::cout << pmRef.ref.id << _(" Can not find the dependencies of this package:") << std::endl;
                for (const auto &it : notFoundAppIds)
                {
                    if (isSameId(it))
                    {
                        continue;
                    }

                    std::cout << it << std::endl;
                }
            }
            findIt = installVector.erase(findIt);
            continue;
        }

        findIt++;
        d->m_pmFuzzyMatchRefs.insert(d->m_pmFuzzyMatchRefs.end(), fuzzyMatchRefs.begin(), fuzzyMatchRefs.end());
    }

    KMUtil::removeDuplicateValues(d->m_pmFuzzyMatchRefs);
    // 获取必要安装列表
    getUsefulRefList();
    map<std::string, std::string> repairIdVersion;
    for (auto &ref : d->m_pmFuzzyMatchRefs)
    {
        KMPMRef pmFuzzyRef = KMUtil::parseSummaryFuzzyKMPMRef(ref);
        pmFuzzyRef.completedRef = KMUtil::dupKMPMCompletedRef(pmFuzzyRef);
        // d->m_pmDownloadRefs.clear();
        d->m_pmDownloadRefs.push_back(pmFuzzyRef.completedRef);
        d->m_parseSummaryJson->getSummaryInfo(pmFuzzyRef);
        d->m_pmRefDownloadList.push_back(pmFuzzyRef);
        refDownloadListForProgress.push_back(pmFuzzyRef);
        repairIdVersion[pmFuzzyRef.ref.id] = pmFuzzyRef.ref.version;
    }

    // 下载包
    InstallTaskContext ctx;
    installing = true; // 先设置安装标志
    bool pullFlag = false;

    std::thread([this, &ctx]()
                {
                    while (installing && !ctx.cancelRequested.load() && !ctx.stopNetworkCheck.load())
                    {
                        checkAndHandleNetwork(d->m_ostreeHandler, &ctx, this);
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    } })
        .detach();

    // 开始真正的拉取
    // pullFlag = d->m_ostreeHandler->pull(REPO_NAME, d->m_pmFuzzyMatchRefs, progressCallback, pmOptions.maxSpeedLimitKilobytes);
    pullFlag = d->m_ostreeHandler->pullWithFallback(d->m_pmFuzzyMatchRefs, progressCallback, this, pmOptions.maxSpeedLimitKilobytes);
    if (pullFlag)
    {
        ctx.stopNetworkCheck.store(true);
    }

    installing = false; // 安装结束，停止监控

    if (ctx.cancelRequested.load())
    {
        return KM_ERROR_NETWORK_FAILED;
    }

    if (!pullFlag)
    {
        return KM_ERROR_PULL_FAILED;
    }

    // 部署包
    deploy(pmOptions);
    for (auto &downloadRef : repairIdVersion)
    {
        printf("Repair  %s complete, install version : %s\n", downloadRef.first.c_str(), downloadRef.second.c_str());
    }
    return KM_ERROR_NO;
}

bool KMPackageManager::currentRefIsInstalled(KMPMRef &pmRef)
{
    KMInstalledAppQuery query;
    bool isInstalled = false;
    std::vector<KMRef> queryVector;
    string kindStr = pmRef.ref.kind.empty() ? "" : pmRef.ref.kind;
    string versionStr = pmRef.ref.version.empty() ? "" : pmRef.ref.version;
    string channelStr = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    string archStr = pmRef.ref.arch.empty() ? "" : pmRef.ref.arch;
    string moduleStr = pmRef.ref.module.empty() ? "binary" : pmRef.ref.module;

    queryVector = query.kind(kindStr).id(pmRef.ref.id).arch(archStr).channel(channelStr).version(versionStr).module(moduleStr).query(KMInstalledAppQuery::All);

    KMVersion installedVersion;
    KMVersion currentVersion;

    if (!queryVector.empty())
    {
        if (versionStr.empty())
        {
            // 未指定版本，则认为已安装
            isInstalled = true;
        }
        else
        {
            currentVersion = KMVersion::parse(versionStr, false);

            // 指定版本，则检查是否已安装
            for (const auto &ref : queryVector)
            {
                installedVersion = KMVersion::parse(ref.version, false);
                if (installedVersion == currentVersion)
                {
                    isInstalled = true;
                    break;
                }
            }
        }
    }
    return isInstalled;
}

bool KMPackageManager::currentRefVersionIsInstalled(KMPMRef &pmRef)
{
    KMInstalledAppQuery query;
    bool isInstalled = false;
    std::vector<KMRef> queryVector;
    string kindStr = pmRef.ref.kind.empty() ? "" : pmRef.ref.kind;
    string versionStr = pmRef.ref.version.empty() ? "" : pmRef.ref.version;
    string channelStr = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    string archStr = pmRef.ref.arch.empty() ? "" : pmRef.ref.arch;
    string moduleStr = pmRef.ref.module.empty() ? "binary" : pmRef.ref.module;

    queryVector = query.kind(kindStr).id(pmRef.ref.id).arch(archStr).channel(channelStr).version(versionStr).module(moduleStr).query(KMInstalledAppQuery::All);

    KMVersion installedVersion;
    KMVersion currentVersion;

    if (!queryVector.empty())
    {
        if (versionStr.empty())
        {
            // 未指定版本，则认为已安装
            // isInstalled = true;
        }
        else
        {
            currentVersion = KMVersion::parse(versionStr, false);

            // 指定版本，则检查是否已安装
            for (const auto &ref : queryVector)
            {
                installedVersion = KMVersion::parse(ref.version, false);
                if (installedVersion == currentVersion)
                {
                    isInstalled = true;
                    break;
                }
            }
        }
    }
    return isInstalled;
}

bool KMPackageManager::currentRefIsUninstalled(const KMPMRef& pmRef)
{
    KMInstalledAppQuery query;
    bool isInstalled = false;
    // std::vector<KMRef> queryVector;
    string kindStr = pmRef.ref.kind.empty() ? "" : pmRef.ref.kind;
    string versionStr = "";
    // string versionStr = pmRef.ref.version.empty() ? "" : pmRef.ref.version;
    string channelStr = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    string moduleStr = pmRef.ref.module.empty() ? "" : pmRef.ref.module;
    string archStr = pmRef.ref.arch.empty() ? "" : pmRef.ref.arch;

    d->m_pmQueryVector = query.kind(kindStr).id(pmRef.ref.id).arch(archStr).channel(channelStr).version(versionStr).module(moduleStr).query(KMInstalledAppQuery::All);

    KMVersion installedVersion;
    KMVersion currentVersion;

    if (!d->m_pmQueryVector.empty())
    {
        if (versionStr.empty())
        {
            // 未指定版本，则认为已安装
            isInstalled = true;
        }
        else
        {
            currentVersion = KMVersion::parse(versionStr, false);

            // 指定版本，则检查是否已安装
            for (const auto &ref : d->m_pmQueryVector)
            {
                installedVersion = KMVersion::parse(ref.version, false);
                if (installedVersion == currentVersion)
                {
                    isInstalled = true;
                    break;
                }
            }
        }
    }
    return isInstalled;
}

bool KMPackageManager::currentFuzzyRefIsInstalled(KMPMRef &pmRef)
{
    KMInstalledAppQuery query;
    bool isInstalled = false;
    string kindStr = pmRef.ref.kind.empty() ? "" : pmRef.ref.kind;
    string versionStr = "";
    string channelStr = pmRef.ref.channel.empty() ? "" : pmRef.ref.channel;
    string moduleStr = pmRef.ref.module.empty() ? "binary" : pmRef.ref.module;
    string archStr = pmRef.ref.arch.empty() ? "" : pmRef.ref.arch;

    std::vector<KMRef> queryVector = query.kind(kindStr).id(pmRef.ref.id).arch(archStr).channel(channelStr).version(versionStr).module(moduleStr).query(KMInstalledAppQuery::All);

    KMVersion installedVersion;
    KMVersion currentVersion = KMVersion::parse(pmRef.ref.version, false);
    bool highVersion = false;

    if (!queryVector.empty())
    {
        // 指定版本，则检查是否已安装
        for (const auto &ref : queryVector)
        {
            installedVersion = KMVersion::parse(ref.version, false);
            std::string idVersion = pmRef.ref.id + pmRef.ref.version;
            auto it = std::find(d->m_pmSpecialIdVersion.begin(), d->m_pmSpecialIdVersion.end(), idVersion);
            bool isSpecialVersion = false;
            if (it != d->m_pmSpecialIdVersion.end())
            {
                isSpecialVersion = true;
            }

            if (ref.kind == "base" || ref.kind == "runtime" || ref.kind == "depend")
            {
                if (installedVersion.getMajor() == currentVersion.getMajor())
                {
                    if (installedVersion.getMinor() == currentVersion.getMinor())
                    {
                        if (installedVersion.getPatch() == currentVersion.getPatch())
                        {
                            if (d->m_pmIsForceInstall)
                            {
                                string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                                std::cout << ref.id << " will update from version to " << pmRef.ref.version << std::endl;
                                d->m_pmOldAVersion.push_back(removeStr);
                            }
                            else
                            {
                                int maxVersion = installedVersion.compare(currentVersion);
                                if (maxVersion == 0)
                                {
                                    isInstalled = true;
                                }
                                else if (maxVersion == -1)
                                {
                                    string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                                    std::cout << ref.id << " will update from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                                    d->m_pmOldAVersion.push_back(removeStr);
                                }
                                else if (maxVersion == 1)
                                {
                                    if (pmRef.ref.isHostapp) // if ref to be installed is hostapp
                                    {
                                        string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                                        std::cout << ref.id << " will DOWNGRADE from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                                        d->m_pmOldAVersion.push_back(removeStr);
                                    }
                                    else if (isSpecialVersion == true)
                                    {
                                        if (!highVersion)
                                        {
                                            std::cout << "Warning: " << pmRef.ref.id << " Installing older version (" << pmRef.ref.version
                                                      << ") alongside existing newer version (kept for compatibility)"
                                                      << std::endl;
                                            highVersion = true;
                                        }
                                    }
                                    else
                                    {
                                        isInstalled = true;
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            if (installedVersion.getPatch() < currentVersion.getPatch())
                            {
                                string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                                std::cout << ref.id << " will update from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                                d->m_pmOldAVersion.push_back(removeStr);
                                continue;
                            }
                            else if (installedVersion.getPatch() > currentVersion.getPatch())
                            {
                                if (pmRef.ref.isHostapp)
                                {
                                    string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                                    std::cout << ref.id << " will DOWNGRADE from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                                    d->m_pmOldAVersion.push_back(removeStr);
                                }
                                else if (isSpecialVersion == true)
                                {
                                    if (!highVersion)
                                    {
                                        std::cout << "Warning: " << pmRef.ref.id << " Installing older version (" << pmRef.ref.version
                                                  << ") alongside existing newer version (kept for compatibility)"
                                                  << std::endl;
                                        highVersion = true;
                                    }
                                }
                                else
                                {
                                    isInstalled = true;
                                }
                            }
                        }
                    }
                    else if (installedVersion.getMinor() < currentVersion.getMinor())
                    {
                        string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                        std::cout << ref.id << " will update from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                        d->m_pmOldAVersion.push_back(removeStr);
                        continue;
                    }
                    else
                    {
                        if (pmRef.ref.isHostapp)
                        {
                            string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                            std::cout << ref.id << " will DOWNGRADE from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                            d->m_pmOldAVersion.push_back(removeStr);
                        }
                        else if (isSpecialVersion == true)
                        {
                            if (!highVersion)
                            {
                                std::cout << "Warning: " << pmRef.ref.id << " Installing older version (" << pmRef.ref.version
                                          << ") alongside existing newer version (kept for compatibility)"
                                          << std::endl;
                                highVersion = true;
                            }
                        }
                        else
                        {
                            isInstalled = true;
                        }
                    }
                }
            }
            else
            {
                if (installedVersion < currentVersion || d->m_pmIsForceInstall || isSpecialVersion == true)
                {
                    if (installedVersion < currentVersion)
                    {
                        std::cout << ref.id << " will update from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                    }
                    else if (installedVersion > currentVersion)
                    {
                        std::cout << ref.id << " will DOWNGRADE from version " << ref.version << " to " << pmRef.ref.version << std::endl;
                    }
                    else if (installedVersion == currentVersion)
                    {
                        std::cout << ref.id << " " << ref.version << " has been installed before !" << std::endl;
                    }

                    string removeStr = ref.baseInstallDir + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
                    d->m_pmOldAVersion.push_back(removeStr);
                }
                else
                {
                    isInstalled = true;
                    break;
                }
            }
        }
    }
    return isInstalled;
}

std::vector<std::string> KMPackageManager::extractInfoFromOK(const std::string &filepath)
{
    std::string filename = filepath.substr(filepath.find_last_of("/") + 1);
    std::vector<std::string> result;

    // 支持的架构列表
    std::vector<std::string> architectures = {"x86_64", "arm64", "loong64", "sw64", "longarch64"};

    // 查找架构位置
    size_t arch_pos = std::string::npos;
    std::string found_arch;

    for (const auto &arch : architectures)
    {
        size_t pos = filename.find(arch);
        if (pos != std::string::npos)
        {
            arch_pos = pos;
            found_arch = arch;
            break;
        }
    }

    if (arch_pos == std::string::npos)
    {
        return result;
    }

    // 确定分隔符：检查架构前面的字符
    char separator = '-';

    if (arch_pos > 0)
    {
        char prev_char = filename[arch_pos - 1];
        if (prev_char == '_')
        {
            separator = '_';
        }
    }

    // 提取ID：架构前的分隔符前面的部分
    size_t last_sep_before_arch = filename.rfind(separator, arch_pos - 1);
    std::string id;

    if (last_sep_before_arch != std::string::npos)
    {
        id = filename.substr(0, last_sep_before_arch);
    }
    else
    {
        // 如果没有找到分隔符，取架构前的所有内容
        id = filename.substr(0, arch_pos);
        // 如果id以分隔符结尾，去除分隔符
        if (!id.empty() && (id.back() == '-' || id.back() == '_'))
        {
            id.pop_back();
        }
    }

    // 架构后面的部分（包含版本号、channel、module等）
    std::string remaining = filename.substr(arch_pos + found_arch.length());
    if (!remaining.empty() && (remaining[0] == '-' || remaining[0] == '_'))
    {
        remaining = remaining.substr(1); // 跳过分隔符
    }

    // 找到倒数第一个分隔符的位置（module之后）
    size_t last_sep = remaining.find_last_of("-_");
    if (last_sep == std::string::npos)
    {
        return result; // 没有足够的分隔符
    }

    // 找到倒数第二个分隔符的位置（channel之后，module之前）
    size_t second_last_sep = remaining.find_last_of("-_", last_sep - 1);
    if (second_last_sep == std::string::npos)
    {
        return result; // 没有足够的分隔符
    }

    // 找到倒数第三个分隔符的位置（版本号之后，channel之前）
    size_t third_last_sep = remaining.find_last_of("-_", second_last_sep - 1);
    if (third_last_sep == std::string::npos)
    {
        // 如果没有倒数第三个分隔符，说明版本号从架构后开始到channel前
        third_last_sep = 0;
    }
    else
    {
        third_last_sep = third_last_sep + 1;
    }

    // 提取各部分信息
    std::string module = remaining.substr(second_last_sep + 1, last_sep - second_last_sep - 1);
    std::string channel = remaining.substr(third_last_sep, second_last_sep - third_last_sep);
    std::string version = remaining.substr(0, third_last_sep > 0 ? third_last_sep - 1 : second_last_sep);

    // 如果third_last_sep为0，说明版本号从开始到channel前
    if (third_last_sep == 0)
    {
        version = remaining.substr(0, second_last_sep);
    }

    // 去除version两端可能的分隔符
    if (!version.empty())
    {
        if (version[0] == '-' || version[0] == '_')
        {
            version = version.substr(1);
        }
        if (!version.empty() && (version.back() == '-' || version.back() == '_'))
        {
            version.pop_back();
        }
    }

    // 去除channel两端可能的分隔符
    if (!channel.empty())
    {
        if (channel[0] == '-' || channel[0] == '_')
        {
            channel = channel.substr(1);
        }
        if (!channel.empty() && (channel.back() == '-' || channel.back() == '_'))
        {
            channel.pop_back();
        }
    }

    if (!id.empty() && !found_arch.empty() && !channel.empty() && !module.empty())
    {
        result.push_back(id);
        result.push_back(found_arch);
        result.push_back(version);
        result.push_back(channel);
        result.push_back(module);
    }

    return result;
}

int KMPackageManager::isInstalled(string name)
{
    kmlogger.debug("Check wait for installing app %s", name.c_str());

    KMPMRef pmRef = KMUtil::parseKMPMRef(name);

    if (pmRef.refSourceType == KMPACKAGE_SOURCE_TYPE_OK)
    {
        std::vector<std::string> result = extractInfoFromOK(name);
        if (result.size() >= 5)
        {
            pmRef.ref.id = result[0];
            pmRef.ref.arch = result[1];
            pmRef.ref.version = result[2];
            pmRef.ref.channel = result[3];
            pmRef.ref.module = result[4];
        }
        else
        {
            kmlogger.error("Invalid result size from extractInfoFromOK: %zu", result.size());
            return KM_ERROR_NOT_INSTALLED;
        }
    }

    if (pmRef.ref.version.find('_') != std::string::npos)
    {
        return KM_ERROR_NOT_INSTALLED;
    }

    if (currentRefIsInstalled(pmRef))
    {
        kmlogger.info("%s is installed", name.c_str());
        return KM_ERROR_NO;
    }

    return KM_ERROR_NOT_INSTALLED;
}

int KMPackageManager::isUpgradabled(string name)
{
    kmlogger.debug("Check wait for update app %s", name.c_str());

    KMPMRef pmRef = KMUtil::parseKMPMRef(name);

    // 不安装,则不更新
    if (!currentRefIsInstalled(pmRef))
    {
        kmlogger.info("%s is not installed", name.c_str());
        return KM_ERROR_NOT_INSTALLED;
    }

    kmlogger.info("开始在远程仓库中查找 ref 对应的最新 commitid ...");

    // 从远程依赖文件查找对应的包是否存在，并获取所有的依赖列表
    bool isMatchedRefFlag = false;
    isMatchedRefFlag = d->m_parseSummaryJson->fuzzyMatchCompletedRef(pmRef, d->m_pmFuzzyMatchRefs);
    d->m_parseSummaryJson->clearNotInstallable();
    if (!isMatchedRefFlag)
    {
        std::cout << _("Not find this online ref ") << name << std::endl;
        return KM_ERROR_REF_NOT_FOUND;
    }

    for (const auto &ref : d->m_pmFuzzyMatchRefs)
    {
        kmlogger.debug("%s", ref.c_str());
    }

    // 依赖列表去重
    KMUtil::removeDuplicateValues(d->m_pmFuzzyMatchRefs);

    vector<string> pmMatchRefs;
    for (const auto &ref : d->m_pmFuzzyMatchRefs)
    {
        if (KMStringUtils::contains(ref, pmRef.ref.id))
        {
            pmMatchRefs.push_back(ref);
        }
    }

    // only one : "stable/x86_64/app/top.openkylin.Music/binary/2.0.1.0"
    for (const auto &ref : pmMatchRefs)
    {
        KMPMRef pmUpdateRef = KMUtil::parseSummaryFuzzyKMPMRef(ref);

        kmlogger.debug("%s", ref.c_str());

        if (!currentFuzzyRefIsInstalled(pmUpdateRef))
        {
            kmlogger.info("need update %s", ref.c_str());
            return KM_ERROR_NO;
        }
    }

    printf("%s\n", _("This app not need update!"));
    return KM_ERROR_NOT_NEED_UPDATE;
}

int KMPackageManager::getAllUpgradableStatus(const std::vector<std::string> &names, std::vector<std::pair<std::string, bool>> &results)
{
    kmlogger.info("Begin batch check upgradable status for %zu packages", names.size());

    results.clear();
    results.reserve(names.size());

    // 一次性下载和解析summary.json
    // string ostreeRepoUrl = d->m_ostreeHandler->getRemoteUrl(REPO_NAME);
    // fs::path summaryUrl = fs::path(ostreeRepoUrl) / "summary.json";

    // int getSummaryState = getRemoteSummary(summaryUrl.string(), false);
    // if (getSummaryState != KM_ERROR_NO)
    // {
    //     kmlogger.error("Get and parse summary.json file error!");
    //     // 即使summary.json获取失败，也要为每个包返回结果
    //     for (const auto &name : names)
    //     {
    //         results.push_back({name, false});
    //     }
    //     return getSummaryState;
    // }

    int overall_error = KM_ERROR_NO;

    for (const auto &name : names)
    {
        kmlogger.debug("Check upgradable status for package: %s", name.c_str());
        KMPMRef pmRef = KMUtil::parseKMPMRef(name);

        // 检查是否已安装
        if (!currentRefIsInstalled(pmRef))
        {
            kmlogger.info("%s is not installed", name.c_str());
            results.push_back({name, false});
            continue;
        }

        // 清空之前的匹配结果
        d->m_pmFuzzyMatchRefs.clear();

        // 从远程依赖文件查找对应的包是否存在
        bool isMatchedRefFlag = d->m_parseSummaryJson->fuzzyMatchCompletedRef(pmRef, d->m_pmFuzzyMatchRefs);
        d->m_parseSummaryJson->clearNotInstallable();
        if (!isMatchedRefFlag)
        {
            kmlogger.info("Not find this online ref: %s", name.c_str());
            results.push_back({name, false});
            continue;
        }

        // 依赖列表去重
        KMUtil::removeDuplicateValues(d->m_pmFuzzyMatchRefs);

        vector<string> pmMatchRefs;
        for (const auto &ref : d->m_pmFuzzyMatchRefs)
        {
            if (KMStringUtils::contains(ref, pmRef.ref.id))
            {
                pmMatchRefs.push_back(ref);
            }
        }

        // 检查是否需要更新
        bool needUpdate = false;
        for (const auto &ref : pmMatchRefs)
        {
            KMPMRef pmUpdateRef = KMUtil::parseSummaryFuzzyKMPMRef(ref);

            if (!currentFuzzyRefIsInstalled(pmUpdateRef))
            {
                kmlogger.info("Need update for %s", ref.c_str());
                needUpdate = true;
                break;
            }
        }

        results.push_back({name, needUpdate});

        if (!needUpdate)
        {
            kmlogger.info("%s does not need update", name.c_str());
        }
    }

    kmlogger.info("Batch check completed, %zu packages processed", results.size());
    return overall_error;
}

int KMPackageManager::getRemoteSummary(const string &summaryUrl, bool isExpired)
{
    static bool hasNetworkError = false;
    fs::path basePath = d->m_folder->getPath();
    fs::path summaryJsonPath = basePath / "summary.json";
    bool needDownload = false;
    bool needReload = false;

    kmlogger.debug("Begin to curl %s", summaryUrl.c_str());

    if (!fs::exists(summaryJsonPath))
    {
        kmlogger.info("summary.json does not exist, will download");
        fs::create_directories(basePath);
        needDownload = true;
    }
    else if (isExpired)
    {
        kmlogger.info("summary.json is expired, will update");
        needDownload = true;
    }
    else if (!d->m_util->isJsonFile(summaryJsonPath.string()))
    {
        kmlogger.error("Existing summary.json is not valid, will re-download");
        needDownload = true;
    }

    // Download if needed
    if (needDownload)
    {
        if (!KMCurl::curlApiDownload(summaryUrl, summaryJsonPath.string()))
        {
            kmlogger.error("Curl download failed. SummaryUrl: %s, Path: %s", summaryUrl.c_str(), summaryJsonPath.string().c_str());
            if (!hasNetworkError)
            {
                std::cout << _("Please check your network connection!") << std::endl;
                hasNetworkError = true;
            }
            return KM_ERROR_CURL_DOWNLOAD_FAILED;
        }

        if (!d->m_util->isJsonFile(summaryJsonPath.string()))
        {
            kmlogger.error("Downloaded summary.json is not valid");
            std::cout << _("summary.json is not valid, please check repo config URL") << std::endl;
            return KM_ERROR_SUMMARY_JSON_INVALID;
        }

        needReload = true;
    }

    // Load and parse the file
    if (d->m_parseSummaryJson->loadFile(summaryJsonPath.string(), needReload) != KM_ERROR_NO)
    {
        kmlogger.error("Failed to load summary.json");
        return KM_ERROR_LOAD_FILE_FAILED;
    }

    return KM_ERROR_NO;
}

int KMPackageManager::getRemoteSummary(const string &summaryUrl, const string &localPath, bool isExpired)
{
    fs::path summaryJsonPath = localPath;
    bool needDownload = false;
    bool needReload = false;

    kmlogger.debug("Begin to curl %s", summaryUrl.c_str());

    if (!fs::exists(summaryJsonPath))
    {
        kmlogger.info("summary.json does not exist, will download");
        fs::create_directories(summaryJsonPath.parent_path());
        needDownload = true;
    }
    else if (isExpired)
    {
        kmlogger.info("summary.json is expired, will update");
        needDownload = true;
    }
    else if (!d->m_util->isJsonFile(summaryJsonPath.string()))
    {
        kmlogger.error("Existing summary.json is not valid, will re-download");
        needDownload = true;
    }

    // Download if needed
    if (needDownload)
    {
        if (!KMCurl::curlApiDownload(summaryUrl, summaryJsonPath.string()))
        {
            kmlogger.error("Curl download failed. SummaryUrl: %s, Path: %s", summaryUrl.c_str(), summaryJsonPath.string().c_str());
            std::cout << KMErrorMsg::getErrorMessage(KM_ERROR_CURL_DOWNLOAD_FAILED) << std::endl;

            return KM_ERROR_CURL_DOWNLOAD_FAILED;
        }

        if (!d->m_util->isJsonFile(summaryJsonPath.string()))
        {
            kmlogger.error("Downloaded summary.json is not valid");
            std::cout << _("summary.json is not valid, please check repo config URL") << std::endl;
            return KM_ERROR_SUMMARY_JSON_INVALID;
        }

        needReload = true;
    }

    // Load and parse the file
    if (d->m_parseSummaryJson->loadFile(summaryJsonPath.string(), needReload) != KM_ERROR_NO)
    {
        kmlogger.error("Failed to load summary.json");
        return KM_ERROR_LOAD_FILE_FAILED;
    }

    return KM_ERROR_NO;
}

int KMPackageManager::downloadAllRemoteSummaries(bool isForce)
{
    auto remotes = d->m_ostreeHandler->getAllRemotesWithPriority();
    bool hasSuccess = false;
    bool anyError = false;

    for (const auto &remote : remotes)
    {
        std::string url = d->m_ostreeHandler->getRemoteUrl(remote.name);
        if (url == "-")
            continue;

        fs::path summaryUrl = fs::path(url) / "summary.json";
        fs::path localSummary = d->m_folder->getPath() / ("summary_" + remote.name + ".json");

        bool isSummaryJsonExpired = KMFileUtils::isFileExpired(localSummary, 3600);
        int ret = getRemoteSummary(summaryUrl.string(), localSummary.string(), isSummaryJsonExpired || isForce);
        if (ret == KM_ERROR_NO)
        {
            hasSuccess = true;
        }
        else
        {
            kmlogger.error("Get and parse summary.json for remote %s failed, err = %d!", remote.name.c_str(), ret);
            anyError = true;
        }
    }

    if (hasSuccess)
    {
        return KM_ERROR_NO;
    }
    else
    {
        return KM_ERROR_LOAD_FILE_FAILED;
    }
}

int KMPackageManager::getAllFuzzyMatchRefVector(const vector<string> &vectorIds, PackageManagerOptions pmOptions, TaskType taskType)
{
    d->m_parseSummaryJson->clearRepoToRefs();

    for (const auto &name : vectorIds)
    {
        kmlogger.debug("Install app %s", name.c_str());
        bool isMatchedRefFlag = false;

        KMPMRef pmRef = d->m_util->parseKMPMRef(name);
        d->m_util->printKMPMRef(name, pmRef);

        // 如果用户指定了 arch，则覆盖默认的 arch
        if (!pmOptions.arch.empty())
        {
            pmRef.ref.arch = pmOptions.arch;
        }

        if (!pmOptions.kind.empty())
        {
            pmRef.ref.kind = pmOptions.kind;
        }

        if (!pmRef.ref.version.empty() && pmOptions.compatible == false)
        {
            pmRef.ref.isSpecialVersion = true;

            if (!d->m_isOffInstall)
            {
                d->m_pmSpecialIdVersion.push_back(pmRef.ref.id + pmRef.ref.version);
            }
        }

        pmRef.ref.isForceInstall = pmOptions.force;
        d->m_pmInstallId.push_back(pmRef.ref.id);

        switch (taskType)
        {
        case TaskType::TASK_TYPE_INSTALL:
            // 已安装当前软件包或指定版本包则不再安装
            if (currentRefVersionIsInstalled(pmRef))
            {
                if (!pmOptions.force)
                {
                    std::cout << _("This package has been installed! ") << pmRef.ref.id << std::endl;
                    return KM_ERROR_ALREADY_INSTALLED;
                }
                kmlogger.info("当前软件包 %s 已安装", name.c_str());
                continue;
            }

            break;
        case TaskType::TASK_TYPE_UPDATE:
            // 未安装该包或该包指定版本,则不更新，直接返回
            if (!currentRefIsUninstalled(pmRef))
            {
                if (pmRef.ref.version.empty())
                {
                    kmlogger.info("未安装该软件包 %s", name.c_str());
                    std::cout << _("Not install this ref ") << name << std::endl;
                    return KM_ERROR_REF_NOT_INSTALLED;
                }
            }
            break;
        case TaskType::TASK_TYPE_UNINSTALL:
            // 未安装该包或该包指定版本,则不更新，直接返回
            if (!currentRefIsUninstalled(pmRef))
            {
                kmlogger.error("%s not installed!", name.c_str());
                printf("%s not installed!", name.c_str());
                return KM_ERROR_REF_NOT_INSTALLED;
            }
            break;
        case TaskType::TASK_TYPE_REPAIR:
            // 未安装该包或该包指定版本,则不修复，直接返回
            if (!currentRefIsUninstalled(pmRef))
            {
                kmlogger.error("%s not installed!", name.c_str());
                printf("%s not installed!", name.c_str());
                return KM_ERROR_REF_NOT_INSTALLED;
            }
            break;
        default:
            break;
        }

        vector<string> fuzzyMatchRefs;
        // 从远程依赖文件查找对应的包是否存在，并获取所有的依赖列表
        isMatchedRefFlag = d->m_parseSummaryJson->fuzzyMatchCompletedRef(pmRef, fuzzyMatchRefs);
        string notInstallable = d->m_parseSummaryJson->getNotInstallable();
        vector<std::string> notFoundAppIds = d->m_parseSummaryJson->getNotFoundAppIds();
        d->m_parseSummaryJson->clearNotInstallable();
        bool installed = d->m_isOffInstall ? d->m_parseSummaryJson->currentFuzzyRefIsInstalled(pmRef) : currentRefIsInstalled(pmRef);

        if (!notInstallable.empty())
        {
            if (!installed)
            {
                d->m_pmCanNotInstallVector.push_back(name);

                if (pmRef.ref.id == notInstallable)
                {
                    std::cout << _("Can not install this package ") << name << std::endl;
                    kmlogger.info("Can not install this package: %s", name.c_str());
                    return KM_ERROR_CANNOT_INSTALL;
                }
                else
                {
                    std::cout << name << _(" Can not install the dependencies of this package:") << notInstallable << std::endl;
                    kmlogger.info("Can not install the dependencies of package %s: %s", name.c_str(), notInstallable.c_str());
                    return KM_ERROR_REF_NOT_INSTALLED;
                }
            }
            else
            {
                std::cout << _("This package has been installed! ") << pmRef.ref.id << std::endl;
                kmlogger.info("This package has been installed: %s", pmRef.ref.id.c_str());
            }

            continue;
        }
        if (!notFoundAppIds.empty())
        {
            if (!installed)
            {
                d->m_pmCanNotInstallVector.push_back(name);

                auto isSameId = [&](const std::string &entry)
                {
                    auto pos = entry.find('/');
                    const std::string prefix = (pos == std::string::npos) ? entry : entry.substr(0, pos);
                    return prefix == pmRef.ref.id;
                };

                if (isSameId(notFoundAppIds[0]))
                {
                    std::cout << _("Not find this online package ") << name << std::endl;
                    kmlogger.info("Not find this online package: %s", name.c_str());
                    return KM_ERROR_REF_NOT_FOUND;
                }
                else
                {
                    std::cout << name << _(" Can not find the dependencies of this package:") << std::endl;
                    kmlogger.info("Can not find the dependencies of package: %s", name.c_str());
                    for (const auto &it : notFoundAppIds)
                    {
                        if (isSameId(it))
                        {
                            continue;
                        }

                        std::cout << it << std::endl;
                        kmlogger.info("Missing dependency: %s", it.c_str());
                    }

                    return KM_ERROR_DEPENDENCY_NOT_FOUND;
                }
            }
            else
            {
                std::cout << _("This package has been installed! ") << pmRef.ref.id << std::endl;
                kmlogger.info("This package has been installed: %s", pmRef.ref.id.c_str());
            }

            continue;
        }

        d->m_pmFuzzyMatchRefs.insert(d->m_pmFuzzyMatchRefs.end(), fuzzyMatchRefs.begin(), fuzzyMatchRefs.end());

        for (const auto &ref : d->m_pmFuzzyMatchRefs)
        {
            kmlogger.debug("%s", ref.c_str());
        }
    }

    // 依赖列表去重
    KMUtil::removeDuplicateValues(d->m_pmFuzzyMatchRefs);
    return KM_ERROR_NO;
}

int KMPackageManager::getCompletedRef(std::string name, std::string version, std::string branch, std::vector<std::string> &completeRef)
{
    std::string module = "binary";
    if (branch.empty())
    {
        branch = "stable";
    }
    std::string ref = name + "/" + version + "/" + module + "/" + branch;
    KMPMRef pmRef = d->m_util->parseKMPMRef(ref);

    // 下载和解析清单依赖文件 summary.json
    int getSummaryState = downloadAllRemoteSummaries();
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file failed, error code: %d", getSummaryState);
        return getSummaryState;
    }

    // 构建完整的 ostree ref
    pmRef.ref.isSpecialVersion = true;
    bool isMatchedRefFlag = d->m_parseSummaryJson->fuzzyMatchCompletedRef(pmRef, completeRef);
    d->m_parseSummaryJson->clearNotInstallable();

    if (!isMatchedRefFlag)
    {
        return KM_ERROR_GET_REF_INFO_FAILED;
    }
    return KM_ERROR_NO;
}

int KMPackageManager::getRemoteStatusInfo(const std::string name, const std::string version, const std::string branch, std::string &packInfoToJson, int &errorCode)
{
    errorCode = KM_ERROR_NO;
    packInfoToJson = KMDbusJson::packRemoteStatusToJson("", "", "");

    if (name.empty() || version.empty())
    {
        kmlogger.info("Name or version is empty, they are provided for remote status check.");
        return KM_ERROR_INVALID_ARGS;
    }

    /* TODO: 获取软件包的签名状态*/
    bool signStatus = true;
    std::string signCN = "麒麟软件有限公司";

    std::vector<std::string> completeRef;
    errorCode = getCompletedRef(name, version, branch, completeRef);

    std::string updateTime;
    if (errorCode == KM_ERROR_NO && !completeRef.empty())
    {
        std::string ref = completeRef.front();
        if (!ref.empty())
        {
            // updateTime = d->m_ostreeHandler->getBranchTimestamp(ref);
            updateTime = d->m_ostreeHandler->getBranchTimestamp(ref.c_str());
        }
    }

    packInfoToJson = KMDbusJson::packRemoteStatusToJson(updateTime, signStatus, signCN);
    kmlogger.debug("Remote status info for package %s is: %s", name.c_str(), packInfoToJson.c_str());

    return errorCode;
}

void KMPackageManager::getUsefulRefList()
{
    string preRef;
    for (auto it = d->m_pmFuzzyMatchRefs.begin(); it != d->m_pmFuzzyMatchRefs.end(); it++)
    {
        string ref = *it;
        if (preRef.empty())
        {
            preRef = ref;
            continue;
        }
        else
        {
            KMPMRef pmFuzzyRef = KMUtil::parseSummaryFuzzyKMPMRef(ref);
            KMPMRef pmPreRef = KMUtil::parseSummaryFuzzyKMPMRef(preRef);
            if (pmFuzzyRef.ref.id == pmPreRef.ref.id && pmFuzzyRef.ref.kind == pmPreRef.ref.kind && pmFuzzyRef.ref.module == pmPreRef.ref.module && pmFuzzyRef.ref.arch == pmPreRef.ref.arch && pmFuzzyRef.ref.channel == pmPreRef.ref.channel)
            {
                if (pmFuzzyRef.ref.kind == "app")
                {
                    if (KMVersion::parse(pmFuzzyRef.ref.version) > KMVersion::parse(pmPreRef.ref.version))
                    {
                        auto findFuzzyIt = std::find(d->m_pmFuzzyMatchRefs.begin(), d->m_pmFuzzyMatchRefs.end(), preRef);
                        if (findFuzzyIt != d->m_pmFuzzyMatchRefs.end())
                        {
                            it = d->m_pmFuzzyMatchRefs.erase(findFuzzyIt);
                        }
                        preRef = ref;
                        continue;
                    }
                }
                else
                {
                    std::istringstream currentVersion(pmFuzzyRef.ref.version);
                    std::istringstream preVersion(pmPreRef.ref.version);
                    int major, minor, patch, secure;
                    int preMajor, preMinor, prePatch, preSecure;
                    char dot;
                    currentVersion >> major >> dot >> minor >> dot >> patch;
                    preVersion >> preMajor >> dot >> preMinor >> dot >> prePatch;
                    if (major != preMajor || minor != preMinor)
                    {
                        preRef = ref;
                        continue;
                    }
                    else
                    {
                        if (patch >= prePatch)
                        {
                            auto findFuzzyIt = std::find(d->m_pmFuzzyMatchRefs.begin(), d->m_pmFuzzyMatchRefs.end(), preRef);
                            if (findFuzzyIt != d->m_pmFuzzyMatchRefs.end())
                            {
                                std::string idVersion = pmPreRef.ref.id + pmPreRef.ref.version;
                                auto idVersionIt = std::find(d->m_pmSpecialIdVersion.begin(), d->m_pmSpecialIdVersion.end(), idVersion);
                                if (idVersionIt == d->m_pmSpecialIdVersion.end())
                                {
                                    it = d->m_pmFuzzyMatchRefs.erase(findFuzzyIt);
                                }
                            }
                            preRef = ref;
                            continue;
                        }
                    }
                }
            }
            else
            {
                preRef = ref;
            }
        }
    }
    return;
}

void KMPackageManager::getDownloadRefList(TaskType taskType, PackageManagerOptions pmOptions)
{
    d->m_pmRefDownloadList.clear();
    refDownloadListForProgress.clear();

    for (auto &ref : d->m_pmFuzzyMatchRefs)
    {
        KMPMRef pmFuzzyRef = KMUtil::parseSummaryFuzzyKMPMRef(ref);
        switch (taskType)
        {
        case TaskType::TASK_TYPE_INSTALL:
            // 安装时，过滤掉已安装的软件
            if (currentFuzzyRefIsInstalled(pmFuzzyRef) && !pmOptions.bundle)
            {
                std::string idVersion = pmFuzzyRef.ref.id + pmFuzzyRef.ref.version;
                auto it = std::find(d->m_pmSpecialIdVersion.begin(), d->m_pmSpecialIdVersion.end(), idVersion);
                if (it == d->m_pmSpecialIdVersion.end())
                {
                    auto idIt = std::find(d->m_pmInstallId.begin(), d->m_pmInstallId.end(), pmFuzzyRef.ref.id);

                    if (idIt != d->m_pmInstallId.end())
                    {
                        std::cout << _("This package has been installed! ") << pmFuzzyRef.ref.id << std::endl;
                    }

                    continue;
                }
            }
            break;
        case TaskType::TASK_TYPE_UPDATE:
            // 更新时也是过滤掉已安装的软件
            if (currentFuzzyRefIsInstalled(pmFuzzyRef))
            {
                continue;
            }
            break;
        case TaskType::TASK_TYPE_REPAIR:
            // 修复时不过滤掉已安装的软件
            if (!currentFuzzyRefIsInstalled(pmFuzzyRef))
            {
                KMPMRef pmRef = pmFuzzyRef;
                pmRef.ref.version = "";
                if (!currentRefIsInstalled(pmRef))
                {
                    continue;
                }
            }
            break;
        default:
            break;
        }

        kmlogger.debug("Installing ref : %s", ref.c_str());

        // 再次构建完整的 ostree ref
        pmFuzzyRef.completedRef = KMUtil::dupKMPMCompletedRef(pmFuzzyRef);
        // d->m_pmDownloadRefs.clear();
        d->m_pmDownloadRefs.push_back(pmFuzzyRef.completedRef);
        pmFuzzyRef.ref.isSpecialVersion = true;
        d->m_parseSummaryJson->getSummaryInfo(pmFuzzyRef);

        d->m_pmRefDownloadList.push_back(pmFuzzyRef);
        refDownloadListForProgress.push_back(pmFuzzyRef);
    }
}

bool KMPackageManager::printUninstallDependsOnList(const vector<std::string> &UninstallVector)
{
    if (UninstallVector.empty())
    {
        return false;
    }

    // 初始化最大宽度
    size_t maxIdWidth = 0;
    size_t maxModuleWidth = 0;
    size_t maxVersionWidth = 0;
    size_t maxChannelWidth = 0;

    // 第一次遍历：计算最大宽度
    for (auto &uninstallIt : UninstallVector)
    {
        std::stringstream ss(uninstallIt);
        std::string token;
        vector<std::string> tokens;

        while (std::getline(ss, token, '/'))
        {
            tokens.push_back(token);
        }

        int tokensSize = tokens.size();
        if (tokensSize >= 6)
        {
            maxIdWidth = std::max(maxIdWidth, tokens[tokensSize - 3].length());
            maxModuleWidth = std::max(maxModuleWidth, tokens[tokensSize - 2].length());
            maxVersionWidth = std::max(maxVersionWidth, tokens[tokensSize - 1].length());
            maxChannelWidth = std::max(maxChannelWidth, tokens[tokensSize - 6].length());
        }
    }

    // 设置最小宽度（确保不小于列标题）
    const size_t minIdWidth = 2;      // "ID" 标题长度
    const size_t minModuleWidth = 6;  // "Module" 标题长度
    const size_t minVersionWidth = 7; // "Version" 标题长度
    const size_t minChannelWidth = 7; // "Channel" 标题长度

    maxIdWidth = std::max(maxIdWidth, minIdWidth) + 5;
    maxModuleWidth = std::max(maxModuleWidth, minModuleWidth) + 5;
    maxVersionWidth = std::max(maxVersionWidth, minVersionWidth) + 5;
    maxChannelWidth = std::max(maxChannelWidth, minChannelWidth) + 5;

    d->m_formatter->setColumnWidth(maxIdWidth, maxModuleWidth, maxVersionWidth, maxChannelWidth, 25, 25, 30, 25, 35, 35);
    d->m_formatter->setShowArch(false);
    d->m_formatter->setShowSize(false);
    d->m_formatter->setShowName(false);
    d->m_formatter->setShowDescription(false);
    d->m_formatter->setShowChannel(true);
    d->m_formatter->printHeader();

    bool foundAnyEntries = false;
    bool printHeader = false;

    // 第二次遍历：打印数据
    for (auto &uninstallIt : UninstallVector)
    {
        std::stringstream ss(uninstallIt);
        std::string token;
        vector<std::string> tokens;

        while (std::getline(ss, token, '/'))
        {
            tokens.push_back(token); // 将切分后的部分添加到 tokens 向量中
        }

        int tokensSize = tokens.size();
        KMEntry entry;
        entry.id = tokens[tokensSize - 3];
        entry.module = tokens[tokensSize - 2];
        entry.version = tokens[tokensSize - 1];
        entry.channel = tokens[tokensSize - 6];
        d->m_formatter->printEntry(entry);
    }

    if (!foundAnyEntries)
    {
        // std::cout << "No matching packages found for uninstallation.\n";
        return false;
    }

    d->m_formatter->printFooter();

    return foundAnyEntries;
}

void KMPackageManager::printDownloadRefList(const vector<KMPMRef> &downloadRefs)
{
    // 初始化最大宽度
    size_t maxIdWidth = 0;
    size_t maxNameWidth = 0;
    size_t maxVersionWidth = 0;

    // 第一次遍历：计算最大宽度
    for (const auto &pmRef : downloadRefs)
    {
        maxIdWidth = std::max(maxIdWidth, pmRef.ref.id.length());
        maxNameWidth = std::max(maxNameWidth, pmRef.name.length());
        maxVersionWidth = std::max(maxVersionWidth, pmRef.ref.version.length());
    }

    // 设置最小宽度（确保不小于列标题）
    const size_t minIdWidth = 10;      // "ID" 标题长度
    const size_t minNameWidth = 20;    // "Name" 标题长度
    const size_t minVersionWidth = 15; // "Version" 标题长度

    maxIdWidth = std::max(maxIdWidth, minIdWidth) + 5;
    maxNameWidth = std::max(maxNameWidth, minNameWidth) + 5;
    maxVersionWidth = std::max(maxVersionWidth, minVersionWidth) + 5;

    // 设置列宽（id, name, version 使用动态宽度，其他列保持固定宽度）
    d->m_formatter->setColumnWidth(maxIdWidth, maxNameWidth, maxVersionWidth, 10, 25, 25, 30, 25, 35, 35);
    d->m_formatter->setShowArch(false);
    d->m_formatter->setShowChannel(true);
    d->m_formatter->printHeader();

    // 第二次遍历：实际打印
    for (const auto &pmRef : downloadRefs)
    {
        string installedSize = KMUtil::formatInstalledSize((double)pmRef.installedSize);

        KMEntry entry;
        entry.id = pmRef.ref.id;
        entry.name = pmRef.name;
        entry.description = pmRef.description;
        entry.module = pmRef.ref.module;
        entry.version = pmRef.ref.version;
        entry.arch = pmRef.ref.arch;
        entry.channel = pmRef.ref.channel;
        entry.size = installedSize;

        d->m_formatter->printEntry(entry);
    }

    d->m_formatter->printFooter();
}

bool KMPackageManager::printAssumeYesChoice(PackageManagerOptions pmOptions)
{
    if (!pmOptions.assumeYes)
    {
        std::cout << _("Do you want to install these changes? [Y/n]: ");

        char yesOption;

        while (true)
        {
            yesOption = std::cin.get();
            
            if (yesOption != '\n')
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            // 检查是否遇到 EOF(Ctrl+D)
            if (std::cin.eof())
            {
                std::cout << _("\nInput ended unexpectedly. Cancel install.\n");
                return false;
            }

            if (d->m_util->isYes(yesOption))
            {
                break;
            }
            else if (d->m_util->isNo(yesOption))
            {
                std::cout << _("Cancel install.\n");
                return false;
            }
            else
            {
                std::cout << _("Do you want to install these changes? [Y/n]: ");
            }
        }
    }

    return true;
}

int KMPackageManager::getTerminalWidth()
{
    struct winsize w{};
    if (!isatty(STDOUT_FILENO) || ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0)
    {
        kmlogger.info("Failed to get terminal width, defaulting to 80.");
        return 80;
    }
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

// static gint64 networkDisconnectedTime = 0;
// static bool isNetworkDisconnected = false;
// static bool isPrintDisconnected = false;
// static bool isPrintFailed = false;

void KMPackageManager::checkAndHandleNetwork(std::shared_ptr<KMOSTreeHandler> handler, InstallTaskContext *ctx, gpointer userData)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
    
    // 如果 manager 为空，跳过网络检查（并发安装时可能没有 manager）
    if (!manager) {
        kmlogger.debug("checkAndHandleNetwork: manager is null, skipping network check");
        return;
    }
    
    int networkStatus = checkNetworkConnection(handler);
    const int failThreshold = 5; // 5 次 × 0.5s = 2.5 秒

    if (networkStatus == -1)
    {
        // ping 不存在，跳过网络检查（默认网络正常）
        kmlogger.info("无法检测网络(ping 不存在)，继续执行");
        return; // 直接返回，不处理网络断开逻辑
    }
    else if (networkStatus == 0)
    {
        // 网络瞬间不可达，先增加连续失败计数
        manager->networkFailCount++;

        // 连续失败次数不足，认为是网络抖动，不算断网
        if (manager->networkFailCount < failThreshold)
        {
            return;
        }

        // 连续失败次数达到阈值，认为网络真正断开
        if (!manager->isNetworkDisconnected)
        {
            // 第一次检测到网络断开
            manager->networkDisconnectedTime = g_get_monotonic_time();
            manager->isNetworkDisconnected = true;

            clearOldProgress(manager);
            std::cout << _("Network disconnected, trying to reconnect...") << std::endl;
            kmlogger.info("网络已断开，正在尝试重连...");
        }
        else
        {
            guint64 disconnectedDuration = (g_get_monotonic_time() - manager->networkDisconnectedTime) / G_USEC_PER_SEC;
            // kmlogger.info("当前时间 %ld", manager->networkDisconnectedTime/G_USEC_PER_SEC);
            kmlogger.info("网络断开，已持续 %ld 秒", disconnectedDuration);

            if (disconnectedDuration > 10)
            {
                handler->cancel();
                std::cout << "\n" << std::endl;
                KMError(_("Network connection failed. Check network settings or retry."));
                ctx->cancelRequested.store(true);
                ctx->networkBroken.store(true);
            }
        }
    }
    else
    {
        manager->networkFailCount = 0;
        // 网络恢复逻辑：只有当之前确实判定为断网时才输出恢复提示
        if (manager->isNetworkDisconnected)
        {
            manager->networkDisconnectedTime = 0;
            kmlogger.info("网络恢复,重置标志位");
            ctx->cancelRequested.store(false);
            ctx->networkBroken.store(false);
            manager->isNetworkDisconnected = false;
            clearOldProgress(manager);
            std::cout << _("Network connection restored, resuming installation...") << std::endl;
        }
    }
}

// 安全 strlen，避免 NULL 且限制最大长度
static inline size_t safe_strnlen(const char *s, size_t maxlen)
{
    if (!s)
        return 0;
    const char *p = s;
    size_t count = 0;
    while (*p && count < maxlen)
    {
        p++;
        count++;
    }
    return count;
}

/**
 * @brief 进度处理回调函数
 *
 * 进度条处理分成3个步骤：
 *  - 1. 当拉取 metadata 时， 增长到 5%;
 *  - 2. 下载数据一直到 97%;
 *  - 3. 写入 ostree objects 为最后的 3%
 *
 * @param progressStatus 在 ostree pull 时，发送的进度等信息结构体
 */
void KMPackageManager::progressCallback(ProgressStatus progressStatus, gpointer userData)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);

    if (!manager || progressStatus.ref.empty())
    {
        kmlogger.error("progressCallback: manager is null or ref is empty");
        return;
    }

    bool sizeMode = false;
    guint64 totalDownloadSize = 0;
    guint64 completedBytes = 0;
    std::unordered_map<std::string, guint64> refSizeMap;

    if (manager->d->m_ostreeHandler) {
        sizeMode = manager->d->m_ostreeHandler->getSizeMode();
        if (sizeMode) {
            totalDownloadSize = manager->d->m_ostreeHandler->getTotalDownloadSize();
            completedBytes = manager->d->m_ostreeHandler->getCompletedBytes();
            refSizeMap = manager->d->m_ostreeHandler->getRefSizeMap();
        }
    }

    kmlogger.debug("enter installProgressCallback, current ref: %s", progressStatus.ref.c_str());
    char buf[1024 * 8] = {0};
    guint64 total = 0;
    guint64 elapsedTime;
    guint newProgress = 0;
    gboolean estimating = false;
    guint64 totalTransferred;
    gboolean lastWasMetadata = progressStatus.lastWasMetadata;
    gchar *formattedBytesTotalTransferred = NULL;
    gchar *formattedBytesTotal = NULL;
    gchar *formattedBytesSec = NULL;
    gchar *speed = NULL;
    gboolean isDBusCall;

    guint totalPercent = 0;       // 总进度
    uint64_t currentProgress = 0; // 当前的进度
    std::string currentRefId;

    int i;
    int fullSize, partial;
    int barLength;
    const char *partialBlocks[] = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"};
    const char *fullBlock = "█";

    int rows = 24, cols = 80;

    elapsedTime = (g_get_monotonic_time() - progressStatus.startTime) / G_USEC_PER_SEC;
    kmlogger.debug("elapsedTime: %ld", elapsedTime);

    if (sizeMode && totalDownloadSize > 0)
    {
        if (*progressStatus.ostreeStatus)
        {
            // 当ostree_status有反馈，则代表fetch成功，当前ref进度到了100%
            kmlogger.debug("install ostree status receive pull success");
            newProgress = 100;
            progressStatus.progress = 100;
            totalPercent = 100;
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;

            kmlogger.debug("SIZE_MODE: ref %s completed successfully", progressStatus.ref.c_str());
        }
        else
        {
            guint64 currentRefSize = 0;
            auto it = refSizeMap.find(progressStatus.ref);
            if (it != refSizeMap.end())
            {
                currentRefSize = it->second;
            }

            guint64 currentRefTransferred = progressStatus.bytesTransferred;
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;

            // 确保不超过大小
            if (currentRefTransferred > currentRefSize)
            {
                currentRefTransferred = currentRefSize;
            }

            newProgress = (currentRefTransferred * 100) / currentRefSize;
            currentRefId = progressStatus.ref;
            kmlogger.debug("SIZE_MODE: ref=%s transferred=%ld size=%ld progress=%d%%", progressStatus.ref.c_str(), currentRefTransferred, currentRefSize, newProgress);

            guint64 totalCompleted = completedBytes + progressStatus.bytesTransferred;

            if (totalDownloadSize > 0)
            {
                totalPercent = (totalCompleted * 100) / totalDownloadSize;
                if (totalPercent > 100)
                    totalPercent = 100;
            }

            progressStatus.progress = totalPercent;
        }
    }
    else
    {
        if (*progressStatus.ostreeStatus)
        {
            // 当ostree_status 有反馈，则代表fetch成功，总进度到了100%
            kmlogger.debug("install ostree status receive pull success");
            newProgress = 100;
            progressStatus.progress = 100;
        }
        else
        {
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;
            formattedBytesTotalTransferred = g_format_size_full(totalTransferred, G_FORMAT_SIZE_IEC_UNITS);

            progressStatus.lastWasMetadata = false;

            if (progressStatus.totalDeltaParts == 0 && (progressStatus.outstandingMetadataFetches > 0 || lastWasMetadata) && progressStatus.metadataFetched < 20)
            {
                /* 在没有完成 metadata 处理时，我们需要执行2次回调函数，
    -            * 因为有时会遇到仅处理元数据会触发调用，但却没进行处理和添加更多元数据。*/
                if (progressStatus.outstandingMetadataFetches > 0)
                {
                    progressStatus.lastWasMetadata = true;
                }

                /* At this point we don't really know how much data there is, so we have to make a
    -            * guess. Since its really hard to figure out early how much data there is we report 1%
    -            * until all objects are scanned. */
                estimating = true;
                snprintf(buf, sizeof(buf), _("Downloading metadata: %u/(estimating) %s"), progressStatus.fetched, formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "");
                kmlogger.debug("Downloading metadata buf: %s", buf);

                /* 增长到 5% 直到所有元数据都被拉取了 */
                newProgress = 0;
                if (progressStatus.requested > 0)
                {
                    newProgress = progressStatus.fetched * 5 / progressStatus.requested;
                }
            }
            else
            {
                if (progressStatus.totalDeltaParts > 0)
                {
                    /* 仅仅使用 deltas, 因此我们可以忽略 regular objects 来获得更好的 sizes;
    -                 *
    -                 * fetchedDeltaPartSize 所有 delta 部分 和 fallback objects 参数的总大小，
    -                 * 它是在最开始可以获得的，因此不用下载。
    -                 */
                    total = progressStatus.totalDeltaPartSize - progressStatus.fetchedDeltaPartSize + progressStatus.totalExtraDataBytes;
                    formattedBytesTotal = g_format_size_full(total, G_FORMAT_SIZE_IEC_UNITS);

                    snprintf(buf, sizeof(buf), _("Downloading: %s/%s"), formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "", formattedBytesTotal ? formattedBytesTotal : "");
                    kmlogger.debug("Downloading buf: %s", buf);
                }
                else
                {
                    /* Non-deltas, 因此我们不能知道除了 object 计数和额外数据的字节大小以外的任何东西，
    -                      为了比较它们和额外数据，我们使用平均对象大小来评估总大小。*/
                    double averageObjectSize = 1;
                    if (progressStatus.fetched > 0)
                    {
                        averageObjectSize = progressStatus.bytesTransferred / (double)progressStatus.fetched;
                    }
                    total = averageObjectSize * progressStatus.requested + progressStatus.totalExtraDataBytes;
                    formattedBytesTotal = g_format_size_full(total, G_FORMAT_SIZE_IEC_UNITS);

                    snprintf(buf, sizeof(buf), _("Downloading extra data: %s/%s"), formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "", formattedBytesTotal ? formattedBytesTotal : "");
                    kmlogger.debug("下载额外数据 buf: %s", buf);

                    snprintf(buf, sizeof(buf), _("Downloading files: %d/%d %s"), progressStatus.fetched, progressStatus.requested, formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "");
                    kmlogger.debug("下载文件: %s", buf);
                }

                if (total > 0)
                {
                    newProgress = 5 + ((totalTransferred / (gdouble)total) * 92); // 这个数据会因下载额外数据导致不准，比如会从 75 降到 48
                }
                // else
                // {
                //     newProgress = 97;
                // }

                /* 增加最后写入对象的 3% 进度 */
                newProgress += getWriteProgress(progressStatus.outstandingWrites);
            }

            if (elapsedTime > 0)
            {
                formattedBytesSec = g_format_size(totalTransferred / elapsedTime);
                snprintf(buf, sizeof(buf), " (%s/s)", formattedBytesSec ? formattedBytesSec : "");
                kmlogger.debug("totalTransferred: %ld elapsedTime: %ld buf: %s", totalTransferred, elapsedTime, buf);
            }
        }

        kmlogger.info("newProgress: %d manager->currentPercent: %d", newProgress, manager->currentPercent);
        if (newProgress < manager->currentPercent)
        {
            newProgress = manager->currentPercent;
        }
        else
        {
            if (newProgress == 100)
            {
                manager->currentPercent = 0;
            }
            else
            {
                manager->currentPercent = newProgress;
            }
        }

        progressStatus.lastTotal = total;
        kmlogger.debug("newProgress: %d\n", newProgress);
        if (newProgress > 100)
        {
            if (!progressStatus.reportedOverflow)
            {
                kmlogger.error("Unexpectedly got > 100%% progress, limiting");
            }
            progressStatus.reportedOverflow = true;
            newProgress = 100;
        }

        guint singlePercent = newProgress; // 单一软件包下载进度，这个单一进度会因下载额外数据导致不准，比如会从 75 降到 48

        currentRefId = progressStatus.ref; // 赋值为当前 ref
        const std::string normalizedCurrentRef = normalizeRefForCompare(progressStatus.ref);

        uint64_t totalSize = getTotalSize(manager);
        uint64_t singleSize = getSingleSize(normalizedCurrentRef, currentRefId, manager);

        // ------------------------------------------------------------
        // 对 runtime/base 使用 fetched/requested 作为拉取进度
        // 防止在 ostree 内部统计还未稳定前，单包进度/总进度过早冲到 90%+
        // 导致进度停留在99%很长时间
        // ------------------------------------------------------------
        bool isDependencyRef = false;
        bool isDependencyFirstInstall = false; // 仅在第一次安装依赖时，才使用基准 denom
        {
            std::string currentRefForCheck = progressStatus.ref;
            for (const auto &item : manager->refDownloadListForProgress)
            {
                std::string pmCompleteRef = item.completedRef;
                KMOSTreeHandler::replaceRef(pmCompleteRef);
                if (currentRefForCheck == pmCompleteRef)
                {
                    if (item.ref.kind == "runtime" || item.ref.kind == "base")
                    {
                        isDependencyRef = true;
                        bool isInstalled = manager->currentRefIsUninstalled(item);
                        isDependencyFirstInstall = !isInstalled;
                    }

                    break;
                }
            }
        }

        // 仅在第一次安装特定依赖包时，使用硬编码的基准 fetched 值来限制进度
        // 如果是更新, ostree是增量更新，基准值方式不准确
        if (isDependencyRef && isDependencyFirstInstall && newProgress != 100)
        {
            // fetched/requested 作为拉取进度百分比
            // 避免 requested 在拉取过程中不断增大导致进度虚高。

            const guint TOP_OPENKYLIN_BASE_EXPECTED = 23652; // top.openkylin.base 最终 fetched 基准值大小
            const guint TOP_OPENKYLIN_UKUI_EXPECTED = 43950; // top.openkylin.ukui 最终 fetched 基准值大小
            const guint KYLIN_V10SP1_BASE_EXPECTED = 64956;  // cn.kylin.v10sp1-base 最终 fetched 基准值大小

            guint64 denom = 0;

            std::string refStr = progressStatus.ref;
            if (refStr.find("top.openkylin.ukui") != std::string::npos)
            {
                denom = TOP_OPENKYLIN_UKUI_EXPECTED;
            }
            else if (refStr.find("cn.kylin.v10sp1-base") != std::string::npos)
            {
                denom = KYLIN_V10SP1_BASE_EXPECTED;
            }
            else if (refStr.find("top.openkylin.base") != std::string::npos)
            {
                denom = TOP_OPENKYLIN_BASE_EXPECTED;
            }

            if (denom > 0)
            {
                guint fetchedRequestedPercent = (guint)((progressStatus.fetched * 100) / denom);
                if (fetchedRequestedPercent > 100)
                    fetchedRequestedPercent = 100;

                // 使用更保守的进度：不要让 singlePercent 超过 fetched/denom
                singlePercent = std::min(singlePercent, fetchedRequestedPercent);
            }
        }

        // 判断是否是DBus调用（通过检查globalProgressCallback和mainAppNameForDBus）
        isDBusCall = manager->globalProgressCallback && !manager->mainAppNameForDBus.empty();

        for (const auto &item : manager->refDownloadListForProgress)
        {
            kmlogger.debug("completedRef: %s size:%ld ", item.completedRef.c_str(), item.installedSize);
            if (normalizeRefForCompare(item.completedRef) == normalizedCurrentRef)
            {
                currentProgress += (double)singlePercent / 100 * singleSize;
                break;
            }
            currentProgress += item.installedSize;
        }
        totalPercent = (double)currentProgress * 100 / totalSize;

        kmlogger.info("singlePercent:%d singleSize: %ld currentProgress: %ld  totalSize: %ld totalPercent: %d", singlePercent, singleSize, currentProgress, totalSize, totalPercent);

        progressStatus.progress = totalPercent;
        progressStatus.estimating = estimating;
    }

    int terminalWidth = getTerminalWidth();
    barLength = std::max(10, terminalWidth - 25); // 修正：下限保护
    fullSize = (barLength * progressStatus.progress) / 100;
    partial = (((barLength * progressStatus.progress) % 100) * G_N_ELEMENTS(partialBlocks)) / 100;

    if (terminalWidth != manager->lastWidth)
    {
        manager->lastWidth = terminalWidth;
    }

    char str[1024 * 8] = {0};

    // 构建静态信息部分
    std::string currentStaticInfo = "正在下载 " + progressStatus.ref;;

    if (currentStaticInfo != manager->lastStaticInfo || !manager->staticInfoDisplayed)
    {
        clearOldProgress(manager);
        std::cout << currentStaticInfo << "\n";
        manager->lastStaticInfo = currentStaticInfo;
        manager->staticInfoDisplayed = true;
    }
    snprintf(str, sizeof(str), "[");

    // 进度条拼接，防止溢出
    size_t pos = safe_strnlen(str, sizeof(str) - 1);
    for (i = 0; i < barLength && pos < (int)sizeof(str) - 2; i++)
    {
        str[pos++] = (i < fullSize) ? '#' : '.';
    }
    str[pos] = '\0';

    size_t curLen = safe_strnlen(str, sizeof(str) - 1);
    snprintf(str + curLen, sizeof(str) - curLen, "] %3d%%", totalPercent);

    // 速度显示
    if (elapsedTime > 0 && totalPercent < 100)
    {
        if (!formattedBytesSec)
            formattedBytesSec = g_format_size(totalTransferred / elapsedTime);
        if (formattedBytesSec)
        {
            speed = g_strdup_printf("%s/s", formattedBytesSec);
            // 强制替换小写k为大写K
            char* k_pos = strstr(speed, "kB/s");
            if (k_pos) 
            {
                k_pos[0] = 'K'; // 把"kB/s" → "KB/s"
            }
            size_t curLen = safe_strnlen(str, sizeof(str) - 1);
            snprintf(str + curLen, sizeof(str) - curLen, " %s", speed);
        }
    }
    else
    {
        speed = g_strdup_printf("5 MB/s");
        size_t curLen = safe_strnlen(str, sizeof(str) - 1);
        snprintf(str + curLen, sizeof(str) - curLen, " %s", speed);
    }

    clearOldProgress(manager);
    std::cout << str << std::flush;

    manager->lastOutput = std::string(str);
    manager->lastProgressStr = manager->lastOutput;

    // kmlogger.debug("currentRef: %s lastRef: %s", progressStatus.ref.c_str(), progressStatus.lastRef.c_str());
    KMOSTreeHandler::replaceRef(progressStatus.lastRef);

    if (progressStatus.ref == progressStatus.lastRef && *progressStatus.ostreeStatus)
    {
        totalPercent = 100;
        printf("\n");
    }

    manager->globalProgressStatus.speed = speed ? std::string(speed) : "5 MB/s";
    // DBus调用时，refId应该是主包的ID，而不是依赖的ID
    if (isDBusCall && !manager->mainAppNameForDBus.empty())
    {
        // 查找主包的refId
        bool foundMainPackage = false;
        for (const auto &item : manager->refDownloadListForProgress)
        {
            if (item.ref.id == manager->mainAppNameForDBus)
            {
                manager->globalProgressStatus.refId = item.ref.id;
                foundMainPackage = true;
                break;
            }
        }
        if (!foundMainPackage)
        {
            manager->globalProgressStatus.refId = currentRefId;
        }
    }
    else
    {
        manager->globalProgressStatus.refId = currentRefId;
    }
    manager->globalProgressStatus.progress = totalPercent;
    manager->globalProgressStatus.timeout = 5;

    kmlogger.debug("install progress------>>:  refId:%s progress:%d speed:%s timeout:%d", manager->globalProgressStatus.refId.c_str(), manager->globalProgressStatus.progress, manager->globalProgressStatus.speed.c_str(), manager->globalProgressStatus.timeout);

    manager->progressJson = KMDbusJson::packProgressInfoToJson(manager->globalProgressStatus.refId, manager->globalProgressStatus.progress, manager->globalProgressStatus.speed, manager->globalProgressStatus.timeout);

    updateProgress(manager, false);

    // 内存释放
    if (formattedBytesTotalTransferred)
        g_free(formattedBytesTotalTransferred);
    if (formattedBytesTotal)
        g_free(formattedBytesTotal);
    if (formattedBytesSec)
        g_free(formattedBytesSec);
    if (speed)
        g_free(speed);

    fflush(stdout);
}

/**
 * @brief 带上下文的进度处理回调函数（支持并发安装）
 *
 * 每个任务有独立的上下文，进度更新会调用该任务的 progressCallback
 * 
 * @param progressStatus 在 ostree pull 时，发送的进度等信息结构体
 * @param context 任务上下文，包含该任务的下载列表、进度回调等
 */
void KMPackageManager::progressCallbackWithContext(ProgressStatus progressStatus, std::shared_ptr<DownloadTaskContext> context)
{
    if (!context || progressStatus.ref.empty())
    {
        kmlogger.error("progressCallbackWithContext: context is null or ref is empty");
        return;
    }

    kmlogger.debug("enter progressCallbackWithContext, taskId: %s, current ref: %s",
                   context->taskId.c_str(), progressStatus.ref.c_str());

    bool sizeMode = false;
    guint64 totalDownloadSize = 0;
    guint64 completedBytes = 0;
    std::unordered_map<std::string, guint64> refSizeMap;

    if (context->tmpHandler)
    {
        sizeMode = context->tmpHandler->getSizeMode();
        if (sizeMode)
        {
            totalDownloadSize = context->tmpHandler->getTotalDownloadSize();
            completedBytes = context->tmpHandler->getCompletedBytes();
            refSizeMap = context->tmpHandler->getRefSizeMap();
        }
    }

    char buf[1024 * 8] = {0};
    guint64 total = 0;
    guint64 elapsedTime;
    guint newProgress = 0;
    gboolean estimating = false;
    guint64 totalTransferred;
    gboolean lastWasMetadata = progressStatus.lastWasMetadata;
    gchar *formattedBytesTotalTransferred = NULL;
    gchar *formattedBytesTotal = NULL;
    gchar *formattedBytesSec = NULL;
    gchar *speed = NULL;

    guint totalPercent = 0;       // 总进度
    uint64_t currentProgress = 0; // 当前的进度

    elapsedTime = (g_get_monotonic_time() - progressStatus.startTime) / G_USEC_PER_SEC;
    kmlogger.debug("elapsedTime: %ld", elapsedTime);

    if (sizeMode && totalDownloadSize > 0)
    {
        if (*progressStatus.ostreeStatus)
        {
            // 当ostree_status有反馈，则代表fetch成功，当前ref进度到了100%
            kmlogger.debug("install ostree status receive pull success");
            newProgress = 100;
            progressStatus.progress = 100;
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;

            kmlogger.debug("SIZE_MODE: ref %s completed successfully", progressStatus.ref.c_str());
        }
        else
        {
            guint64 currentRefSize = 0;
            auto it = refSizeMap.find(progressStatus.ref);
            if (it != refSizeMap.end())
            {
                currentRefSize = it->second;
            }

            guint64 currentRefTransferred = progressStatus.bytesTransferred;
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;

            // 确保不超过大小
            if (currentRefTransferred > currentRefSize)
            {
                currentRefTransferred = currentRefSize;
            }

            newProgress = (currentRefTransferred * 100) / currentRefSize;
            kmlogger.info("SIZE_MODE: ref=%s transferred=%ld size=%ld progress=%d%%", progressStatus.ref.c_str(), currentRefTransferred, currentRefSize, newProgress);

            guint64 totalCompleted = completedBytes + progressStatus.bytesTransferred;

            if (totalDownloadSize > 0)
            {
                totalPercent = (totalCompleted * 100) / totalDownloadSize;
                if (totalPercent > 100)
                    totalPercent = 100;
            }

            progressStatus.progress = totalPercent;
        }
    }
    else
    {
        if (*progressStatus.ostreeStatus)
        {
            // 当ostree_status 有反馈，则代表fetch成功，总进度到了100%
            kmlogger.debug("install ostree status receive pull success");
            newProgress = 100;
            progressStatus.progress = 100;
        }
        else
        {
            totalTransferred = progressStatus.bytesTransferred + progressStatus.transferredExtraDataBytes;
            formattedBytesTotalTransferred = g_format_size_full(totalTransferred, G_FORMAT_SIZE_IEC_UNITS);

            progressStatus.lastWasMetadata = false;

            if (progressStatus.totalDeltaParts == 0 && (progressStatus.outstandingMetadataFetches > 0 || lastWasMetadata) && progressStatus.metadataFetched < 20)
            {
                if (progressStatus.outstandingMetadataFetches > 0)
                {
                    progressStatus.lastWasMetadata = true;
                }

                estimating = true;
                snprintf(buf, sizeof(buf), _("Downloading metadata: %u/(estimating) %s"),
                         progressStatus.fetched, formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "");
                kmlogger.debug("Downloading metadata buf: %s", buf);

                newProgress = 0;
                if (progressStatus.requested > 0)
                {
                    newProgress = progressStatus.fetched * 5 / progressStatus.requested;
                }
            }
            else
            {
                if (progressStatus.totalDeltaParts > 0)
                {
                    total = progressStatus.totalDeltaPartSize - progressStatus.fetchedDeltaPartSize + progressStatus.totalExtraDataBytes;
                    formattedBytesTotal = g_format_size_full(total, G_FORMAT_SIZE_IEC_UNITS);

                    snprintf(buf, sizeof(buf), _("Downloading: %s/%s"),
                             formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "",
                             formattedBytesTotal ? formattedBytesTotal : "");
                    kmlogger.debug("Downloading buf: %s", buf);
                }
                else
                {
                    double averageObjectSize = 1;
                    if (progressStatus.fetched > 0)
                    {
                        averageObjectSize = progressStatus.bytesTransferred / (double)progressStatus.fetched;
                    }
                    total = averageObjectSize * progressStatus.requested + progressStatus.totalExtraDataBytes;
                    formattedBytesTotal = g_format_size_full(total, G_FORMAT_SIZE_IEC_UNITS);

                    snprintf(buf, sizeof(buf), _("Downloading files: %d/%d %s"),
                             progressStatus.fetched, progressStatus.requested,
                             formattedBytesTotalTransferred ? formattedBytesTotalTransferred : "");
                    kmlogger.debug("下载文件: %s", buf);
                }

                if (total > 0)
                {
                    newProgress = 5 + ((totalTransferred / (gdouble)total) * 92);
                }

                newProgress += getWriteProgress(progressStatus.outstandingWrites);
            }

            // 检查是否所有对象都已获取完成且没有待写入操作（与 progressCallback 保持一致）
            // 在计算完 newProgress 后检查，确保进度能正确达到 100%
            if (progressStatus.requested > 0 && progressStatus.fetched >= progressStatus.requested && progressStatus.outstandingWrites == 0)
            {
                // 所有对象都已获取完成，且没有待写入操作，认为下载完成
                kmlogger.debug("All objects fetched (fetched: %u, requested: %u) and no outstanding writes, setting progress to 100%%",
                               progressStatus.fetched, progressStatus.requested);
                newProgress = 100;
                progressStatus.progress = 100;
            }

            if (elapsedTime > 0)
            {
                formattedBytesSec = g_format_size(totalTransferred / elapsedTime);
                snprintf(buf, sizeof(buf), " (%s/s)", formattedBytesSec ? formattedBytesSec : "");
                kmlogger.debug("totalTransferred: %ld elapsedTime: %ld buf: %s", totalTransferred, elapsedTime, buf);
            }
        }

        kmlogger.info("newProgress: %d manager->currentPercent: %d", newProgress, context->currentPercent);

        // 更新任务的 currentPercent
        if (newProgress < context->currentPercent)
        {
            newProgress = context->currentPercent;
        }
        else
        {
            if (newProgress == 100)
            {
                context->currentPercent = 0;
            }
            else
            {
                context->currentPercent = newProgress;
            }
        }

        progressStatus.lastTotal = total;
        if (newProgress > 100)
        {
            if (!progressStatus.reportedOverflow)
            {
                kmlogger.error("Unexpectedly got > 100%% progress, limiting");
            }
            progressStatus.reportedOverflow = true;
            newProgress = 100;
        }

        guint singlePercent = newProgress;
        const std::string normalizedCurrentRef = normalizeRefForCompare(progressStatus.ref);

        // 使用该任务的 refDownloadList 计算总进度
        uint64_t totalSize = 0;
        for (const auto &ref : context->refDownloadList)
        {
            totalSize += ref.installedSize;
        }

        uint64_t singleSize = 0;
        std::string id;
        for (const auto &pmRef : context->refDownloadList)
        {
            std::string pmCompleteRef = pmRef.completedRef;
            KMOSTreeHandler::replaceRef(pmCompleteRef);
            if (normalizedCurrentRef == pmCompleteRef)
            {
                singleSize = pmRef.installedSize;
                id = pmRef.ref.id;
                break;
            }
        }

        bool isDependencyRef = false;
        bool isDependencyFirstInstall = false;
        for (const auto &item : context->refDownloadList)
        {
            std::string pmCompleteRef = item.completedRef;
            KMOSTreeHandler::replaceRef(pmCompleteRef);

            if (pmCompleteRef == progressStatus.ref)
            {
                if (item.ref.kind == "runtime" || item.ref.kind == "base")
                {
                    isDependencyRef = true;
                    bool isInstalled = context->packageManager->currentRefIsUninstalled(item);
                    isDependencyFirstInstall = !isInstalled;
                }

                break;
            }
        }

        if (isDependencyRef && isDependencyFirstInstall && newProgress != 100)
        {
            // fetched/requested 作为拉取进度百分比
            // 避免 requested 在拉取过程中不断增大导致进度虚高。

            const guint TOP_OPENKYLIN_BASE_EXPECTED = 23652; // top.openkylin.base 最终 fetched 基准值大小
            const guint TOP_OPENKYLIN_UKUI_EXPECTED = 43950; // top.openkylin.ukui 最终 fetched 基准值大小
            const guint KYLIN_V10SP1_BASE_EXPECTED = 64956;  // cn.kylin.v10sp1-base 最终 fetched 基准值大小

            guint64 denom = 0;

            std::string refStr = progressStatus.ref;
            if (refStr.find("top.openkylin.ukui") != std::string::npos)
            {
                denom = TOP_OPENKYLIN_UKUI_EXPECTED;
            }
            else if (refStr.find("cn.kylin.v10sp1-base") != std::string::npos)
            {
                denom = KYLIN_V10SP1_BASE_EXPECTED;
            }
            else if (refStr.find("top.openkylin.base") != std::string::npos)
            {
                denom = TOP_OPENKYLIN_BASE_EXPECTED;
            }

            if (denom > 0)
            {
                guint fetchedRequestedPercent = (guint)((progressStatus.fetched * 100) / denom);
                if (fetchedRequestedPercent > 100)
                    fetchedRequestedPercent = 100;

                // 使用更保守的进度：不要让 singlePercent 超过 fetched/denom
                singlePercent = std::min(singlePercent, fetchedRequestedPercent);
            }
        }

        if (!context->refDownloadList.empty())
        {
            std::string lastDownloadRef = context->refDownloadList.back().completedRef;
            kmlogger.debug("lastDownloadRef: %s", lastDownloadRef.c_str());

            for (const auto &item : context->refDownloadList)
            {
                kmlogger.debug("completedRef: %s size:%ld ", item.completedRef.c_str(), item.installedSize);
                if (normalizeRefForCompare(item.completedRef) == normalizedCurrentRef)
                {
                    currentProgress += (double)singlePercent / 100 * singleSize;
                    break;
                }
                currentProgress += item.installedSize;
            }
        }

        if (totalSize > 0)
        {
            totalPercent = (double)currentProgress * 100 / totalSize;
        }

        kmlogger.info("Task %s: singlePercent:%d singleSize: %ld currentProgress: %ld totalSize: %ld totalPercent: %d",
                      context->taskId.c_str(), singlePercent, singleSize, currentProgress, totalSize, totalPercent);

        progressStatus.progress = totalPercent;
        progressStatus.estimating = estimating;
    }

    // 更新任务的进度状态
    // 注意：refId 应该使用主应用名，而不是当前下载的依赖名
    context->progressStatus.refId = context->mainAppName; // 使用主应用名
    context->progressStatus.progress = totalPercent;

    // 计算速度
    if (elapsedTime > 0 && totalPercent < 100)
    {
        if (!formattedBytesSec)
            formattedBytesSec = g_format_size(totalTransferred / elapsedTime);
        if (formattedBytesSec)
        {
            speed = g_strdup_printf("%s/s", formattedBytesSec);
        }
    }
    else
    {
        speed = g_strdup_printf("5 MB/s");
    }

    if (progressStatus.ref == progressStatus.lastRef && *progressStatus.ostreeStatus)
    {
        totalPercent = 100;
        printf("\n");
    }

    totalPercent = std::max((int)totalPercent, context->totalPercent);
    if(totalPercent > context->totalPercent) {
        context->totalPercent = totalPercent;
    }

    context->progressStatus.speed = speed ? std::string(speed) : "5 MB/s";
    context->progressStatus.timeout = 5;

    // 构建进度 JSON（使用主应用名）
    context->progressJson = KMDbusJson::packProgressInfoToJson(
        context->mainAppName,  // 使用主应用名，而不是当前下载的依赖名
        totalPercent,
        context->progressStatus.speed,
        context->progressStatus.timeout
    );

    kmlogger.debug("Task %s progress: refId:%s progress:%d speed:%s timeout:%d", 
                   context->taskId.c_str(), 
                   context->progressStatus.refId.c_str(), 
                   context->progressStatus.progress, 
                   context->progressStatus.speed.c_str(), 
                   context->progressStatus.timeout);

    // 调用该任务的进度回调（添加异常处理，防止崩溃）
    if (context->downloadCallback && !context->progressJson.empty())
    {
        try {
            context->downloadCallback(context->progressJson);
        } catch (const std::exception& e) {
            kmlogger.error("Exception in downloadCallback for task %s: %s", context->taskId.c_str(), e.what());
        } catch (...) {
            kmlogger.error("Unknown exception in downloadCallback for task %s", context->taskId.c_str());
        }
    }

    // 内存释放
    if (formattedBytesTotalTransferred)
        g_free(formattedBytesTotalTransferred);
    if (formattedBytesTotal)
        g_free(formattedBytesTotal);
    if (formattedBytesSec)
        g_free(formattedBytesSec);
    if (speed)
        g_free(speed);
}

int KMPackageManager::checkNetworkConnection(std::shared_ptr<KMOSTreeHandler> handler)
{
    // 检查 ping 命令是否存在
    int pingCheck = system("command -v ping >/dev/null 2>&1");
    bool pingAvailable = (pingCheck == 0);
    
    if (!pingAvailable)
    {
        kmlogger.warn("ping 命令不存在，将使用 TCP 连接测试方法");
    }
    
    auto remotes = handler->getAllRemotesWithPriority();
    if (remotes.empty())
    {
        kmlogger.warn("未配置任何远程仓库，跳过网络检查");
        return -1;
    }
    
    // 依优先级遍历所有远程仓库，只要有一个可达就认为网络正常
    for (const auto &remote : remotes)
    {
        std::string ostreeRepoUrl = handler->getRemoteUrl(remote.name);
        if (ostreeRepoUrl == "-")
        {
            continue; // 跳过无效 URL
        }
        
        // 解析主机部分
        std::string hostPart = extractHostPart(ostreeRepoUrl);
        if (hostPart.empty())
        {
            kmlogger.warn("远程 %s 的 URL 无法解析主机名: %s", remote.name.c_str(), ostreeRepoUrl.c_str());
            continue;
        }
        
        bool reachable = false;
        
        if (pingAvailable)
        {
            // 使用 ping 方法
            std::string pingCmd = "ping -c 1 -W 2 " + hostPart + " > /dev/null 2>&1";
            int ret = system(pingCmd.c_str());
            reachable = (ret == 0);
            kmlogger.info("Ping %s (%s): %s", remote.name.c_str(), hostPart.c_str(), reachable ? "success" : "failed");
        }
        else
        {
            // 使用 timeout + bash + /dev/tcp 方法
            int port = extractPort(ostreeRepoUrl);
            reachable = testTcpConnection(hostPart, port, 2);
            if (!reachable)
            {
                kmlogger.warn("精确端口 %d 不通，尝试常见端口", port);
                // 再尝试多个端口
                reachable = testTcpConnectionWithFallback(hostPart, port, 2);
            }
            kmlogger.info("TCP连接测试 %s (%s:%d): %s", remote.name.c_str(), hostPart.c_str(), port, reachable ? "success" : "failed");
        }
        
        if (reachable)
        {
            return 1; // 任一远程可达即认为网络正常
        }
    }
    
    // 所有远程均不可达
    return 0;
}

std::string KMPackageManager::extractHostPart(const std::string& url)
{
    // 去掉协议头
    size_t pos = url.find("://");
    std::string hostPart = (pos != std::string::npos) ? url.substr(pos + 3) : url;
    
    // 截掉路径，只保留 host[:port]
    pos = hostPart.find('/');
    if (pos != std::string::npos)
        hostPart = hostPart.substr(0, pos);
    
    // 如果是 IPv6 格式 [addr]:port
    if (!hostPart.empty() && hostPart[0] == '[')
    {
        size_t bracketEnd = hostPart.find(']');
        if (bracketEnd != std::string::npos)
        {
            // 提取纯 IPv6 地址（去掉方括号）
            std::string ipv6Addr = hostPart.substr(1, bracketEnd - 1);
            return ipv6Addr;
        }
    }
    else
    {
        // IPv4 或域名，去掉 :port（ping 不需要端口）
        size_t colonPos = hostPart.find(':');
        if (colonPos != std::string::npos)
            hostPart = hostPart.substr(0, colonPos);
    }
    
    return hostPart;
}

int KMPackageManager::extractPort(const std::string& url)
{
    int defaultPort = 80;
    
    // 检查是否是 HTTPS
    if (url.find("https://") == 0)
        defaultPort = 443;
    
    // 去掉协议头
    size_t protocolPos = url.find("://");
    std::string urlWithoutProtocol = (protocolPos != std::string::npos) 
                                     ? url.substr(protocolPos + 3) 
                                     : url;
    
    // 分离主机和路径
    size_t pathPos = urlWithoutProtocol.find('/');
    std::string hostPortPart = (pathPos != std::string::npos) 
                               ? urlWithoutProtocol.substr(0, pathPos) 
                               : urlWithoutProtocol;
    
    // 处理 IPv6 地址
    if (!hostPortPart.empty() && hostPortPart[0] == '[')
    {
        size_t bracketEnd = hostPortPart.find(']');
        if (bracketEnd != std::string::npos && hostPortPart.length() > bracketEnd + 1)
        {
            if (hostPortPart[bracketEnd + 1] == ':')
            {
                std::string portStr = hostPortPart.substr(bracketEnd + 2);
                return std::stoi(portStr);
            }
        }
    }
    else
    {
        // IPv4 或域名
        size_t colonPos = hostPortPart.find(':');
        if (colonPos != std::string::npos)
        {
            std::string portStr = hostPortPart.substr(colonPos + 1);
            return std::stoi(portStr);
        }
    }
    
    return defaultPort;
}

bool KMPackageManager::testTcpConnection(const std::string& host, int port, int timeoutSeconds)
{
    // 处理 IPv6 地址（去掉方括号）
    std::string cleanHost = host;
    if (!cleanHost.empty() && cleanHost[0] == '[' && cleanHost.back() == ']')
    {
        cleanHost = cleanHost.substr(1, cleanHost.length() - 2);
    }
    
    // 构造 bash 命令
    std::string cmd = "timeout " + std::to_string(timeoutSeconds) + 
                      " bash -c \"echo >/dev/tcp/" + cleanHost + "/" + std::to_string(port) + "\" 2>/dev/null";
    
    int ret = system(cmd.c_str());
    
    // system 返回值：成功返回 0
    // timeout 超时返回 124
    return (ret == 0);
}

bool KMPackageManager::testTcpConnectionWithFallback(const std::string& host, int preferredPort, int timeoutSeconds)
{
    // 优先测试首选端口
    if (testTcpConnection(host, preferredPort, timeoutSeconds))
    {
        return true;
    }
    
    // 如果首选端口不通，尝试常见端口
    std::vector<int> fallbackPorts = {80, 443, 8000, 8080, 8888, 8443};
    for (int port : fallbackPorts)
    {
        if (port == preferredPort)
            continue; // 已经测试过
            
        if (testTcpConnection(host, port, timeoutSeconds))
        {
            kmlogger.debug("使用备用端口 %d 连接到 %s", port, host.c_str());
            return true;
        }
    }
    
    return false;
}

void KMPackageManager::clearOldProgress(gpointer userData)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
    if (!manager->lastProgressStr.empty() && manager->lastWidth > 0)
    {
        int lines = (manager->lastProgressStr.length() + manager->lastWidth - 1) / manager->lastWidth;
        std::cout << "\r";
        for (int i = 0; i < lines; ++i)
        {
            std::cout << "\033[K";
            if (i < lines - 1)
            {
                std::cout << "\033[A";
            }
        }
        std::cout << std::flush;
    }
}

int KMPackageManager::getWriteProgress(int outstandingWrites)
{
    return outstandingWrites > 0 ? (int)(3 / (double)outstandingWrites) : 3;
}

uint64_t KMPackageManager::getTotalSize(gpointer userData)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
    uint64_t sumSize = 0;
    for (const auto &ref : manager->refDownloadListForProgress)
    {
        kmlogger.debug("completedRef: %s installedSize:%ld", ref.completedRef.c_str(), ref.installedSize);

        sumSize += ref.installedSize;
    }

    return sumSize;
}

uint64_t KMPackageManager::getSingleSize(const string &completedRef, string &id, gpointer userData)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
    uint64_t singleSize = 0; // 需初始化
    id.clear();

    for (const auto &pmRef : manager->refDownloadListForProgress)
    {
        kmlogger.debug("completedRef: %s installedSize:%ld", pmRef.completedRef.c_str(), pmRef.installedSize);
        std::string pmCompleteRef = pmRef.completedRef;
        KMOSTreeHandler::replaceRef(pmCompleteRef);
        if (completedRef == pmCompleteRef)
        {
            singleSize = pmRef.installedSize;
            id = pmRef.ref.id;
            break;
        }
    }

    if (singleSize == 0)
    {
        kmlogger.warn("No matching package found for completedRef: %s", completedRef.c_str());
    }

    return singleSize;
}

// void KMPackageManager::updateProgress(gpointer userData)
// {
//     KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
//     // 只有当 globalProgressCallback
//     // 是有效的时候才调用,即已经与某个可调用对象关联，并且没有被释放，避免 bad_function_call
//     if (manager->globalProgressCallback && manager->progressJson != "")
//     {
//         manager->globalProgressCallback(manager->progressJson);
//     }
// }

// userData: 当前对象
// isDeploy: true 在线安装表示部署阶段，false 表示下载阶段或离线部署阶段
void KMPackageManager::updateProgress(gpointer userData, bool isDeploy)
{
    KMPackageManager *manager = static_cast<KMPackageManager *>(userData);
    if (manager->progressJson.empty())
        return;

    // 根据阶段调用不同的回调/信号
    if (isDeploy)
    {
        if (manager->deployProgressCallback) // 部署回调
        {
            manager->deployProgressCallback(manager->progressJson); // 发 deployprogresssignal
            kmlogger.debug("emit deployprogresssignal: %s", manager->progressJson.c_str());
        }
    }
    else
    {
        // 只有当 globalProgressCallback
        // 是有效的时候才调用,即已经与某个可调用对象关联，并且没有被释放，避免 bad_function_call
        if (manager->globalProgressCallback && manager->progressJson != "")
        {
            manager->globalProgressCallback(manager->progressJson);
        }
    }
}

string KMPackageManager::formatDuration(guint64 duration)
{
    int h, m, s;

    std::stringstream ss;

    m = duration / 60;
    s = duration % 60;
    h = m / 60;
    m = m % 60;

    if (h > 0)
    {
        // 使用流操作符进行格式化，输出为两位数
        ss.fill('0'); // 填充字符为 '0'
        ss.width(2);  // 设置宽度为 2
        ss << h << ":";
    }

    ss.fill('0');
    ss.width(2);
    ss << m << ":";

    ss.fill('0');
    ss.width(2);
    ss << s;

    return ss.str();
}

std::string KMPackageManager::calculateDownloadTime(const std::string &speed, guint64 sizeBytes)
{
    long int downloadSpeed;
    std::string result;

    kmlogger.debug("speed: %s sizeBytes: %ld", speed.c_str(), sizeBytes);

    // Parse the speed string to get the download speed in bytes/s
    if (speed.find("MB") != std::string::npos)
    {
        downloadSpeed = std::stol(speed.substr(0, speed.find("MB") - 1)) * 1024 * 1024; // Convert MB/s to bytes/s
    }
    else if (speed.find("kB") != std::string::npos)
    {
        downloadSpeed = std::stol(speed.substr(0, speed.find("kB") - 1)) * 1024; // Convert kB/s to bytes/s
    }
    else if (speed.find("bytes") != std::string::npos)
    {
        downloadSpeed = std::stol(speed.substr(0, speed.find("bytes") - 1));
        if (downloadSpeed == 0)
        {
            downloadSpeed = 5 * 1024; // 当 speed 属于 0 bytes/s 时，代表不下载，但进度也应该有，因此默认为5*1024
                                      // = 5KB 计算时间
        }
    }
    else
    {
        if (sizeBytes < 169887)
        {
            return "00:10"; // 给出默认时间 10 秒
        }
        else
        {
            return "01:00"; // 给出默认时间 1 分钟
        }
    }
    kmlogger.debug("speed: %s sizeBytes: %ld downloadSpeed=%ld", speed.c_str(), sizeBytes, downloadSpeed);

    // if (isZero(downloadSpeed))
    if (downloadSpeed == 0 && sizeBytes > 0)
    {
        // return "";
        downloadSpeed = 5 * 1024; // 当 speed 属于 0 bytes/s 时，代表不下载，但进度也应该有，因此默认为50计算时间
    }

    // Calculate the remaining download time in seconds
    int downloadTime = (int)sizeBytes / downloadSpeed;
    kmlogger.debug("speed: %s sizeBytes: %ld downloadSpeed=%ld downloadTime=%d", speed.c_str(), sizeBytes, downloadSpeed, downloadTime);

    // Format the download time as hours:minutes:seconds or minutes:seconds
    if (downloadTime >= 3600)
    {
        int hours = static_cast<int>(downloadTime / 3600);
        int minutes = static_cast<int>((downloadTime % 3600) / 60);
        int seconds = static_cast<int>(downloadTime % 60);
        result = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
        kmlogger.debug("speed: %s sizeBytes: %ld downloadSpeed=%ld downloadTime=%d result:%s", speed.c_str(), sizeBytes, downloadSpeed, downloadTime, result.c_str());
    }
    else
    {
        int minutes = static_cast<int>(downloadTime / 60);
        int seconds = static_cast<int>(downloadTime % 60);
        result = std::to_string(minutes) + ":" + std::to_string(seconds);
        kmlogger.debug("speed: %s sizeBytes: %ld downloadSpeed=%ld downloadTime=%d result:%s", speed.c_str(), sizeBytes, downloadSpeed, downloadTime, result.c_str());
    }

    if (result.find("-") != std::string::npos)
    {
        // 时间为负数不合理，此处先默认给出5分钟
        return "05:00";
    }

    return result;
}

void KMPackageManager::emitTotalDeployProgress(int currentRefPercent)
{
    if (d->m_totalRefCount == 0)
        return;

    int totalPercent =
        static_cast<int>(
            (d->m_currentRefIndex * 100 + currentRefPercent)
            / d->m_totalRefCount);

    if (totalPercent > 100)
        totalPercent = 100;

    globalProgressStatus.progress = totalPercent;
    globalProgressStatus.speed = "";
    globalProgressStatus.timeout = 0;

    kmlogger.debug(
        "deploy total progress ---> ref[%zu/%zu] sub:%d%% total:%d%%",
        d->m_currentRefIndex + 1,
        d->m_totalRefCount,
        currentRefPercent,
        totalPercent);

    progressJson = KMDbusJson::packProgressInfoToJson(
        globalProgressStatus.refId,
        globalProgressStatus.progress,
        globalProgressStatus.speed,
        globalProgressStatus.timeout);

    updateProgress(this, true);
}

void KMPackageManager::emitTotalDeployProgressWithContext(int currentRefPercent, std::shared_ptr<DownloadTaskContext> context)
{
    if (context->totalRefCount == 0)
        return;

    int totalPercent =
        static_cast<int>(
            (context->currentRefIndex * 100 + currentRefPercent)
            / context->totalRefCount);

    if (totalPercent > 100)
        totalPercent = 100;

    context->progressStatus.progress = totalPercent;
    context->progressStatus.speed = "";
    context->progressStatus.timeout = 0;

    kmlogger.debug(
        "deploy total progress ---> ref[%zu/%zu] sub:%d%% total:%d%%",
        context->currentRefIndex + 1,
        context->totalRefCount,
        currentRefPercent,
        totalPercent);

    context->progressJson = KMDbusJson::packProgressInfoToJson(
        context->progressStatus.refId,
        context->progressStatus.progress,
        context->progressStatus.speed,
        context->progressStatus.timeout);

    if (context->deployCallback && !context->progressJson.empty())
    {
        try {
            context->deployCallback(context->progressJson);
        } catch (const std::exception& e) {
            kmlogger.error("Exception in deployCallback for task %s: %s", context->taskId.c_str(), e.what());
        } catch (...) {
            kmlogger.error("Unknown exception in deployCallback for task %s", context->taskId.c_str());
        }
    }
}

int KMPackageManager::deploy(PackageManagerOptions pmOptions)
{
    fs::path basePath = d->m_folder->getPath();
    fs::path layersPath;
    bool kareInstall = true;

    d->m_totalRefCount = d->m_pmRefDownloadList.size();
    d->m_currentRefIndex = 0;

    kmlogger.info("部署开始，总 ref 数: %zu", d->m_totalRefCount);
    for (auto &downloadRef : d->m_pmRefDownloadList)
    {
        globalProgressStatus.refId = downloadRef.ref.id;

        // 当前 ref 起点
        emitTotalDeployProgress(0);

        d->m_installedFiles.clear();

        if (downloadRef.ref.module == "devel")
        {
            layersPath = fs::path(KMStorageDir::getUserBaseDirLocation());
        }
        else
        {
            layersPath = basePath / "layers";
        }

        emitTotalDeployProgress(5);

        kmlogger.info("正在部署 completedRef: %s", downloadRef.completedRef.c_str());
        fs::path idPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id;

        fs::path versionDataPath, tmpPath;

        if (pmOptions.bundle)
        {
            tmpPath = fs::path("/tmp/kaiming");
            versionDataPath = tmpPath / downloadRef.completedRef;
            // 确保临时目录存在
            try
            {
                if (!fs::exists(tmpPath))
                {
                    fs::create_directories(tmpPath);
                    kmlogger.info("Created temporary directory structure: %s", tmpPath.c_str());
                }
            }
            catch (...)
            {
                kmlogger.error("Failed to create temporary directory structure.");
            }
        }
        else
        {
            // bundle为false时，使用正常路径
            versionDataPath = layersPath / downloadRef.completedRef;
        }

        kmlogger.debug("versionDataPath:  %s", versionDataPath.c_str());

        if (downloadRef.ref.kind == "app")
        {
            d->m_installedFiles.insert(idPath.string());
        }

        d->m_installedFiles.insert(versionDataPath.string());

        if (std::filesystem::exists(versionDataPath) || std::filesystem::create_directories(versionDataPath))
        {
            kmlogger.debug("Directory created or already exists successfully!");
        }

        // ostreeManager.checkout("f3b1314b51f2b8865089f13cbae5b7cd449b169f0b36530d27da74beb68aa492", "your-repo-path");
        kmlogger.debug("Begin to checkout ...");
        string completeRef = downloadRef.completedRef;
        string latestCommit = d->m_ostreeHandler->readLatestCommit(downloadRef.completedRef);
        if (pmOptions.isDeployOnly && latestCommit.empty())
        {
            d->m_installedInfo.clear();
            d->m_pmDownloadRefs.clear();
            d->m_pmFuzzyMatchRefs.clear();
            d->m_pmRefDownloadList.clear();
            kmlogger.info("本地仓库未找到 %s 的最新提交", completeRef.c_str());
            return KM_ERROR_NOT_DOWNLOAD;
        }
        bool isCheckout = d->m_ostreeHandler->checkout(latestCommit, versionDataPath.string());
        emitTotalDeployProgress(10);

        if (!isCheckout || pmOptions.bundle)
        {
            fs::path removePath;
            if (downloadRef.ref.module == "binary")
            {
                removePath = d->m_folder->getLayersPath() / downloadRef.completedRef;
            }
            else if (downloadRef.ref.module == "devel")
            {
                removePath = fs::path(KMStorageDir::getUserBaseDirLocation()) / downloadRef.completedRef;
            }

            if (!removePath.empty())
            {
                deleteSpecificRefsFromRepo(removePath.string());
            }

            if (pmOptions.bundle)
            {
                fs::path exportDir = fs::path(pmOptions.subpath);
                if (!std::filesystem::exists(exportDir))
                {
                    std::filesystem::create_directories(exportDir);
                }

                std::string exportCmd = "kaiming build-export -d \"" + std::string(pmOptions.subpath) + "\" \"" + versionDataPath.string() + "\"";
                kmlogger.info("Executing export command: %s", exportCmd.c_str());

                int exportResult = system(exportCmd.c_str());
                if (std::filesystem::exists(versionDataPath))
                {
                    std::filesystem::remove_all(versionDataPath);
                    kmlogger.debug("Cleaned up temporary directory: %s", versionDataPath.c_str());
                }

                if (exportResult != 0)
                {
                    // 导出失败，立即返回
                    d->m_installedInfo.clear();
                    d->m_pmDownloadRefs.clear();
                    d->m_pmFuzzyMatchRefs.clear();
                    d->m_pmRefDownloadList.clear();

                    kmlogger.error("kaiming build-export command execution failed: %d", exportResult);
                    return KM_ERROR_INSTALL_FAILED;
                }
                else
                {
                    kmlogger.info("kaiming build-export command executed successfully");
                    continue;
                }
            }

            if (!isCheckout)
            {
                std::cout << "Checkout files failed" << std::endl;
                kmlogger.info("Checkout files failed");
                // checkout失败，立即返回
                d->m_installedInfo.clear();
                d->m_pmDownloadRefs.clear();
                d->m_pmFuzzyMatchRefs.clear();
                d->m_pmRefDownloadList.clear();
                return KM_ERROR_CHECKOUT_FAILED;
            }

            if (pmOptions.bundle)
            {
                continue;
            }

            return KM_ERROR_INSTALL_FAILED;
        }

        // d->m_ostreeHandler->checkoutWithFallback(latestCommit, downloadRef.completedRef, versionDataPath.string());

        kmlogger.debug("End to checkout.");
        kmlogger.info("baseType:  %s", downloadRef.ref.kind.c_str());
        emitTotalDeployProgress(15);

        fs::path modulePath = idPath / downloadRef.ref.module;
        fs::path signPath = modulePath / downloadRef.ref.version;

        if (!pmOptions.skipSignatureVerification && pmOptions.isOnlineInstall && !pmOptions.isUpgradeType)
        {
            int verifyResult = KM_ERROR_NO;
            // 验签
            verifyResult = verifySignature(downloadRef.ref.id, downloadRef.ref.kind, signPath);
            if (verifyResult != KM_ERROR_NO)
            {
                std::error_code ec;
                bool hasOtherVersion = false;
                if (fs::exists(modulePath, ec))
                {
                    for (const auto &entry : fs::directory_iterator(modulePath, ec))
                    {
                        if (ec)
                        {
                            break;
                        }
                        if (!entry.is_directory())
                        {
                            continue;
                        }
                        // 发现除当前版本外的其他目录
                        if (entry.path() != signPath)
                        {
                            hasOtherVersion = true;
                            break;
                        }
                    }
                }
                if (hasOtherVersion)
                {
                    // module 下还有其他版本，只删当前版本目录
                    if (!removeTmpDeployFiles(signPath))
                    {
                        kmlogger.info("删除临时目录失败");
                    }
                }
                else
                {
                    // module 下只有这一个版本，直接删整个 idPath
                    if (!removeTmpDeployFiles(idPath))
                    {
                        kmlogger.info("删除临时目录失败");
                    }
                }
                d->m_installedInfo.clear();
                d->m_pmDownloadRefs.clear();
                d->m_pmFuzzyMatchRefs.clear();
                d->m_pmRefDownloadList.clear();
                return verifyResult;
            }
        }
        else if (pmOptions.skipSignatureVerification)
        {
            kmlogger.info("用户不进行验签，跳过验签步骤");
        }
        emitTotalDeployProgress(30);

        d->m_hostAppDeploy->setHostAppConfig(versionDataPath, downloadRef.ref.id);
        if (d->m_hostAppDeploy->loadHostAppInfo())
        {
            // not support hostapp
            // std::cout << KMErrorMsg::getErrorMessage(KM_ERROR_NOT_SUPPORT_HOSTAPP) << "\n";
            // return KM_ERROR_NOT_SUPPORT_HOSTAPP;

            d->m_isHostapp = true;

            if (!d->m_hostAppDeploy->collectSynchronizedFiles())
            {
                kareInstall = false;
                continue;
            }

            d->m_hostAppDeploy->processRecordedDesktopFiles(*this);

            try
            {
                // 根据是否存在待移除旧版本来判断此次为升级还是首次安装
                TaskType installTask = TaskType::TASK_TYPE_INSTALL;
                for (const auto &oldPath : d->m_pmOldAVersion)
                {
                    if (oldPath.find("/" + downloadRef.ref.id + "/") != std::string::npos)
                    {
                        installTask = TaskType::TASK_TYPE_UPGRADE;
                        break;
                    }
                }
                d->m_hostAppDeploy->setInstallTaskType(installTask);
                int syncResult = d->m_hostAppDeploy->synchronizeFiles(false);
                if (syncResult != KM_ERROR_NO)
                {
                    kmlogger.error("Synchronization failed, installation aborted");
                    return syncResult;
                }
                KMInfo("Synchronization complete!");

                getAllBaseFilesInDirectory(versionDataPath.string(), d->m_installedFiles);
            }
            catch (const std::exception &e)
            {
                KMError("Error during synchronization: " + std::string(e.what()));
                return KM_ERROR_INSTALL_FAILED;
            }
        }
        else if (downloadRef.ref.kind == "app")
        {
            d->m_hostAppDeploy->handleRmdir();

            KMInfoJson infoJson;
            fs::path infoPath = versionDataPath / "info.json";
            infoJson.loadFile(infoPath.string());

            downloadRef.ref.isSysapp = infoJson.annotations.sysapp;
            std::vector<std::string> plugins = infoJson.export_plugins;

            const bool isImmutableSystem = KMFileUtils::isImmutableSystem();
            const std::string userFilePath = isImmutableSystem ? "/opt/system/lib/systemd/user/" : "/usr/lib/systemd/user/";
            const std::string systemFilePath = isImmutableSystem ? "/opt/system/lib/systemd/system/" : "/usr/lib/systemd/system/";
            const std::string dbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/services" : "/usr/share/dbus-1/services";
            const std::string systemDbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/system-services" : "/usr/share/dbus-1/system-services";
            const std::string systemDbusConfPath = isImmutableSystem ? "/opt/system/conf/dbus-1/system.d" : "/etc/dbus-1/system.d";
            const std::string schemasFilePath = isImmutableSystem ? "/opt/system/resource/glib-2.0/schemas/" : "/usr/share/glib-2.0/schemas";
            // const std::string ukuiNotificationFilePath = isImmutableSystem ? "/opt/system/resource/ukui/ukui-notification/" : "/usr/share/ukui-notification/";
            const std::string ukuiNotificationFilePath = "/etc/ukui/ukui-notification/default-app/";
            const std::string etcFilePath = isImmutableSystem ? " /opt/system/conf/" : "/etc";
            kmlogger.info(isImmutableSystem ? "ImmutableSystem" : "Not ImmutableSystem");

            // modify service file
            fs::path serviceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "services";
            if (!fs::exists(serviceFileDirectoryPath))
            {
                serviceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "services";
            }

            if (fs::exists(serviceFileDirectoryPath))
            {
                modifyServiceFile(downloadRef, serviceFileDirectoryPath.string(), dbusFilePath, d->m_installedFiles);
                // KMFileUtils::copyFilesByExtension(serviceFileDirectoryPath, dbusFilePath, ".service", "service");
            }

            // system-service
            fs::path systemServiceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system-services";
            if (!fs::exists(systemServiceFileDirectoryPath))
            {
                systemServiceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system-services";
            }

            if (fs::exists(systemServiceFileDirectoryPath))
            {
                modifyServiceFile(downloadRef, systemServiceFileDirectoryPath.string(), systemDbusFilePath, d->m_installedFiles);
            }

            // system-service policy file
            fs::path systemdFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system.d";
            if (!fs::exists(systemdFileDirectoryPath))
            {
                systemdFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system.d";
            }

            if (fs::exists(systemdFileDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdFileDirectoryPath, systemDbusConfPath, ".conf", "service", d->m_installedFiles);
            }

            fs::path systemdConfDirectoryPath = versionDataPath / "entries" / "etc" / "dbus-1" / "system.d";

            if (fs::exists(systemdConfDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdConfDirectoryPath, systemDbusConfPath, ".conf", "service", d->m_installedFiles);
            }

            fs::path schemasSourceDir = versionDataPath / "entries" / "share" / "glib-2.0" / "schemas";
            if (fs::exists(schemasSourceDir))
            {
                fs::create_directories(schemasFilePath);
                KMFileUtils::copyFilesByExtension(schemasSourceDir, schemasFilePath, ".xml", "schema", d->m_installedFiles);

                std::string schemaCommand;
                schemaCommand = "glib-compile-schemas " + schemasFilePath + " >/dev/null 2>&1";
                int ret = system(schemaCommand.c_str());
                if (ret != 0)
                {
                    kmlogger.error("glib-compile-schemas failed for %s, return code: %d", schemasFilePath.c_str(), ret);
                }
            }

            fs::path systemPath;
            if (fs::exists(versionDataPath / "entries" / "etc" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "etc" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "usr" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "usr" / "lib" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "lib" / "systemd";
            }

            auto buildSystemdPath = [](const fs::path &basePath, bool isSystem) -> fs::path
            {
                const char *subpath = isSystem ? "system" : "user";
                return basePath / subpath;
            };

            if (!systemPath.string().empty())
            {
                processServiceFiles(buildSystemdPath(systemPath, true), systemFilePath, downloadRef, d->m_installedFiles);
                processServiceFiles(buildSystemdPath(systemPath, false), userFilePath, downloadRef, d->m_installedFiles);
            }

            fs::path sourcePath = versionDataPath / "entries" / "etc";
            std::vector<std::string> excludePatterns = {"*.service", "*.conf", "ld.so.cache"};
            KMFileUtils::syncWithExclusions(sourcePath, etcFilePath, excludePatterns);

            // Create binData file: entries/bin/ref.id
            fs::path binData;

            if (fs::exists(versionDataPath / "entries" / "usr"))
            {
                binData = versionDataPath / "entries" / "usr" / "bin";
            }
            else
            {
                binData = versionDataPath / "entries" / "bin";
            }

            kmlogger.debug("Check directory:  %s", binData.string().c_str());
            if (fs::exists(binData) || fs::create_directories(binData))
            {
                kmlogger.info("create diretory %s success", binData.string().c_str());
            }

            fs::path binDataRef = binData / downloadRef.ref.id;
            createExportBinShellScript(downloadRef, binDataRef.string());

            // 创建目标目录
            auto ensureDirExists = [](const fs::path &path)
            {
                if (!fs::exists(path))
                {
                    fs::create_directory(path);
                }
            };

            ensureDirExists(d->m_folder->getExportsPath());
            ensureDirExists(d->m_folder->getExportsSharePath());
            ensureDirExists(d->m_folder->getExportsBinPath());

            getAllAppFilesInDirectory(versionDataPath.string(), d->m_installedFiles, downloadRef.ref.id);

            // 递归遍历源目录export/并处理文件
            const std::string targetPath = d->m_folder->getPath().string();
            modifyExportsDirectory(downloadRef, (versionDataPath / "entries").string(), targetPath, d->m_installedFiles);
            kmlogger.debug("Exports files symlinked to /opt/kaiming/exports directory successfully.");

            if (fs::exists(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app"))
            {
                KMFileUtils::copyFilesByExtension(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app", ukuiNotificationFilePath, ".json", "ukui", d->m_installedFiles);
            }

            d->m_pmBinName.clear();

            // 生成应用desktop, icon, service等缓存
            const std::vector<std::string> triggerCachePaths = {
                "/opt/kaiming-tools/share/kaiming/shells/gtk-icon-cache.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/mime-database.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/desktop-database.trigger"};

            runMultiperTriggerUpdateCacheCommand(triggerCachePaths);

            KMPluginSynchronizer::syncPlugins(versionDataPath / "files", plugins, d->m_installedFiles);

            createDbusWhitelistSymlinks(versionDataPath / "files", infoJson.annotations.kylin_dbus_whitelist);
        }

        emitTotalDeployProgress(35);

        // 修改文件夹属主为当前用户，处理 run 运行的权限问题
        // modifyDirOwnership(idPath);

        // if (downloadRef.ref.kind == "base")
        // {
        //     modifyBaseDir(versionDataPath);
        // }
        // Todo: Pruning repo: ostree_repo_prune()
        // 生成所有文件列表文件
        std::cout << "\nWaiting for generate " << downloadRef.name << "'s deploy file ..."
                  << "\n";

        if (downloadRef.ref.kind != "app")
        {
            getAllBaseFilesInDirectory(versionDataPath.string(), d->m_installedFiles);
        }

        fs::path deployDataFile = versionDataPath / "deploy";
        bool isDeploy = createInstalledFile(deployDataFile.string(), false, d->m_installedFiles);
        emitTotalDeployProgress(40);
        if (!isDeploy)
        {
            std::cout << "Generate installed deploy file failed!" << std::endl;
        }

        fs::path infoListFile = d->m_folder->getInfoPath() / (downloadRef.ref.id + ".list");
        d->m_installedFiles.insert(deployDataFile.string());
        writeInstalledFilesToList(infoListFile.string(), d->m_installedFiles);

        const char *old_value = getenv("XDG_DATA_DIRS");
        std::string new_value = "/opt/system/resource";
        if (old_value)
        {
            new_value += ":";
            new_value += old_value;
        }
        else
        {
            new_value += ":/usr/local/share:/usr/share";
        }
        emitTotalDeployProgress(50);
        // 设置合并后的值
        setenv("XDG_DATA_DIRS", new_value.c_str(), 1);

        int result = system("exec 3>&1 1>/dev/null 2>&1; update-mime-database /opt/system/resource/mime");
        if (result != 0)
        {
            kmlogger.error("update-mime-database failed with code %d", result);
        }
        else
        {
            kmlogger.info("MIME database updated successfully");
        }
        // 存储已安装的开明包的信息到/opt/kaiming/info/kaiming.list
        fillInstalledInfo(downloadRef);
        writeInstalledInfoToList();
        emitTotalDeployProgress(55);

        std::set<std::string> deployfiles = d->m_hostAppDeploy->getSynchronizedFiles();
        d->m_installedFiles.merge(deployfiles);
        setKsafLabelAndExectl(d->m_installedFiles, downloadRef.ref.id, downloadRef.ref.kind, versionDataPath);

        if (downloadRef.ref.module == "devel")
        {
            KMStorageDir::modifyDirOwnership(KMStorageDir::getUserDevelDataDirLocation());
        }
        emitTotalDeployProgress(90);

        // 创建 deploy ref
        d->m_ostreeHandler->updateDeployRef(downloadRef.completedRef, latestCommit);

        // start services
        startTargetWantsServices(versionDataPath);

        emitTotalDeployProgress(100);

        kmlogger.info("ref 部署完成: %s", downloadRef.completedRef.c_str());

        d->m_currentRefIndex++;
    }

    d->m_installedInfo.clear();
    d->m_pmDownloadRefs.clear();
    d->m_pmFuzzyMatchRefs.clear();
    d->m_pmRefDownloadList.clear();

    if (!kareInstall)
    {
        return KM_ERROR_KARE_INSTALL_FAILED;
    }

    kmlogger.info("部署成功");

    return KM_ERROR_NO;
}

void KMPackageManager::writeInstalledInfoToList()
{
    fs::path filePath = d->m_folder->getInfoPath() / "kaiming.list";

    fs::path dirPath = fs::path(filePath).parent_path();

    if (!fs::exists(dirPath))
    {
        if (!fs::create_directories(dirPath))
        {
            KMError("Error creating directory: " + dirPath.string());
            return;
        }
    }

    // 使用 std::ios::app 模式打开文件以追加内容
    std::ofstream outFile(filePath, std::ios::app);

    if (!outFile)
    {
        KMError("Error opening file: " + filePath.string());
        return;
    }

    auto escapeNewlines = [](const std::string &input) -> std::string
    {
        std::string result;
        for (char ch : input)
        {
            if (ch == '\n')
                result += "\\n";
            else if (ch == '\r')
                result += "\\r";
            else
                result += ch;
        }
        return result;
    };

    for (const auto &[key, value] : d->m_installedInfo)
    {
        if (key == "Id")
        {
            std::cout << "Writing " << value << "'s information to " << filePath.string() << "..." << std::endl;
        }
        outFile << key << " : " << escapeNewlines(value) << "\n";
    }
    outFile << "\n";
    outFile.close();
}

void KMPackageManager::removePackageInfoFromList(const std::string &packageId, const std::string &version, const std::string &arch, const std::string &module)
{
    fs::path filePath = d->m_folder->getInfoPath() / "kaiming.list";
    if (!fs::exists(filePath))
    {
        KMError("File does not exist: " + filePath.string());
        return;
    }

    std::cout << "File exists: " << filePath.string() << ". Deleting " << packageId << "'s information..." << std::endl;

    std::ifstream inFile(filePath);
    if (!inFile)
    {
        KMError("Error opening file for reading: " + filePath.string());
        return;
    }

    std::stringstream buffer;
    std::string line;
    std::vector<std::string> block;

    while (std::getline(inFile, line))
    {
        if (line.empty())
        {
            // 检查块内是否匹配目标字段
            bool matchId = false, matchVersion = false, matchArch = false, matchModule = false;
            for (const auto &entry : block)
            {
                if (entry == "Id : " + packageId)
                    matchId = true;
                if (entry == "Version : " + version)
                    matchVersion = true;
                if (entry == "Arch : " + arch)
                    matchArch = true;
                if (entry == "Module : " + module)
                    matchModule = true;
            }

            if (!(matchId && matchVersion && matchArch && matchModule))
            {
                // 保留该块
                for (const auto &entry : block)
                {
                    buffer << entry << "\n";
                }
                buffer << "\n"; // 保留块之间的空行
            }

            block.clear();
        }
        else
        {
            block.push_back(line);
        }
    }

    // 处理最后一个 block（如果文件末尾没有空行）
    if (!block.empty())
    {
        bool matchId = false, matchVersion = false, matchArch = false;
        for (const auto &entry : block)
        {
            if (entry == "Id : " + packageId)
            {
                matchId = true;
            }
            if (entry == "Version : " + version)
            {
                matchVersion = true;
            }
            if (entry == "Arch : " + arch)
            {
                matchArch = true;
            }
        }

        if (!(matchId && matchVersion && matchArch))
        {
            for (const auto &entry : block)
            {
                buffer << entry << "\n";
            }
            buffer << "\n";
        }
    }

    inFile.close();

    // 写回文件
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile)
    {
        KMError("Error opening file for writing: " + filePath.string());
        return;
    }

    outFile << buffer.str();
    outFile.close();
}

/**
 * @brief 格式化文件大小
 *
 * @param size
 * @return string
 */
std::string KMPackageManager::formatFileSize(double size) const
{
    vector<string> units = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    int index = 0;

    while (size >= 1024.0 && index < units.size())
    {
        size /= 1024.0;
        index++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[index];
    return oss.str();
}

void KMPackageManager::fillInstalledInfo(const KMPMRef &downloadRef)
{
    d->m_installedInfo["Id"] = downloadRef.ref.id;
    d->m_installedInfo["Channel"] = downloadRef.ref.channel;
    d->m_installedInfo["Version"] = downloadRef.ref.version;
    d->m_installedInfo["Arch"] = downloadRef.ref.arch;
    d->m_installedInfo["Module"] = downloadRef.ref.module;
    d->m_installedInfo["Kind"] = downloadRef.ref.kind;
    d->m_installedInfo["InstalledSize"] = formatFileSize(downloadRef.installedSize);
    d->m_installedInfo["Name"] = downloadRef.name;
    d->m_installedInfo["Description"] = downloadRef.description;
    d->m_installedInfo["CompletedRef"] = downloadRef.completedRef;
}

KMEntry KMPackageManager::cutRef(const std::string &completeRef)
{
    std::stringstream ss(completeRef);
    std::string token;
    vector<std::string> tokens;

    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token); // 将切分后的部分添加到 tokens 向量中
    }

    int tokensSize = tokens.size();
    KMEntry entry;
    entry.id = tokens[tokensSize - 3];
    entry.version = tokens[tokensSize - 1];
    entry.arch = tokens[tokensSize - 5];
    entry.module = tokens[tokensSize - 2];
    entry.kind = tokens[tokensSize - 4];
    return entry;
}

int KMPackageManager::deployOfflinePackage(std::map<std::string, string> &mountMap)
{
    fs::path basePath = d->m_folder->getPath();
    fs::path layersPath;

    uint64_t totalSize = 0;
    uint64_t preSize = 0;
    int totalPackage = 0;
    int currentPackage = 0;
    bool kareInstall = true;

    // 计算总的包大小
    for (auto &downloadRef : d->m_pmRefOfflineList)
    {
        totalSize += downloadRef.installedSize;
        totalPackage++;
    }

    kmlogger.info("部署开始");
    for (auto &downloadRef : d->m_pmRefOfflineList)
    {
        currentPackage++;
        std::cout << "正在部署离线包: " << downloadRef.ref.id << " ..." << std::endl;
        kmlogger.info("正在部署 completedRef: %s", downloadRef.completedRef.c_str());

        if (downloadRef.ref.module == "devel")
        {
            layersPath = fs::path(KMStorageDir::getUserBaseDirLocation());
        }
        else
        {
            layersPath = basePath / "layers";
        }

        fs::path idPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id;

        fs::path versionDataPath = layersPath / downloadRef.completedRef;
        kmlogger.debug("versionDataPath:  %s", versionDataPath.c_str());
        d->m_installedFiles.clear();
        d->m_installedInfo.clear();

        // 存储已安装的开明包的信息到/opt/kaiming/info/kaiming.list
        fillInstalledInfo(downloadRef);
        writeInstalledInfoToList();

        if (downloadRef.ref.kind == "app")
        {
            d->m_installedFiles.insert(idPath.string());
        }

        d->m_installedFiles.insert(versionDataPath.string());

        if (std::filesystem::exists(versionDataPath) || std::filesystem::create_directories(versionDataPath))
        {
            kmlogger.debug("Directory created or already exists successfully!");
        }

        kmlogger.debug("Begin to copy data from mountpoint ...");
        fs::path mountValue = mountMap[downloadRef.ref.id + downloadRef.ref.arch + downloadRef.ref.module];
        fs::path mountpointModulePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module;
        fs::path mountpointExtraPath = mountValue / "extra" / "app-depends";

        if (downloadRef.ref.isDeltas)
        {
            if (downloadRef.ref.preVersion.size() > 1)
            {
                mountpointModulePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module / "files" / downloadRef.ref.findPreVersion / "files" / "layers" / downloadRef.ref.id / downloadRef.ref.module;
            }
            else
            {
                mountpointModulePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module / "files" / "layers" / downloadRef.ref.id / downloadRef.ref.module;
            }
        }

        if (std::filesystem::exists(mountpointModulePath))
        {
            std::string sourcePath = KMUtil::escapePath(mountpointModulePath);
            std::string DestPath = KMUtil::escapePath(versionDataPath);
            std::string command = "rsync -a -H  --progress --no-owner --no-group " + sourcePath + " " + DestPath;

            FILE *pipe = popen(command.c_str(), "r");
            if (!pipe)
            {
                std::cerr << "Failed to run rsync command!" << std::endl;
                return 1;
            }

            int printNumber = -1;
            sharedValue = 0;
            // 启动线程来监控rsync的进度
            std::thread progressThread(KMPackageManager::monitorProgress, pipe);

            while (printNumber != 100)
            {
                if (downloadRef.installedSize > 0)
                {
                    int currentValue = sharedValue.load(std::memory_order_relaxed);
                    currentValue = currentValue / totalPackage + 100 * (currentPackage - 1) / totalPackage;
                    if (currentValue > printNumber)
                    {
                        printNumber = currentValue;
                        // 当前百分比等于已安装的+正在安装*当前包所占百分比
                        //  std::string printValue =std::to_string(preSize + (currentValue * downloadRef.installedSize / totalSize));
                        std::cout << "\r" << currentValue << "%" << std::flush;
                        progressJson = KMDbusJson::packProgressOffInfoToJson(downloadRef.ref.id, currentValue);
                        updateProgress(static_cast<gpointer>(this), false);
                    }
                }

                if (feof(pipe))
                // f (feof(pipe) && currentPackage == totalPackage)
                {
                    if (currentPackage == totalPackage)
                    {
                        std::cout << "\r"
                                  << "100%" << std::flush;
                        progressJson = KMDbusJson::packProgressOffInfoToJson(downloadRef.ref.id, 100);
                        updateProgress(static_cast<gpointer>(this), false);
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 适当间隔去获取值，避免过于频繁占用资源
            }

            // 等待进度监控线程和进度条线程完成
            progressThread.join();
            // 关闭管道
            fclose(pipe);

            kmlogger.debug("Data copied from mountpoint successfully.");
        }
        else
        {
            kmlogger.error("mountpointModulePath not exists:  %s", mountpointModulePath.string().c_str());
        }

        if(downloadRef.ref.isOfflineRun)
        {
            std::string src = mountpointExtraPath.string();
            std::string dst = (versionDataPath / "extra" / "app-depends").string();
            // 确保目标父目录存在
            std::string parentDir = (versionDataPath / "extra").string();
            std::string mkdirCmd = "mkdir -p " + parentDir;

            if(system(mkdirCmd.c_str()) != 0)
            {
                kmlogger.error("create directory failed: %s", parentDir.c_str());
                return KM_ERROR_INSTALL_FAILED;
            }

            // 构建 rsync 命令
            std::string rsyncCmd = "rsync -aHv " + src + "/ " + dst + "/";

            int ret = system(rsyncCmd.c_str());
            if(ret != 0) {
                kmlogger.error("rsync failed with code %d: %s -> %s", ret, src.c_str(), dst.c_str());
                return KM_ERROR_INSTALL_FAILED;
            }

            kmlogger.info("rsync success: %s -> %s", src.c_str(), dst.c_str());
        }

        // 根据二进制文件同步最新文件
        if (downloadRef.ref.isDeltas && !downloadRef.ref.isHostapp)
        {
            fs::path preIdPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id / downloadRef.ref.module / downloadRef.ref.findPreVersion;
            if (!upgradeDirectory(versionDataPath, preIdPath))
            {
                return KM_ERROR_INSTALL_DELTA_FAILED;
            }
        }

        kmlogger.debug("End to copy data from mountpoint.");
        kmlogger.info("baseType:  %s", downloadRef.ref.kind.c_str());

        d->m_hostAppDeploy->setHostAppConfig(versionDataPath, downloadRef.ref.id);

        if (downloadRef.ref.isDeltas)
        {
            string newInfoPath = versionDataPath / "info.json.xdelta";
            fs::path preInfoPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id / downloadRef.ref.module / downloadRef.ref.findPreVersion / "info.json";
            std::string tempFile = (versionDataPath / "info.json.new").string();
            std::string command = std::string(KAIMING_DELTAS) + " -d -f -s \"" +
                                  preInfoPath.string() + "\" \"" + newInfoPath + "\" \"" +
                                  tempFile + "\"  > /dev/null 2>&1 ";

            int result = system(command.c_str());
            if (result == 0)
            {
                // 重命名为目标文件
                fs::rename(tempFile, versionDataPath / "info.json");
                fs::copy_file(versionDataPath / "info.json", preInfoPath, fs::copy_options::overwrite_existing);
            }
        }

        if (d->m_hostAppDeploy->loadHostAppInfo())
        {
            // not support hostapp
            // d->m_okFile->umountOK();
            // std::cout << KMErrorMsg::getErrorMessage(KM_ERROR_NOT_SUPPORT_HOSTAPP) << "\n";
            // return KM_ERROR_NOT_SUPPORT_HOSTAPP;

            if (downloadRef.ref.isDeltas)
            {
                d->m_hostAppDeploy->updateHostDeltas();
                fs::path preIdPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id / downloadRef.ref.module / downloadRef.ref.findPreVersion;
                fs::path deletePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module / "files" / "delete.txt";
                processDeleteFile(deletePath.string(), downloadRef.ref);
                fs::remove_all(versionDataPath);
                fs::rename(preIdPath, versionDataPath);
            }
            else
            {
                d->m_isHostapp = true;

                if (!d->m_hostAppDeploy->collectSynchronizedFiles())
                {
                    kareInstall = false;
                    continue;
                }

                d->m_hostAppDeploy->processRecordedDesktopFiles(*this);

                try
                {
                    // 根据是否存在待移除旧版本来判断此次为升级还是首次安装
                    TaskType installTask = TaskType::TASK_TYPE_INSTALL;
                    for (const auto &oldPath : d->m_pmOldAVersion)
                    {
                        if (oldPath.find("/" + downloadRef.ref.id + "/") != std::string::npos)
                        {
                            installTask = TaskType::TASK_TYPE_UPGRADE;
                            break;
                        }
                    }
                    d->m_hostAppDeploy->setInstallTaskType(installTask);
                    int syncResult = d->m_hostAppDeploy->synchronizeFiles(downloadRef.ref.isDeltas);
                    if (syncResult != KM_ERROR_NO)
                    {
                        d->m_okFile->umountOK();
                        kmlogger.error("Synchronization failed, installation aborted");
                        return syncResult;
                    }
                    KMInfo("Synchronization complete!");

                    getAllBaseFilesInDirectory(versionDataPath.string(), d->m_installedFiles);
                }
                catch (const std::exception &e)
                {
                    KMError("Error during synchronization: " + std::string(e.what()));
                    return KM_ERROR_INSTALL_FAILED;
                }
            }
        }
        else if (downloadRef.ref.kind == "app")
        {
            d->m_hostAppDeploy->handleRmdir();

            const bool isImmutableSystem = KMFileUtils::isImmutableSystem();
            const std::string userFilePath = isImmutableSystem ? "/opt/system/lib/systemd/user/" : "/usr/lib/systemd/user/";
            const std::string systemFilePath = isImmutableSystem ? "/opt/system/lib/systemd/system/" : "/usr/lib/systemd/system/";
            const std::string dbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/services" : "/usr/share/dbus-1/services";
            const std::string systemDbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/system-services" : "/usr/share/dbus-1/system-services";
            const std::string systemDbusConfPath = isImmutableSystem ? "/opt/system/conf/dbus-1/system.d" : "/etc/dbus-1/system.d";
            const std::string schemasFilePath = isImmutableSystem ? "/opt/system/resource/glib-2.0/schemas/" : "/usr/share/glib-2.0/schemas";
            // const std::string ukuiNotificationFilePath = isImmutableSystem ? "/opt/system/resource/ukui/ukui-notification/" : "/usr/share/ukui-notification/";
            const std::string ukuiNotificationFilePath = "/etc/ukui/ukui-notification/default-app/";
            const std::string etcFilePath = isImmutableSystem ? " /opt/system/conf/" : "/etc";
            kmlogger.info(isImmutableSystem ? "ImmutableSystem" : "Not ImmutableSystem");

            // modify service file
            fs::path serviceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "services";
            if (!fs::exists(serviceFileDirectoryPath))
            {
                serviceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "services";
            }

            if (fs::exists(serviceFileDirectoryPath))
            {
                modifyServiceFile(downloadRef, serviceFileDirectoryPath.string(), dbusFilePath, d->m_installedFiles);
                // KMFileUtils::copyFilesByExtension(serviceFileDirectoryPath, dbusFilePath, ".service", "service");
            }

            // system-service
            fs::path systemServiceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system-services";
            if (!fs::exists(systemServiceFileDirectoryPath))
            {
                systemServiceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system-services";
            }

            if (fs::exists(systemServiceFileDirectoryPath))
            {
                modifyServiceFile(downloadRef, systemServiceFileDirectoryPath.string(), systemDbusFilePath, d->m_installedFiles);
            }

            // system-service policy file
            fs::path systemdFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system.d";
            if (!fs::exists(systemdFileDirectoryPath))
            {
                systemdFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system.d";
            }

            if (fs::exists(systemdFileDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdFileDirectoryPath, systemDbusConfPath, ".conf", "service", d->m_installedFiles);
            }

            fs::path systemdConfDirectoryPath = versionDataPath / "entries" / "etc" / "dbus-1" / "system.d";

            if (fs::exists(systemdConfDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdConfDirectoryPath, systemDbusConfPath, ".conf", "service", d->m_installedFiles);
            }

            fs::path schemasSourceDir = versionDataPath / "entries" / "share" / "glib-2.0" / "schemas";
            if (fs::exists(schemasSourceDir))
            {
                fs::create_directories(schemasFilePath);
                KMFileUtils::copyFilesByExtension(schemasSourceDir, schemasFilePath, ".xml", "schema", d->m_installedFiles);

                std::string schemaCommand;
                schemaCommand = "glib-compile-schemas " + schemasFilePath + " >/dev/null 2>&1";
                int ret = system(schemaCommand.c_str());
                if (ret != 0)
                {
                    kmlogger.error("glib-compile-schemas failed for %s, return code: %d", schemasFilePath.c_str(), ret);
                }
            }

            fs::path systemPath;
            if (fs::exists(versionDataPath / "entries" / "etc" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "etc" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "usr" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "usr" / "lib" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "lib" / "systemd";
            }

            auto buildSystemdPath = [](const fs::path &basePath, bool isSystem) -> fs::path
            {
                const char *subpath = isSystem ? "system" : "user";
                return basePath / subpath;
            };

            if (!systemPath.string().empty())
            {
                processServiceFiles(buildSystemdPath(systemPath, true), systemFilePath, downloadRef, d->m_installedFiles);
                processServiceFiles(buildSystemdPath(systemPath, false), userFilePath, downloadRef, d->m_installedFiles);
            }

            fs::path sourcePath = versionDataPath / "entries" / "etc";
            std::vector<std::string> excludePatterns = {"*.service", "*.conf", "ld.so.cache"};
            KMFileUtils::syncWithExclusions(sourcePath, etcFilePath, excludePatterns);

            // Create binData file: entries/bin/ref.id
            fs::path binData;

            if (fs::exists(versionDataPath / "entries" / "usr"))
            {
                binData = versionDataPath / "entries" / "usr" / "bin";
            }
            else
            {
                binData = versionDataPath / "entries" / "bin";
            }
            kmlogger.debug("Check directory:  %s", binData.string().c_str());
            if (fs::exists(binData) || fs::create_directories(binData))
            {
                kmlogger.info("create diretory %s success", binData.string().c_str());
            }

            fs::path binDataRef = binData / downloadRef.ref.id;
            createExportBinShellScript(downloadRef, binDataRef.string());

            // 创建目标目录
            auto ensureDirExists = [](const fs::path &path)
            {
                if (!fs::exists(path))
                {
                    fs::create_directory(path);
                }
            };

            ensureDirExists(d->m_folder->getExportsPath());
            ensureDirExists(d->m_folder->getExportsSharePath());
            ensureDirExists(d->m_folder->getExportsBinPath());
            getAllAppFilesInDirectory(versionDataPath.string(), d->m_installedFiles, downloadRef.ref.id);

            // 递归遍历源目录export/并处理文件
            const std::string targetPath = d->m_folder->getPath().string();
            // const std::string targetPath = isImmutableSystem ? d->m_folder->getPath().string() : "/usr/";
            modifyExportsDirectory(downloadRef, (versionDataPath / "entries").string(), targetPath, d->m_installedFiles);
            kmlogger.debug("Exports files symlinked to /opt/kaiming/exports directory successfully.");

            if(!downloadRef.ref.vecDbusWhitelist.empty())

            if (fs::exists(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app"))
            {
                KMFileUtils::copyFilesByExtension(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app", ukuiNotificationFilePath, ".json", "ukui", d->m_installedFiles);
            }

            d->m_pmBinName.clear();

            // 生成应用desktop, icon, service等缓存
            const std::vector<std::string> triggerCachePaths = {
                "/opt/kaiming-tools/share/kaiming/shells/gtk-icon-cache.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/mime-database.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/desktop-database.trigger"};

            runMultiperTriggerUpdateCacheCommand(triggerCachePaths);

            KMPluginSynchronizer::syncPlugins(versionDataPath / "files", downloadRef.ref.vecPlugins, d->m_installedFiles);

            createDbusWhitelistSymlinks(versionDataPath / "files", downloadRef.ref.vecDbusWhitelist);
        }

        if (downloadRef.ref.isDeltas)
        {
            fs::path deletePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module / "files" / "delete.txt";
            if (downloadRef.ref.preVersion.size() > 1)
            {
                deletePath = mountValue / "layers" / downloadRef.ref.id / downloadRef.ref.module / "files" / downloadRef.ref.findPreVersion / "files" / "delete.txt";
            }
            processDeleteFile(deletePath.string(), downloadRef.ref);
            fs::path preIdPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id / downloadRef.ref.module / downloadRef.ref.findPreVersion;
            vector<string> emptyIgnore;
            copyDirectory(versionDataPath, preIdPath, emptyIgnore);
            fs::remove_all(versionDataPath); // 先删除目标目录

            auto tempPath = versionDataPath.string();
            // 现在可以安全重命名
            if (std::rename(preIdPath.c_str(), tempPath.c_str()) != 0)
            {
                std::cerr << "重命名失败: " << strerror(errno) << std::endl;
            }
        }
        // 修改文件夹属主为当前用户，处理 run 运行的权限问题
        // modifyDirOwnership(idPath);

        // if (downloadRef.ref.kind == "base")
        // {
        //     modifyBaseDir(versionDataPath);
        // }

        // 生成所有文件列表文件
        std::cout << "\nWaiting for generate " << downloadRef.name << "'s deploy file ..."
                  << "\n";

        if (downloadRef.ref.kind != "app")
        {
            getAllBaseFilesInDirectory(versionDataPath.string(), d->m_installedFiles);
        }

        fs::path deployDataFile = versionDataPath / "deploy";

        bool isDeploy = createInstalledFile(deployDataFile.string(), downloadRef.ref.isDeltas, d->m_installedFiles);

        if (!isDeploy)
        {
            std::cout << "Generate installed deploy file failed!" << std::endl;
        }

        fs::path infoListFile = d->m_folder->getInfoPath() / (downloadRef.ref.id + ".list");
        d->m_installedFiles.insert(deployDataFile.string());
        writeInstalledFilesToList(infoListFile.string(), d->m_installedFiles);

        const char *old_value = getenv("XDG_DATA_DIRS");
        std::string new_value = "/opt/system/resource";
        if (old_value)
        {
            new_value += ":";
            new_value += old_value;
        }
        else
        {
            new_value += ":/usr/local/share:/usr/share";
        }

        // 设置合并后的值
        setenv("XDG_DATA_DIRS", new_value.c_str(), 1);

        int result = system("exec 3>&1 1>/dev/null 2>&1; update-mime-database /opt/system/resource/mime");
        if (result != 0)
        {
            kmlogger.error("update-mime-database failed with code %d", result);
        }
        else
        {
            kmlogger.info("MIME database updated successfully");
        }

        std::set<std::string> deployfiles = d->m_hostAppDeploy->getSynchronizedFiles();
        d->m_installedFiles.merge(deployfiles);
        setKsafLabelAndExectl(d->m_installedFiles, downloadRef.ref.id, downloadRef.ref.kind, versionDataPath);
        if (downloadRef.ref.module == "devel")
        {
            KMStorageDir::modifyDirOwnership(KMStorageDir::getUserDevelDataDirLocation());
        }

        // start services
        startTargetWantsServices(versionDataPath);

        // Todo: Pruning repo: ostree_repo_prune()
        std::cout << "成功部署离线包: " << downloadRef.ref.id << " " << std::endl;
    }

    d->m_okFile->umountOK();
    d->m_pmDownloadRefs.clear();
    d->m_pmFuzzyMatchRefs.clear();
    d->m_pmRefDownloadList.clear();

    if (!kareInstall)
    {
        return KM_ERROR_KARE_INSTALL_FAILED;
    }

    kmlogger.info("部署成功");

    return KM_ERROR_NO;
}

void KMPackageManager::createFileForEmptyString(const std::string &filename)
{
    // 打开文件进行检查
    std::ifstream inputFile(filename);
    if (inputFile.is_open())
    {
        kmlogger.error("File already exists:  %s", filename.c_str());
        inputFile.close();
        return;
    }

    // 创建文件并打开输出流
    std::ofstream outputFile(filename);
    if (!outputFile.is_open())
    {
        kmlogger.error("Failed to create file:  %s", filename.c_str());
        return;
    }

    // 写入空字符串到文件
    outputFile << "";

    // 关闭文件
    outputFile.close();
}

void KMPackageManager::modifyServiceFile(KMPMRef &pmRef, const string &serviceFileDirectoryPath, const string destPath, std::set<std::string> &installedFiles)
{
    std::vector<std::string> files;
    files.clear();

    bool noModifyServiceFileFlag = false;

    if (!fs::exists(destPath))
    {
        fs::create_directories(destPath); // 创建单级目录
    }

    // 调用函数获取目录下所有文件
    bool success = getAllFilesInDirectory(serviceFileDirectoryPath, files);
    if (success)
    {
        for (const auto &file : files)
        {
            if (file.find(".service") != std::string::npos)
            {
                string serviceFileName = file;
                kmlogger.debug("serviceFileName:  %s", serviceFileName.c_str());

                std::vector<std::string> serviceDatas = getDataFromKVFile(serviceFileName);

                for (auto &line : serviceDatas)
                {
                    // 修改 Exec 字段
                    size_t eqPos = line.find("Exec=");
                    if (eqPos != std::string::npos && eqPos == 0)
                    {
                        std::string value = line.substr(eqPos + strlen("Exec="));
                        if (value.find(" run") != std::string::npos)
                        {
                            // 第一次已进行修改，因此不修改当前文件
                            noModifyServiceFileFlag = true;
                        }

                        string serviceExec = "Exec=";
                        string content;

                        if (pmRef.ref.isSysapp)
                        {
                            content = serviceExec.append("/opt/kaiming-tools/bin/kaiming run ").append(" ").append("--command=").append(value).append(" ").append(pmRef.ref.id);
                        }
                        else
                        {
                            size_t lastPos = value.find_last_of('/');
                            std::string posValue = (lastPos == std::string::npos) ? value : value.substr(lastPos + 1);
                            content = serviceExec.append("/opt/kaiming-tools/bin/kaiming run ").append(" ").append("--command=").append("/opt/apps/").append(pmRef.ref.id).append("/files/bin/").append(posValue).append(" ").append(pmRef.ref.id);
                        }

                        line = content;
                    }
                }

                size_t lastSlashPos = serviceFileName.find_last_of('/');
                std::string filename = serviceFileName.substr(lastSlashPos + 1);
                std::string dest = destPath + '/' + filename;
                kmlogger.info("Copying file from %s to %s", serviceFileName.c_str(), dest.c_str());

                setDataToKVFile(dest, serviceDatas);
                installedFiles.insert(dest);
                // if (!noModifyServiceFileFlag)
                // {
                //     // 将修改后的内容写回 .service 文件
                //     if (setDataToKVFile(serviceFileName, serviceDatas))
                //     {
                //         kmlogger.debug("Desktop file modified successfully.");
                //     }
                // }
            }
        }
    }
    else
    {
        kmlogger.error("Failed to get files in directory.");
    }
}

void KMPackageManager::initializeFixedModifications()
{
    fixedModifications = {
        {"ExecReload", [this](const auto &s, const auto &id)
         { return addOkPrefix(s, id); }},
        {"ExecStartPre", [this](const auto &s, const auto &id)
         { return addOkPrefix(s, id); }},
        {"ExecStart", [this](const auto &s, const auto &id)
         { return formatExecStart(s, id); }},
        {"Type", "simple"},
        {"CapabilityBoundingSet", "CAP_NET_BIND_SERVICE"},
        {"NoNewPrivileges", "false"},
        {"PrivateUsers", "false"},
        {"ProtectControlGroups", "false"},
        {"ProtectHostname", "false"},
        {"ProtectKernelLogs", "false"},
        {"ProtectProc", "invisible"},
        {"RestrictAddressFamilies", ""},
        {"RestrictNamespaces", "false"},
        {"SystemCallArchitectrues", "native"},
        {"SystemCallFilter", ""},
        {"NoExecPaths", ""},
        {"TasksMax", [this](const auto &s, const auto &id)
         { return changeTasks(s); }},
        {"KillMode", "control-group"}};
}

vector<string> KMPackageManager::getServiceName(const vector<string> &files)
{
    vector<string> servicesFiles;
    for (const auto &file : files)
    {
        if (file.find(".service") != std::string::npos)
        {
            kmlogger.debug("serviceFileName:  %s", file.c_str());
            servicesFiles.push_back(file);
        }
    }
    return servicesFiles;
}

string KMPackageManager::addOkPrefix(const string &original, const std::string id)
{
    return "/opt/kaiming-tools/bin/kaiming run --detach=false --command=\"" + original + "\" " + id;
}

string KMPackageManager::addKillPrefix(const string &original, const std::string id)
{
    return "/opt/kaiming-tools/bin/kaiming kill " + id;
}

string KMPackageManager::changeTasks(const string &original)
{
    if (original == "infinity")
    {
        return "512";
    }
    return original;
}

string KMPackageManager::formatExecStart(const std::string &line, const std::string id)
{
    std::string value = line;
    bool need = false;
    if (!value.empty() && value.front() == '"')
        value = value.substr(1);
    if (!value.empty() && value.back() == '"')
        value.pop_back();
    if (!value.empty() && value.back() == '&')
        value.pop_back();
    if (!value.empty() && value.front() == '-')
    {
        value = value.substr(1);
        need = true;
    }

    // 保留 `--supervised systemd --daemonize no`（如果存在）
    std::string extraArgs;
    size_t supervisedPos = value.find("--supervised systemd");
    size_t daemonizePos = value.find("--daemonize no");
    if (supervisedPos != std::string::npos && daemonizePos != std::string::npos)
    {
        extraArgs = " --supervised systemd --daemonize no";
        // 移除 "--supervised systemd" 和 "--daemonize no"
        value.erase(supervisedPos, std::string("--supervised systemd").length());
        // 移除可能出现的多余空格
        while (supervisedPos < value.length() && value[supervisedPos] == ' ')
        {
            value.erase(supervisedPos, 1);
        }
        daemonizePos = value.find("--daemonize no");
        if (daemonizePos != std::string::npos)
        {
            value.erase(daemonizePos, std::string("--daemonize no").length());
            // 移除可能出现的多余空格
            while (daemonizePos < value.length() && (value[daemonizePos] == ' ' || value[daemonizePos - 1] == ' '))
            {
                value.erase(daemonizePos, 1);
            }
        }
    }

    // 将 id 插入到 value 的第一个空格后面
    size_t firstSpacePos = value.find(' ');
    if (firstSpacePos != std::string::npos)
    {
        // 在第一个空格后插入 " id -- "
        value.insert(firstSpacePos + 1, id + " -- ");
    }
    else
    {
        // 如果没有空格，直接在末尾添加 " id -- "
        value += " " + id + " -- ";
    }

    if (need)
    {
        return "-/opt/kaiming-tools/bin/kaiming run --detach=false --command=" + value + extraArgs;
    }

    return "/opt/kaiming-tools/bin/kaiming run --detach=false --command=" + value + extraArgs;
}

bool KMPackageManager::isField(const string &line, const string &field)
{
    return line.rfind(field + "=", 0) == 0;
}

string KMPackageManager::modifyFixedField(const string &line, const string id)
{
    static const std::unordered_set<std::string> commentFileds = {
        "CapabilityBoundingSet",
        "ProtectKernelTunables",
        "ProtectSystem",
        "User",
        "Group",
        "ExecStop",
        "ProtectHome",
        "ProtectSystem",
        "ProtectKernelTunables",
        "ExecPaths"};

    for (const auto &field : commentFileds)
    {
        if (isField(line, field))
        {
            return "#" + line; // 直接注释掉
        }
    }

    for (const auto &[field, modifier] : fixedModifications)
    {
        if (isField(line, field))
        {
            size_t equalPos = line.find('=');
            string originalValue = (equalPos != string::npos) ? line.substr(equalPos + 1) : "";

            if (holds_alternative<CallbackFunc>(modifier))
            {
                // 如果是回调函数，调用它
                CallbackFunc callback = get<CallbackFunc>(modifier);
                string newValue = callback(originalValue, id);
                return field + "=" + newValue;
            }
            else
            {
                // 如果是固定值
                string newValue = get<string>(modifier);
                return field + "=" + newValue;
            }
        }
    }
    return line; // 不修改
}

void KMPackageManager::modifyAppServiceFile(KMPMRef &pmRef, string serviceFile, string destPath)
{
    // 用户级服务跳过改写，避免 User=root/Group=root 导致启动失败
    if (destPath.find("systemd/user") != std::string::npos)
    {
        fs::path dest = destPath;
        if (!fs::exists(dest.parent_path()))
            fs::create_directories(dest.parent_path());
        std::vector<std::string> serviceDatas = getDataFromKVFile(serviceFile);
        if (setDataToKVFile(dest, serviceDatas))
            kmlogger.debug("User service deployed unchanged: %s", destPath.c_str());
        else
            kmlogger.error("Failed to deploy user service unchanged: %s", destPath.c_str());
        return;
    }

    bool noModifyServiceFileFlag = false;

    std::vector<std::string> serviceDatas = getDataFromKVFile(serviceFile);

    bool inServiceSection = false;
    bool hasUser = false;
    bool hasGroup = false;
    size_t serviceSectionEnd = 0;

    for (size_t i = 0; i < serviceDatas.size(); ++i)
    {
        auto &currentLine = serviceDatas[i];
        std::string trimmed = KMStringUtils::strstrip(currentLine);
        if (trimmed.empty() || trimmed[0] == '#')
        {
            continue;
        }

        // 检查是否进入[Service]部分
        if (trimmed == "[Service]")
        {
            inServiceSection = true;
            serviceSectionEnd = i + 1;
            continue;
        }

        // 只在[Service]部分中修改
        if (inServiceSection)
        {
            string modifiedLine = modifyFixedField(currentLine, pmRef.ref.id);
            if (modifiedLine != currentLine)
            {
                currentLine = modifiedLine;
            }
        }
    }

    if (inServiceSection)
    {
        // 直接在服务段末尾插入这三个字段
        serviceDatas.insert(serviceDatas.begin() + serviceSectionEnd, "User=root");
        serviceDatas.insert(serviceDatas.begin() + serviceSectionEnd + 1, "Group=root");
        serviceDatas.insert(serviceDatas.begin() + serviceSectionEnd + 2, "ExecStop=" + addKillPrefix("", pmRef.ref.id));

        // 更新服务段结束位置（增加了3行）
        serviceSectionEnd += 3;
    }

    fs::path dest = destPath;

    if (!fs::exists(dest.parent_path()))
    {
        if (fs::create_directories(dest.parent_path()))
        {
            if (setDataToKVFile(dest, serviceDatas))
            {
                kmlogger.debug("Service file modified successfully.");
            }
            else
            {
                kmlogger.error("Failed to get files in directory.");
            }
        }
    }
    else
    {
        if (setDataToKVFile(dest, serviceDatas))
        {
            kmlogger.debug("Service file modified successfully.");
        }
        else
        {
            kmlogger.error("Failed to get files in directory.");
        }
    }
}

bool KMPackageManager::getAllFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &files)
{
    try
    {
        // 遍历目录中的文件
        for (const auto &entry : fs::recursive_directory_iterator(directoryPath, fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink))
        {
            if (fs::is_symlink(entry.path()) && fs::read_symlink(entry.path()) == entry.path())
            {
                continue;
            }

            // 将文件路径添加到文件列表中
            files.push_back(entry.path().string());
        }

        return true; // 成功获取文件列表
    }
    catch (const std::exception &e)
    {
        kmlogger.error("Failed to get files: %s", e.what());
        return false; // 获取文件列表失败
    }
}

bool KMPackageManager::getAllAppFilesInDirectory(const std::string &directoryPath, std::set<std::string> &files, const std::string &id)
{
    static const std::vector<std::string> binDirs = {
        "files/bin", "files/local/bin", "files/usr/bin", "files/usr/local", "files/opt"};

    KMInfoJson infoJson;
    const std::string jsonPath = directoryPath + "/" + "info.json";
    infoJson.loadFile(jsonPath);
    bool isSysapp = infoJson.getAnnotations().sysapp;

    const std::string infoDir = d->m_folder->getInfoPath();
    fs::create_directories(infoDir);

    std::vector<std::pair<std::string, std::string>> vecExecRecords;
    const fs::path dirPath(directoryPath);
    const std::string appPrefix = "/opt/apps/" + id + "/files";

    try
    {
        std::vector<fs::path> fullBinPaths;
        for (const auto &subdir : binDirs)
        {
            fullBinPaths.push_back(dirPath / subdir);
        }

        // 用于检测循环符号链接的已访问目录集合
        std::set<std::string> visitedDirs;

        for (auto it = fs::recursive_directory_iterator(dirPath, fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink);
             it != fs::recursive_directory_iterator(); ++it)
        {
            try
            {
                // 检测并跳过符号链接循环
                if (fs::is_directory(it->symlink_status()))
                {
                    auto realPath = fs::weakly_canonical(it->path()).string();
                    if (!visitedDirs.insert(realPath).second)
                    {
                        it.disable_recursion_pending(); // 已访问过，跳过
                        continue;
                    }
                }

                // 跳过自引用符号链接
                if (fs::is_symlink(it->path()))
                {
                    auto target = fs::read_symlink(it->path());

                    // 1. 跳过自引用（. 和 ..）
                    if (target == "." || target == "..")
                    {
                        it.disable_recursion_pending();
                        continue;
                    }

                    // 2. 跳过指向 /proc/self/ 的链接（这是进程相关的虚拟文件系统）
                    if (target.string().find("/proc/self/") == 0)
                    {
                        it.disable_recursion_pending();
                        continue;
                    }

                    // 3. 跳过指向 /proc/ 并且包含数字的路径（避免进入其他进程）
                    if (target.string().find("/proc/") == 0)
                    {
                        // 检查是否是指向数字目录（进程ID）
                        auto remaining = target.string().substr(6); // 去掉 "/proc/"
                        if (!remaining.empty() && std::all_of(remaining.begin(), remaining.end(), [](char c) { return std::isdigit(c) || c == '/'; }))
                        {
                            it.disable_recursion_pending();
                            continue;
                        }
                    }
                }

                const auto &path = it->path();
                const auto pathStr = path.string();

                files.insert(pathStr);

                auto perms = fs::symlink_status(path).permissions();
                if ((perms & fs::perms::owner_exec) == fs::perms::none && (perms & fs::perms::group_exec) == fs::perms::none && (perms & fs::perms::others_exec) == fs::perms::none)
                {
                    continue; // 没有执行权限，直接跳过
                }

                if (KMFileUtils::isNonExecExt(path) || !KMFileUtils::isExecutableByFileCmd(path))
                {
                    continue;
                }

                // 检查是否在 bin 路径内
                for (const auto &binPath : fullBinPaths)
                {
                    if (pathStr.find(binPath.string()) == 0)
                    {
                        const std::string execName = path.filename().string();
                        fs::path linkPath;

                        if (binPath.filename() == "opt" && KMFileUtils::isShellScriptByFileCmd(path))
                        {
                            std::string strId = id + "_shellscript";
                            linkPath = fs::path(KAIMING_BIN_PATH) / strId / execName;
                            fs::create_directories(linkPath.parent_path());
                            d->m_pmBinName.push_back(linkPath);
                        }
                        else
                        {
                            linkPath = fs::path(KAIMING_BIN_PATH) / execName;
                            d->m_pmBinName.push_back(execName);
                        }

                        if (fs::exists(linkPath))
                        {
                            fs::remove(linkPath);
                        }
                        fs::create_symlink(KAIMING_PROGRESS_FULL_PATH, linkPath);
                        d->m_installedFiles.insert(linkPath);

                        std::string relPath;
                        const auto pos = pathStr.find("files");
                        relPath = (pos != std::string::npos) ? pathStr.substr(pos + 5) : path.filename().string();
                        if (!isSysapp)
                        {
                            relPath = appPrefix + relPath;
                        }
                        vecExecRecords.emplace_back(std::move(relPath), id);
                        break;
                    }
                }
            }
            catch (const std::exception &ex)
            {
                kmlogger.warn("跳过路径 %s: %s", it->path().string().c_str(), ex.what());
                it.disable_recursion_pending();
                continue;
            }
        }

        // 写入 bin 记录文件
        const std::string infoFile = (fs::path(infoDir) / (id + ".bin")).string();
        std::ofstream ofs(infoFile, std::ios::out | std::ios::trunc);
        if (ofs)
        {
            for (const auto &kv : vecExecRecords)
            {
                ofs << kv.first << " " << kv.second << "\n";
            }
        }

        return true;
    }
    catch (const std::exception &e)
    {
        kmlogger.error("Failed to get files: %s", e.what());
        return false;
    }
}

bool KMPackageManager::getAllBaseFilesInDirectory(const std::string &directoryPath, std::set<std::string> &files)
{
    try
    {
        // 遍历目录中的文件
        for (const auto &entry : fs::recursive_directory_iterator(directoryPath, fs::directory_options::skip_permission_denied))
        {
            if (fs::is_symlink(entry.path()) && fs::read_symlink(entry.path()) == entry.path())
            {
                continue;
            }

            // 将文件路径添加到文件列表中
            files.insert(entry.path().string());
        }

        return true; // 成功获取文件列表
    }
    catch (const std::exception &e)
    {
        kmlogger.error("Failed to get files: %s", e.what());
        return false; // 获取文件列表失败
    }
}

std::vector<std::string> KMPackageManager::getDataFromKVFile(const std::string &filename)
{
    std::vector<std::string> lines;
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();
    }
    else
    {
        kmlogger.error("Failed to open file::  %s", filename.c_str());
    }
    return lines;
}

bool KMPackageManager::setDataToKVFile(const std::string &filename, const std::vector<std::string> &newLines)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        for (const auto &line : newLines)
        {
            file << line << std::endl;
        }
        file.close();
        return true;
    }
    else
    {
        kmlogger.error("inFailed to open file for writingdex:  %s", filename.c_str());
        return false;
    }
}

void KMPackageManager::modifyDesktopFile(KMPMRef &pmRef, string exportDesktopFilePath, const string exportPath)
{
    kmlogger.info("Begin to exportDesktopFilePath: %s\n", exportDesktopFilePath.c_str());
    std::vector<std::string> desktopDatas = getDataFromKVFile(exportDesktopFilePath);
    struct stat originalStat;
    stat(exportDesktopFilePath.c_str(), &originalStat);
    bool needAddKaimingTag = false;
    // std::string baseName;

    // for (auto line : desktopDatas)
    // {
    //     if (line.find("Name=") == 0)
    //     {
    //         baseName = line.substr(strlen("Name="));
    //         break;
    //     }
    // }
    // 遍历 desktopDatas 通过每行 line 修改需要的字段
    for (size_t i = 0; i < desktopDatas.size();)
    {
        std::string &line = desktopDatas[i];
        // 跳过开头为 # 的注释字段
        if (line[0] == '#')
        {
            i++;
            continue;
        }

        // 注释以下内容处理无图标或开始菜单无法点击运行问题
        std::vector<std::string> annotationVector = {
            "DBusActivatable", // 注释 DBusActivatable 字段，否则org.gnome.Sudoku包任务栏无图标
            "StartupNotify",   // 注释 StartupNotify 字段，否则org.gnome.Aisleriot包任务栏无图标
            "TryExec",         // 注释 TryExec 字段，否则org.gnome.Gitg和org.ggsoft.etw在开始菜单无法点击运行
            "StartupWMClass="};

        // 检查是否已经存在StartupWMClass字段
        bool hasStartupWMClass = false;
        for (const auto &line : desktopDatas)
        {
            if (line.find("StartupWMClass=") == 0)
            {
                hasStartupWMClass = true;
                break;
            }
        }

        bool needAnnotation = false;
        for (auto it = annotationVector.begin(); it != annotationVector.end(); ++it)
        {
            if (line.find(*it) == 0)
            {
                needAnnotation = true;
                break;
            }
        }

        if (needAnnotation)
        {
            if (line.find("StartupWMClass=") != 0) // 解决bug452897
            {
                line = "# " + line;
                i++;
                continue;
            }
        }

        // 修改 Exec 字段，否则开始菜单应用运行不起来
        string serviceExec = "Exec=";
        size_t eqPos = line.find("Exec=");

        if (eqPos != std::string::npos && eqPos == 0)
        {
            std::string value = line.substr(eqPos + strlen("Exec="));
            bool modified = false;
            string content;
            // if (!d->m_pmBinName.empty())
            // {
            //     for (auto binName : d->m_pmBinName)
            //     {
            //         std::string fileName;
            //         if (binName.find('/')!= std::string::npos)
            //         {
            //             std::filesystem::path p(binName);
            //             fileName = p.filename().string();
            //         }
            //         else
            //         {
            //             fileName = binName;
            //         }

            //         if (line.find(fileName)!= std::string::npos)
            //         {
            //             // 找到第一个空格的位置
            //             size_t spacePos = line.find(' ', 5); // 从"Exec="后的位置开始搜索
            //             if (spacePos != string::npos) {
            //                 // 替换空格前的内容
            //                 content = "Exec=" + binName + line.substr(spacePos);
            //             }
            //             else
            //             {
            //                 content = serviceExec.append(binName);
            //             }
            //             modified = true;
            //             break;
            //         }
            //     }
            // }

            // 提取 basename（如 /opt/apps/cn.kdocs.kdesktop/files/bin/kdesktop → kdesktop）

            auto extractBasename = [](const std::string &s)
            {
                size_t lastSlash = s.rfind('/');
                std::string name = (lastSlash == std::string::npos) ? s : s.substr(lastSlash + 1);
                size_t space = name.find(' ');
                return (space == std::string::npos) ? name : name.substr(0, space);
            };
            size_t commandPos = value.find("--command=");
            std::string baseName = (commandPos != std::string::npos)
                                       ? extractBasename(value.substr(commandPos + strlen("--command=")))
                                       : extractBasename(value);

            if (value.find(" run") == std::string::npos)
            {
                std::string formatExec = d->m_util->formatExecString(value, pmRef.ref.id);
                kmlogger.info("formatExec: %s", formatExec.c_str());
                line = "Exec=/opt/kaiming-tools/bin/kaiming run " + formatExec;
            }
            else
            {
                size_t pos = line.find("/usr/bin/kaiming");
                if (pos != std::string::npos)
                {
                    line.replace(pos, 16, "/opt/kaiming-tools/bin/kaiming");
                }

                size_t refIdPos = line.find(pmRef.ref.id);
                if (refIdPos != std::string::npos)
                {
                    if (line.find("--", refIdPos + pmRef.ref.id.length()) == std::string::npos)
                    {
                        line.insert(refIdPos + pmRef.ref.id.length(), " --");
                    }
                }
            }

            // 不存在StartupWMClass字段才新增
            if (!hasStartupWMClass)
            {
                if (i + 1 < desktopDatas.size())
                {
                    desktopDatas.insert(desktopDatas.begin() + i + 1, "StartupWMClass=" + baseName);
                    i += 2; // 跳过新增行
                }
                else
                {
                    desktopDatas.push_back("StartupWMClass=" + baseName);
                    i++;
                }
                continue;
            }
        }

        // 保留，暂不修改 Icon 字段
        if (line.find("Icon=") == 0)
        {
            size_t iconPos = line.find("=") + 1;
            std::string path = line.substr(iconPos);

            if (path.find("/usr") == 0)
            {
                path.replace(0, 4, "/opt/kaiming");
            }

            line = "Icon=" + path;
        }

        // 新增 X-Kaiming 字段，用于SDK，开始菜单等获取软件包名
        if (line.find("X-Kaiming=") == 0)
        {
            needAddKaimingTag = false;
        }
        else
        {
            needAddKaimingTag = true;
        }

        i++;
    }

    if (needAddKaimingTag)
    {
        for (auto it = desktopDatas.begin(); it != desktopDatas.end(); ++it)
        {
            if (it->find("[Desktop Entry]") != std::string::npos)
            {
                string kaimingTag = "X-Kaiming=" + pmRef.ref.id;
                desktopDatas.insert(it + 1, kaimingTag);
                break;
            }
        }
    }

    std::string bakpath = exportPath + "_bak";
    // 将修改后的内容写回 .desktop 文件
    if (!setDataToKVFile(bakpath, desktopDatas))
    {
        kmlogger.error("Desktop file modified failed.");
    }

    std::rename(bakpath.c_str(), exportPath.c_str());
    chmod(exportPath.c_str(), originalStat.st_mode);

    // fs::path userDir = KMStorageDir::getUserDirLocation();
    // size_t pos = exportDesktopFilePath.rfind('/');
    // std::string basename = exportDesktopFilePath.substr(pos + 1);
    // auto appDir = userDir / ".var" / "app" / pmRef.ref.id / "upperdir" / "usr" / "share" / "applications";

    // bool chowner = false;
    // if (! fs::exists(fs::path(userDir / ".var" / "app")))
    // {
    //     chowner = true;
    // }

    // if (!fs::exists(appDir) && !fs::create_directories(appDir))
    // {
    //     kmlogger.error ("Error creating directory: %s", appDir.c_str());
    //     return;
    // }

    // std::string appDesktop = appDir / basename;

    // if (!setDataToKVFile(appDesktop, desktopDatas))
    // {
    //     kmlogger.error("Desktop file modified failed.");
    // }

    // if (chowner)
    // {
    //     modifyDirOwnership(userDir / ".var");
    // }
    // else
    // {
    //     modifyDirOwnership(userDir / ".var" / "app" / pmRef.ref.id);
    // }

    // kmlogger.info(" destFilePath: %s", appDesktop.c_str());
    // d->m_installedFiles.insert(appDesktop);

    return;
}

bool KMPackageManager::createDesktopShortcut(const KMPMRef &pmRef, const std::string &shortcutPath)
{
    fs::path sourceFile = shortcutPath;
    fs::path fileName = fs::path(shortcutPath).filename();
    fs::path basePath = d->m_folder->getPath();
    const fs::path infoFile = basePath / "layers" / pmRef.completedRef / "info.json";

    if (!fs::exists(infoFile))
    {
        kmlogger.debug("can't find app info file: %s", infoFile.string().c_str());
        return false;
    }

    KMInfoJson infoJson;
    infoJson.loadFile(infoFile);

    if (infoJson.getAnnotations().desktops.empty())
    {
        return false;
    }
    auto& desktops = infoJson.getAnnotations().desktops;
    if (std::find(desktops.begin(), desktops.end(), fileName.string()) == desktops.end())
    {
        return false;
    }

    std::vector<fs::path> userDesktopDirs;
    DIR *dir = opendir("/home");
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                fs::path path = fs::path("/home") / entry->d_name / "桌面";
                if (fs::exists(path))
                {
                    userDesktopDirs.push_back(path);
                }
            }
        }
        closedir(dir);
    }

    for (const auto &userDesktop : userDesktopDirs)
    {
        fs::path userTargetFile = userDesktop / fileName;
        try
        {
            if (fs::exists(sourceFile))
            {
                fs::copy_file(sourceFile, userTargetFile, fs::copy_options::overwrite_existing);
                KMStorageDir::modifyDirOwnership(userTargetFile);
                chmod(userTargetFile.c_str(), 0755);
                d->m_installedFiles.insert(userTargetFile.string());
                kmlogger.info("Copied desktop file to user desktop: %s", userTargetFile.string().c_str());
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to copy desktop file to user desktop: %s : %s", userTargetFile.string().c_str(), e.what());
            return false;
        }
    }

    return true;
}

bool KMPackageManager::recordDesktopMd5(const std::string &id, const std::string &desktopFilePath, std::set<std::string> &installedFiles)
{
    try
    {
        if (id.empty() || desktopFilePath.empty())
        {
            return false;
        }

        fs::path infoDir = d->m_folder->getInfoPath();
        if (!fs::exists(infoDir))
        {
            fs::create_directories(infoDir);
        }

        fs::path md5File = infoDir / (id + ".md5");
        const std::string md5 = KMFileUtils::getFileMd5sum(desktopFilePath);
        if (md5.empty())
        {
            kmlogger.warn("Failed to calc md5 for %s", desktopFilePath.c_str());
            return false;
        }

        std::ofstream ofs(md5File, std::ios::app);
        if (!ofs)
        {
            kmlogger.error("Open md5 file failed: %s", md5File.string().c_str());
            return false;
        }
        const std::string baseName = fs::path(desktopFilePath).filename().string();
        ofs << baseName << " " << md5 << "\n";
        ofs.close();

        installedFiles.insert(md5File.string());

        return true;
    }
    catch (const std::exception &e)
    {
        kmlogger.error("recordDesktopMd5 failed: %s", e.what());
        return false;
    }
}

bool KMPackageManager::removeUserDesktopShortcutsByMd5(const std::string &id)
{
    try
    {
        if (id.empty())
        {
            return false;
        }

        fs::path infoDir = d->m_folder->getInfoPath();
        fs::path md5File = infoDir / (id + ".md5");
        if (!fs::exists(md5File))
        {
            kmlogger.info("md5 record file not found for %s", id.c_str());
            return false;
        }

        std::ifstream ifs(md5File);
        if (!ifs)
        {
            kmlogger.error("Open md5 file failed: %s", md5File.string().c_str());
            return false;
        }

        // collect md5 values
        std::unordered_set<std::string> md5Set;
        std::string line;
        while (std::getline(ifs, line))
        {
            if (line.empty())
                continue;
            std::istringstream iss(line);
            std::string fname, md5;
            if (!(iss >> fname >> md5))
                continue;
            if (!md5.empty())
                md5Set.insert(md5);
        }
        ifs.close();

        if (md5Set.empty())
        {
            return false;
        }

        // scan all users' Desktop directories
        std::vector<fs::path> desktopDirs;
        DIR *dir = opendir("/home");
        if (dir)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != nullptr)
            {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    fs::path zh = fs::path("/home") / entry->d_name / "桌面";
                    fs::path en = fs::path("/home") / entry->d_name / "Desktop";
                    if (fs::exists(zh))
                        desktopDirs.push_back(zh);
                    if (fs::exists(en))
                        desktopDirs.push_back(en);
                }
            }
            closedir(dir);
        }

        for (const auto &desk : desktopDirs)
        {
            for (const auto &entry : fs::directory_iterator(desk))
            {
                if (!entry.is_regular_file())
                    continue;
                const auto &p = entry.path();
                if (p.extension() != ".desktop")
                    continue;

                std::string curMd5 = KMFileUtils::getFileMd5sum(p.string());
                if (!curMd5.empty() && md5Set.find(curMd5) != md5Set.end())
                {
                    std::error_code ec;
                    fs::remove(p, ec);
                    if (ec)
                    {
                        kmlogger.warn("Failed to remove desktop file: %s (%s)", p.string().c_str(), ec.message().c_str());
                    }
                    else
                    {
                        kmlogger.info("Removed desktop shortcut: %s", p.string().c_str());
                    }
                }
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        kmlogger.error("removeUserDesktopShortcutsByMd5 failed: %s", e.what());
        return false;
    }
}

bool KMPackageManager::createExportBinShellScript(KMPMRef pmRef, string fileName)
{
    // 设置内容
    string shellHead = "#!/bin/sh";
    string content = shellHead.append("\nexec ").append("/opt/kaiming-tools/bin/kaiming run --branch=").append(pmRef.ref.channel).append(" --arch=").append(pmRef.ref.arch).append(" ").append(pmRef.ref.id).append(" \"$@\"\n");

    kmlogger.info("export bin bash content:\n%s", content.c_str());
    // 创建文件并写入内容
    std::ofstream file(fileName);
    if (file.is_open())
    {
        file << content;
        file.close();
    }
    else
    {
        kmlogger.error("Failed to open file:  %s", fileName.c_str());
        return false;
    }

    // 设置文件权限为 0755
    if (chmod(fileName.c_str(), 0755) != 0)
    {
        kmlogger.error("Failed to set file permission for:  %s", fileName.c_str());
        return false;
    }

    return true;
}

bool KMPackageManager::createInstalledFile(string fileName, bool append, const std::set<std::string> &installedFiles)
{
    // 设置内容
    string content;

    for (auto &installedFile : installedFiles)
    {
        content = content.append(installedFile).append("\n");
    }

    // 创建文件并写入内容
    std::ofstream file;
    if (append)
    {
        file.open(fileName, std::ios::app); // 追加模式
    }
    else
    {
        if (fs::exists(fileName))
        {
            fs::remove(fileName);
        }
        file.open(fileName, std::ios::out); // 覆盖模式（默认）
    }

    if (file.is_open())
    {
        file << content;
        file.close();
    }
    else
    {
        kmlogger.error("Failed to open file:  %s", fileName.c_str());
        return false;
    }

    // 设置文件权限为 0755
    if (chmod(fileName.c_str(), 0755) != 0)
    {
        kmlogger.error("Failed to set file permission for:  %s", fileName.c_str());
        return false;
    }

    return true;
}

void KMPackageManager::writeInstalledFilesToList(const std::string &filePath, const std::set<std::string> &installedFiles)
{
    fs::path dirPath = fs::path(filePath).parent_path();

    if (!fs::exists(dirPath))
    {
        if (!fs::create_directories(dirPath))
        {
            KMError("Error creating directory: " + dirPath.string());
            return;
        }
    }

    std::ofstream outFile(filePath);

    if (!outFile)
    {
        KMError("Error opening file: " + filePath);
        return;
    }

    for (const auto &installedFile : installedFiles)
    {
        outFile << installedFile << "\n";
    }

    outFile.close();
}

void KMPackageManager::modifyExportsDirectory(KMPMRef &pmRef, const std::string &srcDir, const std::string &destDir, std::set<std::string> &installedFiles)
{
    bool findDesktop = false;
    for (const auto &entry : fs::directory_iterator(srcDir))
    {
        if (fs::is_directory(entry))
        {
            // 递归处理子目录
            std::string subSrcDir = entry.path().string();
            std::string subDestDir = destDir + "/" + entry.path().filename().string();

            string removeStr = "/usr";
            size_t pos = subDestDir.find(removeStr);

            if (pos != std::string::npos)
            {
                subDestDir.erase(pos, removeStr.length());
            }

            if (subSrcDir.find("entries/share/kylin-user-guide/data/guide/") != std::string::npos)
            {
                size_t pos = subSrcDir.rfind('/');
                std::string basename = subSrcDir.substr(pos + 1);
                std::string dataDestPath = "/opt/system/resource/kylin/kylin-user-guide/data/guide/" + basename;
                handleSpecialFileInstall(subSrcDir, dataDestPath, true);
                continue;
            }

            fs::create_directory(subDestDir);
            modifyExportsDirectory(pmRef, subSrcDir, subDestDir, installedFiles);
        }
        else
        {
            // 处理文件
            std::string srcFilePath = entry.path().string();
            std::string destFilePath = destDir + "/" + entry.path().filename().string();
            kmlogger.debug("srcFilePath:%s destFilePath:%s", srcFilePath.c_str(), destFilePath.c_str());

            string removeStr = "/usr";
            size_t pos = destFilePath.find(removeStr);
            if (pos != std::string::npos)
            {
                destFilePath.erase(pos, removeStr.length());
            }
            kmlogger.debug("srcFilePath:%s destFilePath:%s", srcFilePath.c_str(), destFilePath.c_str());

            if (fs::exists(fs::path(destFilePath)) && fs::is_symlink(fs::path(destFilePath)))
            {
                kmlogger.info("destFilePath  %s exists.", destFilePath.c_str());
                installedFiles.insert(destFilePath);
                continue;
            }

            if (srcFilePath.find("entries/share/pixmaps/") != std::string::npos)
            {
                string destDir = "/opt/kaiming/share/icons/hicolor/32x32/apps/";
                string destDir2 = "/opt/kaiming/share/icons/hicolor/scalable/apps/";
                size_t pos = srcFilePath.find("entries/share/pixmaps/");
                std::string basename = srcFilePath.substr(pos + 22);
                std::string destPath = destDir + basename;
                std::string destPath2 = destDir2 + basename;
                fs::path p(destPath);
                fs::path parent_path = p.parent_path();
                fs::path p2(destPath2);
                fs::path parent_path2 = p2.parent_path();
                std::string dataDestPath = parent_path.string();
                std::string dataDestPath2 = parent_path2.string();
                handleSpecialFileInstall(srcFilePath, dataDestPath, false);
                handleSpecialFileInstall(srcFilePath, dataDestPath2, false);
            }
            else if (srcFilePath.find(".desktop") != std::string::npos || srcFilePath.find(".service") != std::string::npos || srcFilePath.find(".svg") != std::string::npos || srcFilePath.find(".png") != std::string::npos || srcFilePath.find(".xpm") != std::string::npos)
            {

                // 如果是 .desktop, .service, .svg 文件，则创建相对路径软链接到目标目录
                // 此处以 ".desktop" 为后缀，否则会出现 "org.freedesktop.impl.portal.desktop.gtk.service" 也会进行修改。

                if (srcFilePath.find("entries/etc/xdg/autostart") != std::string::npos)
                {
                    // fs::path userDir = KMStorageDir::getUserDirLocation();
                    // fs::path destXdgPath = userDir / ".config" / "autostart";
                    fs::path destXdgPath = "/etc/xdg/autostart";
                    fs::path destFile = destXdgPath / fs::path(srcFilePath).filename();

                    if (KMStringUtils::endsWith(srcFilePath, ".desktop"))
                    {
                        modifyDesktopFile(pmRef, srcFilePath, destFile);
                        kmlogger.info("autostart .desktop srcPath:  %s destFilePath: %s", srcFilePath.c_str(), destFilePath.c_str());
                        installedFiles.insert(destFile.string());
                    }
                }
                else
                {
                    if (KMStringUtils::endsWith(srcFilePath, ".desktop"))
                    {
                        modifyDesktopFile(pmRef, srcFilePath, destFilePath);
                        recordDesktopMd5(pmRef.ref.id, destFilePath, installedFiles);
                        createDesktopShortcut(pmRef, destFilePath);
                        kmlogger.info(".desktop srcPath: %s destFilePath: %s", srcFilePath.c_str(), destFilePath.c_str());

                        // 在 /opt/kaiming/share/idtodesktop 文件中记录映射关系
                        if (!findDesktop)
                        {
                            std::string idToDesktopFile = "/opt/kaiming/share/idtodesktop";
                            std::vector<std::string> idToDesktopFileDatas = getDataFromKVFile(idToDesktopFile);
                            size_t lastSlashPos = srcFilePath.find_last_of("/\\");
                            std::string desktopFileName = (lastSlashPos == std::string::npos) ? srcFilePath : srcFilePath.substr(lastSlashPos + 1);
                            std::string idDesktop = pmRef.ref.id + '=';
                            bool hasId = false;

                            size_t lastDotPos = pmRef.ref.id.rfind('.');
                            std::string result;
                            if (lastDotPos != std::string::npos)
                            {
                                result= pmRef.ref.id.substr(lastDotPos + 1);
                            }

                            if (!result.empty() && desktopFileName.find(result))
                            {
                                findDesktop = true;
                            }
                            for (auto& data : idToDesktopFileDatas)
                            {
                                if (data.find(idDesktop) != std::string::npos)
                                {
                                    data = idDesktop + desktopFileName;
                                    hasId = true;
                                    break;
                                }
                            }

                            if (!hasId)
                            {
                                idToDesktopFileDatas.push_back(idDesktop + desktopFileName);
                            }

                            setDataToKVFile(idToDesktopFile, idToDesktopFileDatas);
                        }
                        
                        installedFiles.insert(destFilePath);
                        continue;
                    }

                    // 获取 srcFilePath 相对于 destFilePath 的相对路径
                    std::string relativePath = fs::relative(fs::path(srcFilePath), fs::path(destFilePath).parent_path()).string();
                    kmlogger.info("Export .desktop, .service, .svg, .png relativePath:  %s destFilePath: %s", relativePath.c_str(), destFilePath.c_str());

                    if (fs::is_symlink(fs::path(destFilePath)) || fs::exists(fs::path(destFilePath)))
                    {
                        // 如果存在，删除该文件或链接
                        kmlogger.info("destFilePath  %s exists. delete it.", destFilePath.c_str());
                        fs::remove(fs::path(destFilePath));
                    }

                    fs::create_directories(fs::path(destFilePath).parent_path());

                    // 复制文件（如果目标已存在则覆盖）
                    fs::copy(fs::path(srcFilePath), fs::path(destFilePath), fs::copy_options::overwrite_existing);
                    installedFiles.insert(destFilePath);
                }
            }
            else if (srcFilePath.find("entries/usr/bin") != std::string::npos || srcFilePath.find("entries/bin") != std::string::npos)
            {
                // 如果是 entries/bin 二进制文件，则创建相对路径软链接到目标目录
                std::string relativePath = fs::relative(fs::path(srcFilePath), fs::path(destFilePath).parent_path()).string();
                kmlogger.info("Export bin relativePath:  %s destFilePath: %s", relativePath.c_str(), destFilePath.c_str());

                // 使用 fs::exists 检测软链接的存在，它会返回目标文件的存在状态，而不是软链接本身。因此，使用 fs::is_symlink 是判断软链接是否存在的正确方法。
                if (fs::is_symlink(fs::path(destFilePath)))
                {
                    // 如果存在，删除该文件或链接
                    kmlogger.info("destFilePath  %s exists. delete it.", destFilePath.c_str());
                    fs::remove(fs::path(destFilePath));
                }

                fs::create_symlink(fs::path(relativePath), fs::path(destFilePath));
                installedFiles.insert(destFilePath);
            }
            else if (srcFilePath.find(".xml") != std::string::npos)
            {
                std::string resourceDir = "/opt/system/resource/mime/packages";
                handleSpecialFileInstall(srcFilePath, resourceDir, false);
            }
            else if (srcFilePath.find(".policy") != std::string::npos)
            {
                string destDir = KMFileUtils::isImmutableSystem() ? "/opt/system/resource/polkit-1/actions" : "/usr/share/polkit-1/actions";
                handleSpecialFileInstall(srcFilePath, destDir, false);
            }
            else if (srcFilePath.find("entries/home/") != std::string::npos)
            {
                fs::path userDir = KMStorageDir::getUserDirLocation();
                size_t pos = srcFilePath.find("entries/home/");
                std::string relativePath = srcFilePath.substr(pos + strlen("entries/home/"));
                fs::path destPath = userDir / relativePath;
                fs::path destDirPath = destPath.parent_path();

                if (!fs::exists(destDirPath))
                {
                    fs::create_directories(destDirPath);
                    kmlogger.info("Created user home directory: %s", destDirPath.string().c_str());
                }

                if (fs::is_directory(srcFilePath))
                {
                    handleSpecialFileInstall(srcFilePath, destPath.string(), true);
                }
                else
                {
                    handleSpecialFileInstall(srcFilePath, destDirPath.string(), false);
                }

                continue;
            }
            else if (srcFilePath.find("entries/assets/") != std::string::npos)
            {
                size_t pos = srcFilePath.find("entries/assets/");
                std::string basename = srcFilePath.substr(pos + 15);
                std::string destPath = "/" + basename;
                fs::path p(destPath);
                fs::path parent_path = p.parent_path();
                std::string dataDestPath = parent_path.string();
                handleSpecialFileInstall(srcFilePath, dataDestPath, false);
                KMStorageDir::modifyDirOwnership(dataDestPath);
                continue;
            }
        }
    }
}

bool KMPackageManager::handleSpecialFileInstall(const std::string &srcFilePath, const std::string &destDir, bool isDir)
{
    try
    {
        // 1. 确保目标目录存在
        if (!fs::exists(destDir))
        {
            fs::create_directories(destDir);
            kmlogger.info("Created resource directory: %s", destDir.c_str());
        }

        // 2. 构造目标路径
        fs::path srcPath(srcFilePath);
        fs::path destDirPath(destDir);
        fs::path destPath = destDirPath / srcPath.filename();

        if (isDir)
        {
            fs::copy(srcPath, destDirPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            d->m_installedFiles.insert(destDir);
            return true;
        }

        kmlogger.info("Copying file from %s to %s", srcFilePath.c_str(), destPath.string().c_str());

        // 3. 检查并删除已存在的目标文件/链接
        if (fs::exists(destPath))
        {
            fs::remove(destPath);
            kmlogger.info("Removed existing file/link: %s", destPath.string().c_str());
        }

        // 4. 执行文件拷贝
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing);

        kmlogger.info("Successfully copied file to %s", destPath.string().c_str());

        // 5. 添加到已安装文件集合
        d->m_installedFiles.insert(destPath.string());

        return true;
    }
    catch (const fs::filesystem_error &e)
    {
        kmlogger.error("Failed to process file %s: %s", srcFilePath.c_str(), e.what());
        return false;
    }
}

void KMPackageManager::runMultiperTriggerUpdateCacheCommand(const vector<string> &cacheShellPaths)
{
    for (const auto &path : cacheShellPaths)
    {
        runTriggerUpdateCacheCommand(path);
    }
}

void KMPackageManager::runTriggerUpdateCacheCommand(const std::string &cacheShellPath)
{
    // 构建命令字符串，将脚本路径和参数拼接在一起
    std::string command = cacheShellPath + " " + "/opt/kaiming";

    kmlogger.info("Generate cache command: %s", command.c_str());

    // 执行命令
    int result = std::system(command.c_str());
    if (result != 0)
    {
        kmlogger.error("Failed to execute command:  %s", command.c_str());
    }
}

// 线程函数：实时读取rsync的输出并解析进度信息
void KMPackageManager::monitorProgress(FILE *pipe)
{
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        std::string line(buffer);
        // 查找包含百分比的行，表示文件传输的进度
        size_t firstPos = line.find('=');
        if (firstPos != std::string::npos)
        {
            size_t secondPos = line.find(')', firstPos + 1);
            // 提取进度百分比
            std::string progresStr = line.substr(firstPos, secondPos); // 获取进度
            size_t thirdPos = progresStr.find('/');
            std::string numBeforeStr = progresStr.substr(1, thirdPos - 1);
            int numBefore, numAfter;
            std::stringstream(numBeforeStr) >> numBefore;
            std::string numAfterStr = progresStr.substr(thirdPos + 1, progresStr.length() - thirdPos - 3);
            std::stringstream(numAfterStr) >> numAfter;
            int result = (numAfter - numBefore);
            int divid = (int)((double)result / numAfter * 100);
            sharedValue.store(divid, std::memory_order_relaxed); // 在线程中修改共享变量的值
        }
    }
}

std::string KMPackageManager::getCertificateOuname(KMLibHandle &lib)
{
    try
    {
        const std::string configPath = "/etc/dpkg/dpkg.cfg";

        if (!KMFileUtils::fileExists(configPath))
        {
            kmlogger.info("Config file not found: %s", configPath.c_str());
            return "";
        }

        using GetProfileStringFunc = int (*)(const char *, const char *, char *);
        auto GetProfileString = lib.loadSymbol<GetProfileStringFunc>("GetProfileString");
        if (!GetProfileString)
            return "";

        char ouname[1024] = {0};
        if (GetProfileString(configPath.c_str(), "ou", ouname) != 0)
        {
            kmlogger.info("Failed to get OU from config file");
            return "";
        }

        kmlogger.info("Got OU name: %s", ouname);
        return std::string(ouname);
    }
    catch (...)
    {
        kmlogger.error("getCertificateOuname failed");
        return "";
    }
}

std::string KMPackageManager::getPemCN(KMLibHandle &lib, const std::string &pemFile)
{
    try
    {
        using GetPemSubCNFunc = int (*)(const char *, char *);
        auto GetPemSubCN = lib.loadSymbol<GetPemSubCNFunc>("getPemSubCN");
        if (!GetPemSubCN)
            return "";

        char subjectCN[256] = {0};
        if (GetPemSubCN(pemFile.c_str(), subjectCN) != 0)
        {
            kmlogger.info("Failed to get CN");
            return "";
        }
        return std::string(subjectCN);
    }
    catch (...)
    {
        kmlogger.error("getPemCN failed");
        return "";
    }
}

bool KMPackageManager::isV10Sp1()
{
    std::ifstream file("/etc/os-release");

    bool isV10Sp1 = false;
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            if (KMStringUtils::startsWith(line, "PRETTY_NAME=") && KMStringUtils::contains(line, "Kylin V10 SP1"))
            {
                isV10Sp1 = true;
                kmlogger.info("The system is V10");
                break;
            }
        }
    }

    return isV10Sp1;
}

int KMPackageManager::verifySignature(std::string id, std::string kind, fs::path path)
{
    // base,runtime类型不验签，v10sp1不验签
    if (kind == "base" || kind == "runtime" || isV10Sp1())
    {
        kmlogger.info("base,runtime类型不验签, v10sp1不验签, 直接部署");
        return KM_ERROR_NO;
    }

    KMLibHandle libSign(nullptr);
    try
    {
        libSign = KMLibHandle("libkylin_signtool.so.0.0.0");
    }
    catch (...)
    {
        kmlogger.error("Skip signature verification");
        return KM_ERROR_NO;
    }

    int safeStatus = getSafeMode(libSign);
    kmlogger.info("Safe mode is: %d", safeStatus);

    if (safeStatus <= 1)
    {
        kmlogger.info("安全模式为关闭模式或获取安全模式失败，跳过验签");
        return KM_ERROR_NO;
    }

    // 签名工具不存在，跳过验签
    if (!isKylinFileSignToolPresent())
    {
        kmlogger.info("签名工具不存在，跳过验签");
        return KM_ERROR_NO;
    }

    // 签名文件检查
    std::string sigFilesPath = (path / "sig").string();
    std::string filesMd5 = KMStringUtils::buildFilename(path, "files.md5");
    std::string filesSign = KMStringUtils::buildFilename(sigFilesPath, "files.sig");
    std::string filesPem = KMStringUtils::buildFilename(sigFilesPath, "files.pem");

    if (!KMFileUtils::fileExists(filesMd5))
    {
        kmlogger.info("签名文件不存在，停止安装");
        std::cout << _("Signature file not exist, installation stopped") << std::endl;
        return KM_ERROR_NO_MD5_FILES;
    }

    if (!KMFileUtils::fileExists(filesSign) || !KMFileUtils::fileExists(filesPem))
    {
        kmlogger.info("files.sig or files.pem not exist");
        std::cout << _("Signature verification failed") << std::endl;
        int errorCode = resolveFailedVerification(safeStatus, path, id);
        return errorCode;
    }

    // 证书验证
    if (!verifyCertificate(libSign, filesPem))
    {
        // 证书验证失败
        kmlogger.info("证书验证失败");
        std::cout << _("Signature verification failed") << std::endl;
        int errorCode = resolveFailedVerification(safeStatus, path, id);
        return errorCode;
    }

    // 命令行验签
    int verifyResult = executeSignatureVerification(filesMd5, filesSign, filesPem);
    if (verifyResult != KM_ERROR_NO)
    {
        kmlogger.info("命令行验签失败");
        std::cout << _("Signature verification failed") << std::endl;
        int errorCode = resolveFailedVerification(safeStatus, path, id);
        return errorCode;
    }

    kmlogger.info("命令行验签成功");
    std::cout << _("Signature verification success") << std::endl;
    // MD5校验
    if (!KMFileUtils::isObsolete(filesMd5))
    {
        kmlogger.info("MD5 file is not old");
        KMDirMd5sum md5sum(path);
        if (!md5sum.checksum("files"))
        {
            kmlogger.info("cheksum MD5 files, the result is KM_ERROR_MD5_CHECK_FAILED");
            return KM_ERROR_MD5_CHECK_FAILED;
        }
    }

    kmlogger.info("Signature verification completed successfully");
    return KM_ERROR_NO;
}

bool KMPackageManager::verifyCertificate(KMLibHandle &lib, const std::string &pemFile)
{
    try
    {
        const std::string configPath = "/etc/dpkg/dpkg.cfg";

        if (!KMFileUtils::fileExists(configPath))
        {
            kmlogger.info("Config file not found: %s", configPath.c_str());
            return false;
        }

        using GetProfileStringFunc = int (*)(const char *, const char *, char *);
        using GetPemSubCNFunc = int (*)(const char *, char *);

        auto GetProfileString = lib.loadSymbol<GetProfileStringFunc>("GetProfileString");
        auto GetPemSubCN = lib.loadSymbol<GetPemSubCNFunc>("getPemSubCN");

        if (!GetProfileString || !GetPemSubCN)
        {
            kmlogger.info("Failed to load required symbols, skip certificate verification");
            return true;
        }

        char ouBuffer[1024] = {0};
        if (GetProfileString("/etc/dpkg/dpkg.cfg", "ou", ouBuffer) != 0)
        {
            kmlogger.info("Failed to get OU name, skip certificate verification");
            return true;
        }
        std::string ouname(ouBuffer);
        kmlogger.info("Got OU name: %s", ouname.c_str());

        if (ouname.empty())
        {
            kmlogger.info("OU name is empty, skip certificate verification");
            return true;
        }

        char cnBuffer[256] = {0};
        if (GetPemSubCN(pemFile.c_str(), cnBuffer) != 0)
        {
            kmlogger.info("Failed to get CN");
            return false;
        }
        std::string cn(cnBuffer);
        kmlogger.info("Got CN: %s", cn.c_str());

        if (!verifyCertCN(ouname, cn))
        {
            kmlogger.info("OU-CN mismatch (OU is : %s, CN is : %s)", ouname.c_str(), cn.c_str());
            return false;
        }

        kmlogger.info("Certificate verified (OU is : %s, CN is : %s)", ouname.c_str(), cn.c_str());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

int KMPackageManager::executeSignatureVerification(const std::string &md5File, const std::string &sigFile, const std::string &pemFile)
{
    std::string command = "kylinfilesigntool -v \"" + md5File + "\" -s \"" + sigFile + "\" -c \"" + pemFile + "\" >/dev/null 2>&1";
    try
    {
        kmlogger.info("Executing: %s", command.c_str());
        int ret = std::system(command.c_str());
        if (ret != 0)
        {
            kmlogger.info("Signature verification failed (exit code: %d)", ret);
            return KM_ERROR_SIGN_FAILED;
        }
        return KM_ERROR_NO;
    }
    catch (...)
    {
        kmlogger.info("Unexpected error during signature verification");
        return KM_ERROR_SIGN_FAILED;
    }
}

std::string KMPackageManager::trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> KMPackageManager::splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0, end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        // 分割子串，并去除前后空格
        std::string token = str.substr(start, end - start);
        token = trim(token);
        tokens.push_back(token);
        start = end + 1;
    }

    // 处理最后一个子串（可能没有分隔符的情况）
    std::string lastToken = str.substr(start);
    lastToken = trim(lastToken);
    tokens.push_back(lastToken);

    return tokens;
}

bool KMPackageManager::verifyCertCN(const std::string &ouname, const std::string &certCN)
{
    // 分割 OU 列表（如 "OU1,OU2" -> ["OU1", "OU2"]）
    std::vector<std::string> allowedOUs = splitString(ouname, ',');

    // 检查 CN 是否在 OU 列表中
    bool isValid = std::find(allowedOUs.begin(), allowedOUs.end(), certCN) != allowedOUs.end();
    if (!isValid)
    {
        kmlogger.error("CN verification failed: %s (allowed OUs: %s)", certCN.c_str(), ouname.c_str());
    }
    else
    {
        kmlogger.info("CN verification success: %s", certCN.c_str());
    }

    return isValid;
}

// 判断签名工具是否存在
bool KMPackageManager::isKylinFileSignToolPresent()
{
    int result = std::system("which kylinfilesigntool > /dev/null 2>&1");
    // 结果为0，找到工具
    return (result == 0);
}

KMErrorCodeEnum KMPackageManager::setKsafLabelAndExectl(const std::set<std::string> &files, const std::string &appName, const std::string &kind, const std::string &path)
{
    if (files.empty() || appName.empty())
    {
        kmlogger.info("Invalid parameters: files or appName is empty");
        return KM_ERROR_INVALID_PARAM;
    }

    try
    {
        KMLibHandle ksafLabelHandle("libksaflabelmgr.so");
        KMLibHandle exectlHandle("libkysec2_exectl_wlist.so.0.0.0");
        if (!ksafLabelHandle || !exectlHandle)
        {
            kmlogger.info("Failed to load required libraries");
            return KM_ERROR_SETLABEL_FAILED;
        }

        using SetLabelFunc = int (*)(const char **, const char *);
        using XattrSetFunc = int (*)(char *[], size_t);
        using SetLabelForInterpreterFunc = int (*)(const char *);

        std::vector<const char *> c_files;
        std::vector<char *> c_paths;
        c_files.reserve(files.size() + 1);
        c_paths.reserve(files.size());
        for (const auto &file : files)
        {
            c_files.push_back(file.c_str());
            c_paths.push_back(const_cast<char *>(file.c_str()));
        }
        c_files.push_back(nullptr); // 以nullptr结尾

        if ((kind == "base" || kind == "runtime") && !d->m_isHostapp)
        {
            // base/runtime只设置解释器的ksaf标记
            kmlogger.info("setLabelForKaimingInterpreter for base or runtime");
            auto setLabelForKaimingInterpreter = ksafLabelHandle.loadSymbol<SetLabelForInterpreterFunc>("setLabelForKaimingInterpreter");

            if (int ret = setLabelForKaimingInterpreter(path.c_str()); ret != 0)
            {
                kmlogger.error("Failed to 'setLabelForKaimingInterpreter': %d, path: %s", ret, path.c_str());
                return KM_ERROR_SETLABEL_FAILED;
            }
        }
        else
        {
            auto setLabelForKaiming = ksafLabelHandle.loadSymbol<SetLabelFunc>("setLabelForKaiming");

            if (int ret = setLabelForKaiming(c_files.data(), appName.c_str()); ret != 0)
            {
                kmlogger.error("Failed to 'setLabelForKaiming' (code: %d)", ret);
                return KM_ERROR_SETLABEL_FAILED;
            }
        }
        emitTotalDeployProgress(60);
        kmlogger.info("set label successfully");

        if (kind == "app" || d->m_isHostapp)
        {
            // exectl only set app
            auto exectl_xattr_multi_set = exectlHandle.loadSymbol<XattrSetFunc>("exectl_wlist_multi_add");
            emitTotalDeployProgress(70);
            int ret = exectl_xattr_multi_set(c_paths.data(), c_paths.size());
            emitTotalDeployProgress(80);
            if (ret < 0)
            {
                kmlogger.error("Failed to set xattr (code: %d)", ret);
                return KM_ERROR_SETLABEL_FAILED;
            }

            kmlogger.info("set exectl xattr successfully (code: %d)", ret);
        }
        emitTotalDeployProgress(85);
    }
    catch (...)
    {
        kmlogger.error("setKsafLabelAndExectl crashed unexpectedly");
        return KM_ERROR_SETLABEL_FAILED;
    }

    kmlogger.info("setKsafLabelAndExectl completed successfully");
    return KM_ERROR_NO;
}

int KMPackageManager::getSafeMode(KMLibHandle &lib)
{
    try
    {
        using GetDpkgInfoFunc = int (*)();
        auto GetDpkgInfo = lib.loadSymbol<GetDpkgInfoFunc>("get_dpkg_file_info");
        if (!GetDpkgInfo)
            return 0;
        return GetDpkgInfo();
    }
    catch (...)
    {
        kmlogger.error("getSafeMode failed");
        return 0;
    }
}

int KMPackageManager::resolveFailedVerification(int safeStatus, fs::path idPath, std::string id)
{
    KMLibHandle libSign("libkysec.so.0.0.0");

    switch (safeStatus)
    {
    case 1:
        kmlogger.info("安全模式为关闭模式，跳过验签");
        return KM_ERROR_NO;

    case 2:
    {
        try
        {
            using SignCheckFunc = int (*)(const char *, const char *);
            SignCheckFunc kysec_check_custom_pkg_sign = libSign.loadSymbol<SignCheckFunc>("kysec_check_custom_pkg_sign");

            // 返回值 1:允许安装;0:禁止安装
            int signStatus = kysec_check_custom_pkg_sign(const_cast<char *>(id.c_str()), const_cast<char *>(idPath.c_str()));
            kmlogger.info("User selection after popup: %s", signStatus ? "允许安装" : "禁止安装");

            return signStatus == 1 ? KM_ERROR_NO : KM_ERROR_USER_ABORTED;
        }
        catch (...)
        {
            kmlogger.error("resolveFailedVerification crashed unexpectedly");
            return KM_ERROR_SIGN_FAILED;
        }
    }

    case 3:
    {
        kmlogger.info("安全模式为阻止模式，验签失败，拒绝安装");
        try
        {
            // 阻止弹窗
            using NotifyFunc = int (*)(const char *, const int *);
            NotifyFunc kysec_notify_pkg_sign = libSign.loadSymbol<NotifyFunc>("kysec_notify_pkg_sign");

            kmlogger.info("idPath  is  %s", idPath.c_str());
            kmlogger.info("id  is  %s", id.c_str());
            int signStatus = kysec_notify_pkg_sign(const_cast<char *>(id.c_str()), 0);
            kmlogger.info("Notification sent with status: %d", signStatus);
            return KM_ERROR_SIGN_FAILED;
        }
        catch (...)
        {
            kmlogger.error("resolveFailedVerification crashed unexpectedly");
            return KM_ERROR_SIGN_FAILED;
        }
    }

    default:
        kmlogger.error("获取安全模式失败");
        return KM_ERROR_GET_SAFE_MODE_FAILED;
    }
}

int KMPackageManager::pullFromOstree(fs::path path, KMPMRef downloadRef)
{
    fs::path tmpPath = path;
    kmlogger.info("Begin to deploy temporary files to: %s", tmpPath.string().c_str());

    // d->m_installedFiles.clear();
    kmlogger.info("completedRef: %s", downloadRef.completedRef.c_str());

    fs::path idPath = tmpPath / downloadRef.ref.id;
    kmlogger.debug("idPath: %s", idPath.string().c_str());

    // d->m_installedFiles.insert(idPath.string());

    if (std::filesystem::exists(idPath) || std::filesystem::create_directories(idPath))
    {
        kmlogger.debug("Directory created or already exists successfully!");
        fs::remove_all(idPath);
    }

    kmlogger.debug("Begin to checkout ...");
    string latestCommit = d->m_ostreeHandler->readLatestCommit(downloadRef.completedRef);
    // 生成files.md5等文件,从 OSTree 仓库中检出（checkout）指定的提交（commit），并将内容部署到指定的路径
    bool isCheckout = d->m_ostreeHandler->checkout(latestCommit, idPath.string());
    std::string removeStr;

    if (!isCheckout)
    {
        if (downloadRef.ref.module == "binary")
        {
            fs::path rootPath = d->m_folder->getLayersPath();
            removeStr = rootPath.string() + "/" + downloadRef.completedRef;
        }
        if (downloadRef.ref.module == "devel")
        {
            fs::path develDir = fs::path(KMStorageDir::getUserBaseDirLocation());
            removeStr = develDir.string() + "/" + downloadRef.completedRef;
        }
        deleteSpecificRefsFromRepo(removeStr);
        std::cout << "Checkout files failed" << std::endl;
        kmlogger.info("Checkout files failed");
        return KM_ERROR_INSTALL_FAILED;
    }
    
    kmlogger.debug("End to checkout.");
    kmlogger.info("End to deploy files.");

    return 0;
}

string KMPackageManager::getRemoteUrl(const string &remoteName)
{
    return d->m_ostreeHandler->getRemoteUrl(remoteName);
}

bool KMPackageManager::clearCacheFiles(const string &id)
{
    const fs::path infoPath = d->m_folder->getInfoPath();
    const fs::path infoListFile = infoPath / (id + ".list");
    const fs::path binListFile = infoPath / (id + ".bin");

    std::error_code ec;
    bool allRemoved = true;

    auto removeFile = [&allRemoved](const fs::path &file)
    {
        std::error_code ec;
        if (!fs::remove(file, ec))
        {
            if (ec.value() != static_cast<int>(std::errc::no_such_file_or_directory))
            {
                kmlogger.debug("Failed to remove %s: %s", file.string().c_str(), ec.message().c_str());
                allRemoved = false;
            }
        }
        return !ec;
    };

    removeFile(infoListFile);
    removeFile(binListFile);

    return allRemoved;
}

int KMPackageManager::removeHostAppDeployFiles(const std::string &strPath, TaskType type)
{
    std::string packageId = KMStringUtils::extractPackageId(strPath);
    if (!packageId.empty())
    {
        d->m_hostAppDeploy->setHostAppConfig(strPath, packageId);
        if (d->m_hostAppDeploy->loadHostAppInfo())
        {
            int uninstallResult = d->m_hostAppDeploy->uninstall(type);
            if (uninstallResult != KM_ERROR_NO)
            {
                kmlogger.error("Uninstall failed for package: %s", packageId.c_str());
                // clearCacheFiles(packageId);
                return uninstallResult;
            }
        }

        clearCacheFiles(packageId);
    }

    return KM_ERROR_NO;
}

void KMPackageManager::removeHardlinksInPlace(const fs::path &dir)
{
    try
    {
        std::unordered_set<ino_t> processed_inodes;

        for (const auto &entry : fs::recursive_directory_iterator(dir))
        {
            if (fs::is_regular_file(entry))
            {
                // 正确获取 inode 的方式
                struct stat file_stat;
                if (stat(entry.path().c_str(), &file_stat) == 0)
                {
                    ino_t inode = file_stat.st_ino;

                    if (processed_inodes.find(inode) == processed_inodes.end())
                    {
                        processed_inodes.insert(inode);
                    }
                    else
                    {
                        // 处理硬链接：创建临时副本
                        fs::path temp_file = entry.path().string() + ".tmp";

                        // 复制内容
                        std::ifstream src(entry.path(), std::ios::binary);
                        std::ofstream dst(temp_file, std::ios::binary);
                        dst << src.rdbuf();
                        src.close();
                        dst.close();

                        // 替换原文件
                        fs::rename(temp_file, entry.path());
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void KMPackageManager::processServiceFiles(const fs::path &sourceDir, const std::string &destDir, KMPMRef &downloadRef, std::set<std::string> &installedFiles)
{
    if (!fs::exists(sourceDir))
        return;

    std::vector<std::string> files;
    if (!getAllFilesInDirectory(sourceDir, files))
        return;

    auto serviceNames = getServiceName(files);
    for (const auto &serviceName : serviceNames)
    {
        size_t lastSlashPos = serviceName.find_last_of('/');
        if (lastSlashPos != std::string::npos)
        {
            // 检查是否为无效软链接
            if (fs::is_symlink(serviceName) && !fs::exists(serviceName))
            {
                continue; // 跳过无效软链接
            }
            std::string filename = serviceName.substr(lastSlashPos + 1);
            std::string destPath = destDir + filename;
            modifyAppServiceFile(downloadRef, serviceName, destPath);
            installedFiles.insert(destPath);
        }
    }
}

void KMPackageManager::startTargetWantsServices(const fs::path &sourceDir)
{
    // check and start services
    const fs::path serviceDir = sourceDir / "entries" / "etc" / "systemd" / "system" / "multi-user.target.wants";
    if (fs::exists(serviceDir))
    {
        for (const auto &entry : fs::directory_iterator(serviceDir))
        {
            if (entry.path().extension() == ".service")
            {
                int reloadResult = system("systemctl daemon-reload");
                if (reloadResult == 0)
                {
                    std::string serviceName = entry.path().stem().string();
                    std::string cmd = "systemctl enable --now " + serviceName;
                    int result = system(cmd.c_str());
                    if (result == 0)
                    {
                        kmlogger.info("Service %s started successfully", serviceName.c_str());
                    }
                    else
                    {
                        kmlogger.error("Failed to start service %s", serviceName.c_str());
                    }
                }
                else
                {
                    kmlogger.error("Failed to reload systemd daemon, exit code: %s", std::to_string(reloadResult).c_str());
                }
            }
        }
    }
}

void KMPackageManager::modifyBaseDir(std::string baseDir)
{
    // 需要检查的文件/目录列表
    const std::vector<std::string> relPaths = {
        "/files/usr/bin/sudo",
        "/files/etc/sudoers",
        "/files/usr/lib/sudo",
        "/files/usr/libexec/sudo"};

    for (const auto &relPath : relPaths)
    {
        struct stat fileStat;
        fs::path target = fs::path(baseDir + relPath);
        // 检查文件/目录是否存在
        if (stat(target.c_str(), &fileStat) == 0)
        {
            // 修改所有者（需要root权限）
            if (chown(target.c_str(), 0, 0) == 0)
            {
                kmlogger.info("Successfully changed owner to root :%s", target.c_str());
            }
        }
    }
}

bool KMPackageManager::copyDirectory(const fs::path &source, const fs::path &destination, vector<string> &ignoreExtension)
{
    try
    {
        if (!fs::exists(source))
        {
            kmlogger.error("源目录不存在: %s", source.c_str());
            return false;
        }

        if (!fs::is_directory(source))
        {
            kmlogger.error("源路径不是目录: %s", source.c_str());
            return false;
        }

        // 处理目标目录为空的情况
        fs::path actualDestination = destination;
        if (destination.empty())
        {
            // 获取源路径的最后一级目录名，并放在根目录下
            string lastDir = source.filename().string();
            actualDestination = "/" + lastDir;
            kmlogger.info("目标目录为空，使用: %s", actualDestination.c_str());
        }

        // 创建目标目录
        fs::create_directories(actualDestination);

        // 使用 directory_iterator 而不是 recursive_directory_iterator
        // 手动处理递归，以便更好地控制错误
        for (const auto &entry : fs::directory_iterator(source))
        {
            fs::path targetPath = actualDestination / entry.path().filename();

            if (fs::is_directory(entry.status()))
            {
                // 递归复制子目录
                if (!copyDirectory(entry.path(), targetPath, ignoreExtension))
                {
                    return false;
                }
            }
            else if (fs::is_regular_file(entry.status()))
            {
                // 检查文件扩展名是否在忽略列表中
                string filename = entry.path().filename().string();
                bool shouldIgnore = false;

                for (const auto &ext : ignoreExtension)
                {
                    if (filename.length() >= ext.length() &&
                        filename.compare(filename.length() - ext.length(), ext.length(), ext) == 0)
                    {
                        shouldIgnore = true;
                        break;
                    }
                }

                // 如果文件扩展名在忽略列表中，跳过复制
                if (shouldIgnore)
                {
                    continue;
                }

                try
                {
                    fs::copy_file(entry.path(), targetPath, fs::copy_options::overwrite_existing);
                    kmlogger.info("复制文件: %s -> %s", entry.path().c_str(), targetPath.c_str());
                }
                catch (const fs::filesystem_error &e)
                {
                    kmlogger.error("复制文件失败: %s", e.what());
                    return false;
                }
            }
            // 可以选择是否处理符号链接等其他类型文件
        }
        return true;
    }
    catch (const fs::filesystem_error &e)
    {
        kmlogger.error("复制目录失败: %s", e.what());
        return false;
    }
}

std::string KMPackageManager::processDeleteLine(const std::string &line, KMRef &ref)
{
    std::string trimmedLine = line;
    while (!trimmedLine.empty() && (trimmedLine.back() == '\n' || trimmedLine.back() == '\r'))
    {
        trimmedLine.pop_back();
    }

    if (trimmedLine.empty())
    {
        return trimmedLine;
    }

    size_t slashCount = 0;
    size_t thirdSlashPos = std::string::npos;

    for (size_t i = 0; i < trimmedLine.length(); ++i)
    {
        if (trimmedLine[i] == '/')
        {
            slashCount++;
            if (slashCount == 3)
            {
                thirdSlashPos = i;
                break;
            }
        }
    }

    std::stringstream ss;
    std::string last = trimmedLine.substr(thirdSlashPos);
    if (ref.isDeltas)
    {
        if (last.find("/files/") == 0)
        {
            ss << last.substr(6);
        }
    }
    else
    {

        ss << "/opt/kaiming/layers/"
           << ref.channel << "/" << ref.arch << "/" << ref.kind << "/" << ref.id << "/" << ref.module << "/" << ref.findPreVersion;

        if (thirdSlashPos != std::string::npos && thirdSlashPos + 1 < trimmedLine.length())
        {
            ss << trimmedLine.substr(thirdSlashPos);
        }
    }

    return ss.str();
}

bool KMPackageManager::processDeleteFile(const std::string &filename, KMRef &ref)
{
    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << " for reading." << std::endl;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(inputFile, line))
    {
        if (!line.empty())
        {
            std::string processedLine = processDeleteLine(line, ref);
            if (fs::exists(processedLine) && fs::is_regular_file(processedLine))
            {
                std::remove(processedLine.c_str());
            }
        }
    }

    inputFile.close();

    return true;
}

bool KMPackageManager::upgradeDirectory(const std::string &newDir, const std::string &oldDir)
{
    // 确保目录路径格式正确
    std::string newBase = newDir;
    std::string oldBase = oldDir;

    if (newBase.back() != '/')
        newBase += "/";
    if (oldBase.back() != '/')
        oldBase += "/";

    // 遍历新目录中的所有文件
    for (const auto &entry : fs::recursive_directory_iterator(newBase))
    {
        if (entry.is_regular_file())
        {
            std::string currentFile = entry.path().string();
            std::string relativePath = currentFile.substr(newBase.length());

            if (entry.path().extension() == ".xdelta")
            {
                // 处理 .xdelta 文件
                std::string targetFile = relativePath.substr(0, relativePath.find(".xdelta"));

                // 构建旧文件路径
                std::string oldFile = oldBase + targetFile;

                // 构建新文件路径（与.xdelta文件在同一目录）
                std::string newFile = newBase + targetFile;

                // 确保目标目录存在
                std::string newFileDir = fs::path(newFile).parent_path().string();
                if (!fs::exists(newFileDir))
                {
                    fs::create_directories(newFileDir);
                }
                // 获取原文件权限
                struct stat oldFileStat;
                mode_t oldPermissions = 0644; // 默认权限
                if (stat(oldFile.c_str(), &oldFileStat) == 0)
                {
                    oldPermissions = oldFileStat.st_mode;
                }

                // 执行xdelta3命令
                std::string command = std::string(KAIMING_DELTAS) + " -d -f -s \"" + oldFile + "\" \"" + currentFile + "\" \"" + newFile + "\"";
                int result = std::system(command.c_str());

                if (result == 0)
                {
                    // 设置新文件权限与原文件相同
                    chmod(newFile.c_str(), oldPermissions);
                    // 删除差量文件
                    std::remove(currentFile.c_str());
                }
                else
                {
                    std::cout << "✗ 失败: " << command << " (错误码: " << result << ")" << std::endl;
                    return false;
                }
            }
        }
    }

    std::cout << "升级完成!" << std::endl;
    return true;
}

void KMPackageManager::refreshRemoteConfiguration()
{
    if (d->m_ostreeHandler)
    {
        d->m_ostreeHandler->refreshRemoteUrl();
    }

    refreshSummaryCache();
}

int KMPackageManager::refreshSummaryCache()
{
    if (d->m_parseSummaryJson)
    {
        d->m_parseSummaryJson->clearCache();
    }

    int getSummaryState = downloadAllRemoteSummaries(true);
    if (getSummaryState != KM_ERROR_NO)
    {
        kmlogger.error("Get and parse summary.json file error, err = %d", getSummaryState);
        return getSummaryState;
    }

    return KM_ERROR_NO;
}

bool KMPackageManager::createDbusWhitelistSymlinks(const fs::path& versionDataPath, const std::vector<std::string>& vecDbusWhitelist, const std::string& targetBaseDir) {
    if (vecDbusWhitelist.empty()) {
        kmlogger.debug("DBus whitelist is empty, no symlinks to create");
        return true;
    }

    fs::path targetDir = targetBaseDir.empty() ? "/etc/kylin-config/basic" : targetBaseDir;

    std::error_code ec;
    if (!fs::exists(targetDir, ec)) {
        if (!fs::create_directories(targetDir, ec)) {
            kmlogger.error("Failed to create directory %s: %s", targetDir.c_str(), ec.message().c_str());
            return false;
        }
        kmlogger.info("Created directory: %s", targetDir.c_str());
    }
    
    bool allSuccess = true;
    int createdCount = 0;
    int failedCount = 0;

    std::unordered_set<std::string> uniquePaths;
    for (const auto& executablePath : vecDbusWhitelist) {
        if (!executablePath.empty()) {
            uniquePaths.insert(executablePath);
        }
    }

    // 为每个可执行程序创建软链接
    for (const auto& relativePath : uniquePaths) {
        std::string processedPath = relativePath;
        size_t filesPos = processedPath.find("/files/");
         if (filesPos == std::string::npos) {
                kmlogger.error("Invalid DBus whitelist path format (no 'files' directory): %s", relativePath.c_str());
                failedCount++;
                allSuccess = false;
                continue;

        } else {
            processedPath = processedPath.substr(filesPos + 7);
        }

        fs::path sourcePath = versionDataPath / processedPath;

        // 检查源可执行文件是否存在且是普通文件
        if (!fs::exists(sourcePath, ec)) {
            kmlogger.error("Executable not found: %s", sourcePath.c_str());
            failedCount++;
            allSuccess = false;
            continue;
        }

        if (!fs::is_regular_file(sourcePath, ec)) {
            kmlogger.error("Path is not a regular file: %s", sourcePath.c_str());
            failedCount++;
            allSuccess = false;
            continue;
        }

        // 生成目标软链接文件名
        fs::path symlinkName = sourcePath.filename();
        fs::path symlinkPath = targetDir / symlinkName;

         if (fs::exists(symlinkPath, ec)) {
            try {
                fs::remove(symlinkPath);
                kmlogger.debug("Removed existing file/link: %s", symlinkPath.c_str());
            } catch (const fs::filesystem_error& e) {
                kmlogger.error("Failed to remove existing file %s: %s", symlinkPath.c_str(), e.what());
                failedCount++;
                allSuccess = false;
                continue;
            }
        }

        // 创建软链接
        try {
            fs::create_symlink(sourcePath, symlinkPath);
            kmlogger.info("Created symlink: %s -> %s", symlinkPath.c_str(), sourcePath.c_str());
            d->m_installedFiles.insert(symlinkPath.string());
            createdCount++;
        } catch (const fs::filesystem_error& e) {
            kmlogger.error("Failed to create symlink %s -> %s: %s", symlinkPath.c_str(), sourcePath.c_str(), e.what());
            failedCount++;
            allSuccess = false;
        }
    }

    kmlogger.info("DBus whitelist symlinks summary: Total %zu executables, Created: %d, Failed: %d", uniquePaths.size(), createdCount, failedCount);

    return allSuccess;
}

void KMPackageManager::resetInstallState()
{
    // 网络状态相关
    networkDisconnectedTime = 0;
    isNetworkDisconnected = false;
    networkFailCount = 0;

    // 下载进度相关
    refDownloadListForProgress.clear();
    // globalProgressStatus = {"", 0, "", 5};  // 恢复你的默认值

    progressJson.clear();
    lastOutput.clear();
    preRemaining.clear();

    currentPercent = 0;
    lastWidth = 0;
    lastProgressStr.clear();
    lastStaticInfo.clear();
    staticInfoDisplayed = false;

    installing = false;
}

// ============================================================================
// 并发安装 Worker 线程实现
// ============================================================================

void KMPackageManager::Private::ensureWorkersStarted()
{
    // 双重检查锁定模式，确保线程只启动一次
    if (m_workersStarted.load(std::memory_order_acquire)) {
        return;  // 线程已启动，直接返回
    }
    
    std::lock_guard<std::mutex> lock(m_workersStartMutex);
    
    // 再次检查（防止多线程竞争）
    if (m_workersStarted.load(std::memory_order_relaxed)) {
        return;
    }
    
    // 启动下载 worker 线程池
    kmlogger.info("Starting download worker threads (lazy initialization)");
    for (size_t i = 0; i < MAX_DOWNLOAD_WORKERS; ++i) {
        m_downloadWorkers.emplace_back(&KMPackageManager::Private::downloadWorkerThread, this);
    }
    
    // 启动写入线程（单线程，串行）
    m_writerThread = std::thread(&KMPackageManager::Private::writerThread, this);
    
    // 设置标志位（使用 release 语义，确保之前的操作对所有线程可见）
    m_workersStarted.store(true, std::memory_order_release);
    kmlogger.info("Worker threads started successfully");
}

void KMPackageManager::Private::downloadWorkerThread()
{
    kmlogger.info("Download worker thread started");
    
    /** 
     *  调用 ostree_repo_pull_with_options 的线程，必须运行 GLib MainLoop
     *  为该 worker 线程创建独立的 GLib MainContext
    **/
    GMainContext *thread_ctx = g_main_context_new();

    // 设为该线程的 default context
    g_main_context_push_thread_default(thread_ctx);

    // 创建 mainloop（不自动运行）
    GMainLoop *thread_loop = g_main_loop_new(thread_ctx, FALSE);

    while (!m_shutdown.load()) {
        std::string taskId;
        
        // 从下载队列获取任务
        {
            std::unique_lock<std::mutex> lock(m_tasksMutex);
            m_downloadCondition.wait(lock, [this] {
                return !m_downloadQueue.empty() || m_shutdown.load();
            });
            
            if (m_shutdown.load()) {
                break;
            }
            
            if (m_downloadQueue.empty()) {
                continue;
            }
            
            taskId = m_downloadQueue.front();
            m_downloadQueue.pop();
        }
        
        // 获取任务上下文
        std::shared_ptr<DownloadTaskContext> context;
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            auto it = m_activeTasks.find(taskId);
            if (it == m_activeTasks.end()) {
                kmlogger.warn("Task %s not found in active tasks", taskId.c_str());
                continue;
            }
            context = it->second;
        }
        
        // 检查是否已取消
        if (context->cancelRequested.load()) {
            kmlogger.info("Task %s was cancelled before download", taskId.c_str());

            context->installResult = KM_ERROR_USER_CANCEL;
            std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_USER_CANCEL);
            if (context->completionCallback) {
                try {  
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_USER_CANCEL, errorInfo);
                } catch (...) {
                    kmlogger.error("Task %s: Exception in completion callback during cancel", taskId.c_str());
                }
            }

            // 从活动任务中移除
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(taskId);
            continue;
        }

        kmlogger.info("Worker thread processing task %s", taskId.c_str());

        // 创建临时 repo 路径
        std::string tmpRepoPath = "/var/tmp/kaiming/job-" + taskId;
        std::shared_ptr<KMOSTreeHandler> tmpHandler;
        std::string mainRepo = context->packageManager->d->m_folder->getRepoPath();

        try {
            // 如果临时目录已存在，先删除（可能是之前失败留下的）
            if (fs::exists(tmpRepoPath)) {
                fs::remove_all(tmpRepoPath);
            }
            fs::create_directories(tmpRepoPath);

            // 创建临时 ostree repo
            g_autoptr(GFile) pathFile = g_file_new_for_path(tmpRepoPath.c_str());
            g_autoptr(OstreeRepo) tmpRepo = ostree_repo_new(pathFile);
            g_autoptr(GError) error = nullptr;

            OstreeRepoMode mode = OSTREE_REPO_MODE_BARE_USER_ONLY;
            if (!ostree_repo_create(tmpRepo, mode, nullptr, &error)) {
                kmlogger.error("Failed to create temp ostree repo: %s", error ? error->message : "unknown");
                throw std::runtime_error("Failed to create temp ostree repo");
            }
            kmlogger.debug("Created temp ostree repo at %s", tmpRepoPath.c_str());

            if (!ostree_repo_open(tmpRepo, nullptr, &error)) {
                kmlogger.error("Failed to open newly created temp repo: %s", error ? error->message : "unknown");
                throw std::runtime_error("Failed to open newly created temp repo");
            }

            // 复制主 repo 的 config 文件到临时 repo
            std::string mainRepoConfig = mainRepo + "/config";
            std::string tmpRepoConfig = tmpRepoPath + "/config";

            if (fs::exists(mainRepoConfig)) {
                fs::copy_file(mainRepoConfig, tmpRepoConfig, fs::copy_options::overwrite_existing);
                kmlogger.debug("Copied main repo config to temp repo: %s", tmpRepoConfig.c_str());
            }

            tmpHandler = std::make_shared<KMOSTreeHandler>(tmpRepoPath);

            // 保存 tmpHandler 到 context，以便取消操作
            context->tmpHandler = tmpHandler;
        } catch (const std::exception& e) {
            kmlogger.error("Failed to setup temp repo %s: %s", tmpRepoPath.c_str(), e.what());

            try {
                fs::remove_all(tmpRepoPath);
            } catch (...) {
            }
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(taskId);
            continue;
        }

        if (!tmpHandler) {
            kmlogger.error("Failed to create temp repo handler for task %s", taskId.c_str());
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(taskId);
            continue;
        }

        // 每个任务有独立的进度回调，会调用该任务的 progressCallback
        auto progressCb = [context](ProgressStatus progressStatus, gpointer userData) {
            KMPackageManager::progressCallbackWithContext(progressStatus, context);
        };

        // 执行下载到临时 repo
        bool downloadSuccess = false;

        try {
            // 如果是从暂停恢复，跳过已完成的 remotes
            // 注意：恢复时 taskState 会被设置为 RUNNING，所以通过检查 completedRemotes 来判断
            bool isResuming = !context->completedRemotes.empty();

            for (const auto &[remote, refs] : context->repoRefMap) {
                // 检查任务状态（暂停或取消都需要退出循环）
                TaskState currentState = context->taskState.load();
                if (currentState == TaskState::CANCELLED || currentState == TaskState::PAUSED || context->cancelRequested.load()) {
                    if (currentState == TaskState::PAUSED) {
                        kmlogger.info("Task %s paused during download", taskId.c_str());
                    } else {
                        kmlogger.info("Task %s cancelled during download", taskId.c_str());
                    }
                    break;
                }

                // 如果是从暂停恢复，检查该 remote 的已下载 refs，只下载未完成的 refs
                std::vector<std::string> refsToDownload = refs;
                if (isResuming && context->completedRemotes.find(remote) != context->completedRemotes.end()) {
                    const std::vector<std::string>& completedRefs = context->completedRemotes[remote];
                    refsToDownload.clear();

                    for (const auto& ref : refs) {
                        if (std::find(completedRefs.begin(), completedRefs.end(), ref) == completedRefs.end()) {
                            refsToDownload.push_back(ref);
                        } else {
                            kmlogger.debug("Task %s: Skipping already downloaded ref %s from remote %s", taskId.c_str(), ref.c_str(), remote.c_str());
                        }
                    }

                    if (refsToDownload.empty()) {
                        kmlogger.info("Task %s: Skipping completed remote %s (all %zu refs already downloaded)", taskId.c_str(), remote.c_str(), refs.size());
                        continue;
                    } else {
                        kmlogger.info("Task %s: Resuming remote %s, %zu/%zu refs already downloaded, downloading remaining %zu refs",
                                     taskId.c_str(), remote.c_str(), completedRefs.size(), refs.size(), refsToDownload.size());
                    }
                }

                kmlogger.info("Pulling %zu refs from remote: %s", refsToDownload.size(), remote.c_str());
                bool result = false;
                try {
                    result = tmpHandler->pull(remote, refsToDownload, progressCb, nullptr, context->pmOptions.maxSpeedLimitKilobytes, { mainRepo });
                } catch (const std::exception& e) {
                    kmlogger.error("Exception in pull for task %s: %s", taskId.c_str(), e.what());
                    result = false;
                } catch (...) {
                    kmlogger.error("Unknown exception in pull for task %s", taskId.c_str());
                    result = false;
                }

                if (result) {
                    downloadSuccess = true;

                    // 记录已完成的 refs（合并到已下载列表中）
                    if (context->completedRemotes.find(remote) == context->completedRemotes.end()) {
                        // 这个 remote 还没有记录，直接记录所有已下载的 refs
                        context->completedRemotes[remote] = refsToDownload;
                    } else {
                        // 合并新下载的 refs 到已下载列表（去重）
                        std::vector<std::string>& completedRefs = context->completedRemotes[remote];
                        for (const auto& ref : refsToDownload) {
                            if (std::find(completedRefs.begin(), completedRefs.end(), ref) == completedRefs.end()) {
                                completedRefs.push_back(ref);
                            }
                        }
                    }
                    kmlogger.info("Task %s: Completed %zu refs from remote %s, total completed refs: %zu", 
                                 taskId.c_str(), refsToDownload.size(), remote.c_str(), context->completedRemotes[remote].size());
                } else {
                    if (context->taskState.load() == TaskState::PAUSED)
                    {
                        kmlogger.info("Task %s paused during pull of remote %s", taskId.c_str(), remote.c_str());
                        break;
                    }

                    kmlogger.error("Pull failed for remote: %s", remote.c_str());
                    downloadSuccess = false;
                    break;
                }
            }
        } catch (const std::exception& e) {
            kmlogger.error("Exception during download for task %s: %s", taskId.c_str(), e.what());
            downloadSuccess = false;
        } catch (...) {
            kmlogger.error("Unknown exception during download for task %s", taskId.c_str());
            downloadSuccess = false;
        }

        TaskState finalTaskState = context->taskState.load();

        // 只有在明确是 CANCELLED 状态或 cancelRequested 为 true 时才认为是取消
        if (finalTaskState == TaskState::CANCELLED || (context->cancelRequested.load() && finalTaskState != TaskState::PAUSED)) {
            // 任务被取消，需要清理资源
            kmlogger.info("Task %s cancelled, cleaning up", taskId.c_str());

            int errorCode = KM_ERROR_USER_CANCEL;
            context->installResult = errorCode;

            if (context->completionCallback) {
                try {
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, errorCode, KMErrorMsg::getErrorMessage(errorCode));
                } catch (...) {
                    kmlogger.error("Task %s: Exception in completion callback during cancel", taskId.c_str());
                }
            }

            fs::remove_all(tmpRepoPath);

            // 从活动任务中移除
            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
            }
        } else if (downloadSuccess && !context->cancelRequested.load() && finalTaskState != TaskState::PAUSED) {
            // 下载成功，加入写入队列
            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                context->tmpRepoPath = tmpRepoPath;
                m_writeQueue.push(taskId);
            }

            m_writeCondition.notify_one();
            kmlogger.info("Task %s download completed, queued for write", taskId.c_str());
        } else if (finalTaskState == TaskState::PAUSED) {
            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                context->tmpRepoPath = tmpRepoPath;
            }
            kmlogger.info("Task %s paused, preserving temp repo and task context for resume", taskId.c_str());
            context->installResult = KM_ERROR_USER_PAUSED;

            // 调用完成回调，通知调用者任务已暂停
            if (context->completionCallback)
            {
                try
                {
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_USER_PAUSED, KMErrorMsg::getErrorMessage(KM_ERROR_USER_PAUSED));
                }
                catch (...)
                {
                    kmlogger.error("Task %s: Exception in completion callback during pause", taskId.c_str());
                }
            }

        } else {
            kmlogger.error("Task %s download failed or cancelled", taskId.c_str());

            int errorCode;
            if (context->cancelRequested.load()) {
                errorCode = KM_ERROR_USER_CANCEL;
            } else {
                errorCode = KM_ERROR_PULL_FAILED;
            }
            context->installResult = errorCode;

            if (context->completionCallback) {
                try {
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, errorCode, KMErrorMsg::getErrorMessage(errorCode));
                } catch (...) {
                    kmlogger.error("Task %s: Exception in completion callback during download failure", taskId.c_str());
                }
            }

            fs::remove_all(tmpRepoPath);

            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(taskId);
        }
    }

    // pull 返回后，确保所有 pending source 被处理完
    while (g_main_context_pending(thread_ctx)) {
        g_main_context_iteration(thread_ctx, FALSE);
    }

    // 清理
    g_main_loop_unref(thread_loop);
    g_main_context_pop_thread_default(thread_ctx);
    g_main_context_unref(thread_ctx);

    kmlogger.info("Download worker thread exiting");
}

void KMPackageManager::Private::writerThread()
{
    kmlogger.info("Writer thread started");

    while (!m_shutdown.load()) {
        std::string taskId;

        // 从写入队列获取任务
        {
            std::unique_lock<std::mutex> lock(m_tasksMutex);
            m_writeCondition.wait(lock, [this] {
                return !m_writeQueue.empty() || m_shutdown.load();
            });

            if (m_shutdown.load()) {
                break;
            }

            if (m_writeQueue.empty()) {
                continue;
            }

            taskId = m_writeQueue.front();
            m_writeQueue.pop();
        }

        // 获取任务上下文
        std::shared_ptr<DownloadTaskContext> context;
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            auto it = m_activeTasks.find(taskId);
            if (it == m_activeTasks.end()) {
                kmlogger.warn("Task %s not found in active tasks", taskId.c_str());
                continue;
            }
            context = it->second;
        }

        // 检查是否已取消
        if (context->cancelRequested.load()) {
            kmlogger.info("Task %s was cancelled before write", taskId.c_str());

            context->installResult = KM_ERROR_USER_CANCEL;
            std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_USER_CANCEL);

            if (context->completionCallback) {
                try {
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_USER_CANCEL, errorInfo);
                } catch (...) {
                    kmlogger.error("Task %s: Exception in completion callback during cancel in writer", taskId.c_str());
                }
            }

            if (!context->tmpRepoPath.empty()) {
                fs::remove_all(context->tmpRepoPath);
            }

            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(taskId);
            continue;
        }

        kmlogger.info("Writer thread processing task %s", taskId.c_str());

        try {
            std::vector<std::string> refsToMerge;
            for (const auto& ref : context->refDownloadList) {
                refsToMerge.push_back(ref.completedRef);
            }

            bool mergeSuccess = false;
            if (!refsToMerge.empty() && !context->tmpRepoPath.empty()) {
                mergeSuccess = m_ostreeHandler->pullLocal(context->tmpRepoPath, refsToMerge, nullptr, nullptr, &context->repoRefMap);
            } else {
                kmlogger.warn("Task %s: No refs to merge or tmp repo path is empty", taskId.c_str());
                mergeSuccess = false;
            }

            if (!mergeSuccess) {
                kmlogger.error("Task %s: Failed to merge temp repo to main repo", taskId.c_str());

                context->installResult = KM_ERROR_INSTALL_FAILED;
                std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_INSTALL_FAILED);

                if (context->completionCallback) {
                    try {
                        context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_INSTALL_FAILED, errorInfo);
                    } catch (...) {
                        kmlogger.error("Task %s: Exception in completion callback during merge failure", taskId.c_str());
                    }
                }

                if (!context->tmpRepoPath.empty()) {
                    fs::remove_all(context->tmpRepoPath);
                }

                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
                continue;
            }
            
            // 仅下载模式：跳过部署相关操作
            if (context->pmOptions.isDownloadOnly)
            {
                kmlogger.info("Task %s: Download only mode, skipping deployment", taskId.c_str());
                std::string errorInfo = KMErrorMsg::getErrorMessage(KM_ERROR_NO);
                // 调用完成回调（下载完成）
                if (context->completionCallback)
                {
                    try
                    {
                        context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_NO, errorInfo);
                    }
                    catch (const std::exception &e)
                    {
                        kmlogger.error("Task %s: Exception in completion callback: %s", taskId.c_str(), e.what());
                    }
                    catch (...)
                    {
                        kmlogger.error("Task %s: Unknown exception in completion callback", taskId.c_str());
                    }
                }

                // 清理临时目录
                if (!context->tmpRepoPath.empty())
                {
                    fs::remove_all(context->tmpRepoPath);
                }

                // 从活动任务中移除
                {
                    std::lock_guard<std::mutex> lock(m_tasksMutex);
                    m_activeTasks.erase(taskId);
                }

                kmlogger.info("Task %s download only completed", taskId.c_str());
                continue;
            }

            (void)m_packageManager->cleanupOldRefsWithRuntimeGuard(m_pmOldAVersion, m_pmIsForceInstall);

            // 部署包（验签、checkout、创建符号链接等）
            int deployResult = deployTask(context);
            context->installResult = deployResult;

            std::string errorInfo = KMErrorMsg::getErrorMessage(deployResult);

            if (context->completionCallback) {
                try {
                    // kmlogger.info("Task %s: Calling completion callback with result: %d, error: %s", taskId.c_str(), deployResult, errorInfo.c_str());
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, deployResult, errorInfo);
                } catch (const std::exception& e) {
                    kmlogger.error("Task %s: Exception in completion callback: %s", taskId.c_str(), e.what());
                } catch (...) {
                    kmlogger.error("Task %s: Unknown exception in completion callback", taskId.c_str());
                }
            } else {
                kmlogger.debug("Task %s: No completion callback set, skipping finishsignal", taskId.c_str());
            }

            if (deployResult != KM_ERROR_NO) {
                kmlogger.error("Task %s: Deploy failed with error code: %d", taskId.c_str(), deployResult);

                if (!context->tmpRepoPath.empty()) {
                    fs::remove_all(context->tmpRepoPath);
                }

                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
                continue;
            }

            if (!context->tmpRepoPath.empty()) {
                fs::remove_all(context->tmpRepoPath);
            }

            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
            }

            kmlogger.info("Task %s write completed", taskId.c_str());
        } catch (const std::exception& e) {
            kmlogger.error("Task %s: Exception in writer thread: %s", taskId.c_str(), e.what());

            context->installResult = KM_ERROR_INSTALL_FAILED;
            std::string errorInfo = std::string("Exception in writer thread: ") + e.what();

            if (context->completionCallback) {
                try {
                    context->completionCallback(context->mainAppName, KM_INSTALL_OPERATION, KM_ERROR_INSTALL_FAILED, errorInfo);
                } catch (...) {
                    kmlogger.error("Task %s: Exception in completion callback during exception handling", taskId.c_str());
                }
            }

            if (!context->tmpRepoPath.empty()) {
                fs::remove_all(context->tmpRepoPath);
            }

            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
            }
        } catch (...) {
            kmlogger.error("Task %s: Unknown exception in writer thread", taskId.c_str());

            if (!context->tmpRepoPath.empty()) {
                fs::remove_all(context->tmpRepoPath);
            }

            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks.erase(taskId);
            }
        }
    }

    kmlogger.info("Writer thread exiting");
}

int KMPackageManager::Private::deployTask(std::shared_ptr<DownloadTaskContext> context)
{
    fs::path basePath = m_folder->getPath();
    fs::path layersPath;
    bool kareInstall = true;

    context->totalRefCount = context->refDownloadList.size();
    context->currentRefIndex = 0;

    kmlogger.info("Task %s: 部署开始", context->taskId.c_str());

    // 使用 context 中的 refDownloadList 而不是全局的 m_pmRefDownloadList
    for (auto &downloadRef : context->refDownloadList)
    {
        m_packageManager->emitTotalDeployProgressWithContext(0, context);
        // 使用 context 中的 installedFiles 而不是全局的 m_installedFiles
        context->installedFiles.clear();

        if (downloadRef.ref.module == "devel")
        {
            layersPath = fs::path(KMStorageDir::getUserBaseDirLocation());
        }
        else
        {
            layersPath = basePath / "layers";
        }

        m_packageManager->emitTotalDeployProgressWithContext(5, context);
        kmlogger.info("Task %s: 正在部署 completedRef: %s", context->taskId.c_str(), downloadRef.completedRef.c_str());
        fs::path idPath = layersPath / downloadRef.ref.channel / downloadRef.ref.arch / downloadRef.ref.kind / downloadRef.ref.id;

        fs::path versionDataPath = layersPath / downloadRef.completedRef;
        kmlogger.debug("versionDataPath:  %s", versionDataPath.c_str());

        if (downloadRef.ref.kind == "app")
        {
            context->installedFiles.insert(idPath.string());
        }

        context->installedFiles.insert(versionDataPath.string());

        if (std::filesystem::exists(versionDataPath) || std::filesystem::create_directories(versionDataPath))
        {
            kmlogger.debug("Directory created or already exists successfully!");
        }

        kmlogger.debug("Begin to checkout ...");
        string latestCommit = m_ostreeHandler->readLatestCommit(downloadRef.completedRef);
        bool isCheckout = m_ostreeHandler->checkout(latestCommit, versionDataPath.string());

        m_packageManager->emitTotalDeployProgressWithContext(10, context);
  
        std::string removeStr;
        if (!isCheckout)
        {
            if (downloadRef.ref.module == "binary")
            {
                fs::path rootPath = m_folder->getLayersPath();
                removeStr = rootPath.string() + "/" + downloadRef.completedRef;
            }

            if (downloadRef.ref.module == "devel")
            {
                fs::path develDir = fs::path(KMStorageDir::getUserBaseDirLocation());
                removeStr = develDir.string() + "/" + downloadRef.completedRef;
            }

            m_packageManager->deleteSpecificRefsFromRepo(removeStr);

            std::cout << "Checkout files failed" << std::endl;
            kmlogger.info("Checkout files failed");
            return KM_ERROR_INSTALL_FAILED;
        }
    
        kmlogger.debug("End to checkout.");
        kmlogger.info("baseType:  %s", downloadRef.ref.kind.c_str());
 
        m_packageManager->emitTotalDeployProgressWithContext(15, context);

        fs::path modulePath = idPath / downloadRef.ref.module;
        fs::path signPath = modulePath / downloadRef.ref.version;

        if (!context->pmOptions.skipSignatureVerification && context->pmOptions.isOnlineInstall && !context->pmOptions.isUpgradeType)
        {
            int verifyResult = KM_ERROR_NO;
            // 验签
            verifyResult = m_packageManager->verifySignature(downloadRef.ref.id, downloadRef.ref.kind, signPath);
            if (verifyResult != KM_ERROR_NO)
            {
                std::error_code ec;
                bool hasOtherVersion = false;
                if (fs::exists(modulePath, ec))
                {
                    for (const auto &entry : fs::directory_iterator(modulePath, ec))
                    {
                        if (ec)
                        {
                            break;
                        }
                        if (!entry.is_directory())
                        {
                            continue;
                        }
                        // 发现除当前版本外的其他目录
                        if (entry.path() != signPath)
                        {
                            hasOtherVersion = true;
                            break;
                        }
                    }
                }
                if (hasOtherVersion)
                {
                    // module 下还有其他版本，只删当前版本目录
                    if (!m_packageManager->removeTmpDeployFiles(signPath))
                    {
                        kmlogger.info("删除临时目录失败");
                    }
                }
                else
                {
                    // module 下只有这一个版本，直接删整个 idPath
                    if (!m_packageManager->removeTmpDeployFiles(idPath))
                    {
                        kmlogger.info("删除临时目录失败");
                    }
                }

                return verifyResult;
            }
        }
        else if (context->pmOptions.skipSignatureVerification)
        {
            kmlogger.info("用户不进行验签，跳过验签步骤");
        }

        m_packageManager->emitTotalDeployProgressWithContext(30, context);

        if (downloadRef.ref.kind == "app")
        {
            KMInfoJson infoJson;
            fs::path infoPath = versionDataPath / "info.json";
            infoJson.loadFile(infoPath.string());

            downloadRef.ref.isSysapp = infoJson.annotations.sysapp;
            std::vector<std::string> plugins = infoJson.export_plugins;

            const bool isImmutableSystem = KMFileUtils::isImmutableSystem();
            const std::string userFilePath = isImmutableSystem ? "/opt/system/lib/systemd/user/" : "/usr/lib/systemd/user/";
            const std::string systemFilePath = isImmutableSystem ? "/opt/system/lib/systemd/system/" : "/usr/lib/systemd/system/";
            const std::string dbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/services" : "/usr/share/dbus-1/services";
            const std::string systemDbusFilePath = isImmutableSystem ? "/opt/system/resource/dbus-1/system-services" : "/usr/share/dbus-1/system-services";
            const std::string systemDbusConfPath = isImmutableSystem ? "/opt/system/conf/dbus-1/system.d" : "/etc/dbus-1/system.d";
            const std::string schemasFilePath = isImmutableSystem ? "/opt/system/resource/glib-2.0/schemas/" : "/usr/share/glib-2.0/schemas";
            const std::string ukuiNotificationFilePath = "/etc/ukui/ukui-notification/default-app/";
            const std::string etcFilePath = isImmutableSystem ? " /opt/system/conf/" : "/etc";
            kmlogger.info(isImmutableSystem ? "ImmutableSystem" : "Not ImmutableSystem");

            fs::path serviceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "services";
            if (!fs::exists(serviceFileDirectoryPath))
            {
                serviceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "services";
            }

            if (fs::exists(serviceFileDirectoryPath))
            {
                m_packageManager->modifyServiceFile(downloadRef, serviceFileDirectoryPath.string(), dbusFilePath, context->installedFiles);
            }

            fs::path systemServiceFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system-services";
            if (!fs::exists(systemServiceFileDirectoryPath))
            {
                systemServiceFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system-services";
            }

            if (fs::exists(systemServiceFileDirectoryPath))
            {
                m_packageManager->modifyServiceFile(downloadRef, systemServiceFileDirectoryPath.string(), systemDbusFilePath, context->installedFiles);
            }

            fs::path systemdFileDirectoryPath = versionDataPath / "entries" / "usr" / "share" / "dbus-1" / "system.d";
            if (!fs::exists(systemdFileDirectoryPath))
            {
                systemdFileDirectoryPath = versionDataPath / "entries" / "share" / "dbus-1" / "system.d";
            }

            if (fs::exists(systemdFileDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdFileDirectoryPath, systemDbusConfPath, ".conf", "service", context->installedFiles);
            }

            fs::path systemdConfDirectoryPath = versionDataPath / "entries" / "etc" / "dbus-1" / "system.d";

            if (fs::exists(systemdConfDirectoryPath))
            {
                KMFileUtils::copyFilesByExtension(systemdConfDirectoryPath, systemDbusConfPath, ".conf", "service", context->installedFiles);
            }

            fs::path schemasSourceDir = versionDataPath / "entries" / "share" / "glib-2.0" / "schemas";
            if (fs::exists(schemasSourceDir))
            {
                fs::create_directories(schemasFilePath);
                KMFileUtils::copyFilesByExtension(schemasSourceDir, schemasFilePath, ".xml", "schema", context->installedFiles);

                std::string schemaCommand;
                schemaCommand = "glib-compile-schemas " + schemasFilePath + " >/dev/null 2>&1";
                int ret = system(schemaCommand.c_str());
                if (ret != 0)
                {
                    kmlogger.error("glib-compile-schemas failed for %s, return code: %d", schemasFilePath.c_str(), ret);
                }
            }

            fs::path systemPath;
            if (fs::exists(versionDataPath / "entries" / "etc" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "etc" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "usr" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "usr" / "lib" / "systemd";
            }
            else if (fs::exists(versionDataPath / "entries" / "lib" / "systemd"))
            {
                systemPath = versionDataPath / "entries" / "lib" / "systemd";
            }

            auto buildSystemdPath = [](const fs::path &basePath, bool isSystem) -> fs::path
            {
                const char *subpath = isSystem ? "system" : "user";
                return basePath / subpath;
            };

            if (!systemPath.string().empty())
            {
                m_packageManager->processServiceFiles(buildSystemdPath(systemPath, true), systemFilePath, downloadRef, context->installedFiles);
                m_packageManager->processServiceFiles(buildSystemdPath(systemPath, false), userFilePath, downloadRef, context->installedFiles);
            }

            fs::path sourcePath = versionDataPath / "entries" / "etc";
            std::vector<std::string> excludePatterns = {"*.service", "*.conf", "ld.so.cache"};
            KMFileUtils::syncWithExclusions(sourcePath, etcFilePath, excludePatterns);

            fs::path binData;

            if (fs::exists(versionDataPath / "entries" / "usr"))
            {
                binData = versionDataPath / "entries" / "usr" / "bin";
            }
            else
            {
                binData = versionDataPath / "entries" / "bin";
            }

            kmlogger.debug("Check directory:  %s", binData.string().c_str());
            if (fs::exists(binData) || fs::create_directories(binData))
            {
                kmlogger.info("create diretory %s success", binData.string().c_str());
            }

            fs::path binDataRef = binData / downloadRef.ref.id;
            m_packageManager->createExportBinShellScript(downloadRef, binDataRef.string());

            auto ensureDirExists = [](const fs::path &path)
            {
                if (!fs::exists(path))
                {
                    fs::create_directory(path);
                }
            };

            ensureDirExists(m_folder->getExportsPath());
            ensureDirExists(m_folder->getExportsSharePath());
            ensureDirExists(m_folder->getExportsBinPath());

            m_packageManager->getAllAppFilesInDirectory(versionDataPath.string(), context->installedFiles, downloadRef.ref.id);

            const std::string targetPath = m_folder->getPath().string();
            m_packageManager->modifyExportsDirectory(downloadRef, (versionDataPath / "entries").string(), targetPath, context->installedFiles);
            kmlogger.debug("Exports files symlinked to /opt/kaiming/exports directory successfully.");

            if (fs::exists(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app"))
            {
                KMFileUtils::copyFilesByExtension(versionDataPath / "entries" / "etc" / "ukui" / "ukui-notification" / "default-app", ukuiNotificationFilePath, ".json", "ukui", context->installedFiles);
            }

            m_pmBinName.clear();

            const std::vector<std::string> triggerCachePaths = {
                "/opt/kaiming-tools/share/kaiming/shells/gtk-icon-cache.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/mime-database.trigger",
                "/opt/kaiming-tools/share/kaiming/shells/desktop-database.trigger"};

            m_packageManager->runMultiperTriggerUpdateCacheCommand(triggerCachePaths);

            KMPluginSynchronizer::syncPlugins(versionDataPath / "files", plugins, context->installedFiles);

            m_packageManager->createDbusWhitelistSymlinks(versionDataPath / "files", infoJson.annotations.kylin_dbus_whitelist);
        }

        m_packageManager->emitTotalDeployProgressWithContext(35, context);

        if (downloadRef.ref.kind != "app")
        {
            m_packageManager->getAllBaseFilesInDirectory(versionDataPath.string(), context->installedFiles);
        }

        fs::path deployDataFile = versionDataPath / "deploy";
        bool isDeploy = m_packageManager->createInstalledFile(deployDataFile.string(), false, context->installedFiles);
        if (!isDeploy)
        {
            std::cout << "Generate installed deploy file failed!" << std::endl;
        }

        m_packageManager->emitTotalDeployProgressWithContext(40, context);
  
        fs::path infoListFile = m_folder->getInfoPath() / (downloadRef.ref.id + ".list");
        context->installedFiles.insert(deployDataFile.string());
        m_packageManager->writeInstalledFilesToList(infoListFile.string(), context->installedFiles);

        const char *old_value = getenv("XDG_DATA_DIRS");
        std::string new_value = "/opt/system/resource";
        if (old_value)
        {
            new_value += ":";
            new_value += old_value;
        }
        else
        {
            new_value += ":/usr/local/share:/usr/share";
        }

        setenv("XDG_DATA_DIRS", new_value.c_str(), 1);

        int result = system("exec 3>&1 1>/dev/null 2>&1; update-mime-database /opt/system/resource/mime");
        if (result != 0)
        {
            kmlogger.error("update-mime-database failed with code %d", result);
        }
        else
        {
            kmlogger.info("MIME database updated successfully");
        }

        m_packageManager->emitTotalDeployProgressWithContext(50, context);

        m_packageManager->fillInstalledInfo(downloadRef);
        m_packageManager->writeInstalledInfoToList();
  
        m_packageManager->emitTotalDeployProgressWithContext(55, context);
 
        std::set<std::string> deployfiles = m_hostAppDeploy->getSynchronizedFiles();
        context->installedFiles.merge(deployfiles);
        m_packageManager->setKsafLabelAndExectl(context->installedFiles, downloadRef.ref.id, downloadRef.ref.kind, versionDataPath);

        m_packageManager->emitTotalDeployProgressWithContext(90, context);

        if (downloadRef.ref.module == "devel")
        {
            KMStorageDir::modifyDirOwnership(KMStorageDir::getUserDevelDataDirLocation());
        }

        // 创建 deploy ref
        m_ostreeHandler->updateDeployRef(downloadRef.completedRef, latestCommit);

        // start services
        m_packageManager->startTargetWantsServices(versionDataPath);

        m_packageManager->emitTotalDeployProgressWithContext(100, context);

        context->currentRefIndex++;
    }

    kmlogger.info("Task %s: 部署成功", context->taskId.c_str());

    return KM_ERROR_NO;
}

std::string KMPackageManager::normalizeRefForCompare(std::string ref)
{
    KMOSTreeHandler::replaceRef(ref);
    return ref;
}

void KMPackageManager::reorderRefListByRepoMap(const std::map<std::string, std::vector<std::string>> &repoRefMap, std::vector<KMPMRef> &refList)
{
    if (repoRefMap.empty() || refList.empty())
    {
        return;
    }

    std::unordered_map<std::string, std::size_t> pullOrderIndex;
    std::size_t order = 0;
    for (const auto &[remote, refs] : repoRefMap)
    {
        (void)remote;
        for (const auto &ref : refs)
        {
            const std::string normalizedRef = normalizeRefForCompare(ref);
            if (pullOrderIndex.find(normalizedRef) == pullOrderIndex.end())
            {
                pullOrderIndex[normalizedRef] = order++;
            }
        }
    }

    std::stable_sort(refList.begin(), refList.end(),
                     [&pullOrderIndex](const KMPMRef &lhs, const KMPMRef &rhs)
                     {
                         const std::string lhsRef = normalizeRefForCompare(lhs.completedRef);
                         const std::string rhsRef = normalizeRefForCompare(rhs.completedRef);
                         const auto lhsIt = pullOrderIndex.find(lhsRef);
                         const auto rhsIt = pullOrderIndex.find(rhsRef);

                         const bool lhsKnown = lhsIt != pullOrderIndex.end();
                         const bool rhsKnown = rhsIt != pullOrderIndex.end();
                         if (lhsKnown != rhsKnown)
                         {
                             return lhsKnown;
                         }
                         if (!lhsKnown && !rhsKnown)
                         {
                             return false;
                         }
                         return lhsIt->second < rhsIt->second;
                     });
}

bool KMPackageManager::hasRunningContainers()
{
    KMPSInfoList info;
    KMCommonUtils::getPsInfo(info);
    return !info.infolist.empty();
}

int KMPackageManager::cleanupOldRefsWithRuntimeGuard(const std::vector<std::string> &oldRefs, bool isForceInstall, bool includeHostAppCleanup)
{
    std::vector<std::string> deferredRefs = d->m_deferredCleanupStore->loadDeferredRefs();
    std::unordered_set<std::string> deferredSet(deferredRefs.begin(), deferredRefs.end());
    const bool runningContainersExist = hasRunningContainers();

    for (const auto &ref : oldRefs)
    {
        KMEntry entry = cutRef(ref);
        const bool shouldDeferRuntimeRemoval = runningContainersExist && (entry.kind == "runtime" || entry.kind == "base");

        if (shouldDeferRuntimeRemoval)
        {
            if (!deferredSet.count(ref))
            {
                deferredRefs.emplace_back(ref);
                deferredSet.insert(ref);
                kmlogger.info("Deferred old ref cleanup while apps running: %s", ref.c_str());
            }
            continue;
        }

        if (includeHostAppCleanup)
        {
            int ret = removeHostAppDeployFiles(ref, TaskType::TASK_TYPE_UPGRADE);
            if (ret != KM_ERROR_NO)
            {
                return ret;
            }
        }

        removeSpecificDeployFiles(ref);
        if (!isForceInstall)
        {
            deleteSpecificRefsFromRepo(ref);
        }
        removePackageInfoFromList(entry.id, entry.version, entry.arch, entry.module);

        if (entry.kind == "app")
        {
            killAppid(entry.id);
        }
    }

    d->m_deferredCleanupStore->saveDeferredRefs(deferredRefs);
    return KM_ERROR_NO;
}

void KMPackageManager::processDeferredOldRefsCleanupOnBootIfNeeded()
{
    const std::string currentBootId = KMDeferredCleanupStore::readKernelBootId();
    if (currentBootId.empty())
    {
        kmlogger.warn("processDeferredOldRefsCleanupOnBootIfNeeded: cannot read boot_id, skip");
        return;
    }

    if (currentBootId == d->m_deferredCleanupStore->readLastCleanupBootId())
    {
        kmlogger.debug("Deferred old refs cleanup skipped (same boot_id as last run, likely systemctl restart)");
        return;
    }

    processDeferredOldRefsCleanup();
    d->m_deferredCleanupStore->writeLastCleanupBootId(currentBootId);
}

void KMPackageManager::processDeferredOldRefsCleanup()
{
    std::vector<std::string> deferredRefs = d->m_deferredCleanupStore->loadDeferredRefs();
    if (deferredRefs.empty())
    {
        return;
    }

    std::vector<std::string> remainRefs;
    // const bool runningContainersExist = hasRunningContainers();

    for (const auto &ref : deferredRefs)
    {
        KMEntry entry = cutRef(ref);
        // if (runningContainersExist && (entry.kind == "runtime" || entry.kind == "base"))
        // {
        //     remainRefs.emplace_back(ref);
        //     continue;
        // }

        removeSpecificDeployFiles(ref);
        deleteSpecificRefsFromRepo(ref);
        removePackageInfoFromList(entry.id, entry.version, entry.arch, entry.module);
    }

    d->m_deferredCleanupStore->saveDeferredRefs(remainRefs);
}
