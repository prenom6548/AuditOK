/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMRunnerOnce.h"

#include "common/KMLogger.h"
#include "common/KMDirUtils.h"
#include "common/KMConfigData.h"
#include "common/KMCrun.h"
#include "common/KMDbusProxyCall.h"

/**
 * @brief 再次运行应用
 * @param appid 应用ID
 * @param commandArgs 运行命令及参数
 * @param sockPath 套接字路径
 * @return true 成功 false 失败
 */
bool KMRunnerOnce::run(const std::string &appid, const std::string &commandArgs, const std::string &sockPath, const std::vector<std::string> &envs)
{
    std::string containerid;
    if (!isAlreadyRun(appid, containerid))
    {
        KMDebug("Application is not running: " + appid);
        return false;
    }

    std::vector<std::string> soptions;
    if (!assembleRunOnceOptions(sockPath, envs, soptions))
    {
        return false;
    }

    if (!runCmd({}, soptions, containerid, commandArgs))
    {
        return false;
    }

    return true;
}

/**
 * @brief 应用是否已运行
 * @return true 成功 false 失败
 */
bool KMRunnerOnce::isAlreadyRun(const std::string &appid, std::string &containerid)
{
    if (appid.empty())
    {
        return false;
    }
    //获取运行状态
    std::tuple<bool, std::string> reply;
    KMDbusProxyCall dbusCall("org.kaiming.proxy", "/org/kaiming/proxy", "org.kaiming.proxy.system");
    GVariant *var = g_variant_new("(ss)", appid.c_str(), std::to_string(getuid()).c_str());
    if (!dbusCall.methodCallSync("getRunStatus", var, "(bs)", reply, true))
    {
        KMError("getRunStatus failed.");
        return false;
    }
    auto [status, cid] = reply;
    containerid = cid;
    return status;
}

/**
 * @brief 运行命令
 * @param options 主命令参数
 * @param soptions 子命令参数
 * @param cid 容器ID
 * @param cmd 运行命令
 * @return true 成功 false 失败
 */
bool KMRunnerOnce::runCmd(const std::vector<std::string> &options, const std::vector<std::string> &soptions, const std::string &cid, const std::string &cmd)
{
    if (cid.empty() || soptions.empty())
    {
        return false;
    }

    std::vector<const char *> optv, soptv;
    if (!options.empty())
    {
        optv.reserve(options.size());
        for (auto &o : options)
        {
            optv.emplace_back(o.c_str());
        }
    }

    soptv.reserve(soptions.size());
    for (auto &s : soptions)
    {
        soptv.emplace_back(s.c_str());
    }

    std::tuple<int, std::string> reply;
    GVariant *opt = options.empty() ? g_variant_new_strv(nullptr, 0) : g_variant_new_strv(optv.data(), optv.size());
    GVariant *sopt = soptions.empty() ? g_variant_new_strv(nullptr, 0) : g_variant_new_strv(soptv.data(), soptv.size());
    GVariant *pcid = g_variant_new_string(cid.c_str());
    std::vector<GVariant *> tmp = { opt, sopt, pcid };
    if (!cmd.empty())
    {
        tmp.emplace_back(g_variant_new_string(cmd.c_str()));
    }
    GVariant *var = g_variant_new_tuple(tmp.data(), tmp.size());
    std::string method = cmd.empty() ? "run" : "enter";
    KMDbusProxyCall dbusCall("org.kaiming.proxy", "/org/kaiming/proxy", "org.kaiming.proxy.system");
    if (!dbusCall.methodCallSync(method.c_str(), var, "(is)", reply, true))
    {
        KMError("run dbus call failed");
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
 * @brief 组装二次运行命令
 * @param commandArgs 运行命令及参数
 * @param sockPath 套接字路径
 * @param envs 环境变量
 * @param cmd 输出的命令字符串
 * @return true 成功 false 失败
 */
bool KMRunnerOnce::assembleRunOnceOptions(const std::string &sockPath, const std::vector<std::string> &envs, std::vector<std::string> &args)
{
    args.emplace_back("--detach");

    for (const auto &env : envs)
    {
        args.emplace_back("--env=" + env);
    }

    if (!sockPath.empty())
    {
        args.emplace_back("--tty");
        args.emplace_back("--console-socket=" + sockPath);
    }

    if (std::string cwdDir; KMDirUtils::getCwdDir(cwdDir))
    {
        args.emplace_back("--cwd=" + cwdDir);
    }

    args.emplace_back("--user=" + std::to_string(getuid())).append(":").append(std::to_string(getgid()));

    return true;
}