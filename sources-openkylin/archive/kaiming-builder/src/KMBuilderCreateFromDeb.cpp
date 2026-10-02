/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include <cstdlib>
#include <filesystem>
#include <vector>

#include "KMBuilderCreateFromDeb.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "common/KMRef.h"
#include "common/KMException.h"

#include "KMBuilderUtils.h"
#include "KMBuilderManifest.h"
#include "KMBuilderDebian.h"
#include "KMBuilderQuery.h"
#include "KMBuilderSource.h"
#include "common/KMProcessUtils.h"
#include "common/KMLogger.h"
#include "common/KMInstalledAppQuery.h"

namespace fs = std::filesystem;

#define MODULE_DEVEL "devel"

class KMBuilderCreateFromDeb::Options: public KMOption::Options
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
    std::string m_base { "stable:top.openkylin.base/1.0.0" };
    std::string m_runtime { "stable:top.openkylin.ukui/1.0.0" };
    std::string m_appid;

    std::string m_source_path;

    bool m_gui;
    bool m_isHelp;
};

void KMBuilderCreateFromDeb::Options::preParseHook()
{
    addOptions();
}

void KMBuilderCreateFromDeb::Options::postParseHook()
{
    if (m_isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
    if (m_appid.empty() || m_source_path.empty())
    {
        KMError(_("APPID SOURCE-PATH must be specified"));
        showUsage();
        exit(EXIT_FAILURE);
    }
    if (KMBuilderUtils::isEmptyDirectory(m_source_path))
    {
        KMError(_("APPID SOURCE-PATH is empty."));
        exit(EXIT_FAILURE);
    }
}

void KMBuilderCreateFromDeb::Options::addOptions()
{
    setDescription(_("\nUsage:\n\tkaiming-builder create-from-debsource [OPTION…] APPID SOURCE-PATH\n"));

    addOption("help", "h", KMOption::value<bool>(&m_isHelp), _("Show help options."));

    addOption("base","", KMOption::value<std::string>(&m_base), _("The basic environmental information used by the application. Format: {channel}:{id}/{version}, example: stable:top.openkylin.base/1.0.0"));
    addOption("runtime", "", KMOption::value<std::string>(&m_runtime), _("The runtime environmental information used by the application. Format: {channel}:{id}/{version}, example: stable:top.openkylin.ukui/1.0.0"));
    addOption("gui", "",KMOption::value<bool>(&m_gui), _("Check if it is called by gui."), true);

    addPositionOption("APPID", KMOption::value<std::string>(&m_appid), 1, _("Project id"));
    addPositionOption("SOURCE-PATH", KMOption::value<std::string>(&m_source_path), 1, _("The path of source."));
}

class KMBuilderCreateFromDeb::Private
{
public:
    int argc;
    char **argv;

    //base或runtime中含有的软件包
    std::map<std::string ,std::string> m_dpkgPackages;
    //本地或远程存在的依赖
    std::map<std::string, std::string> m_depends;
    //需要在components中列出的依赖<id, version>
    std::map<std::string, std::string> m_components;

    //解析得到的debian文件信息
    std::unique_ptr<KMBuilderDebian> m_debian;
    //存储清单文件信息
    std::unique_ptr<KMBuilderManifest> m_manifest;

    std::unique_ptr<Options> m_kmOptions;

public:
    std::string m_projectDir;

    std::string m_proFile;
    bool m_configure = false;
};

REGISTER_SUBCOMMAND_DYNCREATE(create-from-debsource, KMBuilderCreateFromDeb)

KMBuilderCreateFromDeb::KMBuilderCreateFromDeb()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_manifest = std::make_unique<KMBuilderManifest>();
    d->m_debian = std::make_unique<KMBuilderDebian>();
}

KMBuilderCreateFromDeb::~KMBuilderCreateFromDeb()
{

}

int KMBuilderCreateFromDeb::dispose(int argc, char **argv) 
{
    init(argc, argv);
    return run();
}

std::string KMBuilderCreateFromDeb::matchDependByDeb(const std::vector<std::string> &depends, const std::string debName)
{
    for (auto &depend: depends)
    {
        std::string dependName = KMBuilderUtils::splitString(depend, ".")[2];
        if (dependName == debName)
        {
            return depend;
        }
    }
    return "";
}

bool KMBuilderCreateFromDeb::isExists(std::pair<const std::string, std::string> &depend)
{
    std::vector<std::string> dpkgs = KMBuilderUtils::keysFromMap(d->m_dpkgPackages);
    std::vector<std::string> depends = KMBuilderUtils::keysFromMap(d->m_depends);

    std::string key = depend.first;
    if (KMBuilderUtils::contains(depend.second, "Version"))
    {
        size_t pos = depend.second.find_first_of('$');
        if (pos == std::string::npos)
        {
            depend.second = "";
        }
        else
        {
            depend.second = depend.second.substr(0,pos) + d->m_manifest->m_package.m_version;
        }
    }
    //过滤掉deb所需软件包
    if (KMBuilderUtils::startsWith(key, "debhelper") || KMBuilderUtils::startsWith(key, "dh-"))
    {
        return true;
    }
    if (KMBuilderUtils::startsWith(key, "$"))
    {
        return true;
    }

    //base 或 runtime中是否包含
    if (KMBuilderUtils::contains(dpkgs, key))
    {
        std::string version = depend.second;
        std::string relation = "ge";
        if (version.empty() || KMBuilderUtils::contains(version, "nocheck"))
        {
            return true;
        }
        else if (KMBuilderUtils::startsWith(version, ">="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">=")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, ">>"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">>")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, ">"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, "="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "=")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, "<<"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<<")[1]);
            relation = "le";
        }
        else if (KMBuilderUtils::startsWith(version, "<="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<=")[1]);
            relation = "le";
        }
        else if (KMBuilderUtils::startsWith(version, "<"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<")[1]);
            relation = "le";
        }

        if (KMBuilderUtils::endsWith(version, "~"))
        {
            version.pop_back();
            if (relation == "ge")
            {
                relation = "gt";
            }
            else
            {
                relation = "lt";
            }
        }
        if (compareVersion(d->m_dpkgPackages[key], relation, version))
        {
            std::cout << "Found depend " + key + " (" + d->m_dpkgPackages[key] + ") in base or runtime" << std::endl;
            return true;
        }
    }

    //depends
    std::string match_depend = matchDependByDeb(depends, key);
    if (!match_depend.empty())
    {
        std::string version = depend.second; 
        std::string relation = "ge";
        if (version.empty() || KMBuilderUtils::contains(version, "nocheck"))
        {
            d->m_manifest->m_depends.push_back(match_depend);
             std::cout << "Found depend " + key +" in depends" << std::endl;
            return true;
        }
        else if (KMBuilderUtils::startsWith(version, ">="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">=")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, ">>"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">>")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, ">"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, ">")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, "="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "=")[1]);
        }
        else if (KMBuilderUtils::startsWith(version, "<<"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<<")[1]);
            relation = "le";
        }
        else if (KMBuilderUtils::startsWith(version, "<="))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<=")[1]);
            relation = "le";
        }
        else if (KMBuilderUtils::startsWith(version, "<"))
        {
            version = KMBuilderUtils::trimStr(KMBuilderUtils::splitString(version, "<")[1]);
            relation = "le";
        }

        if (KMBuilderUtils::endsWith(version, "~"))
        {
            version.pop_back();
            if (relation == "ge")
            {
                relation = "gt";
            }
            else
            {
                relation = "lt";
            }
        }
        if (compareVersion(d->m_depends[match_depend], relation, version))
        {
            std::cout << "Found depend " + key + "(" + d->m_depends[match_depend] + ") in depends" << std::endl;
            d->m_manifest->m_depends.push_back(match_depend);
            return true;
        }
    }
    return false;
}

void KMBuilderCreateFromDeb::matchDepends(std::map<std::string, std::string> &debianDepends)
{
    for (auto &depend: debianDepends)
    {
        if (isExists(depend))
        {
            continue;
        }
        if (!depend.second.empty())
        {
            std::cout << "Not found depend " + depend.first + "(" + depend.second + ")"<< std::endl;
        }
        else
        {
            std::cout << "Not found depend " + depend.first << std::endl;
        }
        d->m_components[depend.first] = depend.second;
    }
}

void KMBuilderCreateFromDeb::matchDepends(std::vector<std::map<std::string, std::string>> &debianDependsOr)
{
    for (auto &depends: debianDependsOr)
    {
        bool isIn = false;
        std::string key ;
        for (auto &depend: depends)  
        {
            if (isExists(depend))
            {
                isIn = true;
                break;
            }
            if (key.empty())
            {
                if (depend.second.empty())
                {
                    key = depend.first;
                }
                else
                {
                    key = depend.first + "(" + depend.second + ")";
                }
            }
            else
            {
                if (depend.second.empty())
                {
                    key = key + "|" + depend.first;
                }
                else
                {
                    key = key + "|" + depend.first + "(" + depend.second + ")";
                }
            }
        }
        //在base/runtime/depend中都未找到depend包，将包括|在内的信息全都放到depends中。
        if (!isIn)
        {
            d->m_components[key] = "";
        }
    }
}

void KMBuilderCreateFromDeb::loadDpkgList(const std::string &path)
{
    std::vector<std::string> lines = KMBuilderUtils::parseTxt(path);
    for (auto &line:lines)
    {
        std::vector<std::string> tokens = KMBuilderUtils::splitStrBlank(line);
        if (tokens.size() >= 3)
        {
            d->m_dpkgPackages[KMBuilderUtils::splitString(tokens[1], ":")[0]] = tokens[2];  //id,version
        }
    }
}

void KMBuilderCreateFromDeb::loadDepends()
{
    KMBuilderQuery query;
    std::string arch = KMBuilderUtils::getArch();
    std::cout << "Searching dependencies..." << std::endl; 
    std::vector<KMRef> refs = query.getRefsFromLocal(arch, "depend" ,"", "", "", "", "", false);
    std::vector<KMRef> summary_ref = query.getRefsFromSummary(arch, "depend");
    KMBuilderUtils::concatVectors(refs, summary_ref);
    std::sort(refs.begin(), refs.end());
    for (auto &ref: refs)
    {
        // std::string debName = KMBuilderUtils::splitString(ref.m_id, ".")[2];
        d->m_depends[ref.m_id] = ref.m_version;
    }
}

bool KMBuilderCreateFromDeb::compareVersion(const std::string &ver1, const std::string &relation, const std::string &ver2)
{
    std::string cmd = "dpkg --compare-versions " + ver1 + " " + relation + " " + ver2;
    int result = std::system(cmd.c_str());
    return result == 0;
}

std::string KMBuilderCreateFromDeb::matchBuildSystem(const std::string &sourcePath)
{
    bool meson = false, autotools = false, cmake = false, qmake = false;
    for (auto &file: fs::directory_iterator(sourcePath))
    {
        if (file.is_regular_file())
        {
            std::string filename = file.path().filename();
            if (filename == "meson.build")
            {
                meson = true;
            }
            // "autogen", "autogen.sh", "bootstrap", "bootstrap.sh"
            if (filename == "autogen" || filename == "autogen.sh" || filename == "bootstrap" || filename == "bootstrap.sh" || filename == "configure")
            {
                autotools = true;
            }
            if (filename == "CMakeLists.txt")
            {
                cmake = true;
            }
            if (KMBuilderUtils::endsWith(filename, ".pro"))
            {
                qmake = true;
                d->m_proFile = filename;
            }
        }
    }
    
    //优先级：meson > cmake > qmake > autotools
    if (meson)
    {
        return "meson";
    }
    else if (cmake)
    {
        return "cmake";
    }
    else if (qmake)
    {
        return "qmake";
    }
    else if (autotools)
    {
        return "autotools";
    }
    return "simple";
}

void KMBuilderCreateFromDeb::debianToManifest()
{
    d->m_manifest->m_package.m_name = d->m_debian->m_source;
    //version
    try
    {
        std::string command = "dpkg-parsechangelog -SVersion -l " + KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "debian", "changelog");
        std::string result = KMBuilderUtils::executeCommand(command);
        if (KMBuilderUtils::endsWith(result, "\n"))
        {
            result.pop_back();
        }
        d->m_manifest->m_package.m_version = KMBuilderUtils::versionDebToKaiming(result);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n' << "No package version";
    }
    
    d->m_manifest->m_homepage = d->m_debian->m_homepage;
    d->m_manifest->maintainers.push_back(d->m_debian->m_maintainer);
    d->m_manifest->m_license = d->m_debian->m_license;
    matchDepends(d->m_debian->m_depends);
    // 解决含有|的depend
    matchDepends(d->m_debian->m_or_depends);
    //剩余未在depend/base/runtime中找到的depend填充在depends字段的末尾
    fillDepends();


}

void KMBuilderCreateFromDeb::setBuildScript(std::shared_ptr<KMBuilderModule> &component)
{
    std::string prefix = "/opt/apps/" + d->m_kmOptions->m_appid + "/files";

    std::string meson_command = "mkdir _kaiming_build && cd _kaiming_build && meson --prefix=" + prefix + " ../"+ " && ninja -j1 && ninja install";
    std::string cmake_command = "cmake -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX:PATH=" + prefix + " -G \"Ninja\" ./" + " && ninja -j1 && ninja install";
    std::string qmake_command = "qmake PREFIX=" + prefix + " " + d->m_proFile + " && make -j$(nproc) && make install";
    std::string auto_command = "./configure --prefix=" + prefix +  " && make -j$(nproc) && make install";
    std::string simple_command = "${Please set build-script manually}";
    if (!fs::exists(KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "configure")))
    {
        auto_command = "./autogen.sh\n" + auto_command;
    }
    switch (component->buildSystem())
    {
    case KMBuilderModule::BS_Meson:
        component->setProperty(KMBuilderModule::Module_BuildCommands, meson_command);
        break;
    case KMBuilderModule::BS_Cmake :
        component->setProperty(KMBuilderModule::Module_BuildCommands, cmake_command);
        break;
    case KMBuilderModule::BS_Qmake:
        component->setProperty(KMBuilderModule::Module_BuildCommands, qmake_command);
        break;
    case KMBuilderModule::BS_Autotools:
        component->setProperty(KMBuilderModule::Module_BuildCommands, auto_command);
        break;
    default:
        component->setProperty(KMBuilderModule::Module_BuildCommands, simple_command);
        break;
    }

}

std::shared_ptr<KMBuilderModule> KMBuilderCreateFromDeb::setComponent()
{
    std::shared_ptr<KMBuilderModule> component;
    component = std::make_shared<KMBuilderModule>();

    std::string buildsystem = matchBuildSystem(d->m_kmOptions->m_source_path);
    component->setBuildSystem(buildsystem);

    component->setProperty(KMBuilderModule::Module_Name, d->m_manifest->m_package.m_name);
    std::string simple_name = "simple";
    component->setProperty(KMBuilderModule::Module_BuildSystem, simple_name);

    setBuildScript(component);

    std::string sourceType = "dir";
    std::shared_ptr<KMBuilderSource> source = KMBuilderSource::createSource(sourceType);
    std::vector<std::shared_ptr<KMBuilderSource>> sources;
    YAML::Node dirNode;
    dirNode["path"] = "..";
    source->deserialize(dirNode);
    sources.push_back(source);
    component->setBuildSources(sources);
    return component;

}

void KMBuilderCreateFromDeb::fillManifest()
{
    debianToManifest();
    d->m_manifest->m_version = "1";
    d->m_manifest->m_modules["binary"] = ".*";
    d->m_manifest->m_channel = "stable";
    if ( !d->m_kmOptions->m_gui)
    {
        d->m_manifest->m_commands.push_back("{Please fill command}");
    }
    d->m_manifest->m_package.m_id = d->m_kmOptions->m_appid;
    d->m_manifest->m_package.m_kind = PACKGE_KIND_APP;
    d->m_manifest->m_base = d->m_kmOptions->m_base;
    d->m_manifest->m_runtime = d->m_kmOptions->m_runtime;

    d->m_manifest->m_components.push_back(setComponent());
}

std::string KMBuilderCreateFromDeb::getDeployDir(const std::string &kind, const std::string &info)
{
    KMRef ref;
    ref.parseInfo(info);
    std::string home_path = KMBuilderUtils::getHomeDir();
    std::string arch = KMBuilderUtils::getArch();
    std::string layers_path = KMBuilderUtils::buildFilename(home_path, ".local/share/kaiming/layers/");
    return layers_path + "/" + ref.m_channel + "/" + arch + "/" + kind + "/" + ref.m_id + "/" + MODULE_DEVEL + "/" + ref.m_version + "/";
} 

void KMBuilderCreateFromDeb::fillDepends()
{
    if (!d->m_components.empty())
    {
        for (auto &component: d->m_components)
        {
            if (component.first.empty())
            {
                continue;
            }
            std::string depend = component.first + " {need to create depend}";
            if (!component.second.empty())
            {
                depend = component.first + "(" + component.second + ") {need to create depend}";
            }
            d->m_manifest->m_depends.push_back(depend);
        }
    }
}

void KMBuilderCreateFromDeb::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);

    //校验base和runtime
    KMInstalledAppQuery query;
    std::optional<Ref> baseRef = KMInstalledAppQuery::parsePref(d->m_kmOptions->m_base);
    std::vector<Ref> bases = query.kind("base")
                                  .id(baseRef->id)
                                  .module("devel")
                                  .version(baseRef->version)
                                  .query(KMInstalledAppQuery::User);
    if (bases.empty())
    {
        throw KMException("Please install " + d->m_kmOptions->m_base);
    }
    const Ref& base = bases.at(0);
    std::string baseDeployDir = base.baseInstallDir + "/" + base.channel + "/" + base.arch + "/" + base.kind + "/" + base.id+ "/" + base.module + "/" + base.version;
    if (!fs::exists(baseDeployDir))
    {
        throw KMException("Please install " + d->m_kmOptions->m_base);
    }

    std::string runtimeDeployDir;
    if (!d->m_kmOptions->m_runtime.empty())
    {
        std::optional<Ref> runtimeRef = KMInstalledAppQuery::parsePref(d->m_kmOptions->m_runtime);
        std::vector<Ref> runtimes = query.kind("runtime")
                                        .id(runtimeRef->id)
                                        .module("devel")
                                        .version(runtimeRef->version)
                                        .query(KMInstalledAppQuery::User);
        if (runtimes.empty())
        {
            throw KMException("Please install " + d->m_kmOptions->m_runtime);
        }
        const Ref& runtime = runtimes.at(0);
        runtimeDeployDir = runtime.baseInstallDir + "/" + runtime.channel + "/" + runtime.arch + "/" + runtime.kind + "/" + runtime.id+ "/" + runtime.module + "/" + runtime.version;
        if (!fs::exists(runtimeDeployDir))
        {
            throw KMException("Please install " + d->m_kmOptions->m_runtime);
        }
    }

    // 以源debian源码目录作为项目父目录
    d->m_projectDir = KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "kaiming");
    if (!fs::exists(d->m_projectDir))
    {
        if (!fs::create_directories(d->m_projectDir))
        {
            throw KMException("Create project path " + d->m_projectDir + " failed");
        }
    }

    //加载debian数据
    std::string path_control, path_copyright;
    path_control = KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "debian", "control");
    path_copyright = KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "debian", "copyright");
    d->m_debian->parseControl(path_control);
    d->m_debian->parseCopyRight(path_copyright);

    //加载base和runtime软件包列表
    if (d->m_kmOptions->m_runtime.empty())
    {
        std::string dpkg_path_base = KMBuilderUtils::buildFilename(baseDeployDir, "files", "dpkg.list");
        loadDpkgList(dpkg_path_base);
    }
    else
    {
        std::string dpkg_path_runtime = KMBuilderUtils::buildFilename(runtimeDeployDir, "files", "dpkg.list");
        loadDpkgList(dpkg_path_runtime);
    }

    //query子命令部分获取本地及远程summaty.json中的depend
    loadDepends();
}

int KMBuilderCreateFromDeb::run()
{
    fillManifest();

    //生成yaml文件
    d->m_manifest->toYaml(KMBuilderUtils::buildFilename(d->m_projectDir, d->m_kmOptions->m_appid+".km.yaml"));

    generateChangelog();

    std::cout << "The project has been created successfully: " + d->m_projectDir + "\n";
    return EXIT_SUCCESS;
}

void KMBuilderCreateFromDeb::generateChangelog()
{
    std::string srcChangelog = KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "debian", "changelog");
    std::string destChangelog = KMBuilderUtils::buildFilename(d->m_kmOptions->m_source_path, "kaiming", "changelog");
    fs::copy_file(srcChangelog, destChangelog, fs::copy_options::overwrite_existing);

    std::vector<std::string> args;
    args.push_back("kch");
    args.push_back("--increment");
    args.push_back("--no-check-version");
    args.push_back("--append");
    args.push_back("Initial release, from debian to kaiming");

    KMProcessUtils::spawn("kaiming-builder", args);
}
