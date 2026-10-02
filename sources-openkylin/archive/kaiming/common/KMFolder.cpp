/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMFolder.h"

#include <fcntl.h>

#include <fstream>
#include <iostream>

#include "KMLogger.h"

KMFolder::KMFolder(fs::path path, bool user)
{
    m_basePath = path;
    m_entriesPath = path / "entries";
    m_layersPath = path / "layers";
    m_repoPath = path / "repo";
    m_varExportPath = getDefaultSysExportsLocationPath();
    m_summaryPath = m_repoPath / "tmp" / "cache" / "summaries";
    m_user = user;

    initInstalled();
}

KMFolder::~KMFolder() { }

bool KMFolder::ensureRepo(bool allowEmpty)
{
    GCancellable *cancellable = nullptr;

    if (m_repo != nullptr)
    {
        return true;
    }

    if (!fs::exists(m_basePath))
    {
        kmlogger.perror("kaiming install path not exist.");
        return false;
    }

    m_repo = ostree_repo_new(g_file_new_for_path(m_repoPath.c_str()));

    if (!fs::exists(m_repoPath))
    {
        kmlogger.perror("kaiming install repo not exist.");
    }
    else
    {
        if (!ostree_repo_open(m_repo, nullptr, nullptr))
        {
            kmlogger.perror("open kaiming repo error.");
            return false;
        }
    }

    if (m_useSystemHelper)
    {
        fs::path cachePath = fs::path("/var/cache/kaiming/system-cache");
        if (!fs::exists(cachePath))
        {
            if (!fs::create_directories(cachePath))
            {
                kmlogger.perror("ostree cache dir create error. (%s)", cachePath.c_str());
                return false;
            }
        }
        if (!ostree_repo_set_cache_dir(m_repo, AT_FDCWD, cachePath.c_str(), nullptr, nullptr))
        {
            kmlogger.perror("ostree set cache dir error. (%s)", cachePath.c_str());
            return false;
        }
    }
    else
    {
        // 修改配置文件
    }

    // need goon
    std::map<std::string, fs::path> newRepos = findNewRepos();
    if (newRepos.size() > 0)
    {
        if (m_useSystemHelper)
        {
        }
    }

    return true;
}

/**
 * 安装命令需要 sudo 权限创建 ostree repo，而 run 等命令不用，因此需要单独新增接口
 */
bool KMFolder::ensureRepoForSudo(bool allowEmpty)
{
    GCancellable *cancellable = nullptr;
    g_autoptr(GError) error = nullptr;

    if (m_repo != nullptr)
    {
        return true;
    }

    std::vector<fs::path> requiredPaths = {
        m_basePath, m_varExportPath, getInfoPath()
    };

    for (const auto& path : requiredPaths) {
        if (!ensurePath(path)) {
            kmlogger.error("Required path does not exist: %s", path.c_str());
            return false;
        }
    }

    m_repo = ostree_repo_new(g_file_new_for_path(m_repoPath.c_str()));
    if (!fs::exists(m_repoPath))
    {
        // repo 文件夹不存在，则进行创建
        OstreeRepoMode mode = OSTREE_REPO_MODE_BARE_USER_ONLY;
        if (!ostree_repo_create(m_repo, mode, cancellable, &error))
        {
            kmlogger.error("ostree repo create failed!");
            return false;
        }

        // 一般这个目录不会 ostree 自动创建，需要自行创建
        ensurePath(getSummariesPath());

        kmlogger.info("begin to update");

        // 修改对应 repo/config 配置
        GKeyFile *config;
        g_autofree char *group = g_strdup_printf("remote \"%s\"", "kaiming-repo");
        config = ostree_repo_copy_config(m_repo);
        g_key_file_set_string(config, "core", "min-free-space-size", "500MB");
        g_key_file_set_string(config, "core", "mode", "bare-user");
        g_key_file_set_boolean(config, group, "gpg-verify", FALSE);
        g_key_file_set_boolean(config, group, "gpg-verify-summary", FALSE);
        g_key_file_set_string(config, group, "url", REPO_URL);

        if (!ostree_repo_write_config(m_repo, config, &error))
        {
            return false;
        }
    }
    else
    {
        if (!ostree_repo_open(m_repo, nullptr, nullptr))
        {
            // repo 打开失败
            return false;
        }
    }

    if (m_useSystemHelper)
    {
        fs::path cachePath = fs::path("/var/cache/kaiming/system-cache");
        if (!fs::exists(cachePath))
        {
            if (!fs::create_directories(cachePath))
            {
                kmlogger.perror("ostree cache dir create error. (%s)", cachePath.c_str());
                return false;
            }
        }
        if (!ostree_repo_set_cache_dir(m_repo, AT_FDCWD, cachePath.c_str(), nullptr, nullptr))
        {
            kmlogger.perror("ostree set cache dir error. (%s)", cachePath.c_str());
            return false;
        }
    }
    else
    {
        // 修改配置文件
    }

    // need goon
    std::map<std::string, fs::path> newRepos = findNewRepos();
    if (newRepos.size() > 0)
    {
        if (m_useSystemHelper)
        {
        }
    }

    return true;
}

bool KMFolder::ensurePath(const fs::path &path)
{
    if (fs::exists(path))
    {
        return true;
    }

    if (fs::create_directories(path))
    {
        return true;
    }

    return false;
}

bool KMFolder::findIdPaths(const fs::path& rootDir, const std::string& id, std::vector<fs::path>& idPaths, int depth)
{
    if (depth > 3)
    {
        return false; // Limit recursion to the 3rd level
    }

    try {
        for (const auto &entry : fs::directory_iterator(rootDir))
        {
            if (entry.is_directory())
            {
                if (depth == 3 && entry.path().filename() == id)
                {
                    idPaths.push_back(entry.path());
                }
                else
                {
                    findIdPaths(entry.path(), id, idPaths, depth + 1);
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
    return true;
    
}

bool KMFolder::findTmpIdPaths(const fs::path& rootDir, const std::string& id, std::vector<fs::path>& idPaths)
{
    try 
    {
        for (const auto& entry : fs::directory_iterator(rootDir))
        {
            if (fs::is_symlink(fs::symlink_status(entry)))
            {
                continue;
            }
            // 跳过不存在的或不是目录的项
            if (!fs::exists(entry) || !fs::is_directory(entry))
            {
                continue;
            }

            // 跳过名为 systemd-private 开头的目录
            std::string dirName = entry.path().filename().string();
            if (dirName.find("systemd-private") == 0)
            {
                continue;
            }

            // 匹配 id
            if (dirName == id)
            {
                idPaths.push_back(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        // std::cerr << "Filesystem error: " << e.what() << std::endl;
        return true;
    }
    return true;
}

bool KMFolder::findDeployFiles(const fs::path &idPath, std::vector<fs::path> &deployFiles, int depth)
{
    if (depth > 2)
    {
        return false; // Limit recursion to the 2nd level under the ID path
    }

    try
    {
        for (const auto &entry : fs::directory_iterator(idPath))
        {
            if (entry.is_directory())
            {
                findDeployFiles(entry.path(), deployFiles, depth + 1);
            }
            else if (entry.is_regular_file() && entry.path().filename() == "deploy")
            {
                deployFiles.push_back(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return true;
}

std::vector<std::string> KMFolder::readDeployFiles(const fs::path &deployFilePath)
{
    std::set<std::string> uniqueLines;
    std::ifstream file(deployFilePath);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << deployFilePath << std::endl;
        return {};
    }

    while (std::getline(file, line))
    {
        uniqueLines.insert(line);
    }

    file.close();
    return { uniqueLines.begin(), uniqueLines.end() };
}

/**
 * @brief : 读取 deploy 文件
 */
bool KMFolder::loadDeployData(const KMRef &ref, const std::string &commit, const int &requiredVersion)
{
    return fs::exists(m_basePath / ref.kind / ref.id / "current");
}

/**
 * @brief : 获取 deploy 的 base 文件夹 （/var/local/lib/kaiming/runtime/org.kde.Platform/x86_64/5.15-22.08/827a73c556510720a92853f23a62ea8948d0521815e6807c4f4fa55980e3d5aa）
 */
fs::path KMFolder::getDeployPath(const KMRef &ref, const std::string &commit)
{
    fs::path deployDir;
    if (commit != "")
    {
        deployDir = getDeployBasePath(ref) / commit;
    }
    else
    {
        fs::path activePath = m_basePath / "active";
        if (activePath.has_filename())
        {
            if (fs::is_symlink(activePath))
            {
                fs::path realPath = fs::read_symlink(activePath);
                deployDir /= realPath;
            }
        }
    }
    kmlogger.info("deployDir: %s", deployDir.c_str());
    return deployDir;
}

/**
 * @brief : 获取 deploy 的 base 文件夹 （/var/local/lib/kaiming/runtime/org.kde.Platform/x86_64/5.15-22.08）
 */
fs::path KMFolder::getDeployBasePath(const KMRef &ref)
{
    return m_basePath / ref.kind / ref.id / ref.arch / ref.branch;
}

fs::path KMFolder::getHomePath()
{
    auto home = getenv("HOME");
    if (home == nullptr)
    {
        if (getuid() == 0)
        {
            return fs::path("/root");
        }
    }
    return fs::path(home);
}

fs::path KMFolder::getUserDataPath()
{
    return fs::path(g_get_user_data_dir());
}

fs::path KMFolder::getUserCachePath()
{
    return fs::path(g_get_user_cache_dir());
}

/**
 * @brief : 获取配置文件文件夹
 */
fs::path KMFolder::getConfigPathLocation()
{
    char *configPath = getenv("KAIMING_CONFIG_PATH");
    return configPath == nullptr ? KAIMING_CONFIG_PATH : configPath;
}

fs::path KMFolder::getDefaultSysBaseLocationPath()
{
    char *dir = getenv("KAIMING_SYSTEM_DIR");
    if (dir == nullptr)
    {
        return KAIMING_SYSTEM_PATH;
    }

    return fs::path(dir);
}

/**
 * @brief Kare不可变系统用户数据所在目录
 *
 * 该函数目前看着和getDefaultSysBaseLocationPath一致，但只是不可变系统 /var/opt/kaiming 和 /opt/kaiming 目前做了软链接，
 * 因此，为了避免后期不可变系统的改变，保留该接口。
 */
fs::path KMFolder::getDefaultSysExportsLocationPath()
{
    char *dir = getenv("KAIMING_SYSTEM_DIR");
    if (dir == nullptr)
    {
        return KAIMING_SYSTEM_PATH;
    }

    return fs::path(dir);
}

fs::path KMFolder::getUserBaseLocationPath()
{
    char *dir = getenv("KAIMING_USER_DIR");
    if (dir == nullptr)
    {
        return fs::path(getenv("HOME")) / ".local/share/kaiming/";
    }

    return fs::path(dir);
}

fs::path KMFolder::getXdgRuntimePath()
{
    return fs::path(realpath(g_get_user_runtime_dir(), NULL));
}

fs::path KMFolder::getPulseRuntimePath()
{
    char *val = getenv("PULSE_RUNTIME_PATH");
    if (val != nullptr)
    {
        return fs::path(val);
    }

    if (fs::exists(fs::path(realpath(g_get_user_runtime_dir(), NULL)) / "pulse"))
    {
        return fs::path(realpath(g_get_user_runtime_dir(), NULL)) / "pulse";
    }

    // need goon kaiming_run_get_pulse_runtime_dir

    return fs::path();
}

/**
 * @brief : 获取 instance 路径，/run/user/1000/.kaiming
 */
fs::path KMFolder::getInstancePath()
{
    return getXdgRuntimePath() / ".kaiming";
}

fs::path KMFolder::getHomeDirectory()
{
    const char *username = getenv("SUDO_USER"); // 如果通过 sudo 执行，获取原始用户

    if (username == NULL)
    {
        username = getenv("USER"); // 如果没有通过 sudo 执行，获取当前用户
    }

    if (username != NULL)
    {
        struct passwd *pw = getpwnam(username);
        if (pw != NULL)
        {
            return fs::path(pw->pw_dir); // 返回用户的主目录路径
        }
        else
        {
            throw std::runtime_error("User not found");
        }
    }
    else
    {
        throw std::runtime_error("Unable to determine the user");
    }
}

fs::path KMFolder::getAppDataPath(const std::string &appId)
{
    fs::path homeDir= getHomeDirectory();

    string appDataDir = homeDir / ".var" / "app" / appId;
    return fs::path(appDataDir);

}

/**
 * @brief 获取 summaries 路径， /opt/kaiming/repo/tmp/cache/summaries
 *
 * @return fs::path 返回的 summaries 路径
 */
fs::path KMFolder::getSummariesPath()
{
    return m_summaryPath;
}

fs::path KMFolder::getRepoPath()
{
    return m_repoPath;
}

/**
 * @brief : 获取路径下所有文件和文件夹
 */
std::list<fs::path> KMFolder::getPathFileList(const fs::path &path)
{
    if (!fs::exists(path))
    {
        return std::list<fs::path>();
    }

    std::list<fs::path> fileList;

    for (auto &p : fs::directory_iterator(path))
    {
        fileList.push_back(p.path().filename());
    }
    return fileList;
}

/**
 * @brief : 获取 active 对应的 commit 绝对路径
 */
fs::path KMFolder::getActivePath(const KMRef &ref)
{
    // branch 暂时通过文件夹下的唯一文件夹获取
    fs::path pathTmp = m_basePath / ref.kind / ref.id / ref.arch;
    for (auto const &dirEntry : fs::directory_iterator{ pathTmp })
    {
        if (ref.branch.empty() || dirEntry.path().filename().string() == ref.branch)
        {
            return dirEntry.path() / fs::read_symlink(dirEntry.path() / "active");
        }
    }
    return fs::path();
}

fs::path KMFolder::getActivePath(const std::string &appId)
{
    return m_basePath / "app" / appId / "current" / "active";
}

fs::path KMFolder::getRuntimeActivePath(const std::string &runtimeId)
{
    return m_basePath / "runtime" / runtimeId / "current" / "active";
}

bool KMFolder::setActive(const KMRef &ref, std::string &commit)
{
    fs::path tmpPath = m_basePath / ref.kind / ref.id / ref.arch / ref.branch;
    if (!fs::exists(tmpPath / commit))
    {
        return false;
    }

    if (!fs::remove(tmpPath / "active"))
    {
        return false;
    }

    fs::create_symlink(tmpPath / commit, tmpPath / "active");

    return true;
}

bool KMFolder::isInstalled(const std::string &name)
{
    //return fs::exists(m_layersPath / "app" / name) || fs::exists(m_layersPath / "runtime" / name) || fs::exists(m_layersPath / "base" / name);

    return fs::exists(m_layersPath / "stable" / "x86_64" / "app" / name) || fs::exists(m_layersPath / "stable" / "x86_64" / "runtime" / name) || fs::exists(m_layersPath / "stable" / "x86_64" / "base" / name);
}

bool KMFolder::isInstalled(const std::string &name,const KMPMRef &pmRef)
{
    return fs::exists(m_layersPath / "stable" / pmRef.ref.arch / "app" / name) || fs::exists(m_layersPath / "stable" / pmRef.ref.arch / "runtime" / name) || fs::exists(m_layersPath / "stable" / pmRef.ref.arch / "base" / name ) || fs::exists(m_layersPath / "stable" / pmRef.ref.arch / "depend" / name);
}

bool KMFolder::isInstalled(const KMRef &ref)
{
    return fs::exists(m_layersPath / ref.kind / ref.id / ref.channel / ref.version);
}

bool KMFolder::isInstalled(const KMPMRef &pmRef)
{
    return fs::exists(m_layersPath / pmRef.ref.kind / pmRef.ref.id / pmRef.ref.channel /  pmRef.ref.version); 
}

/**
 * @brief : 查找类似安装 app, runtime, base，例如 org.freedesktop.Platform.GL  返回 org.freedesktop.Platform.GL.default
 */
bool KMFolder::isInstalledLike(const KMRef &ref, std::string &id)
{
    for (auto &path : ref.kind == "app" ? m_appInstallList : (ref.kind == "runtime" ? m_runtimeInstallList : m_baseInstallList))
    {
        if (path.compare(0, ref.id.size(), ref.id) == 0)
        {
            id = path;
            return true;
        }
    }

    return false;
}

bool KMFolder::isRuntimeExtension(const KMRef &ref)
{
    return false;
}

KMRef KMFolder::getCurrentRef(std::string &name)
{
    return KMRef();
}

std::map<std::string, fs::path> KMFolder::findNewRepos()
{
    std::map<std::string, fs::path> ret;
    // need goon
    if (m_user)
    {
        return std::map<std::string, fs::path>();
    }

    fs::path configPath = getConfigPathLocation();

    return std::map<std::string, fs::path>();
}

void KMFolder::initInstalled()
{
    if (!fs::exists(m_layersPath))
    {
        return;
    }

    if (fs::exists(m_layersPath / "app"))
    {
        for (auto &entry : fs::directory_iterator(m_layersPath / "app"))
        {
            if (entry.is_directory())
            {
                m_appInstallList.push_back(entry.path().filename());
            }
        }
    }

    if (fs::exists(m_layersPath / "runtime"))
    {
        for (auto &entry : fs::directory_iterator(m_layersPath / "runtime"))
        {
            if (entry.is_directory())
            {
                m_runtimeInstallList.push_back(entry.path().filename());
            }
        }
    }

    if (fs::exists(m_layersPath / "base"))
    {
        for (auto &entry : fs::directory_iterator(m_layersPath / "base"))
        {
            if (entry.is_directory())
            {
                m_baseInstallList.push_back(entry.path().filename());
            }
        }
    }
}