/*
 * Copyright (c) KylinSoft  Co., Ltd. 2025. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMHostAppDeploy.h"
#include "common/KMLogger.h"
#include "common/KMFileUtils.h"
#include "common/KMPackageManager.h"
#include "common/KMError.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define IGNORE_RETURN(func_call)                                  \
    do                                                            \
    {                                                             \
        /* 使用编译器特定属性标记变量为未使用 */                  \
        __attribute__((unused)) auto unused_result = (func_call); \
    } while (0)

class KMHostAppDeploy::Private
{
public:
    fs::path m_versionDataPath;
    std::string m_downloadRefId;
    fs::path m_infoPath;
    std::set<std::string> m_SynchronizedFiles;
    std::vector<std::string> m_vecService;
    std::vector<std::string> m_vecSysGroup;
    std::vector<std::string> m_vecDesktops;
    std::vector<std::string> m_vecAutostart;
    std::vector<std::string> m_vecChmods;
    std::vector<std::string> m_vecInstallkills;
    std::vector<std::string> m_vecUninstallkills;
    std::vector<std::string> m_vecStopservices;
    std::vector<std::string> m_vecRmdir;
    std::vector<std::string> m_vecUnlink;

    std::vector<std::string> m_vecUninstallStopservices; // 卸载时的停止服务
    std::vector<std::string> m_vecUninstallRemove;       // 卸载时的删除操作

    bool m_hostScript;
    std::vector<std::string> m_desktopFilesToRecord;
    TaskType m_installTaskType = TaskType::TASK_TYPE_INSTALL;
    std::map<fs::path, std::string> m_desktopCacheMap;
};

KMHostAppDeploy::KMHostAppDeploy(const std::string &infoPath)
    : d(std::make_unique<Private>())
{
    d->m_infoPath = infoPath;
}

KMHostAppDeploy::~KMHostAppDeploy()
{
}

bool KMHostAppDeploy::loadHostAppInfo()
{
    const fs::path infoFile = d->m_versionDataPath / "info.json";
    if (!fs::exists(infoFile))
    {
        kmlogger.debug("can't find app info file: %s", infoFile.string().c_str());
        return false;
    }

    KMInfoJson infoJson;
    infoJson.loadFile(infoFile);

    const auto annotations = infoJson.getAnnotations();

    d->m_vecService = !annotations.install_actions.start_service.empty() ? annotations.install_actions.start_service : annotations.service;
    d->m_vecSysGroup = !annotations.install_actions.add_group.empty() ? annotations.install_actions.add_group : annotations.sysgroup;
    d->m_vecAutostart = !annotations.install_actions.autostart.empty() ? annotations.install_actions.autostart : annotations.autostart;
    d->m_vecChmods = !annotations.install_actions.chmod.empty() ? annotations.install_actions.chmod : annotations.chmods;

    // 安装时的停止进程 - 优先使用新的，如果没有则使用旧的
    d->m_vecInstallkills = !annotations.install_actions.stop_process.empty() ? annotations.install_actions.stop_process : annotations.installkills;

    // 安装时的停止服务 - 只使用新的 install_actions.stop_service
    d->m_vecStopservices = annotations.install_actions.stop_service;

    // 安装时的删除操作 - 优先使用新的 install_actions.remove，如果没有则使用旧的 rmdir 和 unlink
    if (!annotations.install_actions.remove.empty())
    {
        d->m_vecRmdir = annotations.install_actions.remove;
    }
    else
    {
        // 合并旧的 rmdir 和 unlink
        d->m_vecRmdir = annotations.rmdir;
        d->m_vecRmdir.insert(d->m_vecRmdir.end(), annotations.unlink.begin(), annotations.unlink.end());
    }

    // 卸载时的动作 - 优先使用新的 uninstall_actions，如果没有则使用旧的字段
    d->m_vecUninstallkills = !annotations.uninstall_actions.stop_process.empty() ? annotations.uninstall_actions.stop_process : annotations.uninstallkills;

    // 卸载时的停止服务 - 优先使用新的 uninstall_actions.stop_service，如果没有则使用旧的 annotations.stopservices
    d->m_vecUninstallStopservices = !annotations.uninstall_actions.stop_service.empty() ? annotations.uninstall_actions.stop_service : annotations.stopservices;

    // 卸载时的删除操作 - 优先使用新的 uninstall_actions.remove，如果没有则使用旧的 rmdir 和 unlink
    if (!annotations.uninstall_actions.remove.empty())
    {
        d->m_vecUninstallRemove = annotations.uninstall_actions.remove;
    }
    else
    {
        d->m_vecUninstallRemove = annotations.rmdir;
        d->m_vecUninstallRemove.insert(d->m_vecUninstallRemove.end(), annotations.unlink.begin(), annotations.unlink.end());
    }

    d->m_vecDesktops = annotations.desktops;
    d->m_hostScript = annotations.hostscript;
    d->m_vecUnlink = annotations.unlink;

    return annotations.hostapp;
}

void KMHostAppDeploy::setHostAppConfig(const fs::path &versionPath, const std::string &refID)
{
    d->m_versionDataPath = versionPath;
    d->m_downloadRefId = refID;
}

bool KMHostAppDeploy::collectSynchronizedFiles()
{
    if (!handleKareDriver())
    {
        return false;
    }

    exportDesktopFile();

    fs::path m_extractDir = d->m_versionDataPath / "files";

    for (const auto &entry : fs::recursive_directory_iterator(m_extractDir))
    {
        std::string relativePath = entry.path().string().substr(m_extractDir.string().length());

        if (entry.path() == m_extractDir)
        {
            continue;
        }

        if (fs::is_regular_file(entry) || fs::is_directory(entry) || fs::is_symlink(entry))
        {
            if (fs::is_symlink(entry.path()) && fs::read_symlink(entry.path()) == entry.path())
            {
                continue;
            }

            d->m_SynchronizedFiles.insert(relativePath);
        }
    }
    return true;
}

bool KMHostAppDeploy::checkFileConflicts()
{
    static const std::set<std::string> excludedDirs = {"/etc", "/var", "/usr/local/etc"};
    bool conflictFound = false;

    for (const auto &file : d->m_SynchronizedFiles)
    {
        if (shouldSkipFile(file, excludedDirs))
        {
            KMInfo("Skipping conflict check for: " + file);
            continue;
        }

        if (fs::exists(file))
        {
            KMError("File conflict already exists on system: " + file);
            conflictFound = true;
            // break;
        }
    }
    return conflictFound;
}

std::set<std::string> KMHostAppDeploy::getSynchronizedFiles()
{
    return d->m_SynchronizedFiles;
}

bool KMHostAppDeploy::shouldSkipFile(const std::string &file, const std::set<std::string> &excludedDirs)
{
    return std::any_of(excludedDirs.begin(), excludedDirs.end(),
                       [&file](const std::string &excludedDir)
                       {
                           return file.find(excludedDir) == 0;
                       });
}

int KMHostAppDeploy::synchronizeFiles(bool append)
{
    fs::path sourcePath = d->m_versionDataPath / "files";

    if (!fs::exists(sourcePath) || !fs::is_directory(sourcePath))
    {
        KMError(std::string("Source path does not exist or is not a directory: ") + sourcePath.string());
        return KM_ERROR_INSTALL_FAILED;
    }

    // preinst
    if (d->m_hostScript)
    {
        int rc = executeHostScript("preinst", ScriptLocation::INSTALLED_DIR, d->m_installTaskType);
        if (rc != 0)
        {
            cleanupOnFailure();
            kmlogger.error("Execute preinst script failed with exit code: %d, installation aborted", rc);
            return rc;
        }
    }

    std::set<std::string> excludeDirs = {"lib", "usr", "bin"};
    std::vector<std::string> excludePaths = {"/var/lib/dpkg", "/var/log"};

    // 遍历源目录下的顶级目录
    for (const auto &entry : fs::directory_iterator(sourcePath))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        std::string dirName = entry.path().filename().string();
        if (excludeDirs.find(dirName) != excludeDirs.end())
        {
            KMInfo("Skipping excluded directory: " + dirName);
            continue;
        }

        // 为当前顶层目录构造 rsync 排除项（把 "/var/lib/dpkg" -> "/lib/dpkg"）
        std::vector<std::string> rsyncExcludes;
        for (const auto &ex : excludePaths)
        {
            if (ex.empty())
            {
                continue;
            }

            std::string norm = ex.front() == '/' ? ex.substr(1) : ex;
            std::string prefix = dirName + "/";
            if (norm.rfind(prefix, 0) == 0)
            {
                std::string rel = norm.substr(prefix.size());
                if (!rel.empty())
                {
                    // 以 '/' 锚定到传输根，rsync 在传输 var/ 下会匹配 "/lib/dpkg"
                    rsyncExcludes.push_back(std::string("/") + rel);
                }
            }
        }

        fs::path targetPath = fs::path("/") / dirName;
        std::ostringstream command;
        command << "rsync -av";

        for (const auto &pat : rsyncExcludes)
        {
            command << " --exclude='" << pat << "'";
        }

        command << " '" << entry.path().string() << "/' '"
                << targetPath.string() << "/'";

        KMInfo("Syncing directory: " + entry.path().string());
        int result = system(command.str().c_str());
        if (result != 0)
        {
            KMError("Failed to sync directory: " + targetPath.string());
        }
    }

    executePostInstallActions();

    // Clean & Record
    writeSynchronizedFileList(append);
    fs::remove_all(sourcePath); // delete files

    // postinst
    if (d->m_hostScript)
    {
        int rc = executeHostScript("postinst", ScriptLocation::EXPORT_DIR, d->m_installTaskType);
        if (rc != 0)
        {
            cleanupOnFailure();
            uninstall(d->m_installTaskType);

            kmlogger.error("Execute postinst script failed with exit code: %d, installation aborted", rc);
            return rc;
        }
    }

    return KM_ERROR_NO;
}

void KMHostAppDeploy::handleSchemaCompilation()
{
    fs::path schemaPath("/opt/system/resource/glib-2.0/schemas/");
    if (!fs::exists(schemaPath))
    {
        fs::create_directories(schemaPath);
    }

    std::ostringstream schemaCommand;
    schemaCommand << "glib-compile-schemas /opt/system/resource/glib-2.0/schemas/ >/dev/null 2>&1";
    IGNORE_RETURN(system(schemaCommand.str().c_str()));
}

void KMHostAppDeploy::handleAutoStart()
{
    fs::path autostartDir = "/etc/xdg/autostart";
    if (!fs::exists(autostartDir))
    {
        fs::create_directories(autostartDir);
    }

    for (const auto &autostartFile : d->m_vecAutostart)
    {
        if (autostartFile.empty())
        {
            kmlogger.debug("Empty autostart file path found, skipping...");
            continue;
        }

        fs::path sourceFile = autostartFile;
        fs::path targetFile = autostartDir / fs::path(autostartFile).filename();

        try
        {
            if (fs::exists(sourceFile))
            {
                fs::copy_file(sourceFile, targetFile, fs::copy_options::overwrite_existing);
                kmlogger.info("Copied autostart file: %s", autostartFile.c_str());
            }
            else
            {
                kmlogger.error("Autostart file not found: %s", sourceFile.string().c_str());
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to copy autostart file: %s : %s", autostartFile.c_str(), e.what());
        }
    }
}

void KMHostAppDeploy::handleChmods()
{
    for (const auto &chmodEntry : d->m_vecChmods)
    {
        if (chmodEntry.empty())
        {
            kmlogger.debug("Empty chmod entry found, skipping...");
            continue;
        }

        std::istringstream iss(chmodEntry);
        std::string modeStr, filePath;
        iss >> modeStr >> filePath;

        if (modeStr.empty() || filePath.empty())
        {
            kmlogger.error("Invalid chmod entry: %s", chmodEntry.c_str());
            continue;
        }

        bool isRecursive = false;
        size_t r_pos = modeStr.find("-R");

        if (r_pos != std::string::npos)
        {
            std::string suffix = modeStr.substr(r_pos);
            suffix.erase(std::remove_if(suffix.begin(), suffix.end(), ::isspace), suffix.end());
            if (suffix == "-R")
            {
                isRecursive = true;
                modeStr = modeStr.substr(0, r_pos);
            }
        }

        mode_t mode = static_cast<mode_t>(std::strtol(modeStr.c_str(), nullptr, 8));

        if (isRecursive)
        {
            chmodRecursive(filePath, mode);
            kmlogger.info("Recursively set permission %s for %s", modeStr.c_str(), filePath.c_str());
        }
        else
        {
            int ret = chmod(filePath.c_str(), mode);
            if (ret == 0)
            {
                struct stat pathStat;
                stat(filePath.c_str(), &pathStat);
                const char *type = S_ISDIR(pathStat.st_mode) ? "directory" : "file";
                kmlogger.info("Set permission %s for %s %s", modeStr.c_str(), type, filePath.c_str());
            }
            else
            {
                kmlogger.error("Failed to set permission %s for %s", modeStr.c_str(), filePath.c_str());
            }
        }
    }
}

void KMHostAppDeploy::chmodRecursive(const std::string &path, mode_t mode)
{
    struct stat statBuf;
    if (stat(path.c_str(), &statBuf) != 0)
    {
        return;
    }

    chmod(path.c_str(), mode);

    if (!S_ISDIR(statBuf.st_mode))
    {
        return;
    }

    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        std::string fullPath = path + "/" + entry->d_name;
        chmodRecursive(fullPath, mode);
    }
    closedir(dir);
}
void KMHostAppDeploy::handleInstallKills()
{
    processKillList(d->m_vecInstallkills);
}

void KMHostAppDeploy::handleUninstallKills()
{
    processKillList(d->m_vecUninstallkills);
}

void KMHostAppDeploy::processKillList(const std::vector<std::string> &processes)
{
    for (const auto &procPath : processes)
    {
        if (procPath.empty())
        {
            kmlogger.debug("Empty process entry found, skipping...");
            continue;
        }

        std::string command = "pkill -9 -f '" + procPath + "'";
        int ret = system(command.c_str());
        if (ret == 0 || ret == 9)
        {
            kmlogger.info("Successfully killed process: %s", procPath.c_str());
        }
        else
        {
            kmlogger.error("Failed to kill process: %s", procPath.c_str());
        }
    }
}

void KMHostAppDeploy::handleStopServices()
{
    for (const auto &service : d->m_vecUninstallStopservices)
    {
        if (service.empty())
        {
            kmlogger.debug("Empty stopservice entry found, skipping...");
            continue;
        }

        std::ostringstream cmd;
        cmd << "systemctl stop " << service;
        int ret = system(cmd.str().c_str());
        if (ret == 0)
        {
            kmlogger.info("Stopped service: %s", service.c_str());
        }
        else
        {
            kmlogger.error("Failed to stop service: %s", service.c_str());
        }
    }
}

void KMHostAppDeploy::handleRmdir()
{
    handleRemovePaths(d->m_vecRmdir);
}

void KMHostAppDeploy::handleRemovePaths(const std::vector<std::string> &paths)
{
    static const std::vector<std::string> protectedPrefixes = {
        "/", "/usr", "/var", "/bin", "/lib", "/sbin", "/root", "/boot", "/dev", "/proc", "/sys", "/run"};

    // 检查是否为用户桌面目录
    auto isUserDesktop = [](const fs::path &path) -> bool
    {
        std::string pathStr = path.string();
        size_t pos = pathStr.find("/home/");
        if (pos == 0)
        {
            auto rest = pathStr.substr(6);
            auto slashPos = rest.find('/');
            if (slashPos != std::string::npos)
            {
                std::string subdir = rest.substr(slashPos + 1);
                if (subdir == "桌面" || subdir == "Desktop")
                {
                    return true;
                }
            }
        }
        return false;
    };

    for (const auto &path : paths)
    {
        if (path.empty())
        {
            kmlogger.debug("Empty remove entry found, skipping...");
            continue;
        }

        std::string resolvedPath = resolveHomePath(path);
        fs::path pathToRemove = resolvedPath;
        std::string pathStr = pathToRemove.string();

        // 保护系统目录和用户桌面
        bool isProtected = false;
        for (const auto &prefix : protectedPrefixes)
        {
            if (pathStr == prefix || (pathStr.size() > prefix.size() && pathStr.substr(0, prefix.size() + 1) == prefix + "/"))
            {
                isProtected = true;
                break;
            }
        }
        if (isProtected || isUserDesktop(pathToRemove))
        {
            kmlogger.error("Refused to remove protected or desktop path: %s", pathStr.c_str());
            continue;
        }

        try
        {
            if (fs::exists(pathToRemove))
            {
                // 使用 fs::remove_all 可以同时删除文件和目录
                fs::remove_all(pathToRemove);

                // 根据类型记录日志
                if (fs::is_directory(pathToRemove))
                {
                    kmlogger.info("Removed directory: %s", pathStr.c_str());
                }
                else
                {
                    kmlogger.info("Removed file: %s", pathStr.c_str());
                }
            }
            else
            {
                kmlogger.debug("Path does not exist: %s", pathStr.c_str());
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to remove path: %s : %s", pathStr.c_str(), e.what());
        }
    }
}

void KMHostAppDeploy::handleUnlink()
{
    for (const auto &dirPath : d->m_vecUnlink)
    {
        if (dirPath.empty())
        {
            kmlogger.debug("Empty rmdir entry found, skipping...");
            continue;
        }

        // 解析路径，处理 $HOME 和 ${HOME} 变量
        std::string resolvedPath = resolveHomePath(dirPath);
        fs::path dirToRemove = resolvedPath;
        std::string dirStr = dirToRemove.string();

        try
        {
            if (fs::exists(dirToRemove))
            {
                fs::remove_all(dirToRemove);
                kmlogger.info("Removed directory: %s", dirStr.c_str());
            }
            else
            {
                kmlogger.debug("Directory does not exist or is not a directory: %s", dirStr.c_str());
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to remove directory: %s : %s", dirStr.c_str(), e.what());
        }
    }
}

void KMHostAppDeploy::startServices()
{
    for (const auto &sysgroup : d->m_vecSysGroup)
    {
        if (sysgroup.empty())
        {
            KMDebug("Empty system group found, skipping...");
            continue;
        }

        std::ostringstream sysGroupCommand;
        sysGroupCommand << "addgroup --system " << sysgroup;

        int result = system(sysGroupCommand.str().c_str());
        if (result != 0)
        {
            KMError("Failed to add system group: " + sysgroup + " (exit code: " + std::to_string(result) + ")");
        }
        else
        {
            KMInfo("Successfully add system group: " + sysgroup);
        }
    }

    for (const auto &service : d->m_vecService)
    {
        if (service.empty())
        {
            KMDebug("Empty service name found, skipping...");
            continue;
        }

        int reloadResult = system("systemctl daemon-reload");
        if (reloadResult != 0)
        {
            KMError("Failed to reload systemd daemon (exit code: " + std::to_string(reloadResult) + ")");
            continue;
        }

        std::ostringstream serviceCommand;
        serviceCommand << "systemctl enable --now " << service;

        int result = system(serviceCommand.str().c_str());
        if (result != 0)
        {
            KMError("Failed to enable and start service: " + service + " (exit code: " + std::to_string(result) + ")");
        }
        else
        {
            KMInfo("Successfully enabled and started service: " + service);
        }
    }
}

void KMHostAppDeploy::writeSynchronizedFileList(bool append)
{
    fs::path synchronizedPath = d->m_infoPath / (d->m_downloadRefId + ".hostlist");
    if (!fs::exists(d->m_infoPath))
    {
        if (!fs::create_directories(d->m_infoPath))
        {
            kmlogger.error("Failed to create directory: %s", d->m_infoPath.string().c_str());
            return;
        }
    }

    std::ios_base::openmode mode = std::ios::out;
    if (append)
    {
        mode |= std::ios::app; // 追加模式
    }
    else
    {
        mode |= std::ios::trunc; // 截断模式（覆盖）
    }

    std::ofstream listFile(synchronizedPath, mode);

    if (!listFile.is_open())
    {
        kmlogger.error("Failed to open file: %s", synchronizedPath.string().c_str());
        return;
    }

    for (const auto &file : d->m_SynchronizedFiles)
    {
        listFile << file << "\n";
    }
}

void KMHostAppDeploy::processRecordedDesktopFiles(KMPackageManager &packageManager)
{
    for (const auto &desktopFilePath : d->m_desktopFilesToRecord)
    {
        packageManager.recordDesktopMd5(d->m_downloadRefId, desktopFilePath, d->m_SynchronizedFiles);
    }

    d->m_desktopFilesToRecord.clear();
}

int KMHostAppDeploy::uninstall(TaskType type)
{
    if (d->m_hostScript)
    {
        int rc = executeHostScript("prerm", ScriptLocation::EXPORT_DIR, type);
        if (rc != 0)
        {
            kmlogger.error("Execute prerm script failed with exit code: %d, uninstall aborted", rc);
            return rc;
        }
    }

    executeUninstallActions();

    fs::path synchronizedPath = d->m_infoPath / (d->m_downloadRefId + ".hostlist");
    fs::path infoListPath = d->m_infoPath / (d->m_downloadRefId + ".list");

    // 读取已安装文件列表（hostlist + list 合并）
    std::set<std::string> installedFiles = readSynchronizedFileList(synchronizedPath);
    if (fs::exists(infoListPath))
    {
        std::set<std::string> listFiles = readSynchronizedFileList(infoListPath);
        installedFiles.merge(listFiles);
    }

    if (installedFiles.empty())
    {
        KMInfo("No installed files found or file list is empty.");
        return KM_ERROR_NO;
    }

    handleKareUninstall(installedFiles);

    static const std::set<std::string> excludedDirs = {"/usr/", "/var/", "/lib"};

    fs::path postrmScriptPath = fs::path("/opt/kaiming/exports/share/hostscript") / d->m_downloadRefId / "postrm";
    bool hasPostrmScript = (installedFiles.find(postrmScriptPath.string()) != installedFiles.end());

    // 从待删除列表中移除 postrm 脚本
    std::set<std::string> filesToDelete = installedFiles;
    if (hasPostrmScript)
    {
        filesToDelete.erase(postrmScriptPath.string());
        kmlogger.debug("Preserved postrm script for execution: %s", postrmScriptPath.string().c_str());
    }

    for (const auto &file : filesToDelete)
    {
        fs::path filePath = file;
        std::string filePathStr = filePath.string();

        // 检查文件是否在排除目录中
        if (shouldSkipFile(filePathStr, excludedDirs))
        {
            KMInfo("Skipping protected directory file: " + filePathStr);
            continue;
        }

        try
        {
            if (fs::exists(filePath))
            {
                fs::remove(filePath);
                std::cout << " Deleted file: " << filePath.string() << "\n";
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to delete file: %s - %s", filePathStr.c_str(), e.what());
        }
    }

    removeEmptyDirs(installedFiles);

    // 执行卸载后脚本
    if (d->m_hostScript && hasPostrmScript)
    {
        try
        {
            executeHostScript("postrm", ScriptLocation::EXPORT_DIR, type);

            if (fs::exists(postrmScriptPath))
            {
                fs::remove(postrmScriptPath);
                kmlogger.info("Removed postrm script after execution: %s", postrmScriptPath.string().c_str());
            }

            fs::path hostscriptDir = postrmScriptPath.parent_path();
            if (fs::exists(hostscriptDir) && fs::is_empty(hostscriptDir))
            {
                fs::remove(hostscriptDir);
                kmlogger.debug("Removed empty hostscript directory: %s", hostscriptDir.string().c_str());
            }
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Execute postrm script error: %s", e.what());
        }
    }

    // 清理元数据
    try
    {
        if (fs::exists(synchronizedPath))
        {
            fs::remove(synchronizedPath);
            kmlogger.info("Deleted synchronized file list: %s", synchronizedPath.string().c_str());
        }

        if (fs::exists(infoListPath))
        {
            fs::remove(infoListPath);
            kmlogger.info("Deleted info list file: %s", infoListPath.string().c_str());
        }
    }
    catch (const std::exception &e)
    {
        KMError("Failed to delete list files: " + std::string(e.what()));
    }

    return KM_ERROR_NO;
}

void KMHostAppDeploy::executeUninstallActions()
{
    // 停止相关进程
    if (!d->m_vecUninstallkills.empty())
    {
        processKillList(d->m_vecUninstallkills);
    }

    // 停止服务
    if (!d->m_vecUninstallStopservices.empty())
    {
        handleStopServices();
    }

    // 删除文件
    if (!d->m_vecUninstallRemove.empty())
    {
        handleRemovePaths(d->m_vecUninstallRemove);
    }
}

void KMHostAppDeploy::removeEmptyDirs(const std::set<std::string> &files)
{
    for (const auto &file : files)
    {
        fs::path dir = fs::path(file).parent_path();
        while (dir != "/" && fs::exists(dir) && fs::is_empty(dir))
        {
            fs::remove(dir);
            KMDebug("Removed empty dir: " + dir.string());
            dir = dir.parent_path();
        }
    }
}

std::set<std::string> KMHostAppDeploy::readSynchronizedFileList(const fs::path &listPath)
{
    std::set<std::string> fileSet;

    std::ifstream listFile(listPath);
    if (!listFile)
    {
        kmlogger.error("Failed to open synchronized file list: %s", listPath.string().c_str());
        return fileSet;
    }

    std::string line;
    while (std::getline(listFile, line))
    {
        fileSet.insert(line);
    }

    return fileSet;
}

bool KMHostAppDeploy::handleKareDriver()
{
    fs::path sourceDir = d->m_versionDataPath / "files";
    if (fs::exists(sourceDir) && fs::is_directory(sourceDir))
    {
        for (const auto &entry : fs::directory_iterator(sourceDir))
        {
            if (entry.path().extension() == ".deb")
            {
                const fs::path targetDir = "/opt/kaiming/share/driver";
                fs::create_directories(targetDir);

                fs::path debFilePath = entry.path();
                fs::path targetFilePath = targetDir / debFilePath.filename();

                fs::copy(debFilePath, targetFilePath, fs::copy_options::overwrite_existing);
                d->m_SynchronizedFiles.insert(targetFilePath);
                kmlogger.info("Copy deb file: %s -> %s", debFilePath.string().c_str(), targetFilePath.string().c_str());

                // 读取deb文件的标签
                std::string compatTag = readDebCompatTag(targetFilePath);
                std::string command;

                if (compatTag.find("v10sp1") != std::string::npos)
                {
                    command = "kare -e v10sp1 -i " + targetFilePath.string();
                    kmlogger.info("Using v10sp1 environment based on compat tag");
                }
                else if (compatTag.find("v11") != std::string::npos)
                {
                    command = "kare -e shadow -i " + targetFilePath.string();
                    kmlogger.info("Using shadow environment based on compat tag");
                }
                else
                {
                    // 默认使用v10sp1
                    command = "kare -e v10sp1 -i " + targetFilePath.string();
                    kmlogger.info("Using default v10sp1 environment (no valid compat tag found)");
                }

                kmlogger.info("Executing kare: %s", command.c_str());

                int result = system(command.c_str());
                if (result != 0)
                {
                    fs::remove_all(d->m_versionDataPath);

                    std::set<std::string> pathsToCheck = {d->m_versionDataPath.string()};
                    removeEmptyDirs(pathsToCheck);

                    kmlogger.error("Failed to execute kare command for file: %s", targetFilePath.string().c_str());
                    return false;
                }
                else
                {
                    kmlogger.info("Successfully installed deb package: %s", targetFilePath.string().c_str());
                }

                return true;
            }
        }
    }

    return true;
}

std::string KMHostAppDeploy::readDebCompatTag(const fs::path &debPath)
{
    if (!fs::exists(debPath))
    {
        kmlogger.error("Deb file not found: %s", debPath.string().c_str());
        return "";
    }

    std::ifstream file(debPath.string(), std::ios::binary);
    if (!file.is_open())
    {
        kmlogger.error("Failed to open deb file: %s", debPath.string().c_str());
        return "";
    }

    // 检查ar文件签名
    char signature[8];
    file.read(signature, 8);
    if (std::string(signature, 8) != "!<arch>\n")
    {
        kmlogger.error("Invalid ar file signature in: %s", debPath.string().c_str());
        return "";
    }

    size_t pos = 8;
    while (true)
    {
        char buffer[60];
        file.read(buffer, 60);

        if (file.gcount() == 0)
            break;
        if (file.gcount() != 60)
        {
            kmlogger.error("Truncated ar file: %s", debPath.string().c_str());
            return "";
        }

        // 解析ar文件头
        std::string name(buffer, 16);
        // 去除尾部空格
        name = name.substr(0, name.find_last_not_of(' ') + 1);

        // 文件大小 (10字节)
        std::string size_str(buffer + 48, 10);
        size_t size = std::stoul(size_str.substr(0, size_str.find_last_not_of(' ') + 1));

        // 文件魔数 (2字节)
        if (buffer[58] != 0x60 || buffer[59] != 0x0A)
        {
            kmlogger.error("Invalid file signature in ar: %s", debPath.string().c_str());
            return "";
        }

        // 如果是_compat文件，读取内容
        if (name == "_compat")
        {
            file.seekg(pos + 60);
            std::vector<char> content_buffer(size);
            file.read(content_buffer.data(), size);

            if (file.gcount() != static_cast<std::streamsize>(size))
            {
                kmlogger.error("Failed to read _compat content from: %s", debPath.string().c_str());
                return "";
            }

            std::string content(content_buffer.data(), size);
            // 提取runtime_version部分
            if (content.find("runtime_version:") == 0)
            {
                std::string tag = content.substr(16); // 去掉"runtime_version:"前缀
                // 去除可能的换行符和空白字符
                tag.erase(tag.find_last_not_of(" \n\r\t") + 1);
                kmlogger.info("Found compat tag in %s: %s", debPath.string().c_str(), tag.c_str());
                return tag;
            }
            return "";
        }

        // 跳过文件内容
        size_t skip = size;
        if (skip % 2 != 0)
            skip += 1; // ar文件对齐
        file.seekg(skip, std::ios::cur);
        pos += 60 + skip;

        if (file.tellg() != static_cast<std::streampos>(pos))
        {
            kmlogger.error("File position error in: %s", debPath.string().c_str());
            return "";
        }
    }

    kmlogger.debug("No _compat tag found in: %s", debPath.string().c_str());
    return "";
}

void KMHostAppDeploy::exportDesktopFile()
{
    fs::path sourceDir = d->m_versionDataPath / "files";
    fs::path cacheDir = d->m_versionDataPath / "desktop-cache";

    fs::create_directories(cacheDir);

    struct DesktopDirMap
    {
        fs::path srcDir;
        std::string targetPrefix;
    };

    std::vector<DesktopDirMap> appDirs = {
        {sourceDir / "opt/system/resource/applications",
         "/opt/system/resource/applications"},

        {sourceDir / "usr/share/applications",
         "/usr/share/applications"}};

    for (const auto &dirMap : appDirs)
    {
        const fs::path &appDir = dirMap.srcDir;

        if (!fs::exists(appDir) || !fs::is_directory(appDir))
            continue;

        for (const auto &entry : fs::directory_iterator(appDir))
        {
            const auto &filePath = entry.path();

            if (!fs::is_regular_file(filePath) || filePath.extension() != ".desktop")
                continue;

            std::ifstream inFile(filePath);
            if (!inFile)
                continue;

            std::string content;
            content.assign(std::istreambuf_iterator<char>(inFile), {});
            inFile.close();

            // 已有 X-Kaiming 则跳过
            if (content.find("X-Kaiming=") != std::string::npos)
                continue;

            // 插入字段
            const size_t pos = content.find("[Desktop Entry]");
            if (pos == std::string::npos)
                continue;

            content.insert(pos + 15, "\nX-Kaiming=" + d->m_downloadRefId);

            // 写入缓存文件
            fs::path cachedFile = cacheDir / filePath.filename();

            std::ofstream outFile(cachedFile, std::ios::trunc);
            if (!outFile)
                continue;

            outFile << content;
            outFile.close();

            // 记录：缓存文件路径，用于 md5 记录
            d->m_desktopFilesToRecord.push_back(cachedFile.string());

            // 记录：同步到系统的目标路径（保留原结构）
            std::string finalTarget = dirMap.targetPrefix + "/" + filePath.filename().string();

            d->m_SynchronizedFiles.insert(finalTarget);

            // 记录缓存 -> 系统路径的映射，供同步使用
            d->m_desktopCacheMap[cachedFile] = finalTarget;

            kmlogger.info("Prepared modified desktop: %s -> %s", cachedFile.string().c_str(), finalTarget.c_str());
        }
    }
}

void KMHostAppDeploy::syncModifiedDesktopFiles()
{
    if (d->m_desktopCacheMap.empty())
        return;

    for (auto &item : d->m_desktopCacheMap)
    {
        const fs::path &cachedFile = item.first;
        const std::string &targetPath = item.second;

        try
        {
            fs::path targetDir = fs::path(targetPath).parent_path();
            fs::create_directories(targetDir);

            fs::copy_file(cachedFile, targetPath, fs::copy_options::overwrite_existing);

            kmlogger.info("Synced modified desktop file: %s → %s", cachedFile.string().c_str(), targetPath.c_str());
        }
        catch (std::exception &e)
        {
            kmlogger.error("Failed to sync desktop %s : %s", cachedFile.string().c_str(), e.what());
        }
    }

    // 删除 desktop-cache 目录
    fs::path cacheDir = d->m_versionDataPath / "desktop-cache";
    if (fs::exists(cacheDir))
    {
        try
        {
            fs::remove_all(cacheDir);
            kmlogger.info("Removed desktop-cache: %s", cacheDir.string().c_str());
        }
        catch (const std::exception &e)
        {
            kmlogger.error("Failed to remove desktop-cache %s : %s", cacheDir.string().c_str(), e.what());
        }
    }
}

void KMHostAppDeploy::handleDesktopFiles()
{
    syncModifiedDesktopFiles();

    auto userDesktopDirs = getUserDesktopDirs();
    if (userDesktopDirs.empty())
    {
        kmlogger.warn("No user desktop directories found");
        return;
    }

    const std::vector<fs::path> searchPaths = {
        "/opt/system/resource/applications",
        "/usr/share/applications"};

    for (const auto &desktopName : d->m_vecDesktops)
    {
        if (desktopName.empty())
        {
            kmlogger.debug("Empty desktop file path found, skipping...");
            continue;
        }

        fs::path desktopPath(desktopName);
        fs::path desktopFileName;

        if (desktopPath.has_parent_path())
        {
            desktopFileName = desktopPath.filename();
            kmlogger.debug("Extracted filename from path: %s -> %s", desktopName.c_str(), desktopFileName.string().c_str());
        }
        else
        {
            desktopFileName = desktopPath;
        }

        fs::path sourceFile = findDesktopFile(desktopFileName, searchPaths);
        if (sourceFile.empty())
        {
            kmlogger.warn("Desktop file not found: %s", desktopFileName.string().c_str());
            continue;
        }

        // 拷贝到所有用户桌面
        for (const auto &userDesktop : userDesktopDirs)
        {
            fs::path userTargetFile = userDesktop / desktopFileName;
            try
            {
                fs::copy_file(sourceFile, userTargetFile, fs::copy_options::overwrite_existing);
                KMStorageDir::modifyDirOwnership(userTargetFile);
                chmod(userTargetFile.c_str(), 0755);
                d->m_SynchronizedFiles.insert(userTargetFile.string());
                kmlogger.info("Copied desktop file to user desktop: %s", userTargetFile.string().c_str());
            }
            catch (const std::exception &e)
            {
                kmlogger.error("Failed to copy desktop file to user desktop: %s : %s", userTargetFile.string().c_str(), e.what());
            }
        }
    }
}

std::vector<fs::path> KMHostAppDeploy::getUserDesktopDirs()
{
    std::vector<fs::path> userDesktopDirs;
    DIR *dir = opendir("/home");
    if (!dir)
        return userDesktopDirs;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            std::vector<fs::path> possiblePaths = {
                fs::path("/home") / entry->d_name / "桌面",
                fs::path("/home") / entry->d_name / "Desktop"};

            for (const auto &path : possiblePaths)
            {
                if (fs::exists(path) && fs::is_directory(path))
                {
                    userDesktopDirs.push_back(path);
                    break; // 找到一个就退出
                }
            }
        }
    }
    closedir(dir);
    return userDesktopDirs;
}

fs::path KMHostAppDeploy::findDesktopFile(const fs::path &desktopFileName, const std::vector<fs::path> &searchPaths)
{
    for (const auto &path : searchPaths)
    {
        fs::path fullPath = path / desktopFileName;
        if (fs::exists(fullPath) && fs::is_regular_file(fullPath))
        {
            return fullPath;
        }
    }
    return fs::path();
}

void KMHostAppDeploy::executePostInstallActions()
{
    handleInstallKills();
    handleAutoStart();
    handleDesktopFiles();
    handleChmods();
    handleSchemaCompilation();
    startServices();
}

void KMHostAppDeploy::cleanupOnFailure()
{
    fs::remove_all(d->m_versionDataPath);
    std::set<std::string> pathsToCheck = {d->m_versionDataPath.string()};
    removeEmptyDirs(pathsToCheck);
}

int KMHostAppDeploy::executeHostScript(const std::string &scriptName, ScriptLocation location, TaskType taskType)
{
    try
    {
        fs::path basePath;

        if (location == ScriptLocation::INSTALLED_DIR)
        {
            basePath = d->m_versionDataPath / "files/opt/kaiming/exports/share/hostscript" / d->m_downloadRefId;
        }
        else
        {
            basePath = fs::path("/opt/kaiming/exports/share/hostscript") / d->m_downloadRefId;
        }

        std::string action;
        switch (taskType)
        {
        case TaskType::TASK_TYPE_INSTALL:
            action = "install";
            break;
        case TaskType::TASK_TYPE_UPGRADE:
            action = "upgrade";
            break;
        case TaskType::TASK_TYPE_UNINSTALL:
            action = "uninstall";
            break;
        default:
            action = "install";
            break;
        }

        return executeScript((basePath / scriptName).string(), action);
    }
    catch (const std::exception &e)
    {
        kmlogger.error("Execute %s script error: %s", scriptName.c_str(), e.what());
        return KM_ERROR_SCRIPT_EXECUTION_FAILED;
    }
}

int KMHostAppDeploy::executeScript(const std::string &scriptPath, const std::string &arg)
{
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0)
    {
        kmlogger.info("Script not found: %s", scriptPath.c_str());
        return 0;
    }

    if (chmod(scriptPath.c_str(), fileStat.st_mode | S_IXUSR) != 0)
    {
        kmlogger.error("Failed to make script executable: %s", scriptPath.c_str());
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        kmlogger.error("fork() failed");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        execl(scriptPath.c_str(), scriptPath.c_str(), arg.c_str(), (char *)nullptr);

        kmlogger.error("Failed to execute: %s - %s", scriptPath.c_str(), strerror(errno));
        return EXIT_FAILURE;
    }
    else
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            kmlogger.error("waitpid() failed for %s", scriptPath.c_str());
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status))
        {
            int exitCode = WEXITSTATUS(status);
            return exitCode;
        }
        else
        {
            kmlogger.error("%s terminated abnormally", scriptPath.c_str());
            return EXIT_FAILURE;
        }
    }
}

void KMHostAppDeploy::setInstallTaskType(TaskType type)
{
    d->m_installTaskType = type;
}

void KMHostAppDeploy::handleKareUninstall(const std::set<std::string> &installedFiles)
{

    std::string packageName;
    size_t firstDot = d->m_downloadRefId.find('.');
    if (firstDot != std::string::npos)
    {
        size_t secondDot = d->m_downloadRefId.find('.', firstDot + 1);
        if (secondDot != std::string::npos)
        {
            packageName = d->m_downloadRefId.substr(secondDot + 1);
        }
        else
        {
            packageName = d->m_downloadRefId;
        }
    }
    else
    {
        packageName = d->m_downloadRefId;
    }

    std::string checkCommand = "kare -l | grep -q \"" + packageName + "\"";
    int checkResult = system(checkCommand.c_str());

    if (checkResult != 0)
    {
        kmlogger.info("Kare package not installed or not found: %s", packageName.c_str());
        return;
    }

    std::string command = "kare -P " + packageName;
    kmlogger.info("Executing kare uninstall command: %s", command.c_str());

    int result = system(command.c_str());
    if (result != 0)
    {
        kmlogger.error("Failed to execute kare uninstall command for package: %s", packageName.c_str());
    }
    else
    {
        kmlogger.info("Successfully uninstalled kare package: %s", packageName.c_str());
    }
}

int KMHostAppDeploy::checkAndCleanupUninstalledDrivers()
{
    int cleanedCount = 0;

    if (!fs::exists(d->m_infoPath))
    {
        return 0;
    }

    bool kareAvailable = false;
    {
        FILE *pipe = popen("which kare > /dev/null 2>&1", "r");
        if (pipe)
        {
            int status = pclose(pipe);
            kareAvailable = (status == 0);
        }
    }

    if (!kareAvailable)
    {
        kmlogger.warn("kare command not found, skip driver package cleanup to prevent mistaken removal");
        return 0;
    }

    // 预先获取一次 kare -l 的输出，避免对每个包重复调用外部命令
    std::set<std::string> kareInstalled;
    {
        FILE *pipe = popen("kare -l", "r");
        if (pipe)
        {
            char buffer[4096] = {};
            std::string output;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                output.append(buffer);
            }
            pclose(pipe);
            std::istringstream iss(output);
            std::string line;
            while (std::getline(iss, line))
            {
                if (!line.empty())
                {
                    kareInstalled.insert(line);
                }
            }
        }
        else
        {
            kmlogger.error("Failed to execute 'kare -l'");
            return 0;
        }
    }

    // 遍历所有 .hostlist 文件
    for (const auto &entry : fs::directory_iterator(d->m_infoPath))
    {
        if (entry.path().extension() == ".hostlist")
        {
            std::string hostlistFile = entry.path().filename().string();
            std::string refId = hostlistFile.substr(0, hostlistFile.length() - 9); // 移除 ".hostlist"

            // 从 refId 中提取包名
            std::string packageName;
            size_t firstDot = refId.find('.');
            if (firstDot != std::string::npos)
            {
                size_t secondDot = refId.find('.', firstDot + 1);
                if (secondDot != std::string::npos)
                {
                    packageName = refId.substr(secondDot + 1);
                }
                else
                {
                    packageName = refId;
                }
            }
            else
            {
                packageName = refId;
            }

            // 检查这个 hostlist 文件是否包含驱动包信息
            std::set<std::string> installedFiles = readSynchronizedFileList(entry.path());
            bool hasDriverFile = false;

            for (const auto &file : installedFiles)
            {
                if (file.find("/opt/kaiming/share/driver/") == 0)
                {
                    hasDriverFile = true;
                    break;
                }
            }

            if (hasDriverFile)
            {
                // 使用缓存的 kare -l 结果判断
                bool foundInKare = false;
                for (const auto &line : kareInstalled)
                {
                    if (line.find(packageName) != std::string::npos)
                    {
                        foundInKare = true;
                        break;
                    }
                }

                if (!foundInKare)
                {
                    // 包不在 kare 中，走统一卸载流程（会清理文件、服务、进程并删除.hostlist/.list）
                    kmlogger.info("Driver package %s not found in kare, uninstalling via KMHostAppDeploy::uninstall ...", packageName.c_str());

                    d->m_downloadRefId = refId;

                    // 统一卸载
                    uninstall(TaskType::TASK_TYPE_UNINSTALL);
                    cleanedCount++;
                }
            }
        }
    }

    return cleanedCount;
}

std::vector<std::string> KMHostAppDeploy::getInstalledDriverPackages()
{
    std::vector<std::string> driverPackages;

    if (!fs::exists(d->m_infoPath))
    {
        return driverPackages;
    }

    for (const auto &entry : fs::directory_iterator(d->m_infoPath))
    {
        if (entry.path().extension() == ".hostlist")
        {
            std::string hostlistFile = entry.path().filename().string();
            std::string refId = hostlistFile.substr(0, hostlistFile.length() - 9);

            // 检查这个 hostlist 文件是否包含驱动包信息
            std::set<std::string> installedFiles = readSynchronizedFileList(entry.path());
            bool hasDriverFile = false;

            for (const auto &file : installedFiles)
            {
                if (file.find("/opt/kaiming/share/driver/") == 0)
                {
                    hasDriverFile = true;
                    break;
                }
            }

            if (hasDriverFile)
            {
                // 从 refId 中提取包名
                std::string packageName;
                size_t firstDot = refId.find('.');
                if (firstDot != std::string::npos)
                {
                    size_t secondDot = refId.find('.', firstDot + 1);
                    if (secondDot != std::string::npos)
                    {
                        packageName = refId.substr(secondDot + 1);
                    }
                    else
                    {
                        packageName = refId;
                    }
                }
                else
                {
                    packageName = refId;
                }

                driverPackages.push_back(packageName);
            }
        }
    }

    return driverPackages;
}

const std::vector<std::string> &KMHostAppDeploy::getUninstallkills() const { return d->m_vecUninstallkills; }

const std::vector<std::string> &KMHostAppDeploy::getStopservices() const { return d->m_vecStopservices; }

const std::vector<std::string> &KMHostAppDeploy::getRmdir() const { return d->m_vecRmdir; }

const std::vector<std::string> &KMHostAppDeploy::getUdir() const { return d->m_vecUnlink; }

std::string KMHostAppDeploy::resolveHomePath(const std::string &path)
{
    std::string result = path;

    // 获取当前登录用户
    std::string loginUser = KMStorageDir::getLoginUser();
    if (loginUser.empty())
    {

        return path;
    }

    // 构建用户主目录路径
    std::string userHome = "/home/" + loginUser;

    // 处理 $HOME 开头的路径
    if (path.find("$HOME/") == 0)
    {
        result = userHome + path.substr(5); // 去掉 "$HOME"
        kmlogger.debug("Resolved path: %s -> %s", path.c_str(), result.c_str());
    }
    // 处理 ${HOME} 开头的路径
    else if (path.find("${HOME}/") == 0)
    {
        result = userHome + path.substr(7); // 去掉 "${HOME}"
        kmlogger.debug("Resolved path: %s -> %s", path.c_str(), result.c_str());
    }
    // 处理只有 $HOME 或 ${HOME} 的情况
    else if (path == "$HOME")
    {
        result = userHome;
        kmlogger.debug("Resolved path: %s -> %s", path.c_str(), result.c_str());
    }
    else if (path == "${HOME}")
    {
        result = userHome;
        kmlogger.debug("Resolved path: %s -> %s", path.c_str(), result.c_str());
    }

    return result;
}

void KMHostAppDeploy::updateHostDeltas()
{
    fs::path deltaPath = d->m_versionDataPath / "files";
    std::vector<std::string> files;

    KMPackageManager packageManager;
    bool success = packageManager.getAllFilesInDirectory(deltaPath.string(), files); // 注意：可能需要string()
    fs::path synchronizedPath = d->m_infoPath / (d->m_downloadRefId + ".hostlist");
    std::set<std::string> excludeDirs = {"/lib", "/usr", "/bin", "/var/lib/dpkg", "/var/log"};
    std::ofstream syncFile(synchronizedPath, std::ios::app);

    if (!syncFile.is_open())
    {
        std::cerr << "Failed to open sync file: " << synchronizedPath << std::endl;
        return;
    }

    for (auto file : files)
    {
        fs::path filePath(file);
        if (!fs::is_regular_file(filePath))
        {
            continue;
        }

        bool shouldExclude = false;
        for (const auto &excludeDir : excludeDirs)
        {
            fs::path excludeFullPath = deltaPath / excludeDir.substr(1);
            std::string excludeStr = excludeFullPath.string();

            if (file.find(excludeStr + "/") == 0 || file == excludeStr)
            {
                shouldExclude = true;
                break;
            }
        }

        if (shouldExclude)
        {
            continue;
        }

        if (file.find(deltaPath.string()) == 0)
        {
            std::string relativePath = file.substr(deltaPath.string().length());

            if (!relativePath.empty() && relativePath[0] != '/')
            {
                relativePath = "/" + relativePath;
            }

            std::string hostTargetPath = relativePath;

            if (fs::path(file).extension() == ".xdelta")
            {
                std::string targetFilePath = file.substr(0, file.length() - 7);
                std::string targetRelativePath = relativePath.substr(0, relativePath.length() - 7);
                std::string hostTargetFilePath = targetRelativePath;

                if (fs::exists(hostTargetFilePath))
                {
                    std::string tempFile = hostTargetFilePath + ".new";
                    std::string command = std::string(KAIMING_DELTAS) + " -d -f -s \"" +
                                          hostTargetFilePath + "\" \"" + file + "\" \"" +
                                          tempFile + "\"";

                    int result = system(command.c_str());

                    if (result == 0)
                    {

                        std::string backupFile = hostTargetFilePath + ".bak";

                        if (fs::exists(backupFile))
                        {
                            fs::remove(backupFile);
                        }

                        fs::copy_file(hostTargetFilePath, backupFile);

                        fs::rename(tempFile, hostTargetFilePath);
                        std::cout << "Successfully updated: " << hostTargetFilePath << std::endl;

                        fs::remove(backupFile);
                    }
                }
            }
            else
            {
                try
                {
                    fs::path hostTargetDir = fs::path(hostTargetPath).parent_path();

                    if (!fs::exists(hostTargetDir))
                    {
                        fs::create_directories(hostTargetDir);
                    }

                    fs::copy_file(file, hostTargetPath, fs::copy_options::overwrite_existing);
                    std::cout << "Successfully copied: " << hostTargetPath << std::endl;
                    syncFile << hostTargetPath << std::endl;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Failed to copy file: " << file << " to " << hostTargetPath
                              << ", error: " << e.what() << std::endl;
                }
            }
        }
    }

    syncFile.close();
}
