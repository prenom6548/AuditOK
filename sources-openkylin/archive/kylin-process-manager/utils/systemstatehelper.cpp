#include "systemstatehelper.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int system_state_helper::getLoadAvg() {
    std::ifstream file("/proc/loadavg");
    if (!file.is_open()) {
        return -1;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string firstValue;
        iss >> firstValue; 

        file.close();
        return (int)std::stod(firstValue); 
    }

    file.close();
    return -1;
}

/**
 * @brief 检查系统是否处于 live CD/USB 安装环境
 * @return true 表示是安装环境
 * @return false 表示不是安装环境
 */
bool system_state_helper::isosinstaller() {

    std::ifstream file("/proc/cmdline");
    if (!file.is_open()) {
        return false;
    }

    std::string cmdline_content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

    return cmdline_content.find("casper") != std::string::npos;
}