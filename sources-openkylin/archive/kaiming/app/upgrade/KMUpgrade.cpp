/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMUpgrade.h"

#include <fcntl.h>
#include <sys/personality.h>
#include <grp.h>
#include <pwd.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <unordered_set>

#include "app/list/KMList.h"
#include "app/search/KMSearch.h"
#include "common/KMPackageManager.h"
#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/KMDbusJson.h"
#include "common/KMVersion.h"
#include "common/KMFileUtils.h"

/**
 * @brief 更新子命令的参数解析
 */
class KMUpgrade::Options : public KMOption::Options
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
    bool m_help;
    bool m_assumeYes;
};

void KMUpgrade::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMUpgrade::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief 命令行参数添加描述选项
 */
void KMUpgrade::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming upgrade - upgrade all packages\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_help)->defaultValue(false), _("display Help Options"));
    addOption("assumeyes", "y", KMOption::value<bool>(&m_assumeYes)->defaultValue(false), _("automatic answer to all questions is yes"));
}

class KMUpgrade::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMPackageManager> m_packageManager;
    std::shared_ptr<KMList> m_list;
};

REGISTER_SUBCOMMAND_DYNCREATE(upgrade, KMUpgrade)

KMUpgrade::KMUpgrade()
    : d(std::make_unique<Private>())
{
    init();
}

KMUpgrade::~KMUpgrade() = default;

int KMUpgrade::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv); 
    vector<string> vecUpgrade;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            vecUpgrade.emplace_back(argv[i]);
        }
        else if (!vecUpgrade.empty())
        {
            kmlogger.error("args error");
            return KM_ERROR_INVALID_ARGS;
        }
    }

    // 参数解析
    d->m_options->parseCommandLine(argc, argv);
    int ret = start(vecUpgrade);
    return ret;
}

/**
 * @brief 开始对已获取的用户软件包列表执行升级
 *
 * @param vecName
 * @return int
 */
int KMUpgrade::start(const vector<string> &vecName)
{
    PackageManagerOptions pmOptions;
    pmOptions.assumeYes = d->m_options->m_assumeYes;
    pmOptions.isUpgradeType = true;
    pmOptions.force = true;
    pmOptions.isPrint = false;
    pmOptions.isGetDevel = true;
    std::cout << _("Search matching packages... ") << std::endl;
    std::vector<std::string> filterIds = d->m_packageManager->getUpgradeInfo(pmOptions);

    int ret = 0;
    if (!filterIds.empty())
    {
        ret = d->m_packageManager->processTask(TaskType::TASK_TYPE_UPGRADE, filterIds, pmOptions);
    }

    return ret;
}

void KMUpgrade::init()
{
    d->m_options = std::make_shared<Options>();
    d->m_packageManager = std::make_shared<KMPackageManager>();
    d->m_list = std::make_shared<KMList>();
}

void KMUpgrade::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--assumeyes", "-y",
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
                std::cerr << _("Please use ") << ("'kaiming upgrade --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}
