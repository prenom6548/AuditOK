/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMRunnerPlugins.h"

#include <set>
#include <fstream>
#include <algorithm>

#include "KMRunnerMount.h"
#include "common/KMLogger.h"
#include "common/KMContainer.h"
#include "common/KMStringUtils.h"

namespace fs = std::filesystem;

static const std::string KARE_PLUGIN_DIR = "/opt/system/resource/export/kare";
static const std::vector<std::string> MOZILLA_PLUGIN_DIRS = { "/usr/lib/mozilla/plugins" };
static const std::vector<std::string> KM_PLUGIN_DIRS = { "/opt/system/resource/export/kaiming", "/opt/kaiming/exports/plugins" };

/**
 * @brief 挂载插件
 * @return true 成功 false 失败
 */
bool KMRunnerPlugins::mountPlugins(KMContainer *container, const std::string &appDataPath, const Annotations &annotations, const std::string &appDir)
{
    if (!container)
        return false;

    //处理临时运维模式
    std::error_code ec;
    bool isNormalMode = fs::exists("/var/tmp/ostree/usr-normal-mode", ec);

    for (const auto &dir : MOZILLA_PLUGIN_DIRS)
    {
        std::string hostDir = dir;
        if (isNormalMode && dir.rfind("/usr/", 0) == 0)
        {
            hostDir = "/var/tmp/ostree/usr-normal-mode/" + dir.substr(5);
        }
        std::string kareDir = KARE_PLUGIN_DIR + dir;

        std::vector<std::string> lowers;
        for (const auto &base : KM_PLUGIN_DIRS)
        {
            lowers.emplace_back(base + dir);
        }
        lowers.emplace_back(kareDir);
        lowers.emplace_back(hostDir);
        KMRunnerMount::mountOverlay(container, appDataPath, lowers, "/appplugins" + dir, dir);
    }

    handlePluginDependencies(container, appDataPath);

    if (annotations.hostPlugins)
    {
        // 挂载 VFS 预加载库
        if (!mountVfsPreloadLibrary(container, appDataPath))
        {
            KMInfo("mount VfsPreloadLibrary failed");
        }

        if (!overlayHostPlugins(container, appDataPath, appDir))
        {
            KMInfo("mount overlayHostPlugins failed");
        }
    }
    return true;
}

/**
 * @brief 扫描插件目录中的软链接，并返回解析后的真实路径列表
 * @return 所有软链接的真实路径
 */
std::vector<std::string> KMRunnerPlugins::scanSymlinkParents()
{
    std::error_code ec;
    std::vector<std::string> result;

    for (const auto &sub : MOZILLA_PLUGIN_DIRS)
    {
        std::string dir = sub;
        if (!fs::exists(dir, ec))
        {
            ec.clear();
            continue;
        }

        fs::recursive_directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec);
        fs::recursive_directory_iterator end;

        for (; it != end; it.increment(ec))
        {
            if (ec)
            {
                ec.clear();
                continue;
            }

            const fs::path path = it->path();

            if (!fs::is_symlink(path))
                continue;

            fs::path realPath = fs::canonical(path, ec);
            if (ec)
            {
                ec.clear();
                continue;
            }

            std::string realParent = realPath.parent_path().string();
            if (realParent.empty())
                continue;

            realParent = normalizeOptPath(realParent);

            result.emplace_back(realParent);
        }
    }
    return result;
}

/**
* @brief 处理插件依赖并进行挂载
* @return true 存在 false 不存在
*/
void KMRunnerPlugins::handlePluginDependencies(KMContainer *container, const std::string &appDataPath)
{
    if (!container)
        return;

    static std::set<std::string> mountedDirs;
    std::vector<std::string> realPathsFromHost = scanSymlinkParents();

    // 遍历每个 realPath
    for (const auto &realPath : realPathsFromHost)
    {
        // 跳过 mozilla 目录
        if (std::any_of(MOZILLA_PLUGIN_DIRS.begin(), MOZILLA_PLUGIN_DIRS.end(), [&](const std::string &d) {
                return realPath.rfind(d, 0) == 0;
            }))
            continue;

        if (mountedDirs.count(realPath))
            continue;

        std::vector<std::string> overlaySources;

        overlaySources.emplace_back(realPath);

        // KM 目录
        for (const auto &base : KM_PLUGIN_DIRS)
        {
            std::string kmPath = base + realPath;
            if (fs::exists(kmPath))
                overlaySources.emplace_back(kmPath);
        }

        // KARE 目录
        std::string karePath = KARE_PLUGIN_DIR + realPath;
        if (fs::exists(karePath))
            overlaySources.emplace_back(karePath);

        // 挂载
        if (overlaySources.size() > 1)
        {
            KMRunnerMount::mountOverlay(container, appDataPath, overlaySources, "/symlinkplugins" + realPath, realPath);
        }
        else
        {
            std::vector<std::string> opts = { "rbind", "ro", "rprivate" };
            KMRunnerMount::mountDir(container, appDataPath, false, realPath, realPath, "bind", opts);
        }

        mountedDirs.insert(realPath);
    }
}

/**
 * @brief 处理/opt是软链接
 * @return true 存在 false 不存在
 */
std::string KMRunnerPlugins::normalizeOptPath(const std::string &path)
{
    // 缓存 /opt 的真实路径
    static std::string optRealPath;
    static bool optIsSymlink = false;

    // 初始化
    if (optRealPath.empty() && !optIsSymlink)
    {
        std::error_code ec;

        if (std::filesystem::is_symlink("/opt", ec))
        {
            auto real = std::filesystem::canonical("/opt", ec);
            if (!ec)
            {
                optRealPath = real.string();
                optIsSymlink = true;
            }
        }
    }

    if (optIsSymlink && path.rfind(optRealPath, 0) == 0)
    {
        return "/opt" + path.substr(optRealPath.length());
    }

    return path;
}

/**
 * @brief 为主机系统插件目录创建 overlay 挂载，实现容器内对主机插件的透明访问
 * @param container 容器对象指针
 * @param appDataPath 应用数据路径
 * @param appDir 应用安装根目录
 * @return true 挂载成功 false 挂载失败
 */
bool KMRunnerPlugins::overlayHostPlugins(KMContainer *container, const std::string &appDataPath, const std::string &appDir)
{
    std::vector<std::string> overlayDirs = { "/usr", "/lib", "/lib32", "/lib64", "/bin", "/opt", "/etc" };
    std::string rootDir = "/run/host/unionRootfs";

    for (const auto &dir : overlayDirs)
    {
        if (dir == "/opt")
        {
            // 调用处理 含opt 目录的函数
            processOptDirectory(container, appDataPath, appDir, rootDir, dir);
        }
        else
        {
            // 使用通用挂载函数处理其他目录
            mountOverlayForDir(container, appDataPath, appDir, rootDir, dir);
        }
    };
    return true;
}

/**
 * @brief 通用的叠加挂载函数
 * @param container 容器对象
 * @param appDataPath 应用数据路径
 * @param appDir 应用目录路径
 * @param rootDir 插件重定向根目录
 * @param targetDir 目标目录   
 */
void KMRunnerPlugins::mountOverlayForDir(KMContainer *container, const std::string &appDataPath, const std::string &appDir, const std::string &rootDir, const std::string &targetDir)
{
    std::vector<std::string> dirs = {
        appDir + "/files" + targetDir,
        appDir + "/files/usr" + targetDir,
        KARE_PLUGIN_DIR + targetDir,
    };
    for (const auto &dir : KM_PLUGIN_DIRS)
    {
        dirs.emplace_back(dir + targetDir);
    }

    std::error_code ec;

    // 处理宿主机目录插件 "/usr", "/lib", "/lib32", "/lib64", "/bin", "/opt", "/etc"
    if (fs::exists("/tmp/ostree/usr-normal-mode", ec)) // 临时运维模式
    {
        if (targetDir == "/usr")
        {
            dirs.emplace_back("/tmp/ostree/usr-normal-mode");
        }
    }
    else if (targetDir != "/opt")
    {
        dirs.emplace_back(targetDir);
    }

    std::vector<std::string> lowers;
    for (auto &d : dirs)
    {
        if (!fs::is_symlink(d, ec))
        {
            lowers.emplace_back(d);
        }
    }

    std::string rootDirs = rootDir + targetDir;
    KMRunnerMount::mountOverlay(container, appDataPath, lowers, "/unionRootfs", rootDirs, { "ro" });
}

/**
 * @brief 挂载预加载库
 * @param container 容器对象
 * @param appDataPath 应用数据路径
 * @return true 成功 false 失败
 */
bool KMRunnerPlugins::mountVfsPreloadLibrary(KMContainer *container, const std::string &appDataPath)
{
    std::string src = "/opt/kaiming-tools/lib/libvfspreload.so";
    std::string dst = "/usr/lib/libvfspreload.so";

    std::error_code ec;
    if (!fs::exists(src, ec))
    {
        KMInfo("vfs preload library not found: " + src);
    }
    std::vector<std::string> options = { "rbind", "ro", "rprivate" };

    // 挂载到 /usr/lib/
    KMRunnerMount::mountDir(container, appDataPath, false, src, dst, "bind", options);

    return true;
}

/**
 * @brief 处理 opt 目录的挂载
 * @param container 容器对象
 * @param appDataPath 应用数据路径
 * @param appDir 应用目录路径
 * @param rootDir 插件重定向根目录
 */
void KMRunnerPlugins::processOptDirectory(KMContainer *container, const std::string &appDataPath, const std::string &appDir, const std::string &rootDir, const std::string &targetDir)
{
    std::error_code ec;
    std::string optDir = targetDir;
    // 叠加挂载非宿主机 opt 目录
    mountOverlayForDir(container, appDataPath, appDir, rootDir, targetDir);

    // 处理宿主机/opt
    std::vector<std::string> options = { "rbind", "ro", "rprivate" };
    for (const auto &entry : fs::directory_iterator(optDir))
    {
        if (fs::is_symlink(entry.path(), ec))
            continue;

        std::string src = entry.path().string();

        if (src == "/opt/kaiming" || src == "/opt/kaiming-tools")
            continue;

        if (src == "/opt/apps")
        {
            // 特殊处理 /opt/apps：遍历，逐个 bind mount
            std::error_code ec;
            for (const auto &appEntry : fs::directory_iterator(src, ec))
            {
                if (ec)
                {
                    KMDebug("Failed to iterate /opt/apps: " + ec.message());
                    break;
                }

                std::string appSrc = appEntry.path().string();
                std::string appDst = rootDir + "/opt/apps/" + appEntry.path().filename().string();

                KMRunnerMount::mountDir(container, appDataPath, false, appSrc, appDst, "bind", options);
            }
        }
        else
        {
            // 普通目录：直接挂载整个目录
            std::string dst = rootDir + src;
            KMRunnerMount::mountDir(container, appDataPath, false, src, dst, "bind", options);
        }
    }
}
