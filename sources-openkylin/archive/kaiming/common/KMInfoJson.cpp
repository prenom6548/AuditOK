/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInfoJson.h"
#include <fstream>
#include <filesystem>
#include <regex>
#include <iterator>
#include "KMException.h"
#include "KMStringUtils.h"

namespace fs = std::filesystem;

void KMInfoJson::InfoBase::baseParse()
{
    std::vector<std::string> channel_base = KMStringUtils::splitString(m_baseString, ":");
    std::vector<std::string> id_version = KMStringUtils::splitString(channel_base.at(1), "/");
    m_baseId = id_version.at(0);
    m_baseVersion = id_version.at(1);
}

void KMInfoJson::InfoBase::versionParse()
{
    std::vector<std::string> versionNums = KMStringUtils::splitString(m_baseVersion, ".");
    m_versionMajor = versionNums.at(0);
    if (versionNums.size() >= 2)
    {
        m_versionMinor = versionNums.at(1);
    }
}

bool KMInfoJson::InfoBase::checkVersion(const InfoBase &ver)
{
    if (m_versionMajor != ver.m_versionMajor)
    {
        return false;
    }

    return true;
}

void KMInfoJson::loadFile(const std::string &jsonFile)
{
    fs::path filePath(jsonFile);
    if (!fs::exists(filePath))
    {
        throw KMException("File does not exist: " + jsonFile);
    }

    try
    {
        m_root = KMJsonHelper::loadFile(jsonFile);
        fromJson();
    }
    catch (const KMException &e)
    {
        throw;
    }
    catch (const std::exception &e)
    {
        throw KMException("JSON processing error: " + std::string(e.what()));
    }
}

void KMInfoJson::saveFile(const std::string &jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code ec;
        bool result = fs::create_directories(baseDir.string(), ec);
        if (!result && 0 != ec.value())
        {
            throw KMException(ec.message());
        }
    }

    toJson();

    std::ofstream o(jsonFile);
    o << std::setw(4) << m_root << std::endl;
}

void KMInfoJson::setRoot(nlohmann::json &root)
{
    m_root = root;

    fromJson();
}

nlohmann::json KMInfoJson::root()
{
    toJson();

    return m_root;
}

void KMInfoJson::parsePermissions()
{
    if (!m_root.contains("permission"))
    {
        return;
    }

    if (m_root["permission"].is_array())
    {
        permissions.all = true;
        return;
    }

    KMJsonHelper helper(m_root);
    nlohmann::json j = helper.getObjectValue("permission");
    if (!j.is_object())
    {
        return;
    }

    // 解析 "filesystem" 部分
    if (j.contains("filesystem") && j["filesystem"].is_object())
    {
        KMJsonHelper permFs(j["filesystem"]);
        permissions.xdgUserDirs = permFs.getBoolValue("xdg-user-dirs", false);
        permissions.filesystemDirs = permFs.getStringListValue("filesystem-dirs");
    }

    // 解析 "net" 部分
    if (j.contains("net") && j["net"].is_object())
    {
        KMJsonHelper permNet(j["net"]);
        permissions.network = permNet.getBoolValue("network", false);
        permissions.scanning = permNet.getBoolValue("scanning", false);
    }

    // 解析 "device" 部分
    if (j.contains("device") && j["device"].is_object())
    {
        KMJsonHelper permDevice(j["device"]);
        permissions.bluetooth = permDevice.getBoolValue("bluetooth", false);
        permissions.microphone = permDevice.getBoolValue("microphone", false);
        permissions.camera = permDevice.getBoolValue("camera", false);
        permissions.usb = permDevice.getBoolValue("usb", false);
        permissions.serial = permDevice.getBoolValue("serial", false);
    }

    // 解析 "processManager" 部分
    if (j.contains("processManager") && j["processManager"].is_object())
    {
        KMJsonHelper permProcMan(j["processManager"]);
        permissions.autostart = permProcMan.getBoolValue("autostart", false);
    }

    // 解析 "others" 部分
    if (j.contains("others") && j["others"].is_object())
    {
        KMJsonHelper permOthers(j["others"]);
        permissions.systemNotice = permOthers.getBoolValue("system-notice", false);
        permissions.systemTime = permOthers.getBoolValue("system-time", false);
        permissions.systemServer = permOthers.getBoolValue("system-server", false);
        permissions.windowTop = permOthers.getBoolValue("window-top", false);
        permissions.preventSleep = permOthers.getBoolValue("prevent-sleep", false);
        permissions.systemDisplayResolution = permOthers.getBoolValue("system-display-resolution", false);
        permissions.systemDisplayBrightness = permOthers.getBoolValue("system-display-brightness", false);
        permissions.systemAudioSettings = permOthers.getBoolValue("system-audio-settings", false);
        permissions.systemAudioSize = permOthers.getBoolValue("system-audio-size", false);
        permissions.systemCrontab = permOthers.getBoolValue("system-crontab", false);
        permissions.systemPowerMode = permOthers.getBoolValue("system-power-mode", false);
        permissions.systemPowerPolicy = permOthers.getBoolValue("system-power-policy", false);
    }
}

const Permissions &KMInfoJson::getPermissions() const
{
    return permissions;
}

/**
 * @brief : 解析JSON中的annotations字段
 * @param : 包含自定义信息
 * @return : void
 */
void KMInfoJson::parseAnnotations()
{
    // 解析annotations
    if (m_root.contains("annotations") && m_root["annotations"].is_object())
    {
        KMJsonHelper annotions(m_root["annotations"]);

        annotations.privilege = annotions.getBoolValue("kylin.privilege", false);
        annotations.hostPlugins = annotions.getBoolValue("kylin.hostPlugins", false);
        annotations.sysapp = annotions.getBoolValue("kylin.sysapp", false);
        annotations.gpu = annotions.getBoolValue("kylin.gpu", false);
        annotations.covers = annotions.getObjectKeyValue("kylin.cover");
        annotations.polkit = annotions.getBoolValue("kylin.polkit", false);
        annotations.tray = annotions.getBoolValue("kylin.tray", false);
        annotations.deltas = annotions.getBoolValue("kylin.deltas", false);
        annotations.depends = annotions.getStringListValue("kylin.depends");
        annotations.dependApps = annotions.getStringListValue("kylin.depend-apps");
        annotations.desktops = annotions.getStringListValue("kylin.desktop");
        annotations.kylin_dbus_whitelist = annotions.getStringListValue("kylin.dbus-whitelist");
        annotations.preVersion = annotions.getStringListValue("kylin.preVersion");
        // -------原生安装属性-------------
        annotations.hostapp = annotions.getBoolValue("kylin.hostapp", false);
        annotations.hostscript = annotions.getBoolValue("kylin.script", false);
        annotations.service = annotions.getStringListValue("service");
        annotations.sysgroup = annotions.getStringListValue("kylin.addsystemgroup");
        annotations.autostart = annotions.getStringListValue("kylin.autostart");
        annotations.chmods = annotions.getStringListValue("kylin.chmod");
        annotations.installkills = annotions.getStringListValue("kylin.installkill");
        annotations.uninstallkills = annotions.getStringListValue("kylin.uninstallkill");
        annotations.stopservices = annotions.getStringListValue("kylin.stopservice");
        annotations.rmdir = annotions.getStringListValue("kylin.rmdir");
        annotations.unlink = annotions.getStringListValue("kylin.unlink");

        auto annotationsJson = m_root["annotations"];
        if (annotationsJson.contains("kylin.install") && annotationsJson["kylin.install"].is_object())
        {
            KMJsonHelper installHelper(annotationsJson["kylin.install"]);
            annotations.install_actions.start_service = installHelper.getStringListValue("start_service");
            annotations.install_actions.stop_service = installHelper.getStringListValue("stop_service");
            annotations.install_actions.stop_process = installHelper.getStringListValue("stop_process");
            annotations.install_actions.chmod = installHelper.getStringListValue("chmod");
            annotations.install_actions.add_group = installHelper.getStringListValue("add_group");
            annotations.install_actions.autostart = installHelper.getStringListValue("autostart");
            annotations.install_actions.remove = installHelper.getStringListValue("remove");
        }

        if (annotationsJson.contains("kylin.uninstall") && annotationsJson["kylin.uninstall"].is_object())
        {
            KMJsonHelper uninstallHelper(annotationsJson["kylin.uninstall"]);
            annotations.uninstall_actions.stop_service = uninstallHelper.getStringListValue("stop_service");
            annotations.uninstall_actions.stop_process = uninstallHelper.getStringListValue("stop_process");
            annotations.uninstall_actions.remove = uninstallHelper.getStringListValue("remove");
        }

        auto debinfoMap = annotions.getStringMapValue("kylin.debinfo");
        if (!debinfoMap.empty())
        {
            annotations.debinfo.package = debinfoMap["Package"];
            annotations.debinfo.version = debinfoMap["Version"];
            annotations.debinfo.section = debinfoMap["Section"];
            annotations.debinfo.package_type = debinfoMap["Package-Type"];
            annotations.debinfo.recommends = debinfoMap["Recommends"];
            annotations.debinfo.uploaders = debinfoMap["Uploaders"];
            annotations.debinfo.provides = debinfoMap["Provides"];
            annotations.debinfo.origin = debinfoMap["Origin"];
            annotations.debinfo.multi_arch = debinfoMap["Multi-Arch"];
            annotations.debinfo.build_depends = debinfoMap["Build-Depends"];
            annotations.debinfo.source = debinfoMap["Source"];
            annotations.debinfo.suggests = debinfoMap["Suggests"];
            annotations.debinfo.conflicts = debinfoMap["Conflicts"];
            annotations.debinfo.breaks = debinfoMap["Breaks"];
            annotations.debinfo.replaces = debinfoMap["Replaces"];
            annotations.debinfo.vcs_git = debinfoMap["Vcs-Git"];
            annotations.debinfo.vcs_browser = debinfoMap["Vcs-Browser"];
        }
    }
}

/**
 * @brief 获取已解析的权限信息
 * @return const Permissions& 返回对已解析权限结构体的常量引用
 */
const Annotations &KMInfoJson::getAnnotations() const
{
    return annotations;
}

void KMInfoJson::fromJson()
{
    KMJsonHelper helper(m_root);
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
    license = helper.getStringValue("license");
    homepage = helper.getStringValue("homepage");
    maintainers = helper.getStringListValue("maintainer");
    export_plugins = helper.getStringListValue("export_plugins");

    // name[zh_CN]
    names = helper.fromPrefix("name[");
    // description[zh_CN]
    descriptions = helper.fromPrefix("description[");

    // 解析权限字段
    parsePermissions();
    // 解析自定义字段
    parseAnnotations();
}

void KMInfoJson::permissionToJson()
{
    if (permissions.all)
    {
        std::vector<std::string> permission;
        permission.push_back("all");
        KMJsonHelper helper(m_root);
        helper.setStringListValue("permission", permission);
        return;
    }

    // 写入 Filesystem
    if (permissions.xdgUserDirs || permissions.filesystemDirs.size() > 0)
    {
        KMJsonHelper fsHelper(m_root["permission"]["filesystem"]);
        fsHelper.setBoolValue("xdg-user-dirs", permissions.xdgUserDirs, permissions.xdgUserDirs);
        fsHelper.setStringListValue("filesystem-dirs", permissions.filesystemDirs);
    }

    // 写入 Net
    if (permissions.network || permissions.scanning)
    {
        KMJsonHelper netHelper(m_root["permission"]["net"]);
        netHelper.setBoolValue("network", permissions.network, permissions.network);
        netHelper.setBoolValue("scanning", permissions.scanning, permissions.scanning);
    }

    // 写入 Device
    if (permissions.bluetooth || permissions.microphone || permissions.camera || permissions.usb || permissions.serial)
    {
        KMJsonHelper deviceHelper(m_root["permission"]["device"]);
        deviceHelper.setBoolValue("bluetooth", permissions.bluetooth, permissions.bluetooth);
        deviceHelper.setBoolValue("microphone", permissions.microphone, permissions.microphone);
        deviceHelper.setBoolValue("camera", permissions.camera, permissions.camera);
        deviceHelper.setBoolValue("usb", permissions.usb, permissions.usb);
        deviceHelper.setBoolValue("serial", permissions.serial, permissions.serial);
    }

    // 写入 ProcessManager
    if (permissions.autostart)
    {
        KMJsonHelper pmHelper(m_root["permission"]["processManager"]);
        pmHelper.setBoolValue("autostart", permissions.autostart, permissions.autostart);
    }

    // 写入 Others
    if (permissions.systemNotice || permissions.systemTime || permissions.systemServer || permissions.windowTop || permissions.preventSleep || permissions.systemDisplayResolution || permissions.systemDisplayBrightness || permissions.systemAudioSettings || permissions.systemAudioSize || permissions.systemCrontab || permissions.systemPowerMode || permissions.systemPowerPolicy)
    {
        KMJsonHelper othersHelper(m_root["permission"]["others"]);
        othersHelper.setBoolValue("system-notice", permissions.systemNotice, permissions.systemNotice);
        othersHelper.setBoolValue("system-time", permissions.systemTime, permissions.systemTime);
        othersHelper.setBoolValue("system-server", permissions.systemServer, permissions.systemServer);
        othersHelper.setBoolValue("window-top", permissions.windowTop, permissions.windowTop);
        othersHelper.setBoolValue("prevent-sleep", permissions.preventSleep, permissions.preventSleep);
        othersHelper.setBoolValue("system-display-resolution", permissions.systemDisplayResolution, permissions.systemDisplayResolution);
        othersHelper.setBoolValue("system-display-brightness", permissions.systemDisplayBrightness, permissions.systemDisplayBrightness);
        othersHelper.setBoolValue("system-audio-settings", permissions.systemAudioSettings, permissions.systemAudioSettings);
        othersHelper.setBoolValue("system-audio-size", permissions.systemAudioSize, permissions.systemAudioSize);
        othersHelper.setBoolValue("system-crontab", permissions.systemCrontab, permissions.systemCrontab);
        othersHelper.setBoolValue("system-power-mode", permissions.systemPowerMode, permissions.systemPowerMode);
        othersHelper.setBoolValue("system-power-policy", permissions.systemPowerPolicy, permissions.systemPowerPolicy);
    }
}

void KMInfoJson::annotationsToJson()
{
    auto isDebInfoEmpty = [](const DebInfo &info) {
        return info.package.empty() &&
               info.version.empty() &&
               info.section.empty() &&
               info.package_type.empty() &&
               info.recommends.empty() &&
               info.uploaders.empty() &&
               info.provides.empty() &&
               info.origin.empty() &&
               info.multi_arch.empty() &&
               info.build_depends.empty() &&
               info.source.empty() &&
               info.suggests.empty() &&
               info.conflicts.empty() &&
               info.breaks.empty() &&
               info.replaces.empty() &&
               info.vcs_git.empty() &&
               info.vcs_browser.empty();
    };

    if (annotations.privilege ||
        annotations.sysapp ||
        annotations.gpu ||
        annotations.polkit ||
        annotations.tray ||
        !annotations.depends.empty() ||
        !annotations.dependApps.empty() ||
        !annotations.desktops.empty() ||
        // -------原生安装属性-------------
        annotations.hostapp ||
        annotations.hostscript ||
        annotations.deltas ||
        !annotations.service.empty() ||
        !annotations.sysgroup.empty() ||
        !annotations.autostart.empty() ||
        !annotations.chmods.empty() ||
        !annotations.installkills.empty() ||
        !annotations.uninstallkills.empty() ||
        !annotations.stopservices.empty() ||
        !annotations.rmdir.empty() ||
        !isDebInfoEmpty(annotations.debinfo) ||
        !annotations.unlink.empty() ||
        !annotations.kylin_dbus_whitelist.empty() ||
        !annotations.install_actions.start_service.empty() ||
        !annotations.install_actions.stop_service.empty() ||
        !annotations.install_actions.add_group.empty() ||
        !annotations.install_actions.chmod.empty() ||
        !annotations.install_actions.autostart.empty() ||
        !annotations.install_actions.remove.empty() ||
        !annotations.install_actions.stop_process.empty() ||
        !annotations.uninstall_actions.stop_service.empty() ||
        !annotations.uninstall_actions.remove.empty() ||
        !annotations.uninstall_actions.stop_process.empty())
    {
        nlohmann::json &annotationsNode = m_root["annotations"];
        KMJsonHelper annotationsHelper(annotationsNode);

        annotationsHelper.setBoolValue("kylin.privilege", annotations.privilege, annotations.privilege);
        annotationsHelper.setBoolValue("kylin.hostPlugins", annotations.hostPlugins, annotations.hostPlugins);
        annotationsHelper.setBoolValue("kylin.sysapp", annotations.sysapp, annotations.sysapp);
        annotationsHelper.setBoolValue("kylin.gpu", annotations.gpu, annotations.gpu);
        annotationsHelper.setBoolValue("kylin.tray", annotations.tray, annotations.tray);
        annotationsHelper.setBoolValue("kylin.polkit", annotations.polkit, annotations.polkit);
        annotationsHelper.setBoolValue("kylin.deltas", annotations.deltas, annotations.deltas);
        annotationsHelper.setStringListValue("kylin.depends", annotations.depends);
        annotationsHelper.setStringListValue("kylin.depend-apps", annotations.dependApps);
        annotationsHelper.setStringListValue("kylin.desktop", annotations.desktops);
        annotationsHelper.setStringListValue("kylin.dbus-whitelist", annotations.kylin_dbus_whitelist);
        annotationsHelper.setStringListValue("kylin.preVersion", annotations.preVersion);
        // -------原生安装属性-------------
        annotationsHelper.setBoolValue("kylin.hostapp", annotations.hostapp, annotations.hostapp);
        annotationsHelper.setBoolValue("kylin.script", annotations.hostscript, annotations.hostscript);
        annotationsHelper.setStringListValue("service", annotations.service);
        annotationsHelper.setStringListValue("kylin.addsystemgroup", annotations.sysgroup);
        annotationsHelper.setStringListValue("kylin.autostart", annotations.autostart);
        annotationsHelper.setStringListValue("kylin.chmod", annotations.chmods);
        annotationsHelper.setStringListValue("kylin.installkill", annotations.installkills);
        annotationsHelper.setStringListValue("kylin.uninstallkill", annotations.uninstallkills);
        annotationsHelper.setStringListValue("kylin.stopservice", annotations.stopservices);
        annotationsHelper.setStringListValue("kylin.rmdir", annotations.rmdir);
        annotationsHelper.setStringListValue("kylin.unlink", annotations.unlink);
        // -------kylin.debinfo-------------
        if (!isDebInfoEmpty(annotations.debinfo))
        {
            std::map<std::string, std::string> debinfoMap = {
                { "Package", annotations.debinfo.package },
                { "Version", annotations.debinfo.version },
                { "Section", annotations.debinfo.section },
                { "Package-Type", annotations.debinfo.package_type },
                { "Recommends", annotations.debinfo.recommends },
                { "Uploaders", annotations.debinfo.uploaders },
                { "Provides", annotations.debinfo.provides },
                { "Origin", annotations.debinfo.origin },
                { "Multi-Arch", annotations.debinfo.multi_arch },
                { "Build-Depends", annotations.debinfo.build_depends },
                { "Source", annotations.debinfo.source },
                { "Suggests", annotations.debinfo.suggests },
                { "Conflicts", annotations.debinfo.conflicts },
                { "Breaks", annotations.debinfo.breaks },
                { "Replaces", annotations.debinfo.replaces },
                { "Vcs-Git", annotations.debinfo.vcs_git },
                { "Vcs-Browser", annotations.debinfo.vcs_browser }
            };

            annotationsHelper.setStringMapValue("kylin.debinfo", debinfoMap);
        }

        if (!annotations.install_actions.start_service.empty() ||
            !annotations.install_actions.stop_service.empty() ||
            !annotations.install_actions.add_group.empty() ||
            !annotations.install_actions.chmod.empty() ||
            !annotations.install_actions.autostart.empty() ||
            !annotations.install_actions.remove.empty() ||
            !annotations.install_actions.stop_process.empty())
        {
            nlohmann::json &installNode = annotationsNode["kylin.install"];
            KMJsonHelper installHelper(installNode);
            installHelper.setStringListValue("start_service", annotations.install_actions.start_service);
            installHelper.setStringListValue("stop_service", annotations.install_actions.stop_service);
            installHelper.setStringListValue("add_group", annotations.install_actions.add_group);
            installHelper.setStringListValue("chmod", annotations.install_actions.chmod);
            installHelper.setStringListValue("autostart", annotations.install_actions.autostart);
            installHelper.setStringListValue("remove", annotations.install_actions.remove);
            installHelper.setStringListValue("stop_process", annotations.install_actions.stop_process);
        }

        if (!annotations.uninstall_actions.stop_service.empty() ||
            !annotations.uninstall_actions.remove.empty() ||
            !annotations.uninstall_actions.stop_process.empty())
        {
            nlohmann::json &uninstallNode = annotationsNode["kylin.uninstall"];
            KMJsonHelper uninstallHelper(uninstallNode);
            uninstallHelper.setStringListValue("stop_service", annotations.uninstall_actions.stop_service);
            uninstallHelper.setStringListValue("remove", annotations.uninstall_actions.remove);
            uninstallHelper.setStringListValue("stop_process", annotations.uninstall_actions.stop_process);
        }
    }
}

void KMInfoJson::toJson()
{
    m_root["schema_version"] = schema_version;
    m_root["id"] = id;
    m_root["base"] = base;
    m_root["runtime"] = runtime;

    KMJsonHelper helper(m_root);
    helper.setStringValue("channel", channel);
    helper.setStringValue("name", name);
    helper.setStringValue("description", description);
    helper.setStringValue("kind", kind);
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

KMRef KMInfoJson::baseRef() const
{
    return parseKMRef(base);
}

KMRef KMInfoJson::runtimeRef() const
{
    if (runtime.empty())
    {
        return KMRef();
    }

    return parseKMRef(runtime);
}

/**
 * @brief : 解析ref成KMRef
 * @param : [in] ref, 格式：${channel}:${base-id}/${base-version}/${arch}
 * @return: KMRef
 */
KMRef KMInfoJson::parseKMRef(const std::string &ref)
{
    std::regex reg{ "[:/]" };
    std::vector<std::string> splits{ std::sregex_token_iterator(ref.begin(), ref.end(), reg, -1), std::sregex_token_iterator() };

    if (splits.size() != 4)
    {
        throw KMException("The format of base and runtime must be : ${channel}:${base-id}/${base-version}/${arch}");
    }

    KMRef kmref = {
        .id = splits.at(1),
        .channel = splits.at(0),
        .version = splits.at(2),
        .arch = splits.at(3),
        .module = "",
        .branch = "",
        .kind = "",
        .baseInstallDir = "",
    };

    return kmref;
}
std::string &KMInfoJson::operator[](const std::string &key)
{
    if (key == "base")
    {
        return base;
    }

    if (key == "runtime")
    {
        return runtime;
    }

    if (key == "depend")
    {
        return depend;
    }
    throw std::invalid_argument(key);
}