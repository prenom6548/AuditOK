/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderRun.h"

#include "KMBuilderUtils.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/KMProcessUtils.h"
#include "common/kmtranslation.h"
#include "KMBuilderSource.h"
#include "KMBuilderSourcePatch.h"
#include "KMBuilderSourceShell.h"
#include "KMBuilderContext.h"
#include "KMBuilderBuild.h"

class KMBuilderRun::Options : public KMOption::Options
{

public:
    Options() = default;
    virtual ~Options() = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();

public:

    bool m_isHelp = false;
    bool m_isVerbose = false;
    bool m_isCcache = false;

    std::string m_module;
    std::string m_manifest;
    std::string m_command;
    std::string m_splitOfArgs;
    std::vector<std::string> m_commandArgs;
};


void KMBuilderRun::Options::preParseHook()
{
    addOptions();
}

void KMBuilderRun::Options::postParseHook()
{
    if(m_isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_splitOfArgs.empty())
    {
        if (!m_command.empty())
        {
            // kaiming-builder enter [OPTION…] MANIFEST [COMMAND [--] [ARGUMENT...]]
            int posDirectory = getPositionalOptionIndex("COMMAND");
            int steps = m_splitOfArgs.empty() ? 1 : 2;
            if (posDirectory > 0 && m_originalArgs.size() > posDirectory + steps)
            {
                for (int i = posDirectory + steps; i < m_originalArgs.size(); i++)
                {
                    m_commandArgs.push_back(m_originalArgs.at(i));
                }
            }
        }
    }

    if (m_command.empty())
    {
        m_command = "/bin/bash";
    }
}

void KMBuilderRun::Options::addOptions()
{
    setDescription(_("\nUsage\n \tkaiming-builder enter [OPTION…] MANIFEST [COMMAND [--] [ARGUMENT...]] \n"));
    //帮助选项
    addOption("help","h",KMOption::value<bool>(&m_isHelp),_("Show help options"));
    //应用选项
    addOption("verbose","v", KMOption::value(&m_isVerbose), _("Print debug information during command processing"));
    addOption("ccache","", KMOption::value(&m_isCcache), _("Use ccache"), true);
    addOption("component-name","c", KMOption::value<std::string>(&m_module), _("One of the components's name, which we want to debug"));

    //位置选项
    addPositionOption("MANIFEST", KMOption::value<std::string>(&m_manifest),1, _("The MANIFEST file for building, support .km.yml/.km.yaml"));
    addPositionOption("COMMAND", KMOption::value(&m_command), 1, _("Command to execute within the build environment"));
    addPositionOption("--", KMOption::value(&m_splitOfArgs), 1, _("The start flag of command's arguments"));
    addPositionOption("ARGUMENT", KMOption::value<std::vector<std::string>>(&m_commandArgs), -1, _("The arguments of command"));
}

class KMBuilderRun::Private
{
public:
    int m_argc;
    char **m_argv;
    
    std::unique_ptr<Options>m_kmOptions;
    // 构建清单配置
    std::shared_ptr<KMBuilderManifest> m_manifestConfig;

    // 构建过程中涉及的目录
    std::string m_configFile;
    fs::path m_baseDir;
    fs::path m_appDir;
    fs::path m_stateDir;
    fs::path m_downloadsDir;
    fs::path m_checksumDir;
    fs::path m_ccacheDir;
    fs::path m_buildDir;
};

REGISTER_SUBCOMMAND_DYNCREATE(enter, KMBuilderRun)

KMBuilderRun::KMBuilderRun()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuilderRun::~KMBuilderRun()
{
}

int KMBuilderRun::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMBuilderRun::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc,argv);

    if(d->m_kmOptions->m_isVerbose)
        km_logger.setLogLevel(trace);
    
    if(d->m_kmOptions->m_manifest.empty())
    {
        throw KMException("Must input MANIFEST!");
    }

    d->m_configFile = fs::absolute(d->m_kmOptions->m_manifest).string();
    d->m_baseDir = fs::absolute(fs::current_path());
    d->m_appDir = d->m_baseDir / "build";
    d->m_stateDir = d->m_baseDir / STATE_DIR;
    d->m_buildDir = d->m_stateDir / "sources";
    d->m_ccacheDir = d->m_stateDir / "ccache";

    km_logger.setLogPath(d->m_stateDir);
    
    //加载配置文件yaml/yml
    d->m_manifestConfig = KMBuilderManifest::loadFromYAML(d->m_configFile);
}


int KMBuilderRun::run()
{
    std::vector<std::string> args;
    if(!initArgs(args))
    {
        return EXIT_FAILURE;
    }

    std::vector<char*> exec_args;
    for (auto& arg : args)
    {
        exec_args.push_back(&arg[0]);
    }
    exec_args.push_back(nullptr);

    if (execvp(exec_args[0], exec_args.data()) == -1)
    {
        throw KMException("Unable to start kaiming build: " + std::string(strerror(errno)));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void KMBuilderRun::expandComponents(const std::vector<std::shared_ptr<KMBuilderModule>>components, std::vector<std::shared_ptr<KMBuilderModule>> &expand_components)
{
        for (auto component : components)
    {
        if (component->getData()->m_disabled)
        {
            continue;
        }

        std::vector<std::shared_ptr<KMBuilderModule>> incomponents = component->builderModules();
        if (!incomponents.empty())
        {
            expandComponents(incomponents, expand_components);
        }

        expand_components.push_back(component);
    }
}

void KMBuilderRun::downloadsFiles()
{
    std::map<std::shared_ptr<KMBuilderModule>, std::string> moudle_str_map;
    std::vector<std::shared_ptr<KMBuilderModule>> expand_components;

    //编造参数以获取downloadsDir
    int argc_false = 2;
    char * argv_false[] = 
    {
        const_cast<char*>("build"),
        const_cast<char*>(d->m_kmOptions->m_manifest.c_str()),
    };
    builderContext.parseCommandLine(argc_false,argv_false);

    expandComponents(d->m_manifestConfig->m_components, expand_components);
    for(auto component: expand_components)
    {
        std::string name = std::any_cast<std::string>(component->getProperty(KMBuilderModule::Module_Name));
        if (!createSubdir(d->m_buildDir, name, moudle_str_map[component]))
        {
            throw KMException("Failed to create build dir for component : " + name);
        }

        if (!download(component, moudle_str_map[component]))
        {
            throw KMException("Failed to download sources for component : " + name);
        }
        if (!extract(component,moudle_str_map[component]))
        {
            throw KMException("Failed to extract sources for component : " + name);
        }
    }
}

bool KMBuilderRun::extract(const shared_ptr<KMBuilderModule> &component, const string &sourcedir)
{
    for (auto source : component->buildSources())
    {
        if (source->type() == source->SRC_Archive || source->type() == source->SRC_Git)
        {
            bool unzip_status = source->unzipSource();
            if (unzip_status == false)
                return false;
        }
        else if (source->type() == KMBuilderSource::SRC_Patch)
        {
            static_cast<KMBuilderSourcePatch *>(source.get())->apply();
        }
        else if (source->type() == KMBuilderSource::SRC_Shell)
        {
            static_cast<KMBuilderSourceShell *>(source.get())->extractShell(component,sourcedir);
        }
    }
    return true;
}

void KMBuilderRun::buildInit()
{
    std::vector<std::string> args;
    args.push_back("build-init");
    if (builderContext.isVerbose())
    {
        args.push_back("-v");
    }
    args.push_back("--arch=" + KMBuilderUtils::getArch());
    args.push_back("--kind=" + d->m_manifestConfig->m_package.m_kind);
    args.push_back("--channel=" + d->m_manifestConfig->m_channel);
    args.push_back("--base=" + d->m_manifestConfig->m_base);
    args.push_back("--version="+ d->m_manifestConfig->m_package.m_version);

    if (!d->m_manifestConfig->m_depends.empty())
    {
        for (auto depend : d->m_manifestConfig->m_depends)
        {
            args.push_back("--depend=" + depend);
        }
    }

    if (!d->m_manifestConfig->m_runtime.empty())
    {
        args.push_back("--runtime=" + d->m_manifestConfig->m_runtime);
    }

    if (d->m_manifestConfig->m_annotations.kylin_privilege)
    {
        args.push_back("--kylin-privilege");
    }

    if (d->m_manifestConfig->m_annotations.kylin_sysapp)
    {
        args.push_back("--kylin-sysapp");
    }

    args.push_back(d->m_appDir);
    args.push_back(d->m_manifestConfig->m_package.m_id);

    if (!KMProcessUtils::spawn("kaiming", args, false))
    {
        throw KMException("kaiming build-init failed.");
    }
}

bool KMBuilderRun::initArgs(std::vector<std::string> &args)
{
    if (!fs::exists(d->m_appDir))
    {
        downloadsFiles();
        buildInit();
    }

    // 设置构建命令参数
    args.push_back("kaiming");
    args.push_back("build");
    args.push_back("--debug");

    // 添加构建路径绑定
    std::string moduleName;
    if (!d->m_kmOptions->m_module.empty())
    {
        moduleName = "/" + d->m_kmOptions->m_module;
    }
    std::string buildDirPath = getBuildDir() + moduleName;
    args.push_back("--bind-mount=/run/build" + moduleName + "=" + buildDirPath);
    args.push_back("--build-dir=/run/build" + moduleName);

    if(d->m_kmOptions->m_isCcache)
    {
        std::string ccacheDirPath = getCcacheDir();
        args.push_back("--bind-mount=/run/ccache=" + ccacheDirPath);
    }

    // 加入全局环境变量到args中
    std::vector<std::string> envs;
    getBuildEnv(envs);
    for (const auto& envVar : envs)
    {
        args.push_back("--env=" + envVar);
    }
    
    // 设置具体模块环境变量
    std::vector<std::string> buildOptions;
    const auto& targetModuleName = d->m_kmOptions->m_module;
    for (auto module : d->m_manifestConfig->m_components)
    {
        if ( targetModuleName == std::any_cast<std::string>(module->getProperty(KMBuilderModule::Module_Name)))
        {
            getModuleBuildOptions(module->getBuildOptions(), buildOptions);
        }
    }

    for (auto e : buildOptions)
    {
        args.push_back("--env=" + e);
    }

    // 将应用程序目录和命令行参数添加到命令中  
    args.push_back(getAppDir());

    // 收集配置文件后COMMAND并加入到 args 中
    args.push_back(d->m_kmOptions->m_command);
    
    if (d->m_kmOptions->m_commandArgs.size() > 0)
    {
        args.push_back("--");
    }
    for (const auto& commandArg : d->m_kmOptions->m_commandArgs)
    {
        args.push_back(commandArg);
    }

    //打印
    std::string cmdline;
    for (auto &arg : args)
    {
        cmdline += arg;
        
        cmdline += " ";
    }
    KMDebug(cmdline);
    
    return true;
}

void KMBuilderRun::getModuleBuildOptions(const std::shared_ptr<KMBuildOptions> &options, std::vector<std::string> &buildOptions)
{
    if (options)
    {
        //buildOption中环境变量
        std::map<std::string, std::string> env = options->m_env;

        if (!options->m_cFlags.empty())
        {
            buildOptions.emplace_back("CFLAGS=" + options->m_cFlags);
        }
        if (!options->m_cppFlags.empty())
        {
            buildOptions.emplace_back("CPPFLAGS=" + options->m_cppFlags);
        }
        if (!options->m_cxxFlags.empty())
        {
            buildOptions.emplace_back("CXXFLAGS=" + options->m_cxxFlags);
        }
        if (!options->m_ldFlags.empty())
        {
            buildOptions.emplace_back("LDFLAGS=" + options->m_ldFlags);
        }

        KMBuildOptions::concatEnvironmentPath(env, "PATH", options->m_prependPath, options->m_appendPath);
        KMBuildOptions::concatEnvironmentPath(env, "LD_LIBRARY_PATH", options->m_prependLdLibraryPath, options->m_appendLdLibraryPath);
        KMBuildOptions::concatEnvironmentPath(env, "PKG_CONFIG_PATH", options->m_prependPkgConfigPath, options->m_appendPkgConfigPath);

        for (auto [k, v] : env)
        {
            buildOptions.emplace_back(k + "=" + v);
        }
    }
}

void KMBuilderRun::getBuildEnv(std::vector<std::string> &envs)
{
    //添加全局环境变量
    std::map<std::string, std::string> envs_all = d->m_manifestConfig->m_envs;

    std::string ccachedir = "/run/ccache/disabled";
    if(d->m_kmOptions->m_isCcache)
    {
        ccachedir = "/run/ccache";
        std::string old = envs_all["PATH"];
        if(old.empty())
        {
            old = "$PATH";
        }

        std::string path = std::string("/run/ccache/bin:") + old;
        envs_all["PATH"] = path;
    }
    envs.push_back("CCACHE_DIR=" + ccachedir);

    for(auto [k, v] : envs_all)
    {
        envs.emplace_back(k + "=" + v);
    }
}

std::string KMBuilderRun::getCcacheDir() { return d->m_ccacheDir.string(); }
std::string KMBuilderRun::getBuildDir() { return d->m_buildDir.string(); }
std::string KMBuilderRun::getAppDir() { return d->m_appDir.string(); }
