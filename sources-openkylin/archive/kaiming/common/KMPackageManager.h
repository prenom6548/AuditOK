/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */
#ifndef KMPACKAGEMANAGER_H
#define KMPACKAGEMANAGER_H
#define KAIMING_DELTAS (access("/opt/kaiming-tools/bin/xdelta3", F_OK) == 0 ? "/opt/kaiming-tools/bin/xdelta3" : "/usr/bin/xdelta3")
#include <dbus/dbus.h>
#include <sys/stat.h> // 包含 chmod 函数的头文件

#include <cctype>     // 包含 isupper 函数
#include <cstdlib>    // 用于 std::system 函数
#include <functional> // 进度条函数
#include <initializer_list>
#include <iomanip> // For std::fixed and std::setprecision
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <variant>
#include <sys/stat.h>
#include <atomic>

#include "common/KMFormatter.h"
#include "common/KMEnv.h"
#include "common/KMError.h"
#include "common/KMConfig.h"
#include "common/KMUtil.h"
#include "common/KMCurl.h"
#include "common/KMOSTreeHandler.h"
#include "common/KMParseSummaryJson.h"
#include "common/KMInfoJson.h"
#include "common/KMFolder.h"
#include "common/KMOKFile.h"
#include "common/KMProcessUtils.h"
#include "common/KMHostAppDeploy.h"
#include "KMLibHandle.h"
#include "app/list/KMList.h"

typedef struct _KMPMProgressStatus
{
    std::string refId;
    int progress;
    std::string speed;
    int timeout;
} KMPMProgressStatus;

typedef struct _KMfilterInfo
{
    std::string id;
    std::string version;
    std::string arch;
    std::string module;
} KMfilterInfo;

typedef function<void(std::string ProgressStatus)> KMPMProgressCallBack;

/**
 * @brief 任务完成回调函数类型
 * @param refId 包名
 * @param operateType 操作类型（KM_INSTALL_OPERATION等）
 * @param errorCode 错误码
 * @param errorMessage 错误信息
 */
typedef function<void(const std::string &refId, int operateType, int errorCode, const std::string &errorMessage)> KMPMCompletionCallBack;

/**
 * @brief 从用户传来的命令行参数
 * 
 */
typedef struct PackageManagerOptions
{
    bool assumeYes = false;                  // 用于自动执行操作, false 表示不自动执行操作，true 表示自动执行操作
    std::string arch;                        // 用于指定架构
    std::string kind;                        // 用于指定类型,如 app, runtime, base, extension, depend
    std::string module;                      // 用于指定模块，如 binary, devel
    std::string relativeRepoPath;            // 相对仓库路径
    bool isPrint = true;                     // 是否打印信息
    bool isOnlineInstall = false;            // 是否是在线安装，用于判断是否验签
    bool isUpgradeType = false;              // 用于区分在线安装和upgrade，只有在线安装验签，pgrade不验签
    bool deletedataYes = false;              // 默认不删除应用数据
    bool force = false;                      // 默认不强制安装已经安装的应用
    bool skipSignatureVerification = false;  // 默认不跳过签名验证
    bool upgradable = false;                 // 默认不列出可更新软件包
    bool isGetDevel = false;                 // complete 默认不获取devel
    bool compatible = false;                 // 安装兼容版本
    unsigned int maxSpeedLimitKilobytes = 0; // 最大下载速度限制，单位KB/s，0表示不限速
    bool bundle = false;
    bool isDownloadOnly = false;            // 是否仅下载不安装
    bool isDeployOnly = false;
    std::string subpath;
} PackageManagerOptions;

struct InstallTaskContext {
    std::atomic<bool> cancelRequested{false};
    std::atomic<bool> networkBroken{false};
    std::atomic<bool> stopNetworkCheck{false}; // pull完成后不检查网络
    guint64 networkDisconnectedTime = 0; // 单位：usec
};

/**
 * @brief 下载任务上下文，包含单个下载任务的所有状态
 * 支持多个任务并行下载，每个任务有独立的上下文
 * 合并了 InstallTaskContext 的功能，包含任务取消和网络监控
 */
// 任务状态枚举
enum class TaskState {
    RUNNING,    // 正在运行
    PAUSED,     // 已暂停
    CANCELLED,  // 已取消
    COMPLETED   // 已完成
};

struct DownloadTaskContext
{
    // 任务标识
    std::string taskId; // 任务唯一标识

    // 主应用信息
    std::string mainAppName; // 主应用名

    // 下载列表和进度
    std::vector<KMPMRef> refDownloadList;                       // 该任务的下载列表
    std::map<std::string, std::vector<std::string>> repoRefMap; // remote 到 refs 的映射（用于下载）
    KMPMProgressCallBack downloadCallback;                      // 该任务的下载进度回调
    KMPMProgressCallBack deployCallback;                        // 该任务的部署进度回调
    KMPMCompletionCallBack completionCallback;                  // 该任务的完成回调（用于发送finishsignal）
    KMPMProgressStatus progressStatus;                          // 该任务的进度状态
    std::string progressJson;                                   // 该任务的进度JSON
    int currentRefIndex = 0;
    int totalRefCount = 0;

    // 任务控制（合并自 InstallTaskContext）
    std::atomic<bool> cancelRequested{false};  // 取消请求标志
    std::atomic<bool> networkBroken{false};    // 网络是否断开
    std::atomic<bool> stopNetworkCheck{false}; // pull完成后不检查网络
    guint64 networkDisconnectedTime = 0;       // 网络断开时间，单位：usec

    // 暂停/恢复相关
    std::atomic<TaskState> taskState{TaskState::RUNNING};             // 任务状态
    std::mutex pauseResumeMutex;                                      // 保护暂停/恢复操作的互斥锁
    std::map<std::string, std::vector<std::string>> completedRemotes; // 已完成的 remote->refs 映射（用于恢复时跳过）
    std::string currentRemote;                                        // 当前正在处理的 remote（暂停时记录）
    std::string currentRef;                                           // 当前正在处理的 ref（暂停时记录）

    // 网络状态（每个任务独立）
    bool isNetworkDisconnected = false; // 网络是否断开
    bool isPrintDisconnected = false;   // 是否已打印断开信息
    bool isPrintFailed = false;         // 是否已打印失败信息

    // UI相关状态（如果需要每个任务独立显示）
    std::string lastOutput;
    std::string preRemaining;
    int currentPercent = 0;
    int totalPercent = 0;
    int lastWidth = 0;
    std::string lastProgressStr;
    std::string lastStaticInfo;
    bool staticInfoDisplayed = false;

    // 并发安装相关
    std::string tmpRepoPath;                     // 临时 repo 路径（用于下载到临时位置）
    std::shared_ptr<KMOSTreeHandler> tmpHandler; // 临时 repo handler（用于取消下载操作）

    // 安装选项和结果
    PackageManagerOptions pmOptions;            // 安装选项（包含 maxSpeedLimitKilobytes 等）
    std::vector<std::string> cannotInstallRefs; // 无法安装的包列表
    int installResult = KM_ERROR_NOTHING_TO_DO; // 安装结果

    // 部署相关状态（从全局状态中提取，支持并发）
    std::set<std::string> installedFiles;      // 该任务安装的文件列表

    KMPackageManager* packageManager;

    DownloadTaskContext() : progressStatus({"", 0, "", 5}) {}

    // 向后兼容：可以隐式转换为 InstallTaskContext 的引用
    // 用于需要 InstallTaskContext* 参数的函数
    InstallTaskContext &asInstallTaskContext()
    {
        return *reinterpret_cast<InstallTaskContext *>(&cancelRequested);
    }
};

/**
 * @brief 统一包管理
 * 
 * 将安装、卸载、更新、修复等功能进行统一处理
 * 
 */
class KMPackageManager
{
public:
    KMPackageManager();
    ~KMPackageManager();
    KMPackageManager(const KMPackageManager &) = delete;
    KMPackageManager(KMPackageManager &&) = delete;
    auto operator=(const KMPackageManager &) -> KMPackageManager & = delete;
    auto operator=(KMPackageManager &&) -> KMPackageManager & = delete;

public:
    /**
     * @brief 用于分发安装、卸载、更新、修复等任务
     * 
     * @param taskType 任务类型
     * @param vectorIds 解析的 ref 列表
     * @return int 任务执行返回
     */
    int processTask(TaskType taskType, const std::vector<std::string> &vectorIds, PackageManagerOptions pmOptions);

    /**
     * @brief 安装在线包
     * 
     * @return int 执行返回
     */
    int install(const std::vector<std::string> &vectorIds, PackageManagerOptions pmOptions);

    /**
     * @brief 安装在线包
     * 
     * @return int 执行返回
     */
    int installOfflinePackage(const vector<std::string> &vectorIds, PackageManagerOptions pmOptions, KMPMProgressCallBack downloadCallback = nullptr, KMPMProgressCallBack deployCallback = nullptr);

    /**
     * @brief 给软件商店调用的api（异步并发版本）
     *
     * @param name 开明包名，如org.kde.kclock
     * @param version 开明版本号，这里实际上是ostree
     * branch版本，暂未细化到具体子版本。空代表最新版本，如"master"
     * @param installCallback 进度回调函数
     * @param maxSpeedLimitKilobytes 最大下载速度限制，单位KB/s，0表示不限速
     * @param completionCallback 完成回调函数（可选，用于发送finishsignal）
     * @return std::string 任务ID（job-id），用于后续查询和取消
     */
    std::string installSoftwareCenter(const std::vector<std::string> &vecNames, string version, KMPMProgressCallBack downloadCallback, KMPMProgressCallBack deployCallback, PackageManagerOptions pmOption, unsigned int maxSpeedLimitKilobytes = 0, KMPMCompletionCallBack completionCallback = nullptr);

    /**
     * @brief 将安装任务加入队列（异步并发安装）
     *
     * @param name 开明包名
     * @param version 版本号
     * @param installCallback 进度回调函数
     * @param pmOptions 安装选项
     * @param completionCallback 完成回调函数（可选，用于发送finishsignal）
     * @return std::string 任务ID
     */
    std::string enqueueInstall(const std::vector<std::string>& vectorIds, std::string version, KMPMProgressCallBack downloadCallback, KMPMProgressCallBack deployCallback, PackageManagerOptions pmOptions, KMPMCompletionCallBack completionCallback);
    
    /**
     * @brief 取消安装任务
     * 
     * 支持两种方式：
     * 1. 按 taskId 取消
     * 2. 按包名取消
     *
     * @param taskIdOrName 任务ID或包名
     * @return true 取消成功
     * @return false 任务不存在或已完成
     */
    bool cancelInstall(const std::string& taskIdOrName);
    
    /**
     * @brief 暂停安装任务
     *
     * 实现方式：cancel 当前的 pull + 记录状态
     * 利用 OSTree 的 object 幂等性，恢复时可以从暂停位置继续
     *
     * 支持两种方式：
     * 1. 按 taskId 暂停
     * 2. 按包名暂停
     *
     * @param taskIdOrName 任务ID或包名
     * @return true 暂停成功
     * @return false 任务不存在或已完成
     */
    bool pauseInstall(const std::string& taskIdOrName);

    /**
     * @brief 恢复安装任务
     *
     * 实现方式：从记录的进度位置重新 pull
     * 利用 OSTree 的 object 幂等性，已下载的对象不会重复下载
     *
     * 支持两种方式：
     * 1. 按 taskId 恢复
     * 2. 按包名恢复
     *
     * @param taskIdOrName 任务ID或包名
     * @return true 恢复成功
     * @return false 任务不存在或未暂停
     */
    bool resumeInstall(const std::string& taskIdOrName);

    /**
     * @brief 获取任务状态
     *
     * @param taskId 任务ID
     * @return KMPMProgressStatus 任务状态，如果任务不存在则返回空状态
     */
    KMPMProgressStatus getTaskStatus(const std::string& taskId);

    int updateSoftwareCenter(const std::string& name, std::string version, KMPMProgressCallBack updateCallback, KMPMProgressCallBack deployCallback);

    /**
     * @brief 卸载软件商店调用的api
     * 
     * @param name 开明包名，如org.kde.kclock
     * @return int 执行结果
     */
    int uninstallSoftwareCenter(const std::string& name);

    int deploySoftwareCenter(const std::vector<std::string>& names, PackageManagerOptions pmOptions, KMPMProgressCallBack deployCallback);

    /**
     * @brief 兼容其他类型 OK 离线包
     * 
     * @return int 
     */
    int installFromOK(const std::string &name, std::vector<std::string> &onlinePackageIds, PackageManagerOptions pmOptions, std::map<std::string, std::string> &mountMap, bool &canNotInstall);

    /**
     * @brief 卸载软件包
     * 
     * @param vectorIds 包含待卸载软件包的ID列表
     * @param pmOptions 卸载时的选项参数
     * @return int 返回卸载操作的结果，成功为0
     */
    int uninstall(const vector<std::string> &vectorIds, PackageManagerOptions pmOptions);

    /**
     * @brief 加载所有info.json文件，并返回其信息映射
     * 
     * @return std::unordered_map<std::string, KMInfoJson> 包含所有info.json文件信息的映射
     */
    std::unordered_map<std::string, KMInfoJson> loadAllInfoJsonFiles(std::string pathStr);

    /**
     * @brief 打印待卸载的软件包列表
     * 
     * @param uninstallRefs 待卸载软件包的ID列表
     */

    void printUninstallList(const KMPMRef& pmRef);

    /**
     * @brief: 打印待卸载的依赖软件
     * @return true存在依赖于该软件包的软件, false不存在依赖于该软件包的软件
     */
    bool printUninstallDependsOnList(const std::vector<std::string> &UninstallVector);

    /**
     * @brief 卸载depend软件包
     * 
     * @param uninstallRef 待卸载软件包的depend
     * @param UninstallVector 卸载队列
     */
    bool removeDepend(const string& name, const KMRef& uninstallRef, std::vector<std::string> &UninstallVector);

    /**
     * @brief 打印是否选择卸载
     * 
     */
    bool isContinueUninstall();

    /**
     * @brief 展示用户选择是否假设为卸载操作的提示信息
     * 
     * @param pmOptions 卸载时的选项参数
     * @return bool 用户是否选择了假设卸载操作
     */
    bool printAssumeUninstallYesChoice(const PackageManagerOptions& pmOptions, const KMPMRef& pmRef);

    bool removeDeployFiles(const std::string &name);
    bool removeTmpDeployFiles(const fs::path &path);
    bool removeDeployFiles();
    bool removeSpecificDeployFiles(const std::string &str);
    void deleteFilesIfExist(const std::set<std::string> &fileList);
    std::vector<KMRef> reserveVersion();

    /**
     * @brief 从存储库中删除指定应用或运行时的引用
     * 
     * @param name 要删除引用的应用或运行时名称
     * @return bool 删除是否成功
     */
    bool deleteRefsFromRepo(const std::string &name);
    bool deleteSpecificRefsFromRepo(const std::string &str);

    /**
     * @brief 删除指定应用或运行时的用户数据
     * 
     * @param name 要删除用户数据的应用或运行时名称
     * @return bool 删除是否成功
     */
    bool deleteUserData(const std::string &name);

    /**
     * @brief: delete hostapp deploy files
     * @return int 错误码
     */
    int deleteHostAppFiles();

    void removeEmptyDirs(const std::set<std::string> &files);

    /**
     * @description: 关闭运行程序
     * @param {string} &name appid
     * @return {*}
     */
    void killAppid(const std::string &appId);

    /**
     * @brief 更新软件包
     * 
     * @param vectorIds 解析的 ref 列表
     * @param pmOptions 用户输入的参数
     * @return int 执行返回
     */
    int update(const std::vector<std::string> &vectorIds, PackageManagerOptions pmOptions);

    /**
     * @brief 获取需要upgrade的软件包
     * 
     * @param pmOptions 用户输入的参数
     * @return std::vector<std::string>
     */
    std::vector<std::string> getUpgradeInfo(PackageManagerOptions pmOptions);

    /**
     * @brief 过滤已安装但summary.json不存在的应用
     * 
     * @param pmOptions 
     * @return std::vector<KMfilterInfo> 
     */
    int filterIdsNotInRemote(PackageManagerOptions pmOptions);

    /**
     * @brief 获取需要更新的软件包
     * 
     * @param vectorIds 解析的 ref 列表
     * @param pmOptions 用户输入的参数
     * @return int 执行返回
     */
    int getUpgradableList(const std::vector<KMlocalAppsInfo> &localAppsInfo, PackageManagerOptions pmOptions);

    /**
     * @brief 搜索远程所有软件包
     * *@param vectorIds 解析的 ref 列表
     * @param pmOptions 用户输入的参数
     * @return int 
     */
    int search(const std::string &strID, PackageManagerOptions pmOptions);

    /**
     * @brief 调用dbus服务搜索远程所有软件包
     * *@param vectorIds 解析的 ref 列表
     * @param pmOptions 用户输入的参数
     * @return int 
     */
    int dbusSearch(const std::string &strID, PackageManagerOptions pmOptions);

    /**
     * @brief 更新软件包
     * 
     * @param vectorIds 解析的 ref 列表
     * @param pmOptions 用户输入的参数
     * @return int KM_ERROR_NO: 成功; 其它失败
     */
    int searchInfo(const std::string &strID, PackageManagerOptions pmOptions, std::vector<std::string> &strCompletePath);

    /**
     * @brief 获取远程包信息，供dbus服务使用
     *
     * @param name 包名
     * @param strRemoteInfoJson 返回的JSON信息
     * @param allRemoteInfoJson 返回的所有远程信息列表
     * @param remoteKind 包类型
     * @return int 返回操作结果
     */
    int getRemoteInfo(const std::string &name, std::string &strRemoteInfoJson, std::vector<std::string> &allRemoteInfoJson, std::string &remoteKind, bool &isNameNullable);

    /**
     * @brief 获取ok包的deb信息，供dbus服务使用
     *
     * @param name ok包名
     * @param errorCode 错误码
     * @return string 返回操作结果
     */
    string getDebInfoFromOK(const string &name, int &errorCode);

    /**
     * @brief 对于已安装软件包中存在损坏的文件进行修复
     * 
     * @return int 
     */
    int repair(const std::vector<std::string> &vectorIds, PackageManagerOptions pmOptions);

    /**
     * @brief 当前 module=binary ref 是否安装
     * 
     * @param pmRef 
     * @return true 
     * @return false 
     */
    bool currentRefIsInstalled(KMPMRef &pmRef);

    /**
     * @brief 当前 ref 指定版本是否安装
     * 
     * @param pmRef 
     * @return true 
     * @return false 
     */
    bool currentRefVersionIsInstalled(KMPMRef &pmRef);

    /**
     * @brief 当前 ref 是否安装
     * 
     * @param pmRef 
     * @return true 
     * @return false 
     */
    bool currentRefIsUninstalled(const KMPMRef& pmRef);
    /**
     * @brief 当前module=binary  ref 是否已安装高版本
     * 
     * @param pmRef 
     * @return true 
     * @return false 
     */
    bool currentFuzzyRefIsInstalled(KMPMRef &pmRef);

    /**
     * @brief 当前包是否安装, 给软件商店使用
     *
     * @param name
     * @return int
     */
    int isInstalled(std::string name);

    /**
     * @brief 从ok包中提取信息
     * 
     * @param filepath 
     * @return std::vector<std::string>
     */
    std::vector<std::string> extractInfoFromOK(const std::string &filepath);

    /**
     * @brief 当前包是否可更新, 给软件商店使用
     *
     * @param name
     * @return int
     */
    int isUpgradabled(string name);

    /**
     * @brief 批量检查多个包是否可更新, 给软件商店使用
     *
     * @param names 包名列表
     * @param results 返回结果列表，包含(包名, 是否可更新)的pair
     * @return int 整体错误码
     */
    int getAllUpgradableStatus(const std::vector<std::string> &names, std::vector<std::pair<std::string, bool>> &results);

    /**
     * @brief 得到 summary.json 并解析
     * 
     * @param summaryUrl summary.json 路径
     * @param isExpired summary.json 是否过期
     * @return  是否下载 summary.json 并解析成功, KM_ERROR_NO 表示成功
     */
    int getRemoteSummary(const std::string &summaryUrl, bool isExpired);

    int getRemoteSummary(const string &summaryUrl, const string &localPath, bool isExpired);
    /**
     * @brief 遍历取得所有符合条件的在线包依赖列表
     * 
     * @param vectorIds 用户输入的 ref 列表
     * @param pmOptions 用户输入的参数
     * @param taskType 任务类型
     * @return int 返回操作结果
     */
    int getAllFuzzyMatchRefVector(const std::vector<std::string> &vectorIds, PackageManagerOptions pmOptions, TaskType taskType);

    /**
     * @brief 获取真正安装列表
     * 
     * 对获取的所有依赖列表重新封装 KMPMRef ，再次进行是否安装过滤操作。原因是 app 依赖 base, runtime，但 base, runtime 会重新出现在依赖列表中
     * 
     * @param taskType 任务类型
     */
    void getDownloadRefList(TaskType taskType, PackageManagerOptions pmOptions);

    /**
     * @brief Get the Completed Ref object
     * 
     * @param name 
     * @param version 
     * @param branch 
     * @param fuzzyMatchRefs 
     * @return true 
     * @return false 
     */
    int getCompletedRef(std::string name, std::string version, std::string branch, std::vector<std::string> &pmFuzzyRef);

    int getRemoteStatusInfo(const std::string name, const std::string version, const std::string branch, std::string &packInfoToJson, int &errorCode);

    /**
     * @brief 获取有用安装列表
     * 
     * 对获取的所有安装列表KMPMRef ，再次进行版本过滤操作。
     * 例： app的base版本号为：2.0.2；runtime的base版本号为：2.0.3；则在kaiming install app时，下载的base版本为2.0.3
     *
     */
    void getUsefulRefList();

    /**
     * @brief  打印安装列表
     * 
     */
    void printDownloadRefList(const std::vector<KMPMRef> &downloadRefs);

    /**
     * @brief 用户选择是否展示信息
     * 
     * @param pmOptions 
     */
    bool printAssumeYesChoice(PackageManagerOptions pmOptions);

    /**
     * @brief 获取终端宽度
     * 
     */
    static int getTerminalWidth();

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
    static void progressCallback(ProgressStatus progressStatus, gpointer userData);
    
    /**
     * @brief 带上下文的进度处理回调函数（支持并发安装）
     *
     * 每个任务有独立的上下文，进度更新会调用该任务的 progressCallback
     * 
     * @param progressStatus 在 ostree pull 时，发送的进度等信息结构体
     * @param context 任务上下文，包含该任务的下载列表、进度回调等
     */
    static void progressCallbackWithContext(ProgressStatus progressStatus, std::shared_ptr<DownloadTaskContext> context);

    /**
     * @brief 检查网络连接
     *
     * @param handler ostree handler
     * @return true 网络正常
     * @return false 网络异常
     */
    static int checkNetworkConnection(std::shared_ptr<KMOSTreeHandler> handler);

    static std::string extractHostPart(const std::string& url);

    static int extractPort(const std::string& url);

    static bool testTcpConnection(const std::string& host, int port, int timeoutSeconds);

    static bool testTcpConnectionWithFallback(const std::string& host, int preferredPort, int timeoutSeconds);

    /**
     * @brief 检测网络断开是否超过10s
     * 
     * @param handler 
     */
    static void checkAndHandleNetwork(std::shared_ptr<KMOSTreeHandler> handler, InstallTaskContext* ctx, gpointer userData);
    /**
     * @brief 增加最后写入对象的 3% 进度
     * 
     * @param outstandingWrites 
     * @return int 
     */
    static int getWriteProgress(int outstandingWrites);

    /**
     * @brief 返回要下载列表所有的安装大小总和
     * 
     * @return uint64_t 下载列表所有的安装大小总和
     */
    static uint64_t getTotalSize(gpointer userData);

    /**
     * @brief 得到当前进度条下载的completedRef 对应的要安装大小
     * 
     * @param completedRef 当前进度条下载的completedRef 
     * @param id 当前进度条下载的id
     * @return uint64_t 安装大小
     */
    static uint64_t getSingleSize(const std::string &completedRef, std::string &id, gpointer userData);
    /**
     * @brief 更新进度信息
     * 
     */
    static void updateProgress(gpointer userData, bool isDeploy);

    /**
     * @brief 格式化时间
     * 
     * @param duration ostree 返回的时间
     * @return string 格式后的时间
     */
    static std::string formatDuration(guint64 duration);

    /**
     * @brief 计算需要下载时间
     * 
     * @param speed 下载速度
     * @param sizeBytes 下载的字节数
     * @return std::string 需要下载的时间
     */
    static std::string calculateDownloadTime(const std::string &speed, guint64 sizeBytes);

    /**
     * @brief 清空旧的进度条
     * 
     */
    static void clearOldProgress(gpointer userData);

    void emitTotalDeployProgress(int currentRefPercent);
    void emitTotalDeployProgressWithContext(int currentRefPercent, std::shared_ptr<DownloadTaskContext> context);

    /**
     * @brief 部署数据
     * 
     * @param ostreeManager ostree 操作句柄
     * @return int 部署状态
     */
    int deploy(PackageManagerOptions pmOptions);

    /**
     * @brief 部署离线包数据
     * 
     * @param mountpoint 挂载点
     * @param name 离线包名称
     * @return int 部署状态
     */
    int deployOfflinePackage(std::map<std::string, std::string> &mountMap);

    /**
     *  @brief 检查文件是否存在，如果不存在则创建文件并输入空字符串
     *
     *  新建 "active/files/.ref"，并输入空字符串 ""
     * 
     * @param filename 
     */
    void createFileForEmptyString(const std::string &filename);

    /**
     * @brief 更改service文件
     *
     * @param pmRef
     * @param serviceFileDirectoryPath
     */
    void modifyServiceFile(KMPMRef &pmRef, const std::string &serviceFileDirectoryPath,  const std::string destPath, std::set<std::string> &installedFiles);

    /**
     * @brief 获取当前目录下的所有文件
     *
     * 对于获取service文件有用
     *
     * @param directoryPath
     * @param files
     * @return true
     * @return false
     */
    bool getAllFilesInDirectory(const std::string &directoryPath, std::vector<std::string> &files);

    /**
     * @brief 获取kind=base\runtime当前目录下的所有文件
     *
     * @param directoryPath
     * @param files
     * @return true
     * @return false
     */
    bool getAllBaseFilesInDirectory(const std::string &directoryPath, std::set<std::string> &files);

    /**
     * @brief 获取kind=app当前目录下的所有文件
     *
     * @param directoryPath
     * @param files
     * @return true
     * @return false
     */
    bool getAllAppFilesInDirectory(const std::string &directoryPath, std::set<std::string> &files, const std::string &id);

    /**
     * @brief 读取 .desktop 文件内容
     *
     * @param filename
     * @return std::vector<std::string>
     */
    std::vector<std::string> getDataFromKVFile(const std::string &filename);

    /**
     * @brief 修改 .desktop 文件内容
     *
     * @param filename
     * @param newLines
     * @return true
     * @return false
     */
    bool setDataToKVFile(const std::string &filename, const std::vector<std::string> &newLines);

    /**
     * @brief 更改desktop文件，比如修改Exec, Icon, X-Kaiming
     *
     * @param pmRef
     * @param exportDesktopFilePath
     * @param exportPath
     */
    void modifyDesktopFile(KMPMRef &pmRef, string exportDesktopFilePath, const string exportPath);

    /**
     * @brief Create a Desktop Shortcut object
     * 
     * @param pmRef 
     * @param shortcutPath 快捷方式路径
     * @return true 
     * @return false 
     */
    bool createDesktopShortcut(const KMPMRef &pmRef, const std::string& shortcutPath);

    /**
     * @brief 创建一个 entries/bin/ref.id shell脚本，写入执行内容，并赋予0755权限
     *
     * @param fileName
     * @param pmRef
     * @return true
     * @return false
     */
    bool createExportBinShellScript(KMPMRef pmRef, std::string fileName);

    bool createInstalledFile(string fileName, bool append, const std::set<std::string> &installedFiles);

    /**
     * @brief: 安装文件写入列表
     * @param {string} &filePath 待写入文件
     * @return {*}
     */
    void writeInstalledFilesToList(const std::string &filePath, const std::set<std::string> &installedFiles);

    void writeInstalledInfoToList();

    void removePackageInfoFromList(const std::string &packageId, const std::string &version, const std::string &arch, const std::string &module);
    
    void fillInstalledInfo(const KMPMRef &downloadRef);

    std::string formatFileSize(double size) const;

    KMEntry cutRef(const std::string &completeRef);

    /**
     * @brief 递归遍历/opt/kaiming/exports/目录并处理文件，比如创建软链接
     *
     * @param ref
     * @param srcDir
     * @param destDir
     */
    void modifyExportsDirectory(KMPMRef &ref, const std::string &srcDir, const std::string &destDir, std::set<std::string> &installedFiles);

    /**
     * @brief 接受不定数量的 cacheShellPath 参数，并将它们拼接到命令中，然后执行命令
     *
     * 作用是生成应用desktop, icon, service等缓存
     *
     * @param cacheShellPaths
     */
    void runMultiperTriggerUpdateCacheCommand(const std::vector<string> &cacheShellPaths);

    /**
     * @brief 接受不定数量的 cacheShellPath 参数，并将它们拼接到命令中，然后执行命令
     *
     *
     * @param cacheShellPaths
     */
    void runTriggerUpdateCacheCommand(const std::string &cacheShellPath);

    //本地进度条安装函数
    static void monitorProgress(FILE *pipe);

    /**
     * @brief 在线包&&离线包验签
     * 
     * @return int 执行结果
     */
    int verifySignature(std::string id, std::string kind, fs::path path);

    /**
     * @brief 获取配置的证书ouname
     * 
     * @return std::string 
     */
    std::string getCertificateOuname(KMLibHandle& lib);

    /**
     * @brief 获取证书CN字段
     * 
     * @param pemfile 
     * @return std::string 
     */
    std::string getPemCN(KMLibHandle& lib, const std::string& pemFilePath);

    /**
     * @brief 去除字符串两端的空格
     * 
     * @param str 
     * @return std::string 
     */
    std::string trim(const std::string& str);

    /**
     * @brief 分割字符串（如 "OU1,OU2" -> ["OU1", "OU2"]）
     * 
     * @param str 
     * @param delimiter 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> splitString(const std::string& str, char delimiter);

    /**
     * @brief 检查 CN 是否在 OU 列表中
     * 
     * @param ouname 
     * @param certCN 
     * @return bool
     */
    bool verifyCertCN(const std::string& ouname, const std::string& certCN);

    /**
     * @brief 验证证书
     * 
     * @param lib 
     * @param pemFile 
     * @return bool 
     */
    bool verifyCertificate(KMLibHandle& lib, const std::string& pemFile);
    
    /**
     * @brief 命令行验签
     * 
     * @param md5File 
     * @param sigFile 
     * @param pemFile 
     * @return std::string 
     */
    int executeSignatureVerification(const std::string& md5File, const std::string& sigFile, const std::string& pemFile);
    /**
     * @brief 判断签名工具是否存在
     * 
     * @return bool 执行结果
     */
    bool isKylinFileSignToolPresent();

    bool isV10Sp1();

    /**
     * @brief 获取安全模式
     * 
     * @return int 安全模式：1:关闭, 2:警告, 3:阻止, -1:失败
     */
    int getSafeMode(KMLibHandle& lib);

    /**
     * @brief 根据安全模式获取验签失败后的处理方式
     * 
     * @return int 执行结果
     */
    int resolveFailedVerification(int safeStatus, fs::path idPath, std::string packageName);

    /**
     * @brief 从Ostree拉取离线包
     * 
     * @return int 执行结果
     */
    int pullFromOstree(fs::path path, KMPMRef downloadRef);

    /**
     * @brief: 设置Ksaf标记和执行控制
     * @return KM_ERROR_NO：设置成功 KM_ERROR_SETLABEL_FAILED： 设置失败
     */
    KMErrorCodeEnum setKsafLabelAndExectl(const std::set<std::string> &files, const std::string &appName, const std::string &kind, const std::string &path);

    //初始化service文件修改字段
    void initializeFixedModifications();

    string addOkPrefix(const string &original, const std::string id);

    string addKillPrefix(const string &original, const std::string id);

    string formatExecStart(const std::string &line, const std::string id);

    string changeTasks(const std::string &original);

    bool isField(const std::string &line, const std::string &field);

    //修改service文件
    void modifyAppServiceFile(KMPMRef &pmRef, std::string serviceFile, std::string destPath);

    //修改service文件字段
    string modifyFixedField(const std::string &line, const std::string id);

    //获取service文件
    std::vector<string> getServiceName(const std::vector<std::string> &files);

    //定义回调函数类型
    using CallbackFunc = function<string(const std::string &, const std::string &)>;

    string getRemoteUrl(const std::string &remoteName);

    //删除硬链接
    void removeHardlinksInPlace(const fs::path &dir);

    /**
     * @brief 处理特殊文件的拷贝和安装
     * @param srcFilePath 源文件路径
     * @param destDir 目标目录
     * @return 是否成功
     */
    bool handleSpecialFileInstall(const std::string& srcFilePath, const std::string& destDir, bool isDir);

    void modifyBaseDir(std::string baseDir);

    /**
     * @brief Record the desktop filename and its md5
     * 
     * @param id 包名
     * @param desktopFilePath desktop文件路径
     * @return true 
     * @return false 
     */

    bool recordDesktopMd5(const std::string &id, const std::string &desktopFilePath, std::set<std::string> &installedFiles);

    /**
     * @brief Remove user desktop shortcuts matching md5 values
     * 
     * @param id 
     * @return true 
     * @return false 
     */
    bool removeUserDesktopShortcutsByMd5(const std::string &id);

    int downloadAllRemoteSummaries(bool isForce = false);
    /**
     * @brief copy directory files
     * 
     * @param source 源目录
     * @param destination 目标目录
     */
    bool copyDirectory(const fs::path& source, const fs::path& destination, vector<string>& ingoreExtension);

    std::string processDeleteLine(const std::string& line, KMRef& ref);

    /**
     * @brief 处理增量包需删除的文件
     * 
     * @param filename 目录路径
     * @param ref 包相关信息
     */
    bool processDeleteFile(const std::string& filename, KMRef& ref);

    bool upgradeDirectory(const std::string& newDir, const std::string& oldDir);

    void refreshRemoteConfiguration();

    int refreshSummaryCache();

    int cleanupOldRefsWithRuntimeGuard(const std::vector<std::string> &oldRefs, bool isForceInstall, bool includeHostAppCleanup = false);

    // 仅在系统新一次开机后、本服务首次启动时尝试清理
    void processDeferredOldRefsCleanupOnBootIfNeeded();

private:
    void processDeferredOldRefsCleanup();

    static std::string normalizeRefForCompare(std::string ref);

    static void reorderRefListByRepoMap(const std::map<std::string, std::vector<std::string>> &repoRefMap, std::vector<KMPMRef> &refList);

    static bool hasRunningContainers();

    int removeHostAppDeployFiles(const std::string &strPath, TaskType type = TaskType::TASK_TYPE_UNINSTALL);

    /**
     * @description: 清除缓存日志
     * @param {string} &id : 包名
     * @return true: 成功
     */
    bool clearCacheFiles(const std::string &id);

    /**
     * @description: 统一处理服务文件
     * @param {path&} sourceDir
     * @param {string&} destDir
     * @param {DownloadRef&} downloadRef
     * @return {*}
     */
    void processServiceFiles(const fs::path &sourceDir, const std::string &destDir, KMPMRef &downloadRef, std::set<std::string> &installedFiles);

    /**
     * @description: start自启动服务
     * @param {path} &sourceDir
     * @return {*}
     */
    void startTargetWantsServices(const fs::path &sourceDir);

    void printSearchJson();

    /**
     * @brief 创建 DBus 白名单可执行程序的软链接到宿主机配置目录
     *
     * @param vecDbusWhitelist DBus 白名单列表，每个元素是可执行程序完整路径
     * @param targetBaseDir 目标基础目录，默认为 /etc/kylin-config/basic
     * @return true 所有软链接创建成功
     * @return false 创建过程中出现错误
     */
    bool createDbusWhitelistSymlinks(const fs::path &versionDataPath, const std::vector<std::string> &vecDbusWhitelist, const std::string &targetBaseDir = "/etc/kylin-config/basic");

    void resetInstallState();

private:
    class Private;
    std::unique_ptr<Private> d;
    std::vector<std::string> m_upgragableRefs;
    std::vector<KMfilterInfo> m_upgragableList; // 可更新的列表
    std::unordered_map<std::string, KMInfoJson> m_infoMap;
    std::map<std::string, std::variant<std::string, CallbackFunc>> fixedModifications;

    gint64 networkDisconnectedTime = 0;
    bool isNetworkDisconnected = false;

    std::vector<KMPMRef> refDownloadListForProgress;
    KMPMProgressCallBack globalProgressCallback;
    KMPMProgressCallBack deployProgressCallback;
    std::string mainAppNameForDBus; // DBus调用时的主包名，用于refId设置
    KMPMProgressStatus globalProgressStatus = {"", 0, "", 5};
    std::string progressJson;
    std::string lastOutput;
    std::string preRemaining;
    int currentPercent = 0;
    int lastWidth = 0;
    std::string lastProgressStr;
    std::string lastStaticInfo;
    bool staticInfoDisplayed = false;
    bool installing = false;
    int networkFailCount = 0;
};

#endif
