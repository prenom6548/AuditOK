/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderBuild.h"
#include "KMBuilderContext.h"
#include "KMBuilderSourcePatch.h"
#include "KMBuilderSourceShell.h"
#include "common/KMException.h"
#include "common/KMLogger.h"

#define BUILD_POST_HOOK "/opt/kaiming-tools/bin/build_post_hook.sh"

/**
 * @brief: 模块构建
 * @param: 缓存
 * @param: 模块module
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::doBuild(const shared_ptr<KMBuilderModule> &pBuildModule)
{
    return buildHelper(pBuildModule, m_subdir);
}

/**
 * @brief: 构建过程
 * @param: pBuilderCache缓存结构
 * @param: sourcedir子目录全路径名
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::buildHelper(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir)
{
    KMTrace("KMBuilderBuild::buildHelper invoke begin");

    // 解压，代码提取
    if (!extract(pBuildModule, sourcedir))
    {
        return false;
    }

    string relativeSourceSubdir;
    string sourceSubdir = sourcedir;
    if (!pBuildModule->getData()->m_subdir.empty())
    {
        sourceSubdir = sourceSubdir + "/" + pBuildModule->getData()->m_subdir;
        relativeSourceSubdir = pBuildModule->getData()->m_subdir;
    }

    int type = pBuildModule->buildSystem();
    if (type == KMBuilderModule::BS_Unknow)
    {
        return false;
    }

    KMInfo("========================================================================");
    KMInfo("Building component " + pBuildModule->getData()->m_name + " in " + sourceSubdir);
    KMInfo("========================================================================");

    vector<string> buildArgs, env, secretEnv, confOpt, secretOpt;
    getBuildArgs(pBuildModule->getBuildOptions(), buildArgs);
    getBuildEnv(pBuildModule->getBuildOptions(), env);
    confOpt = pBuildModule->configOptions();
    getBuildConfigOpts(pBuildModule->getBuildOptions(), confOpt);
    getBuildSecretOpts(pBuildModule->getBuildOptions(), secretOpt);
    getBuildSecretEnv(pBuildModule->getBuildOptions(), secretEnv);

    int jobs = builderContext.getJobs();
    if (jobs <= 0)
        jobs = sysconf(_SC_NPROCESSORS_ONLN);

    if (pBuildModule->getData()->m_noParallelMake)
    {
        env.push_back("KAIMING_BUILDER_N_JOB=1");
    }
    else
    {
        env.push_back("KAIMING_BUILDER_N_JOB=" + to_string(jobs));
    }

    string config;
    if (!getConfigFilename(pBuildModule, type, sourcedir, config))
    {
        return false;
    }

    string appDir = builderContext.getAppDir();
    string name = pBuildModule->getData()->m_name;
    string ccacheDir = builderContext.getCcacheDir();
    vector<string> setupargs = setupBuildArgs(appDir, ccacheDir, name, sourcedir, relativeSourceSubdir, buildArgs, env);

    if (!pBuildModule->getData()->m_noAutogen && !genConfigFile(sourcedir, sourceSubdir, config, setupargs))
    {
        return false;
    }

    string buildDir = sourceSubdir;
    if (!config.empty() && fs::exists(config) && !genMakefile(pBuildModule, type, sourcedir, sourceSubdir, config, confOpt, secretOpt, buildDir, setupargs))
    {
        return false;
    }

    if (!isExistMakefile(buildDir, type))
    {
        return false;
    }

    vector<string> testargs;
    getTestArgs(pBuildModule->getBuildOptions(), testargs);
    vector<string> setuptestargs = setupBuildArgs(appDir, ccacheDir, name, sourcedir, relativeSourceSubdir, testargs, env);

    if (!makeAndInstall(pBuildModule, type, appDir, buildDir, secretEnv, setupargs, setuptestargs))
    {
        return false;
    }

    if (!builderContext.getIsKeepBuildDirs())
    {
        return cleanupBuildbdir(KMBuilderUtils::buildFilename(builderContext.getBuildDir(), name), sourceSubdir);
    }

    KMTrace("KMBuilderBuild::buildHelper invoke end");
    return true;
}

/**
 * @brief: 编译安装
 * @param: pBuildModule模块module
 * @param: type 编译类型
 * @param: buildDir 编译目录
 * @param: secEnv 加密环境变量
 * @param: setupArgs 构建参数
 * @param: setupTestArgs 测试参数
 * @return: true-成功，false-失败
 */
bool KMBuilderBuild::makeAndInstall(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &appDir, string &buildDir, const vector<string> &secEnv, const vector<string> &setupArgs, const vector<string> &setupTestArgs)
{
    string makeCmd, makej, makel, testarg;
    switch (type)
    {
        case KMBuilderModule::BS_Meson:
        case KMBuilderModule::BS_Cmake_Ninja:
        {
            makeCmd = "ninja";
            makej = "-j1";
            testarg = "test";
            break;
        }
        case KMBuilderModule::BS_Autotools:
        case KMBuilderModule::BS_Qmake:
        case KMBuilderModule::BS_Cmake:
        {
            makeCmd = "make";
            testarg = "check";
            break;
        }
        case KMBuilderModule::BS_Simple:
            break;

        default:
            break;
    }

    if (!pBuildModule->getData()->m_testRule.empty())
        testarg = pBuildModule->getData()->m_testRule;

    if (!pBuildModule->getData()->m_noParallelMake)
    {
        int jobs = builderContext.getJobs();
        if (jobs <= 0)
            jobs = sysconf(_SC_NPROCESSORS_ONLN);
        makej = string("-j") + to_string(jobs > 12 ? 12 : jobs);
        makel = string("-l") + to_string(jobs * 2);
    }

    if (!makeCmd.empty())
    {
        vector<any> args;
        vector<string> makeargs;
        getMakeArgs(pBuildModule->getBuildOptions(), pBuildModule->getData()->m_makeArgs, makeargs);
        args.push_back(makeCmd);
        args.push_back(string("--"));
        args.push_back(makej);
        args.push_back(makel);
        args.push_back(string("strv"));
        args.push_back(makeargs);

        if (!build(buildDir, setupArgs, args))
            return false;
    }

    if (!pBuildModule->getData()->m_buildScript.empty())
    {
        std::string scriptFile = KMBuilderUtils::buildFilename(buildDir, "kaiming_buildScript.sh");
        if (!KMBuilderUtils::writeScriptToFile(pBuildModule->getData()->m_buildScript, scriptFile))
        {
            throw KMException("Fail to create script file: " + scriptFile +
                              "for component: " + pBuildModule->getData()->m_name);
        }

        vector<any> args = {std::string("./kaiming_buildScript.sh")};

        if (!build(buildDir, setupArgs, args))
            return false;
    }

    if (!pBuildModule->getData()->m_preInstall.empty())
    {
        std::string scriptFile = KMBuilderUtils::buildFilename(buildDir, "kaiming_preInstall.sh");
        if (!KMBuilderUtils::writeScriptToFile(pBuildModule->getData()->m_preInstall, scriptFile))
        {
            throw KMException("Fail to create script file: " + scriptFile +
                              "for component: " + pBuildModule->getData()->m_name);
        }

        vector<any> args = {std::string("./kaiming_preInstall.sh")};

        if (!build(buildDir, setupArgs, args))
            return false;
    }

    if (!pBuildModule->getData()->m_noMakeInstall && makeCmd.length() > 0)
    {
        vector<any> args;
        args.push_back(makeCmd);
        args.push_back(string("--"));
        if (pBuildModule->getData()->m_installRule.empty())
            args.push_back(string("install"));
        else
            args.push_back(pBuildModule->getData()->m_installRule);

        vector<string> installArgs;
        getInstallArgs(pBuildModule->getBuildOptions(), pBuildModule->getData()->m_makeInstallArgs, installArgs);
        args.push_back(string("strv"));
        args.push_back(installArgs);

        if (!build(buildDir, setupArgs, args))
            return false;
    }

    if (!pBuildModule->getData()->m_postInstall.empty())
    {
        std::string scriptFile = KMBuilderUtils::buildFilename(buildDir, "kaiming_postInstall.sh");
        if (!KMBuilderUtils::writeScriptToFile(pBuildModule->getData()->m_postInstall, scriptFile))
        {
            throw KMException("Fail to create script file: " + scriptFile +
                              "for component: " + pBuildModule->getData()->m_name);
        }

        vector<any> args = {std::string("./kaiming_postInstall.sh")};

        if (!build(buildDir, setupArgs, args))
            return false;
    }

    // test unit
    if (pBuildModule->getData()->m_runTests)
    {
        KMInfo("Running tests");

        if (!makeCmd.empty() && !testarg.empty())
        {
            vector<any> args;
            args.push_back(makeCmd);
            args.push_back(string("--"));
            args.push_back(testarg);

            if (!build(buildDir, setupTestArgs, args))
                return false;
        }

        for (const auto testcmd : pBuildModule->getData()->m_testCommands)
        {
            vector<any> args;
            args.push_back(string("/bin/bash"));
            args.push_back(string("--"));
            args.push_back(string("-c"));
            args.push_back(testcmd);

            if (!build(buildDir, setupTestArgs, args))
                return false;
        }
    }

    // 生成一个ld.so.cache
    // std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    // {
    //     vector<any> args;
    //     args.push_back(string("/bin/bash"));
    //     args.push_back(string("--"));
    //     args.push_back(string("-c"));
    //     if (manifest->m_annotations.kylin_sysapp)
    //     {
    //         args.push_back(string("ldconfig -C /etc/ld.so.cache"));
    //     }
    //     else
    //     {
    //         args.push_back(string("ldconfig -C ${APP_INSTALL_PREFIX}/etc/ld.so.cache"));
    //     }

    //     if (!build(buildDir, setupTestArgs, args))
    //     {
    //         return false;
    //     }
    // }

    // 构建完成后的钩子
    {
        std::string scriptFile = KMBuilderUtils::buildFilename(buildDir, "build_post_hook.sh");
        std::string hookFile(BUILD_POST_HOOK);
        const auto copyOptions = fs::copy_options::overwrite_existing;
        fs::copy_file(hookFile, scriptFile, copyOptions);

        vector<any> args = {std::string("./build_post_hook.sh")};

        if (!build(buildDir, setupArgs, args))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief: 获取编译选项前缀
 * @param: KMBuildOptions 构建选项
 * @return: 编译选项前缀
 */
string KMBuilderBuild::getOptionsPrefix(const shared_ptr<KMBuildOptions> &options)
{
    if (options && !options->m_prefix.empty())
    {
        return options->m_prefix;
    }

    // if (getBuildRuntime())
    // {
    //     // return "/usr";
    //     return "/app";
    // }

    // return "/app";

    // 使用overlay叠加挂载方式后，install阶段的prefix=/usr，不再是从flatpak参考来的/app或/usr了
    return "/usr";
}

/**
 * @brief: 获取编译选项libdir
 * @param: KMBuildOptions 构建选项
 * @return: 编译选项libdir
 */
string KMBuilderBuild::getOptionsLibdir(const shared_ptr<KMBuildOptions> &options)
{
    if (options && !options->m_libDir.empty())
    {
        return options->m_libDir;
    }

    if (getBuildRuntime())
    {
        // sdkconfig在新规范中已经没有了
        /*
        KMBuilderSdkConfig *psdkcongfig = builderContext.getBuilderSdkConfig();
        if (psdkcongfig == nullptr)
            return {};

        if (!psdkcongfig->m_libDir.empty())
            return psdkcongfig->m_libDir;
        */
    }

    return "";
}

/**
 * @brief: 获取是否编译运行时
 * @return: true-是, false-否
 */
bool KMBuilderBuild::getBuildRuntime()
{
    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    return manifest->m_package.m_kind == PACKGE_KIND_RUNTIME;
}

/**
 * @brief: 获取install参数
 * @param: KMBuildOptions 构建选项
 * @param: moduleInstallargs 模块install参数
 * @param: args 返回的构建参数
 */
void KMBuilderBuild::getInstallArgs(const shared_ptr<KMBuildOptions> &options, const std::vector<std::string> &moduleInstallargs, vector<string> &args)
{
    for (const auto arg : moduleInstallargs)
        args.push_back(arg);

    for (const auto arg : options->m_makeInstallArgs)
        args.push_back(arg);
}

/**
 * @brief: 获取make参数
 * @param: KMBuildOptions 构建选项
 * @param: moduleMakeargs 模块make参数
 * @param: args 返回的构建参数
 */
void KMBuilderBuild::getMakeArgs(const shared_ptr<KMBuildOptions> &options, const std::vector<std::string> &moduleMakeargs, vector<string> &args)
{
    for (const auto arg : moduleMakeargs)
        args.push_back(arg);

    for (const auto arg : options->m_makeArgs)
        args.push_back(arg);
}

/**
 * @brief: 获取test参数
 * @param: KMBuildOptions 构建选项
 * @param: args 返回的构建参数
 */
void KMBuilderBuild::getTestArgs(const shared_ptr<KMBuildOptions> &options, vector<string> &args)
{
    for (const auto testarg : options->m_testArgs)
        args.push_back(testarg);

    args.push_back("--readonly");
}

/**
 * @brief: 解压源码
 * @param: pBuildModule模块module
 * @param: 子目录全路径名
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::extract(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir)
{
    // TODO: 使用管道popen一次执行多个命令进行的源码提取，出现错误时，无法真正获取到错误状态，后续需要优化
    for (auto source : pBuildModule->buildSources())
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
            static_cast<KMBuilderSourceShell *>(source.get())->extractShell(pBuildModule,sourcedir);
        }
    }
    return true;
}

/**
 * @brief: makefile是否存在
 * @param: buildDir 编译目录
 * @param: type 编译类型
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::isExistMakefile(const string &buildDir, const int &type)
{
    switch (type)
    {
        case KMBuilderModule::BS_Meson:
        case KMBuilderModule::BS_Cmake_Ninja:
        {
            if (fs::exists(buildDir + "/build.ninja"))
            {
                return true;
            }
            
            throw KMException("Can't find build.ninja file in " + buildDir);
        }
        case KMBuilderModule::BS_Autotools:
        case KMBuilderModule::BS_Cmake:
        case KMBuilderModule::BS_Qmake:
        {
            vector<string> makefiles = { "Makefile", "makefile", "GNUmakefile" };
            for (auto makefile : makefiles)
            {
                if (fs::exists(buildDir + "/" + makefile))
                {
                    return true;
                }

                throw KMException("Can't find makefile in " + buildDir);
            }

            break;
        }
        default:
            return true;
    }

    return false;
}

/**
 * @brief : 判断configOpts中是否包含了选项option。
 * @param : [in] configOpts, 选项列表
 * @param : [in] option, 选项的KEY
 * @return: true-包含; false-不包含
 * @note  : 使用场景：
 *              1. 构建流程中的配置环节的prefix和libdir等是否配置
 */ 
bool KMBuilderBuild::containsOption(const std::vector<std::string>& configOpts, const std::string& option)
{
    for (const std::string& item : configOpts)
    {
        if (KMBuilderUtils::contains(item, option))
        {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief: 生成makefile
 * @param: pBuildModule模块module
 * @param: sourceDir 源码目录全路径名
 * @param: sourceSubdir 子目录全路径名
 * @param: config 配置文件全路径名
 * @param: setupArgs 构建参数
 * @param: confopt 配置参数
 * @param: secopt 安全参数
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::genMakefile(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &sourceDir, const string &sourceSubdir, const string &config, const vector<string> &confopt, const vector<string> &secopt, string &buildDir, vector<string> &setupArgs)
{
    if (config.empty())
        return true;
    if (!fs::exists(config))
        return true;

    bool isUseBuildir = (type == KMBuilderModule::BS_Meson);

    string configCmd, configArg, cmakeGenerator;
    if (!isUseBuildir)
    {
        buildDir = sourceSubdir;
        if (type == KMBuilderModule::BS_Cmake || type == KMBuilderModule::BS_Cmake_Ninja)
        {
            configCmd = "cmake";
            configArg = ".";
        }
        else if (type == KMBuilderModule::BS_Qmake)
        {
            configCmd = "qmake";
            configArg = fs::path(config).filename();
        }
        else
        {
            if (type == KMBuilderModule::BS_Meson)
            {
                assert(type != KMBuilderModule::BS_Meson);
            }
            configCmd = "./configure";
        }
    }
    else
    {
        buildDir = sourceSubdir + "/_kaiming_build";
        if (!fs::exists(buildDir) && !fs::create_directories(buildDir))
        {
            throw KMException("Failed to mkdir : " + buildDir);
        }

        if (!buildDir.empty())
        {
            for (size_t i = 0; i < setupArgs.size(); ++i)
            {
                if (setupArgs.at(i).compare(0, strlen("--build-dir="), "--build-dir=") == 0)
                {
                    string builddir = setupArgs.at(i) + string("/_kaiming_build");
                    setupArgs.at(i) = builddir;
                    break;
                }
            }
        }

        if (type == KMBuilderModule::BS_Cmake || type == KMBuilderModule::BS_Cmake_Ninja)
        {
            configCmd = "cmake";
            configArg = "..";
        }
        else if (type == KMBuilderModule::BS_Qmake)
        {
            configCmd = "qmake";
            configArg = string("../") + fs::path(config).filename().string();
        }
        else if (type == KMBuilderModule::BS_Meson)
        {
            configCmd = "meson";
            configArg = "..";
        }
        else
        {
            configCmd = "../configure";
        }
    }

    if (type == KMBuilderModule::BS_Cmake)
    {
        cmakeGenerator = "Unix Makefiles";
    }
    else if (type == KMBuilderModule::BS_Cmake_Ninja)
    {
        cmakeGenerator = "Ninja";
    }

    // string prefix = getOptionsPrefix(pBuildModule->getBuildOptions());
    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    string prefix = "/opt/apps/" + manifest->m_package.m_id + "/files";
    const std::vector<std::string>& configOpts = pBuildModule->configOptions();
    if (manifest->m_annotations.kylin_sysapp)
    {
        prefix = "/usr";
    }

    vector<string> configureArgs;
    if (type == KMBuilderModule::BS_Cmake || type == KMBuilderModule::BS_Cmake_Ninja)
    {
        bool hasPrefix = containsOption(configOpts, "CMAKE_INSTALL_PREFIX:PATH=");
        if (!hasPrefix)
        {
            configureArgs.push_back("-DCMAKE_INSTALL_PREFIX:PATH=" + prefix);
        }

        bool hasLibdir = containsOption(configOpts, "CMAKE_INSTALL_LIBDIR:PATH=");
        if (!hasLibdir)
        {
            configureArgs.push_back("-DCMAKE_INSTALL_LIBDIR:PATH=" + prefix + "/lib");
        }
        
        configureArgs.push_back("-G " + cmakeGenerator);
    }
    else if (type == KMBuilderModule::BS_Qmake)
    {
        bool hasPrefix = containsOption(configOpts, "PREFIX=");
        if (!hasPrefix)
        {
            configureArgs.push_back("PREFIX=" + prefix);
        }
    }
    else
    {
        // autotools and meson 
        bool hasPrefix = containsOption(configOpts, "--prefix=");
        if (!hasPrefix)
        {
            configureArgs.push_back("--prefix=" + prefix);
        }

        bool hasLibdir = containsOption(configOpts, "--libdir=");
        if (!hasLibdir)
        {
            configureArgs.push_back("--libdir=" + prefix + "/lib");
        }
    }

    configureArgs.push_back(configArg);

    vector<any> args;
    args.push_back(configCmd);
    args.push_back(string("--"));
    args.push_back(string("strv"));
    args.push_back(configureArgs);
    args.push_back(string("strv"));
    args.push_back(confopt);
    args.push_back(string("secret"));
    args.push_back(secopt);

    if (!build(buildDir, setupArgs, args))
    {
        throw KMException("Failed to run : " + configCmd + " " + configArg);
    }

    return true;
}

/**
 * @brief: 构建
 * @param: setupArgs 构建参数
 * @param: args 构建参数
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::build(const string &cwdSubdir, const vector<string> &setupArgs, const vector<any> &args)
{
    const string skip = "skip";
    const string strv = "strv";
    const string secret = "secret";
    const string secretenv = "secret_env";
    vector<string> setupargs = setupArgs;
    for (int i = 0; i < args.size(); ++i)
    {
        string strArg;
        std::any arg = args[i];
        if (arg.type() == typeid(string))
            strArg = any_cast<string>(arg);
        if (strArg != secretenv)
            continue;
        arg = args[++i];
        if (arg.type() == typeid(vector<string>))
        {
            vector<string> argvs = any_cast<vector<string>>(arg);
            for (auto &argv : argvs)
            {
                const char *value = getenv(argv.c_str());
                if (value != nullptr)
                    setupargs.push_back("--env=" + argv + "=" + value);
            }
        }
    }

    for (int i = 0; i < args.size(); ++i)
    {
        string strArg;
        std::any arg = args[i];
        if (arg.type() == typeid(string))
            strArg = any_cast<string>(arg);
        else
            continue;
        if (strArg == strv)
        {
            arg = args[++i];
            if (arg.type() == typeid(vector<string>))
            {
                vector<string> argvs = any_cast<vector<string>>(arg);
                for (auto &argv : argvs)
                    setupargs.push_back(argv);
            }
        }
        else if (strArg == secret)
        {
            arg = args[++i];
            if (arg.type() == typeid(vector<string>))
            {
                vector<string> argvs = any_cast<vector<string>>(arg);
                for (auto &argv : argvs)
                {
                    int pos = argv.find("$");
                    if (pos == string::npos)
                        continue;
                    string key = argv.substr(0, pos-1);
                    string value = getenv(key.c_str());
                    if (!value.empty())
                        setupargs.push_back(key + "=" + value);
                }
            }
        }
        else if (strArg == secretenv)
            continue;
        else if (strArg != skip)
            setupargs.push_back(strArg);
    }

    return KMBuilderUtils::spawn(cwdSubdir, setupargs);
}

/**
 * @brief: 构建编译参数
 * @param: appDir 应用目录全路径名
 * @param: cacheDir 缓存目录全路径名
 * @param: name 模块名称
 * @param: sourceDir 源码目录全路径名
 * @param: cwdSubdir 子目录全路径名
 * @param: opts 编译参数
 * @param: env 环境变量
 * @return: true-成功; false-失败
 */
vector<string> KMBuilderBuild::setupBuildArgs(const string &appDir, const string &ccacheDir, const string &name, const string &sourceDir, const string &cwdSubdir, const vector<string> &opts, const vector<string> &env)
{
    vector<string> args;
    args.push_back("kaiming");
    args.push_back("build");
    if (builderContext.isVerbose())
    {
        args.push_back("-v");
    }

    string buildDir = "/run/build/";
    // if (getBuildRuntime())
    // {
    //     buildDir = "/run/build-runtime/";
    // }

    // 这种环境变量可在shell脚本中使用
    args.push_back("--env=KAIMING_BUILDER_BUILDDIR=" + buildDir + name);
    // TODO：权限设置的需要后面确定后才能确定替换方案
    // args.push_back("--nofilesystem=host:reset");
    string realpathDir = realpath(sourceDir.c_str(), nullptr);
    // args.push_back("--filesystem=" + realpathDir);

    if (realpathDir != sourceDir)
    {
        args.push_back("--bind-mount=" + sourceDir + "=" + realpathDir);
    }

    args.push_back("--bind-mount=" + buildDir + name + "=" + realpathDir);

    if (!cwdSubdir.empty())
    {
        args.push_back("--build-dir=" + buildDir + name + "/" + cwdSubdir);
    }
    else
    {
        args.push_back("--build-dir=" + buildDir + name);
    }

    args.push_back("--bind-mount=/run/ccache=" + ccacheDir);

    for (auto opt : opts)
    {
        args.push_back(opt);
    }

    for (auto e : env)
    {
        args.push_back("--env=" + e);
    }

    args.push_back(appDir);

    return args;
}

/**
 * @brief: 生成配置文件
 * @param: sourceDir 源码目录全路径名
 * @param: sourceSubdir 子目录全路径名
 * @param: config 配置文件全路径名
 * @param: setupArgs 构建参数
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::genConfigFile(const string &sourceDir, const string &sourceSubdir, const string &config, vector<string> setupArgs)
{
    if (config.empty())
        return true;
    if (fs::exists(config))
        return true;

    const vector<string> names = { "autogen", "autogen.sh", "bootstrap", "bootstrap.sh" };
    string autogen;
    for (auto name : names)
    {
        if (fs::exists(sourceDir + "/" + name))
        {
            autogen = "./" + name;
            break;
        }
    }

    if (autogen.empty())
    {
        throw KMException("Can't find autogen, autogen.sh, bootstrap.sh or bootstrap");
    }

    // TODO: 容器kaiming-box换成crun，环境变量NOCONFIGURE应已无用
    setupArgs.push_back("--env=NOCONFIGURE=1");

    vector<any> args;
    args.push_back(autogen);

    if (!build(sourceSubdir, setupArgs, args))
        return false;

    if (!fs::exists(config))
    {
        throw KMException("autogen did not create configure");
    }

    return true;
}

/**
 * @brief: 获取配置文件
 * @param: type 构建系统类型
 * @param: sourceSubdir 子目录全路径名
 * @param: config 配置文件全路径名
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::getConfigFilename(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &sourceSubdir, string &config)
{
    switch (type)
    {
        case KMBuilderModule::BS_Unknow:
        {
            throw KMException("component : " + pBuildModule->getData()->m_name + ": Invalid buildsystem");
        }
        case KMBuilderModule::BS_Simple:
        {
            if (pBuildModule->getData()->m_buildScript.empty())
            {
                throw KMException("component : " + pBuildModule->getData()->m_name + ": Buildsystem simple requires specifying \"build-script\"");
            }
            break;
        }
        case KMBuilderModule::BS_Cmake:
        case KMBuilderModule::BS_Cmake_Ninja:
        {
            if (!fs::exists(sourceSubdir + "/CMakeLists.txt"))
            {
                throw KMException("component : " + pBuildModule->getData()->m_name + ": Can't find CMakeLists.txt");
            }
            config = sourceSubdir + "/CMakeLists.txt";
            break;
        }
        case KMBuilderModule::BS_Meson:
        {
            if (!fs::exists(sourceSubdir + "/meson.build"))
            {
                throw KMException("component : " + pBuildModule->getData()->m_name + ": Can't find meson.build");
            }
            config = sourceSubdir + "/meson.build";
            break;
        }
        case KMBuilderModule::BS_Qmake:
        {
            for (auto const &entry : fs::directory_iterator{ sourceSubdir })
            {
                if (!fs::is_regular_file(entry))
                    continue;

                fs::path p = entry.path();
                if (p.extension().string() != ".pro")
                    continue;

                config = p.string();
                break;
            }
            if (config.empty())
            {
                throw KMException("component : " + pBuildModule->getData()->m_name + ": Can't find *.pro file");
            }
            break;
        }
        case KMBuilderModule::BS_Autotools:
        {
            config = sourceSubdir + "/configure";
            if (!pBuildModule->getData()->m_rmConfigure)
                break;

            if (fs::exists(config) && !fs::remove(config))
                return false;

            break;
        }
        default:
            return true;
    }

    return true;
}

/**
 * @brief: 获取配置选项
 * @param: BuildOptions编译选项
 * @param: 返回的配置选项
 */
void KMBuilderBuild::getBuildConfigOpts(const shared_ptr<KMBuildOptions> &options, vector<string> &opts)
{
    if (!options)
    {
        return;
    }

    for (auto opt : options->m_configOpts)
    {
        opts.push_back(opt);
    }
}

/**
 * @brief: 获取秘密选项
 * @param: mapEnv环境变量
 * @param: 返回的配置选项
 */
void KMBuilderBuild::getBuildSecretOpts(const shared_ptr<KMBuildOptions> &options, vector<string> &opts)
{
    if (!options)
    {
        return;
    }

    for (auto opt : options->m_secretOpts)
    {
        opts.push_back(opt);
    }
}

/**
 * @brief: 获取编译参数
 * @param: options
 * @param: args编译参数
 */
void KMBuilderBuild::getBuildArgs(const shared_ptr<KMBuildOptions> &options, vector<string> &args)
{
    if (!options)
    {
        return;
    }

    for (const auto &arg : options->m_buildArgs)
    {
        args.push_back(arg);
    }

    // if (builderContext.getIsSandbox())
    // {
    //     if (!args.empty())
    //         args.clear();
    //     else
    //         args.push_back("--unshare=network");
    // }
}

/**
 * @brief : 处理环境变量中路径变量的前面插入和后面追加
 * @param : [in | out] envs，环境变量map
 * @param : [in] key, 路径类变量的key，如：PATH
 * @param : [in] prepend, 前面追加
 * @param : [in] append, 后面追加
 */ 
static void concatEnvironmentPath(std::map<std::string, std::string> &envs, const std::string &key, const std::string &prepend, const std::string &append)
{
    std::string value = envs[key];
    if (value.empty())
    {
        value = "$" + key;
    }
    
    if (!append.empty())
    {
        value.append(":" + append);
    }
    
    if (!prepend.empty())
    {
        value = prepend + ":" + value;
    }

    if (value != ("$" + key))
    {
        envs[key] = value;
    }
    else
    {
        envs.erase(key);
    }
}

/**
 * @brief: 获取环境变量
 * @param: KMBuildOptions
 * @param: env编译环境变量
 */
void KMBuilderBuild::getBuildEnv(const std::shared_ptr<KMBuildOptions> &options, std::vector<std::string> &env)
{
    std::map<std::string, std::string> envs = builderContext.manifestConfig()->m_envs;
    if (options != nullptr)
    {
        for (auto [k, v] : options->m_env)
        {
            envs[k] = v;
        }

        if (!options->m_cFlags.empty())
        {
            env.push_back("CFLAGS=" + options->m_cFlags);
        }
        if (!options->m_cppFlags.empty())
        {
            env.push_back("CPPFLAGS=" + options->m_cppFlags);
        }
        if (!options->m_cxxFlags.empty())
        {
            env.push_back("CXXFLAGS=" + options->m_cxxFlags);
        }
        if (!options->m_ldFlags.empty())
        {
            env.push_back("LDFLAGS=" + options->m_ldFlags);
        }

        concatEnvironmentPath(envs, "PATH", options->m_prependPath, options->m_appendPath);
        concatEnvironmentPath(envs, "LD_LIBRARY_PATH", options->m_prependLdLibraryPath, options->m_appendLdLibraryPath);
        concatEnvironmentPath(envs, "PKG_CONFIG_PATH", options->m_prependPkgConfigPath, options->m_appendPkgConfigPath);
    }

    std::string ccachedir = "/run/ccache/disabled";
    if (builderContext.isCcache())
    {
        ccachedir = "/run/ccache";
        std::string old = envs["PATH"];
        if (old.empty())
        {
            old = "$PATH";
        }

        std::string path = std::string("/run/ccache/bin:") + old;
        envs["PATH"] = path;
    }
    env.push_back("CCACHE_DIR=" + ccachedir);

    for (auto [k, v] : envs)
    {
        env.push_back(k + "=" + v);
    }

    // getExtenv(options, env);
}

/**
 * @brief: 获取额外的环境变量
 * @param: KMBuildOptions
 * @param: env编译环境变量
 */
void KMBuilderBuild::getExtenv(const shared_ptr<KMBuildOptions> &options, vector<string> &env)
{
    string path = "$PATH";
    string ccachedir = "/run/ccache/disabled";
    if (builderContext.isCcache())
    {
        ccachedir = "/run/ccache";
        path = string("/run/ccache/bin:") + path;
    }

    env.push_back("CCACHE_DIR=" + ccachedir);

    if (!options)
    {
        if (path != "$PATH")
        {
            env.push_back("PATH=" + path);
        }
        
        return;
    }

    string append = options->m_appendPath;
    if (!append.empty())
    {
        path.append(":" + append);
    }

    string prepend = options->m_prependPath;
    if (!prepend.empty())
    {
        // path.append(":" + prepend);
        path = prepend + ":" + path;
    }

    env.push_back("PATH=" + path);

    string ldpath = "$LD_LIBRARY_PATH";
    string appendLdPath = options->m_appendLdLibraryPath;
    if (!appendLdPath.empty())
    {
        ldpath.append(":" + appendLdPath);
    }

    string prependLdPath = options->m_prependLdLibraryPath;
    if (!prependLdPath.empty())
    {
        // ldpath.append(":" + prependLdPath);
        ldpath = prependLdPath + ":" + ldpath;
    }

    env.push_back("LD_LIBRARY_PATH=" + ldpath);

    string pkgconfigpath = "$PKG_CONFIG_PATH";
    string appendPkgConfig = options->m_appendPkgConfigPath;
    if (!appendPkgConfig.empty())
    {
        pkgconfigpath.append(":" + appendPkgConfig);
    }

    string preappendPkgConfig = options->m_prependPkgConfigPath;
    if (!preappendPkgConfig.empty())
    {
        // pkgconfigpath.append(":" + preappendPkgConfig);
        pkgconfigpath = preappendPkgConfig + ":" + pkgconfigpath;
    }

    env.push_back("PKG_CONFIG_PATH=" + pkgconfigpath);
}

/**
 * @brief: 获取加密环境变量
 * @param: BuildOptions编译选项
 * @param: env编译环境变量
 */
void KMBuilderBuild::getBuildSecretEnv(const shared_ptr<KMBuildOptions> &options, vector<string> &env)
{
    if (!options)
    {
        return;
    }

    for (auto [k, v] : options->m_secretEnv)
    {
        env.push_back(k + "=" + v);
    }
}

/**
 * @brief: 创建构建子目录
 * @param: [in], dir，构建目录
 * @param: [in], name，模块名
 * @param: [out], subdir，子目录全路径名
 * @return: true-成功; false-失败
 */
bool createSubdir(const std::string &dir, const std::string &name, std::string &subdir)
{
    if (!KMBuilderUtils::ensureDir(dir))
        return false;

    for (int i = 1; i < 1000; i++)
    {
        subdir = dir + "/" + name + "-" + std::to_string(i);
        if (fs::exists(subdir))
            continue;

        std::error_code ec;
        std::string link = dir + "/" + name;

        if (fs::exists(link))
            fs::remove(link);

        if (fs::create_directories(subdir) && KMBuilderUtils::createDirectorySymlink(subdir, link, ec))
            return true;
    }

    return false;
}

/**
 * @brief: 清理编译目录
 * @param: sourceLink，源码目录链接文件
 * @param: sourceDir，源码目录
 * @return: true-成功; false-失败
 */
bool KMBuilderBuild::cleanupBuildbdir(const string &sourceLink, const string &sourceDir)
{
    if (fs::exists(sourceLink))
    {
        fs::remove(sourceLink);
    }
    
    if (fs::exists(sourceDir))
    {
        return fs::remove_all(sourceDir);
    }

    return true;
}

bool download(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir)
{
    for (auto source : pBuildModule->buildSources())
    {
        if (!source->isEnable())
        {
            continue ;
        }

        if (!source->downloadSource(sourcedir))
        {
            return false;
        }
    }
    
    return true;
}
