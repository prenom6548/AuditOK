/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildInit.h"

#include <fstream>
#include <set>
#include <nlohmann/json.hpp>
#include <unordered_set>

#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "common/KMException.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMInfoJson.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMUtil.h"
#include "common/KMVersion.h"
#include "common/KMProcessUtils.h"

static std::vector<std::string> sysDirWithoutUsr = {
    "etc",
    "opt",
    "tmp",
    "var",
    "home",
    "run",
    "dev",
    "data",
    "boot",
    "root",
    "proc",
    "srv",
    "sys",
    "media"
};

class KMBuildInit::Options : public KMOption::Options
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
    std::string m_arch;
    std::string m_runtime;
    std::string m_base;
    std::string m_type;
    std::string m_channel;
    std::string m_version;
    std::vector<std::string> m_depends;
    bool m_help = false;

    std::string m_buildDir;
    std::string m_appid;
    std::string m_cacheDir;

    bool m_kylin_privilege = false;
    bool m_kylin_sysapp = false;

private:
    std::set<std::string> m_setTypes{ "app", "runtime", "extension", "base", "depend" };
};

void KMBuildInit::Options::preParseHook()
{
    addOptions();
}

void KMBuildInit::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    //检查app的version
    if (!m_version.empty())
    {
        KMVersion ver = KMVersion::parse(m_version);
    }
    
    if (m_buildDir.empty() || m_appid.empty())
    {
        KMError(_("DIRECTORY APPID must be specified"));
        showUsage();
        exit(EXIT_FAILURE);
    }

    m_buildDir = fs::absolute(m_buildDir);

    if (m_setTypes.end() == m_setTypes.find(m_type))
    {
        KMError(m_type + _(" is not a valid build type name, use app、depend、runtime"));
        exit(EXIT_FAILURE);
    }

    std::string errMsg;
    if (!KMBuildinsUtils::isValidAppName(m_appid, errMsg))
    {
        KMError(errMsg);
        exit(EXIT_FAILURE);
    }
}

void KMBuildInit::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming build-init [OPTIONS] DIRECTORY APPID"
                     "\nExample:\n \tkaiming build-init --base=stable:top.openkylin.base/1.0.0 --runtime=stable:top.openkylin.ukui/1.0.0 build top.openkylin.demo\n"));

    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_help), _("Show help options"));

    //应用选项
    addOption("arch", "", KMOption::value<std::string>(&m_arch)->defaultValue(KMUtil::kmGetArch()), _("Arch to use"));
    addOption("runtime", "", KMOption::value<std::string>(&m_runtime), _("Runtime info, format : ${runtime-channel}:${runtime-id}/${runtime-version}, such as : stable:top.openkylin.ukui/1.0.0"));
    addOption("base", "", KMOption::value<std::string>(&m_base), _("Base info, format : ${base-channel}:${base-id}/${base-version}, such as : stable:top.openkylin.base/1.0.0"));
    addOption("kind", "", KMOption::value<std::string>(&m_type)->defaultValue("app"), _("Specify the build type (app, runtime, extension, base, depend)"));
    addOption("channel", "", KMOption::value<std::string>(&m_channel)->defaultValue("stable"), _("Release channel, such as (main, stable, etc.)"));
    addOption("depend", "", KMOption::value<std::vector<std::string>>(&m_depends), _("Format as: {id}/{version}. There can be multiple."));
    addOption("version", "", KMOption::value<std::string>(&m_version), _("app version"));
    addOption("kylin-privilege", "", KMOption::value(&m_kylin_privilege), _("annotations->kylin.privilege"), true);
    addOption("kylin-sysapp", "", KMOption::value(&m_kylin_sysapp), _("annotations->kylin.sysapp"), true);
    addOption("cacheDir", "", KMOption::value(&m_cacheDir), _("The directory of cache"));

    //位置选项
    addPositionOption("DIRECTORY", KMOption::value<std::string>(&m_buildDir), 1, _("The directory for building"));
    addPositionOption("APPID", KMOption::value<std::string>(&m_appid), 1, _("Appid"));
}

class KMBuildInit::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    KMStorageDir m_baseDir;
    KMStorageDir m_runtimeDir;
    std::optional<KMRef> m_originalBaseRef;
    std::optional<KMRef> m_originalRuntimeRef;
    std::string m_baseFiles;
    std::string m_runtimeFiles;
    KMRef m_baseRef;
    KMRef m_runtimeRef;

    std::string m_filesDir;
    std::string m_metadataFile;

    // 依赖包的环境变量
    std::map<std::string, std::string> m_envsOfDepends;
    // 用于记录KAIMING/depends文件，每行格式：${id}/${version}
    std::vector<std::string> m_appDepends;
    nlohmann::json m_rootCache;
    std::string m_fileCache;
};

REGISTER_SUBCOMMAND_DYNCREATE(build-init, KMBuildInit)

bool KMBuildInit::checkBaseOrRuntime(const fs::path &infoPath , const bool runtimeFlag) const
{
    KMInfoJson::InfoBase app, runOrDep;
    
    KMInfoJson infoJson;
    infoJson.loadFile(infoPath);

    //校验base
    if (!runtimeFlag)
    {
        app.m_baseId = d->m_baseRef.id;
        app.m_baseVersion= d->m_baseRef.version;

        runOrDep.m_baseString = infoJson.base;
    }
    else if (infoJson.runtime.empty())
    {
        return false;
    }
    //校验runtime
    else
    {
        app.m_baseId = d->m_runtimeRef.id;
        app.m_baseVersion= d->m_runtimeRef.version;

        runOrDep.m_baseString = infoJson.runtime;
    }

    runOrDep.baseParse();
    app.versionParse();
    runOrDep.versionParse();
    
    if (app.m_baseId != runOrDep.m_baseId || ! app.checkVersion(runOrDep))
    {
        return false;
    }

    return true;

}

KMBuildInit::KMBuildInit()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuildInit::~KMBuildInit()
{
}

int KMBuildInit::dispose(int argc, char **argv)
{
    KMTrace("KMBuildInit::dispose invoke begin");

    init(argc, argv);

    int ret = run();

    KMTrace("KMBuildInit::dispose invoke end");
    return ret;
}

void KMBuildInit::init(int argc, char **argv)
{
    KMTrace("KMBuildInit::init invoke begin");

    d->m_kmOptions->parseCommandLine(argc, argv);

    // 解析base、runtime，并校验是否已经安装
    d->m_originalBaseRef = KMBuildInit::parsePref(d->m_kmOptions->m_base);
    if (!d->m_originalBaseRef)
    {
        throw KMException("The config of base '" + d->m_kmOptions->m_base + "' is wrong");
    }
    
    d->m_baseRef = ensureDevModuleInstalled(*d->m_originalBaseRef, MODULE_NAME_DEV, BASE_TYPE_BASE);
    std::string baseDeployPath = KMStorageDir::getDeployedDir(d->m_baseRef);
    d->m_baseFiles = KMStringUtils::buildFilename(baseDeployPath, "files");
    d->m_appDepends.push_back("[base]: " + d->m_baseRef.id + "/" + d->m_baseRef.version);

    // 解析runtime，并校验是否已经安装
    if (!d->m_kmOptions->m_runtime.empty())
    {
        d->m_originalRuntimeRef = KMBuildInit::parsePref(d->m_kmOptions->m_runtime);
        if (!d->m_originalRuntimeRef)
        {
            throw KMException("The config of runtime '" + d->m_kmOptions->m_runtime + "' is wrong");
        }

        d->m_runtimeRef = ensureDevModuleInstalled(*d->m_originalRuntimeRef, MODULE_NAME_DEV, BASE_TYPE_RUNTIME);
        //  app的base和runtime的base要保持一致
        std::string runtimeDeployPath = KMStorageDir::getDeployedDir(d->m_runtimeRef);
        std::string infoPath = KMStringUtils::buildFilename(runtimeDeployPath, "info.json");

        if (!checkBaseOrRuntime(infoPath))
        {
            throw KMException("The appBase and the runtimeBase is not matched");
        }
        d->m_runtimeFiles = KMStringUtils::buildFilename(runtimeDeployPath, "files");
        d->m_appDepends.push_back("[runtime]: " + d->m_runtimeRef.id + "/" + d->m_runtimeRef.version);
    }

    d->m_filesDir = d->m_kmOptions->m_buildDir + "/files";
    d->m_metadataFile = d->m_kmOptions->m_buildDir + "/info.json";

    if (KMFileUtils::pathExists(d->m_filesDir))
    {
        throw KMException(std::string(_("Build directory already initialized : ")) + d->m_kmOptions->m_buildDir);
    }

    std::string cacheFile = KMStringUtils::buildFilename(d->m_kmOptions->m_cacheDir, "stage-cache.json");
    if (fs::exists(cacheFile))
    {
        d->m_fileCache = cacheFile;
        d->m_rootCache = KMJsonHelper::loadFile(cacheFile);
    }

    KMTrace("KMBuildInit::init invoke end");
}

int KMBuildInit::run()
{
    KMTrace("KMBuildInit::run invoke begin");

    // 创建构建目录
    std::error_code ec;
    if (!KMFileUtils::mkpath(d->m_kmOptions->m_buildDir, ec))
    {
        throw KMException(ec.message());
    }

    // 创建files目录
    if (!KMFileUtils::mkpath(d->m_filesDir, ec))
    {
        throw KMException(ec.message());
    }

    //depends是否为空
    if (!d->m_kmOptions->m_depends.empty())
    {
        if (d->m_kmOptions->m_type == BASE_TYPE_APP)
        {
            std::set<std::string> noRepeat;
            addDependsForApp(d->m_kmOptions->m_depends, noRepeat);

            generateDepends();

            KMJsonHelper helper(d->m_rootCache);
            helper.toJson(d->m_fileCache);
        }
        else if (d->m_kmOptions->m_type == BASE_TYPE_DEPEND)
        {
            std::set<std::string> noRepeat;
            addDependsForDepend(d->m_kmOptions->m_depends, noRepeat);

            generateDepends();
        }
    }

    // 生成info.json文件
    generateMetadata();

    KMProcessUtils::spawn("sync", {}, true);

    KMTrace("KMBuildInit::run invoke end");
    return EXIT_SUCCESS;
}

/**
 * @brief : 生成KAIMING/depends文件，并填充内容
 */
void KMBuildInit::generateDepends()
{
    std::string kaimingDir = d->m_kmOptions->m_buildDir + "/KAIMING";
    std::error_code ec;
    if (!KMFileUtils::mkpath(kaimingDir, ec))
    {
        throw KMException(ec.message());
    }

    std::string dependsFile = kaimingDir + "/depends";

    std::ofstream o(dependsFile);
    for (auto& depend : d->m_appDepends)
    {
        o << depend << std::endl;
    }
}

/**
 * @brief : 校验depend并获取其所在路径
 * @param : depend.id
 * @return : depend所在路径
 */
std::string KMBuildInit::getDependPath(const std::string &depend)
{
    std::vector<std::string> splits = KMStringUtils::splitString(depend, "/");
    if (splits.size() == 1)
    {
        splits.emplace_back("");
    }
    KMInstalledAppQuery query;
    std::vector<KMRef> depends = query.arch(d->m_kmOptions->m_arch)
                                    .kind(BASE_TYPE_DEPEND)
                                    .id(splits.at(0))
                                    .module(MODULE_NAME_DEV)
                                    .version(splits.at(1))
                                    .query(KMInstalledAppQuery::User);
    if (depends.empty())
    {
        std::string name = splits.at(0) + "/" + splits.at(1) + "/" + MODULE_NAME_DEV; // KMStringUtils::buildFilename(splits.at(0), splits.at(1), MODULE_NAME_DEV);

        KMInfo("Preparing to install development depend:  " + name + "...");
        uid_t uid = ::getuid();
        if (uid == 0)
        {
            if (!KMProcessUtils::spawn("kaiming", { "install", "-y", "--compatibility", "--skip-signature-verification", name }))
            {
                throw KMException("Please install it manually: sudo kaiming install " 
                    + splits.at(0) + (splits.at(1).empty() ? "" : ("/" + splits.at(1))));
            }
        }
        else
        {
            if (!KMProcessUtils::spawn("pkexec", { KAIMING_PROGRESS_FULL_PATH, "install", "-y", "--compatibility", "--skip-signature-verification", name }))
            {
                throw KMException("Please install it manually: sudo kaiming install " 
                    + splits.at(0) + (splits.at(1).empty() ? "" : ("/" + splits.at(1))));
            }
        }

        KMProcessUtils::spawn("sync", {}, true);

        depends = query.arch(d->m_kmOptions->m_arch)
                       .kind(BASE_TYPE_DEPEND)
                       .id(splits.at(0))
                       .module(MODULE_NAME_DEV)
                       .version(splits.at(1))
                       .query(KMInstalledAppQuery::User);
    }

    KMRef dependRef = depends.at(0);
    std::string dependDeployPath = KMStorageDir::getDeployedDir(dependRef);
    std::string dependInfo = KMStringUtils::buildFilename(dependDeployPath, "info.json");

    //先校验runtime，runtime匹配可不校验base
    if (d->m_runtimeRef.id.empty() || !checkBaseOrRuntime(dependInfo,true))
    {
        if (!checkBaseOrRuntime(dependInfo))
        {
            throw KMException("The appBase and the dependBase is not matched");
        }
    }
    return dependDeployPath;
}

/**
 * @brief: 处理depend依赖
 */
void KMBuildInit::addDependsForApp(const std::vector<std::string> &depends, std::set<std::string>& noRepeat)
{
    // 用于记录递归深度
    static int s_depth = 0;
    int depth = ++s_depth;

    std::string depend_file_target = "files";
    for (auto const &depend : depends)
    {
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

        std::string dependDeployPath = getDependPath(depend);
        std::string dependInfo = KMStringUtils::buildFilename(dependDeployPath, "info.json");
        //决定拷贝到哪个文件中
        KMInfoJson dependJson;
        dependJson.loadFile(dependInfo);

        //递归
        if (!dependJson.annotations.depends.empty())
        {
            s_depth = depth; // 重置递归深度
            addDependsForApp(dependJson.annotations.depends, noRepeat);
        }

        if (d->m_kmOptions->m_kylin_sysapp)
        {
            depend_file_target = "files";
        }
        else if (dependJson.annotations.sysapp)
        {
            depend_file_target = "depends";
        }
        else
        {
            depend_file_target = "files";
        }

        std::string dependFiles = KMStringUtils::buildFilename(dependDeployPath, "files");
        std::string parent_dir = fs::path(d->m_filesDir).parent_path().string();
        std::string dest_file = KMStringUtils::buildFilename(parent_dir,depend_file_target);

        //创建depends文件夹
        if (!fs::exists(dest_file))
        {
            fs::create_directories(dest_file);
        }

        std::vector<std::string> excludes;
        excludes.push_back("/share/applications");
        excludes.push_back("/local/share/applications");
        excludes.push_back("/usr/share/applications");
        excludes.push_back("/usr/local/share/applications");
        KMFileUtils::rsyncDir(dependFiles, dest_file);

        // dependJson.envs可能已经包含了depends依赖引入的环境变量，故此处追加
        for (auto& env : dependJson.envs)
        {
            KMBuildInit::addEnv(d->m_envsOfDepends, env.first, env.second);
        }

        if (depth == 1)
        {
            d->m_appDepends.push_back("[direct depend]: " + dependJson.id + "/" + dependJson.version);
        }
        else
        {
            d->m_appDepends.push_back("[nested depend]: " + dependJson.id + "/" + dependJson.version);
        }

        if (!d->m_fileCache.empty())
        {
            std::vector<std::string> files = KMFileUtils::getDirFiles(dependFiles);
            for (std::string& file : files)
            {
                file = file.substr(dependFiles.length() + 1);
            }
            KMJsonHelper helper(d->m_rootCache);
            helper.setStringListValue(dependId, files);
        }
    }
}

void KMBuildInit::addEnv(std::map<std::string, std::string>& envs, const std::string &key, const std::string &value)
{
    std::string newValue;
    std::string left = value;
    while (KMStringUtils::contains(left, "$"))
    {
        // value值如：$PATH:/app/bin 或 /app/bin:$PATH 或 $HOME/.config
        size_t start = left.find_first_of('$');
        size_t end_colon = left.find_first_of(':', start);
        size_t end_slash = left.find_first_of('/', start);
        size_t end = end_colon < end_slash ? end_colon : end_slash;

        std::string keyOfEnv;
        std::string replaceKey;
        if (std::string::npos == end)
        {
            keyOfEnv = left.substr(start);
            replaceKey = keyOfEnv;
            keyOfEnv = keyOfEnv.substr(1);
        }
        else
        {
            keyOfEnv = left.substr(start, end - start);
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

        std::string old;
        if (envs.find(keyOfEnv) != envs.end())
        {
            old = envs[keyOfEnv];
        }
        if (!old.empty() && KMStringUtils::endsWith(replaceKey, ":"))
        {
            old += ":";
        }
        else if (!old.empty() && KMStringUtils::endsWith(replaceKey, "/"))
        {
            old += "/";
        }
        if (old.empty())
        {
            newValue += left.substr(0, start + replaceKey.length());
            left = left.substr(start + replaceKey.length());
        }
        else
        {
            KMStringUtils::replace(left, replaceKey, old);
        }
    }

    newValue += left;

    if (KMStringUtils::startsWith(newValue, ":"))
    {
        // value值如：:/app/bin，则认为是在原基础上追加；
        std::string old = envs[key];
        newValue = old + newValue;
    }
    else if (KMStringUtils::endsWith(newValue, ":"))
    {
        // value值如：/app/bin:，则认为是在原基础上增加前缀；
        std::string old = envs[key];
        newValue = newValue + old;
    }

    envs[key] = KMStringUtils::replace(newValue, "::", ":");
}

void KMBuildInit::addDependsForDepend(const std::vector<std::string>& depends, std::set<std::string>& noRepeat)
{
    // 用于记录递归深度
    static int s_depth = 0;
    int depth = ++s_depth;

    // kind=depend的开明包是公共库，需要和base\runtime的目录结构一致，此处默认kylin.sysapp=true
    std::string destDir = KMStringUtils::buildFilename(d->m_kmOptions->m_buildDir, "depends");
    KMFileUtils::mkpath(destDir);

    for (const std::string& depend : depends)
    {
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

        KMInstalledAppQuery query;
        std::vector<KMRef> refs = query.arch(d->m_kmOptions->m_arch)
                                        .kind(BASE_TYPE_DEPEND)
                                        .id(dependId)
                                        .module(MODULE_NAME_DEV)
                                        .version(dependVersion)
                                        .query(KMInstalledAppQuery::User);
        if (refs.empty())
        {
            std::string name = dependId + "/" + dependVersion + "/" + MODULE_NAME_DEV; 

            KMInfo("Preparing to install development depend:  " + name + "...");

            uid_t uid = ::getuid();
            if (uid == 0)
            {
                if (!KMProcessUtils::spawn("kaiming", { "install", "-y", "--compatibility", "--skip-signature-verification", name }))
                {
                    throw KMException("Please install it manually: sudo kaiming install " 
                        + dependId + (dependVersion.empty() ? "" : ("/" + dependVersion)));
                }
            }
            else
            {
                if (!KMProcessUtils::spawn("pkexec", { KAIMING_PROGRESS_FULL_PATH, "install", "-y", "--compatibility", "--skip-signature-verification", name }))
                {
                    throw KMException("Please install it manually: sudo kaiming install " 
                        + dependId + (dependVersion.empty() ? "" : ("/" + dependVersion)));
                }
            }

            KMProcessUtils::spawn("sync", {}, true);

            refs = query.arch(d->m_kmOptions->m_arch)
                        .kind(BASE_TYPE_DEPEND)
                        .id(dependId)
                        .module(MODULE_NAME_DEV)
                        .version(dependVersion)
                        .query(KMInstalledAppQuery::User);
        }

        KMRef& ref = refs.front();
        std::shared_ptr<KMDeploy> deploy = KMStorageDir::getSystemDefaultDir().loadDeployed(ref);
        if (!deploy)
        {
            throw KMException("Failed to load info.json of " + depend);
        }

        if (!deploy->m_infoJson->annotations.depends.empty())
        {
            s_depth = depth; // 重置递归深度
            addDependsForDepend(deploy->m_infoJson->annotations.depends, noRepeat);
        }

        if (depth == 1)
        {
            d->m_appDepends.push_back("[direct depend]: " + ref.id + "/" + ref.version);
        }
        else
        {
            d->m_appDepends.push_back("[nested depend]: " + ref.id + "/" + ref.version);
        }

        std::string srcDir = KMStringUtils::buildFilename(deploy->m_dir, "files");
        if (deploy->m_infoJson->annotations.sysapp)
        {
            KMFileUtils::rsyncDir(srcDir, destDir);
            continue;
        }

        // kind=depend时，当前默认都是sysapp
        // fs::path dependPath(srcDir);
        // for (auto& dirEntry : fs::directory_iterator{ dependPath })
        // {
        //     fs::path p = dirEntry.path();
        //     std::string filename = p.filename();
        //     if (filename == "." || filename == "..")
        //     {
        //         continue ;
        //     }

        //     if (fs::is_regular_file(p))
        //     {
        //         KMFileUtils::cpRfAll(p.string(), destDir);
        //     }
        //     else if (KMStringUtils::contains(sysDirWithoutUsr, filename))
        //     {
        //         std::string dest = KMStringUtils::buildFilename(destDir, filename);
        //         KMFileUtils::mkpath(dest);
        //         KMFileUtils::rsyncDir(p.string(), dest);
        //     }
        //     else
        //     {
        //         // 其他数据默认为放到usr下，如：bin\sbin\lib\lib64\local\share等
        //         std::string dest = KMStringUtils::buildFilename(destDir, "usr", filename);
        //         KMFileUtils::mkpath(dest);
        //         KMFileUtils::rsyncDir(p.string(), dest);
        //     }
        // }
    }
}

/**
 * @brief : 解析base/runtime等的pref
 * @param : [in] pref, 格式如：top.openkylin.base/stable/2.0.0
 * @return: std::optional<KMRef>
 */ 
std::optional<KMRef> KMBuildInit::parsePref(const std::string& pref)
{
    std::optional<KMRef> ref;

    std::regex reg{ "[:/]" };
    std::vector<std::string> splits{ std::sregex_token_iterator(pref.begin(), pref.end(), reg, -1), std::sregex_token_iterator() };
    if (splits.size() != 3)
    {
        KMError("The format of base and runtime must be : ${channel}:${id}/${version}");
        return ref;
    }

    
    if (std::string errMsg; !KMBuildinsUtils::isValidAppName(splits.at(1), errMsg))
    {
        KMError(errMsg);
        return ref;
    }

    ref = {
        .id = splits.at(1),
        .channel = splits.at(0),
        .version = splits.at(2),
        .arch = "",
        .module = "",
        .branch = "",
        .kind = "",
        .baseInstallDir = ""
    };

    return ref;
}

void KMBuildInit::generateMetadata()
{
    KMTrace("KMBuildInit::generateMetadata invoke begin");

    KMInfoJson doc;
    
    doc.id = d->m_kmOptions->m_appid;
    doc.archs.push_back(d->m_kmOptions->m_arch);
    doc.kind = d->m_kmOptions->m_type;
    doc.channel = d->m_kmOptions->m_channel;
    doc.base = d->m_baseRef.channel + ":" + d->m_baseRef.id + "/" + d->m_baseRef.version + "/" + d->m_kmOptions->m_arch;
    if (d->m_originalRuntimeRef)
    {
        doc.runtime = d->m_runtimeRef.channel + ":" + d->m_runtimeRef.id + "/" + d->m_runtimeRef.version + "/" + d->m_kmOptions->m_arch;
    }
    else
    {
        doc.runtime = "";
    }
    doc.version = d->m_kmOptions->m_version;
    doc.annotations.privilege = d->m_kmOptions->m_kylin_privilege;
    doc.annotations.sysapp = d->m_kmOptions->m_kylin_sysapp;
    if (d->m_kmOptions->m_type == BASE_TYPE_DEPEND)
    {
        doc.annotations.depends = d->m_kmOptions->m_depends;
    }
    doc.envs = d->m_envsOfDepends;

    doc.saveFile(KMStringUtils::buildFilename(d->m_kmOptions->m_buildDir, "info.json"));

    KMTrace("KMBuildInit::generateMetadata invoke end");
}

KMRef KMBuildInit::ensureDevModuleInstalled(const KMRef& ref, const std::string& module,const std::string& kind)
{
    
    bool isExactMatchVersion = false; /** 是否为精准匹配版本 */
    bool isInstalled = false;  /** 是否需要下载指定版本 */
    KMVersion version;
    KMInstalledAppQuery query;
    
    auto items = query.channel(ref.channel)
                     .arch(d->m_kmOptions->m_arch)
                     .kind(kind)
                     .id(ref.id)
                     .module(module)
                     .version(ref.version)
                     .query(KMInstalledAppQuery::User);
    try{
        version = KMVersion::parse(ref.version, true);
        isExactMatchVersion = true;
    } catch (const std::exception& e)
    {
        KMInfo("The version '" + ref.version + "' is not an exact version, will try to find the latest compatible version");
    }

    if (isExactMatchVersion && !items.empty())
    {
        KMVersion installedVersion;
        // 指定版本，则检查是否已安装
        for (const auto &ref : items)
        {
            installedVersion = KMVersion::parse(ref.version, false);
            if (installedVersion == version)
            {
                isInstalled = true;
                break;
            }
        }
    }
    
    if (items.empty() || (isExactMatchVersion && !isInstalled)) // 没有安装或者虽然有版本但不是精准匹配版本
    {
        KMInfo("Preparing to install development " + kind + ":  "
               + KMStringUtils::buildFilename(ref.id, ref.version, module, ref.channel));

        std::string name = KMStringUtils::buildFilename(ref.id, ref.version, module);
        uid_t uid = ::getuid();
        bool success = false;
        std::vector<std::string> args;
        args.push_back("install");
        args.push_back("-y");
        args.push_back("--skip-signature-verification");
        if (!isExactMatchVersion) 
        {
            args.push_back("--compatibility");
        }
        args.push_back(name);
        if (uid == 0)
        {
            success = KMProcessUtils::spawn("kaiming", args);
        }
        else
        {
            args.insert(args.begin(),KAIMING_PROGRESS_FULL_PATH);
            success = KMProcessUtils::spawn("pkexec", args);
        }

        if (!success)
        {
            throw KMException(
                "Please install it manually: sudo kaiming install "
                + KMStringUtils::buildFilename(ref.id, ref.version, module, ref.channel));
        }

        // 安装完毕后重新查询
        items = query.channel(ref.channel)
                    .arch(d->m_kmOptions->m_arch)
                    .kind(kind)
                    .id(ref.id)
                    .module(module)
                    .version(ref.version)
                    .query(KMInstalledAppQuery::User);
    }

    return items.at(0);
}
