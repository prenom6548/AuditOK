/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef KMBROWSER_UTILS_H
#define KMBROWSER_UTILS_H

#include <vector>
#include <string>
#include <filesystem>

#include "KMStorageDir.h"
#include "KMLogger.h"
#include "KMCommonUtils.h"

namespace fs = std::filesystem;

struct ExecInfo {
    std::string execPath;  
    std::string appId;     
    std::string version;   
    std::string desktopPath;
};


class KMBrowserUtils
{
public:
    /**
     * @brief 获取系统中所有已注册的浏览器可执行文件路径
     * @return 浏览器可执行文件路径列表
     */
    static std::vector<std::string> getAllBrowserExecutables();

    /**
     * @brief 从配置文件中读取已保存的浏览器路径
     * @return 浏览器可执行文件路径列表
     */
     static std::vector<ExecInfo> loadExecutablesFromFile();

private:
    /**
     * @brief 执行系统命令并获取输出结果
     * @param cmd 要执行的命令
     * @return 包含命令输出和错误码的pair
     */
    static std::pair<std::string, std::error_code> execCommand(const std::string &cmd);

    /**
     * @brief 获取已注册的浏览器桌面文件列表
     * @return 浏览器桌面文件名列表
     */
    static std::vector<std::string> getRegisteredDesktopFiles();

    /**
     * @brief 根据桌面文件名定位完整的桌面文件路径
     * @param desktopNames 桌面文件名列表
     * @return 完整的桌面文件路径列表
     */
    static std::vector<fs::path> locateDesktopFiles(const std::vector<std::string> &desktopNames);

    /**
     * @brief 从桌面文件中提取可执行文件路径
     * @param desktopPaths 桌面文件路径列表
     * @return 可执行文件路径列表
     */
    static std::vector<ExecInfo> extractExecutables(const std::vector<fs::path> &desktopPaths);

    /**
     * @brief 将浏览器可执行文件路径写入配置文件
     * @param executables 可执行文件路径列表
     */
    static void writeExecutablesToFile(const std::vector<ExecInfo> &executables);

    /**
     * @brief 解析Exec=行，提取可执行文件路径和参数
     * @param execLine Exec=行内容
     * @param info 解析结果
     */
    static void parseExecLine(const std::string& execLine, ExecInfo& info);
};

#endif // KMBROWSER_UTILS_H