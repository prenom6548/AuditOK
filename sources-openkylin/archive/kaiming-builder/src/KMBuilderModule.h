/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_MODULE_H__
#define __BUILDER_MODULE_H__

#include <any>
#include <memory>

#include "KMBuilderSource.h"
#include "KMBuildOptions.h"


class KMBuilderModule;

typedef struct
{
    std::string m_jsonPath;
    std::string m_name;
    std::string m_subdir;
    std::string m_postInstall;
    std::string m_preInstall;
    std::vector<std::string> m_configOpts;
    std::vector<std::string> m_secretOpts;
    std::vector<std::string> m_secretEnv;
    std::vector<std::string> m_makeArgs;
    std::vector<std::string> m_makeInstallArgs;
    std::string m_installRule;
    std::string m_testRule;
    std::string m_buildSystem;
    std::vector<std::string> m_ensureWritable;
    std::vector<std::string> m_onlyArches;
    std::vector<std::string> m_skipArches;
    bool m_disabled = false;
    bool m_rmConfigure = false;
    bool m_noAutogen = false;
    bool m_noParallelMake = false;
    bool m_noMakeInstall = false;
    bool m_noPythonTimestampFix = false;
    bool m_cmake = false;
    bool m_buildDir = false;
    bool m_runTests = false;
    std::vector<std::string> m_changes;
    std::vector<std::string> cleanup;
    std::vector<std::string> cleanupPlatform;
    std::vector<std::shared_ptr<KMBuilderSource>> m_sources;
    std::vector<std::shared_ptr<KMBuilderModule>> m_components;
    std::shared_ptr<KMBuildOptions> buildOptions;
    std::string m_buildScript;
    std::vector<std::string> m_testCommands;
} BuilderModuleData;

class KMBuilderModule
{
public:
    enum BuildSystem
    {
        BS_Unknow,
        BS_Qmake,
        BS_Cmake,
        BS_Meson,
        BS_Autotools,
        BS_Cmake_Ninja,
        BS_Simple
    };

    enum modulePropertyID
    {
        Module_Name,
        Module_Subdir,
        Module_RmConfigure,
        Module_Disable,
        Module_NoAutogen,
        Module_NoParallelMake,
        Module_NoMakeInstall,
        Module_NoPythonTimestampFix,
        Module_Cmake,
        Module_BuildSystem,
        Module_InstallRule,
        Module_TestRule,
        Module_Builddir,
        Module_Sources,
        Module_ConfigOpts,
        Module_SecretOpts,
        Module_SecretEnv,
        Module_MakeArgs,
        Module_MakeInstallArgs,
        Module_EnsureWritable,
        Module_OnlyArches,
        Module_SkipArches,
        Module_PostInstall,
        Module_BuildOptions,
        Module_Cleanup,
        Module_CleanupPlatform,
        Module_Modules,
        Module_BuildCommands,
        Module_TestCommands,
        Module_RunTests
    };

    KMBuilderModule();
    ~KMBuilderModule();

    std::shared_ptr<BuilderModuleData> getData();

    void setProperty(modulePropertyID propertyId, const std::any &value);
    std::any getProperty(modulePropertyID propertyId);

    void setBuildSystem(const std::string &buildSystem);
    BuildSystem buildSystem();

    void setConfigOptions(const vector<string> &configOptions);
    const vector<string>& configOptions();

    void setBuildOptions(const shared_ptr<KMBuildOptions> buildOptions);
    shared_ptr<KMBuildOptions> getBuildOptions();

    void setTestCommands(const std::vector<std::string> &testCommands);
    const std::vector<std::string> &testCommands() const;

    void setCleanup(const std::vector<std::string> &cleanup);
    const std::vector<std::string> &cleanup() const;

    void setCleanupPlatform(const std::vector<std::string> &cleanupPlatform);
    const std::vector<std::string> &cleanupPlatform() const;

    void setChanges(const std::vector<std::string> &changes);
    const std::vector<std::string> &getChanges() const;

    void addBuildSource(const shared_ptr<KMBuilderSource> buildSource);
    void setBuildSources(vector<shared_ptr<KMBuilderSource>> buildSources);
    void addBuildSources(vector<shared_ptr<KMBuilderSource>> buildSources);
    const vector<shared_ptr<KMBuilderSource>> &buildSources();

    void setBuilderModules(const vector<shared_ptr<KMBuilderModule>> builderModules);
    const vector<shared_ptr<KMBuilderModule>> &builderModules();

    void setBuildScripts(const std::string &buildScripts);
    const std::string &buildScripts() const;

    void setPostInstalls(const std::string &postInstalls);
    const std::string &postInstalls() const;

    void setPreInstalls(const std::string &preInstalls);
    const std::string &preInstalls() const;

private:
    class Private;
    std::shared_ptr<Private> m_ptr;
};

#endif // !__BUILDER_MODULE_H__
