/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMFOLDER_H__
#define __KMFOLDER_H__

#include <gio/gio.h>
#include <glib.h>
#include <ostree.h>

#include <filesystem>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "KMCommonTypePrivate.h"
#include "KMConfig.h"
#include "KMUtil.h"

namespace fs = std::filesystem;

/**
 * @brief folder class
 */
class KMFolder
{
public:
    KMFolder(fs::path path = getDefaultSysBaseLocationPath(), bool user = false);
    ~KMFolder();

    /**
     * @brief : 获取 base 路径 （ /var/local/lib/kaiming ）
     */
    fs::path getPath() { return m_basePath; }

    fs::path getEntriesPath() { return m_basePath / "entries"; }

    fs::path getLayersPath() { return m_basePath / "layers"; }

    fs::path getAppBasePath() { return m_basePath / "app"; }

    fs::path getRuntimeBasePath() { return m_basePath / "runtime"; }

    fs::path getExportsPath() { return m_basePath / "exports"; }

    fs::path getVarExportsPath() { return m_varExportPath; }

    fs::path getExportsBinPath() { return m_basePath / "bin"; }

    fs::path getExportsSharePath() { return m_basePath / "share"; }

    fs::path getOKMountPointPath() { return m_basePath / "ok"; }

    fs::path getInfoPath() { return m_basePath / "info"; }

    fs::path getDeployBasePath(const KMRef &ref);
    fs::path getDeployPath(const KMRef &ref, const std::string &commit);
    fs::path getActivePath(const KMRef &ref);
    fs::path getActivePath(const std::string &appId);
    fs::path getRuntimeActivePath(const std::string &runtimeId);

    bool setActive(const KMRef &ref, std::string &commit);
    KMRef getCurrentRef(std::string &name);

    OstreeRepo *getRepo() { return m_repo; }

    bool loadDeployData(const KMRef &ref, const std::string &commit, const int &requiredVersion);

    bool isInstalled(const std::string &name);
    bool isInstalled(const std::string &name,const KMPMRef &pmRef);
    bool isInstalled(const KMRef &ref);
    bool isInstalled(const KMPMRef &pmRef);
    bool isInstalledLike(const KMRef &ref, std::string &id);
    bool isBase(const std::string &name);
    bool isDepend(const std::string &name);
    bool isRuntimeExtension(const KMRef &ref);

    bool ensureRepo(bool allowEmpty);
    bool ensureRepoForSudo(bool allowEmpty);

    static bool ensurePath(const fs::path &path);

    static bool findIdPaths(const fs::path& rootDir, const std::string& id, std::vector<fs::path>& idPaths, int depth = 0);
    static bool findTmpIdPaths(const fs::path& rootDir, const std::string& id, std::vector<fs::path>& idPaths);
    static bool findDeployFiles(const fs::path &idPath, std::vector<fs::path> &deployFiles, int depth = 0);
    static std::vector<std::string> readDeployFiles(const fs::path &deployFilePath);

    static fs::path getHomePath();
    static fs::path getUserDataPath();
    static fs::path getUserCachePath();
    static fs::path getConfigPathLocation();
    static fs::path getDefaultSysBaseLocationPath();
    static fs::path getDefaultSysExportsLocationPath();
    static fs::path getUserBaseLocationPath();
    static fs::path getXdgRuntimePath();
    static fs::path getPulseRuntimePath();
    static fs::path getInstancePath();
    static fs::path getAppDataPath(const std::string &appId);
    static std::list<fs::path> getPathFileList(const fs::path &path);
    fs::path getSummariesPath();
    fs::path getRepoPath();

    //获取home/登录账户路径
    static fs::path getHomeDirectory();


private:
    std::map<std::string, fs::path> findNewRepos();
    void initInstalled();

private:
    fs::path m_basePath;
    fs::path m_entriesPath; // 应用导出数据最上层目录，如 "/opt/kaiming/entries"
    fs::path m_layersPath; // 应用数据最上层目录，如 "/opt/kaiming/layers"
    fs::path m_repoPath;
    fs::path m_varExportPath; // kare immutable system     
    fs::path m_summaryPath;

    bool m_user;
    bool m_useSystemHelper = false;
    OstreeRepo *m_repo = nullptr;

    std::list<std::string> m_appInstallList;
    std::list<std::string> m_runtimeInstallList;
    std::list<std::string> m_baseInstallList;
};

#endif // __KMFOLDER_H__