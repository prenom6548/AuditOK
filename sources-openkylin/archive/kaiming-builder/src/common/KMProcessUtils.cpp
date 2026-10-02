/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMProcessUtils.h"

#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

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
bool KMProcessUtils::spawn(const std::string &exec, const std::vector<std::string> &args, bool ignoreExitCode)
{
    return KMProcessUtils::spawn(exec, "", args, ignoreExitCode);
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序
 * @param : [in] workingDirectory, 工作目录
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @return: true - 成功执行；false - 出现错误
*/
bool KMProcessUtils::spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args, bool ignoreExitCode)
{
    std::string cmdline(exec);
    std::vector<const char *> execvpArgs;
    execvpArgs.push_back(exec.c_str());
    for (auto &arg : args)
    {
        execvpArgs.push_back(arg.c_str());

        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            std::string quoted = "'" + arg + "'";
            cmdline += " " + quoted;
        }
        else
        {
            cmdline += " " + arg;
        }
    }
    execvpArgs.push_back(nullptr);
    KMDebug(cmdline);

    int pipefd[2];
    if (::pipe(pipefd) == -1)
    {
        KMError(::strerror(errno));
        return false;
    }

    pid_t pid = ::fork();
    if (pid == -1)
    {
        ::close(pipefd[0]);
        ::close(pipefd[1]);
        KMError(::strerror(errno));

        return false;
    }
    else if (pid == 0)
    {
        // 子进程
        ::close(pipefd[0]); // 关闭读端
        ::dup2(pipefd[1], STDOUT_FILENO); // 将标准输出重定向到管道的写端
        ::close(pipefd[1]); // 关闭原来的写端

        if (!workingDirectory.empty())
        {
            ::chdir(workingDirectory.c_str());
        }

        ::execvp(exec.c_str(), (char **)execvpArgs.data());

        // KMError(exec + " : " + ::strerror(errno)); // 如果execvp返回，说明执行失败
        ::exit(EXIT_FAILURE);
    }
    else
    {
        // 父进程
        ::close(pipefd[1]);  // 关闭写端
        char buffer[1025] = { 0 };
        ssize_t count = 0;

        // 从管道读取数据并输出到标准输出
        std::string tmp, last;
        bool rate = false;
        while ((count = ::read(pipefd[0], buffer, sizeof(char) * 1024)) > 0)
        {
            ::write(STDOUT_FILENO, buffer, count);

            tmp = buffer;
            size_t pos = tmp.find('%');
            size_t pos1 = tmp.find("##");
            size_t pos2 = tmp.find("B/s");
            if (pos != std::string::npos && (pos1 != std::string::npos || pos2 != std::string::npos) && pos > 0 && std::isdigit(tmp[pos-1]))
            {
                last = tmp;
                if (!rate) {
                    rate = true;
                }
            }
            else
            {
                if (rate)
                {
                    km_logger.log(last);
                    rate = false;
                    last = "";
                }
                km_logger.log(tmp);
            }

            ::memset(buffer, 0, 1025);
        }
        if (rate)
        {
            km_logger.log(last);
            rate = false;
            last = "";
        }
        ::close(pipefd[0]); // 关闭读端

        int status = 0;
        pid_t result = ::waitpid(pid, &status, 0);
        if (result == -1)
        {
            KMError(::strerror(errno));
            return false;
        }

        if (!ignoreExitCode)
        {
            if (WIFEXITED(status))
            {
                int exitCode = WEXITSTATUS(status);
                if (exitCode)
                {
                    // KMError(exec + " exited with failure status " + std::to_string(exitCode));
                    return false;
                }
            }
            else if (WIFSIGNALED(status))
            {
                // KMError(exec + std::string(" terminated by signal ") + std::to_string(WTERMSIG(status)));
                return false;
            }
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
bool KMProcessUtils::spawn(const std::string &exec, const std::vector<std::string> &args, std::string &result, bool ignoreExitCode)
{
    return KMProcessUtils::spawn(exec, "", args, result, ignoreExitCode);
}

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序名称，不包含路径
 * @param : [in] workingDirectory, 执行外部程序工作目录
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
 * @return: true - 成功执行；false - 出现错误
 */
bool KMProcessUtils::spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args, std::string &result, bool ignoreExitCode)
{
    std::string cmdline(exec);
    std::vector<const char *> execvpArgs;
    execvpArgs.push_back(exec.c_str());
    for (auto &arg : args)
    {
        execvpArgs.push_back(arg.c_str());

        if (KMProcessUtils::argumentNeedsQuoting(arg.c_str()))
        {
            std::string quoted = "'" + arg + "'";
            cmdline += " " + quoted;
        }
        else
        {
            cmdline += " " + arg;
        }
    }
    execvpArgs.push_back(nullptr);
    KMDebug(cmdline);

    int pipefd[2];
    if (::pipe(pipefd) == -1)
    {
        KMError(::strerror(errno));
        return false;
    }

    pid_t pid = ::fork();
    if (pid == -1)
    {
        ::close(pipefd[0]);
        ::close(pipefd[1]);
        KMError(::strerror(errno));

        return false;
    }
    else if (pid == 0)
    {
        // 子进程
        ::close(pipefd[0]); // 关闭读端
        ::dup2(pipefd[1], STDOUT_FILENO); // 将标准输出重定向到管道的写端
        ::close(pipefd[1]); // 关闭原来的写端

        if (!workingDirectory.empty())
        {
            ::chdir(workingDirectory.c_str());
        }

        ::execvp(exec.c_str(), (char **)execvpArgs.data());

        KMError(exec + " : " + ::strerror(errno)); // 如果execvp返回，说明执行失败
        ::exit(EXIT_FAILURE);
    }
    else
    {
        // 父进程
        ::close(pipefd[1]);  // 关闭写端
        char buffer[1025] = { 0 };
        ssize_t count = 0;

        // 从管道读取数据并输出到标准输出
        while ((count = ::read(pipefd[0], buffer, sizeof(char) * 1024)) > 0)
        {
            result.append(buffer);
            ::memset(buffer, 0, 1025);
        }
        ::close(pipefd[0]); // 关闭读端

        int status = 0;
        pid_t wpid = ::waitpid(pid, &status, 0);
        if (wpid == -1)
        {
            KMError(::strerror(errno));
            return false;
        }

        if (!ignoreExitCode)
        {
            if (WIFEXITED(status))
            {
                int exitCode = WEXITSTATUS(status);
                if (exitCode)
                {
                    KMError(exec + " exited with failure status " + std::to_string(exitCode));
                    return false;
                }
            }
            else if (WIFSIGNALED(status))
            {
                KMError(exec + std::string(" terminated by signal ") + std::to_string(WTERMSIG(status)));
                return false;
            }
        }
    }

    return true;
}
