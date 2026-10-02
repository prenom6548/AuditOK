/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderManifest.h"

#include <filesystem>
#include <set>

#include "KMBuilderUtils.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMYAMLNodeHelper.h"
#include "KMBuilderSource.h"

namespace fs = std::filesystem;

KMBuilderManifest::KMBuilderManifest()
{
    
}

KMBuilderManifest::KMBuilderManifest(YAML::Node rootNode)
{
    m_rootNode = rootNode;
}

KMBuilderManifest::~KMBuilderManifest()
{}

/**
 * @brief : 工厂函数，根据yaml格式的清单文件创建清单对象
 * @param : [in] yaml，yaml清单文件
 * @return: std::shared_ptr<KMBuilderManifest>，创建成功则关联对象；
 * @throws: KMException，创建失败则函数抛出异常
 */  
std::shared_ptr<KMBuilderManifest> KMBuilderManifest::loadFromYAML(const std::string& yaml)
{
    if (!KMBuilderUtils::endsWith(yaml, ".km.yaml") && !KMBuilderUtils::endsWith(yaml, ".km.yml"))
    {
        if (KMBuilderUtils::endsWith(yaml, ".yaml") || KMBuilderUtils::endsWith(yaml, ".yml"))
        {
            KMError("Manifest file must be ***.km.yaml or ***.km.yml");
        }
        else
        {
            throw KMException("Manifest file must be ***.km.yaml or ***.km.yml");
        }
    }

    std::shared_ptr<KMBuilderManifest> manifest;
    YAML::Node rootNode = KMYAMLNodeHelper::loadFromYAML(yaml);
    manifest.reset(new KMBuilderManifest(rootNode));
    manifest->m_yaml = yaml;
    manifest->parseFromYAML();

    return manifest;
}

void KMBuilderManifest::checkChannel()
{
    std::vector<std::string> base = KMBuilderUtils::splitString(m_base,"/");
    std::vector<std::string> runtime = KMBuilderUtils::splitString(m_runtime,"/");
    if (m_channel != base.at(1))
    {
        throw KMException("The appChannel " + m_channel + "and " + "the baseChannel " + base.at(1) + "is not matched");
    }
    if (!m_runtime.empty() && m_channel != runtime.at(1))
    {
        throw KMException("The appChannel " + m_channel + "and " + "the runtimeChannel " + runtime.at(1) + "is not matched");
    }
}

void KMBuilderManifest::parseFromYAML()
{
    // 此方法主要解析顶层属性
    KMYAMLNodeHelper helper(m_rootNode);

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
    maintainers = helper.getStringListValue("maintainer");
    m_exportPlugins = helper.getStringListValue("export_plugins");

    //查看app、base、runtime的channel是否一致
    // checkChannel();

    // package属性
    YAML::Node packageNode = m_rootNode["package"];
    KMYAMLNodeHelper package(packageNode);
    m_package.m_id = package.getStringValue("id");
    m_package.m_name = package.getStringValue("name");
    m_package.m_version = package.getStringValue("version");
    m_package.m_kind = package.getStringValue("kind", PACKGE_KIND_APP);
    m_package.m_description = package.getStringValue("description");
    m_package.m_names = package.fromPrefix("name[");
    m_package.m_descriptions = package.fromPrefix("description[");

    //permission属性
    YAML::Node permissionNode = m_rootNode["permission"];
    if (permissionNode.IsDefined())
    {
        parsePermission(permissionNode);
    }
    else
    {
        m_permission.m_none = true;
    }

    // components属性
    std::string saved_demarshalBaseDir = m_demarshalBaseDir;
    std::string demarshalBaseDir = fs::path(m_yaml).parent_path().string();
    m_demarshalBaseDir = demarshalBaseDir;
    YAML::Node componentsNode = m_rootNode["components"];
    m_components = parseComponents(componentsNode);
    m_demarshalBaseDir = saved_demarshalBaseDir;

    YAML::Node annotationsNode = m_rootNode["annotations"];
    if (annotationsNode.IsDefined())
    {
        parseAnnotations(annotationsNode);
    }
    else
    {
        if (m_package.m_kind == PACKGE_KIND_DEPEND || m_package.m_kind == PACKGE_KIND_RUNTIME)
        {
            // depend类型开明包默认kylin.sysapp=true
            m_annotations.kylin_sysapp = true;
        }
    }

    // package属性
    YAML::Node subpackagesNode = m_rootNode["subpackages"];
    if (subpackagesNode.IsDefined())
    {
        parseSubPackages(subpackagesNode);
    }

    validate();
}

void KMBuilderManifest::parseAnnotations(YAML::Node& annotationsNode)
{
    KMYAMLNodeHelper annotationsHelper(annotationsNode);
    m_annotations.kylin_privilege = annotationsHelper.getBoolValue("kylin.privilege");
    if (m_package.m_kind == PACKGE_KIND_DEPEND || m_package.m_kind == PACKGE_KIND_RUNTIME)
    {
        // depend类型开明包默认kylin.sysapp=true
        m_annotations.kylin_sysapp = annotationsHelper.getBoolValue("kylin.sysapp", true);
    }
    else
    {
        m_annotations.kylin_sysapp = annotationsHelper.getBoolValue("kylin.sysapp");
    }
    m_annotations.kylin_gpu = annotationsHelper.getBoolValue("kylin.gpu");
    m_annotations.kylin_polkit = annotationsHelper.getBoolValue("kylin.polkit");
    m_annotations.kylin_depend_apps = annotationsHelper.getStringListValue("kylin.depend-apps");
    m_annotations.kylin_desktops = annotationsHelper.getStringListValue("kylin.desktop");

    m_annotations.kylin_uninstallkills = annotationsHelper.getStringListValue("kylin.uninstallkill");
    m_annotations.kylin_stopservices = annotationsHelper.getStringListValue("kylin.stopservice");
    m_annotations.kylin_rmdir = annotationsHelper.getStringListValue("kylin.rmdir");
    m_annotations.kylin_unlink = annotationsHelper.getStringListValue("kylin.unlink");

    YAML::Node installNode = annotationsNode["kylin.install"];
    if (installNode.IsDefined())
    {
        KMYAMLNodeHelper installHelper(installNode);
        m_annotations.kylin_install.start_service = installHelper.getStringListValue("start_service");
        m_annotations.kylin_install.stop_service = installHelper.getStringListValue("stop_service");
        m_annotations.kylin_install.add_group = installHelper.getStringListValue("add_group");
        m_annotations.kylin_install.chmod = installHelper.getStringListValue("chmod");
        m_annotations.kylin_install.autostart = installHelper.getStringListValue("autostart");
        m_annotations.kylin_install.remove = installHelper.getStringListValue("remove");
        m_annotations.kylin_install.stop_process = installHelper.getStringListValue("stop_process");
    }

    YAML::Node uninstallNode = annotationsNode["kylin.uninstall"];
    if (uninstallNode.IsDefined())
    {
        KMYAMLNodeHelper uninstallHelper(uninstallNode);
        m_annotations.kylin_uninstall.stop_service = uninstallHelper.getStringListValue("stop_service");
        m_annotations.kylin_uninstall.remove = uninstallHelper.getStringListValue("remove");
        m_annotations.kylin_uninstall.stop_process = uninstallHelper.getStringListValue("stop_process");
    }
}

void KMBuilderManifest::parseSubPackages(YAML::Node& subpackagesNode)
{
    if (subpackagesNode.IsSequence())
    {
        std::set<std::string> ids;
        for (YAML::Node::const_iterator iter = subpackagesNode.begin(); iter != subpackagesNode.end(); iter++)
        {
            YAML::Node subpackageNode = *iter;
            if (subpackageNode.IsMap())
            {
                KMYAMLNodeHelper helper(subpackageNode);
                SubPackage subpackage;
                subpackage.m_id = helper.getStringValue("id", m_package.m_id);
                if (ids.count(subpackage.m_id) > 0)
                {
                    throw KMException("Repeat 'id' property for subpackage : " + subpackage.m_id + " in xxx.km.yaml");
                }
                ids.insert(subpackage.m_id);
                subpackage.m_name = helper.getStringValue("name", m_package.m_name);
                subpackage.m_kind = helper.getStringValue("kind", m_package.m_kind);
                subpackage.m_description = helper.getStringValue("description", m_package.m_description);
                subpackage.m_names = helper.fromPrefix("name[");
                if (subpackage.m_names.empty() && !m_package.m_names.empty())
                {
                    subpackage.m_names = m_package.m_names;
                }
                subpackage.m_descriptions = helper.fromPrefix("description[");
                if (subpackage.m_descriptions.empty() && !m_package.m_descriptions.empty())
                {
                    subpackage.m_descriptions = m_package.m_descriptions;
                }
                subpackage.m_module = helper.getStringValue("module");
                if (subpackage.m_module.empty())
                {
                    subpackage.m_module = subpackage.m_kind == PACKGE_KIND_APP ? "binary" : (subpackage.m_kind == PACKGE_KIND_DEPEND ? "devel" : "");
                    if (subpackage.m_module.empty())
                    {
                        throw KMException("Please configure 'module' property for subpackage : " + subpackage.m_id + " in xxx.km.yaml");
                    }
                }
                subpackage.m_commands = helper.getStringListValue("command");
                if (subpackage.m_kind == PACKGE_KIND_APP && subpackage.m_commands.empty() && !m_commands.empty())
                {
                    subpackage.m_commands = m_commands;
                }
                subpackage.m_includes = helper.getStringListValue("includes");
                if (subpackage.m_includes.empty())
                {
                    throw KMException("Please configure 'includes' property for subpackage : " + subpackage.m_id + " in xxx.km.yaml");
                }
                subpackage.m_depends = helper.getStringListValue("depends");

                m_subpackages.push_back(subpackage);
            }
        }
    }
}

void KMBuilderManifest::parsePermission(const YAML::Node& permissionNode)
{
    // 解析Filesystem部分 
    YAML::Node filesystemNode = permissionNode["filesystem"];
    if (filesystemNode.IsDefined())
    {
        KMYAMLNodeHelper helper(filesystemNode);
        m_permission.m_filesystem.m_xdgUserDirs = helper.getBoolValue("xdg-user-dirs");
        m_permission.m_filesystem.m_filesystemDirs = helper.getStringListValue("filesystem-dirs");
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

std::vector<std::shared_ptr<KMBuilderModule>> KMBuilderManifest::parseComponents(const YAML::Node& modulesNode)
{
    if (!modulesNode.IsDefined())
    {
        throw KMException("Not found 'components' in manifest file");
    }

    std::vector<std::shared_ptr<KMBuilderModule>> modules;
    if (modulesNode.IsSequence())
    {
        for (auto iter = modulesNode.begin(); iter != modulesNode.end(); iter++)
        {
            std::string saved_demarshalBaseDir = m_demarshalBaseDir;

            YAML::Node moduleNode = *iter;
            if (moduleNode.IsScalar())
            {
                // 共享模块，其内容为共享模块所在的文件相对路径，形如：openkylin-shared-modules/mate-common/mate-common.json
                YAML::Node shareModule = loadShareNode(moduleNode);
                if (!shareModule)
                {
                    KMWarn(moduleNode.as<std::string>() + " parse failed ");
                    continue ;
                }

                // 共享模块的base目录变更为共享模块文件所在的目录
                std::string shareFile = moduleNode.as<std::string>();
                shareFile = KMBuilderUtils::buildFilename(m_demarshalBaseDir, shareFile);
                m_demarshalBaseDir = fs::path(shareFile).parent_path().string();

                moduleNode = shareModule;
            }

            // 添加一个 module
            KMYAMLNodeHelper helper(moduleNode);
            std::shared_ptr<KMBuilderModule> builderModule = make_shared<KMBuilderModule>();
            builderModule->setProperty(KMBuilderModule::Module_Name, helper.getStringValue("name"));
            builderModule->setProperty(KMBuilderModule::Module_Subdir, helper.getStringValue("subdir"));           
            builderModule->setProperty(KMBuilderModule::Module_RmConfigure, helper.getBoolValue("rm-configure"));   
            builderModule->setProperty(KMBuilderModule::Module_Disable, helper.getBoolValue("disabled"));           
            builderModule->setProperty(KMBuilderModule::Module_NoAutogen, helper.getBoolValue("no-autogen"));       
            builderModule->setProperty(KMBuilderModule::Module_NoParallelMake, helper.getBoolValue("no-parallel-make"));    
            builderModule->setProperty(KMBuilderModule::Module_NoMakeInstall, helper.getBoolValue("no-make-install"));      
            builderModule->setProperty(KMBuilderModule::Module_NoPythonTimestampFix, helper.getBoolValue("no-python-timestamp-fix"));
            builderModule->setProperty(KMBuilderModule::Module_Cmake, helper.getBoolValue("cmake"));

            std::string buildSystem = helper.getStringValue("buildsystem");
            builderModule->setBuildSystem(buildSystem);

            builderModule->setProperty(KMBuilderModule::Module_InstallRule, helper.getStringValue("install-rule"));
            builderModule->setProperty(KMBuilderModule::Module_TestRule, helper.getStringValue("test-rule"));
            builderModule->setProperty(KMBuilderModule::Module_Builddir, helper.getBoolValue("builddir"));

            YAML::Node sourcesNode = moduleNode["sources"];
            builderModule->setBuildSources(parseSources(sourcesNode));

            builderModule->setConfigOptions(helper.getStringListValue("config-opts"));
            builderModule->setBuildScripts(helper.getStringValue("build-script"));
            builderModule->setTestCommands(helper.getStringListValue("test-commands"));
            builderModule->setCleanup(helper.getStringListValue("cleanup"));
            builderModule->setCleanupPlatform(helper.getStringListValue("cleanup-platform"));
            builderModule->setPostInstalls(helper.getStringValue("post-install"));
            builderModule->setPreInstalls(helper.getStringValue("pre-install"));

            // 处理 modules 包含的 子modules
            YAML::Node subModules = moduleNode["components"];
            if (subModules.IsDefined())
            {
                builderModule->setBuilderModules(parseComponents(subModules));
            }
            YAML::Node buildOptions = moduleNode["build-options"];
            builderModule->setBuildOptions(parseBuildOptions(buildOptions));

            modules.push_back(builderModule);

            m_demarshalBaseDir = saved_demarshalBaseDir;
        }
    }
    else
    {
        throw KMException("'components' must be sequence in manifest file");
    }

    return modules;
}

YAML::Node KMBuilderManifest::loadShareNode(const YAML::Node& shareNode)
{
    std::string shareFile = shareNode.IsScalar() ? shareNode.as<std::string>() : "";
    shareFile = KMBuilderUtils::buildFilename(m_demarshalBaseDir, shareFile);

    if (KMBuilderUtils::endsWith(shareFile, ".yaml") || KMBuilderUtils::endsWith(shareFile, ".yml"))
    {
        return KMYAMLNodeHelper::loadFromYAML(shareFile);
    }
    else if (KMBuilderUtils::endsWith(shareFile, ".json"))
    {
        return KMYAMLNodeHelper::loadFromJson(shareFile);
    }

    return YAML::Node();
}

std::vector<std::shared_ptr<KMBuilderSource>> KMBuilderManifest::parseSources(const YAML::Node& sourcesNode)
{
    if (!sourcesNode.IsDefined())
    {
        throw KMException("Not found 'sources' in manifest file");
    }

    std::vector<std::shared_ptr<KMBuilderSource>> sources;
    if (sourcesNode.IsSequence())
    {
        for (auto iter = sourcesNode.begin(); iter != sourcesNode.end(); iter++)
        {
            std::string saved_demarshalBaseDir = m_demarshalBaseDir;

            YAML::Node sourceNode = *iter;
            if (sourceNode.IsScalar())
            {
                // 共享源，其内容为共享源所在的文件相对路径，形如：openkylin-shared-modules/mate-common/mate-common.json
                YAML::Node shareSource = loadShareNode(sourceNode);
                if (!shareSource)
                {
                    KMWarn(sourceNode.as<std::string>() + " parse failed ");
                    continue ;
                }

                // 共享模块的base目录变更为共享模块文件所在的目录
                std::string shareFile = sourceNode.as<std::string>();
                shareFile = KMBuilderUtils::buildFilename(m_demarshalBaseDir, shareFile);
                m_demarshalBaseDir = fs::path(shareFile).parent_path().string();

                sourceNode = shareSource;
            }

            // 动态构建源类对象，并解析源
            KMYAMLNodeHelper helper(sourceNode);
            std::string sourceType = helper.getStringValue("type");
            std::shared_ptr<KMBuilderSource> source = KMBuilderSource::createSource(sourceType);
            source->setBaseDir(m_demarshalBaseDir);
            source->deserialize(sourceNode);

            sources.push_back(source);

            m_demarshalBaseDir = saved_demarshalBaseDir;
        }
    }
    else
    {
        throw KMException("'sources' must be sequence in manifest file");
    }

    return sources;
}

std::shared_ptr<KMBuildOptions> KMBuilderManifest::parseBuildOptions(YAML::Node &node)
{
    std::shared_ptr<KMBuildOptions> buildOptions = make_shared<KMBuildOptions>();

    if (!node.IsDefined())
    {
        return buildOptions;
    }

    KMYAMLNodeHelper helper(node);
    buildOptions->m_strip = helper.getBoolValue("strip");
    buildOptions->m_noDebuginfo = helper.getBoolValue("no-debuginfo");
    buildOptions->m_noDebuginfoCompression = helper.getBoolValue("no-debuginfo-compression");
    buildOptions->m_cFlags = helper.getStringValue("cflags");
    buildOptions->m_cflagsOverride = helper.getBoolValue("cflags-override");
    buildOptions->m_cppFlags = helper.getStringValue("cppflags");
    buildOptions->m_cppFlagsOverride = helper.getBoolValue("cppflags-override");
    buildOptions->m_cxxFlags = helper.getStringValue("cxxflags");
    buildOptions->m_cxxFlagsOverride = helper.getBoolValue("cxxflags-override");
    buildOptions->m_ldFlags = helper.getStringValue("ldflags");
    buildOptions->m_ldFlagsOverride = helper.getBoolValue("ldflags-override");
    buildOptions->m_appendPath = helper.getStringValue("append-path");
    buildOptions->m_prependPath = helper.getStringValue("prepend-path");
    buildOptions->m_appendLdLibraryPath = helper.getStringValue("append-ld-library-path");
    buildOptions->m_prependLdLibraryPath = helper.getStringValue("prepend-ld-library-path");
    buildOptions->m_appendPkgConfigPath = helper.getStringValue("append-pkg-config-path");
    buildOptions->m_prependPkgConfigPath = helper.getStringValue("prepend-pkg-config-path");
    buildOptions->m_prefix = helper.getStringValue("prefix");
    buildOptions->m_libDir = helper.getStringValue("libdir");
    buildOptions->m_buildArgs = helper.getStringListValue("build-args");
    buildOptions->m_testArgs = helper.getStringListValue("test-args");
    buildOptions->m_configOpts = helper.getStringListValue("config-opts");
    buildOptions->m_secretOpts = helper.getStringListValue("secret-opts");
    buildOptions->m_makeArgs = helper.getStringListValue("make-args");
    buildOptions->m_makeInstallArgs = helper.getStringListValue("make-install-args");

    buildOptions->m_env = helper.getStringMapValue("env");
    buildOptions->m_secretEnv = helper.getStringMapValue("secret-env");

    return buildOptions;
}

void KMBuilderManifest::validate()
{
    if (m_version.empty())
    {
        throw KMException("Not found 'version' in manifest file");
    }
    
    if (m_base.empty())
    {
        throw KMException("Not found 'base' in manifest file");
    }
    
    if (m_package.m_id.empty())
    {
        throw KMException("Not found 'package' -> 'id' in manifest file");
    }
    
    if (m_package.m_kind.empty())
    {
        throw KMException("Not found 'package' -> 'kind' in manifest file");
    }
    
    if (m_package.m_version.empty())
    {
        throw KMException("Not found 'package' -> 'version' in manifest file");
    }

    if (m_subpackages.empty() && m_package.m_kind == PACKGE_KIND_DEPEND && !m_modules.empty() && m_modules.find("devel") == m_modules.end())
    {
        throw KMException("If 'package' -> 'kind' == 'depend', then 'modules' must be devel: '.*'");
    }

    if (m_modules.empty())
    {
        if (m_package.m_kind == PACKGE_KIND_DEPEND)
        {
            m_modules["devel"] = ".*";
        }
        else
        {
            m_modules["binary"] = ".*";
        }
    }
}

void KMBuilderManifest::packageToYaml()
{
    YAML::Node package;
    package["id"] = m_package.m_id;
    package["name"] = m_package.m_name;
    package["kind"] = m_package.m_kind;
    package["version"] = m_package.m_version;
    if (!m_package.m_description.empty())
    {
        package["description"] = m_package.m_description;
    }
    if (!m_package.m_descriptions.empty())
    {
        KMBuilderUtils::mapToYaml(package, m_package.m_descriptions);
    }
    if (!m_package.m_names.empty())
    {
        KMBuilderUtils::mapToYaml(package, m_package.m_names);
    }
    m_rootNode["package"] = package;
}



void KMBuilderManifest::permissionFilesystemToYaml(YAML::Node &yamlNode)
{
    if (m_permission.m_filesystem.m_xdgUserDirs)
    {
        yamlNode["xdg-user-dirs"] = m_permission.m_filesystem.m_xdgUserDirs;
    }

    if (!m_permission.m_filesystem.m_filesystemDirs.empty())
    {
        KMBuilderUtils::vectorToYaml(yamlNode, "filesystem-dirs", m_permission.m_filesystem.m_filesystemDirs);
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
        yamlNode["system-server"] = m_permission.m_others.m_systemTime;
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
    YAML::Node permission;

    YAML::Node filesystem;
    permissionFilesystemToYaml(filesystem);
    if (!filesystem.IsNull())
    {
        permission["filesystem"] = filesystem;
    }

    YAML::Node net;
    permissionNetToYaml(net);
    if (!net.IsNull())
    {
        permission["net"] = net;
    }

    YAML::Node device;
    permissionDeviceToYaml(device);
    if (!device.IsNull())
    {
        permission["device"] = device;
    }

    YAML::Node processManager;
    permissionProcessManagerToYaml(processManager);
    if (!processManager.IsNull())
    {
        permission["processManager"] = processManager;
    }

    YAML::Node others;
    permissionOthersToYaml(others);
    if (!others.IsNull())
    {
        permission["others"] = others;
    }

    if (!permission.IsNull())
    {
        m_rootNode["permission"] = permission;
    }
}

void KMBuilderManifest::annotationToYaml()
{
    YAML::Node annotation;
    if (m_annotations.kylin_privilege)
    {
        annotation["kylin.privilege"] = m_annotations.kylin_privilege;
    }
    if (m_annotations.kylin_sysapp)
    {
        annotation["kylin.sysapp"] = m_annotations.kylin_sysapp;
    }
    if (!m_annotations.kylin_depend_apps.empty())
    {
        KMBuilderUtils::vectorToYaml(annotation, "kylin.depend-apps",m_annotations.kylin_depend_apps );
    }
    if (!annotation.IsNull())
    {
        m_rootNode["annotation"] = annotation;
    }
}

void KMBuilderManifest::buildOptionsToYaml(YAML::Node &yamlNode, const std::shared_ptr<KMBuildOptions> buildOptions)
{
    if (!buildOptions->m_cFlags.empty())
    {
        yamlNode["cflags"] = buildOptions->m_cFlags;
    }
    if (!buildOptions->m_cppFlags.empty())
    {
        yamlNode["cppflags"] = buildOptions->m_cppFlags;
    }
    if (!buildOptions->m_cxxFlags.empty())
    {
        yamlNode["cxxflags"] = buildOptions->m_cxxFlags;
    }
    if (!buildOptions->m_ldFlags.empty())
    {
        yamlNode["ldflags"] = buildOptions->m_ldFlags;
    }
    if (!buildOptions->m_appendPath.empty())
    {
        yamlNode["append-path"] = buildOptions->m_appendPath;
    }
    if (!buildOptions->m_prependPath.empty())
    {
        yamlNode["prepend-path"] = buildOptions->m_prependPath;
    }
    if (!buildOptions->m_appendLdLibraryPath.empty())
    {
        yamlNode["append-ld-library-path"] = buildOptions->m_appendLdLibraryPath;
    }
    if (!buildOptions->m_prependLdLibraryPath.empty())
    {
        yamlNode["prepend-ld-library-path"] = buildOptions->m_prependLdLibraryPath;
    }
    if (!buildOptions->m_appendPkgConfigPath.empty())
    {
        yamlNode["append-pkg-config-path"] = buildOptions->m_appendPkgConfigPath;
    }
    if (!buildOptions->m_prependPkgConfigPath.empty())
    {
        yamlNode["prepend-pkg-config-path"] = buildOptions->m_prependPkgConfigPath;
    }
    if (!buildOptions->m_env.empty())
    {
        KMBuilderUtils::mapToYaml(yamlNode, buildOptions->m_env);
    }
    if (!buildOptions->m_makeArgs.empty())
    {
        KMBuilderUtils::vectorToYaml(yamlNode, "make-args", buildOptions->m_makeArgs);
    }
    if (!buildOptions->m_makeInstallArgs.empty())
    {
        KMBuilderUtils::vectorToYaml(yamlNode, "make-install-args", buildOptions->m_makeInstallArgs);
    }
}

void KMBuilderManifest::componentToYaml(YAML::Node &yamlNode, const std::shared_ptr<KMBuilderModule> component)
{
    std::shared_ptr<BuilderModuleData> data = component->getData();
    yamlNode["name"] = data->m_name;
    yamlNode["buildsystem"] = data->m_buildSystem;
    if (!data->m_buildScript.empty())
    {
        yamlNode["build-script"] = data->m_buildScript;
    }
    if (!data->m_configOpts.empty())
    {
        KMBuilderUtils::vectorToYaml(yamlNode, "config-opts", data->m_configOpts);
    }

    YAML::Node buildOptions;
    buildOptionsToYaml(buildOptions, data->buildOptions);
    if (!buildOptions.IsNull())
    {
        yamlNode["build-options"] = buildOptions;
    }

    if (!data->m_preInstall.empty())
    {
        yamlNode["pre-install"] = data->m_preInstall;
    }

    if (!data->m_postInstall.empty())
    {
        yamlNode["post-install"] = data->m_postInstall;
    }

    if (!data->cleanup.empty())
    {
        KMBuilderUtils::vectorToYaml(yamlNode, "cleanup", data->cleanup);
    }

    if (!data->m_sources.empty())
    {
        YAML::Node sources = YAML::Node(YAML::NodeType::Sequence);
        for (auto &source: data->m_sources)
        {
            YAML::Node sourceNode;
            if (!source->getOnlyArches().empty())
            {
                KMBuilderUtils::vectorToYaml(sourceNode, "only-arches", source->getOnlyArches());
            }
            if (!source->getSkipArches().empty())
            {
                KMBuilderUtils::vectorToYaml(sourceNode, "skip-arches", source->getSkipArches());
            }
            if (!source->getCommand().empty())
            {
                sourceNode["command"] = source->getCommand();
            }
            source->toYaml(sourceNode);
            sources.push_back(sourceNode);
        }
        yamlNode["sources"] = sources;
    }
}

void KMBuilderManifest::toYaml(const std::string &path)
{
    m_rootNode["version"] = m_version;
    packageToYaml();
    if(!m_commands.empty())
    {
        KMBuilderUtils::vectorToYaml(m_rootNode,"command", m_commands);
    }
    if (!m_channel.empty())
    {
        m_rootNode["channel"] = m_channel;
    }
    permissionToYaml();
    if (!m_modules.empty())
    {
        YAML::Node modules;
        KMBuilderUtils::mapToYaml(modules, m_modules);
        m_rootNode["modules"] = modules;
    }

    if (!m_base.empty())
    {
        m_rootNode["base"] = m_base;
    }

    if (!m_runtime.empty())
    {
        m_rootNode["runtime"] = m_runtime;
    }

    if (!m_depends.empty())
    {
        KMBuilderUtils::vectorToYaml(m_rootNode, "depends", m_depends);
    }

    if (!m_cleanups.empty())
    {
        KMBuilderUtils::vectorToYaml(m_rootNode, "cleanup", m_cleanups);
    }

    if (!m_envs.empty())
    {
        YAML::Node envs;
        KMBuilderUtils::mapToYaml(envs, m_envs);
        m_rootNode["env"] = envs;
    }

    if (!m_license.empty())
    {
        m_rootNode["license"] = m_license;
    }

    if (!m_homepage.empty())
    {
        m_rootNode["homepage"] = m_homepage;
    }

    if (!maintainers.empty())
    {
        KMBuilderUtils::vectorToYaml(m_rootNode, "maintainer", maintainers);
    }

    if (!m_exportPlugins.empty())
    {
        KMBuilderUtils::vectorToYaml(m_rootNode, "export_plugins", m_exportPlugins);
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
        m_rootNode["components"] = components;
    }
    std::ofstream fout(path);
    fout << m_rootNode;
    fout.close();
}