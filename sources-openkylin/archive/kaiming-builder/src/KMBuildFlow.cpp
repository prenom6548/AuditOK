/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildFlow.h"
#include "KMBuilderBuild.h"
#include "KMBuilderContext.h"
#include "KMBuilderUtils.h"
#include "KMBuilderCache.h"
#include "KylinDBusSecConfig.h"
#include "common/KMProcessUtils.h"
#include "common/KMException.h"
#include "common/KMInfoJson.h"
#include "common/KMProcessUtils.h"
#include "common/KMLogger.h"
#include "common/KMSubPackagesJson.h"

namespace fs = std::filesystem;

REGISTER_SUBCOMMAND_DYNCREATE(build, KMBuildFlow)

KMBuildFlow::KMBuildFlow()
{
}

KMBuildFlow::~KMBuildFlow()
{}

int KMBuildFlow::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMBuildFlow::init(int argc, char **argv)
{
    // 参数解析
    builderContext.parseCommandLine(argc, argv);

    km_logger.setLogPath(builderContext.getCacheDir());

    // 加载清单配置
    builderContext.loadConfig();

    KMBuilderCache::instance().open(builderContext.getCacheDir());
}

int KMBuildFlow::run()
{
    setCcache(builderContext.isCcache(), builderContext.getCcacheDir());

    if (!builderContext.isFinishOnly() && !builderContext.isExportOnly())
    {
        std::map<std::shared_ptr<KMBuilderModule>, std::string> moudle_str_map;
        downloadFile(moudle_str_map);

        buildInit();

        build(moudle_str_map);

        std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
        if (manifest->m_subpackages.empty())
        {
            if (manifest->m_package.m_kind == PACKGE_KIND_DEPEND)
            {
                std::string dependFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "depends");
                if (fs::exists(dependFiles))
                {
                    KMBuilderUtils::rmRfAll(dependFiles);
                }
            }
            else
            {
                amendDepends();
            }
        }

        cleanup();

        if (builderContext.isBuildOnly())
        {
            return EXIT_SUCCESS;
        }
    }

    if (!builderContext.isExportOnly())
    {
        buildFinish();
    }

    if (builderContext.isExport() || builderContext.isExportAll())
    {
        buildExport();
    }

    KMInfo("Build completed.");

    return EXIT_SUCCESS;
}

/**
 * @brief: 设置ccache目录
 * @param: 是否启用
 * @param: ccache缓存目录
 */
bool KMBuildFlow::setCcache(bool enable, const std::string &ccacheDir)
{
    if (enable)
    {
        std::error_code ec;
        std::string binDir = KMBuilderUtils::buildFilename(ccacheDir, "bin");
        if (!KMBuilderUtils::mkpath(binDir, ec))
        {
            throw KMException(ec.message());
        }

        std::vector<string> compilers = { "cc", "c++", "gcc", "g++" };
        for (auto &compiler : compilers)
        {
            std::string symlinkPath = KMBuilderUtils::buildFilename(binDir, compiler);
            if (fs::exists(symlinkPath, ec) && !fs::remove(symlinkPath, ec))
            {
                throw KMException(ec.message());
            }
            fs::create_symlink("/usr/bin/ccache", symlinkPath, ec);
        }
    }
    else
    {
        std::error_code ec;
        std::string disabledDir = ccacheDir + "/disabled";
        if (!KMBuilderUtils::mkpath(disabledDir, ec))
        {
            throw KMException(ec.message());
        }

        std::string ccacheConfiPath = disabledDir + "/ccache.conf";
        std::ofstream file(ccacheConfiPath, ios::out);
        file << "disable = true\n";
        file.close();
    }

    return true;
}

/**
 * @brief : 编译初始化
 * @return : true-成功; false-失败
 */
bool KMBuildFlow::buildInit()
{
    KMTrace("KMBuildFlow::buildInit invoke begin");

    if (!builderContext.isForceClean())
    {
        if (KMBuilderCache::instance().lookup("init"))
        {
            KMInfo("Cache hit, skipping init stage");
            return true;
        }
    }

    KMInfo("Initializing");

    // 清理appDir，重新初始化
    KMBuilderUtils::rmRfAll(builderContext.getAppDir());

    std::vector<std::string> args;
    initArgs(args);

    if (!KMProcessUtils::spawn("kaiming", args, false))
    {
        throw KMException("kaiming build-init failed.");
    }
    KMBuilderCache::instance().open(builderContext.getCacheDir());
    KMBuilderCache::instance().recordStage("init");

    KMTrace("KMBuildFlow::buildInit invoke end");
    return true;
}

/**
 * @brief : 下载
 * @return : true-成功; false-失败
 */
bool KMBuildFlow::downloadFile(std::map<std::shared_ptr<KMBuilderModule>, std::string> &moudle_str_map)
{
    KMTrace("KMBuildFlow::downloadFile invoke begin");

    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    expandModules(manifest->m_components);
    KMInfo("-------------");
    KMInfo("components: ");
    for (auto module : m_expandModules)
    {
        std::cout << "  - " << std::any_cast<std::string>(module->getProperty(KMBuilderModule::Module_Name)) << std::endl;
    }
    KMInfo("-------------");
    
    KMInfo("Downloading");
    std::string stopAt = builderContext.getStopat();
    for (auto module : m_expandModules)
    {
        std::string name = std::any_cast<std::string>(module->getProperty(KMBuilderModule::Module_Name));
        if (stopAt == name)
        {
            return true;
        }

        if (!builderContext.isForceClean())
        {
            if (KMBuilderCache::instance().lookup("build-component-" + name))
            {
                KMInfo("Cache hit, skipping download : " + name);
                continue;
            }
        }

        if (!createSubdir(builderContext.getBuildDir(), name, moudle_str_map[module]))
        {
            throw KMException("Failed to create build dir for component : " + name);
        }

        if (!download(module, moudle_str_map[module]))
        {
            throw KMException("Failed to download sources for component : " + name);
        }
    }

    KMTrace("KMBuildFlow::downloadFile invoke end");
    return true;
}

/**
 * @brief : 编译
 * @return : true-成功; false-失败
 */
bool KMBuildFlow::build(std::map<std::shared_ptr<KMBuilderModule>, std::string> &moudle_str_map)
{
    KMTrace("KMBuildFlow::build invoke begin");

    // 记录构建前初始化后的文件列表
    m_lastFiles = KMBuilderUtils::getDirFiles(KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files"));
    m_moduleToFiles["init-stage"] = m_lastFiles;
    
    KMInfo("Start building");
    std::string stopAt = builderContext.getStopat();
    for (auto module : m_expandModules)
    {
        std::string name = std::any_cast<std::string>(module->getProperty(KMBuilderModule::Module_Name));
        if (stopAt == name)
        {
            return true;
        }

        if (!isShouldBuild(module))
        {
            continue;
        }

        if (!builderContext.isForceClean())
        {
            if (KMBuilderCache::instance().lookup("build-component-" + name))
            {
                KMInfo("Cache hit, skipping component : " + name);
                continue;
            }
        }

        KMBuilderBuild build;
        build.m_subdir = moudle_str_map[module];
        if (!build.doBuild(module))
        {
            throw KMException("Failed to build component : " + name);
        }

        // 记录当前模块构建新增的文件
        std::vector<std::string> files = KMBuilderUtils::getDirFiles(KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files"));
        std::vector<std::string> incFiles;
        for (auto& file : files)
        {
            if (!KMBuilderUtils::contains(m_lastFiles, file))
            {
                incFiles.push_back(file);
            }
        }
        m_lastFiles = files;
        m_moduleToFiles[name] = incFiles;

        KMBuilderCache::instance().recordStage("build-component-" + name, incFiles);
        m_addComponents = true;
    }

    KMTrace("KMBuildFlow::build invoke end");
    return true;
}

/**
 * @brief : 初始化参数
 * @return : true-成功; false-失败
 */
bool KMBuildFlow::initArgs(std::vector<std::string> &args)
{
    /* 
       用法：kaiming build-init [OPTIONS] DIRECTORY APPID
       样例：kaiming build-init --base=dev.openkylin.Base/channel/23.0.0 --runtime=dev.openkylin.Runtime/channel/23.0.0 build top.openkylin.demo
     */
    // args.push_back("kaiming");
    args.push_back("build-init");
    if (builderContext.isVerbose())
    {
        args.push_back("-v");
    }

    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    args.push_back("--arch=" + builderContext.getArch());
    args.push_back("--kind=" + manifest->m_package.m_kind);
    args.push_back("--channel=" + manifest->m_channel);
    args.push_back("--base=" + manifest->m_base);
    args.push_back("--version=" + manifest->m_package.m_version);
    args.push_back("--cacheDir=" + builderContext.getCacheDir());

    if (!manifest->m_depends.empty())
    {
        for (auto depend : manifest->m_depends)
        {
            args.push_back("--depend=" + depend);
        }
    }

    if (!manifest->m_runtime.empty())
    {
        args.push_back("--runtime=" + manifest->m_runtime);
    }

    if (manifest->m_annotations.kylin_privilege)
    {
        args.push_back("--kylin-privilege");
    }

    if (manifest->m_annotations.kylin_sysapp)
    {
        args.push_back("--kylin-sysapp");
    }

    args.push_back(builderContext.getAppDir());
    args.push_back(manifest->m_package.m_id);

    return true;
}

/**
 * @brief : 展开所有的builderModules
 * @param modules : 所有的builderModules
 * @param expandModules : 展开后的builderModules
 * @return : true-成功; false-失败
 */
void KMBuildFlow::expandModules(const std::vector<std::shared_ptr<KMBuilderModule>> &modules)
{
    for (auto module : modules)
    {
        if (module->getData()->m_disabled)
        {
            continue;
        }

        std::vector<std::shared_ptr<KMBuilderModule>> inmodules = module->builderModules();
        if (!inmodules.empty())
        {
            expandModules(inmodules);
        }

        m_expandModules.push_back(module);
    }
}

/**
 * @brief : 是否需要构建
 * @param module : builderModule模块
 * @return : true-需要; false-不需要
 */
bool KMBuildFlow::isShouldBuild(const std::shared_ptr<KMBuilderModule> &module)
{
    if (module->buildSystem() == KMBuilderModule::BuildSystem::BS_Unknow)
    {
        return false;
    }

    if (!module->buildSources().empty())
    {
        return true;
    }

    // 即使没有源代码，也允许构建simple类型，这通常很有用
    if (module->buildSystem() == KMBuilderModule::BuildSystem::BS_Simple)
    {
        return true;
    }

    return false;
}

/**
 * @brief : 修正depends数据
 */
void KMBuildFlow::amendDepends()
{
    std::string appFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files");
    std::string dependFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "depends");

    if (!fs::exists(dependFiles))
    {
        return ;
    }

    bool sync = false;
    fs::path dependPath(dependFiles);
    for (auto& dirEntry : fs::directory_iterator{ dependPath })
    {
        fs::path p = dirEntry.path();
        std::string filename = p.filename();
        if (filename == "." || filename == "..")
        {
            continue ;
        }

        if (fs::is_regular_file(p))
        {
            KMBuilderUtils::cpAll(p.string(), appFiles);
        }
        else if (filename == "usr")
        {
            KMBuilderUtils::rsyncDir(p.string(), appFiles);
        }
        else
        {
            std::string destDir = KMBuilderUtils::buildFilename(appFiles, filename);
            KMBuilderUtils::mkpath(destDir);

            std::vector<std::string> excludes;
            excludes.push_back("ld.so.cache");
            KMBuilderUtils::rsyncDir(p.string(), destDir, excludes);
        }
        
        sync = true;
    }

    if (sync)
    {
        KMProcessUtils::spawn("sync");
        KMBuilderUtils::rmRfAll(dependFiles);
    }
}

/**
 * @brief : 构建流程中的cleanup环节
 */ 
void KMBuildFlow::cleanup()
{
    KMTrace("KMBuildFlow::cleanup invoke begin");

    std::string appFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files");
    std::string dependFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "depends");
    for (auto& module : m_expandModules)
    {
        if (module->cleanup().empty())
        {
            continue ;
        }

        if (m_moduleToFiles.end() == m_moduleToFiles.find(module->getData()->m_name))
        {
            continue ;
        }

        const std::vector<std::string> &files = m_moduleToFiles.at(module->getData()->m_name);
        std::vector<std::string> toRemoves;
        KMBuildFlow::collectCleanupForPath(files, appFiles, module->cleanup(), toRemoves);

        for (auto& remove : toRemoves)
        {
            KMBuilderUtils::rmRfAll(remove);
        }
    }

    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    if (!manifest->m_cleanups.empty())
    {
        std::vector<std::string> toRemoves;
        KMBuildFlow::collectCleanupForPath(appFiles, appFiles, manifest->m_cleanups, toRemoves);
        KMBuildFlow::collectCleanupForPath(dependFiles, dependFiles, manifest->m_cleanups, toRemoves);
        for (auto& remove : toRemoves)
        {
            KMBuilderUtils::rmRfAll(remove);
        }
    }

    // if (manifest->m_annotations.kylin_sysapp)
    {
        // 删除构建结果中的因挂载而生成到构建结果中的文件，需要在删除空目录之前
        std::vector<std::string> defaultCleanups = {
            "/etc/default/locale",
            "/etc/timezone",
            "/etc/passwd",
            "/etc/group",
            "/etc/pkcs11/pkcs11.conf",
            "/etc/shadow",
            "/etc/gshadow",
            "/etc/machine-id",
            "/etc/localtime",
            "/usr/share/zoneinfo",
            "/etc/resolv.conf",
            "/etc/host.conf",
            "/etc/hosts",
            "/etc/gai.conf",
            "/usr/bin/xdg-open",
            "/etc/ld.so.cache"
        };

        for (auto& cleanup : defaultCleanups)
        {
            std::string remove = KMBuilderUtils::buildFilename(appFiles, cleanup);
            KMBuilderUtils::rmRfAll(remove);
        }

        // 删除构建结果中的空文件夹，这些空文件夹应该都是overlay叠加挂载时创建upperdir时创建的（目的是使得目录可写）
        std::error_code ec;
        if (!KMBuilderUtils::removeEmptyDirectory(appFiles, ec))
        {
            throw KMException(ec.message());
        }
    }

    KMTrace("KMBuildFlow::cleanup invoke end");
}

/**
 * @brief : 收集指定路径下匹配规则的文件/目录列表
 * @param : [in] path, 指定路径
 * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
 * @param : [in] matchs, 匹配规则
 * @param : [out] toRemoves，待删除的路径
 */ 
void KMBuildFlow::collectCleanupForPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string>& toRemoves)
{
    fs::path basePath(path);
    if (!fs::exists(path) || matchs.empty())
    {
        return ;
    }

    for (auto const &dirEntry : fs::directory_iterator{ basePath })
    {
        fs::path p = dirEntry.path();
        std::string filename = p.filename();
        if (filename == "." || filename == "..")
        {
            continue ;
        }

        std::string subPath = p.string();
        std::string unprefixedPath = subPath.substr(prefix.length());

        bool isMatch = false;
        for (auto& match : matchs)
        {
            const char *rest =  nullptr;
            if (match.at(0) == '/')
            {
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), unprefixedPath.c_str());
            }
            else
            {
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), filename.c_str());
            }

            if (rest != nullptr)
            {
                isMatch = true;
                break ;
            }
        }

        if (isMatch)
        {
            toRemoves.push_back(subPath);
            continue ;
        }

        if (fs::is_directory(p))
        {
            KMBuildFlow::collectCleanupForPath(subPath, prefix, matchs, toRemoves);
        }
    }
}

void KMBuildFlow::collectCleanupForPath(const std::vector<std::string>& files, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &toRemoves)
{
    std::string filename;
    for (auto& file : files)
    {
        std::string unprefixedPath = file.substr(prefix.length());

        bool isMatch = false;
        for (auto& match : matchs)
        {
            const char *rest =  nullptr;
            if (match.at(0) == '/')
            {
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), unprefixedPath.c_str());
            }
            else
            {
                filename = fs::path(file).filename();
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), filename.c_str());
            }

            if (rest != nullptr)
            {
                isMatch = true;
                break ;
            }
        }

        if (isMatch)
        {
            toRemoves.push_back(file);
        }
    }
}

void KMBuildFlow::addEnv(std::map<std::string, std::string>& envs, const std::string &key, const std::string &value)
{
    std::string newValue = value;
    while (KMBuilderUtils::contains(newValue, "$"))
    {
        // value值如：$PATH:/app/bin 或 /app/bin:$PATH 或 $HOME/.config
        size_t start = newValue.find_first_of('$');
        size_t end_colon = newValue.find_first_of(':', start);
        size_t end_slash = newValue.find_first_of('/', start);
        size_t end = end_colon < end_slash ? end_colon : end_slash;

        std::string keyOfEnv;
        std::string replaceKey;
        if (std::string::npos == end)
        {
            keyOfEnv = newValue.substr(start);
            replaceKey = keyOfEnv;
            keyOfEnv = keyOfEnv.substr(1);
        }
        else
        {
            keyOfEnv = newValue.substr(start, end - start);
            if (end_colon == end)
            {
                replaceKey = keyOfEnv + ":";
            }
            else if (end_slash == end)
            {
                replaceKey = keyOfEnv + "/";
            }
            keyOfEnv = keyOfEnv.substr(1);
        }

        // std::string old = envs[keyOfEnv];
        std::string old;
        if (envs.find(keyOfEnv) != envs.end())
        {
            old = envs[keyOfEnv];
        }
        if (!old.empty() && KMBuilderUtils::endsWith(replaceKey, ":"))
        {
            old += ":";
        }
        else if (!old.empty() && KMBuilderUtils::endsWith(replaceKey, "/"))
        {
            old += "/";
        }
        KMBuilderUtils::replace(newValue, replaceKey, old);
    }

    if (KMBuilderUtils::startsWith(newValue, ":"))
    {
        // value值如：:/app/bin，则认为是在原基础上追加；
        std::string old = envs[key];
        newValue = old + newValue;
    }
    else if (KMBuilderUtils::endsWith(newValue, ":"))
    {
        // value值如：/app/bin:，则认为是在原基础上增加前缀；
        std::string old = envs[key];
        newValue = newValue + old;
    }

    envs[key] = newValue;
}

/**
 * @brief : 填充info.json的部分信息
 */
void KMBuildFlow::fillInfoJson()
{
    // build-init时已经生成了部分info.json信息，此处填充其它信息
    KMInfoJson metadata;
    metadata.loadFile(KMBuilderUtils::buildFilename(builderContext.getAppDir(), "info.json"));

    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    metadata.commands = manifest->m_commands;
    metadata.modules = manifest->m_modules;
    metadata.name = manifest->m_package.m_name;
    metadata.names = manifest->m_package.m_names;
    metadata.description = manifest->m_package.m_description;
    metadata.descriptions = manifest->m_package.m_descriptions;
    metadata.version = manifest->m_package.m_version;
    metadata.permission = manifest->m_permission;
    // depend的环境变量仅用于构建环境，如需加入运行环境，请在yaml中增加
    // if (metadata.kind == PACKGE_KIND_APP)
    // {
    //     // metadata.envs可能已经包含了depends依赖引入的环境变量，故此处追加
    //     for (auto& env : manifest->m_envs)
    //     {
    //         KMBuildFlow::addEnv(metadata.envs, env.first, env.second);
    //     }
    // }
    metadata.envs = manifest->m_envs;
    metadata.license = manifest->m_license;
    metadata.homepage = manifest->m_homepage;
    metadata.maintainers = manifest->maintainers;
    metadata.annotations = manifest->m_annotations;
    metadata.export_plugins = manifest->m_exportPlugins;

    // 麒麟系统的dbus管控白名单配置
    std::vector<std::string> whitelist = getDbusWhitelist();
    if (!whitelist.empty())
    {
        metadata.annotations.kylin_dbus_whitelist = whitelist;
    }

    metadata.saveFile(KMBuilderUtils::buildFilename(builderContext.getAppDir(), "info.json"));
    if (metadata.kind == PACKGE_KIND_APP && manifest->m_modules.size() > 1) 
    {
        metadata.saveModulesFile(KMBuilderUtils::buildFilename(builderContext.getAppDir(), "modules.json"));
    }
}

/**
 * @brief : 获取dbus服务的白名单列表
 * @return: dbus服务的白名单列表。每个数组元素都是一个可执行程序完整路径。dbus-daemon管控的判断逻辑是：
            (1)判断/proc/进程id/exe 和白名单中的名字是否一致
            (2)判断白名单中的文件是否存在
 */
std::vector<std::string> KMBuildFlow::getDbusWhitelist()
{
    std::vector<std::string> whitelist;

    std::string configDir = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files", "/etc/kylin-config/basic");
    if (fs::exists(configDir))
    {
        fs::path configPath(configDir);
        for (auto const &dirEntry : fs::directory_iterator{ configPath })
        {
            fs::path p = dirEntry.path();
            if (fs::is_regular_file(p))
            {
                std::string filename = p.filename();
                if (KMBuilderUtils::endsWith(filename, ".yaml"))
                {
                    std::shared_ptr<KylinDBusSecConfig> config = KylinDBusSecConfig::loadFromYAML(p.string());
                    if (config)
                    {
                        for (const std::string& bin : config->whitelist)
                        {
                            whitelist.push_back(bin);
                        }
                    }
                }
            }
        }
    }

    return whitelist;
}

/**
 * @brief : 将changelog等文件封装到构建结果中
 */
void KMBuildFlow::finishKaimingDir()
{
    std::string manifest = builderContext.getManifestPath();
    fs::path parentPath = fs::path(manifest).parent_path();
    fs::path pathChangelog = parentPath / "changelog";

    if (fs::exists(pathChangelog))
    {
        std::string kaimingDir = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "KAIMING");
        KMBuilderUtils::mkpath(kaimingDir);

        std::string srcChangelog = pathChangelog.string();
        std::string destChangelog = KMBuilderUtils::buildFilename(kaimingDir, "changelog");
        fs::copy_file(srcChangelog, destChangelog, fs::copy_options::overwrite_existing);
    }
    else
    {
        KMWarn("Not found file : " + pathChangelog.string());
    }
}

/**
 * @brief : 收集子开明包包含的路径列表
 * @param : [in] path, 指定路径; files，指定的文件列表
 * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
 * @param : [in] matchs, 匹配规则
 * @param : [out] includes，子开明包包含的路径列表
 */ 
void KMBuildFlow::collectSubPackagesPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &includes)
{
    fs::path basePath(path);
    if (!fs::exists(path) || matchs.empty())
    {
        return ;
    }

    for (auto const &dirEntry : fs::directory_iterator{ basePath })
    {
        fs::path p = dirEntry.path();
        std::string filename = p.filename();
        if (filename == "." || filename == "..")
        {
            continue ;
        }

        std::string subPath = p.string();
        std::string unprefixedPath = subPath.substr(prefix.length());

        bool isMatch = false;
        for (auto& match : matchs)
        {
            const char *rest =  nullptr;
            if (match.at(0) == '/')
            {
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), unprefixedPath.c_str());
            }
            else
            {
                rest = KMBuilderUtils::pathMatchPrefix(match.c_str(), filename.c_str());
            }

            if (rest != nullptr)
            {
                isMatch = true;
                break ;
            }
        }

        if (isMatch)
        {
            includes.push_back(unprefixedPath);
            continue ;
        }

        if (fs::is_directory(p))
        {
            KMBuildFlow::collectSubPackagesPath(subPath, prefix, matchs, includes);
        }
    }
}

/**
 * @brief : 生成subpackages.json文件
 */ 
void KMBuildFlow::generateSubPackagesJson()
{
    KMSubPackagesJson json;
    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    std::string appFiles = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "files");

    for (auto& subpackage : manifest->m_subpackages)
    {
        KMSubPackagesJson::SubPackage package;
        package.id = subpackage.m_id;
        package.name = subpackage.m_name;
        package.kind = subpackage.m_kind;
        package.version = manifest->m_package.m_version;
        package.module = subpackage.m_module;
        package.description = subpackage.m_description;
        package.names = subpackage.m_names;
        package.descriptions = subpackage.m_descriptions;
        package.commands = subpackage.m_commands;
        std::vector<std::string> includes;
        KMBuildFlow::collectSubPackagesPath(appFiles, appFiles, subpackage.m_includes, includes);
        package.includes = includes;
        package.depends = subpackage.m_depends;

        json.subpackages.push_back(package);
    }

    std::string jsonFile = KMBuilderUtils::buildFilename(builderContext.getAppDir(), "subpackages.json");
    json.saveFile(jsonFile);
}

/**
 * @brief : 构建流程中的finish环节
 */
void KMBuildFlow::buildFinish()
{
    KMTrace("KMBuildFlow::buildFinish invoke begin");

    if (!builderContext.isForceClean() && !m_addComponents && !builderContext.isFinishOnly())
    {
        if (KMBuilderCache::instance().lookup("build-finish"))
        {
            KMInfo("Cache hit, skipping build-finish");
            return;
        }
    }

    std::shared_ptr<KMBuilderManifest> manifest = builderContext.manifestConfig();
    KMInfo("Finishing " + manifest->m_package.m_id);

    fillInfoJson();

    if (manifest->m_subpackages.empty())
    {
        std::vector<std::string> args;
        args.push_back("build-finish");
        if (builderContext.isVerbose())
        {
            args.push_back("-v");
        }
        if (manifest->m_package.m_kind != PACKGE_KIND_APP)
        {
            args.push_back("--noexports");
        }
        args.push_back(builderContext.getAppDir());

        if (!KMProcessUtils::spawn("kaiming", args, false))
        {
            throw KMException("kaiming build-finish failed.");
        }
    }
    else
    {
        generateSubPackagesJson();
    }

    finishKaimingDir();

    KMBuilderCache::instance().recordStage("build-finish");

    KMTrace("KMBuildFlow::buildFinish invoke end");
}

/**
 * @brief : 构建流程中的export环节
 */ 
void KMBuildFlow::buildExport()
{
    KMTrace("KMBuildFlow::buildExport invoke begin");

    if (!builderContext.isForceClean() && !m_addComponents && !builderContext.isFinishOnly() && !builderContext.isExportOnly())
    {
        if (KMBuilderCache::instance().lookup("build-export"))
        {
            KMInfo("Cache hit, skipping build-export");
            return;
        }
    }

    std::vector<std::string> args;
    args.push_back("build-export");
    if (builderContext.isVerbose())
    {
        args.push_back("-v");
    }
    args.push_back("--dest-dir=" + builderContext.getRunDir());
    args.push_back("--bundle-filesystem=" + builderContext.getBundleFilesystem());
    if (builderContext.isExportAppImage())
    {
        args.push_back("--appimage");
    }
    else if (builderContext.isExportAll())
    {
        args.push_back("--all");
    }
    if (!builderContext.getCompression().empty())
    {
        args.push_back("--comp=" + builderContext.getCompression());
    }
    args.push_back(builderContext.getAppDir());

    if (!KMProcessUtils::spawn("kaiming", args, false))
    {
        throw KMException("Failed to export.");
    }

    KMBuilderCache::instance().recordStage("build-export");

    KMTrace("KMBuildFlow::buildExport invoke end");
}

