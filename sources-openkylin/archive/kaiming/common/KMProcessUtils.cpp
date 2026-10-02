/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMProcessUtils.h"

#include <glib.h>
#include <gio/gunixoutputstream.h>
#include <gio/gunixinputstream.h>

#include "KMLogger.h"

/**
 * @brief : 判断参数是否需要用引号括起来
 * @return: true - 需要；false - 不需要
 */ 
bool KMProcessUtils::argumentNeedsQuoting(const char *arg)
{
    if (*arg == '\0')
    {
        return true;
    }

    while (*arg != 0)
    {
        char c = *arg;
        if (!std::isalnum(c) && !(c == '-' || c == '/' || c == '~' || c == ':' || c == '.' || c == '_' || c == '=' || c == '@'))
        {
            return true;
        }
        arg++;
    }

    return false;
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @return: true - 成功执行；false - 出现错误
*/
bool KMProcessUtils::spawn(const std::string &exec, const std::vector<std::string> &args, bool ignoreExitStatus)
{
    return KMProcessUtils::spawn(exec, "", args, ignoreExitStatus);
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序
 * @param : [in] workingDirectory, 工作目录
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @return: true - 成功执行；false - 出现错误
*/
bool KMProcessUtils::spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args, bool ignoreExitStatus)
{
    g_autoptr(GSubprocessLauncher) launcher =  g_subprocess_launcher_new(G_SUBPROCESS_FLAGS_NONE);

    GSubprocessFlags flags = G_SUBPROCESS_FLAGS_NONE;
    g_subprocess_launcher_set_flags(launcher, flags);

    if (!workingDirectory.empty())
    {
        g_subprocess_launcher_set_cwd(launcher, workingDirectory.c_str());
    }

    std::vector<char *> argv;
    argv.push_back(const_cast<char *>(exec.c_str()));
    for (auto &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    std::string cmdline = exec + " ";
    for (auto &arg : args)
    {
        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            g_autofree char *quoted = g_shell_quote(arg.c_str());
            cmdline += quoted;
        }
        else
        {
            cmdline += arg;
        }

        cmdline += " ";
    }
    KMDebug(cmdline);

    g_autoptr(GError) error = nullptr;
    g_autoptr(GSubprocess) subprocess = g_subprocess_launcher_spawnv(launcher, (char **)argv.data(), &error);
    if (subprocess == nullptr)
    {
        if (error)
        {
            KMError(error->message);
        }
        return false;
    }

    gboolean success = g_subprocess_wait(subprocess, nullptr, &error);
    if (!success)
    {
        if (error)
        {
            KMError(error->message);
        }
        return false;
    }

    if (!ignoreExitStatus)
    {
        if (g_subprocess_get_if_exited(subprocess))
        {
            if (g_subprocess_get_exit_status(subprocess))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序名称，不包含路径
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
 * @return: true - 成功执行；false - 出现错误
 */
bool KMProcessUtils::spawn(const std::string &exec, const std::vector<std::string> &args, std::string &result)
{
    return KMProcessUtils::spawn(exec, "", args, result);
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序名称，不包含路径
 * @param : [in] workingDirectory, 执行外部程序工作目录
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
 * @return: true - 成功执行；false - 出现错误
 */
bool KMProcessUtils::spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args, std::string &result)
{
    g_autoptr(GSubprocessLauncher) launcher =  g_subprocess_launcher_new(G_SUBPROCESS_FLAGS_NONE);

    GSubprocessFlags flags = (GSubprocessFlags)(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE);
    g_subprocess_launcher_set_flags(launcher, flags);

    if (!workingDirectory.empty())
    {
        g_subprocess_launcher_set_cwd(launcher, workingDirectory.c_str());
    }

    std::vector<char *> argv;
    argv.push_back(const_cast<char *>(exec.c_str()));
    for (auto &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    std::string cmdline = exec + " ";
    for (auto &arg : args)
    {
        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            g_autofree char *quoted = g_shell_quote(arg.c_str());
            cmdline += quoted;
        }
        else
        {
            cmdline += arg;
        }

        cmdline += " ";
    }
    KMDebug(cmdline);

    g_autoptr(GError) error = nullptr;
    g_autoptr(GSubprocess) subprocess = g_subprocess_launcher_spawnv(launcher, (char **)argv.data(), &error);
    if (subprocess == nullptr)
    {
        if (error)
        {
            result = error->message;
        }
        return false;
    }

    // 4. 等待子进程结束，并同时捕获 stdout 和 stderr 的内容
    gchar *stdout_buf = NULL;
    gchar *stderr_buf = NULL;
    if (!g_subprocess_communicate_utf8 (subprocess,      // 子进程
                                        NULL,            // 不向 stdin 写入数据
                                        NULL,            // 取消标记（不使用）
                                        &stdout_buf,      // 捕获 stdout
                                        &stderr_buf,      // 捕获 stderr
                                        &error))
    {
        if (error)
        {
            result = error->message;
        }
        return false;
    }

    if (stdout_buf != NULL)
    {
        result = (char *)stdout_buf;
    }
    

    if (!g_subprocess_get_successful (subprocess))
    {
        if (stderr_buf != NULL)
        {
            result += (char *)stderr_buf;
        }

        return false;
    }

    return true;
}

std::pair<std::string, int> KMProcessUtils::execCmd(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    int returnCode = -1;

    auto pcloseWrapper = [&returnCode](FILE *cmd) {
        returnCode = pclose(cmd);
    };

    const std::unique_ptr<FILE, decltype(pcloseWrapper)> pipe(popen(cmd, "r"), pcloseWrapper);
    if (pipe)
    {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }

    // 移除字符串末尾的换行符
    if (!result.empty() && result[result.length() - 1] == '\n') {
        result.pop_back();
    }

    return std::make_pair(result, returnCode);
}