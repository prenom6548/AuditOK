/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMDirUtils.h"

#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <sys/utsname.h>

#include "KMLogger.h"
#include "KMConfig.h"
#include "KMStringUtils.h"
#include "KMProcessUtils.h"

namespace fs = std::filesystem;

/**
 * @brief 获取当前工作目录
 * @param dir 返回当前工作目录路径
 * @return true 成功, false 失败
 */
bool KMDirUtils::getCwdDir(std::string &dir)
{
    std::error_code ec;
    dir = fs::current_path(ec).string();
    if (ec)
    {
        KMError("Failed to get current working directory: " + ec.message());
        dir = "";
        return false;
    }

    return true;
}

/**
 * @brief 获取家目录
 * @param home 家目录 
 * @return true 成功 false 失败 
*/
bool KMDirUtils::getHomeDir(std::string &homeDir)
{
    auto home = getenv("HOME");
    if (home == nullptr)
    {
        if (getuid() == 0)
        {
            homeDir = "/root";
        }
        else
        {
            return false;
        }
    }
    else
    {
        homeDir = std::string(home);
    }

    // 确保目录存在
    if (!fs::exists(homeDir))
    {
        return false;
    }

    return true;
}

/**
 * @brief 获取开明包目录
 * @return 开明包目录
 */
std::string KMDirUtils::getKmDefDir()
{
    const char *dir = getenv("KAIMING_SYSTEM_DIR");
    if (dir == nullptr)
    {
        return KAIMING_SYSTEM_PATH;
    }
    return dir;
}

/**
 * @brief 将后面的std::vector中的元素追加到前面std::vector中
 * @param src追加元素后的std::vector
 * @param dest待追加的std::vector
 */
void KMDirUtils::append(std::vector<KMRef> &src, std::vector<KMRef> &&dest)
{
    for (auto &item : dest)
    {
        src.push_back(item);
    }
}

/**
 * @brief 获取pulse runtime路径 
 * @param dir, 返回pulse runtime路径
 * @return true 成功 false 失败
 */
bool KMDirUtils::getPulseRuntimeDir(std::string &dir)
{
    char *val = getenv("PULSE_RUNTIME_PATH");
    if (val != nullptr)
    {
        dir = std::string(val);
        return true;
    }
    std::string runDir;
    if (!getXdgRuntime(runDir))
    {
        return false;
    }
    dir = runDir.append("/pulse");
    if (!fs::exists(dir))
    {
        return false;
    }

    return true;
}

/**
 * @brief 获取xdg runtime路径
 * @param dir, 返回xdg runtime路径
 * @return true 成功 false 失败
 */
bool KMDirUtils::getXdgRuntime(std::string &dir)
{
    static std::string xdgRunDir;
    if (!xdgRunDir.empty())
    {
        dir = xdgRunDir;
        return true;
    }

    if (getuid() == 0)
    {
        xdgRunDir = "/run/user/0";
        dir = xdgRunDir;
        return true;
    }

    xdgRunDir = realpath(g_get_user_runtime_dir(), nullptr);
    if (!fs::exists(xdgRunDir))
    {
        dir = "";
        return false;
    }

    dir = xdgRunDir;
    return true;
}

/**
 * @brief 根据ref信息获取开明包部署路径
 * @param ref开明包基础信息
 * @return true 成功 false 失败
 */
bool KMDirUtils::getDeployedDir(const KMRef &ref, std::string &dir)
{
    std::string kmdir = ref.baseInstallDir.empty() ? getKmDefDir() : ref.baseInstallDir;
    dir.append(kmdir).append("/").append(ref.channel).append("/").append(ref.arch).append("/").append(ref.kind);
    dir.append("/").append(ref.id).append("/").append(ref.module).append("/").append(ref.version);

    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))
    {
        return false;
    }

    if (!fs::exists(dir + "/deploy"))
    {
        return false;
    }

    return true;
}

/**
 * @brief 获取用户data目录
 * @return 返回用户data目录
 */
std::string KMDirUtils::getUserDataDir()
{
    return g_get_user_data_dir();
}

/**
 * @brief 获取用户缓存目录
 * @return 返回用户缓存目录
 */
std::string KMDirUtils::getUserCacheDir()
{
    return g_get_user_cache_dir();
}

/**
 * @brief 获取时区
 * @return 返回时区信息
 */
std::string KMDirUtils::getTimeZone()
{
    std::string localtime("/etc/localtime");
    std::string canonical = realpath(localtime.c_str(), nullptr);
    if (!canonical.empty())
    {
        /* Strip the prefix and slashes if possible. */
        const char *tzdir = getenv("TZDIR");
        if (tzdir != nullptr && KMStringUtils::startsWith(canonical, tzdir))
        {
            std::string_view tzdirView(tzdir);
            std::string canonicalSuffix = canonical.substr(tzdirView.size());
            while (canonicalSuffix.at(0) == '/')
            {
                canonicalSuffix = canonicalSuffix.substr(1);
            }

            return canonicalSuffix;
        }

        std::string defaultTzpath("/usr/share/zoneinfo");
        if (KMStringUtils::startsWith(canonical, defaultTzpath))
        {
            std::string canonicalSuffix = canonical.substr(defaultTzpath.length());
            while (canonicalSuffix.at(0) == '/')
            {
                canonicalSuffix = canonicalSuffix.substr(1);
            }

            return canonicalSuffix;
        }
    }

    if (fs::exists("/etc/timezone"))
    {
        std::fstream fileStream("/etc/timezone", std::ios::in);
        if (fileStream.is_open())
        {
            std::string ret;
            std::getline(fileStream, ret);
            return ret;
        }
    }

    return std::string("UTC");
}

/**
 * @brief 获取系统架构目录 
 * @return 系统架构目录字符串
 */
std::string KMDirUtils::getLibraryDir()
{
#if defined(__x86_64__)
    return "x86_64-linux-gnu";
#elif defined(__i386__)
    return "i386-linux-gnu";
#elif defined(__arm__) && defined(__ARMEL__)
    return "arm-linux-gnueabi";
#elif defined(__arm__) && defined(__ARMHF__)
    return "arm-linux-gnueabihf";
#elif defined(__aarch64__)
    return "aarch64-linux-gnu";
#elif defined(__powerpc64__) && defined(__LITTLE_ENDIAN__)
    return "powerpc64le-linux-gnu";
#elif defined(__powerpc64__) && !defined(__LITTLE_ENDIAN__)
    return "powerpc64-linux-gnu";
#else
    return getKernelArch() + "-linux-gnu";
#endif
}

/**
 * @brief 获取系统内核架构
 * @return 系统内核架构字符串
 */
std::string KMDirUtils::getKernelArch()
{
    static struct utsname buf;
    static const char *arch = nullptr;

    if (arch != nullptr)
    {
        return arch;
    }

    if (uname(&buf))
    {
        arch = "unknown";
        return arch;
    }

    /* By default, just pass on machine, good enough for most arches */
    arch = buf.machine;
    char *m = buf.machine;
    std::string_view mView(m);
    if (mView.size() == 4 && m[0] == 'i' && m[2] == '8' && m[3] == '6')
    {
        arch = "i386";
    }
    else if (g_str_has_prefix(m, "arm"))
    {
        if (g_str_has_suffix(m, "b"))
        {
            arch = "armeb";
        }
        else
        {
            arch = "arm";
        }
    }
    else if (strncmp(m, "mips", sizeof("mips") - 1) == 0)
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mipsel";
#endif
    }
    else if (strncmp(m, "mips64", sizeof("mips64") - 1) == 0)
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mips64el";
#endif
    }

    return arch;
}

/**
 * @brief 获取登录用的uid
 * @param uid 用户ID
 * @return true 存在 false 不存在
 */
bool KMDirUtils::getLoginUid(uid_t &uid)
{
    uid = getuid();
    if (uid != 0)
    {
        return true;
    }

    const char *command = "loginctl list-sessions --no-legend | awk '/seat0/ {print $2}' | head -n 1";
    const auto ret = KMProcessUtils::execCmd(command);
    if (ret.first.empty())
    {
        KMError("Failed to get login session ID");
        return false;
    }
    else
    {
        uid = std::stoi(ret.first);
        if (uid < 1000)
        {
            return false;
        }
        return true;
    }
}
