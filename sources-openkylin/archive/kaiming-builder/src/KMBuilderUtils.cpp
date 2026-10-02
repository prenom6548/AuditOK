/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderUtils.h"

#include <iostream>
#include <regex>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fnmatch.h>

#include "common/KMLogger.h"
#include "common/KMProcessUtils.h"

using namespace std;

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
std::string KMBuilderUtils::searchExecutable(const std::string &exe)
{
    char * path = std::getenv("PATH");
    std::vector<std::string> binDirs;
    if (path && path[0] != '\0')
    {
        binDirs = KMBuilderUtils::splitString(path, ":");
    }
    else
    {
        std::vector<std::string> binDirsDefault = {
            "/usr/local/sbin",
            "/usr/local/bin",
            "/usr/sbin",
            "/usr/bin",
            "/sbin",
            "/bin"
        };

        binDirs = binDirsDefault;
    }

    std::string result;

    for (const std::string& binDir : binDirs)
    {
        std::string execPath = KMBuilderUtils::buildFilename(binDir, exe);
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

static std::string packageArch(const std::string& platformArch)
{
    std::string arch = platformArch;

    if (platformArch == "loongarch64")
    {
        std::string exe = KMBuilderUtils::searchExecutable("dpkg-architecture");
        if (!exe.empty())
        {
            std::string result;
            std::vector<std::string> args;
            args.push_back("-q");
            args.push_back("DEB_HOST_ARCH");
            if (KMProcessUtils::spawn(exe, args, result))
            {
                KMBuilderUtils::replace(result, "\n", "");
                if (!result.empty())
                {
                    arch = result;
                }
            }
        }
    }
    else if (platformArch == "sw_64")
    {
        arch = "sw64";
    }

    return arch;
}

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__aarch64__) && !defined(__arm__)
static string get_kernel_arch()
{
    static struct utsname buf;
    static char *arch = nullptr;

    if (arch != nullptr)
        return arch;

    if (uname(&buf))
    {
        arch = "unknown";
        return arch;
    }

    arch = buf.machine;

    /* Override for some arches */
    string mstr = buf.machine;
    if (mstr.length() == 4 && mstr.at(0) == 'i' && mstr.at(2) == '8' && mstr.at(3) == '6')
    {
        arch = "i386";
    }
    else if (mstr == "mips")
    {
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mipsel";
#  endif
    }
    else if (mstr == "mips64")
    {
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mips64el";
#  endif
    }

    return arch;
}
#endif

KMBuilderUtils::KMBuilderUtils() { }

KMBuilderUtils::~KMBuilderUtils() { }

list<string> KMBuilderUtils::split(string str, char c)
{
    list<string> retList;

    int indexStart = 0;
    int indexEnd = -1;
    do
    {
        indexEnd = str.find(c, indexStart);

        if (indexEnd < 0)
        {
            retList.push_back(str.substr(indexStart, str.length() - indexStart));
            break;
        }

        // 针对  "20.03;askkd;qwecc;"
        if (indexEnd == str.length() - 1)
        {
            retList.push_back(str.substr(indexStart, str.length() - indexStart - 1));
            break;
        }

        retList.push_back(str.substr(indexStart, indexEnd - indexStart));
        indexStart = indexEnd + 1;
    } while (true);

    return retList;
}

const string KMBuilderUtils::getArch()
{
#if defined(__i386__)
    return "i386";
#elif defined(__x86_64__)
    return "x86_64";
#elif defined(__aarch64__)
    return "arm64";
#elif defined(__arm__)
#  if G_BYTE_ORDER == G_LITTLE_ENDIAN
    return "arm";
#  else
    return "armeb";
#  endif
#else
    return packageArch(get_kernel_arch());
#endif
}

bool KMBuilderUtils::ensureDir(fs::path dir)
{
    if (fs::exists(dir))
        return true;

    return KMBuilderUtils::mkpath(dir.string());
}

const string KMBuilderUtils::getCommandResoult(const string &exe, const list<string> &args)
{
    string result;

    string cmd = exe;

    for (auto arg : args)
        cmd.append(" ").append(arg);

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return string();

    size_t ret = 0;
    char buf[1025] = { 0 };
    while ((ret = fread(buf, sizeof(char), 1024, pipe)) == 1024)
    {
        result.append(buf);
    }
    if (feof(pipe) != 0)
        return string();

    result.append(buf);

    return result;
}

/**
 * @brief : 根据分隔符拆分字符串
 * @param : [in] str，源字符串
 * @param : [in] split，分隔符
 * @return: std::vector<std::string>，拆分后的字符串数组
 */
std::vector<std::string> KMBuilderUtils::splitString(const std::string &str, const std::string &split)
{
    std::vector<std::string> values;
    if (str.empty())
        return values;

    int index_start = 0;
    int index_end = -1;
    do
    {
        index_end = str.find(split, index_start);

        if (index_end == std::string::npos)
        {
            values.push_back(str.substr(index_start, str.length() - index_start));
            break;
        }
        else
        {
            values.push_back(str.substr(index_start, index_end - index_start));
        }

        index_start = index_end + split.length();
    } while (index_start < str.length());

    return values;
}

/**
 * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
 * @param : [in] vector，待合并的字符串数组
 * @param : [in] split，分隔符
 * @return: 合并后的长字符串
 */
std::string KMBuilderUtils::join(const std::vector<std::string>& vector, const std::string &split)
{
    std::string value;
    for (auto &e : vector)
    {
        if (!value.empty())
        {
            value += split;
        }
        value += e;
    }

    return value;
}

bool KMBuilderUtils::argumentNeedsQuoting(const char *arg)
{
    if (*arg == '\0')
        return true;

    while (*arg != 0)
    {
        char c = *arg;
        if (!std::isalnum(c) && !(c == '-' || c == '/' || c == '~' || c == ':' || c == '.' || c == '_' || c == '=' || c == '@'))
            return true;
        arg++;
    }
    return false;
}

/**
 * @brief : 将字符串str中连续的ch字符缩减成1个
 * @param : [in], str, 待处理的字符串
 * @param : [in], ch，待瘦身的字符
 * @return: 处理完后的字符串
 */
std::string KMBuilderUtils::removeDuplicateChars(const std::string& str, char ch)
{
    std::string result;
    bool start = false;
    for (int i = 0; i < str.length(); i++)
    {
        if (str.at(i) == ch && start)
        {
            continue;
        }
        else
        {
            result += str.at(i);
        }

        if (str.at(i) == ch)
        {
            start = true;
        }
        else
        {
            start = false;
        }
    }

    return result;
}

/**
 * isEmptyDirectory:
 * @brief : 判断路径是否空目录，使用前先判断是否是目录
 * @param : [in] path，路径
 * @return: true-空目录或不存在; false-不是空目录或不是目录
 */
bool KMBuilderUtils::isEmptyDirectory(const std::string &path)
{
    fs::path p(path);
    if (!fs::exists(p))
        return true;

    if (!fs::is_directory(p))
        return false;

    bool ret = true;
    for (auto const &dirEntry : fs::directory_iterator{ p })
    {
        std::string filename = dirEntry.path().filename();
        if (filename != "." && filename != "..")
        {
            ret = false;
            break;
        }
    }

    return ret;
}

/**
 * @brief : 获取目录下所有的文件列表（包含子目录下的文件）
 * @param : [in] dir，路径
 * @return: 文件列表
 */ 
std::vector<std::string> KMBuilderUtils::getDirFiles(const fs::path &dir)
{
    std::vector<std::string> files;

    if (!fs::exists(dir))
    {
        return files;
    }

    if (!fs::is_directory(dir))
    {
        files.push_back(dir.string());
        return files;
    }

    // 不具备访问权限的目录略过
    if (::access(dir.string().c_str(), R_OK) != 0)
    {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(dir)) 
    {
        std::string filename = entry.path().filename();
        if (filename == "." || filename == "..")
        {
            continue;
        }

        if (::access(entry.path().string().c_str(), R_OK) != 0)
        {
            continue;
        }

        if (fs::is_regular_file(entry.status())) 
        {
            files.push_back(entry.path().string());
        }
        else if (fs::is_symlink(entry.path()))
        {
            // do nothing
        }
        else if (fs::is_directory(entry.path()))
        {
            std::vector<std::string> subs = KMBuilderUtils::getDirFiles(entry.path());
            for (auto &item : subs)
            {
                files.push_back(item);
            }
        }
    }

    return files;
}

/**
 * removeEmptyDirectory:
 * @brief : 递归删除空子目录
 * @param : [in] path，路径
 * @return: true-操作成功; false-操作失败
 */
bool KMBuilderUtils::removeEmptyDirectory(const std::string &path, std::error_code &ec)
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
    
    for (auto const &dirEntry : fs::directory_iterator{ p })
    {
        fs::path subdir = dirEntry.path();
        std::string filename = subdir.filename().string();

        if (filename == "." || filename == "..")
        {
            continue;
        }

        if (!fs::is_directory(subdir))
        {
            continue;
        }

        if (KMBuilderUtils::isEmptyDirectory(subdir.string()))
        {
            bool result = fs::remove(subdir, ec);
            if (!result)
            {
                return false;
            }
        }
        else
        {
            bool result = KMBuilderUtils::removeEmptyDirectory(subdir.string(), ec);
            if (!result)
            {
                return false;
            }

            if (KMBuilderUtils::isEmptyDirectory(subdir.string()))
            {
                result = fs::remove(subdir, ec);
                if (!result)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * @brief : 递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @param : [out] std::error_code，保存错误信息
 * @return : true-成功；false-失败
 */
bool KMBuilderUtils::removeAll(const std::string &path, std::error_code &ec)
{
    // 移除文件或目录的只读属性
    fs::permissions(path, fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec | fs::perms::others_read | fs::perms::others_exec, ec);

    bool result = fs::remove_all(path, ec);
    if (!result && 0 == ec.value())
        return true;
    return result;
}

/**
 * @brief : 递归删除目录或文件
 * @param : [in] path, 待目录或文件
 * @return : true-成功；false-失败
 */
bool KMBuilderUtils::removeAll(const std::string &path)
{
    std::error_code ec;
    bool result = KMBuilderUtils::removeAll(path, ec);
    if (!result)
    {
        KMWarn(ec.message());
    }

    return result;
}

bool KMBuilderUtils::rmRfAll(const std::string &path)
{
    std::vector<std::string> args;
    args.push_back("-rf");
    args.push_back(path);

    std::string result;
    if (!KMProcessUtils::spawn("rm", args, result))
    {
        KMWarn(result);
        return false;
    }
    
    return true;
}

/**
 * @brief : 判断字符串前缀
 * @param : [in] src，原始字符串
 * @param : [in] prefix，字符串前缀
 * @return : true-字符串src是以prefix为前缀；false-字符串src不是以prefix为前缀
 */
bool KMBuilderUtils::startsWith(const std::string &src, const std::string &prefix)
{
    return src.find(prefix) == 0;
}

/**
 * @brief : 判断字符串后缀
 * @param : [in] src，原始字符串
 * @param : [in] sufix，字符串后缀
 * @return : true-字符串src是以sufix为后缀；false-字符串src不是以sufix为后缀
 */
bool KMBuilderUtils::endsWith(const std::string &src, const std::string &sufix)
{
    size_t pos = src.rfind(sufix);
    if (pos == std::string::npos)
        return false;

    size_t sufixLen = sufix.length();
    size_t srcLen = src.length();

    return pos + sufix.length() == src.length();
}

/**
 * @brief : 组装文件名
 * @param : [in] path, 前缀路径
 * @param : [in] name, 后续文件或目录名
 * @return: 组装后的文件路径
 */
std::string KMBuilderUtils::buildFilename(const std::string &path, const std::string &name)
{
    if (path.empty())
        return name;

    if (name.empty())
        return path;

    std::string filename = path + "/" + name;
    KMBuilderUtils::replace(filename, "//", "/");
    KMBuilderUtils::replace(filename, "/./", "/");

    return filename;
}

std::string KMBuilderUtils::buildFilename(const std::string &path, const std::string &name1, const std::string &name2)
{
    std::string filename = path + "/" + name1 + "/" + name2;
    KMBuilderUtils::replace(filename, "//", "/");

    return filename;
}

std::string KMBuilderUtils::buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3)
{
    std::string filename = path + "/" + name1 + "/" + name2 + "/" + name3;
    KMBuilderUtils::replace(filename, "//", "/");

    return filename;
}

/**
 * @brief : 将字符串str中的子串from替换为to
 * @param : [in | out] str, 原字符串
 * @param : [in] from, 需要替换的旧字符子串
 * @param : [in] to, 替换后的新子串
 * @return: 替换后的字符串引用，同str
 */
std::string &KMBuilderUtils::replace(std::string &str, const std::string &from, const std::string &to)
{
    // 替换所有出现的from为to
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // 防止无限循环
    }

    return str;
}


/**
 * @brief : 递归创建目录
 * @param : [in] path, 目录
 * @param : [out] std::error_code，保存错误信息
 * @return : true-存在；false-不存在
 */
bool KMBuilderUtils::mkpath(const std::string &path, std::error_code &ec)
{
    bool result = fs::create_directories(path, ec);
    if (!result && 0 == ec.value())
        return true;
    return result;
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @return : true-存在；false-不存在
 */
bool KMBuilderUtils::mkpath(const std::string &path)
{
    std::error_code ec;
    return KMBuilderUtils::mkpath(path, ec);
}

/**
 * @brief : 递归创建目录，如果目录已经存在则不报告错误
 * @param : [in] path, 目录
 * @param : [in] mode，路径权限，可八进制写法，如：0755
 * @return : true-存在；false-不存在
 */
bool KMBuilderUtils::mkpath(const std::string &path, unsigned int mode)
{
    std::error_code ec;
    if (KMBuilderUtils::mkpath(path, ec))
    {
        fs::permissions(path, (fs::perms)mode);
        return true;
    }

    return false;
}

/**
 * @brief : 在字符串数组v中查找是否包含字符串e
 * @param : [in] v, 字符串数组
 * @param : [in] e, 字符串
 * @return: true-包含；false-不包含
 */
bool KMBuilderUtils::contains(const std::vector<string> &v, const std::string &e)
{
    for (auto &element : v)
    {
        if (e == element)
        {
            return true;
        }
    }

    return false;
}

bool KMBuilderUtils::contains(const std::string &src, const std::string &sub)
{
    return std::string::npos != src.find(sub);
}

/**
 * @brief : 下载url数据到文件dest
 * @param : [in] url，支持HTTP、HTTPS、FTP、FTPS协议
 * @param : [in] 目标文件名，默认则为url的basename
 * @return: true-下载成功；false-下载失败
 * @note  : 使用wget命令实现，优势：可利用其断点续传和代理等
 */ 
bool KMBuilderUtils::downloadUrl(const std::string &url, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-c");
    if (!dest.empty())
    {
        args.push_back("-O");
        args.push_back(dest);

        std::string dir = fs::path(dest).parent_path().string();
        KMBuilderUtils::ensureDir(dir);
    }
    args.push_back("-T");
    args.push_back("3");
    args.push_back("--no-check-certificate");
    args.push_back(url);

    bool ret = KMProcessUtils::spawn("wget", args);

    return ret;
}

bool KMBuilderUtils::downloadUrlNoPrint(const std::string &url, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-c");
    args.push_back("-q");
    if (!dest.empty())
    {
        args.push_back("-O");
        args.push_back(dest);

        std::string dir = fs::path(dest).parent_path().string();
        KMBuilderUtils::ensureDir(dir);
    }
    args.push_back("-T");
    args.push_back("3");
    args.push_back("--no-check-certificate");
    args.push_back(url);

    bool ret = KMProcessUtils::spawn("wget", args);

    return ret;
}

/**
 * @brief : 执行外部程序
 * @param : [in] workingDirectory, 工作目录
 * @param : [in] args, 执行外部程序需要传入的参数列表，包括外部程序本身
 * @param : [in] ignoreExitCode，是否忽略退出码
 * @return: true - 成功执行；false - 出现错误
*/
bool KMBuilderUtils::spawn(const std::string &workingDirectory, const std::vector<std::string> &args, bool ignoreExitCode)
{
    if (args.empty())
    {
        return false;
    }

    std::string exec = args.at(0);

    std::string cmdline;
    std::vector<const char *> execvpArgs;
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
    KMInfo(cmdline);

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

string KMBuilderUtils::argSplice(string prefix, string content)
{
    std::ostringstream streamBuffer;
    streamBuffer << prefix << content;
    string strBuffer = streamBuffer.str();
    return strBuffer;
}

string KMBuilderUtils::argSplice(string prefix, string content, string suffix)
{
    std::ostringstream streamBuffer;
    streamBuffer << prefix << content << suffix;
    string strBuffer = streamBuffer.str();
    return strBuffer;
}

string KMBuilderUtils::argSplice(string prefix, int content)
{
    std::ostringstream streamBuffer;
    streamBuffer << prefix << content;
    string strBuffer = streamBuffer.str();
    return strBuffer;
}

string KMBuilderUtils::getChildPath(const string &parentPath, const string &childName)
{
    // 如果 parentPath 不以斜杠结尾，则添加一个斜杠
    string path = parentPath;
    if (!path.empty() && path.back() != '/' && path.back() != '\\')
    {
        path += '/';
    }
    // 将子项名称添加到路径末尾
    path += childName;
    return path;
}

bool KMBuilderUtils::pathExists(const std::string &path)
{
    return (access(path.c_str(), F_OK) != -1);
}

string KMBuilderUtils::replaceIdPrefix(const string &origId)
{
    string id = origId;
    // 使用 std::replace_if 和 lambda 表达式替换 '-' 为 '_' 直到遇到 '/'
    auto it = find(id.begin(), id.end(), '/');
    if (it != id.end())
    {
        replace_if(
          id.begin(),
          it,
          [](char c)
          {
              return c == '-';
          },
          '_');
    }
    else
    {
        replace_if(
          id.begin(),
          id.end(),
          [](char c)
          {
              return c == '-';
          },
          '_');
    }
    return id;
}

bool KMBuilderUtils::command(string appDir, vector<string> envVars, vector<string> extraArgs, string commandLine)
{
    vector<string> args;

    args.push_back("kaiming");
    args.push_back("build");
    args.push_back("--die-with-parent");
    args.push_back("--nofilesystem=host:reset");

    if (extraArgs.size())
    {
        for (auto extraArg : extraArgs)
        {
            args.push_back(extraArg);
        }
    }

    if (envVars.size())
    {
        for (auto envVar : envVars)
        {
            args.push_back(KMBuilderUtils::argSplice("--env=", envVar));
        }
    }

    args.push_back(appDir);
    args.push_back("/bin/bash");
    args.push_back("--");
    args.push_back("-c");
    args.push_back(commandLine);

    return KMBuilderUtils::spawn("", args);
}

/**
 * @brief : 创建目录软连接
 * @param : target 目标目录
 * @param : link 连接符号
 * @param : ec 错误信息
 * @return: true-创建成功；false-创建失败
 */
bool KMBuilderUtils::createDirectorySymlink(const std::string &target, const std::string &link, std::error_code &ec)
{
    fs::create_directory_symlink(target, link, ec);
    if (0 == ec.value())
        return true;
    else
        return false;
}

/**
 * @brief : 获取目录下所有文件
 * @param : dir 查找的目标目录
 * @param : files 所有的文件
 */
void KMBuilderUtils::getFilesInDir(const std::string &dir, vector<string> &files)
{
    for (auto const &dirEntry : fs::directory_iterator{ dir })
    {
        fs::path p = dirEntry.path();
        if (fs::is_symlink(p))
            continue;
        else if (fs::is_regular_file(p))
            files.emplace_back(p.string());
        else if (fs::is_directory(p))
            getFilesInDir(p.string(), files);
    }
}

bool KMBuilderUtils::breakHardlink(const std::filesystem::path &filePath)
{
    // 检查文件是否存在
    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "File does not exist: %s." << filePath.string() << std::endl;
        return false;
    }

    // 获取文件父目录
    const auto parentPath = filePath.parent_path();

    // 创建临时文件名
    std::filesystem::path tmpPath = parentPath / ".breaklinkXXXXXX";

    try
    {
        // 复制文件
        std::filesystem::copy(filePath, tmpPath, std::filesystem::copy_options::overwrite_existing);

        // 重命名临时文件以替换原始文件
        std::filesystem::rename(tmpPath, filePath);

        // 成功
        return true;
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        // 处理错误
        std::cerr << "Error breaking hardlink: " << e.what() << std::endl;
        return false;
    }
}

std::string KMBuilderUtils::pathMatchPrefix(const std::string &pattern, const std::string &string)
{
    std::string::size_type patternPos = 0;
    std::string::size_type stringPos = 0;

    // cout << "pattern:" << pattern << "----------" << "string:" << string <<  endl;

    // 去除开头的斜杠
    while (patternPos < pattern.size() && pattern[patternPos] == '/')
        ++patternPos;
    while (stringPos < string.size() && string[stringPos] == '/')
        ++stringPos;

    while (patternPos < pattern.size())
    {
        switch (pattern[patternPos++])
        {
        case '?':
            if (stringPos >= string.size() || string[stringPos] == '/')
                return "";
            ++stringPos;
            break;

        case '*':
        {
            char nextPatternChar = (patternPos < pattern.size()) ? pattern[patternPos] : '\0';

            if (nextPatternChar == '/')
            {
                stringPos = string.find('/', stringPos);
                if (stringPos == std::string::npos)
                    return "";
                break;
            }
            else if (nextPatternChar == '\0')
            {
                stringPos = string.find('/', stringPos);
                return (stringPos != std::string::npos) ? string.substr(0, stringPos) : string;
            }

            while (stringPos < string.size())
            {
                std::string remainingPattern = pattern.substr(patternPos); // 减去之前已经处理的'*'
                std::string remainingString = string.substr(stringPos);
                std::string prefix = KMBuilderUtils::pathMatchPrefix(remainingPattern, remainingString);
                if (!prefix.empty())
                {
                    return string.substr(0, stringPos + prefix.size());
                }
                if (string[stringPos] == '/')
                    break;
                ++stringPos;
            }
            return "";
        }

        default:
            if (stringPos >= string.size() || string[stringPos] != pattern[patternPos - 1])
            {
                return "";
            }

            ++stringPos;

            break;
        }
    }

    // 检查是否匹配到了字符串的结尾或斜杠
    if (stringPos == string.size() || string[stringPos] == '/')
    {
        return string.substr(0, stringPos);
    }

    // 如果没有匹配到整个模式，返回空字符串
    return "";
}

bool KMBuilderUtils::matchesPathPattern(string path, string pattern)
{
    if (pattern[0] != '/')
    {
        path = KMBuilderUtils::getBasename(path);
    }

    string prefix = KMBuilderUtils::pathMatchPrefix(pattern, path);
    if (!prefix.empty())
    {
        return true;
    }
    return false;
}

string KMBuilderUtils::getBasename(string path)
{
    if (path.empty())
        return ".";

    // 去除尾部的 '/' 和 '\'（如果有的话）
    size_t lastSlashIdx = path.find_last_of("/");
    if (lastSlashIdx == std::string::npos || lastSlashIdx == path.size() - 1)
    {
        // 如果路径只包含一个目录（如 "/" 或 "C:\"），或者没有目录（如 "file.txt"），则返回当前目录 "."
        return ".";
    }

    // 返回目录名
    return path.substr(lastSlashIdx + 1, path.size() - 1);
}

string KMBuilderUtils::getDirname(const string &path)
{
    if (path.empty())
        return ".";

    // 去除尾部的 '/' 和 '\'（如果有的话）
    size_t lastSlashIdx = path.find_last_of("/");
    if (lastSlashIdx == std::string::npos || lastSlashIdx == path.size() - 1)
    {
        // 如果路径只包含一个目录（如 "/" 或 "C:\"），或者没有目录（如 "file.txt"），则返回当前目录 "."
        return ".";
    }

    // 返回目录名
    return path.substr(0, lastSlashIdx);
}


/**
 * pathMatchPrefix:
 * @brief : 判断路径string是否匹配pattern前缀，返回匹配结束后的部分字符串
 * @param : [in] pattern, 前缀模式
 * @param : [in] string, 路径
 * @return: 返回匹配结束后的部分字符串，不匹配返回null；
 */
const char *KMBuilderUtils::pathMatchPrefix(const char *pattern, const char *string)
{
    char c, test;
    const char *tmp;

    while (*pattern == '/')
    {
        pattern++;
    }

    while (*string == '/')
    {
        string++;
    }

    while (true)
    {
        switch (c = *pattern++)
        {
        case 0:
            if (*string == '/' || *string == 0)
            {
                return string;
            }
            return nullptr;

        case '?':
            if (*string == '/' || *string == 0)
            {
                return nullptr;
            }
            string++;
            break;

        case '*':
            c = *pattern;

            while (c == '*')
            {
                c = *++pattern;
            }

            /* special case * at end */
            if (c == 0)
            {
                tmp = strchr(string, '/');
                if (tmp != nullptr)
                {
                    return tmp;
                }
                return string + strlen(string);
            }
            else if (c == '/')
            {
                string = strchr(string, '/');
                if (string == nullptr)
                {
                    return nullptr;
                }
                break;
            }

            while ((test = *string) != 0)
            {
                tmp = KMBuilderUtils::pathMatchPrefix(pattern, string);
                if (tmp != nullptr)
                {
                    return tmp;
                }
                if (test == '/')
                {
                    break;
                }
                string++;
            }
            return nullptr;

        default:
            if (c != *string)
            {
                return nullptr;
            }
            string++;
            break;
        }
    }

    return nullptr; /* Should not be reached */
}

string KMBuilderUtils::getChildDir(const string &base, const string &child)
{
    string path = base + "/" + child;
    return path;
}

std::string KMBuilderUtils::fileGetContents(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // 将文件内容读入stringstream

    return buffer.str(); // 从stringstream获取字符串
}

bool KMBuilderUtils::fileSetContents(const string &filePath, const string &content)
{
    // std::ofstream file(filePath, std::ios::out | std::ios::trunc); // 清空文件并从头开始写入
    //  或者
    std::ofstream file(filePath, std::ios::app); // 在文件末尾追加数据

    // 检查文件是否成功打开
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return false;
    }

    // 写入内容到文件
    file << content;

    // 关闭文件
    file.close();

    std::cout << "内容已写入文件: " << filePath << std::endl;

    return true;
}

bool KMBuilderUtils::fileReSetContents(const string &filePath, const string &content)
{
    std::ofstream file(filePath, std::ios::out | std::ios::trunc); // 清空文件并从头开始写入

    // 检查文件是否成功打开
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return false;
    }

    // 写入内容到文件
    file << content;

    // 关闭文件
    file.close();

    std::cout << "内容已写入文件: " << filePath << std::endl;

    return true;
}

bool KMBuilderUtils::ifContains(const std::unordered_map<string, bool> &map, string key)
{
    for (const auto &pair : map)
    {
        if (pair.first == key)
        {
            return true;
        }
    }
    return false;
}

vector<string> KMBuilderUtils::enumerateDirectoryPath(const string &path, bool ignoreSymlink)
{
    vector<string> files;

    // 检查路径是否存在
    if (!fs::exists(path) || !fs::is_directory(path))
    {
        km_logger.info("Path does not exist or is not a directory :%s !", path.c_str());
        return files;
    }

    // 遍历目录中的文件和子目录
    for (const auto &entry : fs::directory_iterator(path))
    {
        // 只列举文件（不包括子目录）可以添加额外的检查：entry.is_regular_file()
        if (ignoreSymlink && fs::is_symlink(entry.path()))
            continue;
        files.push_back(entry.path().string());
    }

    return files;
}

vector<string> KMBuilderUtils::concatVectors(const vector<string> &vec1, const vector<string> &vec2)
{
    std::vector<std::string> result;
    result.reserve(vec1.size() + vec2.size()); // 预分配内存以提高效率

    // 将vec1的内容添加到result中
    result.insert(result.end(), vec1.begin(), vec1.end());

    // 将vec2的内容添加到result中
    result.insert(result.end(), vec2.begin(), vec2.end());

    return result;
}

std::string KMBuilderUtils::getSymlinkTarget(const std::string &path)
{
    fs::path symlinkPath(path);
    int maxLen = 1024;
    std::string targetPath;
    char buffer[maxLen]; // 足够大的缓冲区来存储路径
    ssize_t len = readlink(symlinkPath.c_str(), buffer, maxLen - 1);

    if (len != -1)
    {
        // readlink不包含最后的null字符，所以我们手动添加它
        buffer[len] = '\0';
        targetPath = buffer;
        // 可能需要解析路径中的任何相对组件
        targetPath = std::filesystem::absolute(targetPath);
        // targetPath = std::filesystem::absolute(targetPath, symlinkPath.parentPath());
    }
    else
    {
        // 处理错误（例如，如果路径不是符号链接）
        perror("readlink");
    }

    return targetPath;
}

bool KMBuilderUtils::isStringSymlink(const std::string &pathString)
{
    fs::path path(pathString);

    fs::file_status status = fs::symlink_status(path);
    return fs::is_symlink(status);
}

/**
 * @brief 从构建环境.kaiming-builder的绝对路径中获取modlue name 
 */
std::string KMBuilderUtils::getModuleName(const std::string path)
{
    std::vector<std::string> dirs = splitString(path , "/");
    std::vector<std::string> module_name_ = splitString(dirs[dirs.size()-1],"-");
    return module_name_[0];
}

/**
 * @brief 封装函数：将脚本内容写入 .sh 文件并设置可执行权限
 */
bool KMBuilderUtils::writeScriptToFile(const std::string& scriptContent, const std::string& filePath)
{
    // 将内容写入文件
    std::ofstream scriptFile(filePath);
    if (!scriptFile.is_open())
    {
        KMDebug("Failed to create script file: " + filePath);
        return false;
    }

    scriptFile << "#!/bin/bash\n";
    scriptFile << scriptContent << "\n";
    scriptFile.flush();
    scriptFile.close();

    // 修改文件权限为可执行
    if (chmod(filePath.c_str(), 0755) != 0)
    {
        KMDebug("Failed to make script executable: " + filePath);
        return false;
    }

    return true;
}

std::string KMBuilderUtils::readFile(const std::string &path, const std::string &prefix)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw KMException("Open " + path + " failed" );
    }
    std::string line;
    while(std::getline(file, line))
    {
        if (startsWith(line, prefix))
        {
            return line;
        }
    }
    return "";
}

KMRef KMBuilderUtils::comToRef(const std::string &id, const std::string &kind, const std::string &channel, const std::string &name, const std::string &version, const std::string &module)
{
    KMRef ref;
    ref.m_id = id;
    ref.m_kind = kind;
    ref.m_channel = channel;
    ref.m_name = name;
    ref.m_version = version;
    ref.m_module = module;
    return ref;
}

void KMBuilderUtils::concatVectors(std::vector<KMRef> &ref1, const std::vector<KMRef> &ref2)
{
    for (auto &item: ref2)
    {
        if (std::find(ref1.begin(),ref1.end(),item) == ref1.end())
        {
            ref1.push_back(item);
        }
    }
}

bool KMBuilderUtils::checkEnvironment(const std::string &base1, const std::string &base2)
{
    KMRef ref_base1, ref_base2;
    if (base1.empty() || base2.empty())
    {
        return true;
    }
    ref_base1.parseInfo(base1);
    ref_base2.parseInfo(base2);
    if (ref_base1.check(ref_base2))
    {
        return true;
    }
    return false;
}

bool KMBuilderUtils::isNumeric(const std::string &str)
{
    const std::string numbers = "0123456789";
    return str.find_first_not_of(numbers) == std::string::npos;
}

bool KMBuilderUtils::isValidPrerelease(const std::string &str)
{
    const std::string numbers = "0123456789";
    const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-";
    return str.find_first_not_of(numbers + allowedChars) == std::string::npos;
}

bool KMBuilderUtils::matchVersion(const std::string &version, const std::string &version_standard)
{
    KMVersion ver = KMVersion::parse(version);
    KMVersion ver_standard = KMVersion::parse(version_standard);
    if (ver >= ver_standard)
    {
        return true;
    }
    return false;
}

std::string KMBuilderUtils::getHomeDir()
{
    const char *home = std::getenv("HOME");
    if (home !=nullptr)
    {
        return std::string(home);
    }
    return "";
}

bool KMBuilderUtils::matchID(const std::string &text, const std::string &pattern)
{
    if (pattern.find('*') != std::string::npos) 
    {   
        // 使用 fnmatch 进行通配符匹配
        return fnmatch(pattern.c_str(), text.c_str(), FNM_NOESCAPE) == 0;
    } 
    else 
    {
        return text.find(pattern) != std::string::npos;
    }
};

/**
 * @brief : 校验appid是否合法
 * @param : [in] appName, appid
 * @param : [out] errMsg, 错误信息
 */ 
bool KMBuilderUtils::isValidAppName(const std::string &appName, std::string &errMsg)
{
    if (appName.length() > 255)
    {
        errMsg = "Id can't be longer than 255 characters";
        return false;
    }

    // std::regex regexStr("(\\w+\\.)+(\\w+\\.)+([\\w-]+)$");
    std::regex regexStr("([\\w-]+\\.)+([\\w-]+\\.)+([\\w-]+)$");
    std::smatch matchResult;
    if (std::regex_match(appName, matchResult, regexStr))
    {
        // std::regex regexDigit("^\\d.*");
        // std::smatch matchDigit;
        // for (auto element : matchResult)
        // {
        //     std::string str = element.str();
        //     if (std::regex_match(str, matchDigit, regexDigit))
        //     {
        //         errMsg = "Id is wrong! Elements may not begin with a digit.";
        //         return false;
        //     }
        // }
    }
    else
    {
        errMsg = "Id is wrong! Notes:\n"
            "    a) Ids are composed of 3 or more elements separated by a period('.') character.\n"
            "    b) Each element must only contain the ASCII characters '[A-Z][a-z][0-9]_-'.\n"
        //    "    c) All elements must contain at least one character.\n"
        //    "       Elements may not begin with a digit.\n"
        //    "       Additionally '-' is only allowed in the last element."
        ;
        return false;
    }

    return true;
}

std::vector<std::string> KMBuilderUtils::parseTxt(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw KMException("Open " + path + " failed" );
    }
    std::string line;
    std::vector<std::string> lines;
    while(std::getline(file, line))
    {
        lines.push_back(line);
    }
    return lines;
}

std::string KMBuilderUtils::matchPreStr(const std::vector<std::string> &vecs, const std::string &pre)
{
    for (auto &vec: vecs)
    {
        if (startsWith(vec, pre))
        {
            return vec;
        }
    }
    return "";
}

std::string KMBuilderUtils::trimLeftStr(const std::string &str)
{
    size_t index = 0;
    while (index < str.length() && (str[index] == ' ' || str[index] == '\t'))
    {
        index ++;
    }
    return str.substr(index);
}

std::string KMBuilderUtils::trimRightStr(const std::string &str)
{
    size_t index = str.length() - 1;
    while (index > 0 && (str[index] == ' ' || str[index] == ','))
    {
        index--;
    }
    return str.substr(0,index+1);
}

std::vector<std::string> KMBuilderUtils::splitStrBlank(const std::string &str)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start < str.length()) 
    {
        while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) 
        {
            start++;
        }
        if (start == str.length()) 
        {
            break;
        }

        size_t end = start;

        while (end < str.length() && str[end] != ' ' && str[end] != '\t')
        {
            end++;
        }
        // 提取子字符串
        tokens.push_back(str.substr(start, end - start));
        start = end;
    }
    return tokens;
}

std::string KMBuilderUtils::trimStr(const std::string &str)
{
    std::string str_left = trimLeftStr(str);
    return trimRightStr(str_left);
}

std::vector<std::string> KMBuilderUtils::keysFromMap(const std::map<std::string, std::string> &values)
{
    std::vector<std::string> keys;
    for (auto &item: values)
    {
        keys.push_back(item.first);
    }
    return keys;
}

void KMBuilderUtils::vectorToYaml(YAML::Node &yamlNode, std::string key, std::vector<std::string> values)
{
    YAML::Node node = YAML::Node(YAML::NodeType::Sequence);
    for (const auto &value: values)
    {
        node.push_back(value);
    }
    yamlNode[key] = node;
}

void KMBuilderUtils::mapToYaml(YAML::Node &yamlNode, std::map<std::string, std::string>values)
{
    for (auto &item: values)
    {
        yamlNode[item.first] = item.second; 
    }
}

bool KMBuilderUtils::cpAll(const std::string &src, const std::string &dest)
{
    std::vector<std::string> args;
    args.push_back("-rf");
    args.push_back(src);
    args.push_back(dest);
    if (!KMProcessUtils::spawn("cp", args))
    {
        return false;
    }
    return true;
}

bool KMBuilderUtils::copyFile(const std::string &src, const std::string &dest, const std::string &skip)
{
    if (!mkpath(dest))
    {
        return false;
    }
    for (auto &file: fs::directory_iterator(src))
    {
        if (file.path().filename() == skip)
        {
            continue;
        }
        if (!cpAll( file.path(), dest))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief : 使用rsync同步目录中的内容
 * @param : [in] src, 源目录
 * @param : [in] dest, 目标目录
 * @param : [in] excludes, 排除同步路径
 * @return : true-成功；false-失败
 */
bool KMBuilderUtils::rsyncDir(const std::string &src, const std::string &dest, const std::vector<std::string>& excludes)
{
    std::vector<std::string> args;
    args.push_back("-rlpt");
    args.push_back("--ignore-errors");
    for (auto& exclude : excludes)
    {
        args.push_back("--exclude=" + exclude);
    }
    args.push_back(src + "/");
    args.push_back(dest + "/");

    if (!KMProcessUtils::spawn("rsync", args, true))
    {
        return false;
    }

    return true;
}

std::string KMBuilderUtils::executeCommand(const std::string&command)
{
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "popen failed!";
    }
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    int returnCode = pclose(pipe);
    if (returnCode == -1) {
        // 处理 pclose 失败的情况
    }
    return result;
}
std::string KMBuilderUtils::getSysCore()
{
    struct utsname uts;
    if (uname(&uts) == -1) {
        return "";
    }

    std::string kernelName(uts.sysname);
    return kernelName;
}

std::string KMBuilderUtils::toLowerUtf8(const std::string& str) 
{
    wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    wstring wstr = converter.from_bytes(str);
    transform(wstr.begin(), wstr.end(), wstr.begin(), 
                   [](wchar_t c) { return std::towlower(c); });
    return converter.to_bytes(wstr);
}

bool KMBuilderUtils::isV11()
{
    std::string V11_conf = "/etc/.kylin-osinfo";
    if (fs::exists(V11_conf))
    {
        std::vector<std::string> lines = KMBuilderUtils::parseTxt(V11_conf);
        if (lines.empty())
        {
            throw KMException("open " + V11_conf + " error");
        }
        for (auto &line : lines)
        {
            if (KMBuilderUtils::startsWith(line, "MajorVersion"))
            {
                if (KMBuilderUtils::contains(line, "V11"))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

int KMBuilderUtils::getFileTimestamp(const std::string &filePath)
{
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) == 0)
    {
        return fileInfo.st_mtime;
    }
    else
    {
        return 0;
    }
}

bool KMBuilderUtils::isFileExpired(const std::string &filePath, int expiredTime)
{
    auto now = std::chrono::system_clock::now();
    int currentTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    int fileTimestamp = getFileTimestamp(filePath);

    bool isExpired = currentTime - fileTimestamp > expiredTime;

    return isExpired;
}

std::string KMBuilderUtils::extractPre(const std::string &str)
{
    const std::string numbers = "0123456789";
    const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t pos = str.find_first_not_of(numbers + allowedChars);
    if (pos == std::string::npos)
    {
        return str;
    }
    else if (pos != 0)
    {
        return str.substr(0, pos);
    }
    return "";
}

std::string KMBuilderUtils::extractNumPre(const std::string &str)
{
    const std::string numbers = "0123456789";
    size_t pos = str.find_first_not_of(numbers);
    if (pos == 0 || pos == std::string::npos)
    {
        return str;
    }
    return str.substr(0,pos);

}

std::string KMBuilderUtils::versionDebToKaiming(std::string version)
{
    // 剔除":"
    std::string version_changed = "";
    size_t pos = version.find_first_of(':');
    if (pos != std::string::npos)
    {
        version = version.substr(pos+1);
    }

    // ~ -> -
    size_t pos_wavy = version.find_first_of('~');
    if (pos_wavy != std::string::npos)
    {
        version[pos_wavy] = '-';
    }

    std::string upstream, revision, wavy;
    // - 划分upstream和revision
    size_t pos_hyphen = version.find_first_of('-');
    if (pos_hyphen != std::string::npos)
    {
        upstream = version.substr(0,pos_hyphen);
        revision = version.substr(pos_hyphen+1);
    }

    //处理upstream
    int numCount = 2;
    bool endIsNum = false;
    if (!upstream.empty())
    {
        std::vector<std::string> parts = KMBuilderUtils::splitString(upstream, ".");
        switch (parts.size())
        {
        case 1:  
            if (isNumeric(parts[0]))
            {
                version_changed = parts[0] + ".0";
                numCount = 2;
                endIsNum = true;
            }
            else
            {
                std::string pre = extractPre(parts[0]);
                if (isNumeric(pre))
                {
                    version_changed = "0.0." + parts[0];
                }
                else
                {
                    version_changed = "0.0.1-" + parts[0];
                }
            }
            break;
        case 2:
            parts[0] = extractNumPre(parts[0]);
            if (isNumeric(parts[1]))
            {
                version_changed = parts[0] + "."  + parts[1];
                numCount = 2;
                endIsNum = true;
            }
            else
            {
                std::string pre = extractPre(parts[1]);
                if (isNumeric(pre))
                {
                    version_changed =  parts[0] + ".0." + parts[1] ;
                }
                else
                {
                    version_changed = parts[0] + ".0.0-" + parts[1];
                }
            }
            break;
        case 3:
            parts[0] = extractNumPre(parts[0]);
            parts[1] = extractNumPre(parts[1]);
            if (isNumeric(parts[2]))
            {
                version_changed = parts[0] + "." + parts[1] + "." + parts[2];
                endIsNum = true;
                numCount = 3;
            }
            else
            {
                std::string pre = extractPre(parts[2]);
                if (isNumeric(pre))
                {
                    version_changed = parts[0] + "." + parts[1] + "." + parts[2];
                }
                else
                {
                    version_changed = parts[0] + "." + parts[1] + ".0-" + parts[2] ;
                }
            }
            break;
        case 4:
            parts[0] = extractNumPre(parts[0]);
            parts[1] = extractNumPre(parts[1]);
            parts[2] = extractNumPre(parts[2]);
            if (isNumeric(parts[3]))
            {
                version_changed = parts[0] + "." + parts[1] + "." + parts[2] + "." + parts[3];
            }
            else
            {
                std::string pre = extractPre(parts[3]);
                if (isNumeric(pre))
                {
                    version_changed = parts[0] + "." + parts[1] + "." + parts[2] + "." + parts[3];
                }
                else
                {
                    version_changed = parts[0] + "." + parts[1] + "." + parts[2] + "-" + parts[3];
                }
            }
            numCount = 4;
            break;

        default:
            int i = 0;
            for (i = 0 ; i < parts.size() - 1 ; i++)
            {
                parts[i] = extractNumPre(parts[i]);
                version_changed = version_changed + parts[i] + ".";
            }
            version_changed = version_changed + parts[i];
            numCount = 4;
            break;
        }
    }
    // 处理-
    if (!revision.empty())
    {
        if (!endIsNum)
        {
            version_changed = version_changed + "-" + revision;
        }
        else
        {
            size_t pos_hyphen2 = revision.find_first_of(1, '-');
            std::string pre_hyphen = revision;
            if (pos_hyphen2 != std::string::npos)
            {
                pre_hyphen = revision.substr(1, pos_hyphen2 - 1);
            }
            if (isNumeric(pre_hyphen))
            {
                version_changed = version_changed + "." + revision;
            }
            else if (numCount == 2)
            {
                version_changed = version_changed + ".0-" + revision; 
            }
            else
            {
                version_changed = version_changed + "-" + revision; 
            }
        }
    }

    // 删除元数据中+后面数据
    size_t pos_add = version_changed.find_first_of('+');
    if (pos_add != std::string::npos)
    {
        size_t pos_add2 = version_changed.find_first_of('+', pos_add + 1);
        if (pos_add2 != std::string::npos)
        {
            version_changed = version_changed.substr(0, pos_add2);
        }
    }
    
    return version_changed;
}
