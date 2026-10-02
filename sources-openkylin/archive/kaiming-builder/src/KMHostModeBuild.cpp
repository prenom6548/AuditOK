/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMHostModeBuild.h"

#include <filesystem>

#include "common/AptUtils.h"
#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMInfoJson.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMProcessUtils.h"
#include "common/KMSubPackagesJson.h"
#include "common/kmtranslation.h"
#include "KMBuilderUtils.h"
#include "KMHostModeManifest.h"

namespace fs = std::filesystem;

class KMHostModeBuild::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();

public:
    bool m_help = false;
    bool m_isVerbose = false;
    bool m_isForceClean = false;

    int m_jobs = -1;
    std::string m_bundleFilesystem{ "squashfs" };
    std::string m_output;
    std::string m_compression;
    std::string m_srcDirectory;
};

void KMHostModeBuild::Options::preParseHook()
{
    addOptions();
}

void KMHostModeBuild::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_output.empty())
    {
        m_output = fs::current_path().string();
    }
    else if (!fs::exists(m_output))
    {
        KMBuilderUtils::mkpath(m_output);
    }

    if (m_srcDirectory.empty())
    {
        m_srcDirectory = fs::current_path().string();
    }

    if (!fs::exists(m_srcDirectory))
    {
        throw KMException("SRC-DIRECTORY : " + m_srcDirectory + " not exists.");
    }

    m_output = fs::canonical(m_output).string();
    m_srcDirectory = fs::canonical(m_srcDirectory).string();
}

void KMHostModeBuild::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming-builder hostmode-build [OPTIONS] SRC-DIRECTORY\n"));

    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_help), _("Show help options"));

    //应用选项
    addOption("verbose","v", KMOption::value(&m_isVerbose), _("Print debug information during command processing"));
    addOption("force-clean","f", KMOption::value<bool>(&m_isForceClean), _("Clean cache"));
    addOption("jobs", "",KMOption::value<int>(&m_jobs), _("Number of parallel jobs to build (default=NCPU)"));
    addOption("bundle-filesystem", "",KMOption::value(&m_bundleFilesystem), _("The filesystem of the kaiming package's bundle section. Such as squashfs、erofs. Default squashfs."));
    addOption("output","o", KMOption::value<std::string>(&m_output), _("The path for output of build, default $(pwd)"));
    addOption("comp","", KMOption::value<std::string>(&m_compression), _("--comp=<comp [extra options]>. Select <comp> compression, squashfs filesystem's <comp> see command mksquashfs, for example : --comp=xz or --comp='zstd -Xcompression-level 18'; erofs filesystem's <comp> see command mkfs.erofs, for example : --comp=lz4hc"));

    //位置选项
    addPositionOption("SRC-DIRECTORY", KMOption::value<std::string>(&m_srcDirectory), 1, _("The directory of source code."));
}

class KMHostModeBuild::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::shared_ptr<KMHostModeManifest> m_manifest;
    std::string m_appDir;
    std::string m_installDir;
    std::string m_cacheDir;

    KMHostModeManifest::BuildSystem m_buildSystem = KMHostModeManifest::BS_Unknow;
    std::string m_configureFile;
};

REGISTER_SUBCOMMAND_DYNCREATE(hostmode-build, KMHostModeBuild)

KMHostModeBuild::KMHostModeBuild()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMHostModeBuild::~KMHostModeBuild()
{}

int KMHostModeBuild::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMHostModeBuild::init(int argc, char **argv)
{
    KMTrace("KMHostModeBuild::init invoke begin");

    // 参数解析
    d->m_kmOptions->parseCommandLine(argc, argv);

    // 加载清单配置
    d->m_manifest = KMHostModeManifest::loadFromYAML(KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming/manifest.yaml"));
    d->m_appDir = KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming", d->m_manifest->source);
    d->m_installDir = KMBuilderUtils::buildFilename(d->m_appDir, "files");
    d->m_cacheDir = KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, ".kaiming-builder");
    km_logger.setLogPath(d->m_cacheDir);

    if (d->m_kmOptions->m_isForceClean)
    {
        KMBuilderUtils::rmRfAll(d->m_appDir);
        KMBuilderUtils::rmRfAll(d->m_cacheDir);
    }

    KMBuilderUtils::mkpath(d->m_installDir);
    KMBuilderUtils::mkpath(d->m_cacheDir);

    KMTrace("KMHostModeBuild::init invoke end");
}

int KMHostModeBuild::run()
{
    KMTrace("KMHostModeBuild::run invoke begin");

    installBuildDepends();

    ensureBuildSystem();

    if (d->m_buildSystem == KMHostModeManifest::BS_Simple)
    {
        costomBuild();
    }
    else
    {
        configure();

        build();

        install();
    }

    postBuild();

    finish();

    exportPackage();

    KMInfo("Build completed.");

    KMTrace("KMHostModeBuild::run invoke end");
    return EXIT_SUCCESS;
}

void KMHostModeBuild::installBuildDepends()
{
    installDebDepends();
    installKaimingDepends();
}

static void parseOpAndVersion(std::string& v, std::string& op)
{
    if (KMBuilderUtils::startsWith(v, "<="))
    {
        op = "le";
        KMBuilderUtils::replace(v, "<=", op);
    }
    else if (KMBuilderUtils::startsWith(v, "<"))
    {
        op = "lt";
        KMBuilderUtils::replace(v, "<", op);
    }
    else if (KMBuilderUtils::startsWith(v, ">="))
    {
        op = "ge";
        KMBuilderUtils::replace(v, ">=", op);
    }
    else if (KMBuilderUtils::startsWith(v, ">"))
    {
        op = "gt";
        KMBuilderUtils::replace(v, ">", op);
    }
    else if (KMBuilderUtils::startsWith(v, "!"))
    {
        op = "ne";
        KMBuilderUtils::replace(v, "!", op);
    }
    else
    {
        op = "eq";
    }
}

void KMHostModeBuild::installDebDepends()
{
    if (d->m_manifest->build_depends.debs.empty())
    {
        return ;
    }

    KMInfo("Preparing install deb depends ...");

    std::vector<std::string> packages;
    for (auto& depend : d->m_manifest->build_depends.debs)
    {
        if (depend.empty())
        {
            continue;
        }

        std::vector<std::string> fileds = KMBuilderUtils::splitString(depend, "/");
        std::string id = fileds.at(0);
        std::string version;
        if (fileds.size() > 1)
        {
            version = fileds.at(1);
        }

        std::string versionInstalled = Dpkg::list(id);
        if (version.empty() && !versionInstalled.empty())
        {
            continue;
        }
        else if (version.empty())
        {
            packages.push_back(id);

            continue;
        }
        else
        {
            std::vector<std::string> versions = AptCache::madison(id);
            for (auto& v : versions)
            {
                std::string op;
                parseOpAndVersion(version, op);
                if (Dpkg::compareVersions(v, op, version))
                {
                    packages.push_back(id + "=" + v);

                    break;
                }
            }
        }
    }

    if (!packages.empty())
    {
        if (!Apt::install(packages))
        {
            std::string mesg("Try : sudo apt install -y ");
            for (auto& package : packages)
            {
                mesg += " ";
                mesg += package;
            }
            throw KMException(mesg);
        }
    }

    return ;
}

void KMHostModeBuild::installKaimingDepends()
{
    if (d->m_manifest->build_depends.kaimings.empty())
    {
        return ;
    }

    KMInfo("Preparing install kaiming depends ...");

    uid_t uid = ::getuid();

    KMInstalledAppQuery query;
    for (auto& depend : d->m_manifest->build_depends.kaimings)
    {
        std::vector<std::string> fileds = KMBuilderUtils::splitString(depend, "/");
        std::string id = fileds.at(0);
        std::string version;
        if (fileds.size() > 1)
        {
            version = fileds.at(1);
        }

        std::vector<Ref> bases = query.kind("app")
                                    .id(id)
                                    .version(version)
                                    .query(KMInstalledAppQuery::System);
        if (bases.empty())
        {
            std::string ref = id;
            if (!version.empty())
            {
                ref += "/";
                ref += version;
            }
            
            KMInfo("Preparing install : " + ref);

            if (uid == 0)
            {
                if (!KMProcessUtils::spawn("kaiming", { "install", "-y", "--compatibility", "--skip-signature-verification", ref }), false)
                {
                    throw KMException("Please install it manually: sudo kaiming install " + ref);
                }
            }
            else
            {
                // 普通用户需提权安装
                if (!KMProcessUtils::spawn("pkexec", { "/opt/kaiming-tools/bin/kaiming", "install", "-y", "--compatibility", "--skip-signature-verification", ref }), false)
                {
                    throw KMException("Please install it manually: sudo kaiming install "  + ref);
                }
            }
        }
    }
}

void KMHostModeBuild::configure()
{
    KMTrace("KMHostModeBuild::configure invoke begin");

    KMInfo("Configuring ...");

    getConfigFilename();

    if (d->m_buildSystem == KMHostModeManifest::BS_Unknow)
    {
        KMTrace("KMHostModeBuild::configure invoke end : buildsystem unknow");
        return;
    }

    generateMakefile();

    KMTrace("KMHostModeBuild::configure invoke end");
}

void KMHostModeBuild::ensureBuildSystem()
{
    if (d->m_manifest->buildsystem == "autotools")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Autotools;
    }
    else if (d->m_manifest->buildsystem == "cmake")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Cmake;
    }
    else if (d->m_manifest->buildsystem == "cmake-ninja")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Cmake_Ninja;
    }
    else if (d->m_manifest->buildsystem == "meson")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Meson;
    }
    else if (d->m_manifest->buildsystem == "qmake")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Qmake;
    }
    else if (d->m_manifest->buildsystem == "simple")
    {
        d->m_buildSystem = KMHostModeManifest::BS_Simple;
    }
}

void KMHostModeBuild::getConfigFilename()
{
    switch (d->m_buildSystem)
    {
    case KMHostModeManifest::BS_Cmake:
    case KMHostModeManifest::BS_Cmake_Ninja:
    {
        if (!fs::exists(KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "CMakeLists.txt")))
        {
            throw KMException("Can't find CMakeLists.txt in " + d->m_kmOptions->m_srcDirectory);
        }
        d->m_configureFile = "CMakeLists.txt";

        break;
    }
    case KMHostModeManifest::BS_Meson:
    {
        if (!fs::exists(KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "meson.build")))
        {
            throw KMException("Can't find meson.build in " + d->m_kmOptions->m_srcDirectory);
        }
        d->m_configureFile = "meson.build";

        break;
    }
    case KMHostModeManifest::BS_Qmake:
    {
        for (auto const &entry : fs::directory_iterator{ d->m_kmOptions->m_srcDirectory })
        {
            if (!fs::is_regular_file(entry))
            {
                continue;
            }

            fs::path p = entry.path();
            if (p.extension().string() != ".pro")
            {
                continue;
            }

            d->m_configureFile = p.filename();

            break;
        }

        if (d->m_configureFile.empty())
        {
            throw KMException("Can't find *.pro file in " + d->m_kmOptions->m_srcDirectory);
        }
    }
    case KMHostModeManifest::BS_Autotools:
    default:
        vector<string> names = { "autogen", "autogen.sh", "bootstrap", "bootstrap.sh" };
        string autogen;
        for (auto& name : names)
        {
            if (fs::exists(KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, name)))
            {
                autogen = name;
                break;
            }
        }
        if (!autogen.empty())
        {
            if (!KMProcessUtils::spawn(autogen, {}, false))
            {
                throw KMException("Field to generate configure via " + autogen);
            }
        }
        if (fs::exists(KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "configure")))
        {
            d->m_configureFile = "configure";
            d->m_buildSystem = KMHostModeManifest::BS_Autotools;
        }
        else if (d->m_buildSystem == KMHostModeManifest::BS_Autotools)
        {
            throw KMException("Can't find configure in " + d->m_kmOptions->m_srcDirectory);
        }
        
        break;
    }
}

void KMHostModeBuild::generateMakefile()
{
    std::string exec;
    std::vector<std::string> args;
    const std::vector<std::string>& configOptions = d->m_manifest->config_opts;

    switch (d->m_buildSystem)
    {
    case KMHostModeManifest::BS_Cmake:
    case KMHostModeManifest::BS_Cmake_Ninja:
    {
        exec = "cmake";

        if (!containsOption(configOptions, "CMAKE_INSTALL_PREFIX"))
        {
            args.push_back("-DCMAKE_INSTALL_PREFIX=/usr");
        }

        for (auto& option : configOptions)
        {
            args.push_back(option);
        }

        if (d->m_buildSystem == KMHostModeManifest::BS_Cmake)
        {
            args.push_back("-GUnix Makefiles");
        }
        else if (d->m_buildSystem == KMHostModeManifest::BS_Cmake_Ninja)
        {
            args.push_back("-GNinja");
        }
        args.push_back("-DCMAKE_VERBOSE_MAKEFILE=ON");

        args.push_back("..");

        break;
    }
    case KMHostModeManifest::BS_Meson:
    {
        exec = "meson";

        if (!containsOption(configOptions, "--prefix="))
        {
            args.push_back("--prefix=/usr");
        }

        for (auto& option : configOptions)
        {
            args.push_back(option);
        }

        args.push_back("..");

        break;
    }
    case KMHostModeManifest::BS_Qmake:
    {
        exec = "qmake";

        if (!containsOption(configOptions, "PREFIX="))
        {
            args.push_back("PREFIX=/usr");
        }

        for (auto& option : configOptions)
        {
            args.push_back(option);
        }

        args.push_back("../" + d->m_configureFile);

        break;
    }
    case KMHostModeManifest::BS_Autotools:
    default:
        exec = "../configure";

        if (!containsOption(configOptions, "--prefix="))
        {
            args.push_back("--prefix=/usr");
        }

        for (auto& option : configOptions)
        {
            args.push_back(option);
        }
        
        break;
    }

    std::string cmdline(exec);
    for (auto &arg : args)
    {
        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            std::string quoted = "'" + arg + "'";
            cmdline += " " + quoted;
        }
        else
        {
            cmdline += " " + arg;
        }
    }
    KMInfo("cd " + d->m_cacheDir + " && " + cmdline);

    if (!KMProcessUtils::spawn(exec, d->m_cacheDir, args, false))
    {
        throw KMException("Failed to : " + exec);
    }
}

/**
 * @brief : 判断configOpts中是否包含了选项option。
 * @param : [in] configOpts, 选项列表
 * @param : [in] option, 选项的KEY
 * @return: true-包含; false-不包含
 * @note  : 使用场景：
 *              1. 构建流程中的配置环节的prefix和libdir等是否配置
 */ 
bool KMHostModeBuild::containsOption(const std::vector<std::string>& configOpts, const std::string& option)
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

void KMHostModeBuild::build()
{
    if (d->m_buildSystem == KMHostModeManifest::BS_Unknow)
    {
        return;
    }

    KMTrace("KMHostModeBuild::build invoke begin");

    KMInfo("Building ...");

    std::string exec;
    int jobs = d->m_kmOptions->m_jobs;
    if (jobs <= 0)
    {
        jobs = sysconf(_SC_NPROCESSORS_ONLN);
    }

    std::vector<std::string> args;
    args.push_back("-j" + std::to_string(jobs > 12 ? 12 : jobs));
    args.push_back("-l" + std::to_string(jobs * 2));

    switch (d->m_buildSystem)
    {
    case KMHostModeManifest::BS_Meson:
    case KMHostModeManifest::BS_Cmake_Ninja:
    {
        exec = "ninja";
        args.push_back("-v");

        break;
    }
    case KMHostModeManifest::BS_Autotools:
    case KMHostModeManifest::BS_Cmake:
    case KMHostModeManifest::BS_Qmake:
    {
        exec = "make";
        args.push_back("VERBOSE=1");

        break;
    }
    default:
        break;
    }

    const std::vector<std::string>& buildOptions = d->m_manifest->build_opts;
    for (auto& option : buildOptions)
    {
        args.push_back(option);
    }

    std::string cmdline(exec);
    for (auto &arg : args)
    {
        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            std::string quoted = "'" + arg + "'";
            cmdline += " " + quoted;
        }
        else
        {
            cmdline += " " + arg;
        }
    }
    KMInfo("cd " + d->m_cacheDir + " && " + cmdline);

    if (!KMProcessUtils::spawn(exec, d->m_cacheDir, args, false))
    {
        throw KMException("Failed to : " + exec);
    }

    KMTrace("KMHostModeBuild::build invoke end");
}

void KMHostModeBuild::install()
{
    if (d->m_buildSystem == KMHostModeManifest::BS_Unknow)
    {
        return;
    }

    KMTrace("KMHostModeBuild::install invoke begin");

    KMInfo("Installing ...");

    std::string exec;
    std::vector<std::string> args;
    const std::vector<std::string>& installOptions = d->m_manifest->install_opts;

    switch (d->m_buildSystem)
    {
    case KMHostModeManifest::BS_Meson:
    case KMHostModeManifest::BS_Cmake_Ninja:
    {
        exec = "bash";
        args.push_back("-c");
        std::string cmdline("DESTDIR=../kaiming/" + d->m_manifest->source + "/files LC_ALL=C.UTF-8 ninja install");
        for (auto& option : installOptions)
        {
            cmdline += " ";
            cmdline += option;
        }
        args.push_back(cmdline);

        break;
    }
    case KMHostModeManifest::BS_Autotools:
    case KMHostModeManifest::BS_Cmake:
    {
        exec = "make";
        args.push_back("install");

        for (auto& option : installOptions)
        {
            args.push_back(option);
        }

        args.push_back("DESTDIR=../kaiming/" + d->m_manifest->source + "/files");

        break;
    }
    case KMHostModeManifest::BS_Qmake:
    {
        exec = "make";
        args.push_back("install");

        for (auto& option : installOptions)
        {
            args.push_back(option);
        }

        args.push_back("INSTALL_ROOT=" + KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming",d->m_manifest->source, "files"));

        break;
    }
    default:
        break;
    }

    std::string cmdline(exec);
    for (auto &arg : args)
    {
        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            std::string quoted = "'" + arg + "'";
            cmdline += " " + quoted;
        }
        else
        {
            cmdline += " " + arg;
        }
    }
    KMInfo("cd " + d->m_cacheDir + " && " + cmdline);

    if (!KMProcessUtils::spawn(exec, d->m_cacheDir, args, false))
    {
        throw KMException("Failed to : " + exec);
    }

    KMTrace("KMHostModeBuild::install invoke end");
}

void KMHostModeBuild::costomBuild()
{
    KMTrace("KMHostModeBuild::costomBuild invoke begin");

    std::string scriptFile = KMBuilderUtils::buildFilename(d->m_cacheDir, "kaiming_build_script.sh");
    std::string content("\nexport DESTDIR=" + KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming",d->m_manifest->source, "files") + "\nexport INSTALL_ROOT=${DESTDIR}\n");
    content += d->m_manifest->build_script;
    if (!KMBuilderUtils::writeScriptToFile(content, scriptFile))
    {
        throw KMException("Fail to create script file: " + scriptFile);
    }

    KMInfo("Running build-script ...");

    if (!KMProcessUtils::spawn(".kaiming-builder/kaiming_build_script.sh", d->m_kmOptions->m_srcDirectory, {}, false))
    {
        throw KMException("Failed to run : .kaiming-builder/kaiming_build_script.sh");
    }

    KMTrace("KMHostModeBuild::costomBuild invoke end");
}

void KMHostModeBuild::postBuild()
{
    if (d->m_manifest->post_build.empty())
    {
        return;
    }

    KMTrace("KMHostModeBuild::postBuild invoke begin");

    std::string scriptFile = KMBuilderUtils::buildFilename(d->m_cacheDir, "kaiming_post-build.sh");
    std::string content("\nexport DESTDIR=" + KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming",d->m_manifest->source, "files") + "\nexport INSTALL_ROOT=${DESTDIR}\n");
    content += d->m_manifest->post_build;
    if (!KMBuilderUtils::writeScriptToFile(content, scriptFile))
    {
        throw KMException("Fail to create script file: " + scriptFile);
    }

    KMInfo("Running post-build script ...");

    if (!KMProcessUtils::spawn(".kaiming-builder/kaiming_post-build.sh", d->m_kmOptions->m_srcDirectory, {}, false))
    {
        throw KMException("Failed to run : .kaiming-builder/kaiming_post-build.sh");
    }

    KMTrace("KMHostModeBuild::postBuild invoke end");
}

void KMHostModeBuild::finish()
{
    KMTrace("KMHostModeBuild::finish invoke begin");

    KMInfo("Finishing ...");

    // 1. cp安装、卸载脚本到安装目录
    installScriptFiles();

    // 2. 生成info.json
    generateMetadata();

    // 3. 生成subpackages.json
    generateSubPackagesJson();

    KMTrace("KMHostModeBuild::finish invoke end");
}

void KMHostModeBuild::installScriptFiles()
{
    std::string srcDir = KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming");
    std::string destDir = KMBuilderUtils::buildFilename(d->m_kmOptions->m_srcDirectory, "kaiming", d->m_manifest->source, "KAIMING");
    KMBuilderUtils::mkpath(destDir);

    std::string scriptsContent("[ -e changelog ] && cp changelog " + KMBuilderUtils::buildFilename(d->m_manifest->source, "KAIMING") + " \n" + 
                               "find . -maxdepth 1 -name \"*.preinst\" -o -name \"*.postinst\" -o -name \"*.prerm\" -o -name \"*.postrm\" | while read -r file; do \n"
                               "    cp -f $file "  + KMBuilderUtils::buildFilename(d->m_manifest->source, "KAIMING") + " \n"
                               "done\n"
                              );
    
    std::string scriptFile = KMBuilderUtils::buildFilename(d->m_cacheDir, "kaiming_install_uninstall.sh");
    if (!KMBuilderUtils::writeScriptToFile(scriptsContent, scriptFile))
    {
        throw KMException("Fail to create script file: " + scriptFile);
    }

    KMInfo("Sync install or uninstall scripts ...");

    if (!KMProcessUtils::spawn(scriptFile, srcDir, {}, false))
    {
        throw KMException("Failed to run : " + scriptFile);
    }
}

void KMHostModeBuild::generateMetadata()
{
    KMInfoJson metadata;
    metadata.id = d->m_manifest->source;
    metadata.name = d->m_manifest->source;
    metadata.archs.push_back(KMBuilderUtils::getArch());
    metadata.kind = "app";
    metadata.channel = "stable";
    metadata.modules.insert({ "binary", ".*" });
    metadata.version = d->m_manifest->version;
    metadata.commands.push_back("bash");
    metadata.license = d->m_manifest->license;
    metadata.homepage = d->m_manifest->homepage;
    metadata.maintainers = d->m_manifest->maintainers;
    metadata.size = 0;
    metadata.annotations.kylin_hostapp = true;

    metadata.saveFile(KMBuilderUtils::buildFilename(d->m_appDir, "info.json"));
}

void KMHostModeBuild::generateSubPackagesJson()
{
    KMSubPackagesJson json;
    for (auto& package : d->m_manifest->packages)
    {
        KMSubPackagesJson::SubPackage subpackage;
        subpackage.id = package.id;
        subpackage.name = package.name;
        subpackage.kind = "app";
        subpackage.description = package.description;
        subpackage.module = "binary";
        subpackage.commands.push_back("bash");
        KMHostModeBuild::collectSubPackagesPath(d->m_appDir + "/files", d->m_appDir + "/files", package.includes, subpackage.includes);

        json.subpackages.push_back(subpackage);
    }

    std::string jsonFile = KMBuilderUtils::buildFilename(d->m_appDir, "subpackages.json");
    json.saveFile(jsonFile);
}

void KMHostModeBuild::exportPackage()
{
    KMTrace("KMHostModeBuild::exportPackage invoke begin");

    std::vector<std::string> args;
    args.push_back("build-export");
    if (d->m_kmOptions->m_isVerbose)
    {
        args.push_back("-v");
    }
    args.push_back("--dest-dir=" + d->m_kmOptions->m_output);
    args.push_back("--bundle-filesystem=" + d->m_kmOptions->m_bundleFilesystem);
    if (!d->m_kmOptions->m_compression.empty())
    {
        args.push_back("--comp=" + d->m_kmOptions->m_compression);
    }
    args.push_back(d->m_appDir);

    if (!KMProcessUtils::spawn("kaiming", args, false))
    {
        throw KMException("Failed to export.");
    }

    KMTrace("KMHostModeBuild::exportPackage invoke end");
}


/**
 * @brief : 收集子开明包包含的路径列表
 * @param : [in] path, 指定路径; files，指定的文件列表
 * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
 * @param : [in] matchs, 匹配规则
 * @param : [out] includes，子开明包包含的路径列表
 */ 
void KMHostModeBuild::collectSubPackagesPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &includes)
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
            KMHostModeBuild::collectSubPackagesPath(subPath, prefix, matchs, includes);
        }
    }
}

