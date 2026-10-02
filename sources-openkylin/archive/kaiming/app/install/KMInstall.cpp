/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMInstall.h"

#include <sys/personality.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <nlohmann/json.hpp>
#include <unordered_set>

#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "syslog.h"

using namespace nlohmann;

/**
 * @brief 安装子命令的参数解析
 */
class KMInstall::Options : public KMOption::Options
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
    bool m_skipSignatureVerification;
    bool m_force;
    bool m_compatibility;
    std::string m_kind;
    std::vector<string> m_vecRefs;
    unsigned int m_maxSpeedLimitKilobytes; // 限速参数
    bool m_bundle;
    std::string m_subpath;
};

void KMInstall::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMInstall::Options::postParseHook()
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
void KMInstall::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming install [option…] [quote…] - Install applications or runtime\n"));
    // 帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_help)->defaultValue(false), _("display help information"));
    addOption("assumeyes", "y", KMOption::value<bool>(&m_assumeYes)->defaultValue(false), _("automatic answer to all questions is yes"));
    addOption("force", "f", KMOption::value<bool>(&m_force)->defaultValue(false), _("force installation of already installed applications"));
    addOption("compatibility", "c", KMOption::value<bool>(&m_compatibility)->defaultValue(false), _("install a compatible version"));
    // TODO:先默认跳过签名
    addOption("skip-signature-verification", "", KMOption::value<bool>(&m_skipSignatureVerification)->defaultValue(false), _("skip signature verification during installation"));
    addOption("kind", "", KMOption::value<std::string>(&m_kind), _("specify the type, such as app, runtime, base, extension, depend"));

    // 添加限速选项
    addOption("max-speed", "", KMOption::value<unsigned int>(&m_maxSpeedLimitKilobytes)->defaultValue(0), _("maximum download speed in KB/s, 0 means no limit"));
    addOption("bundle", "", KMOption::value<bool>(&m_bundle)->defaultValue(false), _("Download in offline package formatDownload in offline package format"));
    addOption("subpath", "", KMOption::value<std::string>(&m_subpath), _("Offline package download path"));

    // 位置选项
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the application REF, the supported formats are as follows:\n"
                                                                                           "  id\n"
                                                                                           "  id/version\n"
                                                                                           "  id/version/module\n"
                                                                                           "  id/version/module/channel"));
}

class KMInstall::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMPackageManager> m_packageManager;
};

REGISTER_SUBCOMMAND_DYNCREATE(install, KMInstall)

KMInstall::KMInstall()
    : d(std::make_unique<Private>())
{
    init();
}

KMInstall::~KMInstall() = default;

/**
 * @brief 命令行格式支持全部在线安装和全部离线安装两种格式，不支持混合在线和离线安装
 * 
 * 全部在线安装：id，或者全格式: kaiming-repo:stable/${id}/${version}/${arch}/${module} 
 * 全部离线安装：统一 .ok 后缀
 */
int KMInstall::dispose(int argc, char **argv)
{
    vector<string> vecInstall;

    d->m_options->checkUnknownOptions(argc, argv);  // 检查未知选项
    d->m_options->parseCommandLine(argc, argv);     // 正常解析已知选项

    if (d->m_options->m_vecRefs.empty())
    {
        kmlogger.error("Args error");
        KMError(_("Missing required argument REFS"));
        std::cerr << _("Use ") << ("'kaiming install --help'") << _(" for usage.") << std::endl;
        return KM_ERROR_INVALID_ARGS;
    }

    for (const auto &ref : d->m_options->m_vecRefs)
    {
        vecInstall.emplace_back(ref);
    }

    int ret = start(vecInstall);

    return ret;
}

/**
 * @brief 开始对已获取的用户软件包列表执行安装
 * 
 * 此处分为离线包和在线包下载。
 * 
 * 统一离线包格式： .ok 后缀 
 *
 * @param vectorAppIds 待安装的用户软件包列表
 * @return int
 */
int KMInstall::start(const vector<string> &vectorAppIds)
{
    PackageManagerOptions pmOptions;
    pmOptions.assumeYes = d->m_options->m_assumeYes;
    pmOptions.skipSignatureVerification = d->m_options->m_skipSignatureVerification;
    pmOptions.kind = d->m_options->m_kind;
    pmOptions.force = d->m_options->m_force;
    pmOptions.compatible = d->m_options->m_compatibility;
    pmOptions.maxSpeedLimitKilobytes = d->m_options->m_maxSpeedLimitKilobytes;
    pmOptions.bundle = d->m_options->m_bundle;
    pmOptions.subpath = d->m_options->m_subpath;

    int ret = d->m_packageManager->processTask(TaskType::TASK_TYPE_INSTALL, vectorAppIds, pmOptions);
    if (ret == KM_ERROR_ALREADY_INSTALLED ) {
        return EXIT_SUCCESS;
    }

    return ret;
}

void KMInstall::init()
{
    d->m_options = make_shared<Options>();
    d->m_packageManager = make_shared<KMPackageManager>();
}

void KMInstall::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--assumeyes", "-y",
        "--force", "-f",
        "--skip-signature-verification",
        "--kind",
        "--compatibility", "-c",
        "--max-speed",
        "--loglevel",
        "--verbose", "-v",
        "--bundle",
        "--subpath"
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
                std::cerr << _("Please use ") << ("'kaiming install --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}