#include "efficiencymode.h"

#include <QDBusMetaType>
#include <QVariant>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <string>
#include <unordered_set>

#include "loghelper.h"
#include "processinfohelper.h"
#include "systemddbusinterface.h"

Q_DECLARE_METATYPE(std::unordered_set<std::string>)

namespace
{
std::unordered_set<int> pid_filter;

template<typename T>
inline T get_config_value(const std::string &key)
{
    static std::unordered_map<std::string, QVariant> configCache;
    auto it = configCache.find(key);
    if (it != configCache.end()) {
        return it->second.value<T>();
    }
    const std::string efficiencyModeConfigFile = "/etc/kylin-process-manager/efficiency-mode-configs.json";
    std::ifstream file(efficiencyModeConfigFile);
    if (!file.is_open()) {
        PRINT_NOTICE("Failed to open resource sched config file: %s\n", efficiencyModeConfigFile.c_str());
        return T();
    }

    Json::Value root;
    file >> root;
    do {
        const std::string blacklistKey = "BlackList";
        if (root.isMember(blacklistKey)) {
            Json::Value &list = root[blacklistKey];
            if (!list.isArray()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", blacklistKey.c_str());
                break;
            }
            std::unordered_set<std::string> ret;
            for (Json::Value::ArrayIndex idx = 0; idx < list.size(); ++idx) {
                ret.insert(list[idx].asString());
            }
            configCache[blacklistKey] = QVariant::fromValue(ret);
        } else {
            PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", blacklistKey.c_str());
        }
    } while (0);

    do {
        const std::string defaultlistKey = "DefaultList";
        if (root.isMember(defaultlistKey)) {
            Json::Value &list = root[defaultlistKey];
            if (!list.isArray()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", defaultlistKey.c_str());
                break;
            }
            std::unordered_set<std::string> ret;
            for (Json::Value::ArrayIndex idx = 0; idx < list.size(); ++idx) {
                ret.insert(list[idx].asString());
            }
            configCache[defaultlistKey] = QVariant::fromValue(ret);
        } else {
            PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", defaultlistKey.c_str());
        }
    } while (0);

    do {
        const std::string cpuPercentKey = "CPUPercent";
        const std::string cpuWeightKey = "CPUWeight";
        const std::string memoryMaxKey = "MemoryMax";
        const std::string ioWeightKey = "IOWeight";
        if (root.isMember(cpuPercentKey) && root.isMember(cpuWeightKey) && root.isMember(memoryMaxKey) && root.isMember(ioWeightKey)) {
            Json::Value &cpuPercent = root[cpuPercentKey];
            if (!cpuPercent.isNumeric()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", cpuPercentKey.c_str());
                configCache[cpuPercentKey] = 20.0;
            } else if (!cpuPercent.isString() && cpuPercent.asString() == "max") {
                configCache[cpuPercentKey] = -1.0;
            } else {
                configCache[cpuPercentKey] = cpuPercent.asDouble();
            }

            Json::Value &cpuWeight = root[cpuWeightKey];
            if (!cpuWeight.isNumeric()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", cpuWeightKey.c_str());
                configCache[cpuWeightKey] = 10;
            } else {
                configCache[cpuWeightKey] = cpuWeight.asInt();
            }

            Json::Value &memoryMax = root[memoryMaxKey];
            if (!memoryMax.isNumeric()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", memoryMaxKey.c_str());
                configCache[memoryMaxKey] = 512 * 1024 * 1024;
            } else if (!memoryMax.isString() && memoryMax.asString() == "max") {
                configCache[memoryMaxKey] = -1;
            } else {
                configCache[memoryMaxKey] = QVariant::fromValue(memoryMax.asInt64());
            }

            Json::Value &ioWeight = root[ioWeightKey];
            if (!ioWeight.isNumeric()) {
                PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", ioWeightKey.c_str());
                configCache[ioWeightKey] = 10;
            } else {
                configCache[ioWeightKey] = ioWeight.asInt();
            }
        } else {
            PRINT_NOTICE("Resource sched config file format is incorrect, key : %s\n", "EffiencyModeParams");
            configCache[cpuPercentKey] = 20.0;
            configCache[cpuWeightKey] = 10;
            configCache[memoryMaxKey] = 512 * 1024 * 1024;
            configCache[ioWeightKey] = 10;
        }
    } while (0);

    it = configCache.find(key);
    if (it != configCache.end()) {
        return it->second.value<T>();
    }
    return T();
}

inline bool is_can_be_set(int pid)
{
    // 判断是否存在
    if (pid_filter.count(pid) != 0) {
        return false;
    }

    if (!process_info_helper::processExists(pid)) {
        return false;
    }
    // // 判断是否为当前用户进程
    // static SystemdDbusInterface interface;
    // std::string currUid = interface.currentActiveUID();
    // std::string uid = std::to_string(process_info_helper::uid(pid));
    // if (currUid == "" || uid == "-1"){
    //     return false;
    // }
    // if (currUid != uid) {
    //     return false;
    // }
    // 判断是否为黑名单
    std::string identifier = process_info_helper::getExePath(pid);
    static std::unordered_set<std::string> blacklist = get_config_value<std::unordered_set<std::string>>("BlackList");

    if (identifier.empty() || identifier.find("python3") != std::string::npos) {
        identifier = process_info_helper::cmdline(pid);
    }
    if (identifier.empty())
        return false;
    if (blacklist.find(identifier) != blacklist.end()) {
        return false;
    }
    return true;
}
}

EfficiencyMode::EfficiencyMode(CGroupManager *cgroupManager, QObject *parent)
    : m_cgroupManager(cgroupManager)
    , QObject(parent)
{
    qRegisterMetaType<std::unordered_set<std::string>>("std::unordered_set<std::string>");
    qRegisterMetaType<QMap<int, QString>>("QMap<int,QString>");
    qDBusRegisterMetaType<QMap<int, QString>>();
    connect(m_cgroupManager, &CGroupManager::LimitChanged, this, [this](int pid, int tid, QString limitType, bool enable) {
        if (limitType == "freeze") {
            if (enable) {
                Q_EMIT ProcessModeChanged(pid, "freeze");
            } else {
                if (getProcessMode(pid)) {
                    Q_EMIT ProcessModeChanged(pid, "efficiency");
                } else {
                    Q_EMIT ProcessModeChanged(pid, "none");
                }
            }
        }
    });
    startDefaultTimer();
}

EfficiencyMode::~EfficiencyMode()
{
}

void EfficiencyMode::SetProcessMode(int pid, bool enable)
{
    if (!is_can_be_set(pid)) {
        PRINT_NOTICE("SetProcessMode failed, pid : %d\n", pid);
        return;
    }
    PRINT_DBG("SetProcessMode, pid : %d, enable : %d\n", pid, enable);
    static double cpuPercent = get_config_value<double>("CPUPercent");
    static int cpuWeight = get_config_value<int>("CPUWeight");
    static qint64 memoryMax = get_config_value<qint64>("MemoryMax");
    static int ioWeight = get_config_value<int>("IOWeight");
    if (enable) {
        int index = -1;
        if (cpuPercent != -1.0) {
            index = m_cgroupManager->LimitProcessCPUPercent(pid, cpuPercent, "user");
            if (index != -1) {
                m_limitIndex[pid].push_back(index);
            }
        }
        index = m_cgroupManager->LimitProcessCPUWeight(pid, cpuWeight, "user");
        if (index != -1) {
            m_limitIndex[pid].push_back(index);
        }
        if (memoryMax != -1) {
            index = m_cgroupManager->LimitProcessMemoryMax(pid, memoryMax, "user");
            if (index != -1) {
                m_limitIndex[pid].push_back(index);
            }
        }
        index = m_cgroupManager->LimitProcessIOWeight(pid, ioWeight, "user");
        if (index != -1) {
            m_limitIndex[pid].push_back(index);
        }
        if (!getProcessFreeze(pid)) {
            Q_EMIT ProcessModeChanged(pid, "efficiency");
        }
    } else {
        auto it = m_limitIndex.find(pid);
        if (it == m_limitIndex.end())
            return;
        for (int index : it->second) {
            m_cgroupManager->ReleaseLimitByIndex(index);
        }
        m_limitIndex.erase(it);
        if (!getProcessFreeze(pid)) {
            Q_EMIT ProcessModeChanged(pid, "none");
        }
    }
}

bool EfficiencyMode::ProcessHasFeature(int pid)
{
    bool isCanBeSet = is_can_be_set(pid);
    PRINT_DBG("ProcessHasFeature, pid : %d, isCanBeSet : %d\n", pid, isCanBeSet);
    return isCanBeSet;
}

void EfficiencyMode::SetProcessFreeze(int pid, bool enable)
{
    if (!is_can_be_set(pid)) {
        PRINT_NOTICE("SetProcessFreeze failed, pid : %d\n", pid);
        return;
    }
    PRINT_DBG("SetProcessFreeze, pid : %d, enable : %d\n", pid, enable);
    if (enable) {
        m_cgroupManager->FreezeProcess(pid, "user");
    } else {
        m_cgroupManager->thawedProcess(pid);
    }
}

bool EfficiencyMode::ProcessHasFreezeFeature(int pid)
{
    bool isCanBeSet = is_can_be_set(pid);
    PRINT_DBG("ProcessHasFreezeFeature, pid : %d, isCanBeSet : %d\n", pid, isCanBeSet);
    return isCanBeSet;
}

QString EfficiencyMode::GetProcessMode(int pid)
{
    QString ret = "none";
    if (getProcessFreeze(pid)) {
        ret = "freeze";
    } else if (getProcessMode(pid)) {
        ret = "efficiency";
    }
    PRINT_DBG("GetProcessMode, pid : %d, ret : %s\n", pid, ret.toStdString().c_str());
    return ret;
}

QMap<int, QString> EfficiencyMode::ListProcessModes()
{
    QMap<int, QString> ret;
    QList<int> freezeLists = listProcessFreezes();
    QList<int> modeLists = listProcessModes();
    for (int pid : modeLists) {
        ret[pid] = "efficiency";
    }
    for (int pid : freezeLists) {
        ret[pid] = "freeze";
    }
    return ret;
}

void EfficiencyMode::SetPidFilter(int pid, bool enable)
{
    static std::unordered_set<int> tmpefficiyProc;
    if (enable) {
        auto it = m_limitIndex.find(pid);
        if (it != m_limitIndex.end()) {
            tmpefficiyProc.insert(it->first);
            SetProcessMode(pid, false);
        }
        pid_filter.insert(pid);
    } else {
        pid_filter.erase(pid);
        if (tmpefficiyProc.count(pid) != 0) {
            SetProcessMode(pid, true);
            tmpefficiyProc.erase(pid);
        }
    }
    PRINT_DBG("pid filter : %s\n", std::accumulate(pid_filter.begin(), pid_filter.end(), std::string(), [](const std::string &acc, int val) {
                                       return acc.empty() ? std::to_string(val) : acc + "," + std::to_string(val);
                                   }).c_str());
}

bool EfficiencyMode::getProcessMode(int pid)
{
    return m_limitIndex.find(pid) != m_limitIndex.end();
}

QList<int> EfficiencyMode::listProcessModes()
{
    QList<int> ret;
    for (auto &limitIndex : m_limitIndex) {
        ret.append(limitIndex.first);
    }
    return ret;
}

bool EfficiencyMode::getProcessFreeze(int pid)
{
    return m_cgroupManager->GetProcessLimitInfo(pid).contains("Freeze");
}

QList<int> EfficiencyMode::listProcessFreezes()
{
    return m_cgroupManager->listFreezeProcs();
}

void EfficiencyMode::startDefaultTimer()
{
    connect(&m_defaultTimer, &QTimer::timeout, [this]() {
        static std::unordered_set<std::string> defaultlist = get_config_value<std::unordered_set<std::string>>("DefaultList");

        std::vector<int> pids = process_info_helper::allPids();
        for (int pid : pids) {
            std::string identifier = process_info_helper::getExePath(pid);

            if (identifier.empty() || identifier.find("python3") != std::string::npos) {
                identifier = process_info_helper::cmdline(pid);
            }
            if (identifier.empty())
                continue;

            if (defaultlist.find(identifier) != defaultlist.end()) {
                SetProcessMode(pid, true);
            }
        }
    });
    m_defaultTimer.setSingleShot(true);
    m_defaultTimer.start(60 * 1000);
}