/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMCrun.h"

#include <unistd.h>
#include <sys/wait.h>

#include "KMConfigData.h"

/**
 * @brief 执行run命令
 * @param options 命令行参数
 * @return true 成功 false 失败
 */
bool KMCrun::exec(const std::vector<std::string> &options, const bool &isWait)
{
    std::vector<const char *> args;
    if (!getArgs(options, args))
        return false;

    std::variant<bool, std::string> varOutput = true;
    return doCmd(args, varOutput, isWait);
}

/**
 * @brief 获取进程参数
 * @param options 命令行参数
 * @param args 进程参数
 * @return true 成功 false 失败
 */
bool KMCrun::getArgs(const std::vector<std::string> &options, std::vector<const char *> &args)
{
    static std::string progress = KMCrun::getCrun();
    if (!fs::exists(progress))
    {
        KMError("crun do not exist, please install crun first");
        return false;
    }

    args.emplace_back(progress.c_str());
    for (auto &option : options)
    {
        args.emplace_back(option.c_str());
    }
    args.emplace_back(nullptr);

    return true;
}

/**
 * @brief 执行命令
 * @param processArgs 进程参数
 * @param output 输出
 * @return true 成功 false 失败
 */
bool KMCrun::doCmd(const std::vector<const char *> &processArgs, std::variant<bool, std::string> &output, const bool &isWait)
{
    if (!isWait)
    {
        if (execvp(processArgs[0], const_cast<char *const *>(processArgs.data())) == -1)
        {
            KMError("execvp failed: " + std::string(strerror(errno)));
            return false;
        }
        return true;
    }

    int pipes[2];
    if (pipe(pipes) == -1)
    {
        KMError("pipe failed: " + std::string(strerror(errno)));
        return false;
    }

    pid_t child = fork();
    if (child == -1)
    {
        close(pipes[0]);
        close(pipes[1]);
        KMError("fork failed: " + std::string(strerror(errno)));
        return false;
    }
    else if (child == 0)
    {
        close(pipes[0]);

        if (!std::holds_alternative<bool>(output) || !std::get<bool>(output))
        {
            dup2(pipes[1], STDOUT_FILENO);
        }

        if (execvp(processArgs[0], const_cast<char *const *>(processArgs.data())) == -1)
        {
            KMError("execvp failed: " + std::string(strerror(errno)));
            _exit(EXIT_FAILURE);
        }
    }

    close(pipes[1]);

    if (std::holds_alternative<std::string>(output))
    {
        constexpr size_t bufferSize = 1024;
        std::array<char, bufferSize> buffer{};
        ssize_t readCount;
        std::string content;
        while ((readCount = read(pipes[0], buffer.data(), bufferSize)) > 0)
        {
            content.append(buffer.data(), readCount);
        }
        close(pipes[0]);

        if (readCount == -1)
        {
            KMError("read failed: " + std::string(strerror(errno)));
            return false;
        }

        output = content;
    }

    close(pipes[0]);

    int status;
    if (waitpid(child, &status, 0) == -1)
    {
        KMError("waitpid failed: " + std::string(strerror(errno)));
        return false;
    }

    // 若信号退出则直接返回true
    if (WIFSIGNALED(status))
    {
        return true;
    }

    if (!WIFEXITED(status))
    {
        return false;
    }

    int exitCode = WEXITSTATUS(status);
    if (exitCode)
    {
        KMError("Exited with failure status, exitcode : " + std::to_string(exitCode));
        return false;
    }

    return true;
}

std::string KMCrun::getCrun()
{
    // 检查环境变量 KAIMING_BOX
    const char *e = std::getenv("KAIMING_BOX");
    if (e != nullptr)
    {
        return e;
    }

    // 默认路径
    return KAIMING_BOX;
}
