/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <fstream>
#include <set>
#include <array>

#include "KMBrowserUtils.h"
#include "KMStringUtils.h"

/**
 * @brief 获取系统中所有已注册的浏览器可执行文件路径
 * @return 浏览器可执行文件路径列表
 */
std::vector<std::string> KMBrowserUtils::getAllBrowserExecutables()
{
    auto desktopNames = getRegisteredDesktopFiles();
    auto desktopPaths = locateDesktopFiles(desktopNames);
    auto execInfos = extractExecutables(desktopPaths);
    writeExecutablesToFile(execInfos);

    // 从 ExecInfo 结构中提取可执行文件路径
    std::vector<std::string> executables;
    for (const auto &info : execInfos)
    {
        executables.push_back(info.execPath);
    }

    return executables;
}

/**
 * @brief 从配置文件中读取已保存的浏览器路径
 * @return 浏览器可执行文件路径列表
 */
std::vector<ExecInfo> KMBrowserUtils::loadExecutablesFromFile()
{
    std::vector<ExecInfo> result;

    fs::path filePath = "/opt/kaiming-tools/config/browsers.conf";

    if (!fs::exists(filePath))
    {
        KMDebug("配置文件不存在: " + filePath.string());
        return result;
    }

    std::ifstream ifs(filePath);
    if (!ifs.is_open())
    {
        KMDebug("无法打开文件: " + filePath.string());
        return result;
    }

    std::string line;

    while (std::getline(ifs, line))
    {
        // 去掉空白
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        // 解析 exec 信息
        if (line.rfind("exec=", 0) == 0)
        {
            ExecInfo info;
            std::string content = line.substr(5);
            size_t commaPos = content.find(',');

            if (commaPos == std::string::npos)
            {
                // 普通浏览器格式：exec=/usr/bin/firefox
                info.execPath = content;
            }
            else
            {
                // kaiming 浏览器格式：exec=/xxx,appid=yyy
                info.execPath = content.substr(0, commaPos);

                std::string appPart = content.substr(commaPos + 1);
                if (appPart.rfind("appid=", 0) == 0)
                {
                    info.appId = appPart.substr(strlen("appid="));

                    std::string layerPath = "/opt/kaiming/layers/stable";
                    std::string arch = KMCommonUtils::getSingleSubdirectory(layerPath);
                    if (!arch.empty())
                    {
                        std::string versionPath = layerPath + "/" + arch + "/app/" + info.appId + "/binary/";
                        if (fs::exists(versionPath))
                        {
                            for (const auto &entry : fs::directory_iterator(versionPath))
                            {
                                if (fs::is_directory(entry))
                                {
                                    info.version = entry.path().filename().string();
                                    // 添加 desktop 路径
                                    info.desktopPath = versionPath + info.version + "/files/usr/share/applications";
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            result.push_back(info);
        }
    }

    return result;
}

/**
 * @brief 执行系统命令并获取输出结果
 * @param cmd 要执行的命令
 * @return 包含命令输出和错误码的pair
 */
std::pair<std::string, std::error_code> KMBrowserUtils::execCommand(const std::string &command)
{
    if (command.empty())
    {
        return {"", std::make_error_code(std::errc::invalid_argument)};
    }

    std::array<char, 256> buffer{};
    std::ostringstream result;

    // 重置 errno
    errno = 0;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe)
    {
        return {"", std::make_error_code(static_cast<std::errc>(errno))};
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result << buffer.data();
    }

    return {result.str(), std::error_code{}};
}

/**
 * @brief 获取已注册的浏览器桌面文件列表
 * @return 浏览器桌面文件名列表
 */
std::vector<std::string> KMBrowserUtils::getRegisteredDesktopFiles()
{
    std::vector<std::string> desktopFiles;
    auto [output, ec] = execCommand("gio mime x-scheme-handler/http");
    if (ec)
    {
        KMDebug("执行命令失败: gio mime x-scheme-handler/http, 错误: " + ec.message());
        return desktopFiles;
    }

    std::istringstream iss(output);
    std::string line;
    bool parseApps = false;

    while (std::getline(iss, line))
    {
        if (line.find("Registered applications") != std::string::npos || line.find("已注册的应用程序") != std::string::npos)
        {
            parseApps = true;
            continue;
        }
        if (parseApps)
        {
            size_t pos = line.find(".desktop");
            if (pos != std::string::npos)
            {
                std::string file = line.substr(0, pos + 8);
                file.erase(0, file.find_first_not_of(" \t"));
                // 去除尾部空白字符
                file.erase(file.find_last_not_of(" \t") + 1);
                if (!file.empty())
                {
                    desktopFiles.push_back(file);
                }
            }
        }
    }

    return desktopFiles;
}

/**
 * @brief 根据桌面文件名定位完整的桌面文件路径
 * @param desktopNames 桌面文件名列表
 * @return 完整的桌面文件路径列表
 */
std::vector<fs::path> KMBrowserUtils::locateDesktopFiles(const std::vector<std::string> &desktopNames)
{
    std::vector<fs::path> result;

    // 获取XDG数据目录
    const char *xdgDirsEnv = std::getenv("XDG_DATA_DIRS");
    std::string xdgDirs = xdgDirsEnv ? xdgDirsEnv : "/usr/local/share:/usr/share";

    // 获取当前登录用户
    std::string loginUser = KMStorageDir::getLoginUser();

    // 添加用户本地目录（如果能获取到用户信息）
    if (!loginUser.empty())
    {
        std::string userLocalDir = "/home/" + loginUser + "/.local/share";
        // 将用户目录添加到搜索路径的开头，具有更高优先级
        xdgDirs = userLocalDir + ":" + xdgDirs;
    }

    // 添加kaiming默认目录
    xdgDirs += ":/opt/kaiming/share";

    std::istringstream ss(xdgDirs);
    std::string dataDir;

    while (std::getline(ss, dataDir, ':'))
    {
        // 清理路径字符串
        dataDir.erase(0, dataDir.find_first_not_of(" \t"));
        dataDir.erase(dataDir.find_last_not_of(" \t") + 1);

        if (dataDir.empty())
            continue;

        KMDebug("正在搜索目录: " + dataDir);

        fs::path applicationsDir = fs::path(dataDir) / "applications";
        if (!fs::exists(applicationsDir))
        {
            KMDebug("目录不存在: " + applicationsDir.string());
            continue;
        }

        for (const auto &desktopName : desktopNames)
        {
            fs::path fullPath = applicationsDir / desktopName;
            if (fs::exists(fullPath))
            {
                result.push_back(fullPath);
            }
        }
    }
    return result;
}

/**
 * @brief 解析Exec=行，提取可执行文件路径和参数
 * @param execLine Exec=行内容
 * @param info 解析结果
 */
void KMBrowserUtils::parseExecLine(const std::string &execLine, ExecInfo &info)
{
    std::vector<std::string> splits = KMStringUtils::splitString(execLine, " ");
    if (splits.size() >= 2 && splits[0].find("kaiming") != std::string::npos && splits[1] == "run")
    {
        // 从 splits[2] 开始查找 --command 和 appid
        for (size_t i = 2; i < splits.size(); ++i)
        {
            if (splits[i].rfind("--command=", 0) == 0)
            {
                info.execPath = splits[i].substr(strlen("--command="));
                continue;
            }
            if (!splits[i].empty() && splits[i][0] != '-')
            {
                info.appId = splits[i];
                break;
            }
        }
    }
    else
    {
        // 普通浏览器
        std::string execname = splits.front();
        if (!execname.empty())
        {
            info.execPath = execname;
        }
    }
}

/**
 * @brief 从桌面文件中提取可执行文件路径
 * @param desktopPaths 桌面文件路径列表
 * @return 可执行文件路径列表
 */
std::vector<ExecInfo> KMBrowserUtils::extractExecutables(const std::vector<fs::path> &desktopPaths)
{
    std::vector<ExecInfo> result;

    // 使用一个统一的去重集合
    std::set<std::string> uniqueExecs;

    for (const auto &desktopPath : desktopPaths)
    {
        std::ifstream file(desktopPath);
        if (!file.is_open())
        {
            KMDebug("无法打开 .desktop 文件: " + desktopPath.string());
            continue;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.rfind("Exec=", 0) != 0)
                continue;

            std::string execLine = line.substr(5);

            execLine.erase(0, execLine.find_first_not_of(" \t"));
            execLine.erase(execLine.find_last_not_of(" \t") + 1);

            ExecInfo info;
            parseExecLine(execLine, info);
            // 避免重复的浏览器路径被多次添加到最终结果中
            if (!info.execPath.empty())
            {
                std::string key = info.execPath + (info.appId.empty() ? "" : ":" + info.appId);
                if (uniqueExecs.insert(key).second)
                {
                    result.push_back(info);
                }
            }
        }
    }

    return result;
}

/**
 * @brief 将浏览器可执行文件路径写入配置文件
 * @param executables 可执行文件路径列表
 */
void KMBrowserUtils::writeExecutablesToFile(const std::vector<ExecInfo> &executables)
{
    fs::path configDir = "/opt/kaiming-tools/config";
    fs::path filePath = configDir / "browsers.conf";

    std::error_code ec;
    if (!fs::exists(configDir))
    {
        if (!fs::create_directories(configDir, ec))
        {
            KMDebug("无法创建配置目录: " + ec.message());
            return;
        }
    }

    std::vector<std::string> newLines;

    for (const auto &info : executables)
    {
        if (!info.appId.empty())
        {
            // kaiming 浏览器格式
            newLines.push_back("exec=" + info.execPath + ",appid=" + info.appId);
        }
        else
        {
            // 普通浏览器格式
            newLines.push_back("exec=" + info.execPath);
        }
    }

    // 写文件
    std::ofstream ofs(filePath, std::ios::trunc);
    if (!ofs.is_open())
    {
        KMDebug("无法写入配置文件: " + filePath.string());
        return;
    }

    for (auto &line : newLines)
    {
        ofs << line << "\n";
    }
    ofs.close();

    if (ofs.fail())
    {
        KMDebug("写入配置文件时出错: " + filePath.string());
    }
    else
    {
        KMInfo("已更新浏览器配置文件: " + filePath.string());
    }
}
