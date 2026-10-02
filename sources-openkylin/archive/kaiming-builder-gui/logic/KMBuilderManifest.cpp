#include "KMBuilderManifest.h"

#include <fstream>
#include <sstream>
#include <QDir>

#define PACKAGE_KIND_APP "app"

void KMBuilderManifest::packageToYaml()
{
    YAML::Node package = yaml["package"]; // 获取现有节点
    package["id"] = m_package.m_id.toStdString();
    package["name"] = m_package.m_name.toStdString();
    package["version"] = m_package.m_version.toStdString();
    package["kind"] = m_package.m_kind.toStdString();
    if (!m_package.m_description.isEmpty())
    {
        package["description"] = m_package.m_description.toStdString();
    }
    if (!m_package.m_descriptions.isEmpty())
    {
        mapToYaml(package, m_package.m_descriptions);
    }
    if (!m_package.m_names.isEmpty())
    {
        mapToYaml(package, m_package.m_names);
    }
    yaml["package"] = package;
}

void KMBuilderManifest::vectorToYaml(YAML::Node &yamlNode, QString key, QVector<QString> values)
{
    YAML::Node node = YAML::Node(YAML::NodeType::Sequence);
    for (const auto &value: values)
    {
        node.push_back(value.toStdString());
    }
    yamlNode[key.toStdString()] = node;
}

void KMBuilderManifest::QStringListToYaml(YAML::Node &yamlNode, const QString &key, const QStringList &values)
{
    YAML::Node node = YAML::Node(YAML::NodeType::Sequence);
    for (const auto &value : values)
    {
        node.push_back(value.toStdString());
    }
    yamlNode[key.toStdString()] = node;
}

void KMBuilderManifest::mapToYaml(YAML::Node &yamlNode, QMap<QString, QString> values)
{
    for (auto it = values.begin() ; it != values.end(); it++)
    {
        yamlNode[it.key().toStdString()] = it.value().toStdString();
    }
}

void KMBuilderManifest::permissionFilesystemToYaml(YAML::Node &yamlNode)
{
    if (m_permission.m_filesystem.m_xdgUserDirs)
    {
        yamlNode["xdg-user-dirs"] = m_permission.m_filesystem.m_xdgUserDirs;
    }

    if (!m_permission.m_filesystem.m_filesystemDirs.isEmpty())
    {
        QStringListToYaml(yamlNode, "filesystem-dirs", m_permission.m_filesystem.m_filesystemDirs);
    }
}

void KMBuilderManifest::permissionNetToYaml(YAML::Node &yamlNode)
{
    if (m_permission.m_net.m_network)
    {
        yamlNode["network"] = m_permission.m_net.m_network;
    }
    if (m_permission.m_net.m_scanning)
    {
        yamlNode["scanning"] = m_permission.m_net.m_scanning;
    }
}

void KMBuilderManifest::permissionDeviceToYaml(YAML::Node &yamlNode)
{

    if (m_permission.m_device.m_bluetooth)
    {
        yamlNode["bluetooth"] = m_permission.m_device.m_bluetooth;
    }
    if (m_permission.m_device.m_microphone)
    {
        yamlNode["microphone"] = m_permission.m_device.m_microphone;
    }
    if (m_permission.m_device.m_camera)
    {
        yamlNode["camera"] = m_permission.m_device.m_camera;
    }
    if (m_permission.m_device.m_usb)
    {
        yamlNode["usb"] = m_permission.m_device.m_usb;
    }
    if (m_permission.m_device.m_serial)
    {
        yamlNode["serial"] = m_permission.m_device.m_serial;
    }
}

void KMBuilderManifest::permissionProcessManagerToYaml(YAML::Node &yamlNode)
{
    if (m_permission.m_processManager.m_autostart)
    {
        yamlNode["autostart"] = m_permission.m_processManager.m_autostart;
    }
}

void KMBuilderManifest::permissionOthersToYaml(YAML::Node &yamlNode)
{
    if (m_permission.m_others.m_systemNotice)
    {
        yamlNode["system-notice"] = m_permission.m_others.m_systemNotice;
    }
    if (m_permission.m_others.m_systemTime)
    {
        yamlNode["system-time"] = m_permission.m_others.m_systemTime;
    }
    if (m_permission.m_others.m_systemServer)
    {
        yamlNode["system-server"] = m_permission.m_others.m_systemServer;
    }
    if (m_permission.m_others.m_windowTop)
    {
        yamlNode["window-top"] = m_permission.m_others.m_windowTop;
    }
    if (m_permission.m_others.m_preventSleep)
    {
        yamlNode["prevent-sleep"] = m_permission.m_others.m_preventSleep;
    }
    if (m_permission.m_others.m_systemDisplayResolution)
    {
        yamlNode["system-display-resolution"] = m_permission.m_others.m_systemDisplayResolution;
    }
    if (m_permission.m_others.m_systemDisplayBrightness)
    {
        yamlNode["system-display-brightness"] = m_permission.m_others.m_systemDisplayBrightness;
    }
    if (m_permission.m_others.m_systemAudioSetting)
    {
        yamlNode["system-audio-settings"] = m_permission.m_others.m_systemAudioSetting;
    }
    if (m_permission.m_others.m_systemAudioSize)
    {
        yamlNode["system-audio-size"] = m_permission.m_others.m_systemAudioSize;
    }
    if (m_permission.m_others.m_systemCrontab)
    {
        yamlNode["system-crontab"] = m_permission.m_others.m_systemCrontab;
    }
    if (m_permission.m_others.m_systemPowerMode)
    {
        yamlNode["system-power-mode"] = m_permission.m_others.m_systemPowerMode;
    }
    if (m_permission.m_others.m_systemPowerPolicy)
    {
        yamlNode["system-power-policy"] = m_permission.m_others.m_systemPowerPolicy;
    }

}

void KMBuilderManifest::permissionToYaml()
{
    YAML::Node permission = yaml["permission"]; // 获取现有节点;

    YAML::Node filesystem = permission["filesystem"];
    permissionFilesystemToYaml(filesystem);
    if (!filesystem.IsNull())
    {
        permission["filesystem"] = filesystem;
    }

    YAML::Node net = permission["net"];
    permissionNetToYaml(net);
    if (!net.IsNull())
    {
        permission["net"] = net;
    }

    YAML::Node device = permission["device"];
    permissionDeviceToYaml(device);
    if (!device.IsNull())
    {
        permission["device"] = device;
    }

    YAML::Node processManager = permission["processManager"];
    permissionProcessManagerToYaml(processManager);
    if (!processManager.IsNull())
    {
        permission["processManager"] = processManager;
    }

    YAML::Node others = permission["others"];
    permissionOthersToYaml(others);
    if (!others.IsNull())
    {
        permission["others"] = others;
    }

    if (!permission.IsNull())
    {
        yaml["permission"] = permission;
    }
}

void KMBuilderManifest::annotationToYaml()
{
    YAML::Node annotations = yaml["annotations"];
    if (m_annotations.m_kylinPrivilege)
    {
        annotations["kylin.privilege"] = m_annotations.m_kylinPrivilege;
    }
    if (m_annotations.m_kylinSysapp)
    {
        annotations["kylin.sysapp"] = m_annotations.m_kylinSysapp;
    }
    if (!m_annotations.m_kylinDependApps.isEmpty())
    {
        QStringListToYaml(annotations, "kylin.depend-apps", m_annotations.m_kylinDependApps);
    }
    if (!annotations.IsNull())
    {
        yaml["annotations"] = annotations;
    }
}

void KMBuilderManifest::buildOptionsToYaml(YAML::Node &yamlNode, const Component::BuildOptions buildOptions)
{
    if (!buildOptions.m_cFlags.isEmpty())
    {
        yamlNode["cflags"] = buildOptions.m_cFlags.toStdString();
    }
    if (!buildOptions.m_cppFlags.isEmpty())
    {
        yamlNode["cppflags"] = buildOptions.m_cppFlags.toStdString();
    }
    if (!buildOptions.m_cxxFlags.isEmpty())
    {
        yamlNode["cxxflags"] = buildOptions.m_cxxFlags.toStdString();
    }
    if (!buildOptions.m_ldFlags.isEmpty())
    {
        yamlNode["ldflags"] = buildOptions.m_ldFlags.toStdString();
    }
    if (!buildOptions.m_appendPath.isEmpty())
    {
        yamlNode["append-path"] = buildOptions.m_appendPath.toStdString();
    }
    if (!buildOptions.m_prependPath.isEmpty())
    {
        yamlNode["prepend-path"] = buildOptions.m_prependPath.toStdString();
    }
    if (!buildOptions.m_appendLdLibraryPath.isEmpty())
    {
        yamlNode["append-ld-library-path"] = buildOptions.m_appendLdLibraryPath.toStdString();
    }
    if (!buildOptions.m_prependLdLibraryPath.isEmpty())
    {
        yamlNode["prepend-ld-library-path"] = buildOptions.m_prependLdLibraryPath.toStdString();
    }
    if (!buildOptions.m_appendPkgConfigPath.isEmpty())
    {
        yamlNode["append-pkg-config-path"] = buildOptions.m_appendPkgConfigPath.toStdString();
    }
    if (!buildOptions.m_prependPkgConfigPath.isEmpty())
    {
        yamlNode["prepend-pkg-config-path"] = buildOptions.m_prependPkgConfigPath.toStdString();
    }
    if (!buildOptions.m_env.isEmpty())
    {
        mapToYaml(yamlNode, buildOptions.m_env);
    }
    if (!buildOptions.m_makeArgs.empty())
    {
        QStringListToYaml(yamlNode, "make-args", buildOptions.m_makeArgs);
    }
    if (!buildOptions.m_makeInstallArgs.isEmpty())
    {
        QStringListToYaml(yamlNode, "make-install-args", buildOptions.m_makeInstallArgs);
    }
}

void KMBuilderManifest::archiveToYaml(YAML::Node &yamlNode, const Component::Source::Archive archiveValue)
{
    yamlNode["type"] = "archive";
    if (!archiveValue.m_path.isEmpty())
    {
        yamlNode["path"] = archiveValue.m_path.toStdString();
    }
    if (!archiveValue.m_url.isEmpty())
    {
        yamlNode["url"] = archiveValue.m_url.toStdString();
    }
    if (!archiveValue.m_sha256.isEmpty())
    {
        yamlNode["sha256"] = archiveValue.m_sha256.toStdString();
    }
    if (!archiveValue.m_sha512.isEmpty())
    {
        yamlNode["sha512"] = archiveValue.m_sha512.toStdString();
    }
    if (!archiveValue.m_destFilename.isEmpty())
    {
        yamlNode["dest-filename"] = archiveValue.m_destFilename.toStdString();
    }
}

void KMBuilderManifest::gitToYaml(YAML::Node &yamlNode, const Component::Source::Git gitValue)
{
    yamlNode["type"] = "git";
    if (!gitValue.m_url.isEmpty())
    {
        yamlNode["url"] = gitValue.m_url.toStdString();
    }
    if (!gitValue.m_tag.isEmpty())
    {
        yamlNode["tag"] = gitValue.m_tag.toStdString();
    }
    if (!gitValue.m_branch.isEmpty())
    {
        yamlNode["branch"] = gitValue.m_branch.toStdString();
    }
    if (!gitValue.m_commit.isEmpty())
    {
        yamlNode["commit"] = gitValue.m_commit.toStdString();
    }
}

void KMBuilderManifest::fileToYaml(YAML::Node &yamlNode, const Component::Source::File fileValue)
{
    yamlNode["type"] = "file";
    if (!fileValue.m_path.isEmpty())
    {
        yamlNode["path"] = fileValue.m_path.toStdString();
    }
    if (!fileValue.m_url.isEmpty())
    {
        yamlNode["url"] = fileValue.m_url.toStdString();
    }
    if (!fileValue.m_sha256.isEmpty())
    {
        yamlNode["sha256"] = fileValue.m_sha256.toStdString();
    }
    if (!fileValue.m_sha512.isEmpty())
    {
        yamlNode["sha512"] = fileValue.m_sha512.toStdString();
    }
    if (!fileValue.m_destFilename.isEmpty())
    {
        yamlNode["dest-filename"] = fileValue.m_destFilename.toStdString();
    }

}

void KMBuilderManifest::dirToYaml(YAML::Node &yamlNode, const Component::Source::Dir dirValue)
{
    yamlNode["type"] = "dir";
    if (!dirValue.m_path.isEmpty())
    {
        yamlNode["path"] = dirValue.m_path.toStdString();
    }
    if (!dirValue.m_skip.isEmpty())
    {
        vectorToYaml(yamlNode, "skip", dirValue.m_skip);
    }
}

void KMBuilderManifest::patchToYaml(YAML::Node &yamlNode, const Component::Source::Patch patchValue)
{
    yamlNode["type"] = "patch";
    if (!patchValue.m_path.isEmpty())
    {
        yamlNode["path"] = patchValue.m_path.toStdString();
    }
    if (!patchValue.m_paths.isEmpty())
    {
        vectorToYaml(yamlNode, "paths", patchValue.m_paths);
    }
    if (patchValue.m_stripComponents != -1)
    {
        yamlNode["strip-components"] = patchValue.m_stripComponents;
    }
}

void KMBuilderManifest::sourceToYaml(YAML::Node &yamlNode, const Component::Source source)
{

    switch (source.m_type)
    {
        case ARCHIVE:
        {
            archiveToYaml(yamlNode, source.m_archive);
            break;
        }
        case GIT:
        {
            gitToYaml(yamlNode, source.m_git);
            break;
        }
        case FILE:
        {
            fileToYaml(yamlNode, source.m_file);
            break;
        }
        case DIR:
        {
            dirToYaml(yamlNode, source.m_dir);
            break;
        }
        case PATCH:
        {
            patchToYaml(yamlNode, source.m_patch);
            break;
        }
        case UNKNOWN:
        {
            break;
        }
    }

    if (!source.m_onlyArches.isEmpty())
    {
        vectorToYaml(yamlNode, "only-arches", source.m_onlyArches);
    }
    if (!source.m_skipArches.isEmpty())
    {
        vectorToYaml(yamlNode, "skip-arches", source.m_skipArches);
    }
    if (!source.m_command.isEmpty())
    {
        yamlNode["command"] = source.m_command.toStdString();
    }
}

void KMBuilderManifest::componentToYaml(YAML::Node &yamlNode, const Component component)
{
    yamlNode["name"] = component.m_name.toStdString();
    yamlNode["buildsystem"] = component.m_buildSystem.toStdString();
    if (!component.m_buildScript.isEmpty())
    {
        yamlNode["build-script"] = component.m_buildScript.toStdString();
    }
    if (!component.m_configOpts.isEmpty())
    {
        vectorToYaml(yamlNode, "config-opts", component.m_configOpts);
    }

    YAML::Node buildOptions = yamlNode["build-options"];
    buildOptionsToYaml(buildOptions, component.m_buildOptions);
    if (!buildOptions.IsNull())
    {
        yamlNode["build-options"] = buildOptions;
    }

    if (!component.m_preInstall.isEmpty())
    {
        yamlNode["pre-install"] = component.m_preInstall.toStdString();
    }

    if (!component.m_postInstall.isEmpty())
    {
        yamlNode["post-install"] = component.m_postInstall.toStdString();
    }

    if (!component.m_cleanup.isEmpty())
    {
        vectorToYaml(yamlNode, "cleanup", component.m_cleanup);
    }

    if (!component.m_sources.isEmpty())
    {
        YAML::Node sources = YAML::Node(YAML::NodeType::Sequence);
        for (auto &source: component.m_sources)
        {
            YAML::Node sourceNode;
            sourceToYaml(sourceNode, source);
            sources.push_back(sourceNode);
        }
        yamlNode["sources"] = sources;
    }
}

/**
 * @brief 生成yaml
 * @param path: 生成yaml路径
 */
bool KMBuilderManifest::toYaml(QString path)
{
    try
    {
        yaml["version"] = m_version.toStdString();
        packageToYaml();
        vectorToYaml(yaml,"command", m_commands);
        if (!m_channel.isEmpty())
        {
            yaml["channel"] = m_channel.toStdString();
        }
        permissionToYaml();

        if (!m_modules.isEmpty())
        {
            YAML::Node modules;
            mapToYaml(modules, m_modules);
            yaml["modules"] = modules;
        }

        if (!m_base.isEmpty())
        {
            yaml["base"] = m_base.toStdString();
        }

        if (!m_runtime.isEmpty())
        {
            yaml["runtime"] = m_runtime.toStdString();
        }


        vectorToYaml(yaml, "depends", m_depends);
        vectorToYaml(yaml, "cleanup", m_cleanups);


        if (!m_envs.isEmpty())
        {
            YAML::Node envs;
            mapToYaml(envs, m_envs);
            yaml["env"] = envs;
        }

        if (!m_license.isEmpty())
        {
            yaml["license"] = m_license.toStdString();
        }

        if (!m_homepage.isEmpty())
        {
            yaml["homepage"] = m_homepage.toStdString();
        }

        if (!m_maintainers.isEmpty())
        {
            vectorToYaml(yaml, "maintainer", m_maintainers);
        }

        annotationToYaml();

        YAML::Node components = YAML::Node(YAML::NodeType::Sequence);
        for (auto &component: m_components)
        {
            YAML::Node componentNode;
            componentToYaml(componentNode, component);
            components.push_back(componentNode);
        }
        if (!components.IsNull())
        {
            yaml["components"] = components;
        }
        std::ofstream fout(path.toStdString());
        fout << yaml;
        fout.close();
        return true;
    }catch(...)
    {
        return false;
    }
}

/**
 * @brief yamlNode转为字符串
 * @param yamlNode:yaml节点
 * @return yamlNode转换后的字符串
 */

QString KMBuilderManifest::nodeToString(const YAML::Node &yamlNode)
{
    if(yamlNode.IsNull())
    {
       return "Invalid source data";
    }
    std::stringstream ss;
    ss << yamlNode;
    return QString::fromStdString(ss.str());
}




void KMBuilderManifest::parsePermission(const YAML::Node &permissionNode)
{
    // 解析Filesystem部分
    YAML::Node filesystemNode = permissionNode["filesystem"];
    if (filesystemNode.IsDefined())
    {
        KMYAMLNodeHelper helper(filesystemNode);
        m_permission.m_filesystem.m_xdgUserDirs = helper.getBoolValue("xdg-user-dirs");
        m_permission.m_filesystem.m_filesystemDirs = helper.getStringListValue1("filesystem-dirs");
    }

    // 解析Net部分
    YAML::Node netNode = permissionNode["net"];
    if (netNode.IsDefined())
    {
        KMYAMLNodeHelper helper(netNode);
        m_permission.m_net.m_network = helper.getBoolValue("network");
        m_permission.m_net.m_scanning = helper.getBoolValue("scanning");
    }

    // 解析Device部分
    YAML::Node deviceNode = permissionNode["device"];
    if (deviceNode.IsDefined())
    {
        KMYAMLNodeHelper helper(deviceNode);
        m_permission.m_device.m_bluetooth = helper.getBoolValue("bluetooth");
        m_permission.m_device.m_microphone = helper.getBoolValue("microphone");
        m_permission.m_device.m_camera = helper.getBoolValue("camera");
        m_permission.m_device.m_usb = helper.getBoolValue("usb");
        m_permission.m_device.m_serial = helper.getBoolValue("serial");
    }

    // 解析ProcessManager部分
    YAML::Node processManagerNode = permissionNode["processManager"];
    if (processManagerNode.IsDefined())
    {
        KMYAMLNodeHelper helper(processManagerNode);
        m_permission.m_processManager.m_autostart = helper.getBoolValue("autostart");
    }

    // 解析Others部分
    YAML::Node othersNode = permissionNode["others"];
    if (othersNode.IsDefined())
    {
        KMYAMLNodeHelper helper(othersNode);
        m_permission.m_others.m_systemNotice = helper.getBoolValue("system-notice");
        m_permission.m_others.m_systemTime = helper.getBoolValue("system-time");
        m_permission.m_others.m_systemServer = helper.getBoolValue("system-server");
        m_permission.m_others.m_windowTop = helper.getBoolValue("window-top");
        m_permission.m_others.m_preventSleep = helper.getBoolValue("prevent-sleep");
        m_permission.m_others.m_systemDisplayResolution = helper.getBoolValue("system-display-resolution");
        m_permission.m_others.m_systemDisplayBrightness = helper.getBoolValue("system-display-brightness");
        m_permission.m_others.m_systemAudioSetting = helper.getBoolValue("system-audio-settings");
        m_permission.m_others.m_systemAudioSize = helper.getBoolValue("system-audio-size");
        m_permission.m_others.m_systemCrontab = helper.getBoolValue("system-crontab");
        m_permission.m_others.m_systemPowerMode = helper.getBoolValue("system-power-mode");
        m_permission.m_others.m_systemPowerPolicy = helper.getBoolValue("system-power-policy");

    }

}

void KMBuilderManifest::parseBuildOptions(YAML::Node buildOptionsNode, Component::BuildOptions &buildOptions)
{
    KMYAMLNodeHelper helper(buildOptionsNode);

    buildOptions.m_appendLdLibraryPath = helper.getStringValue("append-ld-library-path");
    buildOptions.m_appendPath = helper.getStringValue("append-path");
    buildOptions.m_appendPkgConfigPath = helper.getStringValue("append-pkg-config-path");
    buildOptions.m_cFlags = helper.getStringValue("cflags");
    buildOptions.m_cppFlags = helper.getStringValue("cppflags");
    buildOptions.m_cxxFlags = helper.getStringValue("cxxflags");
    buildOptions.m_env = helper.getStringMapValue("env");
    buildOptions.m_ldFlags = helper.getStringValue("ldflags");
    buildOptions.m_makeArgs = helper.getStringListValue1("make-args");
    buildOptions.m_makeInstallArgs = helper.getStringListValue1("make-install-args");
    buildOptions.m_prependLdLibraryPath = helper.getStringValue("prepend-ld-library-path");
    buildOptions.m_prependPath = helper.getStringValue("prepend-path");
    buildOptions.m_prependPkgConfigPath = helper.getStringValue("prepend-pkg-config-path");
}

void KMBuilderManifest::setType(const QString &typeStr, SOURCEType &type)
{
    if (typeStr == "archive")
    {
        type = ARCHIVE;
    }
    else if (typeStr == "git")
    {
        type = GIT;
    }
    else if (typeStr == "dir")
    {
        type = DIR;
    }
    else if (typeStr == "file")
    {
        type = FILE;
    }
    else
    {
        type = UNKNOWN;
    }

}

void KMBuilderManifest::parseSources(YAML::Node sourcesNode, QVector<Component::Source> &sources, QString base_path)
{
    if (!sourcesNode.IsSequence())
    {
        return;
    }
    for(auto iter = sourcesNode.begin(); iter != sourcesNode.end(); iter++)
    {
        YAML::Node sourceNode = *iter;
        if (sourceNode.IsScalar())
        {
            base_path = KMBuilderStringUtils::getpathParent(KMBuilderStringUtils::buildFilename(m_project_path ,QString::fromStdString(sourceNode.as<std::string>())));
            sourceNode = loadShareNode(sourceNode);

        }
        KMYAMLNodeHelper helper(sourceNode);
        Component::Source source;
        QString type = helper.getStringValue("type");
        if (!base_path.isEmpty())
        {
            source.m_base = base_path;
        }
        setType(type, source.m_type);
        switch (source.m_type)
        {
        case ARCHIVE:
            source.m_archive.m_path = helper.getStringValue("path");
            if (!source.m_archive.m_path.isEmpty() && !source.m_base.isEmpty())
            {
                source.m_archive.m_path = KMBuilderStringUtils::getpathRelative(m_project_path, KMBuilderStringUtils::buildFilename(source.m_base, source.m_archive.m_path));
            }
            source.m_archive.m_url = helper.getStringValue("url");
            source.m_archive.m_sha256 = helper.getStringValue("sha256");
            source.m_archive.m_sha512 = helper.getStringValue("sha512");
            source.m_archive.m_destFilename = helper.getStringValue("dest-filename");
            break;
        case GIT:
            source.m_git.m_url = helper.getStringValue("url");
            source.m_git.m_branch = helper.getStringValue("branch");
            source.m_git.m_tag = helper.getStringValue("tag");
            source.m_git.m_commit = helper.getStringValue("commit");
            break;
        case DIR:
            source.m_dir.m_path = helper.getStringValue("path");
            if (!source.m_dir.m_path.isEmpty() && !source.m_base.isEmpty())
            {
                source.m_dir.m_path = KMBuilderStringUtils::getpathRelative(m_project_path, KMBuilderStringUtils::buildFilename(source.m_base, source.m_dir.m_path));
            }
            source.m_dir.m_skip = helper.getStringListValue("skip");
            break;
        case FILE:
            source.m_file.m_path = helper.getStringValue("path");
            if (source.m_file.m_path.isEmpty()  && !source.m_base.isEmpty())
            {
                source.m_file.m_path = KMBuilderStringUtils::getpathRelative(m_project_path, KMBuilderStringUtils::buildFilename(source.m_base, source.m_file.m_path));
            }
            source.m_file.m_url = helper.getStringValue("url");
            source.m_file.m_sha256 = helper.getStringValue("sha256");
            source.m_file.m_sha512 = helper.getStringValue("sha512");
            source.m_file.m_destFilename = helper.getStringValue("dest-filename");
            break;
        default:
            continue;
        }
        sources.push_back(source);
    }
}

YAML::Node KMBuilderManifest::loadShareNode(const YAML::Node &shareNode)
{
    QString shareFile = shareNode.IsScalar() ? QString::fromStdString(shareNode.as<std::string>()): "";
    shareFile = KMBuilderStringUtils::buildFilename(m_project_path, shareFile);
    if (shareFile.endsWith(".yaml") || shareFile.endsWith(".yml"))
    {
        return KMYAMLNodeHelper::loadFromYAML(shareFile);
    }
    if (shareFile.endsWith(".json"))
    {
        return KMYAMLNodeHelper::loadFromJson(shareFile);
    }
    return YAML::Node();
}

void KMBuilderManifest::parseComponents(const YAML::Node &componentsNode)
{
    if (!componentsNode.IsSequence())
    {
        return;
    }
    for(auto iter = componentsNode.begin(); iter != componentsNode.end(); iter++)
    {
        YAML::Node componentNode = *iter;
        QString share_path;

        if (componentNode.IsScalar())
        {
            share_path = KMBuilderStringUtils::getpathParent(KMBuilderStringUtils::buildFilename(m_project_path, QString::fromStdString(componentNode.as<std::string>())));
            componentNode = loadShareNode(componentNode);
        }

        KMYAMLNodeHelper helper(componentNode);
        Component component;
        component.m_name = helper.getStringValue("name");
        component.m_buildScript = helper.getStringValue("build-script");
        component.m_buildSystem = helper.getStringValue("buildsystem");
        component.m_cleanup = helper.getStringListValue("cleanup");
        component.m_configOpts = helper.getStringListValue("config-opts");
        component.m_preInstall = helper.getStringValue("pre-install");
        component.m_postInstall = helper.getStringValue("post-install");
        parseBuildOptions(componentNode["build-options"], component.m_buildOptions);
        parseSources(componentNode["sources"], component.m_sources, share_path);
        m_components.push_back(component);
    }
}

void KMBuilderManifest::parseFromYaml()
{
    KMYAMLNodeHelper helper(yaml);
    //解析顶层属性
    m_version = helper.getStringValue("version");
    m_commands = helper.getStringListValue("command");
    m_channel = helper.getStringValue("channel", "stable");
    m_modules = helper.getStringMapValue("modules");
    m_base = helper.getStringValue("base");
    m_runtime = helper.getStringValue("runtime");
    m_cleanups = helper.getStringListValue("cleanup");
    m_envs = helper.getStringMapValue("env");
    m_depends = helper.getStringListValue("depends");
    m_license = helper.getStringValue("license");
    m_homepage = helper.getStringValue("homepage");
    m_maintainers = helper.getStringListValue("maintainer");

    //package属性
    YAML::Node packageNode = yaml["package"];
    KMYAMLNodeHelper package(packageNode);
    m_package.m_id = package.getStringValue("id");
    m_package.m_name = package.getStringValue("name");
    m_package.m_version = package.getStringValue("version");
    m_package.m_kind = package.getStringValue("kind", PACKAGE_KIND_APP);
    m_package.m_description = package.getStringValue("description");

    //permission属性
    YAML::Node permissionNode = yaml["permission"];
    if (permissionNode.IsDefined())
    {
        parsePermission(permissionNode);
    }
    else
    {
        m_permission.m_none = true;
    }

    //components属性
    YAML::Node componentsNode = yaml["components"];
    parseComponents(componentsNode);

    //annotations属性
    YAML::Node annotationsNode = yaml["annotations"];
    if (annotationsNode.IsDefined())
    {
        KMYAMLNodeHelper annotationsHelper(annotationsNode);
        m_annotations.m_kylinPrivilege = annotationsHelper.getBoolValue("kylin.privilege");
        m_annotations.m_kylinSysapp = annotationsHelper.getBoolValue("kylin.sysapp");
        m_annotations.m_kylinDependApps = annotationsHelper.getStringListValue1("kylin.depend-apps");
    }
}

void KMBuilderManifest::loadFromYaml(const QString &path)
{
    m_project_path = KMBuilderStringUtils::getpathParent(path);
    if (!path.endsWith(".yaml") && !path.endsWith(".yml"))
    {
        return ;
    }
    yaml = KMYAMLNodeHelper::loadFromYAML(path);
    parseFromYaml();
}

void KMBuilderManifest::cleanup()
{
    //顶级属性初始化
    m_commands.clear();
    m_channel = "stable";
    m_modules.clear();
    m_base.clear();
    m_runtime.clear();
    m_envs.clear();
    m_depends.clear();
    m_license.clear();
    m_homepage.clear();
    m_maintainers.clear();


    //package初始化
    m_package.m_id.clear();
    m_package.m_name.clear();
    m_package.m_kind = "app";
    m_package.m_version.clear();
    m_package.m_description.clear();

    //permission初始化
    Permission permission;
    m_permission = permission;

    //Component初始化
    m_components.clear();

    //Annotation初始化
    Annotations annotation;
    m_annotations = annotation;

}
