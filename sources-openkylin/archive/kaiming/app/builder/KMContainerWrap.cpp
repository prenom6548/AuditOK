/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMContainerWrap.h"

#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <fstream>

#include "common/KMContainer.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/KMProcessUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMUnixSocketServer.h"
#include "common/KMDbusProxyCall.h"
#include "../run/KMRunnerCommon.h"

const std::map<std::string, std::string> KMContainerWrap::m_defalutEnvs = {
    // 注意：里面注释掉的属于非overlay模式时使用的
    // { "PATH", "/app/bin:/runtime/usr/bin:/runtime/bin:/usr/bin:/bin" },
    // { "XDG_CONFIG_DIRS", "/app/etc/xdg:/runtime/etc/xdg:/etc/xdg" },
    // { "XDG_DATA_DIRS", "/app/share:/runtime/usr/share:/runtime/share:/usr/share:/usr/share/runtime/share:/run/host/user-share:/run/host/share" },
    { "PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin" },
    { "XDG_CONFIG_DIRS", "/etc/xdg" },
    { "XDG_DATA_DIRS", "/usr/local/share:/usr/share:/run/host/user-share:/run/host/share" },
    { "LD_LIBRARY_PATH", "" },
    { "SHELL", "/bin/bash" },
    { "TEMP", "" },
    { "TEMPDIR", "" },
    { "TMP", "" },
    { "TMPDIR", "" },
    { "XDG_RUNTIME_DIR", "" },
    { "PYTHONPATH", "" },
    { "PERLLIB", "" },
    { "PERL5LIB", "" },
    { "XCURSOR_PATH", "" },
    { "GST_PLUGIN_PATH_1_0", "" },
    { "GST_REGISTRY", "" },
    { "GST_REGISTRY_1_0", "" },
    { "GST_PLUGIN_PATH", "" },
    { "GST_PLUGIN_SYSTEM_PATH", "" },
    { "GST_PLUGIN_SCANNER", "" },
    { "GST_PLUGIN_SCANNER_1_0", "" },
    { "GST_PLUGIN_SYSTEM_PATH_1_0", "" },
    { "GST_PRESET_PATH", "" },
    { "GST_PTP_HELPER", "" },
    { "GST_PTP_HELPER_1_0", "" },
    { "GST_INSTALL_PLUGINS_HELPER", "" },
    { "KRB5CCNAME", "" },
    { "XKB_CONFIG_ROOT", "" },
    { "GIO_EXTRA_MODULES", "" },
    { "GDK_BACKEND", "" },
};

const std::map<std::string, std::string> KMContainerWrap::m_develEnvs = {
    { "PKG_CONFIG_PATH", std::string("/usr/lib/pkgconfig:/usr/share/pkgconfig:/usr/lib/") + KMUtil::getLibraryDir() + "/pkgconfig" }
};

const std::vector<std::string> KMContainerWrap::m_copyEnvs = {
    "PWD",
    "GDMSESSION",
    "XDG_CURRENT_DESKTOP",
    "XDG_SESSION_DESKTOP",
    "DESKTOP_SESSION",
    "EMAIL_ADDRESS",
    "HOME",
    "HOSTNAME",
    "LOGNAME",
    "REAL_NAME",
    "TERM",
    "USER",
    "USERNAME",
    "QT4_IM_MODULE",
    "QT_IM_MODULE",
    "QT_QPA_PLATFORM",
    "QT_WAYLAND_SHELL_INTEGRATION",
    "QT_WAYLAND_FORCE_DPI",
    "XDG_SESSION_PATH",
    "GTK_IM_MODULE",
};

const std::vector<std::string> KMContainerWrap::m_copyNoDevelEnvs = {
    "LANG",
    "LANGUAGE",
    "LC_ALL",
    "LC_ADDRESS",
    "LC_COLLATE",
    "LC_CTYPE",
    "LC_IDENTIFICATION",
    "LC_MEASUREMENT",
    "LC_MESSAGES",
    "LC_MONETARY",
    "LC_NAME",
    "LC_NUMERIC",
    "LC_PAPER",
    "LC_TELEPHONE",
    "LC_TIME"
};

const std::vector<std::string> KMContainerWrap::m_overlayfs = {
    "/etc",
    "/usr",
    "/opt",
    "/bin",
    "/sbin",
    "/lib",
    "/lib32",
    "/lib64",
    "/libx32",
    "/var"
};

static bool isSudoOrRoot()
{
    uid_t euid = ::geteuid();
    if (euid == 0)
    {
        return true;
    }

    const char *user = std::getenv("USER");
    if (user != nullptr && std::string(user) == "root")
    {
        return true;
    }
    return false;
}

class KMContainerWrap::Private
{
public:
    std::map<std::string, std::string> m_envs;
    std::unique_ptr<KMContainer> m_container;
    std::shared_ptr<KMDeploy> m_baseDeploy;
    std::shared_ptr<KMDeploy> m_runtimeDeploy;
    std::string m_shareMountDir;
    std::string m_appDataDir;
    std::shared_ptr<KMContext> m_appContext;
    std::string m_upperdir; // 有安全标记的系统上，不允许修改部分文件
    std::string m_sockPath;
    KMUnixSocketServer m_server;
};

KMContainerWrap::KMContainerWrap()
    : d(std::make_unique<Private>())
{
    d->m_container = std::make_unique<KMContainer>();
    d->m_shareMountDir = KMStringUtils::buildFilename(KMUtil::getRealXdgRuntimeDir(), ".kaiming", "share-mount");
    KMFileUtils::mkpath(d->m_shareMountDir);
}

void KMContainerWrap::setUpperdir(const std::string &upperdir)
{
    d->m_upperdir = upperdir;
    KMFileUtils::mkpath(d->m_upperdir);
}

KMContainerWrap::~KMContainerWrap() = default;

void KMContainerWrap::setAppID(const std::string &id)
{
    d->m_container->setAppID(id);
    d->m_container->addAnnotation("kaiming.app.id", id);
}

void KMContainerWrap::setupAppDataDir(const std::string &id)
{
    d->m_container->setAppID(id);
    d->m_appDataDir = KMStorageDir::getDataDir(id);
    d->m_container->addAnnotation("kaiming.app.dir", d->m_appDataDir);

    if (::getenv("XDG_DATA_HOME"))
    {
        addEnv("HOST_XDG_DATA_HOME", ::getenv("XDG_DATA_HOME"));
    }

    if (::getenv("XDG_CONFIG_HOME"))
    {
        addEnv("HOST_XDG_CONFIG_HOME", ::getenv("XDG_CONFIG_HOME"));
    }

    if (::getenv("XDG_CACHE_HOME"))
    {
        addEnv("HOST_XDG_CACHE_HOME", ::getenv("XDG_CACHE_HOME"));
    }

    if (::getenv("XDG_STATE_HOME"))
    {
        addEnv("HOST_XDG_STATE_HOME", ::getenv("XDG_STATE_HOME"));
    }
}

void KMContainerWrap::initMinimalEnvs(bool devel, bool useLdSoCache)
{
    std::map<std::string, std::string> envs(m_defalutEnvs);

    if (!useLdSoCache)
    {
        // 注意：里面注释掉的属于非overlay模式时使用的
        // envs["LD_LIBRARY_PATH"] = "/app/lib:/runtime/usr/lib:/runtime/lib:/usr/lib:/lib:/lib64";
        envs["LD_LIBRARY_PATH"] = "/usr/local/lib:/usr/lib:/usr/lib64:/usr/lib32:/lib:/lib64:/lib32:/usr/lib/" + KMUtil::getLibraryDir();
    }

    if (devel)
    {
        for (auto &env : m_develEnvs)
        {
            envs.insert(env);
        }
    }

    for (auto &key : m_copyEnvs)
    {
        const char *value = ::getenv(key.c_str());
        if (value != nullptr)
        {
            envs[key] = value;
        }
        else
        {
            envs[key] = "";
        }
    }

    // if (!devel)
    {
        for (auto &key : m_copyNoDevelEnvs)
        {
            const char *value = ::getenv(key.c_str());
            if (value != nullptr)
            {
                envs[key] = value;
            }
            else
            {
                envs[key] = "";
            }
        }
    }

    return d->m_envs.swap(envs);
}

void KMContainerWrap::removeEnv(const std::string &key)
{
    d->m_envs.erase(key);
}

void KMContainerWrap::addEnv(const std::string &key, const std::string &value)
{
    std::string newValue = value;
    while (KMStringUtils::contains(newValue, "$"))
    {
        // value值如：$PATH:/app/bin 或 /app/bin:$PATH 或 $HOME/.config
        size_t start = newValue.find_first_of('$');
        size_t end_colon = newValue.find_first_of(':', start);
        size_t end_slash = newValue.find_first_of('/', start);
        size_t end = end_colon < end_slash ? end_colon : end_slash;

        std::string keyOfEnv;
        std::string replaceKey;
        if (std::string::npos == end)
        {
            keyOfEnv = newValue.substr(start);
            replaceKey = keyOfEnv;
            keyOfEnv = keyOfEnv.substr(1);
        }
        else
        {
            keyOfEnv = newValue.substr(start, end - start);
            if (end_colon == end)
            {
                replaceKey = keyOfEnv + ":";
            }
            else if (end_slash == end)
            {
                replaceKey = keyOfEnv + "/";
            }
            keyOfEnv = keyOfEnv.substr(1);
        }

        std::string old;
        if (d->m_envs.find(keyOfEnv) != d->m_envs.end())
        {
            old = d->m_envs[keyOfEnv];
        }
        if (!old.empty() && KMStringUtils::endsWith(replaceKey, ":"))
        {
            old += ":";
        }
        else if (!old.empty() && KMStringUtils::endsWith(replaceKey, "/"))
        {
            old += "/";
        }
        KMStringUtils::replace(newValue, replaceKey, old);
    }

    if (KMStringUtils::startsWith(newValue, ":"))
    {
        // value值如：:/app/bin，则认为是在原基础上追加；
        std::string old = d->m_envs[key];
        newValue = old + newValue;
    }
    else if (KMStringUtils::endsWith(newValue, ":"))
    {
        // value值如：/app/bin:，则认为是在原基础上增加前缀；
        std::string old = d->m_envs[key];
        newValue = newValue + old;
    }

    d->m_envs[key] = KMStringUtils::replace(newValue, "::", ":");
}

void KMContainerWrap::addMount(const std::string &src, const std::string &dest, const std::string &type, const std::vector<std::string> &options)
{
    d->m_container->addMount(Mount{ src, dest, type, options });
}

void KMContainerWrap::addShareMount(const std::string &name, const std::string &content, const std::string &dest, const std::vector<std::string> &options)
{
    std::string src = KMStringUtils::buildFilename(d->m_shareMountDir, name);
    fs::path parent = fs::path(src).parent_path();
    KMFileUtils::mkpath(parent.string());
    std::ofstream ofs(src, std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
        throw KMException("Filed to write file : " + src);
    }

    ofs << content << std::endl;
    ofs.close();

    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    ::chmod(src.c_str(), mode);

    d->m_container->addMount(Mount{ src, dest, "bind", options });
}

void KMContainerWrap::createUpperdirFile(const std::string &filename, const std::string &content)
{
    std::string src = KMStringUtils::buildFilename(d->m_upperdir, filename);
    fs::path parent = fs::path(src).parent_path();
    KMFileUtils::mkpath(parent.string());
    std::ofstream ofs(src, std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
        throw KMException("Filed to write file : " + src);
    }

    ofs << content << std::endl;
    ofs.close();

    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    ::chmod(src.c_str(), mode);
}

void KMContainerWrap::lowerToUpperForSubDir(const std::string &lowerdir, const std::string &upperdir)
{
    if (!fs::exists(lowerdir) || !fs::is_directory(lowerdir))
    {
        throw KMException(lowerdir + " lowerdir not exists");
    }

    if (!fs::exists(upperdir) || !fs::is_directory(upperdir))
    {
        throw KMException(upperdir + " upperdir not exists");
    }

    std::string dst;
    for (auto &entry : fs::directory_iterator(lowerdir))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        // 生成upperdir目录
        dst = std::string("/") + entry.path().filename().string();
        if (!KMStringUtils::contains(m_overlayfs, dst))
        {
            continue;
        }

        if (!fs::exists(upperdir + dst))
        {
            KMFileUtils::mkpath(upperdir + dst);
            if (entry.is_symlink())
            {
                continue;
            }

            // 创建各级子目录（目的是使得目录可写）
            // dupUpperDirTree(lowerdir + dst, upperdir + dst);
        }
    }
}

// void KMContainerWrap::dupUpperDirTree(const std::string &src, const std::string &dest)
// {
//     // 不具备访问权限的目录略过，不需要创建对应upperdir目录
//     if (::access(src.c_str(), R_OK) != 0)
//     {
//         return;
//     }

//     for (auto &entry : fs::directory_iterator(src))
//     {
//         if (!entry.is_directory())
//         {
//             continue;
//         }

//         std::string child = entry.path().filename().string();
//         if (child == "." || child == "..")
//         {
//             continue;
//         }

//         std::string destSubdir = KMStringUtils::buildFilename(dest, child);
//         KMFileUtils::mkpath(destSubdir);

//         if (entry.is_symlink())
//         {
//             continue;
//         }

//         std::string srcSubdir = KMStringUtils::buildFilename(src, child);
//         dupUpperDirTree(srcSubdir, destSubdir);
//     }
// }

void KMContainerWrap::setupOverlayfs(const std::string &base, const std::string &runtime, const std::string &upper, const std::string &work, const std::vector<std::string> &depends)
{
    if (base.empty() || !fs::exists(base))
    {
        throw KMException("Base must be set for overlayfs's lowerdir");
    }

    lowerToUpperForSubDir(base, upper);
    if (!runtime.empty())
    {
        lowerToUpperForSubDir(runtime, upper);
    }

    std::string src("overlay");
    std::string type("overlay");
    std::string upperDir, lowerdir, dst;
    std::vector<std::string> options;
    std::string workDir = std::string("workdir=") + work;

    for (auto &entry : fs::directory_iterator(base))
    {
        if (!entry.is_directory())
        {
            continue;
        }

        dst = std::string("/") + entry.path().filename().string();
        if (!KMStringUtils::contains(m_overlayfs, dst))
        {
            continue;
        }
        upperDir = std::string("upperdir=").append(upper + dst);

        lowerdir = "lowerdir=";

        for (const auto &depend : depends)
        {
            if (fs::exists(depend) && fs::exists(depend + dst))
            {
                lowerdir.append(depend + dst + string(":"));
            }
        }

        if (!runtime.empty() && fs::exists(runtime + dst))
        {
            lowerdir.append(runtime + dst + string(":"));
        }
        lowerdir.append(base + dst);

        // 注意workdir不能重复使用，不然可能有未知问题
        std::string subWordDir = workDir + dst;
        KMFileUtils::mkpath(work + dst);
        options = { lowerdir, upperDir, subWordDir };
        // options = { lowerdir, upperDir, subWordDir, "userxattr" };
        {
            // OverlayFS要求upperdir和workdir在同一时间只能被一个挂载点独占使用。若其他进程或容器已占用这些目录，新挂载会因资源冲突失败‌。使用-o index=off参数显式关闭索引保护，允许共享。
            options.emplace_back("index=off");
        }
        d->m_container->addMount(Mount{ src, dst, type, options });
        KMDebug("mount -t overlay overlay -o "+lowerdir+","+upperDir+","+subWordDir+" /run/user/${userid}/kaiming-builder/${id}/${uuid}/rootfs"+dst);
    }

    // 将base中的var目录挂载到容器/var
    // std::string varDir = KMStringUtils::buildFilename(base, "var");
    // if (fs::exists(varDir))
    // {
    //     d->m_container->addMount(Mount{ .source = varDir, .destination = "/var", .type = "bind", .options = { { "rbind", "rprivate" } } });
    // }
}

void KMContainerWrap::setupBase(const std::shared_ptr<KMDeploy> &baseDeploy)
{
    if (!baseDeploy)
    {
        return;
    }

    // base envs
    if (std::shared_ptr<KMInfoJson> infoJson = baseDeploy->m_infoJson; infoJson)
    {
        for (const auto &pair : infoJson->envs)
        {
            addEnv(pair.first, pair.second);
        }
    }
    addEnv("XDG_RUNTIME_DIR", KMUtil::getRealXdgRuntimeDir());

    d->m_container->setBaseDir(baseDeploy->m_dir);
    d->m_container->addAnnotation("kaiming.base.dir", baseDeploy->m_dir);
    d->m_baseDeploy = baseDeploy;

    mountBase();
}

void KMContainerWrap::setupRuntime(const std::shared_ptr<KMDeploy> &runtimeDeploy)
{
    if (!runtimeDeploy)
    {
        return;
    }

    // runtime envs
    std::shared_ptr<KMInfoJson> infoJson = runtimeDeploy->m_infoJson;
    if (infoJson)
    {
        for (const auto &pair : infoJson->envs)
        {
            addEnv(pair.first, pair.second);
        }
    }

    d->m_container->addAnnotation("kaiming.runtime.dir", runtimeDeploy->m_dir);
    d->m_runtimeDeploy = runtimeDeploy;
}

/**
 * @brief 挂载基础目录
 */
void KMContainerWrap::mountBase()
{
    std::string dst, src, type;
    std::vector<std::string> options;

    dst = "/proc";
    src = "/proc";
    type = "bind";
    options = { "rbind", "rprivate" };
    d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

    dst = "/sys";
    src = "tmpfs";
    type = "tmpfs";
    options = { "nodev", "nosuid", "rw", "noexec", "relatime" };
    d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

    dst = "/run";
    src = "tmpfs";
    type = "tmpfs";
    options = { "nodev", "nosuid", "rw", "noexec", "relatime", "mode=755" };
    d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

    dst = "/home";
    src = "tmpfs";
    type = "tmpfs";
    options = { "nodev", "nosuid", "rw", "mode=755" };
    d->m_container->addMount(Mount{ src, dst, type, options });

    dst = "/sys/fs/cgroup";
    src = "cgroup";
    type = "cgroup";
    options = { "nosuid", "noexec", "nodev", "relatime", "rw" };
    d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

    dst = "/sys/block";
    src = "/sys/block";
    type = "bind";
    options = { "rbind", "ro", "rprivate" };
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = "/sys/bus";
    src = "/sys/bus";
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = "/sys/class";
    src = "/sys/class";
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = "/sys/dev";
    src = "/sys/dev";
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = "/sys/devices";
    src = "/sys/devices";
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = string("/proc/self/ns/user");
    src = string("/run/.userns");
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    dst = "/etc/timezone";
    src = "/var/db/zoneinfo";
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }

    {
        dst = "/run/host/share";
        src = "/usr/share";
        type = "bind";
        options = { "rbind", "ro", "rprivate" };
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

        dst = "/run/host/user-share";
        src = "/usr/local/share";
        if (fs::exists(src))
        {
            d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
        }
    }

    auto *home = getenv("HOME");
    if (home == nullptr)
    {
        throw KMException("Couldn't get HOME from env");
    }
    if (fs::exists(home))
    {
        type = "bind";
        options = { "rbind", "rprivate" };
        d->m_container->addMount(Mount{ .source = string(home), .destination = string(home), .type = type, .options = options });
    }

    // 解决提示：QStandardPaths: wrong permissions on runtime directory /run/user/1000, 0755 instead of 0700
    if (fs::exists(KMUtil::getRealXdgRuntimeDir()))
    {
        // 挂载 /run/user/1000
        std::string dir = KMUtil::getRealXdgRuntimeDir();
        d->m_container->addMount(Mount{ .source = dir, .destination = dir, .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    {
        // LC_ALL等环境变量用的宿主机的，故/etc/default/locale也用宿主机的，允许容器内修改用于测试应用的多语言功能
        std::string content;
        KMFileUtils::readFile("/etc/default/locale", content);
        options = { "rbind", "rprivate" };
        // addShareMount("/etc/default/locale", content, "/etc/default/locale", options);
        createUpperdirFile("/etc/default/locale", content);

        if (isSudoOrRoot())
        {
            if (fs::exists("/etc/passwd"))
            {
                std::string passwd;
                KMFileUtils::readFile("/etc/passwd", passwd);
                createUpperdirFile("/etc/passwd", passwd);
            }
            if (fs::exists("/etc/group"))
            {
                std::string group;
                KMFileUtils::readFile("/etc/group", group);
                createUpperdirFile("/etc/group", group);
            }
            if (fs::exists("/etc/shadow"))
            {
                std::string shadow;
                KMFileUtils::readFile("/etc/shadow", shadow);
                createUpperdirFile("/etc/shadow", shadow);
            }
            if (fs::exists("/etc/gshadow"))
            {
                std::string gshadow;
                KMFileUtils::readFile("/etc/gshadow", gshadow);
                createUpperdirFile("/etc/gshadow", gshadow);
            }
        }
    }

    {
        // 安全编译用的，刘波那边要求加上的
        std::string content("zlib");
        options = { "rbind", "rprivate" };
        addShareMount("/CurrentlyBuilding", content, "/CurrentlyBuilding", options);
    }

    {
        // 使用宿主机的用户及密码、sudo权限
        options = { "rbind", "ro", "rprivate" };
        type = "bind";
        if (!isSudoOrRoot())
        {
            if (fs::exists("/etc/passwd"))
            {
                d->m_container->addMount(Mount{ "/etc/passwd", "/etc/passwd", type, options });
            }
            if (fs::exists("/etc/group"))
            {
                d->m_container->addMount(Mount{ "/etc/group", "/etc/group", type, options });
            }
            if (fs::exists("/etc/shadow"))
            {
                d->m_container->addMount(Mount{ "/etc/shadow", "/etc/shadow", type, options });
            }
            if (fs::exists("/etc/gshadow"))
            {
                d->m_container->addMount(Mount{ "/etc/gshadow", "/etc/gshadow", type, options });
            }
        }
        if (fs::exists("/etc/pkcs11/pkcs11.conf"))
        {
            d->m_container->addMount(Mount{ "/etc/pkcs11/pkcs11.conf", "/etc/pkcs11/pkcs11.conf", type, options });
        }

        if (fs::exists("/etc/machine-id"))
        {
            d->m_container->addMount(Mount{ "/etc/machine-id", "/etc/machine-id", type, options });
        }
        else if (fs::exists("/var/lib/dbus/machine-id"))
        {
            d->m_container->addMount(Mount{ "/var/lib/dbus/machine-id", "/etc/machine-id", type, options });
        }
    }

    setupTZData();

    setupMonitorPath();
}

/**
 * @brief : 设置系统时间挂载参数，如果系统的存在用系统的，系统的不存在，用base/files下的
 */
void KMContainerWrap::setupTZData()
{
    std::string rawTimezone = KMUtil::kmGetTimeZone();
    std::string timezoneContent = rawTimezone + "\n";
    std::string localtimeContent = KMStringUtils::buildFilename(d->m_baseDeploy->m_dir, "files", "usr/share/zoneinfo", rawTimezone);
    std::string runtimeTimezone = KMStringUtils::buildFilename(d->m_baseDeploy->m_dir, "files", "usr/share/zoneinfo");

    std::vector<std::string> options{ "rbind", "ro", "rprivate" };
    std::string type("bind");
    if (fs::exists(runtimeTimezone))
    {
        // 检查主机/usr/share/zoneinfo
        if (fs::is_directory("/usr/share/zoneinfo"))
        {
            // 这里我们假设主机时区文件存在于主机数据中
            d->m_container->addMount(Mount{ "/usr/share/zoneinfo", "/usr/share/zoneinfo", type, options });
            d->m_container->addMount(Mount{ KMStringUtils::buildFilename("/usr/share/zoneinfo", rawTimezone), "/etc/localtime", type, options });
        }
        else if (fs::exists(localtimeContent))
        {
            d->m_container->addMount(Mount{ localtimeContent, "/etc/localtime", type, options });
        }
    }

    // addShareMount("timezone", timezoneContent, "/etc/timezone", options);
    createUpperdirFile("/etc/timezone", timezoneContent);
}

void KMContainerWrap::setupMonitorPath()
{
    std::vector<std::string> options{ "rbind", "ro", "rprivate" };
    std::string type("bind");

    if (fs::exists("/etc/resolv.conf"))
    {
        d->m_container->addMount(Mount{ "/etc/resolv.conf", "/etc/resolv.conf", type, options });
    }

    if (fs::exists("/etc/host.conf"))
    {
        d->m_container->addMount(Mount{ "/etc/host.conf", "/etc/host.conf", type, options });
    }

    if (fs::exists("/etc/hosts"))
    {
        d->m_container->addMount(Mount{ "/etc/hosts", "/etc/hosts", type, options });
    }

    if (fs::exists("/etc/gai.conf"))
    {
        d->m_container->addMount(Mount{ "/etc/gai.conf", "/etc/gai.conf", type, options });
    }
}

void KMContainerWrap::applyContextEnvVars(const std::shared_ptr<KMContext> &context)
{
    for (const auto &pair : context->getEnvVars())
    {
        if (pair.second.empty())
        {
            removeEnv(pair.first);
        }
        else
        {
            addEnv(pair.first, pair.second);
        }
    }
}

void KMContainerWrap::setupDev()
{
    // 默认用于操作所有设备的权限
    std::string dst, src, type;
    std::vector<std::string> options;

    dst = "/dev";
    src = "tmpfs";
    type = "tmpfs";
    options = { "nosuid", "strictatime", "mode=0755", "size=65536k" };
    d->m_container->addMount(Mount{ src, dst, type, options });

    dst = "/dev";
    src = "/dev";
    type = "bind";
    options = { "rbind", "rprivate" };
    d->m_container->addMount(Mount{ src, dst, type, options });

    dst = "/dev/pts";
    src = "devpts";
    type = "devpts";
    options = { "nosuid", "noexec", "newinstance", "ptmxmode=0666", "mode=0620" };
    d->m_container->addMount(Mount{ src, dst, type, options });

    // 除非明确允许，否则不要公开主机/dev/shm，只公开设备节点
    if (fs::is_directory("/dev/shm"))
    {
        // 不要做任何特别的事情：在共享/dev中包含shm。主机和所有沙盒和子沙盒都共享/dev/shm
    }
    else if (fs::is_symlink("/dev/shm"))
    {
        // 在debian上（使用sysv init），主机/dev/shm是/run/shm的符号链接，因此我们无法在其上挂载。
        std::string link = fs::read_symlink("/dev/shm");
        if (link == "/run/shm" && fs::is_directory("/run/shm"))
        {
            d->m_container->addMount(Mount{ "/run/shm", "/run/shm", "bind", { { "rbind", "rprivate" } } });
        }
    }

    // 解决udev管控信息
    src = std::string("/run/udev");
    dst = std::string("/run/udev");
    type = std::string("bind");
    options = { "rbind", "rprivate" };
    if (fs::exists(src))
    {
        d->m_container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });
    }
}

void KMContainerWrap::setupFileSystem()
{
    // TODO: 文件系统权限控制

    std::string dst, src, type;
    std::vector<std::string> options;

    // --filesystem=/tmp
    dst = "/tmp";
    src = "/tmp";
    type = "bind";
    options = { "rbind", "rprivate" };
    d->m_container->addMount(Mount{ src, dst, type, options });

    if (fs::exists("/lost+found"))
    {
        d->m_container->addMount(Mount{ .source = "/lost+found", .destination = "/lost+found", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/backup"))
    {
        d->m_container->addMount(Mount{ .source = "/backup", .destination = "/backup", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/data"))
    {
        d->m_container->addMount(Mount{ .source = "/data", .destination = "/data", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/root"))
    {
        d->m_container->addMount(Mount{ .source = "/root", .destination = "/root", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/cdrom"))
    {
        d->m_container->addMount(Mount{ .source = "/cdrom", .destination = "/cdrom", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/mnt"))
    {
        d->m_container->addMount(Mount{ .source = "/mnt", .destination = "/mnt", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/media"))
    {
        d->m_container->addMount(Mount{ .source = "/media", .destination = "/media", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    if (fs::exists("/run/media"))
    {
        d->m_container->addMount(Mount{ .source = "/run/media", .destination = "/run/media", .type = "bind", .options = { { "rbind", "rprivate" } } });
    }

    d->m_container->addMount(Mount{ .source = "/var/log", .destination = "/var/log", .type = "bind", .options = { { "rbind", "rprivate" } } });
}

void KMContainerWrap::setupWayland()
{
    const char *waylandDisplay = ::getenv("WAYLAND_DISPLAY");
    if (!waylandDisplay)
    {
        waylandDisplay = "wayland-0";
    }

    std::string waylandSocket(waylandDisplay);
    std::string userRuntimeDir = KMUtil::getRealXdgRuntimeDir();
    if (waylandDisplay[0] != '/')
    {
        waylandSocket = KMStringUtils::buildFilename(userRuntimeDir, waylandDisplay);
    }

    if (!KMStringUtils::startsWith(waylandDisplay, "wayland-") || ::strchr(waylandDisplay, '/') != nullptr)
    {
        waylandDisplay = "wayland-0";
        addEnv("WAYLAND_DISPLAY", waylandDisplay);
    }

    std::string sandboxWaylandSocket = KMStringUtils::buildFilename("/run/kaiming", waylandDisplay);

    struct stat statbuf;
    if (::stat(waylandSocket.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFMT) == S_IFSOCK)
    {
        d->m_container->addMount(Mount{ waylandSocket, sandboxWaylandSocket, "bind", { { "rbind", "ro", "rprivate" } } });
    }
}

static bool parseX11Display(const std::string &display, std::string &x11Socket, std::string &remoteHost, std::string &displayNrOut)
{
    /* Use the last ':', not the first, to cope with [::1]:0 */
    size_t pos = display.find_last_of(':');
    if (std::string::npos == pos)
    {
        KMWarn("No colon found in DISPLAY=" + display);
        return false;
    }

    std::string colon = display.substr(pos);
    if (colon.length() > 1 && !::isdigit(colon.at(1)))
    {
        KMWarn("Colon not followed by a digit in DISPLAY=" + display);
        return false;
    }

    int nr = 1;
    while (colon.length() > nr && ::isdigit(colon.at(nr)))
    {
        ++nr;
    }

    displayNrOut = colon.substr(1, nr - 1);

    if (display == colon || KMStringUtils::startsWith(display, "unix:"))
    {
        x11Socket = "/tmp/.X11-unix/X" + displayNrOut;
    }
    else if (display.at(0) == '[' && display.at(pos - 1) == ']')
    {
        remoteHost = display.substr(1, pos - 2);
    }
    else
    {
        remoteHost = display.substr(0, pos - 1);
    }

    return true;
}

void KMContainerWrap::setupX11()
{
    /* 
       始终覆盖/tmp/.X11unix，这样我们就永远看不到主机，以防我们可以访问主机/tmp。如果你请求X访问，我们无论如何都会把正确的东西放在这里。
       我们在这里需要小心一点，因为有两种情况下，潜在的恶意进程可以访问/tmp并创建符号链接，原则上这可能会导致我们创建目录并将tmpfs挂载到符号链接的目标位置，而不是预期的位置：
       使用--filesystem=/tmp时，它是主机/tmp，但由于/tmp/的特殊历史状态。X11unix，我们可以假设它是由主机系统在用户代码运行之前预先创建的。
       当/tmp在同一应用程序ID的所有实例之间共享时，原则上应用程序可以控制/tmp中的内容，但实际上它不能干扰/tmp/.X11unix，
       因为我们无条件地这样做——因此在应用程序代码运行时，/tmp/.X11 unix已经是一个挂载点，这意味着应用程序无法重命名或删除它。
    */

    std::string display = getenv("DISPLAY") == nullptr ? "" : getenv("DISPLAY");
    std::string x11Socket;
    if (!display.empty())
    {
        std::string remoteHost, displayNr;
        if (!parseX11Display(display, x11Socket, remoteHost, displayNr))
        {
            removeEnv("DISPLAY");
            return;
        }

        assert(!displayNr.empty());

        if (!x11Socket.empty() && fs::exists(x11Socket))
        {
            assert(KMStringUtils::startsWith(x11Socket, "/tmp/.X11-unix/X"));
            d->m_container->addMount(Mount{ x11Socket, x11Socket, "bind", { { "rbind", "ro", "rprivate" } } });
            addEnv("DISPLAY", display);
        }
        else if (!x11Socket.empty())
        {
            KMWarn("X11 socket " + x11Socket + "does not exist in filesystem, trying to use abstract socket instead.");
        }
        else
        {
            KMDebug("Assuming --share=network gives access to remote X11");
        }

        char *pXauthFile = ::getenv("XAUTHORITY");
        if (pXauthFile != nullptr && fs::exists(pXauthFile))
        {
            std::string dest = "/run/kaiming/Xauthority";
            d->m_container->addMount(Mount{ pXauthFile, dest, "bind", { { "rbind", "ro", "rprivate" } } });
            addEnv("XAUTHORITY", dest);
        }
    }
    else
    {
        removeEnv("DISPLAY");
    }
}

void KMContainerWrap::setupSsh()
{
    std::string sandboxAuthSocket = "/run/kaiming/ssh-auth";
    const char *authSocket = ::getenv("SSH_AUTH_SOCK");

    if (!authSocket)
    {
        return; /* ssh agent not present */
    }

    fs::path authSock(authSocket);
    if (!fs::exists(authSock))
    {
        /* Let's clean it up, so that the application will not try to connect */
        removeEnv("SSH_AUTH_SOCK");
        return;
    }

    d->m_container->addMount(Mount{ authSocket, sandboxAuthSocket, "bind", { { "rbind", "ro", "rprivate" } } });
    addEnv("SSH_AUTH_SOCK", sandboxAuthSocket);
}

/* Try to find a default server from a pulseaudio confguration file */
std::string KMContainerWrap::getPulseaudioServerUserConfig(const std::string &path)
{
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line))
    {
        line = KMStringUtils::strstrip(line);
        if (line.empty())
        {
            continue;
        }

        if (line.at(0) == ';' || line.at(0) == '#')
        {
            continue;
        }

        if (KMStringUtils::startsWith(line, ".include "))
        {
            std::string rec_path = line.substr(9);
            rec_path = KMStringUtils::strstrip(rec_path);
            std::string found = getPulseaudioServerUserConfig(rec_path);
            if (!found.empty())
            {
                return found;
            }
        }
        else if (KMStringUtils::startsWith(line, "["))
        {
            return "";
        }
        else
        {
            std::vector<std::string> parts = KMStringUtils::splitString(line, "=");
            if (parts.size() == 2)
            {
                std::string key = KMStringUtils::strstrip(parts.at(0));
                if (key == "default-server")
                {
                    std::string value = KMStringUtils::strstrip(parts.at(1));
                    KMDebug("Found pulseaudio socket from configuration file '" + path + "' : " + value);
                    return value;
                }
            }
        }
    }

    return "";
}

std::string KMContainerWrap::getPulseaudioServer()
{
    const char *pulseServerEnv = ::getenv("PULSE_SERVER");
    if (pulseServerEnv != nullptr)
    {
        return pulseServerEnv;
    }

    const char *pulseClientconfig = ::getenv("PULSE_CLIENTCONFIG");
    if (pulseClientconfig != nullptr)
    {
        return getPulseaudioServerUserConfig(pulseClientconfig);
    }

    std::string pulseUserConfig = KMStringUtils::buildFilename(g_get_user_config_dir(), "pulse/client.conf");
    std::string pulseServer = getPulseaudioServerUserConfig(pulseUserConfig);
    if (!pulseServer.empty())
    {
        return pulseServer;
    }

    pulseServer = getPulseaudioServerUserConfig("/etc/pulse/client.conf");
    if (!pulseServer.empty())
    {
        return pulseServer;
    }

    return "";
}

std::string KMContainerWrap::parsePulseServer(const std::string &value, bool &remote)
{
    std::vector<std::string> servers = KMStringUtils::splitString(value, " ");

    for (std::string server : servers)
    {
        if (KMStringUtils::startsWith(server, "{"))
        {
            /*
             * TODO: compare the value within {} to the local hostname and D-Bus machine ID,
             * and skip if it matches neither.
             */
            size_t pos = server.find_first_of('}');
            if (std::string::npos == pos)
            {
                continue;
            }

            server = server.substr(pos + 1);
        }

        if (KMStringUtils::startsWith(server, "unix:"))
        {
            return server.substr(5);
        }

        if ('/' == server.at(0))
        {
            return server;
        }

        if (KMStringUtils::startsWith(server, "tcp:"))
        {
            remote = true;
            return "";
        }
    }

    return "";
}

void KMContainerWrap::setupPulseaudio()
{
    /*
        PulseAudio是一个跨平台的、可通过网络工作的声音服务，主要用于Linux和FreeBSD操作系统。
        PulseAudio的主要功能包括音频路由和混音、高级音频操作以及网络透明性。它能够将音频从一个源路由到一个或多个接收点，也能够将来自不同源的音频混合到一个接收点，从而绕过硬件的任何限制。
        此外，PulseAudio还允许在音频数据在应用程序和硬件之间传递时执行高级操作，例如将音频传输到不同的机器，更改样本格式或通道计数，或将多个声音混合到一个输入/输出。

        PulseAudio的设计和实现也体现了其高度的灵活性和可扩展性。它通过模块化的设计，使得许多服务器功能都在模块中实现，包括网络协议、设备驱动程序、桌面集成等。
        这种设计不仅使得PulseAudio能够适应各种复杂的音频处理需求，也使得开发者可以根据需要添加或修改功能，以满足特定的应用场景。

        总的来说，PulseAudio是一个功能强大且灵活的声音服务器，它为Linux和其他POSIX兼容系统提供了一种高效、安全且易于管理的音频处理解决方案。
        无论是对于普通用户还是专业开发者来说，PulseAudio都是一个非常有价值的工具。
    */

    bool remote = false;
    std::string pulseaudioSocket;
    std::string pulseaudioServer = getPulseaudioServer();

    if (!pulseaudioServer.empty())
    {
        pulseaudioSocket = parsePulseServer(pulseaudioServer, remote);
    }

    if (pulseaudioSocket.empty() && !remote)
    {
        std::string pulseRuntimeDir = KMUtil::getPulseRuntimeDir();
        pulseaudioSocket = KMStringUtils::buildFilename(pulseRuntimeDir, "native");

        if (!fs::exists(pulseaudioSocket))
        {
            pulseaudioSocket = "";
        }
    }

    if (pulseaudioSocket.empty() && !remote)
    {
        pulseaudioSocket = KMFileUtils::realpath("/var/run/pulse/native");
        if (!pulseaudioSocket.empty() && !fs::exists(pulseaudioSocket))
        {
            pulseaudioSocket = "";
        }
    }

    removeEnv("PULSE_SERVER");

    if (remote)
    {
        KMDebug("Using remote PulseAudio server \"" + pulseaudioServer + "\"");
        addEnv("PULSE_SERVER", pulseaudioServer);
    }
    else if (!pulseaudioSocket.empty() && fs::exists(pulseaudioSocket))
    {
        const char *sandboxSocketPath = "/run/kaiming/pulse/native";
        const char *pulseServer = "unix:/run/kaiming/pulse/native";
        const char *configPath = "/run/kaiming/pulse/config";
        bool shareShm = false; /* TODO: When do we add this? */
        std::string clientConfig = std::string("enable-shm=") + (shareShm ? "yes" : "no");
        addShareMount("pulseaudio", clientConfig, configPath, { "rbind", "ro", "rprivate" });
        d->m_container->addMount(Mount{ pulseaudioSocket, sandboxSocketPath, "bind", { { "rbind", "ro", "rprivate" } } });
        addEnv("PULSE_SERVER", pulseServer);
        addEnv("PULSE_CLIENTCONFIG", configPath);
    }
    else
    {
        KMDebug("Could not find pulseaudio socket");
    }

    /* 
        还允许ALSA访问。名称并不理想。然而，由于ALSA和PulseAudio的实际许可基本相同，并且由于我们不想为我们计划在未来用门户网站/管道取代的东西添加更多许可，
        我们重新解释了PulseAudio也意味着ALSA。
     */
    if (!remote && fs::is_directory(fs::path(std::string("/dev/snd"))))
    {
        d->m_container->addMount(Mount{ "/dev/snd", "/dev/snd", "bind", { { "rbind", "rprivate" } } });
    }
}

void KMContainerWrap::setupPcsc()
{
    /*
        **PCSC是个人计算机与智能卡之间的通信标准，用于提供安全的两端通讯连接和数据交换服务**。以下是对PCSC的详细解析：

        1. **基本概述**：
        - PCSC全称为Personal Computer-Smart Card，是一种允许个人计算机与智能卡进行交互的通信协议[^1^]。
        - 该标准定义了如何在计算机系统中使用智能卡，包括数据的读取、写入以及安全认证等操作。
        2. **主要功能**：
        - PCSC提供了一套完整的API接口，使得开发者能够方便地在应用程序中集成智能卡功能。通过这些API，可以实现对智能卡内数据的读写、加密解密等操作。
        - 由于智能卡通常用于存储敏感信息，如用户身份验证数据、银行账户信息等，因此PCSC特别注重通信的安全性。它采用了多种加密算法和安全机制来确保数据传输过程中的安全。
        3. **应用场景**：
        - 在需要高安全性的身份验证场景下，如登录系统、电子签名等，PCSC可以发挥重要作用。通过使用智能卡作为身份标识，结合PCSC提供的API进行身份验证，可以有效防止身份伪造和盗用。
        - 在金融领域，许多银行业务都需要使用到智能卡，如ATM取款、转账等。PCSC为这些业务提供了可靠的技术支持，确保了交易过程的安全性和便捷性。
        4. **技术特点**：
        - PCSC具有跨平台性，可以在Windows、Linux等多种操作系统上运行。这使得开发者可以使用同一套代码在不同的平台上实现智能卡功能，降低了开发成本和维护难度。
        - PCSC支持多种智能卡类型，包括但不限于接触式卡片、非接触式卡片以及双界面卡片等。这使得用户可以根据自己的需求选择合适的智能卡类型进行使用。
        5. **未来展望**：
        - 随着物联网技术的不断发展，越来越多的设备开始具备联网功能并需要使用到智能卡进行身份验证或数据存储。PCSC作为一种成熟的智能卡通信标准，有望在这些领域得到更广泛的应用和发展。
        - 随着人工智能技术的不断进步，未来的智能卡可能会具备更加强大的数据处理能力和智能化程度。PCSC也将随之升级和完善以适应这些变化并满足更高的性能要求。

        总的来说，PCSC作为一种重要的智能卡通信标准，在多个领域都有着广泛的应用前景和发展潜力。
    */
    const char *sandboxPcscSocket = "/run/pcscd/pcscd.comm";

    const char *pcscSocket = getenv("PCSCLITE_CSOCK_NAME");
    if (pcscSocket)
    {
        if (!fs::exists(fs::path(std::string(pcscSocket))))
        {
            removeEnv("PCSCLITE_CSOCK_NAME");
            return;
        }
    }
    else
    {
        pcscSocket = "/run/pcscd/pcscd.comm";
        if (!fs::exists(fs::path(std::string(pcscSocket))))
        {
            return;
        }
    }

    d->m_container->addMount(Mount{ pcscSocket, sandboxPcscSocket, "bind", { { "rbind", "ro", "rprivate" } } });
    addEnv("PCSCLITE_CSOCK_NAME", sandboxPcscSocket);
}

bool KMContainerWrap::cupsCheckServerIsSocket(const std::string &server)
{
    if (KMStringUtils::startsWith(server, "/") && std::string::npos == server.find_first_of(':'))
    {
        return true;
    }

    return false;
}

/* Try to find a default server from a cups confguration file */
std::string KMContainerWrap::getCupsServerNameConfig(const std::string &path)
{
    std::ifstream file(path);
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            line = KMStringUtils::strstrip(line);

            if (line.empty() || line.at(0) == '#')
            {
                continue;
            }

            std::vector<std::string> tokens = KMStringUtils::splitString(line, " ");
            if (tokens.size() != 2)
            {
                continue;
            }

            if (tokens.at(0).empty() || tokens.at(1).empty())
            {
                continue;
            }

            if (tokens.at(0) == "ServerName")
            {
                std::string server = tokens.at(1);
                server = KMStringUtils::strstrip(server);
                if (cupsCheckServerIsSocket(server))
                {
                    return server;
                }
            }
        }
    }
    else
    {
        KMDebug("Cann't open CUPS configuration file : " + path);
    }

    return "";
}

std::string KMContainerWrap::getCupsServerName()
{
    // 我们目前不支持位于网络上的cups服务器，如果检测到此类服务器，我们只会忽略它，在最坏的情况下，我们会回退到默认套接字
    const char *cupsServer = getenv("CUPS_SERVER");
    if (cupsServer && cupsCheckServerIsSocket(cupsServer))
    {
        return cupsServer;
    }

    std::string cupsConfigPath = KMStringUtils::buildFilename(g_get_home_dir(), ".cups/client.conf");
    std::string cupsServerString = getCupsServerNameConfig(cupsConfigPath);
    if (!cupsServerString.empty())
    {
        return cupsServerString;
    }

    cupsServerString = getCupsServerNameConfig("/etc/cups/client.conf");
    if (!cupsServerString.empty())
    {
        return cupsServerString;
    }

    // Fallback to default socket
    return "/var/run/cups/cups.sock";
}

void KMContainerWrap::setupCups()
{
    /*
        **CUPS是一个广泛使用的打印系统，支持多种操作系统，并提供了丰富的功能如网络打印和队列管理等**。以下是对CUPS的详细解析：

        1. **基本概述**：
        - CUPS是Common UNIX Printing System的缩写，即通用Unix打印系统[^3^]。它是一个集成的服务，包括前端接收打印命令的相关程序、后端控制打印机硬件的程序以及中间的打印驱动。
        2. **主要功能**：
        - CUPS使用IPP（Internet Printing Protocol）来管理打印工作及队列，但同时也支持LPD（Line Printer Daemon）、SMB（Server Message Block）以及Socket等通信协议[^3^]。
        - 它提供了一套完整的管理打印设备、实现可靠打印、网络打印的方案。
        3. **技术特点**：
        - CUPS在接收到打印命令时，会进行一个格式的预转换，比如将.jpg等各种文件先转换成PostScript格式，然后进一步将PostScript格式转换成CUPS内部格式[^3^]。
        - 在使用过程中需要安装解释性的驱动，让CUPS能通过这类驱动将信息转换成一般打印机能识别的信息。现在使用最多的这类驱动名叫GhostScript[^3^]。
        4. **应用场景**：
        - 在Linux下进行CUPS驱动开发是遵循GPL协议规定的，也就是说必须开源[^3^]。
        - CUPS可以用于搭建打印服务器，支持移动设备如安卓手机、平板等进行无线打印[^5^]。
        5. **未来展望**：
        - 随着物联网技术的不断发展，越来越多的设备开始具备联网功能并需要使用到打印服务。CUPS作为一种成熟的打印系统解决方案，有望在这些领域得到更广泛的应用和发展。
        - 随着人工智能技术的不断进步，未来的打印机可能会具备更加强大的数据处理能力和智能化程度。CUPS也将随之升级和完善以适应这些变化并满足更高的性能要求。

        总的来说，CUPS作为通用Unix打印系统，为打印服务提供了强大而灵活的支持。通过了解其基本概述、主要功能、技术特点、应用场景以及未来展望等方面的内容，我们可以更好地理解CUPS的作用和价值所在。
    */
    std::string sandboxServerName = "/var/run/cups/cups.sock";
    std::string cupsServerName = getCupsServerName();

    if (!fs::exists(cupsServerName))
    {
        KMDebug("Could not find CUPS server");
        return;
    }

    d->m_container->addMount(Mount{ cupsServerName, sandboxServerName, "bind", { { "rbind", "ro", "rprivate" } } });
}

void KMContainerWrap::setupGpgAgent()
{
    std::string agentSocket;
    KMProcessUtils::spawn("gpgconf", { "--list-dir", "agent-socket" }, agentSocket);
    KMStringUtils::replace(agentSocket, "\n", "");
    if (agentSocket.empty() || agentSocket.at(0) != '/')
    {
        KMDebug("'gpgconf --list-dir agent-socket' no result");
        return;
    }

    std::string sandboxAgentSocket = KMStringUtils::buildFilename(KMUtil::getRealXdgRuntimeDir(), "/gnupg/S.gpg-agent");
    if (KMStringUtils::startsWith(agentSocket, KMUtil::getRealXdgRuntimeDir()))
    {
        // 宿主机${XDG-RUNTIME-DIR}目录已经挂载到容器的${XDG-RUNTIME-DIR}，无需再将宿主机中的对应的子目录挂载进容器
        return;
    }
    if (fs::exists(sandboxAgentSocket))
    {
        d->m_container->addMount(Mount{ agentSocket, sandboxAgentSocket, "bind", { { "rbind", "ro", "rprivate" } } });
    }
}

std::string KMContainerWrap::extractUnixPathFromDbusAddress(const std::string &address)
{
    if (address.empty())
    {
        return address;
    }

    if (!KMStringUtils::startsWith(address, "unix:"))
    {
        return "";
    }

    size_t pos = address.find("path=");
    if (std::string::npos == pos)
    {
        return "";
    }

    pos += strlen("path=");

    size_t posEnd = pos;
    while (posEnd < address.length() && address.at(posEnd) != ',')
    {
        ++posEnd;
    }

    return address.substr(pos, posEnd - pos);
}

void KMContainerWrap::setupSessionDbus()
{
    std::string sandboxSocketPath("/run/kaiming/bus");
    std::string sandboxDbusAddress("unix:path=/run/kaiming/bus");
    std::string dbusSessionSocket;

    const char *dbusAddress = ::getenv("DBUS_SESSION_BUS_ADDRESS");
    if (dbusAddress != nullptr)
    {
        dbusSessionSocket = extractUnixPathFromDbusAddress(dbusAddress);
    }
    else
    {
        std::string userRuntimeDir = KMUtil::getRealXdgRuntimeDir();
        dbusSessionSocket = KMStringUtils::buildFilename(userRuntimeDir, "bus");

        struct stat statbuf;
        if (::stat(dbusSessionSocket.c_str(), &statbuf) < 0 || (statbuf.st_mode & S_IFMT) != S_IFSOCK || statbuf.st_uid != ::getuid())
        {
            KMInfo("Failed to stat " + dbusSessionSocket);
            return;
        }
    }

    if (!dbusSessionSocket.empty())
    {
        if (fs::exists(dbusSessionSocket))
        {
            d->m_container->addMount(Mount{ dbusSessionSocket, sandboxSocketPath, "bind", { { "rbind", "ro", "rprivate" } } });
            addEnv("DBUS_SESSION_BUS_ADDRESS", sandboxDbusAddress);
        }

        return;
    }

    return;
}

void KMContainerWrap::setupSystemDbus()
{
    std::string dbusSystemSocket;
    std::string dbusSystemSocketString("/var/run/dbus/system_bus_socket");
    const char *dbusAddress = ::getenv("DBUS_SYSTEM_BUS_ADDRESS");
    if (dbusAddress != nullptr)
    {
        dbusSystemSocket = extractUnixPathFromDbusAddress(dbusAddress);
    }
    else if (fs::exists(dbusSystemSocketString))
    {
        dbusSystemSocket = dbusSystemSocketString;
    }

    if (!dbusSystemSocket.empty())
    {
        d->m_container->addMount(Mount{ dbusSystemSocket, "/run/dbus/system_bus_socket", "bind", { { "rbind", "ro", "rprivate" } } });
        addEnv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/run/dbus/system_bus_socket");

        return;
    }

    return;
}

void KMContainerWrap::setupA11yDbus()
{
    //暂不使用
}

void KMContainerWrap::setupGssproxy()
{
    //暂不使用
}

void KMContainerWrap::setupResolved()
{
    std::string resolved = "/run/systemd/resolve/io.systemd.Resolve";
    if (fs::exists(resolved))
    {
        d->m_container->addMount(Mount{ .source = resolved, .destination = resolved, .type = "bind", .options = { { "rbind", "ro", "rprivate" } } });
    }
}

void KMContainerWrap::setupJournal()
{
    std::string journalSocket = "/run/systemd/journal/socket";
    std::string journalStdout = "/run/systemd/journal/stdout";
    if (fs::exists(journalSocket))
    {
        d->m_container->addMount(Mount{ .source = journalSocket, .destination = journalSocket, .type = "bind", .options = { { "rbind", "ro", "rprivate" } } });
    }
    if (fs::exists(journalStdout))
    {
        d->m_container->addMount(Mount{ .source = journalStdout, .destination = journalStdout, .type = "bind", .options = { { "rbind", "ro", "rprivate" } } });
    }
}

void KMContainerWrap::setupBluetooth()
{
    Seccomp seccomp;
    seccomp.defaultAction = "SCMP_ACT_ALLOW";
    seccomp.architectures = { "SCMP_ARCH_X86_64", "SCMP_ARCH_ARM", "SCMP_ARCH_AARCH64" };

    Syscall syscall;
    syscall.names = { "socket" };
    syscall.action = "SCMP_ACT_ERRNO";
    syscall.errnoRet = 1;

    SyscallArg arg;
    arg.index = 0;
    arg.op = "SCMP_CMP_EQ";
    arg.value = 31;

    syscall.args.push_back(arg);
    seccomp.syscalls.push_back(syscall);
    d->m_container->setSeccomp(seccomp);
}

void KMContainerWrap::setupAppContext(const std::shared_ptr<KMContext> &context)
{
    d->m_appContext = context;

    applyContextEnvVars(context);

    // 放开全部权限
    // 随着业务的发展，构建环境和运行环境的差异越来越大；很多功能需要借助安装等操作才能正常使用，故不再追求构建环境能够测试验证应用所有的功能。
    setupUserMap();
    setupDev();
    setupFileSystem();
    setupWayland();
    setupX11();
    setupSsh();
    setupPulseaudio();
    setupPcsc();
    setupCups();
    setupGpgAgent();
    setupSessionDbus();
    setupSystemDbus();
    setupA11yDbus();
    setupGssproxy();
    setupResolved();
    setupJournal();
    // mountXdgOpen();
    // setupNetWork();
    setupBluetooth();
    setProcessCap(allCap);
}

void KMContainerWrap::setProcess(const Process &p)
{
    d->m_container->setProcess(p);
}

Process KMContainerWrap::getProcess()
{
    return d->m_container->getProcess();
}

void KMContainerWrap::setupProcessEnvs()
{
    std::vector<std::string> envs;

    for (const auto &pair : d->m_envs)
    {
        if (pair.second.empty() || pair.first.empty())
        {
            continue;
        }

        envs.emplace_back(pair.first + "=" + pair.second);
    }

    d->m_container->setProcessEnv(envs);
}

void KMContainerWrap::setupUserMap()
{
    if (isSudoOrRoot())
    {
        // d->m_container->setNameSpace({ { "mount" }, { "uts" } });
    }
    else
    {
        setUidMappings({ { getuid(), getuid(), 1 } });
        setGidMappings({ { getgid(), getgid(), 1 } });
        // d->m_container->setNameSpace({ { "mount" }, { "uts" }, { "user" } });
        d->m_container->addNameSpace("user");
    }
}

/**
 * @brief 设置uid映射
 * @param uidMappings uid映射数组
 */
void KMContainerWrap::setUidMappings(const std::vector<IDMap> &uidMappings)
{
    d->m_container->setUidMappings(uidMappings);
}

/**
 * @brief 设置gid映射
 * @param gidMappings gid映射数组
 */
void KMContainerWrap::setGidMappings(const std::vector<IDMap> &gidMappings)
{
    d->m_container->setGidMappings(gidMappings);
}

bool KMContainerWrap::run()
{
    return d->m_container->run();
}

/**
 * @brief 设置process的cap能力
 * @param cap 能力列表
 * @return true 成功 false 失败
 */
void KMContainerWrap::setProcessCap(const std::vector<int> &cap)
{
    d->m_container->setLinuxCapabilities(cap);
    d->m_container->setProcessBoundingCapabilities(cap);
    d->m_container->setProcessAmbientCapabilities(cap);
    d->m_container->setProcessEffectiveCapabilities(cap);
    d->m_container->setProcessInheritableCapabilities(cap);
    d->m_container->setProcessPermittedCapabilities(cap);
}
