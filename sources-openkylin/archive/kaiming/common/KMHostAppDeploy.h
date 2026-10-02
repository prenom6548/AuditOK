/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef _KMHOSTAPPDEPLOY_H
#define _KMHOSTAPPDEPLOY_H

#include <filesystem>
#include <iostream>
#include <set>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <vector>

#include "common/KMCommonTypePrivate.h"
#include "common/KMInfoJson.h"

namespace fs = std::filesystem;

enum class ScriptLocation {
    INSTALLED_DIR,
    EXPORT_DIR
};

class KMPackageManager;

class KMHostAppDeploy
{
public:
    explicit KMHostAppDeploy(const std::string &infoPath);
    virtual ~KMHostAppDeploy();

    friend class KMPackageManager;

public:
    /**
     * @brief 加载 info.json 以确定是否是 HostApp
     */
    bool loadHostAppInfo();

    /**
     * @brief 设置版本路径,refID
     */
    void setHostAppConfig(const fs::path &versionPath, const std::string &refID);

    /**
     * @brief 遍历解压目录，收集同步文件
     */
    bool collectSynchronizedFiles();

    /**
     * @brief: 获取同步文件列表
     * @return 返回同步文件列表
     */
    std::set<std::string> getSynchronizedFiles();

    /**
     * @brief 检查并清理已卸载的驱动包
     * @return 返回清理的驱动包数量
     */
    int checkAndCleanupUninstalledDrivers();

    /**
     * @brief 获取所有已安装的驱动包信息
     * @return 返回驱动包名列表
     */
    std::vector<std::string> getInstalledDriverPackages();

    /**
     * @brief 检查文件是否已经存在，避免覆盖
     */
    bool checkFileConflicts();

    /**
     * @brief 执行 rsync 同步文件
     * @return 0 表示成功，非0表示失败（脚本执行失败时返回错误码）
     */
    int synchronizeFiles(bool append);

    /**
     * @brief 卸载方法
     * @return 0 表示成功，非0表示失败（脚本执行失败时返回错误码）
     */
    int uninstall(TaskType type = TaskType::TASK_TYPE_UNINSTALL);

    /**
     * @brief 执行卸载操作
     */
    void executeUninstallActions();

    /**
     * @brief 处理记录的desktop文件，供KMPackageManager调用
     * @param packageManager KMPackageManager实例的引用
     */
    void processRecordedDesktopFiles(KMPackageManager &packageManager);

    const std::vector<std::string>& getUninstallkills() const;
    const std::vector<std::string>& getStopservices() const;
    const std::vector<std::string>& getRmdir() const;
    const std::vector<std::string>& getUdir() const;

    /**
     * @description: 删除指定文件
     * @return {*}
     */
    void handleUnlink();

    std::string resolveHomePath(const std::string& path);
private:
    /**
     * @brief 写入已同步文件列表
     */
    void writeSynchronizedFileList(bool append);

    /**
     * @brief 读取已安装文件列表
     */
    std::set<std::string> readSynchronizedFileList(const fs::path &listPath);

    /**
     * @brief: 跳过特定文件
     * @param {string} &file 待检测的文件
     * @param {set<std::string>} &excludedDirs 需排除的目录
     * @return {*}
     */
    bool shouldSkipFile(const std::string &file, const std::set<std::string> &excludedDirs);

    /**
     * @brief: 清理空目录
     * @param {set<std::string>} &files 待清理所在目录文件
     * @return {*}
     */
    void removeEmptyDirs(const std::set<std::string> &files);

    /**
     * @brief: 编译 GSettings schema 文件
     * @return {*}
     */
    void handleSchemaCompilation();

    /**
     * @brief: 启动非容器应用相关服务
     * @return {*}
     */
    void startServices();

    /**
     * @description: handle autostart files
     * @return {*}
     */
    void handleAutoStart();

    /**
     * @description: handle desktop files
     * @return {*}
     */
    void handleDesktopFiles();

    /**
     * @description: 获取用户桌面目录
     * @return {std::vector}
     */
    std::vector<fs::path> getUserDesktopDirs();

    /**
     * @description: 查找桌面文件
     * @return {fs::path}
     */
    fs::path findDesktopFile(const fs::path& desktopFileName, const std::vector<fs::path>& searchPaths);

    /**
     * @description: handle chmod files
     * @return {*}
     */
    void handleChmods();

    void chmodRecursive(const std::string &path, mode_t mode);

    /**
     * @description: 安装时结束进程
     * @return {*}
     */
    void handleInstallKills();

    /**
     * @description: 卸载时结束进程
     * @return {*}
     */
    void handleUninstallKills();

    void processKillList(const std::vector<std::string> &processes);
    /**
     * @description: 卸载时停止服务
     * @return {*}
     */
    void handleStopServices();

    /**
     * @description: 删除指定目录
     * @return {*}
     */
    void handleRmdir();

    /**
     * @description: 删除路径
     * @return {*}
     */
    void handleRemovePaths(const std::vector<std::string>& paths);

    /**
     * @brief 处理kare driver
     */
    bool handleKareDriver();

    /**
     * @brief 读取deb文件标签
     */
    std::string readDebCompatTag(const fs::path& debPath);
    /**
     * @brief 导出desktop文件，开始菜单卸载等使用
     */
    void exportDesktopFile();

    void syncModifiedDesktopFiles();

    /**
     * @brief 执行钩子脚本
     * 
     * @param scriptPath 脚本路径
     * @param arg 执行动作
     */
    /**
     * @brief 执行钩子脚本并返回退出码
     * @return 脚本退出码（0 表示成功）
     */
    int executeScript(const std::string &scriptPath, const std::string &arg);

    /**
     * @brief: 执行主机应用程序脚本
     * @param scriptName 脚本文件名（如 "preinst", "postinst", "prerm", "postrm"）
     * @param location 脚本执行位置
     * @param taskType 任务类型，决定传递给脚本的动作参数
     * @return int 执行结果
     *         - 0: 脚本执行成功
     *         - 非0: 脚本执行失败或出现错误
     */
    int executeHostScript(const std::string &scriptName, ScriptLocation location, TaskType taskType);

    /**
    * @brief: 安装失败时的清理操作
    */
    void cleanupOnFailure();

    /**
     * @brief 执行安装后配置
     */
    void executePostInstallActions();

    /**
     * @brief 设置安装脚本动作（install/upgrade），由上层根据 currentFuzzyRefIsInstalled 判断
     */
    void setInstallTaskType(TaskType type);

    /**
     * @brief  卸载 kare 驱动
     */
    void handleKareUninstall(const std::set<std::string>& installedFiles);

    void updateHostDeltas();

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif //_KMHOSTAPPDEPLOY_H
