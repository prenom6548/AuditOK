/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMUpdate.h"

#include <fcntl.h>
#include <sys/personality.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>

#include "app/list/KMList.h"
#include "common/KMPackageManager.h"
#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"

/**
 * @brief 更新子命令的参数解析
 */
class KMUpdate::Options : public KMOption::Options
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
    bool m_upgradable;
    vector<string> m_vecRefs;
};

void KMUpdate::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMUpdate::Options::postParseHook()
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
void KMUpdate::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming update [options] [quote…] - upgrade package description\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_help)->defaultValue(false), _("display Help Options"));
    addOption("assumeyes", "y", KMOption::value<bool>(&m_assumeYes)->defaultValue(false), _("automatic answer to all questions is yes"));
    addOption("upgradable", "", KMOption::value<bool>(&m_upgradable)->defaultValue(false), _("List all upgradable applications"));

    //位置选项
    addPositionOption("REFS", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the application REF"));
}

class KMUpdate::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMPackageManager> m_packageManager;
};

REGISTER_SUBCOMMAND_DYNCREATE(update, KMUpdate)

KMUpdate::KMUpdate()
    : d(std::make_unique<Private>())
{
    init();
}

KMUpdate::~KMUpdate() = default;

int KMUpdate::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv); 
    vector<string> vecUpdate;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            vecUpdate.emplace_back(argv[i]);
        }
        else if (!vecUpdate.empty())
        {
            kmlogger.error("args error");
            return KM_ERROR_INVALID_ARGS;
        }
    }

    // 参数解析
    d->m_options->parseCommandLine(argc, argv);

    int ret = start(vecUpdate);

    return ret;
}

/**
 * @brief 开始对已获取的用户软件包列表执行更新
 *
 * @param vecName
 * @return int
 */
int KMUpdate::start(const vector<string> &vecName)
{
    int ret = 0;
    if (vecName.empty())
    {
        std::cout << _("Search matching packages... ") << std::endl;
        KMList listIds;
        std::vector<KMlocalAppsInfo> localAppsInfo = listIds.getLocalAppIds(false);

        PackageManagerOptions pmOptions;
        pmOptions.assumeYes = d->m_options->m_assumeYes;
        pmOptions.upgradable = d->m_options->m_upgradable;
        pmOptions.isGetDevel = true;
        pmOptions.isPrint = false;
        ret = d->m_packageManager->getUpgradableList(localAppsInfo, pmOptions);
        if (ret != KM_ERROR_NO)
        {
            return ret;
        }

        if (!d->m_options->m_upgradable)
        {
            // 通过 D-Bus 调用系统更新，参数传 "", ""
            GError *error = nullptr;
            GDBusConnection *connection = nullptr;
            GVariant *result = nullptr;

            // 连接到系统总线
            connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
            if (!connection)
            {
                kmlogger.error("Failed to connect to system bus: %s", error->message);
                g_clear_error(&error);
                return KM_ERROR_NO;
            }

            // 检查服务是否存在
            GVariant *nameOwner = g_dbus_connection_call_sync(
                connection,
                "org.freedesktop.DBus",                    // DBus 服务
                "/org/freedesktop/DBus",                   // DBus 对象路径
                "org.freedesktop.DBus",                    // DBus 接口
                "GetNameOwner",                            // 方法名：获取服务所有者
                g_variant_new("(s)", "org.kylin.kaiming"), // 参数：服务名
                G_VARIANT_TYPE("(s)"),                     // 返回类型：字符串
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                nullptr,
                &error);

            if (error || !nameOwner)
            {
                if (error)
                {
                    kmlogger.info("org.kylin.kaiming service not available: %s", error->message);
                    g_clear_error(&error);
                }
                else
                {
                    kmlogger.info("org.kylin.kaiming service not available");
                }

                g_object_unref(connection);
                return KM_ERROR_NO;
            }

            // 调用 D-Bus 方法，参数为两个空字符串
            result = g_dbus_connection_call_sync(
                connection,
                "org.kylin.kaiming",           // 服务名
                "/org/kylin/kaiming",          // 对象路径
                "org.kylin.kaiming.interface", // 接口名
                "update",                      // 方法名
                g_variant_new("(ss)", "", ""), // 参数：两个空字符串 "", ""
                G_VARIANT_TYPE("(b)"),         // 返回类型：布尔值
                G_DBUS_CALL_FLAGS_NONE,
                -1,
                nullptr,
                &error);

            if (error)
            {
                kmlogger.error("D-Bus call failed: %s", error->message);
                g_clear_error(&error);
                ret = KM_ERROR_NO;
            }
            else
            {
                gboolean success = FALSE;
                g_variant_get(result, "(b)", &success);
                kmlogger.info("D-Bus update call completed with result: %s", success ? "true" : "false");

                ret = KM_ERROR_NO;
                g_variant_unref(result);
            }

            g_object_unref(connection);
        }
    }
    else
    {
        PackageManagerOptions pmOptions;
        pmOptions.assumeYes = d->m_options->m_assumeYes;

        ret = d->m_packageManager->processTask(TaskType::TASK_TYPE_UPDATE, vecName, pmOptions);
    }

    return ret;
}

void KMUpdate::init()
{
    d->m_options = std::make_shared<Options>();
    d->m_packageManager = std::make_shared<KMPackageManager>();
}

void KMUpdate::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--assumeyes", "-y",
        "--upgradable",
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
                std::cerr << _("Please use ") << ("'kaiming update --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}