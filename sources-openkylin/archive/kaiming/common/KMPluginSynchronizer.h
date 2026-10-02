/*
 * Copyright (c) KylinSoft  Co., Ltd. 2025. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#pragma once

#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

class KMPluginSynchronizer {
public:
    /**
     * @brief 同步插件到目标目录
     * @param versionDataPath 版本数据路径
     * @param exportPlugins 要同步的插件列表
     * @param targetBaseDir 目标基础目录（默认为/opt/kaiming/exports/plugins）
     * @return bool 同步是否成功
     */
    static bool syncPlugins(const fs::path& versionDataPath,
                          const std::vector<std::string>& exportPlugins,
                          std::set<std::string>& installedFiles,
                          const fs::path& targetBaseDir = "/opt/system/resource/export/kaiming");

    // 同步单个插件目录
    static bool syncSinglePlugin(const fs::path& sourcePluginDir, const fs::path& targetPluginDir, std::set<std::string>& syncedFiles);

private:
    // 读取现有插件列表
    static bool readExistingPluginsList(const fs::path& pluginsListFile, std::set<std::string>& allPlugins);
    
    // 写入插件列表到文件
    static bool writePluginsList(const fs::path& pluginsListFile, const std::set<std::string>& allPlugins);

    static std::string replaceArchWildcard(const std::string& path);

    static bool containsPlugin(const std::string& path, const std::string& pluginName);

private:
    static const std::unordered_map<std::string, std::string> g_specialPluginMappings;
};