#include "KMCommonUtils.h"

#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

#include "KMCrun.h"
#include "KMLogger.h"
#include "KMStringUtils.h"
#include "KMProcessUtils.h"
#include "KMJsonHelper.h"
#include "KMBrowserUtils.h"

namespace fs = std::filesystem;

/**
 * @brief 运行gtask命令
 * @param task GTask对象
 * @param command 要执行的命令
 * @param pwd 命令执行的工作目录
 * @param cb 回调函数
 * @param args 回调函数参数
 */
void KMCommonUtils::runGtaskCommand(GTask *task, const std::string &command, const std::string &pwd, callback cb, void *args)
{
    // 回调函数不能为空
    if (!cb)
        return;

    int pipeFD[2];
    if (pipe(pipeFD) == -1)
    {
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "create pipe failed");
        return;
    }

    auto pipeGuard = std::unique_ptr<int, void (*)(int *)>(pipeFD, [](int *fds) {
        close(fds[0]);
        close(fds[1]);
    });

    pid_t pid = fork();
    if (pid == -1)
    {
        g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_FAILED, "fork failed");
        return;
    }

    if (pid == 0)
    {
        close(pipeFD[0]);
        dup2(pipeFD[1], STDOUT_FILENO);
        dup2(pipeFD[1], STDERR_FILENO);
        close(pipeFD[1]);
        if (!pwd.empty())
        {
            if (chdir(pwd.c_str()) != 0)
            {
                KMInfo(std::string("chdir failed: ") + strerror(errno));
            }
        }

        std::string cmd;
        if (!prepareCommand(command, cmd))
        {
            exit(EXIT_FAILURE);
        }

        const char *sh = "/bin/bash";
        const char *args[] = { sh, "-c", cmd.c_str(), nullptr };
        execv(sh, const_cast<char *const *>(args));
        KMError("exec command failed !");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipeFD[1]);
        char buffer[256] = { 0 };
        ssize_t count = 0;
        std::string line;
        while ((count = read(pipeFD[0], buffer, sizeof(buffer))) > 0)
        {
            for (ssize_t i = 0; i < count; ++i)
            {
                char c = buffer[i];
                if (c == '\r')
                {
                    // 遇到回车，表示覆盖当前行
                    cb(args, line.c_str());
                    line.clear();
                }
                else if (c == '\n')
                {
                    cb(args, line.c_str());
                    line.clear();
                }
                else
                {
                    line += c;
                }
            }
        }

        if (!line.empty())
        {
            cb(args, line.c_str());
        }

        close(pipeFD[0]);

        // 等待子进程结束
        int status = 0;
        waitpid(pid, &status, 0);
        GString *output = g_string_new(nullptr);
        g_string_append_printf(output, "exit=%d", WEXITSTATUS(status));
        g_task_return_pointer(task, g_string_free(output, false), g_free);
    }
}

/**
 * @brief 准备xdg-open命令
 * @param path 要打开的路径
 * @param appId 应用ID
 * @param appVersion 应用版本
 * @param cmd 输出命令
 * @param true 成功 false 失败
 */
bool KMCommonUtils::prepareXdgOpenCmd(const std::string &path, const std::string &appId, const std::string &version, std::string &cmd)
{
    kmlogger.info("xdg-open path: %s", path.c_str());

    // 去除头尾的引号
    std::string strpath = path.substr(1, path.length() - 2);
    gchar *scheme = g_uri_parse_scheme(strpath.c_str());
    bool isNonFileUri = scheme != nullptr && g_ascii_strcasecmp(scheme, "file") != 0;
    g_free(scheme);

    // 非 file URI 交给系统默认处理，避免被误认为应用内相对路径。
    if (isNonFileUri || g_str_has_prefix(strpath.c_str(), "--") || strpath.empty())
    {
        cmd = "xdg-open " + path;
        return true;
    }

    std::string localPath;
    if (g_str_has_prefix(strpath.c_str(), "file://"))
    {
        auto pLocalPath = g_filename_from_uri(strpath.c_str(), nullptr, nullptr);
        if (pLocalPath == nullptr)
        {
            KMError("Invalid file:// URI");
            return false;
        }

        localPath = pLocalPath;
        if (localPath.size() > 1 && localPath[1] == '/')
        {
            localPath.erase(0, 1); // 去掉多余的斜杠
        }
    }
    else
    {
        localPath = strpath;
    }

    // 只有 file:// 或无前缀路径，且路径不存在时，尝试拼装完整路径
    if (!fs::exists(localPath))
    {
        std::string layerPath = "/opt/kaiming/layers/stable";
        std::string arch = getSingleSubdirectory(layerPath);
        if (!arch.empty())
        {
            // 拼接完整路径（注意加斜杠分隔）
            localPath = layerPath + "/" + arch + "/app/" + appId + "/binary/" + version + "/files" + localPath;
        }
    }

    // 用最终路径执行 xdg-open
    cmd = std::string("xdg-open \"") + localPath + "\"";

    return true;
}

/**
 * @brief 准备命令行
 * @param command 原始命令
 * @return true 成功 false 失败
 */
bool KMCommonUtils::prepareCommand(const std::string &command, std::string &cmd)
{
    cmd = command;
    std::vector<std::string> splits = KMStringUtils::splitString(command, " ");
    std::string execpath = splits.front();

    if (execpath.empty())
        return false;

    if (execpath == "xdg-open")
    {
        std::string appid = splits[1];
        std::string version = splits[2];
        std::string url = command.substr(command.find(version) + version.length() + 1);
        return prepareXdgOpenCmd(url, appid, version, cmd);
    }

    std::string appId = "";
    std::string browserExecPath = "";
    if (!execpath.empty() && !fs::exists(execpath))
    {
        auto all = KMBrowserUtils::loadExecutablesFromFile();
        for (auto &e : all)
        {
            if (execpath == e.execPath)
            {
                appId = e.appId;
                browserExecPath = e.execPath;
                break;
            }
        }
        // 找到匹配的浏览器
        if (!appId.empty() && !browserExecPath.empty())
        {
            cmd.append("/opt/kaiming-tools/bin/kaiming run --command=").append(browserExecPath).append(" ").append(appId).append(" -- ");
        }
        else
        {
            cmd = fs::path(execpath).filename().string();
        }

        for (int i = 1; i < splits.size(); ++i)
        {
            cmd.append(" ").append(splits[i]);
        }
    }

    return true;
}

/**
 * @brief 参数解析
 * @param str 输入字符串
 * @param args 输出参数列表
 * @return true 存在 false 不存在
 */
bool KMCommonUtils::parseArgs(const std::string &str, std::vector<std::string> &args)
{
    int argc = 0;
    char **argv = nullptr;
    GError *error = nullptr;
    if (!g_shell_parse_argv(str.c_str(), &argc, &argv, &error))
    {
        if (error)
        {
            g_error_free(error);
        }
        return false;
    }

    for (int i = 0; i < argc; ++i)
    {
        args.emplace_back(argv[i]);
    }

    return true;
}

/**
 * @brief 获取ps信息
 * @param psInfos 获取的ps信息结构
 */
void KMCommonUtils::getPsInfo(KMPSInfoList &psInfos, std::string targetAppid, bool isFind)
{
    std::string dir = "/run/crun";
    if (!fs::exists(dir))
        return;

    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        for (auto const &entry : fs::directory_iterator(dir))
        {
            std::string cid = entry.path().filename().string();
            if (cid.empty())
                continue;

            if (cid.size() != 36)
                continue; // 跳过非开明格式的目录

            // 解析status文件，获取pid,bundle
            std::error_code ec;
            std::string crunDir = entry.path().string();
            std::string statusDir = entry.path() / "status";
            if (!fs::exists(statusDir))
            {
                std::error_code ec;
                fs::remove_all(crunDir, ec);
                continue;
            }

            nlohmann::json j = KMJsonHelper::loadFile(statusDir);
            int64_t pid = j["pid"];
            std::string bundle = j["bundle"];

            // 只处理开明的容器
            if (bundle.find("kaiming") == std::string::npos)
            {
                continue;
            }

            // 用kill系统命令判断这个容器是否在运行中
            if (kill(pid, 0) == -1)
            {
                fs::remove_all(crunDir, ec);
                continue;
            }

            // 获取appid
            std::string appid = fs::path(bundle).parent_path().filename().string();
            if (appid.empty())
            {
                continue;
            }
            if (isFind && appid != targetAppid)
            {
                continue;
            }

            KMPSInfoItem item;
            item.cid = cid;
            item.pid = pid;
            item.path = bundle;
            item.appid = appid;
            psInfos.infolist.emplace_back(item);

            if (isFind && psInfos.infolist.size() >= 2)
            {
                break; // 同一个appid最多只应存在两个
            }
        }
    }
}

/**
 * @brief brief 获取当前运行的进程信息，并以 JSON 格式返回。
 * @param[out] ret 返回状态，0 表示成功，-1 表示失败。
 * @param[out] msg 如果成功，包含 JSON 字符串；失败时包含错误描述。
 * @return true 表示成功；false 表示失败。
 */
bool KMCommonUtils::ps(int &ret, std::string &msg)
{
    KMPSInfoList info;
    getPsInfo(info);
    try
    {
        ret = 0;
        nlohmann::json j = info;
        msg = j.dump();
    }
    catch (...)
    {
        ret = -1;
        msg = "Failed to convert PS info to JSON format.";
        return false;
    }

    return true;
}

void KMCommonUtils::clearBackendProc(const std::string &nsid)
{
    if (nsid.empty())
        return;

    while (true)
    {
        std::string command = "lsns -n -o NS,PID -t uts | grep " + nsid + " | awk '{print $NF}'";
        std::string pid = KMProcessUtils::execCmd(command.c_str()).first;
        if (!pid.empty())
        {
            kill(std::stoi(pid), SIGKILL);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief 停止正在运行的容器
 * @param[out] ret 返回状态码，成功为 0，失败为 -1。
 * @param[out] msg 若成功则为空，失败时包含错误信息（如未找到目标进程）。
 * @param[in] appid 可为应用 ID、进程 ID 或容器 ID
 * @return true 表示成功终止目标进程；false 表示未找到对应进程或执行失败。
 */
bool KMCommonUtils::kmkill(int &ret, std::string &msg, const std::string &appid)
{
    KMPSInfoList info;
    getPsInfo(info);
    bool bFind = false;
    for (auto &item : info.infolist)
    {
        if (item.appid == appid || std::to_string(item.pid) == appid || item.cid == appid)
        {
            bFind = true;
            std::string command = "lsns -n -t uts | grep " + item.appid + " | awk '{print $1}'";
            std::string nsid = KMProcessUtils::execCmd(command.c_str()).first;
            KMDebug("namespace id: " + nsid);
            std::vector<std::string> killArgs = { "kill", "-a", item.cid, "SIGKILL" };
            if (!KMCrun::exec(killArgs))
            {
                ret = -1;
                msg = "can not kill " + appid;
                KMError(msg);
            }
            killArgs = { "delete", "-f", item.cid };
            if (!KMCrun::exec(killArgs))
            {
                ret = -1;
                msg = "can not delete " + appid;
                KMError(msg);
            }

            clearBackendProc(nsid);
        }
    }

    if (!bFind)
    {
        ret = -1;
        msg = "[kill] can not find " + appid;
        return false;
    }

    ret = 0;
    msg.clear();

    return true;
}

/**
 * @brief 查询运行状态
 * @param[out] ret 返回状态码，成功为 0，失败为 -1。
 * @param[out] msg 若成功则为空，失败时包含错误信息（如未找到目标进程）。
 * @param[in] appid 可为应用 ID、进程 ID 或容器 ID
 * @return true 表示成功终止目标进程；false 表示未找到对应进程或执行失败。
 */
bool KMCommonUtils::getRunStatus(const std::string &appid, const std::string &uid, std::string &cid)
{
    KMPSInfoList info;
    getPsInfo(info, appid, true);
    for (const auto &item : info.infolist)
    {
        if (item.appid == appid)
        {
            std::error_code ec;
            std::string runDir;
            runDir.append("/run/user/").append(uid).append("/kaiming/").append(appid).append("/").append(item.cid);
            if (fs::exists(runDir, ec))
            {
                KMDebug(appid + " already running!");
                cid = item.cid;
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief 获取子目录
 * @param path 路径
 * @return 返回子目录路径
 */
std::string KMCommonUtils::getSingleSubdirectory(const std::string &path)
{
    std::error_code ec;
    if (!fs::exists(path, ec))
    {
        return {};
    }

    for (const auto &entry : fs::directory_iterator(path, ec))
    {
        if (entry.is_directory(ec))
        {
            return entry.path().filename().string();
        }
    }

    return {};
}

/**
 * @brief 获取子目录
 * @param path 路径
 * @return 返回子目录路径
 */
gchar **KMCommonUtils::makeArgv(bool isrun, const gchar *const *options, const gchar *const *soptions, const gchar *cid, const gchar *command)
{
    std::vector<std::string> args;
    args.emplace_back(KAIMING_BOX);
    args.emplace_back("--log");
    args.emplace_back(std::string("/tmp/") + cid + ".log");

    // crun options
    if (options)
    {
        for (int i = 0; options[i]; ++i)
        {
            args.emplace_back(options[i]);
        }
    }

    args.emplace_back(isrun ? "run" : "exec");

    // sub command soptions
    if (soptions)
    {
        for (int i = 0; soptions[i]; ++i)
        {
            args.emplace_back(soptions[i]);
        }
    }

    args.emplace_back(cid);

    // command (仅在 exec 模式下)
    if (!isrun && command)
    {
        parseArgs(command, args);
    }

    // 转换成 gchar**（用 g_strdup 确保生命周期）
    gchar **argv = g_new0(gchar *, args.size() + 1);
    for (size_t i = 0; i < args.size(); ++i)
    {
        argv[i] = g_strdup(args[i].c_str());
    }
    argv[args.size()] = nullptr;

    return argv;
}

/**
 * @brief 重定向标准输入输出到 /dev/null
 */
void KMCommonUtils::redirectToNull()
{
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        KMError("open /dev/null failed: " + std::string(strerror(errno)));
        return;
    }

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
}