/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMPS.h"

#include <unordered_set>

#include "common/KMLogger.h"
#include "common/KMDbusProxyCall.h"
#include "common/KMConfigData.h"
#include "common/KMCrun.h"
#include "common/kmtranslation.h"

namespace fs = std::filesystem;

/**
 * @brief 安装子命令的参数解析
 */
class KMPS::Options : public KMOption::Options
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
    std::vector<std::string> m_vecRefs;
};

void KMPS::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMPS::Options::postParseHook()
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
void KMPS::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming ps [options]  - list the running container applications\n"));
    addOption("help", "h", KMOption::value<bool>(&m_bHelp), _("display help options"));
}

class KMPS::Private
{
public:
    std::shared_ptr<Options> m_options;
};

REGISTER_SUBCOMMAND_DYNCREATE(ps, KMPS)

KMPS::KMPS()
    : d_ptr(std::make_shared<Private>())
{
    d_ptr->m_options = std::make_shared<Options>();
}

KMPS::~KMPS() = default;

/**
 * @brief 调用dbus接口查看正在运行的容器应用
 */
bool KMPS::ps()
{
    std::tuple<int, std::string> reply;
    KMDbusProxyCall dbusCall("org.kaiming.proxy", "/org/kaiming/proxy", "org.kaiming.proxy.system");
    bool ret = dbusCall.methodCallSync<std::tuple<int, std::string>>("ps", nullptr, "(is)", reply, true);
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

    try
    {
        nlohmann::json j = nlohmann::json::parse(msg);
        KMPSInfoList psInfos = j.get<KMPSInfoList>();
        printAppInfo(psInfos);
    }
    catch (const nlohmann::json::exception &e)
    {
        KMError(std::string("Failed to parse response JSON: ") + e.what());
        return false;
    }

    return true;
}

/**
 * @brief ps 命令参数处理,不需要参数
 * 列出所有正在运行的容器应用,只支持不含参数
 * 使用实例:
 * kaiming ps
 * @param argc 参数个数
 * @param argv 参数列表
 * @return int 0:成功,非0:失败
 */
int KMPS::dispose(int argc, char **argv)
{
    d_ptr->m_options->checkUnknownOptions(argc, argv);
    d_ptr->m_options->parseCommandLine(argc, argv);
    return ps() ? 0 : -1;
}

/**
 * @brief 格式化输出包信息
 */
void KMPS::printAppInfo(const KMPSInfoList &psInfos) const
{
    // 打印表头
    printf("%-32s %-52s %-12s %-80s\n", _("appid"), _("containerid"), _("pid"), _("path"));

    // 遍历数组并打印每个元素
    for (const auto &psInfo : psInfos.infolist)
    {
        printf(_("%-30s %-50s %-10s %-80s\n"),
               psInfo.appid.c_str(),
               psInfo.cid.c_str(),
               std::to_string(psInfo.pid).c_str(),
               psInfo.path.c_str());
    }
}

void KMPS::Options::checkUnknownOptions(int argc, char **argv)
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
                KMError("Please use 'kaiming ps --help' to see available options.");
                exit(EXIT_FAILURE);
            }
        }
    }
}