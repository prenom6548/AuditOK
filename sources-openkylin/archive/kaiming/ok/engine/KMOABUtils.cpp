/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABUtils.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <cstring>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <limits.h> // for PATH_MAX
#include <uuid/uuid.h>
#include <array>

#include "common/KMLogger.h"
#include "common/KMStringUtils.h"

namespace fs = std::filesystem;

#define IGNORE_RETURN(func_call) \
    do { \
        /* 使用编译器特定属性标记变量为未使用 */ \
        __attribute__((unused)) auto unused_result = (func_call); \
    } while(0)

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @param : [in] ignoreExitStatus，是否忽略外部程序退出状态；true-忽略; false-不忽略
 * @return: true - 成功执行；false - 出现错误
*/
bool KMOABUtils::spawn(const std::string &exec, const std::vector<std::string> &args, bool ignoreExitStatus)
{
    std::string cmdline(exec);
    std::vector<const char *> execvpArgs;
    execvpArgs.push_back(exec.c_str());
    for (auto &arg : args)
    {
        execvpArgs.push_back(arg.c_str());
        cmdline += " " + arg;
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
            IGNORE_RETURN(::write(STDOUT_FILENO, buffer, count));
            ::memset(buffer, 0, 1025);
        }
        ::close(pipefd[0]); // 关闭读端

        int status = 0;
        pid_t result = ::waitpid(pid, &status, 0);
        if (result == -1)
        {
            KMError(::strerror(errno));
            return false;
        }

        if (!ignoreExitStatus)
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

/**
 * @brief : 执行外部程序
 * @param : [in] exec, 外部程序名称，不包含路径
 * @param : [in] args, 执行外部程序需要传入的参数列表
 * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
 * @return: true - 成功执行；false - 出现错误
 */
bool KMOABUtils::spawn(const std::string &exec, const std::vector<std::string> &args, std::string &result)
{
    std::string cmdline(exec);
    std::vector<const char *> execvpArgs;
    execvpArgs.push_back(exec.c_str());
    for (auto &arg : args)
    {
        execvpArgs.push_back(arg.c_str());
        cmdline += " " + arg;
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

    return true;
}

/**
 * @brief : 递归创建目录
 * @param : [in] path, 目录
 * @param : [out] std::error_code，保存错误信息
 * @return : true-存在；false-不存在
 */
bool KMOABUtils::mkpath(const std::string &path, std::error_code &ec)
{
    bool result = fs::create_directories(path, ec);
    if (!result && 0 == ec.value())
    {
        return true;
    }
    return result;
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @return : true-存在；false-不存在
 */
bool KMOABUtils::mkpath(const std::string &path)
{
    std::error_code ec;
    return KMOABUtils::mkpath(path, ec);
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @param : [in] mode，路径权限，可八进制写法，如：0755
 * @return : true-存在；false-不存在
 */
bool KMOABUtils::mkpath(const std::string &path, unsigned int mode)
{
    std::error_code ec;
    if (KMOABUtils::mkpath(path, ec))
    {
        fs::permissions(path, (fs::perms)mode);
        return true;
    }

    return false;
}

/**
 * canonical:
 * @brief : 规范化路径，绝对路径并路径中不包含.和..
 * @param : [in] path，路径
 * @return: true-空目录或不存在; false-不是空目录或不是目录
 */
std::string KMOABUtils::canonical(const std::string &path)
{
    return fs::canonical(path);
}

/**
 * isEmptyDirectory:
 * @brief : 判断路径是否空目录，使用前先判断是否是目录
 * @param : [in] path，路径
 * @return: true-空目录或不存在; false-不是空目录或不是目录
 */
bool KMOABUtils::isEmptyDirectory(const std::string &path)
{
    fs::path p(path);
    if (!fs::exists(p))
    {
        return true;
    }

    if (!fs::is_directory(p))
    {
        return false;
    }

    return fs::is_empty(p);
}

/**
 * @brief : 强制递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @return : true-成功；false-失败
 */
bool KMOABUtils::rmRfAll(const std::string &path)
{
    std::vector<std::string> args;
    args.push_back("-rf");
    args.push_back(path);

    return KMOABUtils::spawn("rm", args);
}

/**
 * @brief : 目录或文件拷贝
 * @param : [in] src, 源目录或文件
 * @param : [in] dest, 目标目录或文件
 * @return : true-成功；false-失败
 */
bool KMOABUtils::copyAll(const std::string &src, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-R");
    args.push_back(src);
    args.push_back(dest);

    return KMOABUtils::spawn("cp", args);
}

/**
 * @brief : 移动文件或重命名文件
 * @param : [in] src, 源目录或文件
 * @param : [in] dest, 目标目录或文件
 * @return : true-成功；false-失败
 */
bool KMOABUtils::mv(const std::string &src, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-f");
    args.push_back(src);
    args.push_back(dest);

    return KMOABUtils::spawn("mv", args);
}

/**
 * @brief : 获取文件sha256校验和
 * @param : [in] file，文件路径
 * @return: 文件内容校验和
  */ 
std::string KMOABUtils::getFileSha256sum(const std::string &file)
{
    std::string result;
    if (!fs::exists(file))
    {
        return result;
    }
    
    std::vector<std::string> args;
    args.push_back("-b");
    args.push_back(file);

    if (KMOABUtils::spawn("sha256sum", args, result))
    {
        std::vector<std::string> splits = KMStringUtils::splitString(result, " ");
        if (!splits.empty())
        {
            result = splits.at(0);
        }
        return result;
    }
    
    KMError(result);
    return "";
}

/**
 * @brief : 查找可执行文件路径
 * @param : [in] exe，可执行文件名
 * @return: 可执行文件绝对路径
 * @note  : 只探索下面目录，如果需要请自行添加
 *          /usr/local/sbin
 *          /usr/local/bin
 *          /usr/bin
 *          /usr/sbin
 *          /bin
 *          /sbin
 */ 
std::string KMOABUtils::searchExecutable(const std::string &exe)
{
    char * path = std::getenv("PATH");
    std::vector<std::string> binDirs;
    if (path && path[0] != '\0')
    {
        binDirs = KMStringUtils::splitString(path, ":");
    }
    else
    {
        std::vector<std::string> binDirsDefault = {
            "/usr/local/sbin",
            "/usr/local/bin",
            "/usr/sbin",
            "/usr/bin",
            "/sbin",
            "/bin",
            "/opt/system/bin"
        };

        binDirs = binDirsDefault;
    }

    std::string result;

    for (const std::string& binDir : binDirs)
    {
        std::string execPath = KMStringUtils::buildFilename(binDir, exe);
        fs::path p(execPath);
        if (!fs::exists(p))
        {
            continue;
        }

        if (!fs::is_regular_file(p))
        {
            continue;
        }

        return execPath;
    }

    return result;
}

std::string KMOABUtils::generateUUID()
{
    uuid_t uuidtime;
    // 生成基于时间的UUID
    uuid_generate_time(uuidtime);

    // 将UUID转换为字符串格式
    std::array<char, 64> uuidarray;
    uuid_unparse(uuidtime, uuidarray.data());
    return std::string(uuidarray.data());
}
