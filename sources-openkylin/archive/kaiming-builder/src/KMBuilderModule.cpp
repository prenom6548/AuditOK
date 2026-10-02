/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderModule.h"

#include <iostream>

using namespace std;

class KMBuilderModule::Private
{
public:
    string m_name;
    BuildSystem m_buildSystem;

    vector<string> m_configOptions;
    vector<string> m_buildCommands;

    vector<shared_ptr<KMBuilderSource>> m_sources;
    vector<shared_ptr<KMBuilderModule>> m_components;

    std::shared_ptr<BuilderModuleData> m_data;

    map<modulePropertyID, any> m_propertyMap;
};

KMBuilderModule::KMBuilderModule()
    : m_ptr(make_shared<Private>())
{
    m_ptr->m_data = make_shared<BuilderModuleData>();
    m_ptr->m_data->buildOptions = make_shared<KMBuildOptions>();
}

KMBuilderModule::~KMBuilderModule()
{
}

std::shared_ptr<BuilderModuleData> KMBuilderModule::getData()
{
    return m_ptr->m_data;
}

void KMBuilderModule::setProperty(modulePropertyID propertyId, const std::any &value)
{
    m_ptr->m_propertyMap[propertyId] = value;

    switch (propertyId)
    {
    case Module_Name:
        m_ptr->m_data->m_name = std::any_cast<string>(value);
        break;
    case Module_BuildSystem:
        m_ptr->m_data->m_buildSystem = std::any_cast<string>(value);
        break;
    case Module_BuildCommands:
        m_ptr->m_data->m_buildScript = std::any_cast<string>(value);
        break;
    case Module_RmConfigure:
        m_ptr->m_data->m_rmConfigure = std::any_cast<bool>(value);
        break;
    case Module_Disable:
        m_ptr->m_data->m_disabled = std::any_cast<bool>(value);
        break;
    case Module_NoAutogen:
        m_ptr->m_data->m_noAutogen = std::any_cast<bool>(value);
        break;
    case Module_NoParallelMake:
        m_ptr->m_data->m_noParallelMake = std::any_cast<bool>(value);
        break;
    case Module_NoMakeInstall:
        m_ptr->m_data->m_noMakeInstall = std::any_cast<bool>(value);
        break;
    case Module_NoPythonTimestampFix:
        m_ptr->m_data->m_noPythonTimestampFix = std::any_cast<bool>(value);
        break;
    case Module_Cmake:
        m_ptr->m_data->m_cmake = std::any_cast<bool>(value);
        break;
    case Module_Builddir:
        m_ptr->m_data->m_buildDir = std::any_cast<bool>(value);
        break;
    case Module_Subdir:
        m_ptr->m_data->m_subdir = std::any_cast<string>(value);
        break;
    case Module_InstallRule:
        m_ptr->m_data->m_installRule = std::any_cast<string>(value);
        break;
    case Module_TestRule:
        m_ptr->m_data->m_testRule = std::any_cast<string>(value);
        break;
    case Module_RunTests:
        m_ptr->m_data->m_runTests = std::any_cast<bool>(value);
        break;
    default:
        break;
    }
}

std::any KMBuilderModule::getProperty(modulePropertyID propertyId)
{
    return m_ptr->m_propertyMap[propertyId];
}

void KMBuilderModule::setBuildSystem(const std::string &buildSystem)
{
    if (buildSystem == "")
    {
        // 如果是空默认是 autotools
        m_ptr->m_buildSystem = BS_Autotools;
    }
    else if (buildSystem == "qmake")
    {
        m_ptr->m_buildSystem = BS_Qmake;
    }
    else if (buildSystem == "cmake")
    {
        m_ptr->m_buildSystem = BS_Cmake;
    }
    else if (buildSystem == "meson")
    {
        m_ptr->m_buildSystem = BS_Meson;
    }
    else if (buildSystem == "autotools")
    {
        m_ptr->m_buildSystem = BS_Autotools;
    }
    else if (buildSystem == "cmake-ninja")
    {
        m_ptr->m_buildSystem = BS_Cmake_Ninja;
    }
    else if (buildSystem == "simple")
    {
        m_ptr->m_buildSystem = BS_Simple;
    }
    else
    {
        m_ptr->m_buildSystem = BS_Unknow;
    }
}

KMBuilderModule::BuildSystem KMBuilderModule::buildSystem()
{
    return m_ptr->m_buildSystem;
}

void KMBuilderModule::setConfigOptions(const vector<string> &configOptions)
{
    m_ptr->m_configOptions = configOptions;
    m_ptr->m_data->m_configOpts = configOptions;
}

const vector<string>& KMBuilderModule::configOptions()
{
    return m_ptr->m_configOptions;
}

void KMBuilderModule::setBuildOptions(const shared_ptr<KMBuildOptions> buildOptions)
{
    m_ptr->m_data->buildOptions = buildOptions;
}

shared_ptr<KMBuildOptions> KMBuilderModule::getBuildOptions()
{
    return m_ptr->m_data->buildOptions;
}

void KMBuilderModule::setTestCommands(const std::vector<std::string> &testCommands)
{
    m_ptr->m_data->m_testCommands = testCommands;
}

const std::vector<std::string> &KMBuilderModule::testCommands() const
{
    return m_ptr->m_data->m_testCommands;
}

void KMBuilderModule::setCleanup(const std::vector<std::string> &cleanup)
{
    m_ptr->m_data->cleanup = cleanup;
}

const std::vector<std::string> &KMBuilderModule::cleanup() const
{
    return m_ptr->m_data->cleanup;
}

void KMBuilderModule::setCleanupPlatform(const std::vector<std::string> &cleanupPlatform)
{
    m_ptr->m_data->cleanupPlatform = cleanupPlatform;
}

const std::vector<std::string> &KMBuilderModule::cleanupPlatform() const
{
    return m_ptr->m_data->cleanupPlatform;
}

void KMBuilderModule::setBuildScripts(const std::string &buildScripts)
{
    m_ptr->m_data->m_buildScript = buildScripts;
}

const std::string &KMBuilderModule::buildScripts() const
{
    return m_ptr->m_data->m_buildScript;
}

void KMBuilderModule::setPostInstalls(const std::string &postInstalls)
{
    m_ptr->m_data->m_postInstall = postInstalls;
}

const std::string &KMBuilderModule::postInstalls() const
{
    return m_ptr->m_data->m_postInstall;
}

void KMBuilderModule::setPreInstalls(const std::string &preInstalls)
{
    m_ptr->m_data->m_preInstall = preInstalls;
}

const std::string &KMBuilderModule::preInstalls() const
{
    return m_ptr->m_data->m_preInstall;
}

void KMBuilderModule::setChanges(const std::vector<std::string> &changes)
{
    m_ptr->m_data->m_changes = changes;
}

const std::vector<std::string> &KMBuilderModule::getChanges() const
{
    return m_ptr->m_data->m_changes;
}

void KMBuilderModule::addBuildSource(const shared_ptr<KMBuilderSource> buildSource)
{
    m_ptr->m_data->m_sources.push_back(buildSource);
}
void KMBuilderModule::setBuildSources(vector<shared_ptr<KMBuilderSource>> buildSources)
{
    m_ptr->m_data->m_sources = buildSources;
}
void KMBuilderModule::addBuildSources(vector<shared_ptr<KMBuilderSource>> buildSources)
{
    // 垃圾代码，这样写法会插入多次，请后来者引以为戒
    m_ptr->m_data->m_sources.insert(m_ptr->m_data->m_sources.end(), buildSources.begin(), buildSources.end());
}

void KMBuilderModule::setBuilderModules(const vector<shared_ptr<KMBuilderModule>> builderModules)
{
    m_ptr->m_data->m_components = builderModules;
}
const vector<shared_ptr<KMBuilderSource>> &KMBuilderModule::buildSources()
{
    return m_ptr->m_data->m_sources;
}

const vector<shared_ptr<KMBuilderModule>> &KMBuilderModule::builderModules()
{
    return m_ptr->m_data->m_components;
}
