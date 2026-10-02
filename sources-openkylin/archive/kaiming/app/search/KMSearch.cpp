/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSearch.h"

#include <sys/personality.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>

#include "common/KMBuildinsUtils.h"
#include "common/KMConfig.h"
#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/KMUtil.h"
#include "common/kmtranslation.h"
#include "common/KMParseSummaryJson.h"
#include "common/KMDbusJson.h"

class KMSearch::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    void checkUnknownOptions(int argc, char** argv);

protected:
    /**
     * @brief 命令行参数解析后操作
     */
    void preParseHook() override;

    /**
     * @brief 命令行参数解析后操作
     */
    void postParseHook() override;

private:
    /**
     * @brief 命令行参数添加描述选项
     */
    void addOptions();

public:
    bool m_bHelp;
    std::string m_remote;
    // std::string m_name;
    std::string m_kind;
    std::string m_module;
    vector<string> m_vecRefs;
};

void KMSearch::Options::preParseHook()
{
    addOptions();
}

void KMSearch::Options::postParseHook()
{
    if (m_bHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief 命令行参数添加描述选项
 */
void KMSearch::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming search [options…] [quote…] - search for remote software package description\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display Help Options"));

    //应用选项
    addOption("kind","", KMOption::value<std::string>(&m_kind), _("specify the type, such as app, runtime, base, extension"));
    addOption("module","", KMOption::value<std::string>(&m_module), _("specify the module, such as binary, devel"));
    //位置选项
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the application REF"));
}

class KMSearch::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMFolder> m_folder;
    std::string m_repoPath;
    std::shared_ptr<KMPackageManager> m_packageManager;

    bool isRemoteApi = false; // 获取远程信息，供 dbus api调用
    bool isSearchId;
};

REGISTER_SUBCOMMAND_DYNCREATE(search, KMSearch)

KMSearch::KMSearch()
    : d(std::make_unique<Private>())
{
    init();
}

KMSearch::~KMSearch() = default;

/**
 * @brief search 命令参数处理
 * 列出所有已安装的软件包信息,只支持不含参数
 * @param argc
 * @param argv
 * @return int
 */
int KMSearch::dispose(int argc, char **argv)
{
    vector<string> vecSearch;

    d->m_options->checkUnknownOptions(argc, argv);
    d->m_options->parseCommandLine(argc, argv);

    for (const auto &ref : d->m_options->m_vecRefs)
    {
        vecSearch.emplace_back(ref);
    }

    if (vecSearch.empty())
    {
        vecSearch.emplace_back("");
    }

    int ret = dbusStart(vecSearch);

    return ret;
}

int KMSearch::dbusStart(const vector<string> &vectorIds)
{
    PackageManagerOptions pmOptions;
    pmOptions.relativeRepoPath = d->m_options->m_remote;
    pmOptions.kind = d->m_options->m_kind;
    pmOptions.module = d->m_options->m_module;
    if (pmOptions.arch.empty())
    {
        pmOptions.arch = KMUtil::kmGetArch();
    }
    int ret = d->m_packageManager->dbusSearch(vectorIds[0], pmOptions);

    if ((d->m_options->m_remote == "kaiming-repo" || d->isRemoteApi) && !d->isSearchId)
    {
        // 没有在summary.json中找到可匹配的id
        return KM_ERROR_REMOTE_NOT_FOUND;
    }

    return ret;
}

int KMSearch::start(const vector<string> &vectorIds)
{
    PackageManagerOptions pmOptions;
    pmOptions.relativeRepoPath = d->m_options->m_remote;
    pmOptions.kind = d->m_options->m_kind;
    pmOptions.module = d->m_options->m_module;
    if (pmOptions.arch.empty())
    {
        pmOptions.arch = KMUtil::kmGetArch();
    }
    int ret = d->m_packageManager->processTask(TaskType::TASK_TYPE_SEARCH, vectorIds, pmOptions);

    if ((d->m_options->m_remote == "kaiming-repo" || d->isRemoteApi) && !d->isSearchId)
    {
        // 没有在summary.json中找到可匹配的id
        return KM_ERROR_REMOTE_NOT_FOUND;
    }

    return ret;
}

int KMSearch::getRemoteInfo(const std::string &name, std::string &strRemoteInfoJson, std::vector<std::string> &allRemoteInfoJson, std::string &remoteKind)
{
    d->isRemoteApi = true;
    d->isSearchId = false;

    PackageManagerOptions pmOptions;
    pmOptions.relativeRepoPath = d->m_options->m_remote;
    pmOptions.isPrint = false;
    pmOptions.kind = remoteKind;
    std::vector<std::string> allRemoteInfo;
    int errorCode = d->m_packageManager->searchInfo(name, pmOptions, allRemoteInfo);

    if (!allRemoteInfo.empty() && !allRemoteInfo[0].empty())
    {
        d->isSearchId = true;
    }

    if ((d->m_options->m_remote == "kaiming-repo" || d->isRemoteApi) && !d->isSearchId)
    {
        // 没有在summary.json中找到可匹配的id
        strRemoteInfoJson = KMDbusJson::errorRemoteJson();
        return KM_ERROR_REMOTE_NOT_FOUND;
    }

    if (errorCode != KM_ERROR_NO)
    {
        strRemoteInfoJson = KMDbusJson::errorRemoteJson();
    }
    else
    {
        if (!allRemoteInfo.empty())
        {
            for (auto &info : allRemoteInfo)
            {
                strRemoteInfoJson = KMDbusJson::parseRemotePathToJson(info);
                kmlogger.info("remoteInfoJson: %s", strRemoteInfoJson.c_str());
                allRemoteInfoJson.push_back(strRemoteInfoJson);
            }
            
        }
    }

    return errorCode;
}

void KMSearch::init()
{
    d->m_options = std::make_shared<Options>();
    d->m_folder = std::make_shared<KMFolder>("/opt/kaiming", false);
    d->m_packageManager = std::make_shared<KMPackageManager>();
}

void KMSearch::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--kind",
        "--module",
        "--loglevel",
        "--verbose", "-v",
    };

    for (int i = 1; i < argc; ++i) 
    {
        std::string arg(argv[i]);

        if (arg.size() >= 1 && arg[0] == '-') 
        {
            std::string opt = arg;
            auto eq_pos = opt.find('=');
            if (eq_pos != std::string::npos)
            {
                opt = opt.substr(0, eq_pos);
            }

            if (validOptions.find(opt) == validOptions.end()) 
            {
                KMError(_("Unrecognized option “") + arg + "”");
                std::cerr << _("Please use ") << ("'kaiming search --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}