#include "KMPluginSynchronizer.h"
#include "KMLogger.h"
#include "KMDirUtils.h"

#include <fstream>
#include <iostream>
#include <set>

const std::unordered_map<std::string, std::string> KMPluginSynchronizer::g_specialPluginMappings = {
    {"peony-extensions", "/opt/system/lib/peony-extensions/"}
};

bool KMPluginSynchronizer::syncPlugins(const fs::path& versionDataPath,
                                   const std::vector<std::string>& exportPlugins,
                                   std::set<std::string>& installedFiles,
                                   const fs::path& targetBaseDir) {

    fs::path pluginsListFile = targetBaseDir / "plugins.list";
    std::set<std::string> allPlugins;
    std::set<std::string> syncedFiles;

    try {
        if (!fs::exists(targetBaseDir)) {
            fs::create_directories(targetBaseDir);
        }

        if (fs::exists(pluginsListFile)) {
            if (!readExistingPluginsList(pluginsListFile, allPlugins)) {
                return false;
            }
        }

        std::unordered_map<std::string, std::string> specialPlugins; // plugin_name -> source_path

        // 处理 exportPlugins，支持 ':' 分割的路径
        for (const auto& plugin : exportPlugins) {
            if (plugin.empty()) continue;

            std::string targetPluginPath = plugin;
            std::string sourcePluginPath = plugin;

            size_t colonPos = plugin.find(':');
            if (colonPos != std::string::npos) {
                sourcePluginPath = plugin.substr(0, colonPos);
                targetPluginPath = plugin.substr(colonPos + 1);
                kmlogger.info("Split path - source: %s, target: %s", sourcePluginPath.c_str(), targetPluginPath.c_str());
            }

            // "/lib/${arch}/peony-extensions:/usr/lib/${arch}/peony-extensions"
            // 检查是否为特殊插件
            bool isSpecialPlugin = false;
            for (const auto &[pluginName, specialPath] : g_specialPluginMappings)
            {
                if (containsPlugin(sourcePluginPath, pluginName) ||
                    containsPlugin(targetPluginPath, pluginName))
                {

                    std::string resolvedSource = replaceArchWildcard(sourcePluginPath);
                    specialPlugins[pluginName] = resolvedSource;
                    isSpecialPlugin = true;
                    kmlogger.info("Found special plugin %s: %s", pluginName.c_str(), plugin.c_str());
                    break;
                }
            }

            if (isSpecialPlugin)
            {
                continue;
            }

            // 将目标路径添加到 allPlugins（用于记录和后续处理）
            if (!targetPluginPath.empty()) {
                allPlugins.insert(targetPluginPath);
            }

            std::string sourceRel = sourcePluginPath;
            std::string targetRel = targetPluginPath;

            if(!sourceRel.empty() && sourceRel[0] == '/') {
                sourceRel.erase(0, 1);
            }
            if(!targetRel.empty() && targetRel[0] == '/') {
                targetRel.erase(0, 1);
            }

            fs::path sourcePluginDir = versionDataPath / sourceRel;
            fs::path targetPluginDir = targetBaseDir / targetRel;

            kmlogger.info("Plugin syncing from: %s to: %s", sourcePluginDir.string().c_str(), targetPluginDir.string().c_str());

            std::set<std::string> pluginFiles;
            if (!syncSinglePlugin(sourcePluginDir, targetPluginDir, pluginFiles)) {
                kmlogger.error("Failed to sync plugin: %s", plugin.c_str());
                return false;
            }

            // 将插件文件添加到总集合中
            syncedFiles.insert(pluginFiles.begin(), pluginFiles.end());
        }

        // 处理特殊插件
        for (const auto &[pluginName, specialDest] : g_specialPluginMappings)
        {
            if (specialPlugins.find(pluginName) != specialPlugins.end())
            {
                std::string sourcePath = specialPlugins[pluginName];
                if (!sourcePath.empty() && sourcePath[0] == '/')
                {
                    sourcePath.erase(0, 1);
                }

                sourcePath = (versionDataPath / sourcePath).string();

                kmlogger.info("Special handling for %s: %s -> %s",
                              pluginName.c_str(), sourcePath.c_str(), specialDest.c_str());

                std::set<std::string> specialPluginFiles;
                if (!syncSinglePlugin(fs::path(sourcePath), fs::path(specialDest), specialPluginFiles))
                {
                    kmlogger.error("Failed to sync special plugin: %s", pluginName.c_str());
                    return false;
                }

                // 收集特殊插件的文件
                syncedFiles.insert(specialPluginFiles.begin(), specialPluginFiles.end());
                // allPlugins.insert(specialDest);
            }
        }

        // 将原有的插件也同步一次（确保完整性）
        // for (const auto& plugin : allPlugins) {
        //     // 跳过已经在上面处理过的插件
        //     bool alreadyProcessed = false;
        //     for (const auto& exported : exportPlugins) {
        //         size_t colonPos = exported.find(':');
        //         std::string targetPath = (colonPos != std::string::npos) ? exported.substr(colonPos + 1) : exported;
        //         if (targetPath == plugin) {
        //             alreadyProcessed = true;
        //             break;
        //         }
        //     }
        //     if (alreadyProcessed) continue;

        //     // 处理原有的插件（没有在本次 exportPlugins 中的）
        //     std::string pluginRel = plugin;
        //     if(!pluginRel.empty() && pluginRel[0] == '/') {
        //         pluginRel.erase(0, 1);
        //     }

        //     fs::path sourcePluginDir = versionDataPath / pluginRel;
        //     fs::path targetPluginDir = targetBaseDir / pluginRel;

        //     kmlogger.info("Existing plugin syncing from: %s to: %s", sourcePluginDir.string().c_str(), targetPluginDir.string().c_str());

        //     if (!syncSinglePlugin(sourcePluginDir, targetPluginDir)) {
        //         kmlogger.error("Failed to sync existing plugin: %s", plugin.c_str());
        //     }
        // }

        // 将收集到的文件路径添加到传入的installedFiles中
        installedFiles.insert(syncedFiles.begin(), syncedFiles.end());

        if (!writePluginsList(pluginsListFile, allPlugins)) {
            return false;
        }

        return true;

    } catch (const fs::filesystem_error& e) {
        kmlogger.error("Filesystem error: %s", std::string(e.what()).c_str());
        return false;
    } catch (const std::exception& e) {
        kmlogger.error("FError: %s", std::string(e.what()).c_str());
        return false;
    }
}

bool KMPluginSynchronizer::readExistingPluginsList(const fs::path& pluginsListFile, std::set<std::string>& allPlugins) {
    try {
        std::ifstream inFile(pluginsListFile);
        if (!inFile.is_open()) {
            kmlogger.error("Failed to open plugins list file for reading: %s", pluginsListFile.string().c_str());
            return false;
        }
        
        std::string line;
        while (std::getline(inFile, line)) {
            if (!line.empty()) {
                allPlugins.insert(line);
            }
        }
        inFile.close();
        return true;
        
    } catch (const std::exception& e) {
        kmlogger.error("Error reading plugins list: %s", std::string(e.what()).c_str());
        return false;
    }
}

bool KMPluginSynchronizer::writePluginsList(const fs::path& pluginsListFile, const std::set<std::string>& allPlugins) {
    try {
        std::ofstream outFile(pluginsListFile);
        if (!outFile.is_open()) {
            kmlogger.error("Failed to open plugins list file for writing: %s", pluginsListFile.string().c_str());
            return false;
        }
        
        for (const auto& plugin : allPlugins) {
            outFile << plugin << std::endl;
        }
        outFile.close();
        return true;
        
    } catch (const std::exception& e) {
        kmlogger.error("Error writing plugins list: %s", std::string(e.what()).c_str());
        return false;
    }
}

bool KMPluginSynchronizer::syncSinglePlugin(const fs::path& sourcePluginDir, const fs::path& targetPluginDir, std::set<std::string>& syncedFiles) {
    try {
        if (!fs::exists(sourcePluginDir)) {
            kmlogger.error("Source path does not exist: %s", sourcePluginDir.string().c_str());
            return false;
        }

        fs::create_directories(targetPluginDir.parent_path());

        std::string src = sourcePluginDir.string();
        std::string dst = targetPluginDir.string();

        if (fs::is_directory(sourcePluginDir)) {
            if (src.back() != '/') src += '/';
        }

        // -a: Archive mode; enables recursive copying while preserving symbolic links, permissions, timestamps, etc.
        // --links: Preserve symbolic links
        std::string cmd = "rsync -a -H --links " + src + " " + dst;

        kmlogger.info("Executing rsync: %s", cmd.c_str());
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            kmlogger.error("rsync failed with code %d", ret);
            return false;
        }

        // 同步完成后，扫描目标目录获取文件列表
        if (fs::exists(targetPluginDir)) {
            if (fs::is_directory(targetPluginDir)) {
                // 如果是目录，递归获取所有文件
                for (const auto& entry : fs::recursive_directory_iterator(targetPluginDir)) {
                    if (fs::is_regular_file(entry.path())) {
                        syncedFiles.insert(entry.path().string());
                    }
                }
            } else {
                // 如果是单个文件
                syncedFiles.insert(targetPluginDir.string());
            }
        }

        kmlogger.info("rsync completed: %s -> %s", sourcePluginDir.string().c_str(), targetPluginDir.string().c_str());
        return true;
    }
    catch (const std::exception& e) {
        kmlogger.error("Exception during syncSinglePlugin: %s", e.what());
        return false;
    }
}

std::string KMPluginSynchronizer::replaceArchWildcard(const std::string& path) {
    std::string result = path;

    // 查找 ${arch} 通配符
    size_t pos = result.find("${arch}");
    if (pos != std::string::npos) {
        // 获取架构信息
        std::string archStr = KMDirUtils::getLibraryDir();
        kmlogger.info("Replacing ${arch} with: %s", archStr.c_str());

        // 替换所有出现的 ${arch}
        while (pos != std::string::npos) {
            result.replace(pos, 7, archStr); // 7 是 "${arch}" 的长度
            pos = result.find("${arch}", pos + archStr.length());
        }
    }

    return result;
}

bool KMPluginSynchronizer::containsPlugin(const std::string& path, const std::string& pluginName) {
    return path.find(pluginName) != std::string::npos;
}
