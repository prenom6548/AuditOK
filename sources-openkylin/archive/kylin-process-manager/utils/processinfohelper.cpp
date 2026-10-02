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

#include "processinfohelper.h"
#include "loghelper.h"
#include <QDebug>
#include <QFile>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <libproc2/pids.h> // libproc2 的主头文件
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DEFULT_CGROUP_MAP \
    {                     \
        {                 \
            "", ""        \
        }                 \
    }

namespace
{
void collectSocketInodes(int pid, std::unordered_set<unsigned long> &out)
{
    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd";
    DIR *dir = opendir(fdPath.c_str());
    if (!dir)
        return;

    dirent *entry;
    char linkBuf[4096] = {0};

    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string linkPath = fdPath + "/" + entry->d_name;
        ssize_t len = readlink(linkPath.c_str(), linkBuf, sizeof(linkBuf) - 1);
        if (len <= 0)
            continue;

        linkBuf[len] = '\0';
        if (strstr(linkBuf, "socket:") == linkBuf) {
            unsigned long inode;
            if (sscanf(linkBuf + 7, "[%lu]", &inode) == 1) {
                out.insert(inode);
            }
        }
    }
    closedir(dir);
}

std::unordered_set<unsigned long> readSystemUdpInodes()
{
    std::unordered_set<unsigned long> udpInodes;
    std::ifstream udpFile("/proc/net/udp");
    if (!udpFile.is_open())
        return udpInodes;

    std::string line;
    std::getline(udpFile, line);

    while (std::getline(udpFile, line)) {
        std::istringstream iss(line);
        std::vector<std::string> fields;
        std::string field;

        while (iss >> field)
            fields.push_back(field);

        if (fields.size() < 10)
            continue;

        char *endPtr;
        unsigned long inode = strtoul(fields[9].c_str(), &endPtr, 10);
        if (endPtr == fields[9].c_str() || *endPtr != '\0') {
            continue;
        }
        udpInodes.insert(inode);
    }
    return udpInodes;
}

} // namespace

namespace process_info_helper
{

std::string cmdline(int pid)
{
    // 1. 防御性检查：PID非法直接返回空
    if (pid <= 0) {
        PRINT_NOTICE("Invalid PID for cmdline: %d\n", pid);
        return "";
    }

    std::string cmdlineFilePath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlineFilePath);

    // 2. 检查文件是否成功打开
    if (!cmdlineFile.is_open()) {
        return "";
    }

    std::stringstream buffer;
    // 3. 读取文件内容（处理读取失败场景）
    buffer << cmdlineFile.rdbuf();
    std::string cmdline = buffer.str();

    // 4. 提前处理空字符串，避免后续越界
    if (cmdline.empty()) {
        return "";
    }

    // 5. 替换所有 '\0' 为空格（标准库更高效、简洁）
    std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');

    // 6. 安全移除末尾空格（先检查非空 + 最后一个字符是空格）
    // 用 [] 替代 at()：at() 抛异常，[] 是未定义行为，但提前检查后无风险
    while (!cmdline.empty() && cmdline.back() == ' ') {
        cmdline.pop_back();
    }

    return cmdline;
}

std::string commContent(int pid)
{
    // 防御性检查：PID非法直接返回空
    if (pid <= 0) {
        PRINT_NOTICE("Invalid PID: %d\n", pid);
        return "";
    }

    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(commPath);
    // 检查文件是否成功打开
    if (!commFile.is_open()) {
        PRINT_NOTICE("commFile open failed! PID: %d\n", pid);
        return "";
    }

    std::stringstream buffer;
    // 读取文件内容（处理读取失败场景）
    buffer << commFile.rdbuf();
    std::string name = buffer.str();

    // ===== 修复核心：所有操作前先检查字符串非空 =====
    if (name.empty()) {
        PRINT_NOTICE("Process name is empty! PID: %d\n", pid);
        return "";
    }

    // 移除末尾的换行符（先检查长度 >=1）
    if (name.back() == '\n') {
        name.pop_back();
        // 移除后可能变为空，需重新检查
        if (name.empty()) {
            return "";
        }
    }

    // 替换空字符为空格（修复 end()-1 越界）
    // 正确写法：replace 整个字符串范围，无需 end()-1
    std::replace(name.begin(), name.end(), '\0', ' ');

    // 清理可能的首尾空格（可选，增强鲁棒性）
    size_t start = name.find_first_not_of(" \t");
    size_t end = name.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
        name = name.substr(start, end - start + 1);
    } else {
        // 全是空格，返回空
        name = "";
    }

    return name;
}

bool processExists(int pid)
{
    std::ifstream file("/proc/" + std::to_string(pid));
    return file.good();
}

int parentPid(int pid)
{
    QString procStatus = QString("/proc/%1/status").arg(pid);
    QFile file(procStatus);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QTextStream ts(&file);
    QString line;
    while (ts.readLineInto(&line)) {
        if (line.contains("PPid:")) {
            QString strPpid = line.split(":").last();
            strPpid.remove(QChar::Space);
            return strPpid.toUInt();
        }
    }
    return -1;
}

static bool isNumeric(const char* str) {
    if (!str || *str == '\0') return false;
    while (*str) {
        if (*str < '0' || *str > '9') return false;
        str++;
    }
    return true;
}

static pid_t getParentPid(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE* fp = fopen(path, "r");
    if (!fp) return -1;

    pid_t ppid = -1;
    fscanf(fp, "%*d %*s %*c %d", &ppid);
    fclose(fp);
    return ppid;
}

static void buildAllProcessMap(std::unordered_map<pid_t, pid_t>& pidMap) {
    DIR* dir = opendir("/proc");
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir))) {
        if (!isNumeric(ent->d_name)) continue;
        pid_t pid = atoi(ent->d_name);
        pid_t ppid = getParentPid(pid);
        if (pid > 0 && ppid > 0) {
            pidMap[pid] = ppid;
        }
    }
    closedir(dir);
}

static void collectAllChildren(pid_t parent,
    const std::unordered_map<pid_t, pid_t>& pidMap,
    std::unordered_set<pid_t>& result)
{
    for (const auto& pair : pidMap) {
        pid_t pid = pair.first;
        pid_t ppid = pair.second;
        if (ppid == parent && !result.count(pid)) {
            result.insert(pid);
            collectAllChildren(pid, pidMap, result);
        }
    }
}

std::vector<int> childPids(int parentPid)
{
    std::unordered_map<pid_t, pid_t> pidMap;
    buildAllProcessMap(pidMap);
    std::unordered_set<pid_t> childrenSet;
    collectAllChildren(parentPid, pidMap, childrenSet);
    return std::vector<int>(childrenSet.begin(), childrenSet.end());
}

std::vector<int> childPids(const std::vector<int>& parentPids)
{
    std::unordered_map<pid_t, pid_t> pidMap;
    buildAllProcessMap(pidMap);
    std::unordered_set<pid_t> childrenSet;
    for (int pid : parentPids) {
        collectAllChildren(pid, pidMap, childrenSet);
    }
    return std::vector<int>(childrenSet.begin(), childrenSet.end());
}

std::string cgroup(int pid)
{
    if (pid <= 0) {
        return std::string();
    }
    std::string cgroupFilePath = "/proc/" + std::to_string(pid) + "/cgroup";
    std::ifstream cgroupFile(cgroupFilePath);

    if (!cgroupFile.is_open()) {
        PRINT_NOTICE("cgroupFile open failed! %d\n", pid);
        return std::string();
    }

    std::string cgroup, line;
    while (std::getline(cgroupFile, line)) {
        auto lastPos = line.rfind("::");
        if (lastPos != std::string::npos) {
            cgroup = line.substr(lastPos + 2);
            break;
        }
    }
    cgroupFile.close();
    return cgroup;
}

CgroupMap cgroupMap(int pid)
{
    if (pid <= 0) {
        return DEFULT_CGROUP_MAP;
    }
    std::string cgroupFilePath = "/proc/" + std::to_string(pid) + "/cgroup";
    std::ifstream cgroupFile(cgroupFilePath);

    if (!cgroupFile.is_open()) {
        qWarning() << "cgroupFile open failed!" << pid;
        return DEFULT_CGROUP_MAP;
    }

    CgroupMap groupMap = DEFULT_CGROUP_MAP;
    std::string attr, path, line;
    while (std::getline(cgroupFile, line)) {
        auto pos = line.find(":");
        if (pos == std::string::npos) {
            cgroupFile.close();
            return groupMap;
        }
        auto pos1 = line.find(":", pos + 1);
        if (pos1 == std::string::npos) {
            cgroupFile.close();
            return groupMap;
        }
        attr = line.substr(pos + 1, pos1 - pos - 1);
        path = line.substr(pos1 + 1);
        if (path == "/")
            path = "";
        if (!path.empty()) {
            if (path.front() == '/') {
                path.erase(0, 1);
            }
            if (path.back() != '/') {
                path.append("/");
            }
        }
        if (attr.empty())
            attr = "";
        if (attr.find("systemd") != std::string::npos)
            attr = "systemd";
        auto pos2 = attr.find(",");
        if (pos2 != std::string::npos) {
            groupMap[attr.substr(0, pos2)] = path;
            groupMap[attr.substr(pos2 + 1)] = path;
        }
        groupMap[attr] = path;
    }
    cgroupFile.close();
    return groupMap;
}

std::vector<int> pidsOfCgroup(const std::string &cgroupPath)
{
    std::ifstream cgroupFile(cgroupPath + "/cgroup.procs");

    if (!cgroupFile.is_open()) {
        return {};
    }

    std::string line;
    std::vector<int> pids;
    while (std::getline(cgroupFile, line)) {
        pids.emplace_back(std::stoi(line));
    }
    cgroupFile.close();
    return pids;
}

bool isShellScript(const QString &filePath)
{
    // QProcess process;
    // process.start("file", QStringList() << "--mime-type" << filePath);
    // if (!process.waitForFinished()) {
    //     qDebug() << "执行file命令失败";
    //     return false;
    // }

    // QString output = process.readAllStandardOutput();
    // return (output.contains("text/x-shellscript") || output.contains("shell script"));
    std::ifstream file(filePath.toStdString());
    if (!file.is_open())
        return false;

    std::string firstLine;
    std::getline(file, firstLine);
    file.close();

    return firstLine.find("#!") == 0;
}

bool isDigitString(const std::string &s)
{
    if (s.empty())
        return false;
    for (char c : s) {
        if (!isdigit(c))
            return false;
    }
    return true;
}

std::vector<int> allPids()
{
    std::vector<int> pids;
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        perror("opendir(/proc) failed");
        return pids;
    }

    dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (!isDigitString(entry->d_name))
            continue;

        int pid = static_cast<int>(atoi(entry->d_name));
        if (pid > 0)
            pids.push_back(pid);
    }

    closedir(procDir);
    return pids;
}

static std::string baseName(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

std::string processName(int pid)
{
    if (pid <= 0) {
        return "";
    }

    std::string procPath = "/proc/" + std::to_string(pid) + "/";
    std::ifstream file(procPath);
    if (!file.good()) {
        return "";
    }

    std::string exeLinkPath = procPath + "exe";
    char buf[PATH_MAX];
    ssize_t len = readlink(exeLinkPath.c_str(), buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::string path(buf);
        if (path.find("python") != std::string::npos || path.find("bash") != std::string::npos) {

        } else {
            const std::string deletedSuffix = " (deleted)";
            if (path.size() >= deletedSuffix.size() && path.substr(path.size() - deletedSuffix.size()) == deletedSuffix) {
                path.erase(path.size() - deletedSuffix.size());
            }
            return baseName(path);
        }
    }

    std::string cmdlineFilePath = procPath + "cmdline";
    std::ifstream cmdlineFile(cmdlineFilePath);
    if (cmdlineFile.is_open()) {
        std::string firstArg;
        std::getline(cmdlineFile, firstArg, '\0');
        if (!firstArg.empty()) {
            return baseName(firstArg);
        }
    }

    std::string commFilePath = procPath + "comm";
    std::ifstream commlineFile(commFilePath);
    if (commlineFile.is_open()) {
        std::string comm;
        std::getline(commlineFile, comm);
        return comm;
    }

    return "";
}

std::vector<int> pidsByName(const std::string &targetName, bool exactMatch)
{
    std::vector<int> result;
    std::vector<int> pids = allPids();

    for (int pid : pids) {
        std::string procName = commContent(pid);
        if (procName.empty())
            continue;

        bool matched = false;
        if (exactMatch) {
            matched = (procName == targetName);
        } else {
            matched = (procName.find(targetName) != std::string::npos);
        }

        if (matched) {
            result.push_back(pid);
        }
    }

    return result;
}

std::unordered_map<std::string, std::vector<int>> pidsByNames(const std::vector<std::string> &targetNames, bool exactMatch)
{
    std::unordered_map<std::string, std::vector<int>> result;
    std::unordered_set<std::string> targetSet(targetNames.begin(), targetNames.end());
    std::vector<int> pids = allPids();

    for (int pid : pids) {
        std::string procName = commContent(pid);
        if (procName.empty())
            continue;

        for (const auto &target : targetNames) {
            bool matched = false;
            if (exactMatch) {
                matched = (procName == target);
            } else {
                matched = (procName.find(target) != std::string::npos);
            }
            if (matched) {
                result[target].push_back(pid);
                break;
            }
        }
    }

    return result;
}

std::vector<int> pidsByCmdline(const std::string &targetName, bool exactMatch)
{
    std::vector<int> result;
    std::vector<int> pids = allPids();

    for (int pid : pids) {
        std::string procCmdline = cmdline(pid);
        if (procCmdline.empty())
            continue;

        bool matched = false;
        if (exactMatch) {
            matched = (procCmdline == targetName);
        } else {
            matched = (procCmdline.find(targetName) != std::string::npos);
        }

        if (matched) {
            result.push_back(pid);
        }
    }

    return result;
}

std::unordered_map<std::string, std::vector<int>> pidsByCmdlines(const std::vector<std::string> &targetNames, bool exactMatch)
{
    std::unordered_map<std::string, std::vector<int>> result;
    std::unordered_set<std::string> targetSet(targetNames.begin(), targetNames.end());
    std::vector<int> pids = allPids();

    for (int pid : pids) {
        std::string procCmdline = cmdline(pid);
        if (procCmdline.empty())
            continue;

        for (const auto &target : targetNames) {
            bool matched = false;
            if (exactMatch) {
                matched = (procCmdline == target);
            } else {
                matched = (procCmdline.find(target) != std::string::npos);
            }
            if (matched) {
                result[target].push_back(pid);
                break;
            }
        }
    }

    return result;
}

int udpSocketCount(int pid)
{
    if (pid <= 0)
        return -1;

    std::unordered_set<unsigned long> procSockets;
    collectSocketInodes(pid, procSockets);
    if (procSockets.empty())
        return 0;

    std::unordered_set<unsigned long> systemUdpInodes = readSystemUdpInodes();
    if (systemUdpInodes.empty())
        return 0;

    return std::count_if(procSockets.begin(), procSockets.end(),
                         [&systemUdpInodes](unsigned long inode) {
                             return systemUdpInodes.count(inode);
                         });
}

uid_t uid(pid_t pid)
{
    std::string path = "/proc/" + std::to_string(pid);

    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return st.st_uid;
    }

    return static_cast<uid_t>(-1);
}

std::vector<pid_t> tids(pid_t pid)
{
    std::vector<pid_t> tids;
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/task", pid);

    DIR *dir = opendir(path);
    if (!dir) {
        // 进程不存在或权限不足
        return tids;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过当前目录和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 将目录名转换为整数 TID
        pid_t tid = static_cast<pid_t>(atoi(entry->d_name));
        tids.push_back(tid);
    }

    closedir(dir);
    return tids;
}

std::string getExePath(pid_t pid)
{
    char link_target[PATH_MAX];
    std::string proc_exe = "/proc/" + std::to_string(pid) + "/exe";

    ssize_t len = readlink(proc_exe.c_str(), link_target, sizeof(link_target) - 1);
    if (len == -1) {
        // 可能是内核线程、已退出、或权限不足
        return "";
    }
    link_target[len] = '\0';

    std::string path(link_target);

    // 处理 "(deleted)" 后缀（例如：/tmp/miner (deleted)）
    const std::string deleted_suffix = " (deleted)";
    if (path.size() >= deleted_suffix.size() && path.substr(path.size() - deleted_suffix.size()) == deleted_suffix) {
        path.erase(path.size() - deleted_suffix.size());
    }

    return path;
}
}
