/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMReference.h"

#include <filesystem>

#include "common/KMLogger.h"
#include "common/KMStringUtils.h"
#include "common/KMDirUtils.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMPermissionConfig.h"

namespace fs = std::filesystem;

const std::string TOPDIR = "layers";
const std::string DEVEL = "devel";

enum RefIdentify
{
    REF_ID,
    REF_VERSION,
    REF_MODULE,
    REF_CHANNEL,
    REF_MAX
};

class KMReference::Private
{
public:
    std::string id;
    std::string channel;
    std::string version;
    std::string arch;
    std::string module;
    std::string dir;
    std::string baseDir;
    std::string runtimeDir;
    std::vector<std::string> commands;
    std::map<std::string, std::string> envs;
    Permissions perms;
    Annotations annotations;
};

KMReference::KMReference()
{
    d_ptr = std::make_shared<Private>();
}

/**
 * @brief 解析包名
 * @param id 包名
 * @return KMReference
 */
bool KMReference::parse(const std::string &id)
{
    std::vector<std::string> splits = KMStringUtils::splitString(id, "/");
    const size_t size = splits.size();
    d_ptr->id = (size > REF_ID) ? splits[REF_ID] : "";
    d_ptr->version = (size > REF_VERSION) ? splits[REF_VERSION] : "";
    d_ptr->module = (size > REF_MODULE) ? splits[REF_MODULE] : "";
    d_ptr->channel = (size > REF_CHANNEL) ? splits[REF_CHANNEL] : "";
    KMDebug("id: " + d_ptr->id + " version: " + d_ptr->version + " module:" + d_ptr->module + " channel:" + d_ptr->channel);
    return true;
}

/**
  * @brief 获取appId
  * @param appid app名称
  * @return true 成功，false 失败
  */
bool KMReference::getAppId(std::string &appid)
{
    if (d_ptr->id.empty())
        return false;
    appid = d_ptr->id;
    return true;
}

/**
 * @brief 获取架构
 * @param appid 架构名称
 * @return true 成功，false 失败
 */
bool KMReference::getArch(std::string &arch)
{
    if (d_ptr->arch.empty())
        return false;
    arch = d_ptr->arch;
    return true;
}

/**
 * @brief 获取目录
 * @param id 包名
 */
bool KMReference::getLayerDir(const std::string &id, bool isDevel)
{
    if (!parse(id))
    {
        KMError("can't parse id " + id);
        return false;
    }

    if (!getPackageDir(isDevel))
    {
        KMError("can't get package or depend dir " + id);
        return false;
    }

    return true;
}

/**
 * @brief 获取app, base, runtime的目录
 * @return true 成功,false 失败
 */
bool KMReference::getPackageDir(bool isDevel)
{
    KMInstalledAppQuery appQuery;
    appQuery.id(d_ptr->id).channel(d_ptr->channel).version(d_ptr->version).module(d_ptr->module);
    std::vector<KMRef> refs = appQuery.query();
    if (refs.empty())
    {
        KMError("can't find app " + d_ptr->id);
        return false;
    }

    KMRef ref = refs.front();
    if (!KMDirUtils::getDeployedDir(ref, d_ptr->dir))
    {
        KMError("can't get app dir : " + d_ptr->id);
        return false;
    }

    std::string infofile = d_ptr->dir + "/info.json";
    if (!fs::exists(infofile))
    {
        KMError("can't find app info file " + infofile);
        return false;
    }
    // 保存当前架构
    d_ptr->arch = ref.arch;

    // 若获取失败会抛出异常
    KMInfoJson info;
    info.loadFile(infofile);
    d_ptr->perms = info.getPermissions();
    if (!ensurePerms())
    {
        KMError("can't ensure perms " + d_ptr->id);
        return false;
    }
    d_ptr->annotations = info.getAnnotations();
    KMRef baseRef = info.baseRef();

    //获取version
    d_ptr->version = info.version;

    // 获取命令列表
    d_ptr->commands = info.commands;
    // 获取环境变量app列表
    std::map<std::string, std::string> appEnvs = info.envs;

    // 获取base目录
    std::string baseId = baseRef.id;
    if (isDevel)
    {
        baseRef.module = DEVEL;
    }
    baseRef.kind = "base";
    appQuery.ref(baseRef);
    refs.clear();
    if (isDevel)
    {
        refs = appQuery.query(KMInstalledAppQuery::User);
    }
    else
    {
        refs = appQuery.query(KMInstalledAppQuery::System);
    }
    bool useExtra = false;
    if (refs.empty())
    {
        std::string extraDir = d_ptr->dir + "/extra/app-depends";

        if (fs::exists(extraDir))
        {
            d_ptr->baseDir = extraDir;
            useExtra = true;
            KMDebug("base dir not found, fallback to extra dir");
        }
        else
        {
            KMError("can't find base : " + baseId);
            return false;    
        }

    }
    if (!useExtra)
    {
        baseRef = refs.front();
        if (!KMDirUtils::getDeployedDir(baseRef, d_ptr->baseDir))
        {
            KMError("can't get base dir : " + baseRef.id);
            return false;
        }

        std::string baseInfoFile = d_ptr->baseDir + "/info.json";
        if (!fs::exists(baseInfoFile))
        {
            KMError("can't find base info file " + baseInfoFile);
            return false;
        }
        KMInfoJson baseInfo;
        baseInfo.loadFile(baseInfoFile);
        std::map<std::string, std::string> baseEnvs = baseInfo.envs;

        // 获取runtime目录
        KMRef runtimeRef = info.runtimeRef();
        std::map<std::string, std::string> runtimeEnvs;
        if (!runtimeRef.id.empty())
        {
            std::string runtimeId = runtimeRef.id;
            if (isDevel)
            {
                runtimeRef.module = DEVEL;
            }
            runtimeRef.kind = "runtime";
            appQuery.ref(runtimeRef);
            refs.clear();
            if (isDevel)
            {
                refs = appQuery.query(KMInstalledAppQuery::User);
            }
            else
            {
                refs = appQuery.query(KMInstalledAppQuery::System);
            }
            if (refs.empty())
            {
                KMError("can't find runtime : " + runtimeId);
                return false;
            }
            runtimeRef = refs.front();
            if (!KMDirUtils::getDeployedDir(runtimeRef, d_ptr->runtimeDir))
            {
                KMError("can't get runtime dir : " + runtimeRef.id);
                return false;
            }

            std::string runtimeInfoFile = d_ptr->runtimeDir + "/info.json";
            if (!fs::exists(runtimeInfoFile))
            {
                KMError("can't find runtime info file : " + runtimeInfoFile);
                return false;
            }
            KMInfoJson runtimeInfo;
            runtimeInfo.loadFile(runtimeInfoFile);
            runtimeEnvs = runtimeInfo.envs;
        }

        // 按优先级设置环境变量
        appendEnvs(d_ptr->envs, baseEnvs);
        appendEnvs(d_ptr->envs, runtimeEnvs);
    }

    appendEnvs(d_ptr->envs, appEnvs);

    return true;
}

/**
 * @brief 环境变量合并，环境变量优先级：base > runtime > app且处理追加环境变量例如$PATH=/usr:$PATH
 * @param envs 环境变量列表
 * @param newEnvs 追加环境变量列表
 */
void KMReference::appendEnvs(std::map<std::string, std::string> &envs, const std::map<std::string, std::string> &newEnvs)
{
    for (auto const &[k, v] : newEnvs)
    {
        if (v.find("$" + k) == std::string::npos)
        {
            envs[k] = v;
            continue;
        }
        auto it = envs.find(k);
        if (it == envs.end())
        {
            envs[k] = v;
        }
        else
        {
            envs[k] = it->second + ":" + v;
        }
    }
}

/**
 * @brief 获取app目录
 * @param appDir app目录
 * @return app目录
 */
bool KMReference::getAppDir(std::string &appDir)
{
    if (!fs::exists(d_ptr->dir))
    {
        KMError("can't find app dir " + d_ptr->dir);
        return false;
    }

    appDir = d_ptr->dir;
    return true;
}

/**
 * @brief 获取base目录
 * @param baseDir base目录
 * @return base目录
 */
bool KMReference::getBaseDir(std::string &baseDir)
{
    if (!fs::exists(d_ptr->baseDir))
    {
        KMError("can't find base dir " + d_ptr->baseDir);
        return false;
    }

    baseDir = d_ptr->baseDir;
    return true;
}

/**
 * @brief 获取runtime目录
 * @param runtimeDir runtime目录
 * @return runtime目录
 */
bool KMReference::getRuntimeDir(std::string &runtimeDir)
{
    if (!fs::exists(d_ptr->runtimeDir))
    {
        KMDebug("can't find runtime dir " + d_ptr->runtimeDir);
    }

    runtimeDir = d_ptr->runtimeDir;
    return true;
}

/**
 * @brief 获取Commands命令
 * @param commands 命令列表
 * @return true 成功，false 失败
 */
bool KMReference::getCommands(std::vector<std::string> &commands)
{
    commands = d_ptr->commands;
    return true;
}

/**
 * @brief 获取Version
 * @param version 版本号
 * @return true 成功，false 失败
 */
bool KMReference::getVersion(std::string &version) const
{
    version = d_ptr->version;
    return true;
}

/**
 * @brief 获取环境变量列表
 * @param envs 环境变量列表
 * @return true 成功，false 失败
 */
bool KMReference::getEnvs(std::map<std::string, std::string> &envs)
{
    envs = d_ptr->envs;
    return true;
}

/**
 * @brief 获取自定义信息
 * @return 自定义结构
 */
Annotations KMReference::getAnnotations() const
{
    return d_ptr->annotations;
}

/**
 * @brief 获取权限管控信息
 * @return 权限管控
 */
Permissions KMReference::getPermissions() const
{
    return d_ptr->perms;
}

/**
 * @brief 检查权限管控信息
 */
bool KMReference::ensurePerms()
{
    KMPermissionConfig config(d_ptr->id);
    if (!config.isExist())
    {
        return true;
    }

    AppPermissionConfig appPerm = config.getPermissionConfig();
    if (!appPerm.filesystemDirs)
    {
        d_ptr->perms.filesystemDirs.clear();
    }
    d_ptr->perms.xdgUserDirs = appPerm.xdgUserDirs;
    d_ptr->perms.network = appPerm.network;
    d_ptr->perms.scanning = appPerm.scanning;
    d_ptr->perms.bluetooth = appPerm.bluetooth;
    d_ptr->perms.microphone = appPerm.microphone;
    d_ptr->perms.camera = appPerm.camera;
    d_ptr->perms.usb = appPerm.usb;
    d_ptr->perms.serial = appPerm.serial;
    d_ptr->perms.autostart = appPerm.autostart;
    d_ptr->perms.systemNotice = appPerm.systemNotice;
    d_ptr->perms.systemTime = appPerm.systemTime;
    d_ptr->perms.systemServer = appPerm.systemServer;
    d_ptr->perms.windowTop = appPerm.windowTop;
    d_ptr->perms.preventSleep = appPerm.preventsleep;
    d_ptr->perms.systemDisplayResolution = appPerm.systemDisplayResolution;
    d_ptr->perms.systemDisplayBrightness = appPerm.systemDisplayBrightness;
    d_ptr->perms.systemAudioSettings = appPerm.systemAudioSettings;
    d_ptr->perms.systemAudioSize = appPerm.systemAudioSize;
    d_ptr->perms.systemCrontab = appPerm.systemCrontab;
    d_ptr->perms.systemPowerMode = appPerm.systemPowerMode;
    d_ptr->perms.systemPowerPolicy = appPerm.systemPowerPolicy;
    return true;
}
