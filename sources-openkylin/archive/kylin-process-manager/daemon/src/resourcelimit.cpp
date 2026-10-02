#include "resourcelimit.h"
#include "loghelper.h"
#include <functional>

// 等级枚举
enum class Level { Low,
                   Middle,
                   High };

namespace
{
// 将字符串等级转换为枚举
Level levelFromString(const QString &level)
{
    static const std::map<QString, Level> map = {
        {"low", Level::Low},
        {"middle", Level::Middle},
        {"high", Level::High}};
    auto it = map.find(level);
    if (it != map.end())
        return it->second;
    // 此处返回 Low 作为默认
    return Level::Low;
}

// 等级到具体数值的映射
double getCpuPercent(Level level)
{
    static const std::map<Level, double> map = {
        {Level::Low, 10.0},
        {Level::Middle, 30.0},
        {Level::High, 50.0}};
    return map.at(level);
}

int getCpuWeight(Level level)
{
    static const std::map<Level, int> map = {
        {Level::Low, 102},
        {Level::Middle, 307},
        {Level::High, 512}};
    return map.at(level);
}

quint64 getMemoryMax(Level level)
{
    static const std::map<Level, quint64> map = {
        {Level::Low, 512 * 1024 * 1024}, // 512MB
        {Level::Middle, 1024 * 1024 * 1024}, // 1GB
        {Level::High, 2ULL * 1024 * 1024 * 1024} // 2GB
    };
    return map.at(level);
}

int getIoWeight(Level level)
{
    static const std::map<Level, int> map = {
        {Level::Low, 10},
        {Level::Middle, 30},
        {Level::High, 50}};
    return map.at(level);
}
}

// 定义操作类型：接受 pid, tid, level，返回错误码
using ResourceOperation = std::function<int(int pid, int tid, Level level)>;
using SelectOperation = std::function<bool(int pid, int tid)>;

// 构建策略映射表
class ResourceLimiter
{
public:
    ResourceLimiter(CGroupManager *cgroupManager, std::map<int, LimitInfo> &indexComparison, std::atomic<int> &nextIndex)
    {
        // 执行
        // 进程级别
        m_resourceOperations[{"cpu", false}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int, Level level) -> int {
            double percent = getCpuPercent(level);
            int weight = getCpuWeight(level);
            int ret1 = cgroupManager->LimitProcessCPUPercent(pid, percent, "user");
            int ret2 = cgroupManager->LimitProcessCPUWeight(pid, weight, "user");
            if (ret1 != -1 && ret2 != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, 0, "cpu", std::vector<int>{ret1, ret2}});
                return index;
            } else {
                PRINT_NOTICE("resource limit process cpu failed, pid : %d,  percent : %f, weight : %d, level : %d\n", pid, percent, weight, static_cast<int>(level));
                if (ret1 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret1);
                }
                if (ret2 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret2);
                }
            }
            return -1;
        };
        m_resourceOperations[{"memory", false}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int, Level level) -> int {
            quint64 mem = getMemoryMax(level);
            int ret = cgroupManager->LimitProcessMemoryMax(pid, mem, "user");
            if (ret != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, 0, "memory", std::vector<int>{ret}});
                return index;
            } else {
                PRINT_NOTICE("resource limit process memory failed, pid : %d,  mem : %llu, level : %d\n", pid, mem, static_cast<int>(level));
            }
            return -1;
        };
        m_resourceOperations[{"io", false}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int, Level level) -> int {
            int weight = getIoWeight(level);
            int ret = cgroupManager->LimitProcessIOWeight(pid, weight, "user");
            if (ret != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, 0, "io", std::vector<int>{ret}});
                return index;
            } else {
                PRINT_NOTICE("resource limit process io failed, pid : %d,  mem : %d, level : %d\n", pid, weight, static_cast<int>(level));
            }
            return -1;
        };
        m_resourceOperations[{"all", false}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int, Level level) -> int {
            double percent = getCpuPercent(level);
            int cpuWeight = getCpuWeight(level);
            quint64 mem = getMemoryMax(level);
            int ioWeight = getIoWeight(level);

            int ret1 = cgroupManager->LimitProcessCPUPercent(pid, percent, "user");
            int ret2 = cgroupManager->LimitProcessCPUWeight(pid, cpuWeight, "user");
            int ret3 = cgroupManager->LimitProcessMemoryMax(pid, mem, "user");
            int ret4 = cgroupManager->LimitProcessIOWeight(pid, ioWeight, "user");
            if (ret1 != -1 && ret2 != -1 && ret3 != -1 && ret4 != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, 0, "all", std::vector<int>{ret1, ret2, ret3, ret4}});
                return index;
            } else {
                PRINT_NOTICE("resource limit process all failed, pid : %d,  cpu percent : %f, cpu weight : %d, mem : %llu, io weight : %d, level : %d\n", pid, percent, cpuWeight, mem, ioWeight, static_cast<int>(level));
                if (ret1 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret1);
                }
                if (ret2 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret2);
                }
                if (ret3 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret3);
                }
                if (ret4 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret4);
                }
            }
            return -1;
        };

        // 线程级别
        m_resourceOperations[{"cpu", true}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int tid, Level level) -> int {
            double percent = getCpuPercent(level);
            int weight = getCpuWeight(level);
            int ret1 = cgroupManager->LimitThreadCPUPercent(pid, tid, percent, "user");
            int ret2 = cgroupManager->LimitThreadCPUWeight(pid, tid, weight, "user");
            if (ret1 != -1 && ret2 != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, tid, "cpu", std::vector<int>{ret1, ret2}});
                return index;
            } else {
                PRINT_NOTICE("resource limit thread cpu failed, pid : %d, tid : %d,  percent : %f, weight : %d, level : %d\n", pid, tid, percent, weight, static_cast<int>(level));
                if (ret1 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret1);
                }
                if (ret2 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret2);
                }
            }
            return -1;
        };
        m_resourceOperations[{"memory", true}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int tid, Level level) -> int {
            quint64 mem = getMemoryMax(level);
            int ret = cgroupManager->LimitThreadMemoryMax(pid, tid, mem, "user");
            if (ret != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, tid, "memory", std::vector<int>{ret}});
                return index;
            } else {
                PRINT_NOTICE("resource limit thread memory failed, pid : %d, tid : %d,  mem : %llu, level : %d\n", pid, tid, mem, static_cast<int>(level));
            }
            return -1;
        };
        m_resourceOperations[{"io", true}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int tid, Level level) -> int {
            int weight = getIoWeight(level);
            int ret = cgroupManager->LimitThreadIOWeight(pid, tid, weight, "user");
            if (ret != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, tid, "io", std::vector<int>{ret}});
                return index;
            } else {
                PRINT_NOTICE("resource limit thread io failed, pid : %d, tid : %d,  mem : %d, level : %d\n", pid, tid, weight, static_cast<int>(level));
            }
            return -1;
        };
        m_resourceOperations[{"all", true}] = [cgroupManager, &indexComparison, &nextIndex](int pid, int tid, Level level) -> int {
            double percent = getCpuPercent(level);
            int cpuWeight = getCpuWeight(level);
            quint64 mem = getMemoryMax(level);
            int ioWeight = getIoWeight(level);

            int ret1 = cgroupManager->LimitThreadCPUPercent(pid, tid, percent, "user");
            int ret2 = cgroupManager->LimitThreadCPUWeight(pid, tid, cpuWeight, "user");
            int ret3 = cgroupManager->LimitThreadMemoryMax(pid, tid, mem, "user");
            int ret4 = cgroupManager->LimitThreadIOWeight(pid, tid, ioWeight, "user");
            if (ret1 != -1 && ret2 != -1 && ret3 != -1 && ret4 != -1) {
                int index = nextIndex++;
                indexComparison.emplace(index, LimitInfo{pid, tid, "all", std::vector<int>{ret1, ret2, ret3, ret4}});
                return index;
            } else {
                PRINT_NOTICE("resource limit thread all failed, pid : %d, tid : %d,  cpu percent : %f, cpu weight : %d, mem : %llu, io weight : %d, level : %d\n", pid, tid, percent, cpuWeight, mem, ioWeight, static_cast<int>(level));
                if (ret1 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret1);
                }
                if (ret2 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret2);
                }
                if (ret3 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret3);
                }
                if (ret4 != -1) {
                    cgroupManager->ReleaseLimitByIndex(ret4);
                }
            }
            return -1;
        };

        // 查询
        // 进程级别
        m_selectOperations[{"cpu", false}] = [cgroupManager, &indexComparison](int pid, int) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == 0 && (ic.second.res_type == "cpu" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"memory", false}] = [cgroupManager, &indexComparison](int pid, int) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == 0 && (ic.second.res_type == "memory" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"io", false}] = [cgroupManager, &indexComparison](int pid, int) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == 0 && (ic.second.res_type == "cpu" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"all", false}] = [cgroupManager, &indexComparison](int pid, int) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == 0 && ic.second.res_type == "all") {
                    return true;
                }
            }
            return false;
        };
        // 线程级别
        m_selectOperations[{"cpu", true}] = [cgroupManager, &indexComparison](int pid, int tid) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == tid && (ic.second.res_type == "cpu" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"memory", true}] = [cgroupManager, &indexComparison](int pid, int tid) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == tid && (ic.second.res_type == "memory" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"io", true}] = [cgroupManager, &indexComparison](int pid, int tid) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == tid && (ic.second.res_type == "io" || ic.second.res_type == "all")) {
                    return true;
                }
            }
            return false;
        };
        m_selectOperations[{"all", true}] = [cgroupManager, &indexComparison](int pid, int tid) -> int {
            for (auto &ic : indexComparison) {
                if (ic.second.pid_ == pid && ic.second.tid_ == tid && ic.second.res_type == "all") {
                    return true;
                }
            }
            return false;
        };
    }

    int execute(const QString &resType, int pid, int tid, Level level) const
    {
        bool isThread = (tid != 0);
        auto key = std::make_pair(resType, isThread);
        auto it = m_resourceOperations.find(key);
        if (it == m_resourceOperations.end()) {
            return -1; // 不支持的资源类型
        }
        return it->second(pid, tid, level);
    }

    bool inquiry(const QString &resType, int pid, int tid) const
    {
        bool isThread = (tid != 0);
        auto key = std::make_pair(resType, isThread);
        auto it = m_selectOperations.find(key);
        if (it == m_selectOperations.end()) {
            return -1; // 不支持的资源类型
        }
        return it->second(pid, tid);
    }

private:
    std::map<std::pair<QString, bool>, ResourceOperation> m_resourceOperations;
    std::map<std::pair<QString, bool>, SelectOperation> m_selectOperations;
};

ResourceLimit::ResourceLimit(CGroupManager *cgroupManager, QObject *parent)
    : m_cgroupManager(cgroupManager)
    , m_resourceLimiter(new ResourceLimiter(m_cgroupManager, m_indexComparison, m_nextIndex))
    , QObject(parent)
{
}

ResourceLimit::~ResourceLimit()
{
    delete m_resourceLimiter;
}

int ResourceLimit::LimitResource(int pid, int tid, const QString &resType, QString level)
{
    Level lvl = levelFromString(level);
    return m_resourceLimiter->execute(resType, pid, tid, lvl);
}

bool ResourceLimit::FreezeProcess(int pid, int tid, bool freeze)
{
    static std::map<std::pair<int, int>, int> freezeIndex;
    if (freeze) {
        int index = -1;
        if (tid == 0) {
            index = m_cgroupManager->FreezeProcess(pid, "user");
        } else {
            index = m_cgroupManager->FreezeThread(pid, tid, "user");
        }
        if (index == -1) {
            return false;
        }
        freezeIndex[{pid, tid}] = index;
    } else {
        auto it = freezeIndex.find({pid, tid});
        if (it != freezeIndex.end()) {
            m_cgroupManager->ReleaseLimitByIndex(it->second);
        } else {
            return false;
        }
    }

    return true;
}

bool ResourceLimit::ReleaseLimit(int index)
{
    auto it = m_indexComparison.find(index);
    if (it == m_indexComparison.end()) {
        return false;
    }
    bool ret = true;
    for (int idx : it->second.real_limit) {
        ret &= m_cgroupManager->ReleaseLimitByIndex(idx);
    }
    return ret;
}

QString ResourceLimit::GetLimitInfo(int pid, int tid) const
{
    if (tid == 0) {
        return m_cgroupManager->GetProcessLimitInfo(pid);
    }
    return m_cgroupManager->GetThreadLimitInfo(pid, tid);
}

bool ResourceLimit::IsLimited(int pid, int tid, const QString &resType) const
{
    return m_resourceLimiter->inquiry(resType, pid, tid);
}