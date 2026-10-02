/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInfoJson.h"
#include <fstream>
#include <filesystem>
#include <regex>
#include <iterator>
#include "KMException.h"
#include "../KMBuilderUtils.h"

namespace fs = std::filesystem;

void KMInfoJson::loadFile(const std::string& jsonFile)
{
    m_root = KMNlohmannJsonHelper::loadFile(jsonFile);
    
    fromJson();
}

void KMInfoJson::saveFile(const std::string& jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code e;
        if (!KMBuilderUtils::mkpath(baseDir.string(), e))
        {
            throw KMException(e.message());
        }
    }

    toJson();

    std::ofstream o(jsonFile);
    o << std::setw(4) << m_root << std::endl;
}

void KMInfoJson::saveModulesFile(const std::string& jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code e;
        if (!KMBuilderUtils::mkpath(baseDir.string(), e))
        {
            throw KMException(e.message());
        }
    }
    
    nlohmann::json  modules_root;
    KMNlohmannJsonHelper helper(modules_root);
    helper.setStringMapValue("modules",modules);
    
    std::ofstream o(jsonFile);
    o << std::setw(4) << modules_root << std::endl;
}

void KMInfoJson::fromJson()
{
    KMNlohmannJsonHelper helper(m_root);
    schema_version = helper.getStringValue("schema_version", "1.0");
    id = helper.getStringValue("id");
    archs = helper.getStringListValue("arch");
    base = helper.getStringValue("base");
    runtime = helper.getStringValue("runtime");
    channel = helper.getStringValue("channel");
    commands = helper.getStringListValue("command");
    name = helper.getStringValue("name");
    description = helper.getStringValue("description");
    kind = helper.getStringValue("kind");
    module = helper.getStringValue("module");
    size = helper.getUInt64Value("size");
    version = helper.getStringValue("version");
    envs = helper.getStringMapValue("env");
    parsePermissionFromJson();
    license = helper.getStringValue("license");
    homepage = helper.getStringValue("homepage");
    maintainers = helper.getStringListValue("maintainer");
    export_plugins = helper.getStringListValue("export_plugins");

    // name[zh_CN]
    names = helper.fromPrefix("name[");
    // description[zh_CN]
    descriptions = helper.fromPrefix("description[");

    // 解析annotations
    if (m_root.contains("annotations")) 
    {
        nlohmann::json& annotationsNode = m_root["annotations"];
        KMNlohmannJsonHelper annotationsHelper(annotationsNode);
        annotations.kylin_privilege = annotationsHelper.getBoolValue("kylin.privilege");
        annotations.kylin_sysapp = annotationsHelper.getBoolValue("kylin.sysapp");
        annotations.kylin_gpu = annotationsHelper.getBoolValue("kylin.gpu");
        annotations.kylin_polkit = annotationsHelper.getBoolValue("kylin.polkit");
        annotations.kylin_depend_apps = annotationsHelper.getStringListValue("kylin.depend-apps");
        annotations.kylin_desktops = annotationsHelper.getStringListValue("kylin.desktop");
        annotations.kylin_dbus_whitelist = annotationsHelper.getStringListValue("kylin.dbus-whitelist");

        annotations.kylin_uninstallkills = annotationsHelper.getStringListValue("kylin.uninstallkill");
        annotations.kylin_stopservices = annotationsHelper.getStringListValue("kylin.stopservice");
        annotations.kylin_rmdir = annotationsHelper.getStringListValue("kylin.rmdir");
        annotations.kylin_unlink = annotationsHelper.getStringListValue("kylin.unlink");

        if (annotationsNode.contains("kylin.install"))
        {
            KMNlohmannJsonHelper installHelper(annotationsNode["kylin.install"]);
            annotations.kylin_install.start_service = installHelper.getStringListValue("start_service");
            annotations.kylin_install.stop_service = installHelper.getStringListValue("stop_service");
            annotations.kylin_install.add_group = installHelper.getStringListValue("add_group");
            annotations.kylin_install.chmod = installHelper.getStringListValue("chmod");
            annotations.kylin_install.autostart = installHelper.getStringListValue("autostart");
            annotations.kylin_install.remove = installHelper.getStringListValue("remove");
            annotations.kylin_install.stop_process = installHelper.getStringListValue("stop_process");
        }

        if (annotationsNode.contains("kylin.uninstall"))
        {
            KMNlohmannJsonHelper uninstallHelper(annotationsNode["kylin.uninstall"]);
            annotations.kylin_uninstall.stop_service = uninstallHelper.getStringListValue("stop_service");
            annotations.kylin_uninstall.remove = uninstallHelper.getStringListValue("remove");
            annotations.kylin_uninstall.stop_process = uninstallHelper.getStringListValue("stop_process");
        }

        // 原生部署属性
        annotations.kylin_hostapp = annotationsHelper.getBoolValue("kylin.hostapp");
    }
}

void KMInfoJson::toJson()
{
    m_root["schema_version"] = schema_version;
    m_root["id"] = id;
    m_root["base"] = base;
    m_root["runtime"] = runtime;
    
    KMNlohmannJsonHelper helper(m_root);
    helper.setStringValue("channel", channel);
    helper.setStringValue("name", name);
    helper.setStringValue("description", description);
    helper.setStringValue("kind", kind);
    determineModule();
    helper.setStringValue("module", module);
    helper.setUInt64Value("size", size);
    helper.setStringValue("version", version);
    helper.setStringValue("license", license);
    helper.setStringValue("homepage", homepage);

    helper.setStringListValue("arch", archs);
    helper.setStringListValue("command", commands);
    helper.add(names);
    helper.add(descriptions);
    helper.setStringMapValue("env", envs);
    helper.setStringListValue("maintainer", maintainers);
    helper.setStringListValue("export_plugins", export_plugins);

    permissionToJson();

    annotationsToJson();
}

void KMInfoJson::determineModule()
{
    if (kind == "depend")
    {
        module = "devel";
        return;
    }

    bool hasDevel = modules.find("devel") != modules.end();
    bool hasBinary = modules.find("binary") != modules.end();
    if (hasBinary)
    {
        module = "binary";
    }
    else if (hasDevel)
    {
        module = "devel";
    }
    else
    {
        throw KMException("Neither 'binary' nor 'devel' found in modules.");
    }
}

void KMInfoJson::parsePermissionFromJson()
{
    if (!m_root.contains("permission") || m_root["permission"].is_array())
    {
        permission.m_none = true;
        return;
    }

    nlohmann::json& permissionNode = m_root["permission"];

    // 解析 Filesystem
    if (permissionNode.contains("filesystem")) 
    {
        KMNlohmannJsonHelper fsHelper(permissionNode["filesystem"]);
        permission.m_filesystem.m_xdgUserDirs = fsHelper.getBoolValue("xdg-user-dirs");
        permission.m_filesystem.m_filesystemDirs = fsHelper.getStringListValue("filesystem-dirs");
    }

    // 解析 Net
    if (permissionNode.contains("net")) 
    {
        KMNlohmannJsonHelper netHelper(permissionNode["net"]);
        permission.m_net.m_network = netHelper.getBoolValue("network");
        permission.m_net.m_scanning = netHelper.getBoolValue("scanning");
    }

    // 解析 Device
    if (permissionNode.contains("device")) 
    {
        KMNlohmannJsonHelper deviceHelper(permissionNode["device"]);
        permission.m_device.m_bluetooth = deviceHelper.getBoolValue("bluetooth");
        permission.m_device.m_microphone = deviceHelper.getBoolValue("microphone");
        permission.m_device.m_camera = deviceHelper.getBoolValue("camera");
        permission.m_device.m_usb = deviceHelper.getBoolValue("usb");
        permission.m_device.m_serial = deviceHelper.getBoolValue("serial");
    }

    // 解析 ProcessManager
    if (permissionNode.contains("processManager")) 
    {
        KMNlohmannJsonHelper pmHelper(permissionNode["processManager"]);
        permission.m_processManager.m_autostart = pmHelper.getBoolValue("autostart");
    }

    // 解析 Others
    if (permissionNode.contains("others")) 
    {
        KMNlohmannJsonHelper othersHelper(permissionNode["others"]);
        permission.m_others.m_systemNotice = othersHelper.getBoolValue("system-notice");
        permission.m_others.m_systemTime = othersHelper.getBoolValue("system-time");
        permission.m_others.m_systemServer = othersHelper.getBoolValue("system-server");
        permission.m_others.m_windowTop = othersHelper.getBoolValue("window-top");
        permission.m_others.m_preventSleep = othersHelper.getBoolValue("prevent-sleep");
        permission.m_others.m_systemDisplayResolution = othersHelper.getBoolValue("system-display-resolution");
        permission.m_others.m_systemDisplayBrightness = othersHelper.getBoolValue("system-display-brightness");
        permission.m_others.m_systemAudioSetting = othersHelper.getBoolValue("system-audio-settings");
        permission.m_others.m_systemAudioSize = othersHelper.getBoolValue("system-audio-size");
        permission.m_others.m_systemCrontab = othersHelper.getBoolValue("system-crontab");
        permission.m_others.m_systemPowerMode = othersHelper.getBoolValue("system-power-mode");
        permission.m_others.m_systemPowerPolicy = othersHelper.getBoolValue("system-power-policy");

    }
}

void KMInfoJson::permissionToJson()
{
    if (permission.m_none)
    {
        return;
    }

    nlohmann::json& permissionNode = m_root["permission"];

    // 写入 Filesystem
    if (permission.m_filesystem.m_xdgUserDirs || permission.m_filesystem.m_filesystemDirs.size() > 0)
    {
        KMNlohmannJsonHelper fsHelper(permissionNode["filesystem"]);
        fsHelper.setBoolValue("xdg-user-dirs", permission.m_filesystem.m_xdgUserDirs, permission.m_filesystem.m_xdgUserDirs);
        fsHelper.setStringListValue("filesystem-dirs", permission.m_filesystem.m_filesystemDirs);
    }

    // 写入 Net
    if (permission.m_net.m_network || permission.m_net.m_scanning)
    {
        KMNlohmannJsonHelper netHelper(permissionNode["net"]);
        netHelper.setBoolValue("network", permission.m_net.m_network, permission.m_net.m_network);
        netHelper.setBoolValue("scanning", permission.m_net.m_scanning, permission.m_net.m_scanning);
    }

    // 写入 Device
    if (permission.m_device.m_bluetooth || permission.m_device.m_microphone || permission.m_device.m_camera || permission.m_device.m_usb || permission.m_device.m_serial)
    {
        KMNlohmannJsonHelper deviceHelper(permissionNode["device"]);
        deviceHelper.setBoolValue("bluetooth", permission.m_device.m_bluetooth, permission.m_device.m_bluetooth);
        deviceHelper.setBoolValue("microphone", permission.m_device.m_microphone, permission.m_device.m_microphone);
        deviceHelper.setBoolValue("camera", permission.m_device.m_camera, permission.m_device.m_camera);
        deviceHelper.setBoolValue("usb", permission.m_device.m_usb, permission.m_device.m_usb);
        deviceHelper.setBoolValue("serial", permission.m_device.m_serial, permission.m_device.m_serial);
    }

    // 写入 ProcessManager
    if (permission.m_processManager.m_autostart)
    {
        KMNlohmannJsonHelper pmHelper(permissionNode["processManager"]);
        pmHelper.setBoolValue("autostart", permission.m_processManager.m_autostart, permission.m_processManager.m_autostart);
    }

    // 写入 Others
    if (permission.m_others.m_systemNotice || permission.m_others.m_systemTime || permission.m_others.m_systemServer || 
        permission.m_others.m_windowTop || permission.m_others.m_preventSleep || permission.m_others.m_systemDisplayResolution ||
        permission.m_others.m_systemDisplayBrightness || permission.m_others.m_systemAudioSetting || permission.m_others.m_systemAudioSize || permission.m_others.m_systemCrontab ||
        permission.m_others.m_systemPowerMode || permission.m_others.m_systemPowerPolicy )
    {
        KMNlohmannJsonHelper othersHelper(permissionNode["others"]);
        othersHelper.setBoolValue("system-notice", permission.m_others.m_systemNotice, permission.m_others.m_systemNotice);
        othersHelper.setBoolValue("system-time", permission.m_others.m_systemTime, permission.m_others.m_systemTime);
        othersHelper.setBoolValue("system-server", permission.m_others.m_systemServer, permission.m_others.m_systemServer);
        othersHelper.setBoolValue("window-top", permission.m_others.m_windowTop, permission.m_others.m_windowTop);
        othersHelper.setBoolValue("prevent-sleep", permission.m_others.m_preventSleep, permission.m_others.m_preventSleep);
        othersHelper.setBoolValue("system-display-resolution", permission.m_others.m_systemDisplayResolution, permission.m_others.m_systemDisplayResolution);
        othersHelper.setBoolValue("system-display-brightness", permission.m_others.m_systemDisplayBrightness, permission.m_others.m_systemDisplayBrightness);
        othersHelper.setBoolValue("system-audio-settings", permission.m_others.m_systemAudioSetting, permission.m_others.m_systemAudioSetting);
        othersHelper.setBoolValue("system-audio-size", permission.m_others.m_systemAudioSize, permission.m_others.m_systemAudioSize);
        othersHelper.setBoolValue("system-crontab", permission.m_others.m_systemCrontab, permission.m_others.m_systemCrontab);
        othersHelper.setBoolValue("system-power-mode", permission.m_others.m_systemPowerMode, permission.m_others.m_systemPowerMode);
        othersHelper.setBoolValue("system-power-policy", permission.m_others.m_systemPowerPolicy, permission.m_others.m_systemPowerPolicy);
    }
}

void KMInfoJson::annotationsToJson()
{
    if (annotations.kylin_privilege || 
        annotations.kylin_sysapp || 
        annotations.kylin_gpu || 
        annotations.kylin_polkit || 
        annotations.kylin_hostapp ||
        !annotations.kylin_depend_apps.empty() || 
        !annotations.kylin_desktops.empty() || 
        !annotations.kylin_uninstallkills.empty() || 
        !annotations.kylin_stopservices.empty() || 
        !annotations.kylin_rmdir.empty() ||
        !annotations.kylin_unlink.empty() ||
        !annotations.kylin_dbus_whitelist.empty() ||
        !annotations.kylin_install.start_service.empty() ||
        !annotations.kylin_install.stop_service.empty() ||
        !annotations.kylin_install.add_group.empty() ||
        !annotations.kylin_install.chmod.empty() ||
        !annotations.kylin_install.autostart.empty() ||
        !annotations.kylin_install.remove.empty() ||
        !annotations.kylin_install.stop_process.empty() ||
        !annotations.kylin_uninstall.stop_service.empty() ||
        !annotations.kylin_uninstall.remove.empty() ||
        !annotations.kylin_uninstall.stop_process.empty()
    )
    {
        nlohmann::json& annotationsNode = m_root["annotations"];
        KMNlohmannJsonHelper annotationsHelper(annotationsNode);
        annotationsHelper.setBoolValue("kylin.privilege", annotations.kylin_privilege, annotations.kylin_privilege);
        annotationsHelper.setBoolValue("kylin.sysapp", annotations.kylin_sysapp, annotations.kylin_sysapp);
        annotationsHelper.setBoolValue("kylin.gpu", annotations.kylin_gpu, annotations.kylin_gpu);
        annotationsHelper.setBoolValue("kylin.polkit", annotations.kylin_polkit, annotations.kylin_polkit);
        annotationsHelper.setStringListValue("kylin.depend-apps", annotations.kylin_depend_apps);
        annotationsHelper.setStringListValue("kylin.desktop", annotations.kylin_desktops);

        annotationsHelper.setStringListValue("kylin.uninstallkill", annotations.kylin_uninstallkills);
        annotationsHelper.setStringListValue("kylin.stopservice", annotations.kylin_stopservices);
        annotationsHelper.setStringListValue("kylin.rmdir", annotations.kylin_rmdir);
        annotationsHelper.setStringListValue("kylin.unlink", annotations.kylin_unlink);
        annotationsHelper.setStringListValue("kylin.dbus-whitelist", annotations.kylin_dbus_whitelist);

        // 原生部署属性
        annotationsHelper.setBoolValue("kylin.hostapp", annotations.kylin_hostapp, annotations.kylin_hostapp);

        if (!annotations.kylin_install.start_service.empty() ||
            !annotations.kylin_install.stop_service.empty() ||
            !annotations.kylin_install.add_group.empty() ||
            !annotations.kylin_install.chmod.empty() ||
            !annotations.kylin_install.autostart.empty() ||
            !annotations.kylin_install.remove.empty() ||
            !annotations.kylin_install.stop_process.empty()
        )
        {
            nlohmann::json& installNode = annotationsNode["kylin.install"];
            KMNlohmannJsonHelper installHelper(installNode);
            installHelper.setStringListValue("start_service", annotations.kylin_install.start_service);
            installHelper.setStringListValue("stop_service", annotations.kylin_install.stop_service);
            installHelper.setStringListValue("add_group", annotations.kylin_install.add_group);
            installHelper.setStringListValue("chmod", annotations.kylin_install.chmod);
            installHelper.setStringListValue("autostart", annotations.kylin_install.autostart);
            installHelper.setStringListValue("remove", annotations.kylin_install.remove);
            installHelper.setStringListValue("stop_process", annotations.kylin_install.stop_process);
        }

        if (!annotations.kylin_uninstall.stop_service.empty() ||
            !annotations.kylin_uninstall.remove.empty() ||
            !annotations.kylin_uninstall.stop_process.empty()
        )
        {
            nlohmann::json& uninstallNode = annotationsNode["kylin.uninstall"];
            KMNlohmannJsonHelper uninstallHelper(uninstallNode);
            uninstallHelper.setStringListValue("stop_service", annotations.kylin_uninstall.stop_service);
            uninstallHelper.setStringListValue("remove", annotations.kylin_uninstall.remove);
            uninstallHelper.setStringListValue("stop_process", annotations.kylin_uninstall.stop_process);
        }
    }
    else
    {
        // 是否要清空annotations节点？
    }
}

