/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef KMOSTREEHANDLER_H
#define KMOSTREEHANDLER_H

// #include <glib-2.0/glib/gvariant.h> // Exist compire error, so replace it with <glib.h>
#include <glib.h>
#include <ostree-1/ostree.h>

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <deque>

#include "KMUtil.h"

using namespace std;
namespace fs = std::filesystem;

typedef struct _ProgressStatus
{
    string ref;
    string lastRef;

    /* Extra data information */
    guint64 outstandingExtraData;
    guint64 totalExtraData;
    guint64 transferredExtraDataBytes;
    guint64 totalExtraDataBytes; /* the sum of all extra data file sizes (in bytes) */

    char *ostreeStatus; /* only sent by OSTree when the pull ends (with or without an error) */
    guint64 startTime;
    guint64 bytesTransferred;     /* every and all transferred data (in bytes) */
    guint64 fetchedDeltaPartSize; /* the size (in bytes) of already fetched static deltas */
    guint64 totalDeltaPartSize;   /* the total size (in bytes) of static deltas */

    guint outstandingWrites; /* all missing writes (sum of outstanding content, metadata and delta writes) */
    uint32_t fetched;        /* sum of content + metadata fetches */
    uint32_t requested;      /* sum of requested content + metadata fetches */

    guint outstandingMetadataFetches; /* missing metadata-only fetches */
    guint metadataFetched;            /* the number of fetched metadata objects */
    guint totalDeltaParts;

    /* Self-progress-reporting fields, not from OSTree */
    guint progress;
    guint lastTotal;

    /* Flags */
    guint estimating : 1;
    guint lastWasMetadata : 1;
    guint done : 1;
    guint reportedOverflow : 1;

} ProgressStatus;

struct RemoteInfo
{
    std::string name;
    std::string url;
    int priority;
};

/**
 * @brief ostree hander
 */
class KMOSTreeHandler
{
public:
    KMOSTreeHandler(const string &path);
    ~KMOSTreeHandler();

    typedef function<void(ProgressStatus progressStatus, gpointer userData)> CallBack;

    void cancel();

    /**
     * @brief 根据 ref 的完整id 获取最新的 commit
     *
     * @param refCompletedId ref 的完整id
     * @return string 最新的 commit
     */
    string readLatestCommit(const string &refCompletedId);

    /**
     * @brief 获取 remote 地址
     *
     * @param remoteName
     * @return string
     */
    string getRemoteUrl(const string &remoteName);

    /**
     * @brief Get the Local Remote Url object
     *
     * @param configPath
     * @param remoteName
     * @return std::string
     */
    std::string getLocalRemoteUrl(const std::string &configPath, const std::string &remoteName);

    /**
     * @brief 数据同步，同步之后数据为压缩格式，执行完成后需要 checkout，同步函数
     *
     * @param remoteName 远程仓库名称
     * @param vecRefs 要拉取的引用列表
     * @param cb 进度回调函数
     * @param lowSpeedLimitBytes 最低速度限制（字节/秒），0表示不限速
     * @return 成功返回true，失败返回false
     */
    bool pull(const string &remoteName, vector<string> vecRefs, CallBack cb, gpointer userData, unsigned int maxSpeedLimitKilobytes = 0, vector<string> localcacheRepos = {});

    /**
     * @brief 多仓库数据同步
     * 
     * @param refs 
     * @param cb 进度回调
     * @return true 
     * @return false 
     */
    bool pullWithFallback(const std::vector<std::string> &refs, CallBack cb, gpointer userData, unsigned int maxSpeedLimitKilobytes = 0);
    
    /**
     * @brief 从本地临时 repo 合并到主 repo（用于并发安装）
     * 
     * 将临时 repo 中的 refs 合并到当前 repo（主 repo）
     * 类似 ostree pull-local 命令的功能
     * 
     * @param tmpRepoPath 临时 repo 的路径
     * @param vecRefs 要合并的 ref 列表
     * @param cb 进度回调函数（可选，可以为 nullptr）
     * @param userData 用户数据
     * @param repoRefMap remote 到 refs 的映射（可选，用于统一 ref 命名时确定每个 ref 对应的 remote）
     * @return true 成功
     * @return false 失败
     */
    bool pullLocal(const std::string &tmpRepoPath, const std::vector<std::string> &vecRefs, CallBack cb = nullptr, gpointer userData = nullptr, const std::map<std::string, std::vector<std::string>> *repoRefMap = nullptr);

    /**
     * @brief 检出，同步函数
     *
     * @param commit
     * @return true
     * @return false
     */
    bool checkout(const string &commit, const string &tmpTargetPath, const string &subPath = "");

    /**
     * @brief 创建 deploy ref, 为了避免 deployed commits being pruned (pull --no-deploy)
     *
     * ostree lookup: ostree refs /opt/kaiming/repo |grep deploy
     * deploy ref formats: deploy/app/org.kde.kclock/x86_64/master
     *
     * @param completedRef 完整的ref，如 app/org.kde.kclock/x86_64/master
     * @param commit ref对应的commit, 如 f3b1314b51f2b8865089f13cbae5b7cd449b169f0b36530d27da74beb68aa492
     * @return true
     * @return false
     */
    bool updateDeployRef(const string &completedRef, const string &commit);

    /**
     * @brief 从 repo 中删除 ref 标签，没有真实删除数据，需要执行 repoPrune 同步删除数据
     *
     * @param remoteName
     * @param ref
     * @param commit
     * @return true
     * @return false
     */
    bool deleteRefTag(const string &remoteName, const string &ref, const string &commit = "");

    /**
     * @brief ostree repo 删除数据
     *
     * @return true
     * @return false
     */
    bool repoPrune();

    void updateProgress();

    /**
     * @brief 获取 remote 对应已安装的所有 app 和 runtime
     *
     * @return map<string, KMRef>:
     *  key：commit
     *  value：ref 结构体
     */
    map<string, KMRef> getLocalRefs();

    map<string, map<string, KMRef>> getAllLocalRefs();

    static void replaceRef(std::string &ref);

    /**
     * @brief       Get Ostree the Branch Timestamp
     * @param       branch_name : 分支名， 比如："stable/x86_64/app/com.qq.QQmusic/binary/1.1.5"
     * @return      std::string ： 返回时间戳
     */ 
    std::string getBranchTimestamp(const char *target_branch);

    /**
     * @brief Get the All Remotes With Priority object
     * 
     * @return std::vector<RemoteInfo> 远程仓库信息 
    */
    std::vector<RemoteInfo> getAllRemotesWithPriority();

    void refreshRemoteUrl();
    void reloadRepo();
    std::string getCurrentRemoteUrl(const std::string& remoteName);

    bool isCancelled() const;
    bool resetCancelled() const;

    // 用于进度回调访问进度状态（支持并发安装）
    ProgressStatus& getProgressStatus();
    std::deque<std::string>& getQueuePull();

    bool getSizeMode() const;
    guint64 getTotalDownloadSize() const;
    guint64 getCompletedBytes() const;
    const std::unordered_map<std::string, guint64>& getRefSizeMap() const;

private:
    void initProgress();
    bool readDetachedSizeInfo(const std::string &ref, guint64 &downloadSize, guint64 &installSize);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif