/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMStorageDir.h"

#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include "KMConfig.h"
#include "KMProcessUtils.h"
#include "KMStringUtils.h"
#include "KMFileUtils.h"
#include "KMLogger.h"

class KMStorageDir::Private
{
public:
    bool m_user;
    std::string m_baseDir;
};

KMStorageDir::KMStorageDir() {}

KMStorageDir::KMStorageDir(const KMStorageDir &other)
{
    d = other.d;
}

KMStorageDir::KMStorageDir(const std::string &path, bool user)
    : d(std::make_shared<KMStorageDir::Private>())
{
    d->m_baseDir = path;
    d->m_user = user;

    /* Work around possible deadlock due to: https://bugzilla.gnome.org/show_bug.cgi?id=674885 */
    // g_type_ensure (G_TYPE_UNIX_SOCKET_ADDRESS);
}

KMStorageDir::~KMStorageDir() {}

/**
 * @brief : 获取KMDir对象路径
 */
std::string KMStorageDir::getBasePath()
{
    return d->m_baseDir;
}

/**
 * @brief : 获取开明包系统级安装的base目录
 * @return: 开明包系统级安装的base目录
 */
std::string KMStorageDir::getSystemDefaultBaseDirLocation()
{
    const char *systemDir = std::getenv("KAIMING_SYSTEM_DIR");
    if (nullptr == systemDir || *systemDir == '0')
    {
        return std::string(KAIMING_SYSTEM_PATH) + "/layers";
    }

    return std::string(systemDir) + "/layers";
}

/**
 * @brief : 获取用户数据目录，一般为XDG_DATA_DIR
 * @return: 用户数据目录
 */
std::string KMStorageDir::getUserDataDirLocation()
{
    const char *home = std::getenv("HOME");
    if (nullptr == home || *home == '0')
    {
        return "/home/" + getLoginUser() + "/.local/share";
    }

    std::string userHome(home);
    if (userHome == "/root")
    {
        return "/home/" + getLoginUser() + "/.local/share";
    }

    return std::string(home) + "/.local/share";
}

std::string KMStorageDir::getUserDevelDataDirLocation()
{
    const char *sudoUser = getenv("SUDO_USER");
    if (sudoUser)
    {
        struct passwd *pw = getpwnam(sudoUser);
        if (pw)
        {
            return std::string(fs::path(pw->pw_dir) / ".local" / "share"); // 使用原始用户的主目录
        }
        else
        {
            return std::string("/home/") + sudoUser + std::string("/.local/share");
        }
    }
    return getUserDataDirLocation();
}

/**
 * @brief : 获取开明包用户级安装base目录
 * @return: 开明包用户级安装base目录
 */
std::string KMStorageDir::getUserBaseDirLocation()
{
    const char *userDir = std::getenv("KAIMING_USER_DIR");
    if (nullptr == userDir || *userDir == '0')
    {
        return KMStorageDir::getUserDevelDataDirLocation() + "/kaiming/layers";
    }

    return std::string(userDir) + "/layers";
}

/**
 *  @brief: 获取当前登录用户
 */
std::string KMStorageDir::getLoginUser()
{
    const char *command = "loginctl list-sessions --no-legend | awk '/seat0/ {print $3}' | head -n 1 ";

    const auto ret = KMProcessUtils::execCmd(command);

    return ret.first.empty() ? "" : ret.first;
}

std::string KMStorageDir::getLoginUserPrimaryGroup() {
    std::string username = getLoginUser();
    if (username.empty()) {
        return "";
    }

    struct passwd *pwd = getpwnam(username.c_str());
    if (!pwd) {
        return "";
    }

    struct group *grp = getgrgid(pwd->pw_gid);
    if (!grp) {
        return "";
    }

    return grp->gr_name;
}

/**
 * @brief : 获取开明包用户安装目录KMStorageDir对象
 * @return: 开明包用户用户安装目录KMStorageDir对象
 */
KMStorageDir KMStorageDir::getUserDir()
{
    return KMStorageDir(KMStorageDir::getUserBaseDirLocation(), true);
}

/**
 * @brief : 获取应用数据目录
 * @param : [in] id，应用id
 * @return: 数据目录
 */
std::string KMStorageDir::getDataDir(const std::string &id)
{
    const char *home = std::getenv("HOME");
    if (nullptr == home || *home == '0')
    {
        return std::string("~/.var/app/") + id;
    }

    return std::string(home) + "/.var/app/" + id;
}

/**
 * @brief : 获取开明包系统级安装的KMStorageDir对象
 * @return: 开明包系统级安装的KMStorageDir对象
 */
KMStorageDir KMStorageDir::getSystemDefaultDir()
{
    return KMStorageDir(KMStorageDir::getSystemDefaultBaseDirLocation(), false);
}

/**
 * @brief : 根据开明包ref信息获取开明包安装的base目录
 * @param : [in] ref, 开明包ref信息
 * @return: 开明包安装的base目录
 * @note  : 目前开明包仅支持系统级安装
 */
std::string KMStorageDir::getRefBaseDirLocation(const KMRef &ref)
{
    return (ref.baseInstallDir.empty() ? KMStorageDir::getSystemDefaultBaseDirLocation() : ref.baseInstallDir) + "/" + ref.channel + "/" + ref.arch + "/" + ref.kind + "/" + ref.id + "/" + ref.module + "/" + ref.version;
}

/**
 * getDeployedDir:
 * @brief : 根据ref信息获取开明包部署路径
 * @param : [in] ref, 开明包基础信息
 * @param : [out] ref, 会更新里面的commit信息（主要是将active>具体的commitid）
 * @return: 开明包部署路径
 */
std::string KMStorageDir::getDeployedDir(const KMRef &ref)
{
    fs::path deployedDir(KMStorageDir::getRefBaseDirLocation(ref));

    if (deployedDir.empty())
    {
        return "";
    }

    if (!fs::is_directory(deployedDir))
    {
        return "";
    }

    if (!fs::exists(deployedDir / "deploy"))
    {
        return "";
    }

    return deployedDir.c_str();
}

/**
 * loadDeployed:
 * @brief : 根据ref信息查询开明包是否部署
 * @param : [in] ref, 开明包基础信息
 * @return: 返回部署信息
 */
std::shared_ptr<KMDeploy> KMStorageDir::loadDeployed(const KMRef &ref)
{
    std::shared_ptr<KMDeploy> deploy;
    std::string deployDir = KMStorageDir::getDeployedDir(ref);
    if (!deployDir.empty())
    {
        std::shared_ptr<KMInfoJson> infoJson = std::make_shared<KMInfoJson>();
        infoJson->loadFile(KMStringUtils::buildFilename(deployDir, "info.json"));

        deploy = std::make_shared<KMDeploy>();
        deploy->m_infoJson = infoJson;
        deploy->m_dir = deployDir;
        deploy->m_ref = ref;
    }

    return deploy;
}

std::string KMStorageDir::getXdgRuntimeDir(const std::string &username)
{
    struct passwd *pw = getpwnam(username.c_str());
    return pw ? "/run/user/" + std::to_string(pw->pw_uid) : "";
}

std::string KMStorageDir::getUserDirLocation()
{
    const char *sudoUser = getenv("SUDO_USER");
    if (sudoUser)
    {
        struct passwd *pw = getpwnam(sudoUser);
        if (pw)
        {
            return std::string(fs::path(pw->pw_dir)); // 使用原始用户的主目录
        }
        else
        {
            return std::string("/home/") + sudoUser;
        }
    }
    else
    {
        const char *home = std::getenv("HOME");
        if (nullptr == home || *home == '0')
        {
            return "/home/" + getLoginUser();
        }
        std::string userHome(home);
        if (userHome == "/root")
        {
            return "/home/" + getLoginUser();
        }
        return std::string(home);
    }
}

void KMStorageDir::modifyDirOwnership(const fs::path &path)
{
    std::string loginUser = KMStorageDir::getLoginUser();
    if (loginUser.empty())
    {
        kmlogger.warn("Login user is empty, skipping ownership modification");
        return;
    }

    std::string primaryGroup = KMStorageDir::getLoginUserPrimaryGroup();
    if (primaryGroup.empty()) {
        kmlogger.warn("Primary group is empty, skipping ownership modification");
        return;
    }

    std::string ownerShip = loginUser + ":" + primaryGroup;
    std::vector<std::string> args = {"-R", ownerShip, path.string()};

    if (!KMProcessUtils::spawn("chown", args))
    {
        kmlogger.error("Failed to chown directory: %s", path.string().c_str());
    }
    else
    {
        kmlogger.info("Successfully changed ownership of %s to %s", path.string().c_str(), ownerShip.c_str());
    }
}