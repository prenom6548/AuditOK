/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOSTreeHandler.h"
#include "KMPackageManager.h"

#include <sys/stat.h>  // For chmod
#include <iostream>
#include <queue>
#include <deque>       // For std::deque
#include <cstring>     // For memset
#include <cstdlib>     // For system()
#include <glib.h>

#include "common/KMLogger.h"

typedef struct
{
    const char *commit;
    guint64 timestamp;
    guint commitCacheVersion;
    bool indexedDeltas;

    const char *title;
    const char *comment;
    const char *description;
    const char *homepage;
    const char *icon;
    const char *collection_id;
    const char *commitDefaultBranch;
    const char *commitRedirectUrl;
    const char *deployCollectionId;
    const char *commitAuthenticatorName;

    GFile *root;
    GVariant *metadata;
} CommitInfo;

class KMOSTreeHandler::Private
{
public:
    fs::path m_repoPath;
    OstreeRepo *m_ostreeRepo = nullptr;
    GCancellable *m_cancellable = nullptr;
    OstreeAsyncProgress *m_pullProgress = nullptr;
    CallBack m_progressCallback = nullptr;
    gpointer m_userData = nullptr;

    GError *m_error = nullptr;

    bool m_initialized = false;
    bool m_cancelled = false;

    // 每个实例独立的进度状态（支持并发安装）
    ProgressStatus m_progressStatus;
    std::deque<std::string> m_queuePull;

    /* ====== size info ====== */
    guint64 m_totalDownloadSize = 0;
    std::unordered_map<std::string, guint64> m_refSizeMap;

    /* ===== runtime progress ====== */
    guint64 m_completedBytes = 0;
    std::string m_currentRef;
    bool m_sizeMode = false;
};

#ifdef OSTREE_HAVE_REPO_PULL_FLAGS_COMMIT_AND_SIZE_ONLY
#define KM_HAS_SIZE_PULL 1
#else
#define KM_HAS_SIZE_PULL 0
#endif

static void progress_change(OstreeAsyncProgress *progress, gpointer userData)
{
    KMOSTreeHandler *manager = (KMOSTreeHandler *)userData;
    if (!manager) {
        return;
    }

    if (manager->isCancelled()) {
        kmlogger.debug("progress_change: operation cancelled, ignoring progress update");
        return;
    }

    // 使用实例特定的进度状态（支持并发安装）
    ProgressStatus &progressStatus = manager->getProgressStatus();
    std::deque<std::string> &queuePull = manager->getQueuePull();

    bool downloading_extra_data, caught_error;

    ostree_async_progress_get(progress,
                              "transferred-extra-data-bytes", "t", &progressStatus.transferredExtraDataBytes,
                              "total-extra-data-bytes", "t", &progressStatus.totalExtraDataBytes,
                              "status", "s", &progressStatus.ostreeStatus,
                              "start-time", "t", &progressStatus.startTime,
                              "bytes-transferred", "t", &progressStatus.bytesTransferred,
                              "fetched-delta-part-size", "t", &progressStatus.fetchedDeltaPartSize,
                              "total-delta-part-size", "t", &progressStatus.totalDeltaPartSize,
                              "fetched", "u", &progressStatus.fetched,
                              "requested", "u", &progressStatus.requested,
                              "outstanding-metadata-fetches", "u", &progressStatus.outstandingMetadataFetches,
                              "metadata-fetched", "u", &progressStatus.metadataFetched,
                              "total-delta-parts", "u", &progressStatus.totalDeltaParts,
                              "outstanding-writes", "u", &progressStatus.outstandingWrites,
                              //   "downloading-extra-data", "u", downloading_extra_data, // 存在段错误
                              nullptr);

    if (!queuePull.empty())
    {
        progressStatus.ref = queuePull.front();
    }

    kmlogger.info("progressStatus ==>  status : %s  ref: %s queuePullSize: %zu requested: %u fetched: %u outstandingWrites: %u",
                  progressStatus.ostreeStatus != NULL ? progressStatus.ostreeStatus : "",
                  progressStatus.ref.c_str(),
                  queuePull.size(),
                  progressStatus.requested,
                  progressStatus.fetched,
                  progressStatus.outstandingWrites);

    manager->updateProgress();

    // 当有多个 ref 时，完成一个后移除
    if (queuePull.size() > 1 && ((progressStatus.requested > 0 && progressStatus.fetched == progressStatus.requested) || (progressStatus.ostreeStatus && *progressStatus.ostreeStatus)))
    {
        kmlogger.info("开始 pop front for ref %s , ", progressStatus.ref.c_str());
        queuePull.pop_front();
    }
}

KMOSTreeHandler::KMOSTreeHandler(const string &path)
    : d(std::make_unique<Private>())
{
    g_autoptr(GFile) pathFile = g_file_new_for_path(path.data());
    d->m_ostreeRepo = ostree_repo_new(pathFile);
    d->m_repoPath = path;
    d->m_cancellable = g_cancellable_new();

    if(!ostree_repo_open(d->m_ostreeRepo, nullptr, nullptr))
    {
        KMFolder foder;
        foder.ensureRepoForSudo(false);
        d->m_ostreeRepo = ostree_repo_new(pathFile);
        ostree_repo_open(d->m_ostreeRepo, nullptr, nullptr);
    }

    d->m_initialized = true;
    kmlogger.info("init ostree handler successful!");
}

KMOSTreeHandler::~KMOSTreeHandler()
{
    if (d->m_pullProgress) {
        g_signal_handlers_disconnect_by_data(d->m_pullProgress, this);
        g_object_unref(d->m_pullProgress);
        d->m_pullProgress = nullptr;
    }

    if (d->m_cancellable) {
        g_cancellable_cancel(d->m_cancellable);
        g_object_unref(d->m_cancellable);
        d->m_cancellable = nullptr;
    }

    if (d->m_ostreeRepo) {
        g_object_unref(d->m_ostreeRepo);
        d->m_ostreeRepo = nullptr;
    }
}

void KMOSTreeHandler::cancel() 
{
    if (d->m_cancellable && !g_cancellable_is_cancelled(d->m_cancellable)) 
    {
        g_cancellable_cancel(d->m_cancellable);
        kmlogger.info("OSTree pull operation cancelled");
        d->m_cancelled = true;
        // memset(&progressStatus, 0, sizeof(progressStatus));
    }
}

bool KMOSTreeHandler::isCancelled() const 
{ 
    return d->m_cancelled; 
}

bool KMOSTreeHandler::resetCancelled() const 
{ 
    d->m_cancelled = false;
    return d->m_cancelled; 
}

string KMOSTreeHandler::readLatestCommit(const string &refCompletedId)
{
    char *res = NULL;
    // string refSpec = d->m_repoPath.append(":").append(refCompletedId);
    string refSpec = refCompletedId;
    replaceRef(refSpec);

    if (!ostree_repo_resolve_rev(d->m_ostreeRepo, refSpec.c_str(), false, &res, &d->m_error))
    {
        kmlogger.error("Failed to resolve ref %s: %s", refSpec.c_str(), d->m_error->message);
        return "";
    }

    string outRes = string(res);
    g_free(res);
    return outRes;
}

std::string KMOSTreeHandler::getRemoteUrl(const string &remoteName)
{
    if (!d->m_ostreeRepo || remoteName.empty())
    {
        return "-";
    }

    char *tmp_url = nullptr;
    if (!ostree_repo_remote_get_url(d->m_ostreeRepo, remoteName.c_str(), &tmp_url, &d->m_error))
    {
        return "-";
    }

    std::string cached_url(tmp_url);
    g_free(tmp_url);

    std::string configPath = "/opt/kaiming/repo/config";
    std::string local_url = getLocalRemoteUrl(configPath, remoteName);

    if (local_url != cached_url && local_url != "-")
    {
        return local_url;
    }

    return cached_url;
}

std::string KMOSTreeHandler::getLocalRemoteUrl(const std::string &configPath, const std::string &remoteName)
{
    GKeyFile *keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, configPath.c_str(), G_KEY_FILE_NONE, nullptr))
    {
        g_key_file_free(keyfile);
        return "-";
    }

    std::string group = "remote \"" + remoteName + "\"";
    gchar *url = g_key_file_get_string(keyfile, group.c_str(), "url", nullptr);
    g_key_file_free(keyfile);

    if (!url)
    {
        return "-";
    }

    std::string result(url);
    g_free(url);
    return result;
}

void KMOSTreeHandler::replaceRef(std::string& ref) {
    size_t pos = 0;
    while ((pos = ref.find('+', pos)) != std::string::npos) {
        ref.replace(pos, 1, "--");
        pos += 2;
    }
}

bool KMOSTreeHandler::pull(const string &remoteName, vector<string> vecRefs, CallBack cb, gpointer userData, unsigned int maxSpeedLimitKilobytes, vector<string> localcacheRepos) {
    kmlogger.info("KMOSTreeHandler::pull called: remote=%s, refs count=%zu", remoteName.c_str(), vecRefs.size());

    initProgress();

    d->m_progressCallback = cb;
    d->m_progressStatus.lastRef = vecRefs.empty() ? "" : vecRefs.back();
    // 传入KMPackageManager的this
    d->m_userData = userData;

    d->m_totalDownloadSize = 0;
    d->m_completedBytes = 0;
    d->m_refSizeMap.clear();

    // 清空队列，准备新的 pull 操作
    d->m_queuePull.clear();

    if (d->m_cancellable)
    {   
        g_object_unref(d->m_cancellable);
    }

    d->m_cancellable = g_cancellable_new();

    for (auto &ref : vecRefs)
    {
        guint64 downloadSize = 0;
        guint64 installSize = 0;

#if KM_HAS_SIZE_PULL
        char *refs[2] = { (char *)ref.c_str(), nullptr };
        bool ostreePullRes = ostree_repo_pull(
            d->m_ostreeRepo,
            remoteName.c_str(),
            refs,
            OSTREE_REPO_PULL_FLAGS_COMMIT_AND_SIZE_ONLY,
            nullptr,
            nullptr,
            &d->m_error);

        readDetachedSizeInfo(ref, downloadSize, installSize);
#endif

        d->m_refSizeMap[ref] = downloadSize;

        if (downloadSize > 0)
            d->m_totalDownloadSize += downloadSize;
    }

    if (d->m_totalDownloadSize > 0)
        d->m_sizeMode = true;
    else
        d->m_sizeMode = false;

    // test data
    // d->m_totalDownloadSize = 1169326557;
    // d->m_refSizeMap["stable/x86_64/app/org.videolan.vlc/binary/3.0.9.2-1kylin0.1.esm3k0.1"] = 32589714;
    // d->m_refSizeMap["stable/x86_64/base/cn.kylin.v10sp1-base/binary/1.0.8"] = 1136736843;
    // d->m_sizeMode = true;

    kmlogger.info("Starting pull loop for %zu refs", vecRefs.size());

    for (auto &ref : vecRefs)
    {
        replaceRef(ref);

        d->m_currentRef = ref;
        d->m_queuePull.push_back(ref);
        //char *refs[2] = { (char *)ref.c_str(), nullptr };
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

        GVariantBuilder refs_builder;
        g_variant_builder_init(&refs_builder, G_VARIANT_TYPE("as"));
        g_variant_builder_add(&refs_builder, "s", ref.c_str());
        g_variant_builder_add(&builder, "{sv}", "refs", g_variant_builder_end(&refs_builder));

        if (maxSpeedLimitKilobytes > 0)
        {
            g_variant_builder_add(&builder, "{sv}", "max-speed-limit-kilobyte", g_variant_new_uint32(maxSpeedLimitKilobytes));
        }

        if (!localcacheRepos.empty())
        {
            GVariantBuilder paths_builder;
            g_variant_builder_init(&paths_builder, G_VARIANT_TYPE("as"));

            for (const auto &path : localcacheRepos)
            {
                g_variant_builder_add(&paths_builder, "s", path.c_str());
                kmlogger.debug("Adding localcache-repo: %s", path.c_str());
            }

            g_variant_builder_add(&builder, "{sv}", "localcache-repos", g_variant_builder_end(&paths_builder));
        }

        GVariant* options = g_variant_builder_end(&builder);
        g_clear_error(&d->m_error);

        // 检查是否已被取消
        if (d->m_cancellable && g_cancellable_is_cancelled(d->m_cancellable)) {
            kmlogger.info("Pull operation cancelled before starting for ref: %s", ref.c_str());
            d->m_queuePull.pop_front();
            return false;
        }

        guint64 beforeBytes = ostree_async_progress_get_uint64(d->m_pullProgress, "bytes-transferred");

        kmlogger.info("Starting pull for ref: %s from remote: %s", ref.c_str(), remoteName.c_str());
        // bool ostreePullRes = ostree_repo_pull(d->m_ostreeRepo, remoteName.c_str(), refs, OSTREE_REPO_PULL_FLAGS_NONE, d->m_pullProgress, nullptr, &d->m_error);
        bool ostreePullRes = ostree_repo_pull_with_options(d->m_ostreeRepo, remoteName.c_str(), options, d->m_pullProgress, d->m_cancellable, &d->m_error);

        guint64 afterBytes = ostree_async_progress_get_uint64(d->m_pullProgress, "bytes-transferred");

        g_variant_unref(options);

        kmlogger.info("ostree_repo_pull_with_options returned: %s for ref: %s", ostreePullRes ? "true" : "false", ref.c_str());

        if (!ostreePullRes)
        {
            if (d->m_error != nullptr)
            {
                std::cerr << "ostree pull failed: " << d->m_error->message << std::endl;
                kmlogger.error("ostree pull failed for ref %s: %s", ref.c_str(), d->m_error->message);

                g_error_free(d->m_error);
                d->m_error = nullptr;
            }
            else
            {
                std::cerr << "ostree pull failed with unknown error" << std::endl;
                kmlogger.error("ostree pull failed with unknown error for ref: %s", ref.c_str());
            }

            d->m_queuePull.pop_front();
            if (isCancelled())
            {
                resetCancelled();
            }
            return false; 
        }

        d->m_completedBytes += (afterBytes - beforeBytes);
        kmlogger.info("Successfully pulled ref: %s", ref.c_str());
    }

    ostree_async_progress_finish(d->m_pullProgress);
    d->m_queuePull.clear();

    return true;
}

bool KMOSTreeHandler::deleteRefTag(const string &remoteName, const string &ref, const string &commit)
{
    GError *err = nullptr;
    string strRef = ref;

    replaceRef(strRef);

    if (!ostree_repo_set_ref_immediate(d->m_ostreeRepo, remoteName != "" ? remoteName.c_str() : nullptr, strRef.c_str(), NULL, NULL, &err))
    {
        kmlogger.error("delete repo ref %s error (%s)", ref.c_str(), err->message);
        return false;
    }
    kmlogger.info("delete ref %s in remote %s", ref.c_str(), remoteName.c_str());
    return true;
}

bool KMOSTreeHandler::repoPrune()
{
    GError *err = nullptr;
    gint objectsTotal, objectsPruned;
    guint64 prunedObjectSizeTotal;
    if (!ostree_repo_prune(d->m_ostreeRepo, OSTREE_REPO_PRUNE_FLAGS_REFS_ONLY, 0, &objectsTotal, &objectsPruned, &prunedObjectSizeTotal, NULL, &err))
    {
        if (err != nullptr)
        {
            kmlogger.error("prune repo error: %s (code: %d, domain: %s)", err->message, err->code, g_quark_to_string(err->domain));
            g_error_free(err);
        }
        else
        {
            kmlogger.error("prune repo error: unknown error");
        }
        return false;
    }
    return true;
}

ProgressStatus& KMOSTreeHandler::getProgressStatus()
{
    return d->m_progressStatus;
}

std::deque<std::string> &KMOSTreeHandler::getQueuePull()
{
    return d->m_queuePull;
}

bool KMOSTreeHandler::getSizeMode() const { return d->m_sizeMode; }
guint64 KMOSTreeHandler::getTotalDownloadSize() const { return d->m_totalDownloadSize; }
guint64 KMOSTreeHandler::getCompletedBytes() const { return d->m_completedBytes; }
const std::unordered_map<std::string, guint64> &KMOSTreeHandler::getRefSizeMap() const { return d->m_refSizeMap; }

void KMOSTreeHandler::updateProgress()
{
    if (d->m_cancelled)
    {
        // ostree pull取消后不再返回进度
        return;
    }
    if (d->m_progressCallback.operator bool())
    {
        // 使用实例特定的进度状态（支持并发安装）
        d->m_progressCallback(d->m_progressStatus, d->m_userData);
    }
}

bool KMOSTreeHandler::checkout(const string &commit, const string &tmpTargetPath, const string &subPath)
{
    OstreeRepoCheckoutAtOptions options = {};
    options.mode = OSTREE_REPO_CHECKOUT_MODE_NONE;
    options.overwrite_mode = OSTREE_REPO_CHECKOUT_OVERWRITE_UNION_FILES;
    options.enable_fsync = FALSE;
    options.bareuseronly_dirs = FALSE;
    if (subPath.length() > 0)
    {
        options.subpath = subPath.c_str();
    }
    g_clear_error(&d->m_error);

    // 获取指定的 commit 对象
    // g_autoptr(GVariant) commitVariant = nullptr;
    // commitVariant = getCommitData(commit)

    bool checkoutRes = ostree_repo_checkout_at(d->m_ostreeRepo, &options, AT_FDCWD, tmpTargetPath.c_str(), commit.c_str(), nullptr, &d->m_error);
    if (!checkoutRes && d->m_error)
    {
        kmlogger.error(d->m_error->message);
        return false;
    }
    return true;
}

/**
 * @brief 创建 deploy 时的ostree refs
 *
 * ostree refs 查看： `ostree refs --repo repo`
 *      deploy/app/org.kde.kclock/x86_64/master
 *      kaiming-repo:app/org.kde.kclock/x86_64/master
 *
 * @param completedRef 完整的ref，如 app/org.kde.kclock/x86_64/master
 * @param commit ref对应的commit, 如 f3b1314b51f2b8865089f13cbae5b7cd449b169f0b36530d27da74beb68aa492
 * @return true
 * @return false
 */
bool KMOSTreeHandler::updateDeployRef(const string &completedRef, const string &commit)
{
    string deployRef = string("deploy/").append(completedRef);
    replaceRef(deployRef);

    if (!ostree_repo_set_ref_immediate(d->m_ostreeRepo, nullptr, deployRef.c_str(), commit.c_str(), nullptr, nullptr))
    {
        return false;
    }

    return true;
}

void KMOSTreeHandler::initProgress()
{
    d->m_pullProgress = ostree_async_progress_new();

    // 初始化实例特定的进度状态
    memset(&d->m_progressStatus, 0, sizeof(ProgressStatus));
    d->m_queuePull.clear();

    ostree_async_progress_set(d->m_pullProgress,
                              "transferred-extra-data-bytes", "t", 0,
                              "total-extra-data-bytes", "t", 0,
                              "status", "s", "",
                              "start-time", "t", 0,
                              "bytes-transferred", "t", 0,
                              "fetched-delta-part-size", "t", 0,
                              "total-delta-part-size", "t", 0,
                              "fetched", "u", 0,
                              "requested", "u", 0,
                              "outstanding-metadata-fetches", "u", 0,
                              "metadata-fetched", "u", 0,
                              "total-delta-parts", "u", 0,
                              "outstanding-writes", "u", 0,
                              nullptr);

    g_signal_connect(d->m_pullProgress, "changed", G_CALLBACK(progress_change), (gpointer)this);
}

map<string, KMRef> KMOSTreeHandler::getLocalRefs()
{
    map<string, KMRef> mapRef;

    g_autoptr(GHashTable) table = NULL;
    g_clear_error(&d->m_error);
    if (!ostree_repo_list_refs(d->m_ostreeRepo, nullptr, &table, nullptr, &d->m_error))
    {
        return mapRef;
    }

    if (!table)
    {
        return mapRef;
    }

    gpointer key, value;
    GHashTableIter iter;
    g_hash_table_iter_init(&iter, table);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        KMRef ref;
        char *refspec = (char *)key;
        char *checksum = (char *)value;

        // 统一格式：所有 ref 都应该是 remote:ref 格式（例如 kaiming-repo:stable/x86_64/app/.../binary/...）
        list<string> listStr = KMUtil::split(refspec, ':');
        string refPath; // 实际的 ref 路径（去掉 remote 前缀后的部分）

        if (listStr.size() == 2)
        {
            // 格式：remote:ref，取 ref 部分
            refPath = listStr.back();
        }
        else if (listStr.size() == 1)
        {
            // 兼容旧数据：如果还有不带前缀的本地 ref（可能是旧版本安装的），也处理
            refPath = listStr.front();
        }
        else
        {
            // 不支持的格式，跳过
            continue;
        }

        // 跳过 deploy/ 前缀的 ref（这些是部署 ref，不是我们要找的）
        if (refPath.find("deploy/") == 0)
        {
            continue;
        }

        list<string> refDescrible = KMUtil::split(refPath, '/');

        if (refDescrible.size() != 6)//  原为4
        {
            continue;
        }
        ref.branch = refDescrible.front();
        refDescrible.pop_front();

        ref.arch = refDescrible.front();
        refDescrible.pop_front();
       
        ref.kind = refDescrible.front();
        refDescrible.pop_front();

        ref.id = refDescrible.front();
        refDescrible.pop_front();

        ref.module = refDescrible.front();
        refDescrible.pop_front();
 
        ref.version = refDescrible.front();
        refDescrible.pop_front();

        mapRef[checksum] = ref;
        g_hash_table_iter_steal(&iter);
    }
    return mapRef;
}

map<string, map<string, KMRef>> KMOSTreeHandler::getAllLocalRefs()
{
    map<string, map<string, KMRef>> allRefs;
    auto remotes = getAllRemotesWithPriority();
    for (const auto& remote : remotes) {
        allRefs[remote.name] = getLocalRefs();
    }

    //allRefs["local"] = getLocalRefs("");
    return allRefs;
}

std::vector<RemoteInfo> KMOSTreeHandler::getAllRemotesWithPriority() {
    std::vector<RemoteInfo> remotes;
    GKeyFile *keyfile = g_key_file_new();
    std::string configPath = "/opt/kaiming/repo/config";
    if (!g_key_file_load_from_file(keyfile, configPath.c_str(), G_KEY_FILE_NONE, nullptr)) {
        g_key_file_free(keyfile);
        return remotes;
    }
    gsize len = 0;
    gchar **groups = g_key_file_get_groups(keyfile, &len);
    for (gsize i = 0; i < len; ++i) {
        std::string group(groups[i]);
        if (group.find("remote \"") == 0) {
            RemoteInfo info;
            info.name = group.substr(8, group.length() - 9); // remote "xxx"
            gchar *url = g_key_file_get_string(keyfile, group.c_str(), "url", nullptr);
            info.url = url ? url : "-";
            g_free(url);
            info.priority = g_key_file_get_integer(keyfile, group.c_str(), "priority", nullptr);
            remotes.push_back(info);
        }
    }
    g_strfreev(groups);
    g_key_file_free(keyfile);
    std::sort(remotes.begin(), remotes.end(), [](const RemoteInfo &a, const RemoteInfo &b) {
        return a.priority < b.priority;
    });
    return remotes;
}


// 支持 remote fallback 的 pull
bool KMOSTreeHandler::pullWithFallback(const std::vector<std::string>& refs, CallBack cb, gpointer userData, unsigned int maxSpeedLimitKilobytes) {
    auto remotes = getAllRemotesWithPriority();
    for (const auto& ref : refs) {
        bool found = false;
        for (const auto& remote : remotes) {
            if (pull(remote.name, {ref}, cb, userData, maxSpeedLimitKilobytes)) {
                found = true;
                break;
            }
        }
        if (!found) {
            kmlogger.error("Failed to pull ref %s from all remotes", ref.c_str());
            return false;
        }
    }
    return true;
}

// 从本地临时 repo 合并到主 repo
// 使用 ostree pull-local 命令实现（没有 ostree_repo_pull_local API）
bool KMOSTreeHandler::pullLocal(const std::string &tmpRepoPath, const std::vector<std::string> &vecRefs, CallBack cb, gpointer userData, const std::map<std::string, std::vector<std::string>> *repoRefMap)
{
    kmlogger.info("Pulling from local temp repo: %s to main repo: %s", tmpRepoPath.c_str(), d->m_repoPath.c_str());

    if (!fs::exists(tmpRepoPath)) {
        kmlogger.error("Temp repo path does not exist: %s", tmpRepoPath.c_str());
        return false;
    }

    // 在临时仓库里“清洗”带 remote 前缀的 ref（例如 kaiming-repo:stable/...）
    // 只保留冒号后的真实 ref 名，避免后续 pull-local 报Invalid ref name kaiming-repo:stable/...
    {
        g_autoptr(GFile) tmpRepoFile = g_file_new_for_path(tmpRepoPath.c_str());
        g_autoptr(OstreeRepo) tmpRepo = ostree_repo_new(tmpRepoFile);
        g_autoptr(GError) error = nullptr;

        if (!ostree_repo_open(tmpRepo, nullptr, &error)) {
            kmlogger.error("Failed to open temp repo %s for ref sanitize: %s", tmpRepoPath.c_str(), error ? error->message : "unknown");
            g_clear_error(&error);
        } else {
            GHashTable *refs = nullptr;
            if (!ostree_repo_list_refs(tmpRepo, nullptr, &refs, nullptr, &error)) {
                kmlogger.error("Failed to list refs in temp repo %s: %s", tmpRepoPath.c_str(), error ? error->message : "unknown");
                g_clear_error(&error);
            } else {
                GHashTableIter iter;
                gpointer keyPtr, valuePtr;
                g_hash_table_iter_init(&iter, refs);
                while (g_hash_table_iter_next(&iter, &keyPtr, &valuePtr)) {
                    const char *refName = static_cast<const char *>(keyPtr);
                    const char *commit = static_cast<const char *>(valuePtr);
                    if (!refName || !commit) {
                        continue;
                    }

                    const char *colon = strchr(refName, ':');
                    if (!colon) {
                        continue; // 没有 remote: 前缀的 ref 保持不变
                    }

                    // remote-style 名称：REMOTE:REF
                    std::string remoteName(refName, colon - refName);
                    std::string localRef(colon + 1);
                    std::string newRef = localRef;

                    if (newRef.empty()) {
                        kmlogger.warn("Skip empty sanitized ref converted from %s", refName);
                        continue;
                    }

                    kmlogger.info("Sanitize temp repo ref: %s -> %s (commit %s)", refName, newRef.c_str(), commit);
                    g_autoptr(GError) setErr = nullptr;

                    // 写入到本地仓库时使用不带前缀的ref
                    if (!ostree_repo_set_ref_immediate(tmpRepo, nullptr, newRef.c_str(), commit, nullptr, &setErr)) {
                        kmlogger.error("Failed to set sanitized ref %s in temp repo: %s", newRef.c_str(), setErr ? setErr->message : "unknown");
                        continue;
                    }

                    // 删除旧的带 remote 前缀的远程 ref（remote = remoteName，ref = localRef，commit 传 NULL 即删除）
                    g_autoptr(GError) delErr = nullptr;
                    if (!ostree_repo_set_ref_immediate(tmpRepo, remoteName.c_str(), localRef.c_str(), nullptr, nullptr, &delErr)) {
                        kmlogger.error("Failed to delete old remote-style ref %s (remote=%s, ref=%s) in temp repo: %s",
                                      refName, remoteName.c_str(), localRef.c_str(), delErr ? delErr->message : "unknown");
                    }
                }
                g_hash_table_unref(refs);
            }
        }
    }

    // 这里不再根据 vecRefs 去解析临时仓库里的 ref，
    // 而是直接让 ostree 把该临时仓库里的所有 refs 合并到主仓库。
    std::string command = "ostree --repo=" + d->m_repoPath.string() + " pull-local " + tmpRepoPath + " 2>&1";
    kmlogger.debug("Executing pull-local command: %s", command.c_str());

    FILE *fp = popen(command.c_str(), "r");
    if (!fp) {
        kmlogger.error("Failed to execute pull-local command");
        return false;
    }

    char buffer[4096];
    std::string output;
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        output += buffer;
    }

    int ret = pclose(fp);
    if (ret != 0) {
        kmlogger.error("pull-local command failed, return code: %d", ret);
        if (!output.empty()) {
            kmlogger.error("pull-local error output: %s", output.c_str());
        }
        return false;
    }

    if (!output.empty()) {
        kmlogger.debug("pull-local output: %s", output.c_str());
    }

    // 统一 ref 命名：将 pull-local 后产生的本地 ref（不带 remote 前缀）
    // 转换为带 remote 前缀的格式（与命令行安装保持一致）
    // 例如：stable/x86_64/app/... -> kaiming-repo:stable/x86_64/app/...
    {
        g_autoptr(GError) error = nullptr;
        GHashTable *refs = nullptr;

        // 获取主 repo 中所有不带 remote 前缀的本地 ref
        if (!ostree_repo_list_refs(d->m_ostreeRepo, nullptr, &refs, nullptr, &error)) {
            kmlogger.warn("Failed to list refs in main repo for ref name unification: %s", error ? error->message : "unknown");
            g_clear_error(&error);
        } else if (refs) {
            // 创建从 ref 到 remote 的反向映射（如果提供了 repoRefMap）
            std::map<std::string, std::string> refToRemoteMap;
            if (repoRefMap) {
                for (const auto &[remote, refList] : *repoRefMap) {
                    for (const auto &ref : refList) {
                        refToRemoteMap[ref] = remote;
                    }
                }
            }

            // 如果没有提供 repoRefMap，使用默认 remote
            auto remotes = getAllRemotesWithPriority();
            std::string defaultRemote = remotes.empty() ? "kaiming-repo" : remotes[0].name;

            GHashTableIter iter;
            gpointer keyPtr, valuePtr;
            g_hash_table_iter_init(&iter, refs);
            while (g_hash_table_iter_next(&iter, &keyPtr, &valuePtr)) {
                const char *refName = static_cast<const char *>(keyPtr);
                const char *commit = static_cast<const char *>(valuePtr);

                if (!refName || !commit) {
                    continue;
                }

                // 跳过已经带 remote 前缀的 ref（格式：remote:ref）
                if (strchr(refName, ':') != nullptr) {
                    continue;
                }

                // 跳过 deploy/ 前缀的 ref
                if (strncmp(refName, "deploy/", 7) == 0) {
                    continue;
                }

                // 根据 repoRefMap 确定该 ref 对应的 remote
                std::string targetRemote = defaultRemote;
                if (repoRefMap) {
                    auto it = refToRemoteMap.find(refName);
                    if (it != refToRemoteMap.end()) {
                        targetRemote = it->second;
                    } else {
                        // 如果 ref 不在 repoRefMap 中，使用默认 remote
                        kmlogger.debug("Ref %s not found in repoRefMap, using default remote %s", refName, defaultRemote.c_str());
                    }
                }

                // 将本地 ref 转换为带 remote 前缀的格式
                kmlogger.info("Unifying ref name: %s -> %s:%s (commit %s)", refName, targetRemote.c_str(), refName, commit);

                g_autoptr(GError) setErr = nullptr;
                // 设置带 remote 前缀的 ref
                if (!ostree_repo_set_ref_immediate(d->m_ostreeRepo, targetRemote.c_str(), refName, commit, nullptr, &setErr)) {
                    kmlogger.warn("Failed to set remote-style ref %s:%s: %s", targetRemote.c_str(), refName, setErr ? setErr->message : "unknown");
                    continue;
                }

                // 删除原来的本地 ref（不带 remote 前缀）
                g_autoptr(GError) delErr = nullptr;
                if (!ostree_repo_set_ref_immediate(d->m_ostreeRepo, nullptr, refName, nullptr, nullptr, &delErr)) {
                    kmlogger.warn("Failed to delete local ref %s: %s", refName, delErr ? delErr->message : "unknown");
                }
            }
            g_hash_table_unref(refs);
        }
    }

    kmlogger.info("Successfully merged temp repo %s to main repo %s", tmpRepoPath.c_str(), d->m_repoPath.c_str());
    return true;
}

// std::string KMOSTreeHandler::getBranchTimestamp(std::string& targetBranch) {
//     g_autoptr(GError) error = nullptr;

//     replaceRef(targetBranch); // '+' -> '--'

//     char *refs[2] = { const_cast<char*>(targetBranch.c_str()), nullptr };
//     if (!ostree_repo_pull(d->m_ostreeRepo, REPO_NAME, refs, OSTREE_REPO_PULL_FLAGS_COMMIT_ONLY, nullptr, nullptr, &error))
//     {
//         // g_printerr("Pull failed: %s\n", error->message);
//         kmlogger.error("Pull failed: %s\n", error->message);
//         return "";
//     }

//     g_autofree gchar *commit_id = nullptr;
//     if (!ostree_repo_resolve_rev(d->m_ostreeRepo, targetBranch.c_str(), FALSE, &commit_id, &error)) {
//         kmlogger.error("Failed to resolve branch %s: %s\n", targetBranch.c_str(), error->message);
//         return "";
//     }

//     g_autoptr(GVariant) commit_variant = nullptr;
//     if (!ostree_repo_load_commit(d->m_ostreeRepo, commit_id, &commit_variant, nullptr, &error)) {
//         kmlogger.error("Failed to load commit %s: %s\n", commit_id, error->message);
//         return "";
//     }

//     gint64 timestamp = ostree_commit_get_timestamp(commit_variant);

//     char buf[64] = { 0 };
//     time_t t = timestamp;
//     struct tm tm;
//     gmtime_r(&t, &tm);
//     strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
//     return std::string(buf);
// }

// std::string KMOSTreeHandler::getBranchTimestamp(std::string& targetBranch) {
//     g_autoptr(GError) error = nullptr;

//     replaceRef(targetBranch); // '+' -> '--'

//     // 按优先级尝试所有远程仓库
//     auto remotes = getAllRemotesWithPriority();
//     for (const auto& remote : remotes) {
//         char *refs[2] = { const_cast<char*>(targetBranch.c_str()), nullptr };
//         if (!ostree_repo_pull(d->m_ostreeRepo, remote.name.c_str(), refs, OSTREE_REPO_PULL_FLAGS_COMMIT_ONLY, nullptr, nullptr, &error))
//         {
//             kmlogger.debug("Pull from remote %s failed: %s", remote.name.c_str(), error->message);
//             g_clear_error(&error);
//             continue;
//         }

//         g_autofree gchar *commit_id = nullptr;
//         if (!ostree_repo_resolve_rev(d->m_ostreeRepo, targetBranch.c_str(), FALSE, &commit_id, &error))
//         {
//             kmlogger.debug("Failed to resolve branch %s from remote %s: %s", targetBranch.c_str(), remote.name.c_str(), error->message);
//             g_clear_error(&error);
//             continue;
//         }

//         g_autoptr(GVariant) commit_variant = nullptr;
//         if (!ostree_repo_load_commit(d->m_ostreeRepo, commit_id, &commit_variant, nullptr, &error))
//         {
//             kmlogger.debug("Failed to load commit %s from remote %s: %s", commit_id, remote.name.c_str(), error->message);
//             g_clear_error(&error);
//             continue;
//         }

//         gint64 timestamp = ostree_commit_get_timestamp(commit_variant);

//         char buf[64] = {0};
//         time_t t = timestamp;
//         struct tm tm;
//         gmtime_r(&t, &tm);
//         strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
//         kmlogger.debug("Successfully got timestamp from remote %s: %s", remote.name.c_str(), buf);
//         return std::string(buf);
//     }

//     kmlogger.error("Failed to get branch timestamp from all remotes for branch: %s", targetBranch.c_str());
//     return "";
// }

std::string KMOSTreeHandler::getBranchTimestamp(const char *target_branch)
{
    g_autoptr(GError) error = nullptr;
    g_autoptr(GBytes) summaryBytes = nullptr;
    g_autoptr(GBytes) sigBytes = nullptr;

    // 按优先级尝试所有远程仓库
    auto remotes = getAllRemotesWithPriority();
    for (const auto &remote : remotes)
    {
        if (!ostree_repo_remote_fetch_summary(d->m_ostreeRepo, remote.name.c_str(), &summaryBytes, &sigBytes, NULL, &error))
        {
            kmlogger.debug("Failed to fetch summary %s failed: %s", remote.name.c_str(), error->message);
            g_printerr("Failed to fetch summary: %s\n", error->message);
            g_clear_error(&error);
            continue;
        }

        g_autofree char *summaryPath = g_strdup("/opt/kaiming/repo/summary");
        FILE *fp = fopen(summaryPath, "wb");
        if (!fp)
        {
            kmlogger.debug("Failed to create summary file %s from remote %s: %s", target_branch, remote.name.c_str(), error->message);
            g_printerr("Failed to create summary file\n");
            continue;
        }

        fwrite(g_bytes_get_data(summaryBytes, NULL), 1, g_bytes_get_size(summaryBytes), fp);
        fclose(fp);

        std::string cmd = "ostree summary --view --repo=" + std::string("/opt/kaiming/repo") + " | grep -A 5 '* " + target_branch + "' | grep 'Timestamp'";

        std::array<char, 128> buffer;
        std::string rawTimestamp;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
        {
            continue;
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()))
        {
            rawTimestamp += buffer.data();
        }

        std::string timestamp;
        const char *p = rawTimestamp.c_str();

        while (*p && isspace(*p))
            p++;

        if (strncmp(p, "Timestamp", 9) == 0)
        {
            p += 9;
            while (*p && *p != ':')
                p++;
            if (*p == ':')
            {
                p++;
                while (*p && isspace(*p))
                    p++;
                if (strlen(p) >= 20)
                {
                    char clean[20];
                    memcpy(clean, p, 10);
                    clean[10] = ' ';
                    memcpy(clean + 11, p + 11, 8);
                    timestamp.assign(clean, 19);
                    return timestamp;
                }
            }
        }

        kmlogger.debug("No timestamp found for branch %s in remote %s", target_branch, remote.name.c_str());
    }

    kmlogger.error("Failed to get branch timestamp from all remotes for branch: %s", target_branch);
    return "";
}

void KMOSTreeHandler::refreshRemoteUrl()
{
    if (!d->m_initialized) return;

    kmlogger.info("Refreshing remote URL configuration");

    // 重新加载 repo 以读取最新配置
    reloadRepo();
}

void KMOSTreeHandler::reloadRepo()
{
    g_clear_error(&d->m_error);

    std::string repoPath = d->m_repoPath;

    if (d->m_ostreeRepo) {
        g_object_unref(d->m_ostreeRepo);
        d->m_ostreeRepo = nullptr;
    }

    // 重新创建并打开 repo
    g_autoptr(GFile) pathFile = g_file_new_for_path(repoPath.c_str());
    d->m_ostreeRepo = ostree_repo_new(pathFile);

    if (!ostree_repo_open(d->m_ostreeRepo, nullptr, &d->m_error)) {
        kmlogger.error("Failed to reopen ostree repo: %s", d->m_error->message);

        // 尝试重新创建 repo 目录
        KMFolder folder;
        folder.ensureRepoForSudo(false);

        // 再次尝试打开
        d->m_ostreeRepo = ostree_repo_new(pathFile);
        if (!ostree_repo_open(d->m_ostreeRepo, nullptr, &d->m_error)) {
            kmlogger.error("Failed to reopen ostree repo after recovery attempt: %s", d->m_error->message);
            g_clear_error(&d->m_error);
            return;
        }
    }

    d->m_initialized = true;
    kmlogger.info("Successfully reloaded ostree repo with updated configuration");
}

std::string KMOSTreeHandler::getCurrentRemoteUrl(const std::string& remoteName)
{
    // 总是从配置文件读取最新的 URL
    std::string configPath = "/opt/kaiming/repo/config";
    std::string latestUrl = getLocalRemoteUrl(configPath, remoteName);

    if (!latestUrl.empty() && latestUrl != "-") {
        return latestUrl;
    }

    // 如果配置文件读取失败，回退到从 repo 读取
    if (!d->m_ostreeRepo || remoteName.empty()) {
        return "-";
    }

    char *tmp_url = nullptr;
    if (!ostree_repo_remote_get_url(d->m_ostreeRepo, remoteName.c_str(), &tmp_url, &d->m_error)) {
        kmlogger.warn("Failed to get remote URL from repo: %s", d->m_error->message);
        g_clear_error(&d->m_error);
        return "-";
    }

    std::string cached_url(tmp_url);
    g_free(tmp_url);
    return cached_url;
}

bool KMOSTreeHandler::readDetachedSizeInfo(const std::string &ref, guint64 &downloadSize, guint64 &installSize)
{
    downloadSize = 0;
    installSize  = 0;

    char *commit = nullptr;

    if (!ostree_repo_resolve_rev(d->m_ostreeRepo, ref.c_str(), FALSE, &commit, nullptr))
        return false;

    GVariant *metadata = nullptr;

    if (!ostree_repo_read_commit_detached_metadata(d->m_ostreeRepo, commit, &metadata, nullptr, nullptr))
    {
        g_free(commit);
        return false;
    }

    g_free(commit);

    if (!metadata)
        return false;

    GVariant *value;

    value = g_variant_lookup_value(metadata, "download-size", G_VARIANT_TYPE_STRING);
    if (value)
    {
        const char *size_str = g_variant_get_string(value, NULL);
        downloadSize = g_ascii_strtoull(size_str, NULL, 10);
        g_print("download-size (string): %s -> %" G_GUINT64_FORMAT "\n", size_str, downloadSize);
        g_variant_unref(value);
    }

    value = g_variant_lookup_value(metadata, "install-size", G_VARIANT_TYPE_STRING);
    if (value)
    {
        const char *size_str = g_variant_get_string(value, NULL);
        installSize = g_ascii_strtoull(size_str, NULL, 10);
        g_print("install-size (string): %s -> %" G_GUINT64_FORMAT "\n", size_str, installSize);
        g_variant_unref(value);
    }

    g_variant_unref(metadata);

    return downloadSize > 0;
}