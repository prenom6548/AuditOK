/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMRepair.h"

#include <sys/personality.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>

#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "syslog.h"

/**
 * @brief 修复命令行选项
 */
class KMRepair::Options : public KMOption::Options
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
    string m_sideload;
    vector<string> m_vecRefs;
    std::string m_remote;

};

void KMRepair::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMRepair::Options::postParseHook()
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
void KMRepair::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming repair [options…] [quote…] - repair applications or runtime\n"));
    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display help options"));

    //应用选项

    //位置选项
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the application REF"));
}

class KMRepair::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMFolder> m_folder;
    std::shared_ptr<KMUtil> m_util;
    std::shared_ptr<KMEnv> m_env;
    map<string, KMRef> map_refs;
    shared_ptr<KMOSTreeHandler> ostree_handler;
    std::shared_ptr<KMPackageManager> m_packageManager;
    std::string m_remoteRepoName;
    std::string m_repoIndexUrl;
    std::string m_repoIndexLocalGzPath;
    std::string m_repoIndexLocalSubPath;

    stack<string> m_relatedRefs; // 先遍历运行时，再遍历app。因此先压入app,之后提取的时候顺序不会乱
};

REGISTER_SUBCOMMAND_DYNCREATE(repair, KMRepair)

static InstallCallBack globalProgressCallback;                     // 回调函数
static RepairProgressStatus installProgressStatus = { "", 0, "", 5 }; // 修复进度回调函数参数
static string progressJson = "";

static string lastOutput; // 保存最后一次进度条输出
static string globalLastId;
static string preRemaining = ""; // 保存先前的剩余时间

KMRepair::KMRepair()
    : d(std::make_unique<Private>())
{
    init();
}

KMRepair::~KMRepair() = default;

int KMRepair::dispose(int argc, char **argv)
{
    vector<string> vecRepair;

    d->m_options->checkUnknownOptions(argc, argv); 
    d->m_options->parseCommandLine(argc, argv);

    if (d->m_options->m_vecRefs.size() < 1)
    {
        kmlogger.error("Args error");
        KMError(_("Missing required argument REFS"));
        std::cerr << _("Use ") << ("'kaiming repair --help'") << _(" for usage.") << std::endl;
        return KM_ERROR_INVALID_ARGS;
    }

    for (const auto &ref : d->m_options->m_vecRefs)
    {
        vecRepair.emplace_back(ref);
    }

    int ret = start(vecRepair);

    return ret;
}

/**
 * @brief 开始对已获取的用户软件包列表执行修复
 *
 * @param vecName
 * @return int
 */
int KMRepair::start(const vector<string> &vectorIds)
{
    PackageManagerOptions pmOptions;
    pmOptions.relativeRepoPath = d->m_options->m_remote;
    int ret = d->m_packageManager->processTask(TaskType::TASK_TYPE_REPAIR, vectorIds, pmOptions);
    
    return ret;
}

void KMRepair::init()
{
    d->m_options = std::make_shared<Options>();
    d->m_folder = std::make_shared<KMFolder>("/opt/kaiming", false);
    d->ostree_handler = std::make_shared<KMOSTreeHandler>(d->m_folder->getRepoPath());
    d->m_util = std::make_shared<KMUtil>();
    d->m_env = std::make_shared<KMEnv>();
    d->m_packageManager = std::make_shared<KMPackageManager>();
}

void KMRepair::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
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
                std::cerr << _("Please use ") << ("'kaiming repair --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}