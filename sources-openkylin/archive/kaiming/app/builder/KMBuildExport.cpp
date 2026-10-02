/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildExport.h"

#include <filesystem>
#include <optional>
#include <regex>
#include <unordered_set>

#include "common/KMException.h"
#include "common/KMInfoJson.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/KMJsonHelper.h"
#include "common/KMOKPackager.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMStorageDir.h"
#include "common/kmtranslation.h"
#include "common/KMSubPackagesJson.h"

namespace fs = std::filesystem;

enum RefIdentify
{
    REFIDENTIFY_UNKNOWN,
    REFIDENTIFY_ID,
    REFIDENTIFY_VERSION,
    REFIDENTIFY_MODULE,
    REFIDENTIFY_CHANNEL,
    REFIDENTIFY_MAX
};

class KMBuildExport::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();
    bool parseRef(const std::string &id);
    std::string getDirFromRef();
    
public:
    // application options
    bool m_help = false;
    bool m_refMode = false;
    std::string m_destDir;
    std::string m_bundleFilesystem{ BUNDLE_FILESYSTE_SQUASHFS };
    std::string m_compression;

    // position options
    std::string m_directory;

    //app information
    std::string m_directory_ref;
    std::string m_ref;
    std::string m_id;
    KMRef m_app;
};

std::string KMBuildExport::Options::getDirFromRef()
{
    m_ref = m_directory_ref;
    if (!parseRef(m_ref))
    {
        KMError("Can't parse id: " + m_ref);
        showUsage();
        exit(EXIT_FAILURE);
    }
    else
    {
        KMInstalledAppQuery appQuery;
        std::vector<KMRef> refs = appQuery.channel(m_app.channel)
                            .arch(m_app.arch)
                            .kind(m_app.kind)
                            .id(m_app.id)
                            .module(m_app.module)
                            .version(m_app.version)
                            .query(KMInstalledAppQuery::All);
        if (refs.empty())
        {
            KMError("Can't find ref:" + m_ref);
            exit(EXIT_FAILURE);
        }
        KMRef ref = refs.front();
        m_directory = KMStorageDir::getDeployedDir(ref);
        return m_directory;
    }

}

bool KMBuildExport::Options::parseRef(const std::string &id)
{
    regex pattern(R"(^([^\/]*)(?:\/([^\/]*))?(?:\/([^\/]*))?(?:\/([^\/]*))?$)");
    smatch matches{};
    if (!regex_match(id, matches, pattern))
    {
        return false;
    }

    if (matches.size() < REFIDENTIFY_MAX)
    {
        return false;
    }

    if (!regex_match(id, matches, pattern))
    {
        KMError("appid does not match the rule");
        return false;
    }
    m_app.id = matches[REFIDENTIFY_ID].str();
    m_app.version = matches[REFIDENTIFY_VERSION].str();
    m_app.module = matches[REFIDENTIFY_MODULE].str();
    m_app.channel = matches[REFIDENTIFY_CHANNEL].str();

    KMDebug("id: " + m_app.id + " version: " + m_app.version + " module:" + m_app.module + " channel:" + m_app.channel);
    return true;
}

void KMBuildExport::Options::preParseHook()
{
    addOptions();
}

void KMBuildExport::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    //ref_mode与m_directory区分
    if (m_refMode)
    {
        m_directory = getDirFromRef();
    }
    else
    {
        m_directory = m_directory_ref;
    }

    if (m_directory.empty())
    {
        KMError(_("DIRECTORY must be specified"));
        showUsage();
        exit(EXIT_FAILURE);
    }

    if (!fs::exists(m_directory))
    {
        KMError(m_directory + " doesn't exists.");
        exit(EXIT_FAILURE);
    }

    m_directory = fs::canonical(m_directory);
}

void KMBuildExport::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming build-export [OPTION…] (DIRECTORY | REF) \n"));

    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_help), _("Show help options"));

    //应用选项
    addOption("dest-dir","d", KMOption::value<std::string>(&m_destDir)->defaultValue(fs::absolute(fs::current_path()).string()), _("The directory to save the ok package. Default current working directory."));
    addOption("bundle-filesystem","", KMOption::value(&m_bundleFilesystem), _("The filesystem for bundle. Such as squashfs、erofs，default squashfs."));
    addOption("ref-mode","",KMOption::value<bool>(&m_refMode), _("Use the ref pattern."));
    addOption("comp","", KMOption::value<std::string>(&m_compression), _("--comp=<comp [extra options]>. Select <comp> compression, squashfs filesystem's <comp> see command mksquashfs, for example : --comp=xz or --comp='zstd -Xcompression-level 18'; erofs filesystem's <comp> see command mkfs.erofs, for example : --comp=lz4hc"));
    
    //位置选项
    addPositionOption("DIRECTORY", KMOption::value<std::string>(&m_directory_ref), 1, _("The build dir"));
    addPositionOption("REF", KMOption::value<std::string>(&m_directory_ref), -1, _("The ref type is supported：\n \tid\n \tid/version\n \tid/version/module\n \tid/version/module/channel"));
}

class KMBuildExport::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;
    std::string m_filesDir;
    std::string m_infoJsonFile;
    std::shared_ptr<KMInfoJson> m_infoJson;
    std::string m_modulesJsonFile;
    std::map<std::string, std::string> m_modules;
    std::string m_subpackagesJsonFile;
    std::shared_ptr<KMSubPackagesJson> m_subpackagesJson;

    KMRef m_baseRef;
    KMRef m_runtimeRef;
    std::shared_ptr<KMDeploy> m_baseDeploy;
    std::shared_ptr<KMDeploy> m_runtimeDeploy;
};

REGISTER_SUBCOMMAND_DYNCREATE(build-export, KMBuildExport)

KMBuildExport::KMBuildExport()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuildExport::~KMBuildExport() = default;

int KMBuildExport::dispose(int argc, char **argv)
{
    KMTrace("KMBuildExport::dispose invoke begin");

    init(argc, argv);

    int ret = run();

    KMTrace("KMBuildExport::dispose invoke end");
    return ret;
}

/**
 * @brief : modules.json的反序列化 
 * @param : jsonFile modules.json的路径
 */
static std::map<std::string, std::string> getModulesFromJson(const std::string &jsonFile)
{
    nlohmann::json m_root = KMJsonHelper::loadFile(jsonFile);

    KMJsonHelper helper(m_root);
    return helper.getStringMapValue("modules");
}

void KMBuildExport::parseSubpackagesJson()
{
    d->m_subpackagesJson = KMSubPackagesJson::loadFile(d->m_subpackagesJsonFile);
}

void KMBuildExport::init(int argc, char **argv)
{
    KMTrace("KMBuildExport::init invoke begin");

    d->m_kmOptions->parseCommandLine(argc, argv);

    d->m_filesDir = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "files");
    d->m_infoJsonFile = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "info.json");
    if (!fs::exists(d->m_filesDir) || !fs::exists(d->m_infoJsonFile))
    {
        throw KMException(d->m_kmOptions->m_directory + _(" directory dose not include files and info.json"));
    }
    if (KMFileUtils::isEmptyDir(d->m_filesDir))
    {
        throw KMException(d->m_filesDir + _(" is empty."));
    }

    d->m_infoJson = std::make_shared<KMInfoJson>();
    d->m_infoJson->loadFile(d->m_infoJsonFile);

    d->m_subpackagesJsonFile = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "subpackages.json");
    d->m_modulesJsonFile = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "modules.json");
    // subpackages.json拆包功能和--appimage选项不能与--all选项同时使用
    if (fs::exists(d->m_subpackagesJsonFile))
    {
        parseSubpackagesJson();
    }
    else if (fs::exists(d->m_modulesJsonFile))
    {
        d->m_modules = getModulesFromJson(d->m_modulesJsonFile);
    }
    else
    {
        d->m_modules[d->m_infoJson->module] = ".*";
    }

    KMTrace("KMBuildExport::init invoke end");
}

void KMBuildExport::generateSubpackages()
{
    KMTrace("KMBuildExport::generateSubpackages invoke begin");

    for (auto& subpackage : d->m_subpackagesJson->subpackages)
    {
        KMOKPackager packager(d->m_kmOptions->m_destDir);
        
        packager.setBundleFilesystem(d->m_kmOptions->m_bundleFilesystem);
        packager.setCompression(d->m_kmOptions->m_compression);
        packager.appendLayer({ d->m_kmOptions->m_directory, d->m_infoJson });
        packager.setSubpackageMode(d->m_subpackagesJson, subpackage);

        std::string okFilename = subpackage.id + "-" + 
                                d->m_infoJson->archs.at(0) + "-" + 
                                d->m_infoJson->version + "-" + 
                                d->m_infoJson->channel + "-" + 
                                subpackage.module + "-" + 
                                d->m_kmOptions->m_bundleFilesystem;
        okFilename += ".ok";

        packager.pack(okFilename);
    }

    KMTrace("KMBuildExport::generateSubpackages invoke end");
}

void KMBuildExport::generatePackagesByModules()
{
    KMTrace("KMBuildExport::generatePackagesByModules invoke begin");

    for (const auto& [key, modulePaths] : d->m_modules)
    {
        KMOKPackager packager(d->m_kmOptions->m_destDir);
        
        packager.setBundleFilesystem(d->m_kmOptions->m_bundleFilesystem);
        packager.setCompression(d->m_kmOptions->m_compression);

        d->m_infoJson->module = key;
        packager.setModulePaths(key,modulePaths);

        packager.appendLayer({ d->m_kmOptions->m_directory, d->m_infoJson });

        std::string okFilename = d->m_infoJson->id + "-" + 
                                d->m_infoJson->archs.at(0) + "-" + 
                                d->m_infoJson->version + "-" + 
                                d->m_infoJson->channel + "-" + 
                                d->m_infoJson->module + "-" + 
                                d->m_kmOptions->m_bundleFilesystem;
        okFilename += ".ok";
        packager.pack(okFilename);
    }

    KMTrace("KMBuildExport::generatePackagesByModules invoke end");
}

int KMBuildExport::run()
{
    KMTrace("KMBuildExport::run invoke begin");

    KMFileUtils::mkpath(d->m_kmOptions->m_destDir);

    if (d->m_subpackagesJson)
    {
        generateSubpackages();
    }
    else
    {
        generatePackagesByModules();
    }

    KMTrace("KMBuildExport::run invoke end");
    return EXIT_SUCCESS;
}
