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

#ifndef PROCESSINFOHELPER_H
#define PROCESSINFOHELPER_H

#include <QProcess>
#include <QString>
#include <string>
#include <vector>
#include <unordered_map>

using CgroupMap = std::unordered_map<std::string, std::string>;

namespace process_info_helper
{

std::string cmdline(int pid);
std::string commContent(int pid);
bool processExists(int pid);
int parentPid(int pid);
std::vector<int> childPids(int parentPid);
std::vector<int> childPids(const std::vector<int> &parentPids);
std::string cgroup(int pid);
CgroupMap cgroupMap(int pid);
std::vector<int> pidsOfCgroup(const std::string &cgroupPath);
bool isShellScript(const QString &filePath);
bool isDigitString(const std::string& s); 
std::vector<int> allPids();
std::string processName(int pid);
std::vector<int> pidsByName(const std::string& targetName, bool exactMatch = true);
std::vector<int> pidsByCmdline(const std::string& targetName, bool exactMatch = true);
std::unordered_map<std::string, std::vector<int>> pidsByNames(const std::vector<std::string>& targetNames, bool exactMatch = true);
std::unordered_map<std::string, std::vector<int>> pidsByCmdlines(const std::vector<std::string>& targetNames, bool exactMatch = true);
int udpSocketCount(int pid);
uid_t uid(pid_t pid);
std::vector<pid_t> tids(pid_t pid);
std::string getExePath(pid_t pid);
}

#endif // PROCESSINFOHELPER_H
