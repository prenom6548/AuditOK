/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMKill.h"

#include <unordered_set>
#include <sstream>
#include <iomanip>

#include "common/KMLogger.h"
#include "common/kmtranslation.h"
#include "common/KMContainer.h"
#include "common/KMDbusProxyCall.h"
#include "common/KMConfigData.h"

class KMKill::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    void checkUnknownOptions(int argc, char **argv);

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
    std::string id;
};

void KMKill::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMKill::Options::postParseHook()
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
void KMKill::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming kill [option…] ref - Stop the running container application\n"));

    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("Display Help Options"));

    //位置选项
    std::stringstream ss;
    ss << _("ref support types:") << "\n"
       << std::setw(34) << " " << _("containerId containerId") << "\n"
       << std::setw(34) << " " << _("pid processID") << "\n"
       << std::setw(34) << " " << _("id applicationID");
    addPositionOption("ref", KMOption::value<std::string>(&id), 1, ss.str());
}

REGISTER_SUBCOMMAND_DYNCREATE(kill, KMKill)

class KMKill::Private
{
public:
    std::unique_ptr<Options> options;
};

KMKill::KMKill()
    : m_dPtr(std::make_shared<Private>())
{
    m_dPtr->options = std::make_unique<Options>();
}

KMKill::~KMKill()
{
}

/**
 * @brief 调用dbus接口杀死进程
 * @param id 应用名
 */
bool KMKill::kill(std::string id)
{
    if (id.empty())
    {
        KMError("ref is not empty!");
        return false;
    }

    KMDbusProxyCall dbusCall("org.kaiming.proxy", "/org/kaiming/proxy", "org.kaiming.proxy.system");
    std::tuple<int, std::string> reply;
    GVariant *var = g_variant_new("(s)", id.c_str());
    bool ret = dbusCall.methodCallSync<std::tuple<int, std::string>>("kill", var, "(is)", reply, true);
    if (!ret)
    {
        KMError("dbus call failed");
        return false;
    }

    auto [reti, msg] = reply;

    if (reti != 0)
    {
        KMError(msg);
        return false;
    }

    return true;
}

/**
 * @brief 子命令运行
 * @param argc 参数个数
 * @param argv 参数数组
 * @return int 0 成功
 */
int KMKill::dispose(int argc, char **argv)
{
    m_dPtr->options->checkUnknownOptions(argc, argv);
    m_dPtr->options->parseCommandLine(argc, argv);
    return kill(m_dPtr->options->id) ? 0 : -1;
}

void KMKill::Options::checkUnknownOptions(int argc, char **argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help",
        "-h",
        "--loglevel",
        "--verbose",
        "-v",
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
                KMError("Please use 'kaiming kill --help' to see available options.");
                exit(EXIT_FAILURE);
            }
        }
    }
}