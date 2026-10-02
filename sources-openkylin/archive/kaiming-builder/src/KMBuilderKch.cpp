/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderKch.h"

#include <filesystem>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <locale>
#include <unistd.h>
#include <limits.h>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/kmtranslation.h"
#include "common/KMChangelog.h"
#include "common/KMProcessUtils.h"
#include "KMBuilderUtils.h"
#include "KMBuilderManifest.h"
#include "KMHostModeManifest.h"

REGISTER_SUBCOMMAND_DYNCREATE(kch, KMBuilderKch)

namespace fs = std::filesystem;

static std::string getCurrentDate() 
{
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    // 设置本地化为英语（C/POSIX标准或en_US.UTF-8）
    std::locale::global(std::locale("en_US.UTF-8"));  // 或 "C"

    std::ostringstream oss;
    oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S %z");
    return oss.str();
}

class KMBuilderKch::Options : public KMOption::Options
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
    bool m_isCreate = false;
    bool m_isIncrement = false;
    bool m_isShowLastVersion = false;
    bool m_isNoCheckVersion = false;
    std::string m_package;
    std::vector<std::string> m_entrys;
    std::string m_inputFile;
};

void KMBuilderKch::Options::preParseHook()
{
    addOptions();
}

void KMBuilderKch::Options::postParseHook()
{
    if (m_isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

void KMBuilderKch::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming-builder kch [options] \n"));

    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_isHelp), _("Show help options"));

    //应用选项
    addOption("create","c", KMOption::value<bool>(&m_isCreate), _("Create a new changelog"));
    addOption("increment","i", KMOption::value<bool>(&m_isIncrement), _("Add a new changelog record"));
    addOption("append","a", KMOption::value<std::vector<std::string>>(&m_entrys), _("Append a new entry to the current changelog"));
    addOption("no-check-version","", KMOption::value<bool>(&m_isNoCheckVersion), _("Don't check version"));
    addOption("last-version","", KMOption::value<bool>(&m_isShowLastVersion), _("Show the last version of changelog"));
    addOption("changelog-file","", KMOption::value<std::string>(&m_inputFile), _("Must be used with '--last-version' to specify the changelog file"));
}

class KMBuilderKch::Private
{
public:
    struct Package
    {
        std::string package;
        std::string version;
        bool isDeb = false;
    };
public:
    std::unique_ptr<Options> m_kmOptions;
    Package m_package;

    std::string m_manifest;
    std::string m_destDir;
    std::string m_changelog;

    std::string m_userName = "MaintainerName";
    std::string m_email = "email@example.com";
};

KMBuilderKch::KMBuilderKch()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuilderKch::~KMBuilderKch()
{
}

int KMBuilderKch::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMBuilderKch::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMBuilderKch::run()
{
    if (d->m_kmOptions->m_isCreate)
    {
        create();
    }
    else if (d->m_kmOptions->m_isIncrement)
    {
        increment();
    }
    else if (!d->m_kmOptions->m_entrys.empty())
    {
        append(d->m_kmOptions->m_entrys);
    }
    else if (d->m_kmOptions->m_isShowLastVersion)
    {
        km_logger.setLogLevel(KMLogLevel::err);

        showLastVersion();
    }

    return EXIT_SUCCESS;
}

void KMBuilderKch::create()
{
    if (!findManifest())
    {
        throw KMException("Not found manifest file (*.km.yaml or *.km.yml or manifest.yaml)");
    }

    initMaintainer();

    d->m_changelog = KMBuilderUtils::buildFilename(d->m_destDir, "changelog");
    if (fs::exists(d->m_changelog))
    {
        throw KMException(d->m_changelog + " already exists.");
    }

    KMChangelog changelog(d->m_changelog);
    ChangelogEntry entry;
    entry.package = d->m_package.package;
    entry.version = d->m_package.version;
    entry.distributions.push_back("UNRELEASED");
    entry.urgency = "medium";
    if (!d->m_kmOptions->m_entrys.empty())
    {
        for (auto& change : d->m_kmOptions->m_entrys)
        {
            entry.changes.push_back(change);
        }
    }
    else
    {
        entry.changes.push_back("Initial release. (Closes: #XXXXXX)");
    }

    entry.maintainer = d->m_userName + " <" + d->m_email + ">";
    entry.date = getCurrentDate();
    changelog.addFirst(entry);
}

void KMBuilderKch::increment()
{
    if (!findManifest())
    {
        throw KMException("Not found manifest file (*.km.yaml or *.km.yml or manifest.yaml)");
    }

    initMaintainer();

    d->m_changelog = KMBuilderUtils::buildFilename(d->m_destDir, "changelog");
    if (!fs::exists(d->m_changelog))
    {
        throw KMException("Not found changelog file : " + d->m_changelog);
    }

    ChangelogEntry lastEntry;
    {
        KMChangelog changelog(d->m_changelog);
        changelog.open();
        changelog.parseNext(lastEntry);
    }

    if (lastEntry.version.empty())
    {
        // 认为是空changelog文件
        lastEntry.distributions.clear();
        lastEntry.distributions.push_back("UNRELEASED");
    }
    else
    {
        if (!d->m_kmOptions->m_isNoCheckVersion && !d->m_package.isDeb && !compareVersions(d->m_package.version, ">", lastEntry.version))
        {
            throw KMException("The package.version property in *.km.yaml must be greater than the latest version number in the changelog");
        }
        else if (!d->m_kmOptions->m_isNoCheckVersion && d->m_package.isDeb && !compareVersions(d->m_package.version, "gt", lastEntry.version))
        {
            throw KMException("The version property in manifest.yaml must be greater than the latest version number in the changelog");
        }
    }
    lastEntry.package = d->m_package.package;
    lastEntry.version = d->m_package.version;
    lastEntry.urgency = "medium";
    lastEntry.changes.clear();
    if (!d->m_kmOptions->m_entrys.empty())
    {
        for (auto& change : d->m_kmOptions->m_entrys)
        {
            lastEntry.changes.push_back(change);
        }
    }
    else
    {
        lastEntry.changes.push_back("Non-maintainer upload.");
    }

    lastEntry.maintainer = d->m_userName + " <" + d->m_email + ">";
    lastEntry.date = getCurrentDate();

    KMChangelog changelog(d->m_changelog);
    changelog.add(lastEntry);
}

void KMBuilderKch::append(const std::vector<std::string>& entrys)
{
    if (!findManifest())
    {
        throw KMException("Not found manifest file (*.km.yaml or *.km.yml or manifest.yaml)");
    }
    d->m_changelog = KMBuilderUtils::buildFilename(d->m_destDir, "changelog");
    if (!fs::exists(d->m_changelog))
    {
        throw KMException("Not found changelog file : " + d->m_changelog);
    }

    ChangelogEntry lastEntry;
    KMChangelog changelog(d->m_changelog);
    changelog.open();
    if (!changelog.parseNext(lastEntry))
    {
        throw KMException("Not found changelog entry in file : " + d->m_changelog);
    }

    if (!entrys.empty())
    {
        for (auto& change : entrys)
        {
            lastEntry.changes.push_back(change);
        }
    }
    else
    {
        lastEntry.changes.push_back("");
    }

    changelog.update(lastEntry);
}

void KMBuilderKch::showLastVersion()
{
    if (d->m_kmOptions->m_inputFile.empty())
    {
        if (!findManifest())
        {
            throw KMException("Not found manifest file (*.km.yaml or *.km.yml or manifest.yaml)");
        }
        d->m_changelog = KMBuilderUtils::buildFilename(d->m_destDir, "changelog");
    }
    else
    {
        d->m_changelog = d->m_kmOptions->m_inputFile;
    }

    if (!fs::exists(d->m_changelog))
    {
        throw KMException("Not found changelog file : " + d->m_changelog);
    }

    ChangelogEntry lastEntry;
    KMChangelog changelog(d->m_changelog);
    changelog.open();
    if (!changelog.parseNext(lastEntry))
    {
        throw KMException("Not found changelog entry in file : " + d->m_changelog);
    }

    std::cout << lastEntry.version << std::endl;
}

bool KMBuilderKch::findManifest()
{
    fs::path curPath = fs::current_path();
    if (fs::exists(curPath / "kaiming"))
    {
        d->m_destDir = KMBuilderUtils::buildFilename(curPath.string(), "kaiming");
        for (auto const &dirEntry : fs::directory_iterator{ d->m_destDir })
        {
            fs::path p = dirEntry.path();
            if (fs::is_symlink(p))
            {
                continue;
            }
            else if (fs::is_regular_file(p))
            {
                d->m_manifest = p.string();
                std::string filename = p.filename();
                if (filename == "manifest.yaml")
                {
                    std::shared_ptr<KMHostModeManifest> manifestYaml = KMHostModeManifest::loadFromYAML(d->m_manifest);
                    d->m_package.package = manifestYaml->source;
                    d->m_package.version = manifestYaml->version;
                    d->m_package.isDeb = true;

                    return true;
                }
                else if (KMBuilderUtils::endsWith(d->m_manifest, ".km.yaml") || KMBuilderUtils::endsWith(d->m_manifest, ".km.yml"))
                {
                    std::shared_ptr<KMBuilderManifest> manifestYaml = KMBuilderManifest::loadFromYAML(d->m_manifest);
                    d->m_package.package = manifestYaml->m_package.m_id;
                    d->m_package.version = manifestYaml->m_package.m_version;
                    d->m_package.isDeb = false;

                    return true;
                }
            }
        }
    }
    else
    {
        d->m_destDir = curPath.string();
        for (auto const &dirEntry : fs::directory_iterator{ d->m_destDir })
        {
            fs::path p = dirEntry.path();
            if (fs::is_symlink(p))
            {
                continue;
            }
            else if (fs::is_regular_file(p))
            {
                d->m_manifest = p.string();
                if (KMBuilderUtils::endsWith(d->m_manifest, ".km.yaml") || KMBuilderUtils::endsWith(d->m_manifest, ".km.yml"))
                {
                    std::shared_ptr<KMBuilderManifest> manifestYaml = KMBuilderManifest::loadFromYAML(d->m_manifest);
                    d->m_package.package = manifestYaml->m_package.m_id;
                    d->m_package.version = manifestYaml->m_package.m_version;
                    d->m_package.isDeb = false;

                    return true;
                }
            }
        }
    }

    d->m_destDir = "";
    d->m_manifest = "";

    return false;
}

void KMBuilderKch::initMaintainer()
{
    std::vector<std::string> args;
    args.push_back("config");
    args.push_back("user.name");

    std::string result;
    if (KMBuilderUtils::searchExecutable("git").empty() || !KMProcessUtils::spawn("git", args, result))
    {
        KMWarn("Suggest the following configuration : git config --global user.name \"yourname\"");

        // 获取用户名
        char username[LOGIN_NAME_MAX];
        if (getlogin_r(username, sizeof(username)) == 0) {
             d->m_userName = username;
        }
    }
    else
    {
        KMBuilderUtils::replace(result, "\n", "");
        d->m_userName = result;
    }

    args.clear();
    result = "";
    args.push_back("config");
    args.push_back("user.email");
    if (KMBuilderUtils::searchExecutable("git").empty() || !KMProcessUtils::spawn("git", args, result))
    {
        KMWarn("Suggest the following configuration : git config --global user.email \"youremail\"");

        // 获取主机名
        char hostname[HOST_NAME_MAX];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            d->m_email = d->m_userName + "@" + hostname;
        }
    }
    else
    {
        KMBuilderUtils::replace(result, "\n", "");
        d->m_email = result;
    }
}

bool KMBuilderKch::compareVersions(const std::string& v1, const std::string& operation, const std::string& v2)
{
    std::vector<std::string> args;
    args.push_back("compare-versions");
    args.push_back(v1);
    args.push_back(operation);
    args.push_back(v2);

    bool rst = KMProcessUtils::spawn("kaiming", args, false);

    return rst;
}

bool KMBuilderKch::compareDebianVersions(const std::string& v1, const std::string& operation, const std::string& v2)
{
    std::vector<std::string> args;
    args.push_back("--compare-versions");
    args.push_back(v1);
    args.push_back(operation);
    args.push_back(v2);

    bool rst = KMProcessUtils::spawn("dpkg", args, false);

    return rst;
}
