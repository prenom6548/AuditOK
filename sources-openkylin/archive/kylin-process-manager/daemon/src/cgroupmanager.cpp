#include "cgroupmanager.h"
#include <QFile>
#include <QFileInfo>
#include <fstream>
#include <linux/magic.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <thread>
#include <unistd.h>

#include "loghelper.h"
#include "misc.h"

/*
 * cgroup 限制说明：
 * - CPU使用率限制：
 *   - v1通过cpu.cfs_quota_us实现（需要配合cpu.cfs_period_us 默认值：100000）
 *   - v2通过cpu.max实现（默认值：max 100000）
 * - CPU权重限制：
 *   - v1通过cpu.shares实现（默认值：1024，最小值：2，最大值：262144）
 *   - v2通过cpu.weight实现（默认值：100，最小值：1，最大值：10000）
 * - 内存限制：
 *   - v1通过memory.soft_limit_in_bytes实现（默认值：9223372036854771712，最小值：0，最大值：9223372036854771712（输入-1））
 *   - v2通过memory.high实现（默认值：max, 最小值：0，最大值：max）
 * - IO限制：
 *   - v1通过blkio.bfq.weight实现（默认值：100，最小值：1，最大值：1000）
 *   - v2通过io.weight实现（默认值：100，最小值：1，最大值：10000）
 */

// cgroup路径说明： 所有路径名返回的函数除根目录外都不带前缀斜杠,所有路径只要是文件夹都带有后缀斜杠

namespace
{
const char *cleaner_path = "/usr/bin/kylin-process-manager-cleaner";
const char *cgroup_path = "/sys/fs/cgroup/";
const char *user_prefix = "user/";
const char *service_prefix = "service/";
const char *user_slice_prefix = "user.slice/";
const char *system_slice_prefix = "system.slice/";
}

CGroupManager::CGroupManager()
    : m_cgroupVersion(cgroupVersion())
    , m_groupEnable(false)
{
    initReleaseAgent();
}

CGroupManager::~CGroupManager()
{
    for (auto &pair : m_records) {
        resetLimitValue(pair.second);
    }
}

int CGroupManager::LimitProcessCPUPercent(int pid, double cpuPercent, QString mode)
{
    static unsigned int kernelNum = std::thread::hardware_concurrency();
    if (cpuPercent < 0.0 || cpuPercent > kernelNum * 100.0) {
        PRINT_NOTICE("limit process cpu percent pid : %d, cpuPercent limit : %f, out of range\n", pid, cpuPercent);
        return -1;
    }
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("limit process cpu percent pid : %d, get proc info failed\n", pid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = CPU_PERCENT;
    rec.value_ = QVariant::fromValue(cpuPercent);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitProcessCPUWeight(int pid, int cpuWeight, QString mode)
{
    if (cpuWeight < 2 || cpuWeight > 262144) {
        PRINT_NOTICE("limit process cpu weight pid : %d, cpuWeight limit : %d, out of range\n", pid, cpuWeight);
        return -1;
    }
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("limit process cpu weight pid : %d, get proc info failed\n", pid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = CPU_WEIGHT;
    rec.value_ = QVariant::fromValue(cpuWeight);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitProcessMemoryMax(int pid, uint64_t memoryMax, QString mode)
{
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("limit process memory max pid : %d, get proc info failed\n", pid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = MEMORY_MAX;
    rec.value_ = QVariant::fromValue(memoryMax);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitProcessIOWeight(int pid, int ioWeight, QString mode)
{
    if (ioWeight < 1 || ioWeight > 1000) {
        PRINT_NOTICE("limit process io weight pid : %d, ioWeight limit : %d, out of range\n", pid, ioWeight);
        return -1;
    }
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("limit process io weight pid : %d, get proc info failed\n", pid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = IO_WEIGHT;
    rec.value_ = QVariant::fromValue(ioWeight);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::FreezeProcess(int pid, QString mode)
{
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("freeze process pid : %d, get proc info failed\n", pid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = FREEZE;
    rec.value_ = 1;
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitThreadCPUPercent(int pid, int tid, double cpuPercent, QString mode)
{
    if (cpuPercent < 0.0 || cpuPercent > 100.0) {
        PRINT_NOTICE("limit thread cpu percent pid : %d, tid : %d, cpuPercent limit : %f, out of range\n", pid, tid, cpuPercent);
        return -1;
    }
    // 获取线程信息
    int info = addTaskInfo(pid, tid);
    if (info == -1) {
        PRINT_NOTICE("limit thread cpu percent pid : %d, tid :%d, get task info failed\n", pid, tid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = THREAD;
    rec.pid_ = pid;
    rec.tid_ = tid;
    rec.limit_type = CPU_PERCENT;
    rec.value_ = QVariant::fromValue(cpuPercent);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitThreadCPUWeight(int pid, int tid, int cpuWeight, QString mode)
{
    if (cpuWeight < 2 || cpuWeight > 262144) {
        PRINT_NOTICE("limit thread cpu weight pid : %d, tid : %d, cpuWeight limit : %d, out of range\n", pid, tid, cpuWeight);
        return -1;
    }
    // 获取进程信息
    int info = addTaskInfo(pid, tid);
    if (info == -1) {
        PRINT_NOTICE("limit thread cpu weight pid : %d, tid :%d, get task info failed\n", pid, tid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = THREAD;
    rec.pid_ = pid;
    rec.tid_ = tid;
    rec.limit_type = CPU_WEIGHT;
    rec.value_ = QVariant::fromValue(cpuWeight);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitThreadMemoryMax(int pid, int tid, uint64_t memoryMax, QString mode)
{
    // 获取进程信息
    int info = addTaskInfo(pid, tid);
    if (info == -1) {
        PRINT_NOTICE("limit thread memory max pid : %d, tid :%d, get task info failed\n", pid, tid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.pid_ = pid;
    if (m_cgroupVersion == 1) {
        rec.tid_ = tid;
        rec.target_type = THREAD;
    } else {
        rec.tid_ = 0;
        rec.target_type = PROCESS;
    }
    rec.limit_type = MEMORY_MAX;
    rec.value_ = QVariant::fromValue(memoryMax);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::LimitThreadIOWeight(int pid, int tid, int ioWeight, QString mode)
{
    if (ioWeight < 1 || ioWeight > 1000) {
        PRINT_NOTICE("limit thread io weight pid : %d, tid : %d, ioWeight limit : %d, out of range\n", pid, tid, ioWeight);
        return -1;
    }
    // 获取进程信息
    int info = addTaskInfo(pid, tid);
    if (info == -1) {
        PRINT_NOTICE("limit thread io weight pid : %d, tid :%d, get task info failed\n", pid, tid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.pid_ = pid;
    if (m_cgroupVersion == 1) {
        rec.tid_ = tid;
        rec.target_type = THREAD;
    } else {
        rec.tid_ = 0;
        rec.target_type = PROCESS;
    }
    rec.limit_type = IO_WEIGHT;
    rec.value_ = QVariant::fromValue(ioWeight);
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

int CGroupManager::FreezeThread(int pid, int tid, QString mode)
{
    // 获取进程信息
    int info = addTaskInfo(pid, tid);
    if (info == -1) {
        PRINT_NOTICE("limit thread cpu percent pid : %d, tid :%d, get task info failed\n", pid, tid);
        return -1;
    }

    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = THREAD;
    rec.pid_ = pid;
    rec.tid_ = tid;
    rec.limit_type = FREEZE;
    rec.value_ = 1;
    rec.mode_ = mode.toStdString();

    return executeLimitRecord(rec);
}

bool CGroupManager::ReleaseLimitByIndex(int index)
{
    auto it = m_records.find(index);
    if (it == m_records.end()) {
        PRINT_NOTICE("release limit by index, index : %d not exist\n", index);
        return false;
    }

    LimitRecord rec = it->second;

    m_records.erase(it);

    LinkedData *linkedData = getLinkedData(rec.pid_, rec.tid_);
    if (linkedData == nullptr) {
        return false;
    }

    linkedData->limit_indices.erase(index);

    // 如果这个限制正在执行，则需要重新检查是否有其他限制可以执行
    if (linkedData->active_limits.find(index) != linkedData->active_limits.end()) {
        linkedData->active_limits.erase(index);
        limitChangedAfter(rec, false);

        int realIndex = checkLimitRecord(rec.pid_, rec.tid_, rec.limit_type);
        // 说明不需要再执行任何限制
        // 分两种情况
        // 1、进程没有了限制，需要先查询线程的限制是否还有，如果还有线程限制，需要重置进程限制
        //    如果进程、线程均没有限制，则直接将pid恢复回初始cgroup中，删除当前cgroup
        // 2、线程没有了限制，则直接将tid恢复到进程所在cgroup中
        if (realIndex == -1) {
            if (rec.target_type == PROCESS) { // 进程
                if (isThreadLimit(rec.pid_, rec.limit_type)) {
                    resetLimitValue(rec);
                } else {
                    if (isCgroupActive(linkedData->active_limits, rec.limit_type)) {
                        resetLimitValue(rec);
                    } else {
                        restoreCgroup(rec.pid_, 0, rec.limit_type);
                    }
                }
            } else { // 线程
                if (isCgroupActive(linkedData->active_limits, rec.limit_type)) {
                    resetLimitValue(rec);
                } else {
                    restoreCgroup(rec.pid_, rec.tid_, rec.limit_type);
                }
            }
        } else { // 说明有其他的限制，直接更新限制值即可
            if (!createCgroupForTarget(m_records[realIndex]))
                return false;
            if (!setLimitValue(m_records[realIndex]))
                return false;
        }
    }

    return true;
}

bool CGroupManager::ReleaseProcessLimit(int pid, bool threadSync)
{
    bool success = true;
    if (threadSync) {
        // 先解除线程限制
        for (auto it = m_records.begin(); it != m_records.end();) {
            const auto &rec = it->second;
            int index = rec.index_;
            it = std::next(it);
            if (rec.pid_ == pid && rec.tid_ != 0) {
                if (!ReleaseLimitByIndex(index)) {
                    PRINT_NOTICE("Failed to release thread limit for pid: %d, tid: %d\n", pid, rec.tid_);
                    success = false;
                }
            }
        }
    }

    // 再解除进程限制
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        it = std::next(it);
        if (rec.pid_ == pid && rec.tid_ == 0) {
            if (!ReleaseLimitByIndex(rec.index_)) {
                PRINT_NOTICE("Failed to release process limit for pid: %d\n", pid);
                success = false;
            }
        }
    }

    return success;
}

bool CGroupManager::ReleaseThreadLimit(int pid, int tid)
{
    bool success = true;
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        it = std::next(it);
        if (rec.pid_ == pid && rec.tid_ == tid) {
            if (!ReleaseLimitByIndex(rec.index_)) {
                PRINT_NOTICE("Failed to release thread limit for pid: %d, tid: %d\n", pid, tid);
                success = false;
            }
        }
    }

    return success;
}

QString CGroupManager::GetProcessLimitInfo(int pid)
{
    QString info;
    for (const auto &pair : m_records) {
        const auto &rec = pair.second;
        if (rec.pid_ == pid && rec.tid_ == 0) {
            info += formatLimitInfo(rec) + "\n";
        }
    }
    return info;
}

QString CGroupManager::GetThreadLimitInfo(int pid, int tid)
{
    QString info;
    for (const auto &pair : m_records) {
        const auto &rec = pair.second;
        if (rec.pid_ == pid && rec.tid_ == tid) {
            info += formatLimitInfo(rec) + "\n";
        }
    }
    return info;
}

QString CGroupManager::GetLimitInfoByIndex(int index)
{
    auto it = m_records.find(index);
    if (it == m_records.end())
        return QString();
    return formatLimitInfo(it->second);
}

bool CGroupManager::SetCurrentUserProperty(QString currentUserUID, QString currentUserSlice, QString currentUserService, QString currentUserSession)
{
    UserInfo userInfo;
    userInfo.uid = currentUserUID.toStdString();
    userInfo.slice = currentUserSlice.toStdString();
    userInfo.service = currentUserService.toStdString();
    userInfo.session = currentUserSession.toStdString();
    m_userInfos[currentUserUID.toStdString()] = userInfo;
    return createCgroupRootPath(userInfo.uid);
}

void CGroupManager::SetGroupPath(QString currentUserUID, QString name, QString path)
{
    if (m_userInfos.find(currentUserUID.toStdString()) == m_userInfos.end()) {
        PRINT_NOTICE("set group path failed, can not find user info, current user uid : %s\n", currentUserUID.toStdString().c_str());
        return;
    }
    m_groupInfos[currentUserUID.toStdString()][name.toStdString()].group_ = name.toStdString();
    std::string cgroupPath = path.toStdString();
    std::string realPath = "";
    const std::string userPrefix(user_prefix);
    if (misc::string::startWith(cgroupPath, userPrefix)) {
        cgroupPath = cgroupPath.substr(userPrefix.length(), path.length() - userPrefix.length());
        const std::string servicePrefix(service_prefix);
        if (misc::string::startWith(cgroupPath, servicePrefix)) {
            std::string prefix = user_slice_prefix + m_userInfos[currentUserUID.toStdString()].slice + "/" + m_userInfos[currentUserUID.toStdString()].service;
            realPath = prefix + "/" + cgroupPath.substr(8, cgroupPath.size() - 8) + "/";
        }
    }
    m_groupInfos[currentUserUID.toStdString()][name.toStdString()].path_ = realPath;
}

void CGroupManager::SetGroupProperty(QString currentUserUID, QString name, QString node, QString value)
{
    static std::unordered_map<std::string, LimitType> nodeLimit = {
        {"cpu.cfs_quota_us", CPU_PERCENT},
        {"cpu.shares", CPU_WEIGHT},
        {"memory.limit_in_bytes", MEMORY_MAX},
        {"cpu.max", CPU_PERCENT},
        {"cpu.weight", CPU_WEIGHT},
        {"cgroup.freeze", FREEZE},
    };
    auto it = nodeLimit.find(node.toStdString());
    if (it == nodeLimit.end()) {
        PRINT_NOTICE("setgroupproperty unsupported limit node : %s\n", node.toStdString().c_str());
        return;
    }
    LimitType limitType = it->second;
    auto parseValue = [](LimitType limitType, const QString &value) -> LimitValue {
        LimitValue ret;
        if (limitType == CPU_PERCENT) {
            int quotaUs = value.toInt();
            ret = QVariant::fromValue((double)quotaUs / 100000 * 100);
        } else if (limitType == CPU_WEIGHT) {
            ret = QVariant::fromValue(value.toInt());
        } else if (limitType == MEMORY_MAX) {
            ret = QVariant::fromValue(value.toULong());
        } else if (limitType == FREEZE) {
            ret = QVariant::fromValue(value.toInt());
        } else {
            PRINT_NOTICE("setgroupproperty unsupported limit type : %d\n", static_cast<int>(limitType));
        }
        return ret;
    };
    LimitValue val;
    if (!value.isEmpty()) {
        val = parseValue(limitType, value);
    }
    m_groupInfos[currentUserUID.toStdString()][name.toStdString()].limits_[limitType] = val;
    updateGroupLimit(name.toStdString(), currentUserUID.toStdString());
}

void CGroupManager::CreateGroupPath(QString currentUserUID, QString name)
{
    auto it = m_groupInfos.find(currentUserUID.toStdString());
    if (it == m_groupInfos.end()) {
        PRINT_NOTICE("create group path can not find user info, current user uid : %s\n", currentUserUID.toStdString().c_str());
        return;
    }

    auto it1 = it->second.find(name.toStdString());
    if (it1 == it->second.end()) {
        PRINT_NOTICE("create group path can not find group info, name : %s\n", name.toStdString().c_str());
        return;
    }
    GroupInfo &groupInfo = it1->second;
    std::vector<std::string> cgroupPaths;
    if (m_cgroupVersion == 1) {
        for (const auto &pair : groupInfo.limits_) {
            // 创建路径
            std::string cgroupPath = cgroupBasePath(pair.first) + groupInfo.path_;
            cgroupPaths.push_back(cgroupPath);
        }
    } else {
        cgroupPaths.push_back(cgroupBasePath(LimitType()) + groupInfo.path_);
    }
    for (const auto &cgroupPath : cgroupPaths) {
        if (containCreatedPath(cgroupPath)) {
            continue;
        }
        PRINT_DBG("create group path, name : %s, cgroup path : %s\n", name.toStdString().c_str(), cgroupPath.c_str());
        if (!ensureDirectory(cgroupPath)) {
            PRINT_NOTICE("update group limit ensure directory failed, path : %s\n", cgroupPath.c_str());
            continue;
        }
        if (m_cgroupVersion == 2) {
            enableSubTreeControl(cgroupPath, {"io", "cpu", "memory"});
        }
    }

    updateGroupLimit(name.toStdString(), currentUserUID.toStdString());
}

void CGroupManager::RemoveGroupPath(QString currentUserUID, QString name)
{
    auto it = m_groupInfos.find(currentUserUID.toStdString());
    if (it == m_groupInfos.end()) {
        PRINT_NOTICE("remove group path can not find user info, current user uid : %s\n", currentUserUID.toStdString().c_str());
        return;
    }

    auto it1 = it->second.find(name.toStdString());
    if (it1 == it->second.end()) {
        return;
    }
    std::string path = it1->second.path_;
    RemoveCGroupPath(QString::fromStdString(path));
}

void CGroupManager::LimitProcessByGroup(QString name, int pid)
{
    m_procGroup[pid] = name.toStdString();
    PRINT_DBG("limit process by group, name : %s, pid : %d\n", name.toStdString().c_str(), pid);
    // 获取进程信息
    int info = addProcInfo(pid);
    if (info == -1) {
        PRINT_NOTICE("limit process by group pid : %d, get proc info failed\n", pid);
        return;
    }

    // 查找分组找到限制集
    auto it = m_groupInfos.find(std::to_string(m_procInfos[pid].uid_));
    if (it == m_groupInfos.end()) {
        PRINT_NOTICE("limit process by group can not find group info, uid : %d\n", m_procInfos[pid].uid_);
        return;
    }

    auto it1 = it->second.find(name.toStdString());
    if (it1 == it->second.end()) {
        PRINT_NOTICE("limit process by group can not find group info, name : %s\n", name.toStdString().c_str());
        return;
    }
    GroupInfo &groupInfo = it1->second;
    // 记录限制信息
    LimitRecord rec;
    rec.index_ = -1;
    rec.target_type = PROCESS;
    rec.pid_ = pid;
    rec.tid_ = 0;
    rec.limit_type = NONE;
    rec.mode_ = "auto";
    if (groupInfo.limits_.empty()) {
        executeLimitRecord(rec);
    }
    for (const auto &pair : groupInfo.limits_) {
        executeLimitRecord(rec);
    }
}

void CGroupManager::RemoveCGroupPath(QString path)
{
    PRINT_DBG("remove cgroup path : %s\n", path.toStdString().c_str());
    std::vector<std::string> paths;
    const std::string cgroupPath(cgroup_path);
    if (m_cgroupVersion == 1) {
        paths.push_back(cgroupPath + "cpu");
        paths.push_back(cgroupPath + "memory");
        paths.push_back(cgroupPath + "blkio");
        paths.push_back(cgroupPath + "freezer");
    } else {
        paths.push_back(cgroupPath);
    }
    for (const auto &base : paths) {
        std::string fullPath = base + path.toStdString();
        removeCgroupIfEmpty(fullPath);
    }
}

void CGroupManager::SetGroupEnable(bool enable)
{
    std::vector<LimitRecord> records;
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        if (rec.group_.empty()) {
            records.emplace_back(std::move(it->second));
            PRINT_NOTICE("Update limit target type : %d, pid : %d, tid : %d, limit type : %d, group : %s, mode : %s\n",
                         static_cast<int>(rec.target_type), rec.pid_, rec.tid_, static_cast<int>(rec.limit_type), rec.group_.c_str(), rec.mode_.c_str());
        }
        it = std::next(it);
        PRINT_DBG("Release limit target type : %d, pid : %d, tid : %d, limit type : %d, group : %s, mode : %s\n",
                  static_cast<int>(rec.target_type), rec.pid_, rec.tid_, static_cast<int>(rec.limit_type), rec.group_.c_str(), rec.mode_.c_str());
        if (!ReleaseLimitByIndex(rec.index_)) {
            PRINT_NOTICE("Failed to release limit target type : %d, pid : %d, tid : %d, limit type : %d, group : %s, mode : %s\n",
                         static_cast<int>(rec.target_type), rec.pid_, rec.tid_, static_cast<int>(rec.limit_type), rec.group_.c_str(), rec.mode_.c_str());
        }
    }
    m_groupEnable = enable;
    if (!enable) {
        m_procGroup.clear();
    }
    for (LimitRecord &rec : records) {
        executeLimitRecord(rec);
    }
}

void CGroupManager::SetFreezeEnable(bool enable)
{
    PRINT_NOTICE("SetFreezeEnable, enable: %d\n", enable);
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        it = std::next(it);
        if (rec.limit_type != FREEZE && rec.limit_type != MEMORY_MAX)
            continue;
        std::string file = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, rec.tid_, rec.group_, getTargetType(rec));
        if (!containCreatedPath(file)) {
            continue;
        }

        if (enable) {
            PRINT_DBG("set freeze enable limit type : %d, file : %s\n", static_cast<int>(rec.limit_type), file.c_str());
            setLimitValueToGroup(rec.limit_type, rec.value_, file);
        } else {
            PRINT_DBG("set freeze disable limit type : %d, file : %s\n", static_cast<int>(rec.limit_type), file.c_str());
            resetLimitValueToGroup(rec.limit_type, file);
        }
    }
    for (auto it = m_groupInfos.begin(); it != m_groupInfos.end(); ++it) {
        auto &groupInfoMap = it->second;
        for (auto groupIt = groupInfoMap.begin(); groupIt != groupInfoMap.end(); ++groupIt) {
            auto &groupInfo = groupIt->second;
            auto it = groupInfo.limits_.find(FREEZE);
            if (it == groupInfo.limits_.end())
                continue;
            std::string file = cgroupBasePath(FREEZE) + groupInfo.path_;
            if (!containCreatedPath(file)) {
                continue;
            }
            if (enable) {
                PRINT_DBG("set freeze enable group limit file : %s\n", file.c_str());
                setLimitValueToGroup(FREEZE, it->second, file);
            } else {
                PRINT_DBG("set freeze disable group limit file : %s\n", file.c_str());
                resetLimitValueToGroup(FREEZE, file);
            }
        }
    }
}

void CGroupManager::ThawedProcess(int pid)
{
    PRINT_NOTICE("ThawedProcess, pid: %d\n", pid);
    thawedProcess(pid);
}

bool CGroupManager::isUserVaild(std::string uid)
{
    return m_userInfos.find(uid) != m_userInfos.end();
}

void CGroupManager::thawedProcess(int pid)
{
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        it = std::next(it);
        if (rec.pid_ == pid && rec.tid_ == 0 && rec.limit_type == FREEZE) {
            if (!ReleaseLimitByIndex(rec.index_)) {
                PRINT_NOTICE("Failed to release process freeze limit for pid: %d\n", pid);
            }
        }
    }
}

QList<int> CGroupManager::listFreezeProcs()
{
    QList<int> ret;
    for (auto it = m_records.begin(); it != m_records.end();) {
        const auto &rec = it->second;
        int index = rec.index_;
        it = std::next(it);
        if (rec.tid_ == 0 && rec.limit_type == FREEZE) {
            ret.append(rec.pid_);
        }
    }
    return ret;
}

QList<int> CGroupManager::getGroupPids(const QString &groupName)
{
    QList<int> result;
    auto pids = groupPids(groupName.toStdString());
    for (int pid : pids) {
        result.append(pid);
    }
    return result;
}

int CGroupManager::cgroupVersion()
{
    struct statfs fs;
    statfs(cgroup_path, &fs);
    std::stringstream ss;
    ss << fs.f_type;
    if (fs.f_type == TMPFS_MAGIC) {
        return 1;
    } else if (fs.f_type == CGROUP2_SUPER_MAGIC) {
        return 2;
    }

    PRINT_NOTICE("Unknown cgroup version.");
    return -1;
}

std::string CGroupManager::cgroupBasePath(LimitType type)
{
    std::string base;
    const std::string cgroupPath(cgroup_path);
    if (m_cgroupVersion == 1) {
        switch (type) {
        case CPU_PERCENT:
        case CPU_WEIGHT:
            base = cgroupPath + "cpu/";
            break;
        case MEMORY_MAX:
            base = cgroupPath + "memory/";
            break;
        case IO_WEIGHT:
            base = cgroupPath + "blkio/";
            break;
        case FREEZE:
            base = cgroupPath + "freezer/";
            break;
        }
    } else {
        base = cgroupPath;
    }
    return base;
}

std::string CGroupManager::cgroupController(LimitType type)
{
    std::string controller;
    if (m_cgroupVersion == 1) {
        switch (type) {
        case CPU_PERCENT:
        case CPU_WEIGHT:
            controller = "cpu";
            break;
        case MEMORY_MAX:
            controller = "memory";
            break;
        case IO_WEIGHT:
            controller = "blkio";
            break;
        case FREEZE:
            controller = "freezer";
            break;
        }
    } else {
        controller = "";
    }

    return controller;
}

std::string CGroupManager::cgroupControlType(LimitType type)
{
    std::string controlType;
    switch (type) {
    case CPU_PERCENT:
    case CPU_WEIGHT:
        controlType = "cpu";
        break;
    case MEMORY_MAX:
        controlType = "memory";
        break;
    case IO_WEIGHT:
        controlType = "io";
        break;
    case FREEZE:
        controlType = "freeze";
        break;
    }

    return controlType;
}

std::string CGroupManager::cgroupControlPath(pid_t pid, pid_t tid, std::string group, TargetType targetType)
{
    std::string uid = "";
    std::string procName = "";
    auto it = m_procInfos.find(pid);
    if (it == m_procInfos.end()) {
        PRINT_NOTICE("cgroup control path proc info not find, pid : %d\n", pid);
    } else {
        uid = std::to_string(it->second.uid_);
        procName = it->second.proc_name;
    }

    std::string prefix = "";
    std::string suffix = "";
    auto userIt = m_userInfos.find(uid);
    // user.slice/user-x.slice/user@x.service
    if (userIt != m_userInfos.end()) {
        if (m_cgroupVersion == 1 || (m_cgroupVersion == 2 && targetType == PROCESS)) {
            auto groupIt = m_groupInfos.find(uid);
            const std::string userPrefix(user_slice_prefix);
            if (groupIt == m_groupInfos.end()) {
                PRINT_NOTICE("cgroup control path group info not find, uid : %s\n", uid.c_str());
                prefix = userPrefix + userIt->second.slice + "/" + userIt->second.service + "/";
            } else {
                if (m_groupEnable) {
                    if (group.empty())
                        group = "Default";
                    auto groupIt1 = groupIt->second.find(group);
                    if (groupIt1 == groupIt->second.end()) {
                        PRINT_NOTICE("cgroup control path group info not find, group : %s\n", group.c_str());
                        prefix = userPrefix + userIt->second.slice + "/" + userIt->second.service + "/";
                    } else {
                        prefix = groupIt1->second.path_;
                    }
                } else {
                    prefix = userPrefix + userIt->second.slice + "/" + userIt->second.service + "/";
                }
            }
        }
        suffix = ".scope/";
    } else {
        // system.slice
        if (m_cgroupVersion == 1 || (m_cgroupVersion == 2 && targetType == PROCESS)) {
            const std::string systemPrefix(system_slice_prefix);
            prefix = systemPrefix;
        }
        suffix = ".service/";
    }
    std::string name;
    name = procName + suffix;
    if (tid != 0) {
        name = name + "task_" + std::to_string(tid) + suffix;
    }

    return prefix + name;
}

std::string CGroupManager::cgroupOldControlPath(pid_t pid, LimitType limitType)
{
    auto it = m_procInfos.find(pid);
    if (it == m_procInfos.end()) {
        PRINT_NOTICE("cgroup old control path can not find proc info pid : %d\n", pid);
        return "";
    }
    return it->second.old_cgroups[cgroupController(limitType)];
}

void CGroupManager::addCreatedPath(std::string path)
{
    std::unique_lock lock(m_pathMutex);
    m_pathCreated.insert(path);
}

void CGroupManager::removeCreatedPath(std::string path)
{
    std::unique_lock lock(m_pathMutex);
    m_pathCreated.erase(path);
}

bool CGroupManager::containCreatedPath(std::string path)
{
    std::shared_lock lock(m_pathMutex);
    return m_pathCreated.find(path) != m_pathCreated.end();
}

std::vector<std::string> CGroupManager::cgroupRootPaths(const std::string &uid)
{
    std::vector<std::string> path;
    const std::string cgroupPath(cgroup_path);
    if (m_cgroupVersion == 1) {
        path.push_back(cgroupPath + "cpu/");
        path.push_back(cgroupPath + "memory/");
        path.push_back(cgroupPath + "blkio/");
        path.push_back(cgroupPath + "freezer/");
    } else {
        path.push_back(cgroupPath);
    }
    size_t n = path.size();
    path.reserve(4 * n);
    for (size_t i = 0; i < n; ++i) {
        path.push_back(path[i] + user_slice_prefix);
    }

    auto it = m_userInfos.find(uid);
    if (it == m_userInfos.end()) {
        PRINT_NOTICE("cgroup root path user info not find, uid : %s\n", uid.c_str());
    } else {
        for (size_t i = 0; i < n; ++i) {
            path.push_back(path[i] + user_slice_prefix + it->second.slice + "/");
        }

        for (size_t i = 0; i < n; ++i) {
            path.push_back(path[i] + user_slice_prefix + it->second.slice + "/" + it->second.service + "/");
        }
    }

    for (size_t i = 0; i < n; ++i) {
        path[i] = path[i] + system_slice_prefix;
    }
    return path;
}

std::vector<int> CGroupManager::groupPids(const std::string path)
{
    std::vector<int> pids;
    std::string filePath = path + "cgroup.procs";
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return pids;
    }
    pid_t pid;
    while (file >> pid) {
        pids.push_back(pid);
    }
    return pids;
}

bool CGroupManager::ensureDirectory(const std::string &path)
{
    PRINT_DBG("ensure directory : %s\n", path.c_str());
    if (mkdir(path.c_str(), 0755) == 0 || errno == EEXIST) {
        addCreatedPath(path);
        return true;
    }
    return false;
}

bool CGroupManager::writeFile(const std::string &file, const std::string &content)
{
    PRINT_DBG("write file : %s, content : %s\n", file.c_str(), content.c_str());
    std::ofstream ofs(file);
    if (!ofs.is_open()) {
        PRINT_NOTICE("write file : %s, content : %s failed, can not open file\n", file.c_str(), content.c_str());
        return false;
    }
    ofs << content;
    if (ofs.fail()) {
        PRINT_NOTICE("write file : %s, content : %s failed, can not write file\n", file.c_str(), content.c_str());
    }
    return !ofs.fail();
}

bool CGroupManager::readFile(const std::string &file, std::string &content)
{
    std::ifstream ifs(file);
    if (!ifs.is_open())
        return false;
    std::getline(ifs, content);
    return !ifs.fail();
}

bool CGroupManager::moveProcToCgroup(pid_t pid, const std::string &cgroupPath)
{
    std::string procFile = cgroupPath + "cgroup.procs";
    return writeFile(procFile, std::to_string(pid));
}

bool CGroupManager::moveTaskToCgroup(pid_t tid, const std::string &cgroupPath)
{
    std::string tasksFile = cgroupPath;
    if (m_cgroupVersion == 2) {
        tasksFile += "cgroup.threads";
    } else {
        tasksFile += "tasks";
    }
    return writeFile(tasksFile, std::to_string(tid));
}

void CGroupManager::moveCgroupToCgroup(std::string cgroupPath, const std::string &oldCgroupPath)
{
    std::vector<int> pids = groupPids(cgroupPath);
    for (int pid : pids) {
        moveProcToCgroup(pid, oldCgroupPath);
    }
}

bool CGroupManager::enableNotifyOnRelease(const std::string &cgroupPath)
{
    if (m_cgroupVersion == 1) {
        std::string notifyOnReleasePath = cgroupPath + "notify_on_release";
        return writeFile(notifyOnReleasePath, "1");
    } else {
        m_cgroupv2ReleaseNotification.addWatchGroup(
            cgroupPath,
            [this](std::string groupPath) {
                removeCgroupIfEmpty(groupPath);
            });
        return true;
    }
    return false;
}

CGroupManager::TargetType CGroupManager::getTargetType(const LimitRecord &rec)
{
    TargetType targetType = rec.target_type;
    if (m_cgroupVersion == 2 && targetType == PROCESS) {
        targetType = isThreadLimit(rec.pid_, rec.limit_type) ? THREAD : PROCESS;
    }
    return targetType;
}

bool CGroupManager::setLimitValue(const LimitRecord &rec)
{

    LinkedData *linkedData = getLinkedData(rec.pid_, rec.tid_);
    if (linkedData == nullptr) {
        return false;
    }

    linkedData->active_limits.insert(rec.index_);
    limitChangedAfter(rec, true);

    std::string file = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, rec.tid_, rec.group_, getTargetType(rec));
    if (!containCreatedPath(file)) {
        return false;
    }

    if (!rec.value_.isValid()) {
        return resetLimitValueToGroup(rec.limit_type, file);
    }

    return setLimitValueToGroup(rec.limit_type, rec.value_, file);
}

bool CGroupManager::setLimitValueToGroup(LimitType limitType, LimitValue limitValue, std::string file)
{
    std::string value;
    if (m_cgroupVersion == 2) {
        switch (limitType) {
        case CPU_PERCENT: {
            file = file + "cpu.max";
            long period = 100000;
            long quota = static_cast<long>(period * limitValue.toDouble() / 100.0);
            value = std::to_string(quota);
            break;
        }
        case CPU_WEIGHT: {
            file = file + "cpu.weight";
            value = std::to_string(limitValue.toInt());
            break;
        }
        case MEMORY_MAX: {
            file = file + "memory.high";
            value = std::to_string(limitValue.value<uint64_t>());
            break;
        }
        case IO_WEIGHT: {
            file = file + "io.weight";
            value = std::to_string(limitValue.toInt());
            break;
        }
        case FREEZE: {
            file = file + "cgroup.freeze";
            value = "1";
            break;
        }
        }
    } else { // v1
        switch (limitType) {
        case CPU_PERCENT: {
            long period = 100000;
            long quota = static_cast<long>(period * limitValue.toDouble() / 100.0);
            if (!writeFile(file + "cpu.cfs_period_us", std::to_string(period)))
                return false;
            file = file + "cpu.cfs_quota_us";
            value = std::to_string(quota);
            break;
        }
        case CPU_WEIGHT: {
            file = file + "cpu.shares";
            value = std::to_string(limitValue.toInt());
            break;
        }
        case MEMORY_MAX: {
            file = file + "memory.soft_limit_in_bytes";
            value = std::to_string(limitValue.value<uint64_t>());
            break;
        }
        case IO_WEIGHT: {
            file = file + "blkio.bfq.weight";
            int v1Weight = limitValue.toInt();
            if (v1Weight < 1)
                v1Weight = 1;
            if (v1Weight > 1000)
                v1Weight = 1000;
            value = std::to_string(v1Weight);
            break;
        }
        case FREEZE: {
            file = file + "freezer.state";
            value = "FROZEN";
            break;
        }
        }
    }
    return writeFile(file, value);
}

bool CGroupManager::resetLimitValue(const LimitRecord &rec)
{
    std::string file = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, rec.tid_, rec.group_, getTargetType(rec));
    if (!containCreatedPath(file)) {
        return false;
    }

    return resetLimitValueToGroup(rec.limit_type, file);
}

bool CGroupManager::resetLimitValueToGroup(LimitType limitType, std::string file)
{
    // 重置为无限制或默认值
    if (m_cgroupVersion == 2) {
        switch (limitType) {
        case CPU_PERCENT:
            return writeFile(file + "cpu.max", "max");
        case CPU_WEIGHT:
            return writeFile(file + "cpu.weight", "100");
        case MEMORY_MAX:
            return writeFile(file + "memory.high", "max");
        case IO_WEIGHT:
            return writeFile(file + "io.weight", "100");
        case FREEZE:
            return writeFile(file + "cgroup.freeze", "0");
        }
    } else {
        switch (limitType) {
        case CPU_PERCENT:
            return writeFile(file + "cpu.cfs_quota_us", "-1");
        case CPU_WEIGHT:
            return writeFile(file + "cpu.shares", "1024");
        case MEMORY_MAX:
            return writeFile(file + "memory.soft_limit_in_bytes", "-1");
        case IO_WEIGHT:
            return writeFile(file + "blkio.bfq.weight", "100");
        case FREEZE:
            return writeFile(file + "freezer.state", "THAWED");
        }
    }
    return true;
}

bool CGroupManager::createCgroupRootPath(const std::string &uid)
{
    auto paths = cgroupRootPaths(uid);
    for (const auto &p : paths) {
        ensureDirectory(p);
        if (m_cgroupVersion == 2) {
            if (!enableSubTreeControl(p, {"io", "cpu", "memory"}))
                return false;
        }
    }
    return true;
}

void CGroupManager::removeCgroupRootPath(const std::string &uid)
{
    auto paths = cgroupRootPaths(uid);
    for (const auto &p : paths) {
        removeCgroupIfEmpty(p);
    }
}

bool CGroupManager::createCgroupForTarget(const LimitRecord &rec)
{
    // 进程的 cgroup 创建
    std::string cgroupPath = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, 0, rec.group_, getTargetType(rec));
    if (!containCreatedPath(cgroupPath)) {
        if (!ensureDirectory(cgroupPath))
            return false;
        if (m_cgroupVersion == 2 && rec.target_type == THREAD) {
            enableSubTreeControl(cgroupPath, {"cpu"});
        }
        std::string oldCgroupPath = cgroupBasePath(rec.limit_type) + cgroupOldControlPath(rec.pid_, rec.limit_type);
        if (oldCgroupPath != cgroupPath) {
            if (!enableNotifyOnRelease(cgroupPath))
                return false;
        }
        // 如果没有创建 先将全部进程加入分组
        if (!moveProcToCgroup(rec.pid_, cgroupPath))
            return false;
    }

    // 创建线程限制的 cgroup 路径
    if (rec.target_type == THREAD) {
        cgroupPath = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, rec.tid_, rec.group_, rec.target_type);
        if (!containCreatedPath(cgroupPath)) {
            if (!ensureDirectory(cgroupPath))
                return false;
            if (m_cgroupVersion == 2) {
                changeGroupType(cgroupPath, "threaded");
            }
            if (!enableNotifyOnRelease(cgroupPath))
                return false;
            // 将目标任务移入 cgroup
            if (!moveTaskToCgroup(rec.tid_, cgroupPath))
                return false;
        }
    }
    return true;
}

void CGroupManager::removeCgroupIfEmpty(const std::string &path)
{
    PRINT_DBG("try to remove cgroup path : %s\n", path.c_str());
    // 检查目录是否为空（没有 tasks 或 cgroup.procs 文件中有任务）
    std::string tasksFile = path;
    if (m_cgroupVersion == 2) {
        tasksFile += "/cgroup.procs";
    } else {
        tasksFile += "/tasks";
    }
    std::ifstream ifs(tasksFile);
    if (ifs.is_open()) {
        std::string line;
        if (std::getline(ifs, line) && !line.empty()) {
            return; // 还有任务
        }
    }
    PRINT_DBG("cgroup path : %s is empty, try to remove\n", path.c_str());
    // 尝试删除目录
    rmdir(path.c_str());
    removeCreatedPath(path);
}

void CGroupManager::initReleaseAgent()
{
    if (m_cgroupVersion == 1) {
        static std::vector<std::string> controllers = {"cpu", "memory", "blkio", "freezer"};
        for (const auto &controller : controllers) {
            std::string releaseAgentPath = cgroup_path + controller + "/release_agent";
            if (!writeFile(releaseAgentPath, cleaner_path)) {
                PRINT_NOTICE("init release agent fail : controller : %s\n", controller.c_str());
            }
        }
    }
}

bool CGroupManager::enableSubTreeControl(const std::string &path, const std::vector<std::string> &controllers)
{
    std::string content = "";
    for (const auto &controller : controllers) {
        content = content + "+" + controller + " ";
    }
    std::string fullPath = path + "cgroup.subtree_control";
    if (!writeFile(fullPath, content)) {
        PRINT_NOTICE("enable subtree control fail : path : %s, content : %s\n", fullPath.c_str(), content.c_str());
        return false;
    }
    if (!readFile(fullPath, content)) {
        PRINT_NOTICE("enable subtree control read fail : path : %s, content : %s\n", fullPath.c_str(), content.c_str());
        return false;
    }
    PRINT_DBG("subtree control : path : %s, content : %s\n", fullPath.c_str(), content.c_str());
    for (const auto &controller : controllers) {
        if (content.find(controller) == std::string::npos) {
            return false;
        }
    }
    return true;
}

void CGroupManager::changeGroupType(const std::string &path, const std::string &type)
{
    std::string fullPath = path + "cgroup.type";
    if (!writeFile(fullPath, type)) {
        PRINT_NOTICE("change group type fail : path : %s, type : %s\n", fullPath.c_str(), type.c_str());
    }
}

void CGroupManager::restoreCgroup(pid_t pid, pid_t tid, LimitType limitType)
{
    // 如果恢复线程的cgroup
    // cgroup v1
    // 1、如果进程已经没有了限制，直接将进程cgroup恢复到初始cgroup中，删除当前cgroup
    // 2、如果进程还有限制，直接将线程恢复到进程所在cgroup中，删除线程cgroup
    // cgroup v2
    // 1、如果进程已经没有限制了同上
    // 2、如果进程还有限制，线程和进程均需要移动
    std::string groupname = getGroupName(pid);
    TargetType targetType = tid == 0 ? PROCESS : THREAD;
    std::string procCgroupPath = cgroupBasePath(limitType) + cgroupControlPath(pid, 0, groupname, targetType);
    if (tid == 0) {
        // 恢复进程cgroup
        std::string originalPath = cgroupBasePath(limitType) + cgroupOldControlPath(pid, limitType);
        // 若限制分组和初始分组一致，则不需要移动，删除创建路径记录，恢复限制参数
        if (procCgroupPath == originalPath) {
            resetLimitValueToGroup(limitType, procCgroupPath);
            removeCreatedPath(procCgroupPath);
        } else {
            moveCgroupToCgroup(procCgroupPath, originalPath);
            removeCgroupIfEmpty(procCgroupPath);
        }
    } else {
        // 恢复线程cgroup
        std::string threadCgroupPath = cgroupBasePath(limitType) + cgroupControlPath(pid, tid, groupname, targetType);
        LinkedData *linkedData = getLinkedData(pid, 0);
        if (linkedData == nullptr) {
            PRINT_NOTICE("restore cgroup can not find link data pid : %d\n", pid);
            return;
        }

        moveTaskToCgroup(tid, procCgroupPath);
        removeCgroupIfEmpty(threadCgroupPath);
        if (isThreadLimit(pid, limitType)) {
            // 如果进程的其他线程限制还在使用，将线程移回进程所在cgroup
        } else if (isCgroupActive(linkedData->active_limits, limitType)) {
            // cgroup v1 ：如果进程的cgroup还在使用，将线程移回进程所在cgroup
            // cgroup v2 ：如果进程的cgroup还在使用，将进程整个移走，删除原来的线程路径和进程路径
            if (m_cgroupVersion == 1) {

            } else {
                std::string procCgroupPath1 = cgroupBasePath(limitType) + cgroupControlPath(pid, 0, groupname, PROCESS);
                if (!containCreatedPath(procCgroupPath1)) {
                    ensureDirectory(procCgroupPath1);
                    std::string originalPath = cgroupBasePath(limitType) + cgroupOldControlPath(pid, limitType);
                    if (originalPath != procCgroupPath1) {
                        enableNotifyOnRelease(procCgroupPath1);
                    }

                    moveProcToCgroup(pid, procCgroupPath1);
                }
            }
        } else {
            // 如果进程的cgroup已经没有限制了，将线程移回初始cgroup
            // 若限制分组和初始分组一致，需要移动线程回到进程cgroup，删除创建路径记录
            std::string originalPath = cgroupBasePath(limitType) + cgroupOldControlPath(pid, limitType);
            if (procCgroupPath == originalPath) {
                removeCreatedPath(procCgroupPath);
            } else {
                moveCgroupToCgroup(procCgroupPath, originalPath);
                removeCgroupIfEmpty(procCgroupPath);
            }
        }
    }
}

void CGroupManager::updateGroupLimit(const std::string &name, const std::string &uid)
{
    auto it = m_groupInfos.find(uid);
    if (it == m_groupInfos.end()) {
        PRINT_NOTICE("update group limit can not find user info, uid : %s\n", uid.c_str());
        return;
    }

    auto it1 = it->second.find(name);
    if (it1 == it->second.end()) {
        PRINT_NOTICE("update group limit can not find group info, name : %s\n", name.c_str());
        return;
    }
    GroupInfo groupInfo = it1->second;
    for (const auto &pair : groupInfo.limits_) {
        // 路径查找
        std::string cgroupPath = cgroupBasePath(pair.first) + groupInfo.path_;
        if (!containCreatedPath(cgroupPath)) {
            continue;
        }

        if (!pair.second.isValid()) {
            resetLimitValueToGroup(pair.first, cgroupPath);
            continue;
        }

        // 设置限制
        if (!setLimitValueToGroup(pair.first, pair.second, cgroupPath)) {
            PRINT_NOTICE("update group limit set limit value failed, path : %s\n", cgroupPath.c_str());
            continue;
        }
    }
}

int CGroupManager::addLimitRecord(LimitRecord &rec)
{
    if (rec.index_ == -1) {
        int idx = m_nextIndex++;
        rec.index_ = idx;
    }
    m_records[rec.index_] = rec;

    return rec.index_;
}

int CGroupManager::executeLimitRecord(LimitRecord &rec)
{
    rec.group_ = getGroupName(rec.pid_);

    int index = addLimitRecord(rec);

    // 将限制记录关联到进程信息
    LinkedData *linkedData = getLinkedData(rec.pid_, rec.tid_);
    if (linkedData == nullptr) {
        return -1;
    }

    std::vector<int> inactiveLimits; // 需要清理的限制
    std::vector<LimitRecord> limits;
    if (rec.target_type == PROCESS) {
        for (auto it = linkedData->active_limits.begin(); it != linkedData->active_limits.end();) {
            auto recordIt = m_records.find(*it);
            if (recordIt == m_records.end()) {
                it = linkedData->active_limits.erase(it);
                continue;
            }
            LimitRecord &record = recordIt->second;
            if (cgroupController(record.limit_type) == cgroupController(rec.limit_type) && record.group_ != rec.group_) {
                // cgroup v1 : 如果是同一控制器下的不同分组的限制，说明之前的限制会被当前限制覆盖，需要清理掉之前的限制
                // cgroup v2 : 只要组别变了，之前的限制就需要清理
                if (record.mode_ == "auto") {
                    it = linkedData->active_limits.erase(it);
                    limitChangedAfter(record, false);
                    linkedData->limit_indices.erase(record.index_);
                    inactiveLimits.push_back(record.index_);
                } else {
                    limits.emplace_back(std::move(record));
                    PRINT_DBG("record limit target type : %d, pid : %d, tid : %d, limit type : %d, group : %s, mode : %s\n",
                              static_cast<int>(record.target_type), record.pid_, record.tid_, static_cast<int>(record.limit_type), record.group_.c_str(), record.mode_.c_str());
                }
                m_records.erase(recordIt);
            } else {
                ++it;
            }
        }
    }
    linkedData->limit_indices.insert(index);

    // 执行限制
    int realIndex = checkLimitRecord(rec.pid_, rec.tid_, rec.limit_type);
    if (realIndex != index) {
        // 若实际执行的限制和当前限制不一致，说明已经有优先级更高的同类限制在执行，直接返回成功
        return index;
    }
    if (!createCgroupForTarget(rec))
        return -1;
    if (!setLimitValue(rec))
        return -1;

    if (rec.target_type == PROCESS) {
        // 如果inactiveLimits不为空，说明有被当前限制覆盖的限制，需要删除掉残留的路径
        std::unordered_set<int> pids;
        for (int inactiveIndex : inactiveLimits) {
            auto recordIt = m_records.find(inactiveIndex);
            if (recordIt == m_records.end()) {
                continue;
            }
            LimitRecord &record = recordIt->second;
            removeCgroupIfEmpty(cgroupBasePath(record.limit_type) + cgroupControlPath(record.pid_, record.tid_, record.group_, record.target_type));
            pids.insert(record.pid_);
        }
        for (LimitRecord &limit : limits) {
            executeLimitRecord(limit);
        }

        for (int pid : pids) {
            updateThreadLimitRecords(pid);
        }
    }

    // cgroup v2 : 如果是线程控制，进程控制路径发生了变化更新进程的限制值
    if (m_cgroupVersion == 2) {
        if (rec.target_type == THREAD) {
            do {
                LinkedData *linkedData = getLinkedData(rec.pid_, 0);
                if (linkedData == nullptr) {
                    break;
                }
                for (int index : linkedData->active_limits) {
                    auto it = m_records.find(index);
                    if (it == m_records.end()) {
                        continue;
                    }
                    LimitRecord &record = it->second;
                    std::string file = cgroupBasePath(record.limit_type) + cgroupControlPath(record.pid_, 0, record.group_, THREAD);
                    if (!containCreatedPath(file)) {
                        continue;
                    }

                    if (record.value_.isValid()) {
                        setLimitValueToGroup(record.limit_type, record.value_, file);
                    }
                }
            } while (0);
        }
    }

    return index;
}

void CGroupManager::limitChangedAfter(const LimitRecord &rec, bool enable)
{
    // 不打算频繁发送信号，只发送需要的改变信号
    if (rec.limit_type == FREEZE && rec.target_type == PROCESS) {
        Q_EMIT LimitChanged(rec.pid_, rec.tid_, QString::fromStdString(cgroupControlType(rec.limit_type)), enable);
    }
}

void CGroupManager::updateThreadLimitRecords(pid_t pid)
{
    // 进程的限制更换了路径，故需更新线程的限制
    auto it = m_procInfos.find(pid);
    if (it == m_procInfos.end()) {
        return;
    }
    for (int tid : it->second.tids_) {
        LinkedData *linkedData = getLinkedData(pid, tid);
        if (linkedData == nullptr) {
            continue;
        }
        std::vector<LimitRecord> limits;
        // 已经没有实际在运行的限制了，全部清空
        linkedData->active_limits.clear();
        // 在现有限制中重新更新分组，执行线程限制
        for (int index : linkedData->limit_indices) {
            auto it1 = m_records.find(index);
            if (it1 == m_records.end()) {
                continue;
            }
            limits.emplace_back(std::move(it1->second));
            m_records.erase(it1);
        }
        linkedData->limit_indices.clear();
        for (auto &limit : limits) {
            removeCgroupIfEmpty(cgroupBasePath(limit.limit_type) + cgroupControlPath(limit.pid_, limit.tid_, limit.group_, limit.target_type));
            executeLimitRecord(limit);
        }
    }
}

int CGroupManager::checkLimitRecord(pid_t pid, pid_t tid, LimitType limitType)
{
    // 判断现行控制器下 要执行限制规则
    LinkedData *linkedData = getLinkedData(pid, tid);
    if (linkedData == nullptr) {
        return -1;
    }

    // 如果限制为冻结，若有活跃的限制直接返回即可
    if (limitType == FREEZE) {
        for (int index : linkedData->active_limits) {
            auto it1 = m_records.find(index);
            if (it1 == m_records.end())
                continue;
            if (limitType == it1->second.limit_type) {
                return it1->second.index_;
            }
        }
    }

    auto exeIt = m_records.end();
    for (int index : linkedData->limit_indices) {
        auto it1 = m_records.find(index);
        if (it1 == m_records.end())
            continue;
        if (limitType == it1->second.limit_type) {
            if (exeIt == m_records.end()) {
                exeIt = it1;
                // 如果限制为冻结，走到这里说明当前进程/线程还没有冻结生效，找到一个返回即可
                if (limitType == FREEZE) {
                    return it1->second.index_;
                }
            }
            // 规则
            if (cmpLimitRule(exeIt->second, it1->second)) {
                exeIt = it1;
            }
        }
    }

    if (exeIt != m_records.end()) {
        return exeIt->second.index_;
    }
    return -1;
}

bool CGroupManager::isCgroupActive(const std::unordered_set<int> &activeLimits, LimitType limitType)
{
    // 判断是否还有活跃的限制路径
    // v10
    // cpu控制器下 CPU_PERCENT、CPU_WEIGHT
    // memory控制器下 MEMORY_MAX
    // io控制器下 IO_WEIGHT
    // freeze控制器下 FREEZE
    static std::unordered_map<LimitType, int> group = {
        {CPU_PERCENT, 0},
        {CPU_WEIGHT, 0},
        {MEMORY_MAX, 1},
        {IO_WEIGHT, 2},
        {FREEZE, 3},
    };
    int target = group[limitType];
    bool isActive = false;
    for (int limitIndex : activeLimits) {
        auto it = m_records.find(limitIndex);
        if (it == m_records.end()) {
            continue;
        }
        if (m_cgroupVersion == 1) {
            if (group[it->second.limit_type] == target) {
                isActive = true;
                break;
            }
        } else {
            isActive = true;
            break;
        }
    }
    return isActive;
}

bool CGroupManager::isThreadLimit(pid_t pid, LimitType limitType)
{
    std::unordered_set<pid_t> tids;
    auto it = m_procInfos.find(pid);
    if (it != m_procInfos.end()) {
        tids = it->second.tids_;
    } else {
        PRINT_NOTICE("is thread limit proc info can not find pid : %d\n", pid);
    }
    bool threadIsLimit = false;
    for (pid_t tid : tids) {
        auto it = m_taskInfos.find(tid);
        if (it == m_taskInfos.end()) {
            continue;
        }
        if (isCgroupActive(it->second.linked_data.active_limits, limitType)) {
            threadIsLimit = true;
            break;
        }
    }
    return threadIsLimit;
}

bool CGroupManager::cmpLimitRule(const LimitRecord &rec1, const LimitRecord &rec2)
{
    LimitType limitType = rec1.limit_type;
    if (limitType != rec2.limit_type)
        return false;
    if (rec1.mode_ == rec2.mode_) {
        // 如果模式相同时
        // 规则：限制以值更小的限制为准
        return cmpLimitValue(rec1.value_, rec2.value_, limitType) == 1;
    } else {
        // 模式{auto、user}
        // auto 自动识别的限制
        // user 用户设置的限制
        // 规则：用户设置的高于自动识别的
        static std::unordered_map<std::string, int> priorityMap = {
            {"auto", 50},
            {"user", 100}};
        return priorityMap[rec1.mode_] < priorityMap[rec2.mode_];
    }
    return false;
}

int CGroupManager::cmpLimitValue(const LimitValue &value1, const LimitValue &value2, LimitType limitType)
{
    if (!value1.isValid() && !value2.isValid()) {
        return 0;
    } else if (!value1.isValid()) {
        return 1;
    } else if (!value2.isValid()) {
        return -1;
    } else {
    }
    switch (limitType) {
    case CPU_PERCENT: {
        double diff = value2.toDouble() - value1.toDouble();
        if (std::abs(diff) < 1e-6) {
            return 0;
        } else if (diff < 0) {
            return 1;
        } else {
            return -1;
        }
    } break;
    case CPU_WEIGHT: {
        int diff = value2.toInt() - value1.toInt();
        if (diff == 0) {
            return 0;
        } else if (diff < 0) {
            return 1;
        } else {
            return -1;
        }
    } break;
    case MEMORY_MAX: {
        uint64_t diff = value2.value<uint64_t>() - value1.value<uint64_t>();
        if (diff == 0) {
            return 0;
        } else if (diff < 0) {
            return 1;
        } else {
            return -1;
        }
    } break;
    case IO_WEIGHT: {
        int diff = value2.toInt() - value1.toInt();
        if (diff == 0) {
            return 0;
        } else if (diff < 0) {
            return 1;
        } else {
            return -1;
        }
    } break;
    default:
        return 0;
    }
    return 0;
}

int CGroupManager::addProcInfo(pid_t pid)
{
    if (!process_info_helper::processExists(pid)) {
        return -1;
    }
    auto it = m_procInfos.find(pid);
    if (it != m_procInfos.end()) {
        std::vector<pid_t> tids = process_info_helper::tids(pid);
        it->second.tids_.clear();
        it->second.tids_.insert(tids.begin(), tids.end());
        // 已经存在
        return pid;
    }
    ProcInfo info;
    info.pid_ = pid;
    info.uid_ = process_info_helper::uid(pid);
    std::vector<pid_t> tids = process_info_helper::tids(pid);
    info.tids_.insert(tids.begin(), tids.end());
    int ppid = pid;
    do {
        ppid = process_info_helper::parentPid(ppid);
        auto it1 = m_procInfos.find(ppid);
        if (it1 != m_procInfos.end()) {
            info.old_cgroups = it1->second.old_cgroups;
            break;
        }
    } while (ppid != -1);
    if (info.old_cgroups.empty()) {
        info.old_cgroups = process_info_helper::cgroupMap(pid);
    }
    info.proc_name = process_info_helper::processName(pid);
    m_procInfos[pid] = info;
    return pid;
}

int CGroupManager::addTaskInfo(pid_t pid, pid_t tid)
{
    // 先添加进程信息
    int infoPid = addProcInfo(pid);
    if (infoPid == -1) {
        return -1;
    }
    auto &tids = m_procInfos[infoPid].tids_;
    if (tids.find(tid) == tids.end()) {
        return -1;
    }
    auto it = m_taskInfos.find(tid);
    if (it != m_taskInfos.end()) {
        // 已经存在
        return tid;
    }
    TaskInfo info;
    info.tid_ = tid;
    info.pid_ = pid;
    m_taskInfos[tid] = info;
    return tid;
}

CGroupManager::LinkedData *CGroupManager::getLinkedData(pid_t pid, pid_t tid)
{
    LinkedData *linkedData = nullptr;
    if (tid == 0) {
        auto it = m_procInfos.find(pid);
        if (it != m_procInfos.end()) {
            linkedData = &it->second.linked_data;
        }
    } else {
        auto it = m_taskInfos.find(tid);
        if (it != m_taskInfos.end()) {
            linkedData = &it->second.linked_data;
        }
    }
    return linkedData;
}

std::string CGroupManager::getGroupName(pid_t pid)
{
    std::string groupname = "";
    auto it = m_procGroup.find(pid);
    if (it != m_procGroup.end()) {
        groupname = it->second;
    }
    return groupname;
}

QString CGroupManager::formatLimitInfo(const LimitRecord &rec)
{
    QString info;
    info += QString("Index: %1\n").arg(rec.index_);
    info += QString("Target: %2 %3\n").arg(rec.tid_ == 0 ? "Process" : "Thread").arg(rec.tid_ == 0 ? QString::number(rec.pid_) : QString("%1:%2").arg(rec.pid_).arg(rec.tid_));
    info += "Limit Type: ";
    switch (rec.limit_type) {
    case CPU_PERCENT:
        info += "CPU Percent";
        break;
    case CPU_WEIGHT:
        info += "CPU Weight";
        break;
    case MEMORY_MAX:
        info += "Memory Max";
        break;
    case IO_WEIGHT:
        info += "IO Weight";
        break;
    case FREEZE:
        info += "Freeze";
        break;
    }
    info += "\n";

    std::string cgroupPath = cgroupBasePath(rec.limit_type) + cgroupControlPath(rec.pid_, rec.tid_, rec.group_, getTargetType(rec));
    info += "CGroup Path: " + QString::fromStdString(cgroupPath) + "\n";
    switch (rec.limit_type) {
    case CPU_PERCENT:
        info += QString("Value: %1%\n").arg(rec.value_.toDouble());
        break;
    case CPU_WEIGHT:
        info += QString("Value: %1\n").arg(rec.value_.toInt());
        break;
    case MEMORY_MAX:
        info += QString("Value: %1 bytes\n").arg(rec.value_.value<uint64_t>());
        break;
    case IO_WEIGHT:
        info += QString("Value: %1\n").arg(rec.value_.toInt());
        break;
    case FREEZE:
        break;
    }

    return info;
}
