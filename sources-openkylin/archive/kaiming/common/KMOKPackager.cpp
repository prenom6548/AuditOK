/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOKPackager.h"

#include <uuid/uuid.h>
#include <iostream>
#include <fstream>

#include "KMConfig.h"
#include "KMException.h"
#include "KMLogger.h"
#include "KMUtil.h"
#include "KMStringUtils.h"
#include "KMProcessUtils.h"
#include "KMFileUtils.h"
#include "KMDirMd5sum.h"
#include "KMInstalledAppQuery.h"
#include "KMStorageDir.h"

class KMOKPackager::Private
{
public:
    std::string m_buildDir;
    std::string m_cacheDir;
    std::string m_okPackage;
    std::string m_module;
    std::vector<std::string> m_modulePaths;
    KMSubPackagesJson::SubPackage m_subpackage;
    std::shared_ptr<KMSubPackagesJson> m_subpackagesJson;

    std::string m_bundleFilesystem{ BUNDLE_FILESYSTE_EROFS };
    std::string m_compression;
    std::vector<Layer> m_layers;

    KMOABMetadata m_metadata;
    bool m_largerThan10M = false;
};

KMOKPackager::KMOKPackager(const std::string &buildDir)
    : d(std::make_shared<Private>())
{
    d->m_buildDir = buildDir;
    d->m_cacheDir = KMStringUtils::buildFilename(buildDir, ".kaiming-builder/package");
}

KMOKPackager::~KMOKPackager()
{}

void KMOKPackager::setBundleFilesystem(const std::string &filesystem)
{
    d->m_bundleFilesystem = filesystem;
}

void KMOKPackager::setCompression(const std::string &compression)
{
    d->m_compression = compression;
}

void KMOKPackager::setModulePaths(const std::string &moduleName, const std::string& paths)
{
    if (paths != "*" || paths != ".*")
    {
        std::vector<std::string> splits = KMStringUtils::splitString(paths,"\n");

        if (!KMStringUtils::contains(splits, "*") && !KMStringUtils::contains(splits, ".*"))
        {
            d->m_module = moduleName;
            d->m_modulePaths = KMStringUtils::splitString(paths,"\n");
        }
    }
}

void KMOKPackager::setSubpackageMode(std::shared_ptr<KMSubPackagesJson> subpackagesJson, const KMSubPackagesJson::SubPackage& subpackage)
{
    d->m_subpackagesJson = subpackagesJson;
    d->m_subpackage = subpackage;
}

void KMOKPackager::appendLayer(Layer layer)
{
    d->m_layers.push_back(layer);
}

void KMOKPackager::pack(const std::string &okFilename)
{
    KMTrace("KMOKPackager::pack invoke begin");

    std::string okEngine = KMStringUtils::buildFilename(KAIMING_TOOLS_BIN_PATH, "ok-engine");
    if (!fs::exists(okEngine))
    {
        throw KMException(okEngine + " is missing.");
    }

    if (!KMFileUtils::isEmptyDirectory(d->m_cacheDir))
    {
        // KMFileUtils::rmRfAll(d->m_cacheDir);
        fs::remove_all(d->m_cacheDir);
        KMInfo("clear cache : " + d->m_cacheDir);
    }
    KMInfo("mkpath : " + d->m_cacheDir);
    KMFileUtils::mkpath(d->m_cacheDir);

    d->m_okPackage = KMStringUtils::buildFilename(d->m_cacheDir, okFilename);
    KMDebug("delete old " + d->m_okPackage);
    std::error_code ec;
    if (fs::exists(d->m_okPackage) && !fs::remove(d->m_okPackage, ec))
    {
        throw KMException(ec.message());
    }

    KMDebug("copy file " + okEngine + " to " + d->m_okPackage);
    if (!fs::copy_file(okEngine, d->m_okPackage, ec))
    {
        throw KMException(ec.message());
    }

    packBundle();

    packMetadata();

    // 从缓存目录移动到目标目录
    std::string okPackage = KMStringUtils::buildFilename(d->m_buildDir, okFilename);
    if (fs::exists(okPackage) && !fs::remove(okPackage, ec))
    {
        throw KMException(ec.message());
    }

    std::string result;
    if (!KMProcessUtils::spawn("mv", { d->m_okPackage, okPackage }, result))
    {
        throw KMException(result);
    }

    fs::permissions(okPackage, fs::perms::all);

    KMFileUtils::rmRfAll(d->m_cacheDir);

    KMInfo("Packaging completed. Generate file: " + okPackage);

    KMTrace("KMOKPackager::pack invoke end");
}

void KMOKPackager::addNewSection(const std::string &sectionName, const std::string &dataFile)
{
    std::string result;
    std::vector<std::string> args;
    args.push_back("--add-section");
    args.push_back(sectionName + "=" + dataFile);
    args.push_back(d->m_okPackage);
    args.push_back(d->m_okPackage);

    if (!KMProcessUtils::spawn("objcopy", args, result))
    {
        throw KMException(result);
    }
    else if (!result.empty())
    {
        KMDebug(result);
    }
}

void KMOKPackager::packBundle()
{
    KMTrace("KMOKPackager::packBundle invoke begin");

    KMInfo("Packaging bundle ...");

    d->m_largerThan10M = false;

    std::string bundleDir = KMStringUtils::buildFilename(d->m_cacheDir, "bundle");
    std::error_code ec;
    if (!KMFileUtils::mkpath(bundleDir, ec))
    {
        throw KMException(ec.message());
    }

    if (d->m_subpackagesJson)
    {
        prepareBundleBySubpackageMode(bundleDir);
    }
    else
    {
        prepareBundle(bundleDir);
    }

    std::string bundleFile;
    if (d->m_bundleFilesystem == BUNDLE_FILESYSTE_SQUASHFS)
    {
        bundleFile = KMStringUtils::buildFilename(d->m_cacheDir, "bundle.squashfs");
        generateSquashfsBundle(bundleFile, bundleDir);
    }
    else //if (d->m_bundleFilesystem == BUNDLE_FILESYSTE_EROFS)
    {
        bundleFile = KMStringUtils::buildFilename(d->m_cacheDir, "bundle.ef");
        generateErofsBundle(bundleFile, bundleDir);
    }

    // calculate digest
    std::string digest = KMFileUtils::getFileSha256sum(bundleFile);
    if (digest.empty())
    {
        throw KMException("Failed to sha256sum " + bundleFile);
    }
    d->m_metadata.digest = digest;

    std::string bundleSection(KMOABMETADATA_PREFIX ".bundle");
    addNewSection(bundleSection, bundleFile);

    d->m_metadata.sections.bundle = bundleSection;

    KMTrace("KMOKPackager::packBundle invoke end");
}

static void filterSubPathsForSplitPackage(const std::string& rootPath, const std::vector<std::string>& paths, const std::string& outputFile, const std::vector<std::string>& appendPaths) 
{
    std::ofstream outFile(outputFile, std::ios::trunc);

    if (!outFile.is_open()) 
    {
        throw KMException("Failed to open file: " + outputFile);
    }

    for (const auto& path : paths) 
    {
        fs::path targetPath = KMStringUtils::buildFilename(rootPath, "files", path);

        if (fs::exists(targetPath)) 
        {
            std::string relativePath = fs::relative(targetPath, rootPath).string();
            outFile << relativePath << "\n";
        } 
        else 
        {
            std::regex regexPattern(path);

            for (const auto& entry : fs::recursive_directory_iterator(rootPath + "/files")) 
            {
                if (std::regex_match(entry.path().string(), regexPattern)) 
                {
                    std::string relativePath = fs::relative(entry.path(), rootPath).string();
                    outFile << relativePath << "\n";
                }
            }
        }
    }

    for (auto& append : appendPaths)
    {
        outFile << append << "\n";
    }

    outFile.close();
}

void KMOKPackager::prepareBundle(const std::string &bundleDir)
{
    KMTrace("KMOKPackager::prepareBundle invoke begin");

    std::error_code ec;
    std::string layersDir = KMStringUtils::buildFilename(bundleDir, "layers");
    if (!KMFileUtils::mkpath(layersDir, ec))
    {
        throw KMException(ec.message());
    }
    for (Layer layer : d->m_layers)
    {
        KMInfo("Copying " + layer.m_info->id + " ...");

        if (layer.m_info->kind == BASE_TYPE_APP && !d->m_module.empty())
        {
            std::string appDir = KMStringUtils::buildFilename(layersDir, layer.m_info->id, layer.m_info->module);
            if (!KMFileUtils::mkpath(appDir, ec))
            {
                throw KMException(ec.message());
            }

            std::vector<std::string> appendPaths;
            std::vector<std::string> args;
            args.push_back("-rlptH");
            args.push_back("--info=progress2");
            args.push_back("--ignore-errors");
            args.push_back("--exclude=entries/etc/ld.so.cache");
            args.push_back("--exclude=files/etc/ld.so.cache");
            if (layer.m_info->module == "binary")
            {
                appendPaths.push_back("entries");
            }
            else
            {
                layer.m_info->commands.clear();
            }
            std::string modulePathsFile = KMStringUtils::buildFilename(d->m_cacheDir, "paths.txt");
            filterSubPathsForSplitPackage(layer.m_layerDir, d->m_modulePaths, modulePathsFile, appendPaths);
            args.push_back("--files-from=" + modulePathsFile);
            args.push_back(layer.m_layerDir + "/");
            args.push_back(appDir + "/");

            if (!KMProcessUtils::spawn("rsync", args, true))
            {
                throw KMException("Failed to copy " + layer.m_layerDir + " to " + appDir);
            }
            KMInfo("\nSyncing, please wait a moment ...\n");
            KMProcessUtils::spawn("sync", {}, true);

            std::string metadataFile = KMStringUtils::buildFilename(appDir, "info.json");
            layer.m_info->size = KMFileUtils::getDirectorySize(fs::path(appDir));
            layer.m_info->saveFile(metadataFile);

            // 更新更精确
            layer.m_info->loadFile(metadataFile);
            layer.m_info->size = KMFileUtils::getDirectorySize(fs::path(appDir));
            layer.m_info->saveFile(metadataFile);
            if (layer.m_info->size > 10*1024*1024)
            {
                d->m_largerThan10M = true;
            }

            // 求md5校验和
            KMDirMd5sum md5sum(appDir);
            md5sum.md5sum("files");
        }
        else
        {
            std::string appDir = KMStringUtils::buildFilename(layersDir, layer.m_info->id, layer.m_info->module);
            if (!KMFileUtils::mkpath(appDir, ec))
            {
                throw KMException(ec.message());
            }

            std::vector<std::string> args;
            args.push_back("-rlptH");
            args.push_back("--info=progress2");
            args.push_back("--ignore-errors");
            args.push_back("--exclude=deploy");
            args.push_back("--exclude=modules.json");
            args.push_back("--exclude=entries/etc/ld.so.cache");
            args.push_back("--exclude=files/etc/ld.so.cache");
            args.push_back(layer.m_layerDir + "/");
            args.push_back(appDir + "/");
            if (!KMProcessUtils::spawn("rsync", args, true))
            {
                throw KMException("Failed to copy " + layer.m_layerDir + " to " + appDir);
            }

            KMInfo("\nSyncing, please wait a moment ...\n");
            KMProcessUtils::spawn("sync", {}, true);

            // 求md5校验和
            KMDirMd5sum md5sum(appDir);
            md5sum.md5sum("files");

            std::string metadataFile = KMStringUtils::buildFilename(appDir, "info.json");
            layer.m_info->size = KMFileUtils::getDirectorySize(fs::path(appDir));
            layer.m_info->saveFile(metadataFile);
            if (layer.m_info->size > 10*1024*1024)
            {
                d->m_largerThan10M = true;
            }
        }

        d->m_metadata.layers.push_back(*(layer.m_info));
    }

    KMProcessUtils::spawn("sync", {}, true);

    KMTrace("KMOKPackager::prepareBundle invoke end");
}

void KMOKPackager::copyChangelogToBundle(const std::string& appDir)
{
    KMTrace("KMOKPackager::copyChangelogToBundle invoke begin");

    Layer layer = d->m_layers.at(0);
    std::string changelog = KMStringUtils::buildFilename(layer.m_layerDir, "KAIMING/changelog");
    if (fs::exists(changelog))
    {
        std::string changelogDir = KMStringUtils::buildFilename(appDir, "KAIMING");
        KMFileUtils::mkpath(changelogDir);

        std::error_code ec;
        const auto copyOptions = fs::copy_options::overwrite_existing;
        std::string changelogFile = KMStringUtils::buildFilename(changelogDir, "changelog");
        fs::copy_file(changelog, changelogFile, copyOptions, ec);
    }

    KMTrace("KMOKPackager::copyChangelogToBundle invoke end");
}

void KMOKPackager::recordDependsInfo(const std::string& appDir, const std::vector<std::string>& appDepends)
{
    if (appDepends.empty())
    {
        return ;
    }

    std::string kaimingDir = appDir + "/KAIMING";
    std::error_code ec;
    if (!KMFileUtils::mkpath(kaimingDir, ec))
    {
        throw KMException(ec.message());
    }

    std::string dependsFile = kaimingDir + "/depends";

    std::ofstream o(dependsFile);
    for (auto& depend : appDepends)
    {
        o << depend << std::endl;
    }
}

void KMOKPackager::copyDependsToBundle(const std::vector<std::string>& depends, const std::string& appDir, std::vector<std::string>& appDepends, std::set<std::string>& noRepeat)
{
    // 用于记录递归深度
    static int s_depth = 0;
    int depth = ++s_depth;

    KMInstalledAppQuery query;
    for (auto& depend : depends)
    {
        if (depend.empty())
        {
            continue;
        }

        std::vector<std::string> splits = KMStringUtils::splitString(depend, "/");
        if (splits.size() == 1)
        {
            splits.emplace_back("");
        }
        std::string dependId = splits.at(0);
        std::string dependVersion = splits.at(1);

        if (dependId.empty())
        {
            continue;
        }

        // 去重
        if (noRepeat.count(dependId) > 0)
        {
            continue;
        }

        noRepeat.insert(dependId);

        // 判断是否本次拆包的兄弟包
        bool siblings = false;
        for (auto& subpackage : d->m_subpackagesJson->subpackages)
        {
            if (subpackage.id == dependId)
            {
                siblings = true;

                if (!subpackage.depends.empty() && subpackage.kind == BASE_TYPE_DEPEND)
                {
                    copyDependsToBundle(subpackage.depends, appDir, appDepends, noRepeat);
                    s_depth = depth; 
                }

                if (subpackage.kind == BASE_TYPE_DEPEND)
                {
                    if (depth == 1)
                    {
                        appDepends.push_back("[direct depend]: " + subpackage.id + "/" + subpackage.version);
                    }
                    else
                    {
                        appDepends.push_back("[nested depend]: " + subpackage.id + "/" + subpackage.version);
                    }
                }

                break;
            }
        }
        if (siblings)
        {
            // 兄弟包数据已经在subpackage.includes中拷贝，无需后面的depend查询、拷贝逻辑

            continue;
        }

        std::vector<KMRef> refs = query.arch(KMUtil::kmGetArch())
                                    .kind(BASE_TYPE_DEPEND)
                                    .id(dependId)
                                    .module(MODULE_NAME_DEV)
                                    .version(dependVersion)
                                    .query(KMInstalledAppQuery::User);
        if (refs.empty())
        {
            std::string name = dependId + "/" + dependVersion + "/" + MODULE_NAME_DEV; 
            throw KMException("Please install it manually: sudo kaiming install " + name);
        }

        // 加载depend的info.json
        KMRef& ref = refs.at(0);
        std::string deployedDir = KMStorageDir::getDeployedDir(ref);
        std::string infoJsonFile = KMStringUtils::buildFilename(deployedDir, "info.json");
        KMInfoJson infoJson;
        infoJson.loadFile(infoJsonFile);
        if (!infoJson.annotations.depends.empty())
        {
            copyDependsToBundle(infoJson.annotations.depends, appDir, appDepends, noRepeat);
            s_depth = depth; 
        }

        std::string target("files");
        // if (infoJson.annotations.sysapp)
        // {
        //     target = "depends";
        // }

        std::string srcDir = KMStringUtils::buildFilename(deployedDir, "files");
        std::string destDir = KMStringUtils::buildFilename(appDir, target);
        KMFileUtils::mkpath(destDir);
        // KMFileUtils::rsyncDir(srcDir, destDir);

        fs::path dependPath(srcDir);
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
                KMFileUtils::cpRfAll(p.string(), destDir);
            }
            else if (filename == "usr")
            {
                KMFileUtils::rsyncDir(p.string(), destDir);
            }
            else
            {
                std::string dest = KMStringUtils::buildFilename(destDir, filename);
                KMFileUtils::mkpath(dest);

                std::vector<std::string> excludes;
                excludes.push_back("ld.so.cache");
                KMFileUtils::rsyncDir(p.string(), dest, excludes);
            }
        }

        KMProcessUtils::spawn("sync", {}, true);
        
        if (depth == 1)
        {
            appDepends.push_back("[direct depend]: " + ref.id + "/" + ref.version);
        }
        else
        {
            appDepends.push_back("[nested depend]: " + ref.id + "/" + ref.version);
        }
    }
}

void KMOKPackager::copyIncludesToBundle(const std::string& srcDir, const std::string& destDir)
{
    KMTrace("KMOKPackager::copyIncludesToBundle invoke begin");

    std::string includesFile = KMStringUtils::buildFilename(d->m_cacheDir, "subpackage.includes");
    std::string content = KMStringUtils::join(d->m_subpackage.includes, "\n");
    KMFileUtils::syncWriteFile(includesFile, content);

    std::vector<std::string> args;
    args.push_back("-rlptH");
    args.push_back("--info=progress2");
    args.push_back("--ignore-errors");
    args.push_back("--files-from=" + includesFile);
    args.push_back(srcDir + "/");
    args.push_back(destDir + "/");

    if (!KMProcessUtils::spawn("rsync", args, true))
    {
        throw KMException("Failed to copy " + srcDir + " to " + destDir);
    }

    KMTrace("KMOKPackager::copyIncludesToBundle invoke end");
}

void KMOKPackager::exportEntries(const std::string& appDir)
{
    KMTrace("KMOKPackager::exportEntries invoke begin");

    if (d->m_subpackage.kind == PACKGE_KIND_APP)
    {
        if (!KMProcessUtils::spawn("kaiming", { { "build-finish", appDir } }))
        {
            throw KMException("Failed to build-finish " + appDir);
        }
    }

    KMTrace("KMOKPackager::exportEntries invoke end");
}

void KMOKPackager::amendLayer(Layer& layer)
{
    layer.m_info->id = d->m_subpackage.id;
    layer.m_info->name = d->m_subpackage.name;
    layer.m_info->names = d->m_subpackage.names;
    layer.m_info->kind = d->m_subpackage.kind;
    layer.m_info->description = d->m_subpackage.description;
    layer.m_info->descriptions = d->m_subpackage.descriptions;
    layer.m_info->module = d->m_subpackage.module;
    layer.m_info->commands = d->m_subpackage.commands;
    if (layer.m_info->kind == PACKGE_KIND_DEPEND)
    {
        layer.m_info->annotations.depends = d->m_subpackage.depends;
        for (auto& depend : layer.m_info->annotations.depends)
        {
            if (depend.empty())
            {
                continue;
            }

            std::vector<std::string> splits = KMStringUtils::splitString(depend, "/");
            if (splits.size() > 1)
            {
                continue;
            }

            std::string dependId = splits.at(0);

            if (dependId.empty())
            {
                continue;
            }

            // 判断是否本次拆包的兄弟包
            for (auto& subpackage : d->m_subpackagesJson->subpackages)
            {
                if (subpackage.id == dependId)
                {
                    depend = dependId + "/" + layer.m_info->version;

                    break;
                }
            }
        }
    }
}

void KMOKPackager::amendSubpackage(std::vector<std::string> depends, std::set<std::string>& noRepeat)
{
    for (auto& depend : depends)
    {
        if (depend.empty())
        {
            continue;
        }

        std::vector<std::string> splits = KMStringUtils::splitString(depend, "/");
        std::string dependId = splits.at(0);

        if (dependId.empty())
        {
            continue;
        }

        if (noRepeat.count(dependId))
        {
            continue;
        }

        noRepeat.insert(dependId);

        // 判断是否本次拆包的兄弟包
        for (auto& subpackage : d->m_subpackagesJson->subpackages)
        {
            if (subpackage.id == dependId)
            {
                for (auto &include : subpackage.includes)
                {
                    d->m_subpackage.includes.push_back(include);
                }

                if (!subpackage.depends.empty())
                {
                    amendSubpackage(subpackage.depends, noRepeat);
                }

                break;
            }
        }
    }
}

void KMOKPackager::prepareBundleBySubpackageMode(const std::string &bundleDir)
{
    KMTrace("KMOKPackager::prepareBundleBySubpackageMode invoke begin");

    std::error_code ec;
    std::string layersDir = KMStringUtils::buildFilename(bundleDir, "layers");
    if (!KMFileUtils::mkpath(layersDir, ec))
    {
        throw KMException(ec.message());
    }

    Layer layer = d->m_layers.at(0);
    amendLayer(layer);

    std::string appDir = KMStringUtils::buildFilename(layersDir, layer.m_info->id, layer.m_info->module);
    std::string filesDir = KMStringUtils::buildFilename(layersDir, layer.m_info->id, layer.m_info->module, "files");
    if (!KMFileUtils::mkpath(filesDir, ec))
    {
        throw KMException(ec.message());
    }

    // 拷贝changelog-->KAIMING/changelog
    copyChangelogToBundle(appDir);

    if (layer.m_info->kind == PACKGE_KIND_APP)
    {
        std::vector<std::string> appDepends;
        appDepends.push_back("[base]: " + layer.m_info->base);
        if (!layer.m_info->runtime.empty())
        {
            appDepends.push_back("[runtime]: " + layer.m_info->runtime);
        }
        std::set<std::string> noRepeat;
        copyDependsToBundle(d->m_subpackage.depends, appDir, appDepends, noRepeat);

        // 记录KAIMING/depends
        recordDependsInfo(appDir, appDepends);

        noRepeat.clear();
        amendSubpackage(d->m_subpackage.depends, noRepeat);
    }

    std::string srcDir = KMStringUtils::buildFilename(layer.m_layerDir, "files");
    copyIncludesToBundle(srcDir, filesDir);

    std::string metadataFile = KMStringUtils::buildFilename(appDir, "info.json");
    layer.m_info->saveFile(metadataFile);

    exportEntries(appDir);

    // 求md5校验和
    KMDirMd5sum md5sum(appDir);
    md5sum.md5sum("files");

    layer.m_info->size = KMFileUtils::getDirectorySize(fs::path(appDir));
    layer.m_info->saveFile(metadataFile);
    if (layer.m_info->size > 10*1024*1024)
    {
        d->m_largerThan10M = true;
    }

    d->m_metadata.layers.push_back(*(layer.m_info));

    KMProcessUtils::spawn("sync", {}, true);

    KMTrace("KMOKPackager::prepareBundleBySubpackageMode invoke end");
}

void KMOKPackager::generateErofsBundle(const std::string &bundleFile, const std::string &bundleDir)
{
    KMTrace("KMOKPackager::generateErofsBundle invoke begin");

    uuid_t uuidtime;
    // 生成基于时间的UUID
    uuid_generate_time(uuidtime);
    // 将UUID转换为字符串格式
    std::array<char, 64> uuidarray;
    uuid_unparse(uuidtime, uuidarray.data());
    std::string uuid = std::string(uuidarray.data());

    d->m_metadata.uuid = uuid;

    KMInfo("Doing mkfs.erofs ...");

    std::vector<std::string> args;
    if (d->m_compression.empty())
    {
        args.push_back("-zlz4hc");
    }
    else
    {
        args.push_back("-z" + d->m_compression);
    }
    // args.push_back("-b4096");
    args.push_back("-d3");
    args.push_back(bundleFile);
    args.push_back(bundleDir);

    if (!KMProcessUtils::spawn("mkfs.erofs", args))
    {
        throw KMException("Failed to generate bundle file.");
    }

    KMTrace("KMOKPackager::generateErofsBundle invoke end");
}

void KMOKPackager::generateSquashfsBundle(const std::string &bundleFile, const std::string &bundleDir)
{
    KMTrace("KMOKPackager::generateSquashfsBundle invoke begin");

    KMInfo("Doing mksquashfs ...");

    std::vector<std::string> args;
    args.push_back(bundleDir);
    args.push_back(bundleFile);
    args.push_back("-nopad");
    args.push_back("-noappend");
    // args.push_back("-info");

    if (!d->m_compression.empty())
    {
        std::vector<std::string> splits = KMStringUtils::splitString(d->m_compression, " ");
        std::vector<std::string> comps{ "gzip", "lzo", "lz4", "xz", "zstd", "lzma"};
        if (splits.size() > 0 && KMStringUtils::contains(comps, splits.at(0)))
        {
            args.push_back("-comp");
            for (const std::string& item : splits)
            {
                args.push_back(item);
            }
        }
    }

    if (d->m_largerThan10M)
    {
        args.push_back("-b");
        args.push_back("1M");
    }

    if (!KMProcessUtils::spawn("mksquashfs", args))
    {
        throw KMException("Failed to generate bundle file.");
    }

    uuid_t uuidtime;
    // 生成基于时间的UUID
    uuid_generate_time(uuidtime);
    // 将UUID转换为字符串格式
    std::array<char, 64> uuidarray;
    uuid_unparse(uuidtime, uuidarray.data());
    std::string uuid = std::string(uuidarray.data());

    d->m_metadata.uuid = uuid;

    KMTrace("KMOKPackager::generateSquashfsBundle invoke end");
}

void KMOKPackager::packMetadata()
{
    KMTrace("KMOKPackager::packMetadata invoke begin");

    KMInfo("Packaging meta ...");

    std::string metadataFile = KMStringUtils::buildFilename(d->m_cacheDir, "metadata.json");
    d->m_metadata.bundleFilesystem = d->m_bundleFilesystem;
    d->m_metadata.saveFile(metadataFile);

    std::string metaSection(KMOABMETADATA_PREFIX ".meta");
    addNewSection(metaSection, metadataFile);

    KMTrace("KMOKPackager::packMetadata invoke end");
}
