/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMEnv.h"

#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMStorageDir.h"

class KMEnv::Private
{
public:
    std::string m_systemBaseDir;
    std::string m_systemExportsDir;
    std::string m_xdgDataDirs;
};

KMEnv::KMEnv()
    : d(make_shared<Private>())
{
}

KMEnv::~KMEnv() { }


/**
 * @brief 判断当前环境是否以 sudo 或 root 权限运行
 *
 * @return true sudo 或者 root权限
 * @return false 非root 和 sudo 权限
 */
bool KMEnv::isSudoOrRoot()
{
    uid_t euid = geteuid();
    if (euid == 0)
    {
        return true;
    }
    const char *user = std::getenv("USER");
    if (user != nullptr && std::string(user) == "root")
    {
        return true;
    }

    std::cout << _("Insufficient execution permissions, please add 'sudo' and execute again!") << std::endl;

    return false;
}

/**
 * @brief 校验 XDG_DATA_DIRS 合法性
 *
 * @return std::string
 */
std::string KMEnv::getXdgDataDirs()
{
    const char *xdgDataDirs = std::getenv("XDG_DATA_DIRS");
    if (nullptr == xdgDataDirs || *xdgDataDirs == '0')
    {
        return std::string("/usr/share/ukui-wayland:/usr/local/share:/usr/share");
    }

    return std::string(xdgDataDirs);
}

/**
 * @brief 通过环境变量获取自定义远程仓库地址
 *
 * 配置自定义软件源仓库(http://10.41.118.234/repo)实例：
 *      1. 修改 /opt/kaiming/repo/config 的 url=http://10.41.118.234/repo
 *      2. sudo KAIMING_REPO_URL=http://10.41.118.234/repo install app/top.openkylin.Clock/x86_64/stable
 *
 * @return std::string
 */
std::string KMEnv::getRepoUrl()
{
    const char *xdgDataDirs = std::getenv("XDG_DATA_DIRS");
    if (nullptr == xdgDataDirs || *xdgDataDirs == '0')
    {
        return std::string("/usr/share/ukui-wayland:/usr/local/share:/usr/share");
    }

    return std::string(xdgDataDirs);
}

/**
 * @brief 是否通过环境校验
 *
 * @return true 通过环境校验
 * @return false 未通过环境校验
 */
bool KMEnv::passEnvCheck()
{
    if (!isSudoOrRoot())
    {
        return false;
    }

    return true;
}
