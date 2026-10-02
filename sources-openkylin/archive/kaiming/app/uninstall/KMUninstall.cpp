/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMUninstall.h"

#include <memory>
#include <unordered_set>

#include "common/KMError.h"
#include "common/KMFolder.h"
#include "common/KMLogger.h"
#include "common/KMOSTreeHandler.h"

REGISTER_SUBCOMMAND_DYNCREATE(uninstall, KMUninstall)

class KMUninstall::Options : public KMOption::Options
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
    bool m_assumeYes;
    bool m_deleteDataYes;

    string m_sideload;
    string m_repoPath;
    vector<string> m_vecRefs;
};

void KMUninstall::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMUninstall::Options::postParseHook()
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
void KMUninstall::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming uninstall [Options…] [quote…] - uninstalling package description\n"));

    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display help options"));
    addOption("assumeyes", "y", KMOption::value<bool>(&m_assumeYes)->defaultValue(false), _("automatic answer to all questions is yes"));
    //应用选项
    addOption("delete-data", "", KMOption::value<bool>(&m_deleteDataYes)->defaultValue(false), _("delete user data"));

    //位置选项
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the application REF"));
}

class KMUninstall::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMPackageManager> m_packageManager;
};

KMUninstall::KMUninstall()
    : m_dPtr(make_shared<Private>())
{
    init();
}

KMUninstall::~KMUninstall() = default;

int KMUninstall::dispose(int argc, char **argv)
{
    vector<string> vecUninstall;

    m_dPtr->m_options->checkUnknownOptions(argc, argv); 
    m_dPtr->m_options->parseCommandLine(argc, argv);

    if (m_dPtr->m_options->m_vecRefs.empty())
    {
        kmlogger.error("Args error");
        KMError(_("Missing required argument REFS"));
        std::cerr << _("Use ") << ("'kaiming uninstall --help'") << _(" for usage.") << std::endl;
        return KM_ERROR_INVALID_ARGS;
    }

    for (const auto &ref : m_dPtr->m_options->m_vecRefs)
    {
        vecUninstall.emplace_back(ref);
    }

    return process(vecUninstall);
}

/// @brief 暂时只支持卸载 app，runtime 不支持卸载
/// @param vec_name 要卸载的 app
/// @return
int KMUninstall::process(const vector<string> &vectorAppIds)
{
    PackageManagerOptions pmOptions;
    pmOptions.assumeYes = m_dPtr->m_options->m_assumeYes;
    pmOptions.deletedataYes = m_dPtr->m_options->m_deleteDataYes;
    pmOptions.relativeRepoPath = m_dPtr->m_options->m_repoPath;

    int ret = m_dPtr->m_packageManager->processTask(TaskType::TASK_TYPE_UNINSTALL, vectorAppIds, pmOptions);
    return ret;
}

void KMUninstall::init()
{
    m_dPtr->m_options = std::make_shared<Options>();
    m_dPtr->m_packageManager = std::make_shared<KMPackageManager>();
}

void KMUninstall::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--assumeyes", "-y",
        "--delete-data",
        "--unused",
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
                std::cerr << _("Please use ") << ("'kaiming uninstall --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}