/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMEnter.h"

#include <map>
#include <sstream>
#include <unistd.h>
#include <unordered_set>
#include <future>

#include "common/KMConfigData.h"
#include "common/KMDbusProxyCall.h"
#include "common/KMCrun.h"
#include "common/KMLogger.h"
#include "common/KMDirUtils.h"
#include "common/kmtranslation.h"
#include "common/KMUnixSocketServer.h"

REGISTER_SUBCOMMAND_DYNCREATE(enter, KMEnter)

/**
   * @brief 安装子命令的参数解析
   */
class KMEnter::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

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
    std::string ref;
    std::vector<std::string> args;
    std::string m_splitOfArgs;
};

void KMEnter::Options::preParseHook()
{
    addOptions();
}

/**
   * @brief 命令行参数解析后操作
   */
void KMEnter::Options::postParseHook()
{
    if (m_bHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    args.clear();
    int posRef = getPositionalOptionIndex("ref");
    int steps = m_splitOfArgs.empty() ? 1 : 2;
    if (posRef > 0 && m_originalArgs.size() > posRef + steps)
    {
        for (int i = posRef + steps; i < m_originalArgs.size(); i++)
        {
            args.emplace_back(m_originalArgs.at(i));
        }
    }
}
/**
   * @brief 命令行参数添加描述选项
   */
void KMEnter::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming enter [options...] ref [--] commands - run commands in a running container\n"));

    //帮助选项
    addOption("help", "h", KMOption::value(&m_bHelp)->defaultValue(false), _("display Help Options"));

    //位置选项
    std::stringstream ss;
    ss << _("ref support types:") << "\n"
       << std::setw(34) << " " << _("containerId containerId") << "\n"
       << std::setw(34) << " " << _("pid processID") << "\n"
       << std::setw(34) << " " << _("id applicationID");
    addPositionOption("ref", KMOption::value<std::string>(&ref), 1, ss.str());
    addPositionOption("--", KMOption::value(&m_splitOfArgs), 1, _("The start flag of command's arguments(not kaiming's options)"));
    addPositionOption("args", KMOption::value<std::vector<std::string>>(&args), -1, _("command parameter\n"));
}

class KMEnter::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::vector<std::string> commandArg;
    std::string sockPath;
    KMUnixSocketServer server;
};

KMEnter::KMEnter()
    : d_ptr(std::make_shared<Private>())
{
    init();
}

KMEnter::~KMEnter() = default;

/**
 * @brief 在正在运行的容器中运行命令
 * @param argc 参数个数
 * @param argv 参数列表
 * @return int 0:成功,非0:失败
 */
int KMEnter::dispose(int argc, char **argv)
{
    KMDebug("KMEnter::dispose invoke begin");
    if (!parseArgs(argc, argv))
    {
        return false;
    }

    return start() ? EXIT_SUCCESS : -1;
}

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数列表
 * @return true 成功 false 失败
 */
bool KMEnter::parseArgs(int argc, char **argv)
{
    // 参数解析
    d_ptr->m_options->parseCommandLine(argc, argv);

    for (auto &arg : d_ptr->m_options->args)
    {
        d_ptr->commandArg.emplace_back(arg);
    }

    if (d_ptr->commandArg.empty())
    {
        d_ptr->commandArg.emplace_back("/bin/bash");
    }

    return true;
}

/**
 * @brief 获取容器id
 * @param cid 命令
 * @return true 成功 false 失败
*/
bool KMEnter::getCid(std::string &cid)
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

    KMPSInfoList psInfos;
    try
    {
        nlohmann::json j = nlohmann::json::parse(msg);
        psInfos = j.get<KMPSInfoList>();
    }
    catch (const nlohmann::json::exception &e)
    {
        KMError((std::string("Failed to parse response JSON: ") + e.what()));
        return false;
    }

    bool isFind = false;
    std::string containerId;
    std::string id = d_ptr->m_options->ref;
    for (const auto &i : psInfos.infolist)
    {
        if (id == i.cid || id == i.appid || id == std::to_string(i.pid))
        {
            cid = i.cid;
            isFind = true;
            break;
        }
    }
    if (!isFind)
    {
        KMError("[enter] can not find " + id);
        return false;
    }

    return true;
}

/**
 * @brief 调用dbus 进入容器 
 * @param cid 容器id
 * @return true 运行成功 false 运行失败
 */
bool KMEnter::enter(std::string &cid)
{
    std::string cmd;
    for (auto const &arg : d_ptr->commandArg)
    {
        cmd = cmd.empty() ? cmd.append("\"").append(arg).append("\"") : cmd.append(" ").append("\"").append(arg).append("\"");
    }

    std::vector<const char *> soptv;
    auto sopts = assembleEnterOpts();
    for (auto &s : sopts)
    {
        soptv.emplace_back(s.c_str());
    }

    std::tuple<int, std::string> reply;
    GVariant *opt = g_variant_new_strv(nullptr, 0);
    GVariant *sopt = g_variant_new_strv(soptv.data(), soptv.size());
    GVariant *pcid = g_variant_new_string(cid.c_str());
    GVariant *pcmd = g_variant_new_string(cmd.c_str());
    std::vector<GVariant *> tmp = { opt, sopt, pcid, pcmd };
    GVariant *var = g_variant_new_tuple(tmp.data(), tmp.size());
    KMDbusProxyCall dbusCall("org.kaiming.proxy", "/org/kaiming/proxy", "org.kaiming.proxy.system");
    bool ret = dbusCall.methodCallSync("enter", var, "(is)", reply, true);
    if ((!ret))
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
 * @brief 开始运行
 * @return true 成功 false 失败
 */
bool KMEnter::start()
{
    std::string cid;
    if (!getCid(cid))
    {
        return false;
    }
    
    if (!startUnixSocketServer())
    {
        KMError("start unix socket server failed");
    }

    if (!enter(cid))
    {
        return false;
    }

    return wait();
}

/**
 * @brief 等待进程结束
 * @return true 成功 false 失败
 */
bool KMEnter::wait()
{
    return d_ptr->server.wait();
}

/**
 * @brief 组装运行参数
 * @return 返回参数列表
 */
std::vector<std::string> KMEnter::assembleEnterOpts()
{
    std::vector<std::string> opts;
    uid_t uid = getuid();
    gid_t gid = getgid();
    opts.emplace_back("--detach");
    opts.emplace_back("--user=" + std::to_string(uid) + ":" + std::to_string(gid));

    if (!d_ptr->sockPath.empty())
    {
        opts.emplace_back("--tty");
        opts.emplace_back("--console-socket=" + d_ptr->sockPath);
    }

    if (std::string cwdDir; KMDirUtils::getCwdDir(cwdDir))
    {
        opts.emplace_back("--cwd=" + cwdDir);
    }

    return opts;
}

void KMEnter::init()
{
    d_ptr->m_options = std::make_shared<Options>();
}

bool KMEnter::startUnixSocketServer()
{
    if (!d_ptr->server.create())
        return false;

    d_ptr->sockPath = d_ptr->server.getPath();

    return d_ptr->server.run();
}
