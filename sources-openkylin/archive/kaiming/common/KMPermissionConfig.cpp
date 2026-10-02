/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMPermissionConfig.h"

#include <filesystem>

#include "KMDirUtils.h"
#include "KMLogger.h"
#include "KMJsonHelper.h"

namespace fs = std::filesystem;

class KMPermissionConfig::Private
{
public:
    std::string m_appId;
};

KMPermissionConfig::KMPermissionConfig(const std::string &id)
    : d_ptr(std::make_shared<Private>())
{
    d_ptr->m_appId = id;
}

KMPermissionConfig::~KMPermissionConfig()
{
}

bool KMPermissionConfig::loadFile(const std::string &metafile)
{
    if (!fs::exists(metafile))
    {
        KMInfo(metafile + " not exists");
        return false;
    }

    return true;
}

/**
 * @brief　加载配置文件
 */
bool KMPermissionConfig::loadConfiguration()
{
    std::string homeDir;
    if (!KMDirUtils::getHomeDir(homeDir))
    {
        return false;
    }

    std::string jsonFile = homeDir.append("/.config/kaiming-permission.json");
    if (!fs::exists(jsonFile))
    {
        return false;
    }

    m_root = KMJsonHelper::loadFile(jsonFile);
    if (!loadFile(jsonFile))
    {
        KMError("Failed to load the permission configuration file.");
        return false;
    }

    return true;
}


/**
 * @brief 解析配置文件
 */
AppPermissionConfig KMPermissionConfig::getPermissionConfig()
{
    if (!loadConfiguration())
    {
        return {};
    }

    AppPermissionConfig permissions;
    for (const auto& [appId, appPermissions] : m_root.items()) 
    {
        if (appId == d_ptr->m_appId)
        {
            KMJsonHelper perm(appPermissions);
            permissions.xdgUserDirs = perm.getBoolValue("xdg-user-dirs",false);
            permissions.filesystemDirs = perm.getBoolValue("filesystem-dirs",false);
            permissions.network = perm.getBoolValue("network",false);
            permissions.scanning = perm.getBoolValue("scanning",false);
            permissions.bluetooth = perm.getBoolValue("bluetooth",false);
            permissions.microphone = perm.getBoolValue("microphone",false);
            permissions.camera = perm.getBoolValue("camera",false);
            permissions.usb = perm.getBoolValue("usb",false);
            permissions.serial = perm.getBoolValue("serial",false);
            permissions.autostart = perm.getBoolValue("autostart",false);
            permissions.systemNotice = perm.getBoolValue("system-notice",false);
            permissions.systemTime = perm.getBoolValue("system-time",false);
            permissions.systemServer = perm.getBoolValue("system-server",false);
            permissions.windowTop = perm.getBoolValue("window-top",false);
            permissions.preventsleep = perm.getBoolValue("prevent-sleep",false);
            permissions.systemDisplayResolution = perm.getBoolValue("system-display-resolution",false);
            permissions.systemDisplayBrightness = perm.getBoolValue("system-display-brightness",false);
            permissions.systemAudioSettings = perm.getBoolValue("system-audio-settings",false);
            permissions.systemAudioSize = perm.getBoolValue("system-audio-size",false);
            permissions.systemCrontab = perm.getBoolValue("system-crontab",false);
            permissions.systemPowerMode = perm.getBoolValue("system-power-mode",false);
            permissions.systemPowerPolicy = perm.getBoolValue("system-power-policy",false);
        }

    }
    
    return permissions;
}

/**
 * @brief 是否存在管控配置
 * @return true 成功，false 失败
 */
bool KMPermissionConfig::isExist()
{
   if (!loadConfiguration())
    {
        return false;
    }

    AppPermissionConfig config;
   
    if (m_root[d_ptr->m_appId].is_null())
    {
        return false;
    }

    return true;
}


