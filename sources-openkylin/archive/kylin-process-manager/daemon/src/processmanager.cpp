/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "processmanager.h"
#include "loghelper.h"
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

ProcessManager::ProcessManager(CGroupManager *cgroupManager, QObject *parent)
    : QObject{parent}
    , m_cgroupManager(cgroupManager)
    , m_refreshRateManager(new RefreshRateManager())
{
    connect(&m_systemResourceManager, &SystemResourceManager::ResourceThresholdWarning,
            this, &ProcessManager::ResourceThresholdWarning);
    m_systemResourceManager.startWatchers();
    initInputIdle();
    initCpuFreq();
    setcpuset();
    initSamplingRate();
    initKernelSched();
    initDirty();
    initNetParams();
}

void ProcessManager::initKernelSched()
{
    std::string params[] = {
        "kernel/sched_min_granularity_ns",
        "kernel/sched_wakeup_granularity_ns"
    };
    std::ifstream file("/proc/sys/" + params[0]);
    if (!file.is_open()) {
        return;
    }
    file >> m_kernelschedoriginalvalues[0];
    file.close();

    std::ifstream file1("/proc/sys/" + params[1]);
    if (!file1.is_open()) {
        return;
    }
    file1 >> m_kernelschedoriginalvalues[1];
    file1.close();
    // for (int i = 0; i < 2; ++i) {
    //     if (!read_sysctl_value(params[i], original_values[i])) {
    //         std::cerr << "无法读取参数: " << params[i] << std::endl;
    //         return 1;
    //     }
    //     std::cout << "原始值 " << params[i] << ": " << original_values[i] << std::endl;
    // }
}
void ProcessManager::initDirty()
{
    std::string params[] = {
        "vm/dirty_ratio",
        "vm/dirty_background_ratio",
        "vm/dirty_writeback_centisecs",
        "vm/dirty_expire_centisecs"
    };
    for (int i = 0; i < 4; ++i) {
        std::ifstream file("/proc/sys/" + params[i]);
        if (!file.is_open()) {
            continue;
        }
        file >> m_dirtyoriginalvalues[i];
        file.close();
    }
}
void ProcessManager::initNetParams()
{
    std::string params[] = {
        "ipv4/conf/all/arp_ignore",
        "ipv4/conf/all/drop_gratuitous_arp",
        "ipv4/conf/all/arp_filter",
        "ipv6/icmp/echo_ignore_all",
        "ipv6/icmp/echo_ignore_anycast",
        "ipv6/icmp/echo_ignore_multicast"
    };
    for (int i = 0; i < 6; ++i) {
        std::ifstream file("/proc/sys/net/" + params[i]);
        if (!file.is_open()) {
            continue;
        }
        file >> m_netparamsoriginalvalues[i];
        file.close();
    }
}
void ProcessManager::initSamplingRate()
{
    std::string filePath = "/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate";
    std::ifstream inFile(filePath);
    
    // 检查文件是否存在
    if (!inFile.is_open()) {
        std::cerr << "文件 " << filePath << " 不存在." << std::endl;
        return;
    }   

    // 读取文件内容
    std::getline(inFile, m_originSamplingRate);
    inFile.close();

}

void ProcessManager::initInputIdle()
{
    connect(&m_listener, &InputMonitor::idle, this, [this] {
        Q_EMIT IdleStatus(true);
    });
    connect(&m_listener, &InputMonitor::active, this, [this] {
        Q_EMIT IdleStatus(false);
    });
}

void ProcessManager::setcpuset()
{
    std::string filePath = "/sys/fs/cgroup/cpuset/cgroup.clone_children";
    
    // 打开文件
    std::ofstream file(filePath);
    
    // 检查文件是否成功打开
    if (!file.is_open()) {
        PRINT_NOTICE("open /sys/fs/cgroup/cpuset/cgroup.clone_children error!\n");
        return;
    }
    
    // 写入值1到文件
    file << "1";
    
    // 检查写入是否成功
    if (file.fail()) {
        PRINT_NOTICE("write /sys/fs/cgroup/cpuset/cgroup.clone_children error!\n");
        file.close();
        return;
    }
    
    // 关闭文件
    file.close();
    
}
void ProcessManager::initCpuFreq()
{
    // 获取所有CPU核心
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir("/sys/devices/system/cpu")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string name(ent->d_name);
            if (name.find("cpu") == 0 && name.size() > 3) {
                m_cpuCores.push_back(name);
            }
        }
        closedir(dir);
    } 

    //std::map<int, int> m_maxFreqMap;
    //std::map<int, int> m_minFreqMap;

    // 读取每个CPU核心的频率信息
    for (const auto& core : m_cpuCores) {
        //read_frequencies(core, max_freq_map, min_freq_map);
        std::ifstream file("/sys/devices/system/cpu/" + core + "/cpufreq/scaling_available_frequencies");
        if (!file.is_open()) {
            std::cerr << "无法打开文件: /sys/devices/system/cpu/" << core << "/cpufreq/scaling_available_frequencies" << std::endl;
            continue;;
        }

        std::string line;
        std::getline(file, line);
        file.close();

        std::istringstream iss(line);
        std::vector<int> frequencies;
        int freq;
        int max_freq = 0;
        int min_freq = 0;

        while (iss >> freq) {
            frequencies.push_back(freq);
        }
        if (frequencies.empty()) {
            return;
        }

        min_freq = *std::min_element(frequencies.begin(), frequencies.end());
        max_freq = *std::max_element(frequencies.begin(), frequencies.end());

        int cpu_number = std::stoi(core.substr(3)); // 提取CPU编号
        m_maxFreqMap[cpu_number] = max_freq;
        m_minFreqMap[cpu_number] = min_freq;
    }
    for (const auto& pair : m_maxFreqMap) {
        PRINT_DBG("initCpuFreq maxfreq -- %d: %d khz\n", pair.first, pair.second);
        //std::cout << "CPU " << pair.first << ": " << pair.second << " kHz" << std::endl;
    }
    for (const auto& pair : m_minFreqMap) {
        //std::cout << "CPU " << pair.first << ": " << pair.second << " kHz" << std::endl;
        PRINT_DBG("initCpuFreq minfreq -- %d: %d khz\n", pair.first, pair.second);
    }
}

void ProcessManager::SetCurrentScene(const QStringList &sceneids)
{
    m_sceneids = sceneids;
    Q_EMIT SceneChanged(sceneids);
}

QStringList ProcessManager::GetCurrentScene()
{
    return m_sceneids;
}

void ProcessManager::LockGSettingsKey(const QString &key, bool lock)
{
    const char *lock_file = "/etc/dconf/db/local.d/locks/90-processmanager-lock";
    const char *dconf_update = "dconf update";
    const char *lock_dir = "/etc/dconf/db/local.d/locks";

    std::string mkdir_cmd = "mkdir -p " + std::string(lock_dir);
    if (std::system(mkdir_cmd.c_str()) != 0) {
        PRINT_NOTICE("%s failed\n", mkdir_cmd.c_str());
        return;
    }

    if (lock) {
        std::string cmd = "echo '" + key.toStdString() + "' | sudo tee -a " + lock_file + " > /dev/null";
        if (std::system(cmd.c_str()) != 0) {
            PRINT_NOTICE("%s failed\n", cmd.c_str());
            return;
        }
        PRINT_DBG("%s sucessed\n", cmd.c_str());
    } else {
        std::string escapedKey = key.toStdString();
        size_t pos = 0;
        while ((pos = escapedKey.find('/', pos)) != std::string::npos) {
            escapedKey.replace(pos, 1, "\\/");
            pos += 2;
        }
        std::string cmd = "sudo sed -i '/" + escapedKey + "/d' " + lock_file;
        if (std::system(cmd.c_str()) != 0) {
            PRINT_NOTICE("%s failed\n", cmd.c_str());
            return;
        }
        PRINT_DBG("%s sucessed\n", cmd.c_str());
    }

    if (std::system(dconf_update) != 0) {
        PRINT_NOTICE("%s failed\n", dconf_update);
        return;
    }
    PRINT_DBG("%s sucessed\n", dconf_update);
}

void ProcessManager::ClosePSR()
{
    PRINT_DBG("ProcessManager::ClosePSR()");
    if(m_refreshRateManager->isSupportPSR()) {
        m_refreshRateManager->closePSR();
    } else {
        PRINT_DBG("ProcessManager::ClosePSR() : not support PSR\n");
    }
}

void ProcessManager::OpenPSR()
{
    PRINT_DBG("ProcessManager::OpenPSR()");
    if(m_refreshRateManager->isSupportPSR()) {
        m_refreshRateManager->openPSR();
    } else {
        PRINT_DBG("ProcessManager::OpenPSR() : not support PSR\n");
    }
}

void ProcessManager::SetVRRRefreshRate(QString mode)
{
    PRINT_DBG("ProcessManager::SetVRRRefreshRate(): mode %s\n", mode.toStdString().c_str());
    if(m_refreshRateManager->isSupportVRR()) {
        m_refreshRateManager->setVRRRefreshRate(mode.toStdString());
    } else {
        PRINT_DBG("ProcessManager::SetVRRRefreshRate() : not support VRR\n");
    }
}

void ProcessManager::ReverseVRRRefreshRate()
{
    PRINT_DBG("ProcessManager::ReverseRefreshRate()");
    if(m_refreshRateManager->isSupportVRR()) { 
        m_refreshRateManager->reverseVRRRefreshRate();
    } else {
        PRINT_DBG("ProcessManager::ReverseRefreshRate() : not support VRR\n");
    }
}

void ProcessManager::SetSystemdUnitRun(const QString &unitName, bool start)
{
    m_systemdUnitManager.setSystemdUnitRun(unitName, start);
}

void ProcessManager::ReclaimProcesses(const QList<int> &pids) const
{
    QString errorMessage = m_systemResourceManager.reclaimProcesses(pids);
    if (!errorMessage.isEmpty()) {
        sendErrorReply(QDBusError::ErrorType::Failed, errorMessage);
    }
}

void ProcessManager::ReclaimProcessGroups(const QStringList &groupNames) const
{
    QList<int> pids;
    for (const auto &groupName : groupNames) {
        pids << m_cgroupManager->getGroupPids(groupName);
    }

    QString errorMessage = m_systemResourceManager.reclaimProcesses(pids);
    if (!errorMessage.isEmpty()) {
        sendErrorReply(QDBusError::ErrorType::Failed, errorMessage);
    }
}

double ProcessManager::GetCpuEnergyConsumption() const
{
    if (!m_cpuEnergyMeter.isAvailable()) {
        // sendErrorReply(QDBusError::ErrorType::NotSupported,
        //                "The intel rapl interface is not supported.");
        return 0;
    }

    return m_cpuEnergyMeter.energyConsumption();
}

void ProcessManager::SetSystemCpuFreq(bool cpufreq)
{
    if (cpufreq)
    {
        for (const auto& pair : m_minFreqMap) {
            std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(pair.first) + "/cpufreq/scaling_max_freq";
            std::ofstream file(path);
            if (file.is_open()) {
                file << pair.second << std::endl;
                file.close();
                //std::cout << "设置CPU " << pair.first << " 的最小频率为 " << pair.second << " kHz" << std::endl;
                PRINT_DBG("SetSystemCpuFreq set minfreq -- %d: %d khz\n", pair.first, pair.second);
            }
        }
    }
    else
    {
        for (const auto& pair : m_maxFreqMap) {
            std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(pair.first) + "/cpufreq/scaling_max_freq";
            std::ofstream file(path);
            if (file.is_open()) {
                file << pair.second << std::endl;
                file.close();
                //std::cout << "设置CPU " << pair.first << " 的最大频率为 " << pair.second << " kHz" << std::endl;
                PRINT_DBG("SetSystemCpuFreq set maxfreq -- %d: %d khz\n", pair.first, pair.second);
            }
        }
        
    }
}
void ProcessManager::SetKernelSched(bool kernelsched)
{
    PRINT_DBG("SetKernelSched,kernelsched = %d\n", kernelsched);
    std::string params[] = {
    "kernel/sched_min_granularity_ns",
    "kernel/sched_wakeup_granularity_ns"
    };
    if(kernelsched)
    {
        std::ofstream file("/proc/sys/" + params[0]);
        if (!file.is_open()) {
            return;
        }
        long long new_value = m_kernelschedoriginalvalues[0] * 100;
        file << new_value;
        file.close();

        std::ofstream file1("/proc/sys/" + params[1]);
        if (!file1.is_open()) {
            return;
        }
        new_value = m_kernelschedoriginalvalues[1] * 100;
        file1 << new_value;
        file1.close();
    }
    else
    {
        std::ofstream file("/proc/sys/" + params[0]);
        if (!file.is_open()) {
            return;
        }
        //long long new_value = m_original_values[0] * 100;
        file << m_kernelschedoriginalvalues[0];
        file.close();

        std::ofstream file1("/proc/sys/" + params[1]);
        if (!file1.is_open()) {
            return;
        }
        //new_value = m_original_values[1] * 100;
        file1 << m_kernelschedoriginalvalues[1];
        file1.close();
    }
}
void ProcessManager::SetDirty(bool dirty)
{
    PRINT_DBG("SetDirty,dirty = %d\n", dirty);
    std::string params[] = {
        "vm/dirty_ratio",
        "vm/dirty_background_ratio",
        "vm/dirty_writeback_centisecs",
        "vm/dirty_expire_centisecs"
    };
    long int new_value[] = {80,80,500000,500000};
    if(dirty)
    {
        for(int i =0 ;i < 4;i++)
        { 
            std::ofstream file("/proc/sys/" + params[i]);
            if (!file.is_open()) {
                continue;
            }
            //long long new_value = m_originalvalues[0] * 100;
            file << new_value[i];
            file.close();
        }
    }
    else
    {
         for(int i =0 ;i < 4;i++)
        { 
            std::ofstream file("/proc/sys/" + params[i]);
            if (!file.is_open()) {
                continue;
            }
            //long long new_value = m_originalvalues[0] * 100;
            file << m_dirtyoriginalvalues[i];
            file.close();
        }
    }
}
void ProcessManager::SetNetParams(bool netparams)
{
    PRINT_DBG("SetNetParams,netparams = %d\n", netparams);
    std::string params[] = {
        "ipv4/conf/all/arp_ignore",
        "ipv4/conf/all/drop_gratuitous_arp",
        "ipv4/conf/all/arp_filter",
        "ipv6/icmp/echo_ignore_all",
        "ipv6/icmp/echo_ignore_anycast",
        "ipv6/icmp/echo_ignore_multicast"
    };
    long int new_value = 1;
    if(netparams)
    {
        for(int i =0 ;i < 6;i++)
        { 
            std::ofstream file("/proc/sys/net/" + params[i]);
            if (!file.is_open()) {
                continue;
            }
            //long long new_value = m_originalvalues[0] * 100;
            file << new_value;
            file.close();
        }
    }
    else
    {
         for(int i =0 ;i < 6;i++)
        { 
            std::ofstream file("/proc/sys/net/" + params[i]);
            if (!file.is_open()) {
                continue;
            }
            //long long new_value = m_originalvalues[0] * 100;
            file << m_netparamsoriginalvalues[i];
            file.close();
        }
    }
}
pid_t ProcessManager::find_pid_by_name(const std::string& process_name) {
    DIR* dir;
    struct dirent* ent;
    char buf[512];

    if ((dir = opendir("/proc")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // Check if the entry is a directory and represents a process ID
            if (ent->d_type == DT_DIR && std::isdigit(ent->d_name[0])) {
                pid_t pid = static_cast<pid_t>(std::stoi(ent->d_name));
                snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
                std::ifstream cmdline(buf);
                std::string cmd;
                if (cmdline >> cmd && cmd.find(process_name) != std::string::npos) {
                    closedir(dir);
                    return pid;
                }
            }
        }
        closedir(dir);
    } else {
        perror("opendir");
    }

    return -1; // Process not found
}
double ProcessManager::parse_cpu_usage(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    int count = 0;
    double cpu_usage = 0.0;

    // Skip the first 8 fields
    for (int i = 0; i < 8; ++i) {
        iss >> token;
    }

    // The 9th field is the CPU usage
    iss >> cpu_usage;
    return cpu_usage;
}
void ProcessManager::SetCpuLimit(bool cpulimit)
{
    PRINT_DBG("SetCpuLimit,cpulimit = %d\n", cpulimit);
    if(cpulimit)
    {
        m_pid = fork();
        if(m_pid == 0)
        {
            std::string process_name = "/opt/kingsoft/wps-office/office6/wpp";
            pid_t pid_wpp = find_pid_by_name(process_name);
            if (pid_wpp == -1) {
                //std::cerr << "Process not found!" << std::endl;
                PRINT_NOTICE("SetCpuLimit,Process not found\n");
                return;
            }
            std::string command = "top -bn5 -p " + std::to_string(pid_wpp) + " | grep " + std::to_string(pid_wpp);
            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) {
                //std::cerr << "Failed to run command" << std::endl;
                PRINT_NOTICE("SetCpuLimit,Failed to run command\n");
                return;
            }

            std::vector<double> cpu_usages;
            std::string line;
            char buffer[128];
            int temp_first = 0;
            // Read the output of the command
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                line = buffer;
                // Skip the first result
                if ((cpu_usages.size() == 0)&&(temp_first == 0)) {
                    temp_first = 1;
                    continue;
                }
                double cpu_usage = parse_cpu_usage(line);
                cpu_usages.push_back(cpu_usage);
            }

            pclose(pipe);
            // Calculate the average CPU usage
            if (cpu_usages.size() < 4) {
                //std::cerr << "Not enough data to calculate average CPU usage" << std::endl;
                return;
            }

            double sum = 0.0;
            for (double usage : cpu_usages) {
                sum += usage;
            }

            int average_cpu_usage = int(sum / cpu_usages.size()/3);
            std::cout << "Average CPU Usage: " << average_cpu_usage << "%" << std::endl;
            PRINT_DBG("SetCpuLimit,average_cpu_usage = %d\n", average_cpu_usage);
            if (average_cpu_usage < 5)
            {
                return;
            }
            char str_num1[12];
            char str_num2[12];
            snprintf(str_num1, sizeof(str_num1), "%d", pid_wpp);
            snprintf(str_num2, sizeof(str_num2), "%d", average_cpu_usage);
            // 使用execl调用当前目录下的a.out
            execl("/usr/bin/cpulimit_fork", "cpulimit_fork",str_num1,str_num2, (char *)NULL);
            PRINT_DBG("SetCpuLimit,execl failed\n");
            // // 如果execl成功，这里的代码不会被执行
            perror("execl failed");
            return;
        }
    }
    else
    {
        int status;
         if (waitpid(m_pid, &status, WNOHANG) == 0) {
            // kill掉子进程
            kill(m_pid, SIGINT);
            PRINT_DBG("SetCpuLimit,kill m_pid SIGINT\n");
        }
    }
}
void ProcessManager::SetSamplingRate(bool samplingrate)
{
    PRINT_DBG("SetSamplingRate,samplingrate = %d\n", samplingrate);
    std::string filePath = "/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate";
    std::ifstream inFile(filePath);
    
    // 检查文件是否存在
    if (!inFile.is_open()) {
        std::cerr << "文件 " << filePath << " 不存在." << std::endl;
        return;
    }   

    if (samplingrate)
    {
        // 写入新值
        std::ofstream outFile(filePath);
        if (!outFile.is_open()) {
            std::cerr << "无法打开文件 " << filePath << " 进行写入." << std::endl;
            return;
        }   

        outFile << "500000" << std::endl;
        outFile.close();
    }
    else
    {
        // 恢复原始值
        std::ofstream outFile(filePath);
        if (!outFile.is_open()) {
            std::cerr << "无法打开文件 " << filePath << " 进行写入." << std::endl;
            return;
        }

        outFile << m_originSamplingRate << std::endl;
        outFile.close();
    }
}
