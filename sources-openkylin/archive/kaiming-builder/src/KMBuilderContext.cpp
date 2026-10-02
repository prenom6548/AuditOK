/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderContext.h"

#include <cstdlib>
#include <iostream>
#include <memory>

#include "KMBuilderUtils.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "config.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"

class KMBuilderContext::Options: public KMOption::Options
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
    std::string m_arch;
    bool m_isCcache = false;
    bool m_isBuildOnly = false;
    bool m_isFinishOnly = false;
    bool m_isExportOnly = false;
    bool m_isForceClean = false;
    bool m_isSandbox = false;
    std::string m_stopAt;
    int m_jobs = -1;
    bool m_isKeepBuildDirs = false;
    bool m_isExport = true;
    bool m_isExportAll = false;
    bool m_appimage = false;
    std::string m_manifest;
    std::string m_programName;
    std::string m_bundleFilesystem{ "squashfs" };
    std::string m_output;
    std::string m_compression;
};

void KMBuilderContext::Options::preParseHook()
{
    addOptions();
}

void KMBuilderContext::Options::postParseHook()
{
    if(m_isHelp)
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
    m_output = fs::canonical(m_output).string();
}

void KMBuilderContext::Options::addOptions()
{
    setDescription(_("\nUsage\n \tkaiming-builder build [OPTION…] MANIFEST\n"));

    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_isHelp), _("Show help options"));
    //应用选项
    addOption("verbose","v", KMOption::value(&m_isVerbose), _("Print debug information during command processing"));
    addOption("arch","", KMOption::value<std::string>(&m_arch), _("Architecture to build for (must be host compatible)"));
    addOption("ccache","", KMOption::value<bool>(&m_isCcache), _("Use ccache"), true);
    addOption("build-only", "",KMOption::value<bool>(&m_isBuildOnly), _("Stop after build、clean, don't run finish, export phases"));
    addOption("finish-only", "",KMOption::value<bool>(&m_isFinishOnly), _("Only run finish, export phases"));
    addOption("export-only", "",KMOption::value<bool>(&m_isExportOnly), _("Only run export phase."));
    addOption("export", "",KMOption::value<bool>(&m_isExport)->defaultValue(true), _("Generate kaiming package , Only export this project."), true);
    addOption("export-all", "",KMOption::value<bool>(&m_isExportAll), _("Generate kaiming package, include base、runtime、app, etc."));
    addOption("appimage","",KMOption::value<bool>(&m_appimage), _("Generate kaiming package, which like appimage."), true);
    addOption("runnable","",KMOption::value<bool>(&m_appimage), _("Generate kaiming package, which like appimage."));
    addOption("bundle-filesystem", "",KMOption::value(&m_bundleFilesystem), _("The filesystem of the kaiming package's bundle section. Such as squashfs、erofs. Default squashfs."));
    addOption("force-clean","f", KMOption::value<bool>(&m_isForceClean), _("Erase previous contents of DIRECTORY"));
    addOption("keep-source-dirs","k",KMOption::value<bool>(&m_isKeepBuildDirs), _("Don't delete source directories after build"));
    addOption("stop-at","", KMOption::value<std::string>(&m_stopAt), _("COMPONENTNAME. Stop building at this component (implies --build-only)"));
    addOption("jobs", "",KMOption::value<int>(&m_jobs), _("Number of parallel jobs to build (default=NCPU)"));
    addOption("output","o", KMOption::value<std::string>(&m_output), _("The path for output of build, default $(pwd)"));
    addOption("comp","", KMOption::value<std::string>(&m_compression), _("--comp=<comp [extra options]>. Select <comp> compression, squashfs filesystem's <comp> see command mksquashfs, for example : --comp=xz or --comp='zstd -Xcompression-level 18'; erofs filesystem's <comp> see command mkfs.erofs, for example : --comp=lz4hc"));

    //位置选项
    addPositionOption("MANIFEST", KMOption::value<std::string>(&m_manifest), 1,_("The MANIFEST file for building, support .km.yml/.km.yaml"));
}

class KMBuilderContext::Private
{
public:

    std::unique_ptr<Options>m_kmOptions;
    int m_argc;
    char **m_argv;

    std::shared_ptr<KMBuilderManifest> m_manifestConfig;

    // 构建过程中涉及的目录
    fs::path m_baseDir;
    fs::path m_appDir;
    fs::path m_runDir;
    fs::path m_stateDir;
    fs::path m_cacheDir;
    fs::path m_downloadsDir;
    fs::path m_checksumDir;
    fs::path m_ccacheDir;
    fs::path m_buildDir;
    std::string m_configFile;
};

KMBuilderContext::KMBuilderContext(token)
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuilderContext::~KMBuilderContext()
{

}

void KMBuilderContext::checkOptions()
{
    bool isEmptyAppDir = KMBuilderUtils::isEmptyDirectory(getAppDir());

    if (isExportOnly() || isFinishOnly())
    {
        if (isEmptyAppDir)
        {
            throw KMException("App dir '" + getAppDir() + "' is empty or doesn't exist.");
        }
    }
    else
    {
        if (!isEmptyAppDir)
        {
            if (isForceClean())
            {
                KMDebug("Emptying app dir : " + getAppDir());
                KMBuilderUtils::rmRfAll(getAppDir());
                KMBuilderUtils::rmRfAll(getCacheDir());
                KMBuilderUtils::rmRfAll(getBuildDir());
                KMBuilderUtils::rmRfAll(getDownloadsDir());
            }
            else
            {
                // throw KMException("App dir '" + getAppDir() + "' is not empty. Please delete the existing contents or use --force-clean.");
            }
        }
        else
        {
            KMDebug("Emptying app dir : " + getAppDir());
            KMBuilderUtils::rmRfAll(getAppDir());
            KMBuilderUtils::rmRfAll(getCacheDir());
            KMBuilderUtils::rmRfAll(getBuildDir());
            KMBuilderUtils::rmRfAll(getDownloadsDir());

            d->m_kmOptions->m_isForceClean = true;
        }
    }
}

std::string KMBuilderContext::getArch() const
{
    return d->m_kmOptions->m_arch.length() > 0 ? d->m_kmOptions->m_arch : KMBuilderUtils::getArch();
}

void KMBuilderContext::showCmdline() const
{
    std::cout << "cmdline:";
    for (int i = 0; i < d->m_argc; ++i)
    {
        std::cout << " ";
        std::cout << d->m_argv[i];
    }
    std::cout << std::endl;
}

void KMBuilderContext::parseCommandLine(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);

    if (d->m_kmOptions->m_isVerbose)
        km_logger.setLogLevel(trace);

    if (d->m_kmOptions->m_manifest.empty())
    {
        throw KMException("Must input MANIFEST!");
    }

    // 获取编译 base 路径（配置文件所在文件夹）
    d->m_configFile = fs::absolute(d->m_kmOptions->m_manifest).string();

    d->m_baseDir =fs::absolute(d->m_kmOptions->m_output).string();
    d->m_appDir = d->m_baseDir / "build";
    d->m_runDir = d->m_baseDir;
    d->m_stateDir = d->m_runDir / STATE_DIR;
    d->m_buildDir = d->m_stateDir / "sources";
    d->m_cacheDir = d->m_stateDir / "cache";
    d->m_ccacheDir = d->m_stateDir / "ccache";
    d->m_downloadsDir = d->m_stateDir / "downloads";
    d->m_checksumDir = d->m_stateDir / "checksum";

    checkOptions();
}

/**
 * @brief : 加载配置文件 yaml/yml
 */
void KMBuilderContext::loadConfig()
{
    d->m_manifestConfig = KMBuilderManifest::loadFromYAML(d->m_configFile);
}

bool KMBuilderContext::isBuildOnly() const { return d->m_kmOptions->m_isBuildOnly; }

bool KMBuilderContext::isExportOnly() const { return d->m_kmOptions->m_isExportOnly; }

bool KMBuilderContext::isFinishOnly() const { return d->m_kmOptions->m_isFinishOnly; }

bool KMBuilderContext::isForceClean() const { return d->m_kmOptions->m_isForceClean; }

bool KMBuilderContext::isExport() const { return d->m_kmOptions->m_isExport; }

bool KMBuilderContext::isExportAll() const { return d->m_kmOptions->m_isExportAll; }

bool KMBuilderContext::isExportAppImage() const { return d->m_kmOptions->m_appimage; }

bool KMBuilderContext::isCcache() const { return d->m_kmOptions->m_isCcache; }

bool KMBuilderContext::isVerbose() const { return d->m_kmOptions->m_isVerbose; }

std::string KMBuilderContext::getManifestPath() const { return d->m_configFile; }

std::string KMBuilderContext::getBaseDir() const { return d->m_baseDir.string(); }

std::string KMBuilderContext::getAppDir() const { return d->m_appDir.string(); }

std::string KMBuilderContext::getRunDir() const { return d->m_runDir.string(); }

std::string KMBuilderContext::getStateDir() const { return d->m_stateDir.string(); }

std::string KMBuilderContext::getCcacheDir() const { return d->m_ccacheDir.string(); }

std::string KMBuilderContext::getCacheDir() const { return d->m_cacheDir.string(); }

std::string KMBuilderContext::getBuildDir() const { return d->m_buildDir.string(); }

std::string KMBuilderContext::getDownloadsDir() const { return d->m_downloadsDir.string(); }

bool KMBuilderContext::getIsKeepBuildDirs() { return d->m_kmOptions->m_isKeepBuildDirs; }

int KMBuilderContext::getJobs() const { return d->m_kmOptions->m_jobs; }

std::string KMBuilderContext::getStopat() const { return d->m_kmOptions->m_stopAt; }

std::shared_ptr<KMBuilderManifest> KMBuilderContext::manifestConfig() { return d->m_manifestConfig; }

std::string KMBuilderContext::getBundleFilesystem() const { return d->m_kmOptions->m_bundleFilesystem; }

std::string KMBuilderContext::getCompression() const { return d->m_kmOptions->m_compression; }
