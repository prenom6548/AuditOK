/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMInfoCommand.h"

#include <cstring>
#include <sys/personality.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>

#include "common/KMBuildinsUtils.h"
#include "common/KMConfig.h"
#include "common/KMError.h"
#include "common/KMDbusJson.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/kmtranslation.h"
#include "common/KMInfoJson.h"
#include "common/KMVersion.h"
#include "app/search/KMSearch.h"
#define YELLOW_COLOR 33
#define CYAN_COLOR 36

class KMInfoCommand::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    void checkUnknownOptions(int argc, char** argv);

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
    std::string m_remote;

    bool m_isListFiles = false;
    std::string m_name;
};

void KMInfoCommand::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMInfoCommand::Options::postParseHook()
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
void KMInfoCommand::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming info [option…] [quote…] - View information on locally installed apps\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display help information"));
    addOption("listfiles", "L", KMOption::value<bool>(&m_isListFiles), _("display all files installed by the application"));

    //位置选项
    addPositionOption("NAME", KMOption::value<std::string>(&m_name), 1, _("application name"));
}

class KMInfoCommand::Private
{
public:
    std::unique_ptr<Options> m_options;
    std::unique_ptr<KMFolder> m_folder;

    KMLocalInfo m_localInfo;
    KMInfoJson m_localDebInfo;

    bool isRemoteApi = false; // 获取远程信息，供 dbus api调用
    bool isSearchId = false;
};

REGISTER_SUBCOMMAND_DYNCREATE(info, KMInfoCommand)

KMInfoCommand::KMInfoCommand()
    : d(std::make_unique<Private>())
{
    init();
}

KMInfoCommand::~KMInfoCommand() = default;

/**
 * @brief list 命令参数处理,不需要参数
 *
 * 列出所有已安装的软件包信息,只支持不含参数
 *
 * 使用实例:
 *      1. kaiming list
 *
 * @param argc
 * @param argv
 * @return int
 */
int KMInfoCommand::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv); 
    // 参数解析
    d->m_options->parseCommandLine(argc, argv);

    if (d->m_options->m_name.empty())
    {
        KMError(_("Missing required argument NAME. Use 'kaiming info --help' for usage."));
        return KM_ERROR_INVALID_ARGS;
    }

    int ret = start(d->m_options->m_name);

    return ret;
}

/**
 * @brief 列出所有已安装软件包的信息
 *
 *
 * @param name
 * @return int
 */
int KMInfoCommand::start(const std::string &name)
{
    string refId = name;
    bool isFound = false;
    d->m_localInfo.version = "";
    kmlogger.info("refId: %s", refId.c_str());

    if (!isValidRefId(refId))
    {
        kmlogger.error("refId %s is invalid.", refId.c_str());
        return KM_ERROR_INVALID_ARGS;
    }

    if ((d->m_options->m_remote == "kaiming-repo" || d->isRemoteApi) && !d->isSearchId)
    {
        // 没有在summary.json中找到可匹配的id
        return KM_ERROR_REMOTE_NOT_FOUND;
    }

    if (d->m_options->m_isListFiles)
    {
        const auto& infoPath = d->m_folder->getInfoPath();
        const fs::path infoListFile = infoPath / (name + ".list");
        const fs::path hostListFile = infoPath / (name + ".hostlist");
        fileInListFile(infoListFile);
        fileInListFile(hostListFile);
        return KM_ERROR_NO;
    }

    if (auto layersPath = d->m_folder->getLayersPath(); fs::exists(layersPath))
    {
        findInfoJsonFiles(layersPath, "info.json", layersPath, refId, isFound);
    }

    if (auto develPath = fs::path(KMStorageDir::getUserDevelDataDirLocation()); fs::exists(develPath) && isFound == false)
    {
        findInfoJsonFiles(develPath, "info.json", develPath, refId, isFound);
    }

    if (!isFound)
    {
        std::cout << " " << refId << " not find" << endl;
        return KM_ERROR_REF_NOT_FOUND;
    }

    return KM_ERROR_NO;
}

void KMInfoCommand::init()
{
    d->m_options = std::make_unique<Options>();
    d->m_folder = std::make_unique<KMFolder>("/opt/kaiming", false);
}

/**
 * @brief 给软件商店调用的api
 *
 * @param name 开明包名，如org.kde.kclock
 * @param version 开明版本号，这里实际上是ostree branch版本，暂未细化到具体子版本。空代表最新版本，如"master"
 * @param error
 * @return true
 * @return false
 */
string KMInfoCommand::getLocalDebInfo(const string &name, int &errorCode)
{
    int ret = start(name);
    errorCode = ret;

    // 获取 debinfo
    DebInfo &deb = d->m_localDebInfo.annotations.debinfo;

    nlohmann::json jsonNode;

    jsonNode["Package"]       = deb.package.empty() ? "" : deb.package;
    jsonNode["Version"]       = deb.version.empty() ? "" : deb.version;
    jsonNode["Section"]       = deb.section.empty() ? "" : deb.section;
    jsonNode["Package-Type"]  = deb.package_type.empty() ? "" : deb.package_type;
    jsonNode["Recommends"]    = deb.recommends.empty() ? "" : deb.recommends;
    jsonNode["Uploaders"]     = deb.uploaders.empty() ? "" : deb.uploaders;
    jsonNode["Provides"]      = deb.provides.empty() ? "" : deb.provides;
    jsonNode["Origin"]        = deb.origin.empty() ? "" : deb.origin;
    jsonNode["Multi-Arch"]    = deb.multi_arch.empty() ? "" : deb.multi_arch;
    jsonNode["Build-Depends"] = deb.build_depends.empty() ? "" : deb.build_depends;
    jsonNode["Source"]        = deb.source.empty() ? "" : deb.source;
    jsonNode["Suggests"]      = deb.suggests.empty() ? "" : deb.suggests;
    jsonNode["Conflicts"]     = deb.conflicts.empty() ? "" : deb.conflicts;
    jsonNode["Breaks"]        = deb.breaks.empty() ? "" : deb.breaks;
    jsonNode["Replaces"]      = deb.replaces.empty() ? "" : deb.replaces;
    jsonNode["Vcs-Git"]       = deb.vcs_git.empty() ? "" : deb.vcs_git;
    jsonNode["Vcs-Browser"]   = deb.vcs_browser.empty() ? "" : deb.vcs_browser;

    string debInfoJson = jsonNode.dump();

    kmlogger.info("DebInfo JSON: %s", debInfoJson.c_str());

    return debInfoJson;
}

string KMInfoCommand::getLocalInfo(const string &name, int &errorCode)
{
    int ret = KM_ERROR_NO;

    ret = start(name);

    errorCode = ret;

    string packInfoToJson;

    if (errorCode != KM_ERROR_NO)
    {
        packInfoToJson = KMDbusJson::errorLocalJson();
    }
    else
    {
        packInfoToJson = KMDbusJson::packLocalInfoToJson(d->m_localInfo.id, d->m_localInfo.name, d->m_localInfo.version, std::to_string(uint64_t(d->m_localInfo.size)), d->m_localInfo.branch,
                                                            d->m_localInfo.arch, d->m_localInfo.module, d->m_localInfo.description);
    }

    kmlogger.info("Info packInfoToJson: %s", packInfoToJson.c_str());

    return packInfoToJson;
}

/**
 * @brief 校验refId参数是否合法的接口
 *
 * 校验规则：以"."为分隔符切分字符串refId得到的子串个数小于3个，则代表参数非法，返回false
 *
 * @param refId
 * @return true
 * @return false
 */
bool KMInfoCommand::isValidRefId(const std::string &refId) const
{
    if (refId == "")
    {
        return false;
    }

    std::vector<std::string> tokens;
    std::istringstream iss(refId);
    std::string token;
    while (std::getline(iss, token, '.'))
    {
        tokens.push_back(token);
    }

    if (tokens.size() < 3)
    {
        return false;
    }

    return true;
}

/**
 * @brief 格式化输出下载的包信息，运行时输出是下载大小，软件包是安装大小
 *
 * @param ref
 * @param index
 */
void KMInfoCommand::printRefInfo(const KMLocalInfo &info)
{
    // 获取各个字段的最大长度
    size_t maxIdLength          = std::max<size_t>(strlen(_("Id")), info.id.size());
    size_t maxNameLength        = std::max<size_t>(strlen(_("Name")), info.name.size());
    size_t maxBranchLength      = std::max<size_t>(strlen(_("Branch")), info.branch.size());
    size_t maxModuleLength      = std::max<size_t>(strlen(_("Module")), info.module.size());
    size_t maxVersionLength     = std::max<size_t>(strlen(_("Version")), info.version.size());
    size_t maxSizeLength        = std::max<size_t>(strlen(_("Size")), formatFileSize(info.size).size());
    size_t maxDescriptionLength = std::max<size_t>(strlen(_("Description")), info.description.size());
    size_t maxRuntimeLength     = std::max<size_t>(strlen(_("Runtime")), info.runtime.size());

    // 动态设置每列的宽度
    const int pad = 5;
    int nameWidth        = static_cast<int>(maxNameLength) + pad;
    int idWidth          = static_cast<int>(maxIdLength) + pad;
    int branchWidth      = static_cast<int>(maxBranchLength) + pad;
    int moduleWidth      = static_cast<int>(maxModuleLength) + pad;
    int versionWidth     = static_cast<int>(maxVersionLength) + pad;
    int sizeWidth        = static_cast<int>(maxSizeLength) + pad;
    int descriptionWidth = static_cast<int>(maxDescriptionLength) + pad;
    int runtimeWidth     = static_cast<int>(maxRuntimeLength) + 0;  

    // 打印表头
    printf("\033[%dm%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s\033[0m\n", 
           CYAN_COLOR,
           nameWidth, _("Name"),
           idWidth, _("Id"),
           branchWidth, _("Branch"),
           moduleWidth, _("Module"),
           versionWidth, _("Version"),
           sizeWidth, _("Size"),
           descriptionWidth, _("Description"),
           runtimeWidth, _("Runtime"));

    // 打印数据行
    printf("%-*s%-*s%-*s%-*s%-*s%-*s%-*s%-*s\n",
           nameWidth, info.name.c_str(),
           idWidth, info.id.c_str(),
           branchWidth, info.branch.c_str(),
           moduleWidth, info.module.c_str(),
           versionWidth, info.version.c_str(),
           sizeWidth, formatFileSize(info.size).c_str(),
           descriptionWidth, info.description.c_str(),
           runtimeWidth, info.runtime.c_str());

    if (d->m_localInfo.commitData != nullptr)
    {
        printf("\n");
        printBoldColorHead(YELLOW_COLOR, _("Commit:"), info.commit.c_str());

        if (info.parent != " ")
        {
            printBoldColorHead(YELLOW_COLOR, _("Parent:"), info.parent.c_str());
        }

        if (info.subject != " ")
        {
            printBoldColorHead(YELLOW_COLOR, _("Subject:"), info.subject.c_str());
        }

        if (info.date != "?")
        {
            printBoldColorHead(YELLOW_COLOR, _("Date:"), info.date.c_str());
        }
    }
}

/**
 * @brief 标题printf打印时加粗和加颜色
 *
 * @param text 打印文本
 * @param color 颜色选择，31 红色 33 黄色
 * @return string
 */
void KMInfoCommand::printBoldColorHead(int color, const char *head, const char *text) const
{
    printf("\033[1m\033[%dm%s\033[0m %s\n", color, head, text);
}

/**
 * @brief 格式化文件大小
 *
 * @param size
 * @return string
 */
string KMInfoCommand::formatFileSize(double size) const
{
    vector<string> units = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    int index = 0;

    while (size >= 1024.0 && index < units.size())
    {
        size /= 1024.0;
        index++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[index];
    return oss.str();
}

void KMInfoCommand::findInfoJsonFiles(const fs::path &dirPath, const std::string &targetFilename, const std::string &destDir, const std::string &refId, bool &isFound)
{
    KMInfoJson infoJson;
    // 遍历并处理所有的info.json
    for (const auto &dirEntry : fs::directory_iterator(dirPath))
    {
        KMLocalInfo localInfo;
        // 找到所有的 info.json 文件
        if (fs::is_symlink(fs::path(dirEntry)))
        {
            continue;
        }

        if (fs::is_directory(dirEntry))
        {
            // 递归处理子目录
            std::string subSrcDir = dirEntry.path().string();
            std::string subDestDir = destDir + "/" + dirEntry.path().filename().string();
            fs::create_directory(subDestDir);

            // 过滤 "entires", "files" 目录，避免过深遍历或遍历到 bluetooth 这种 700权限目录导致读取权限有问题
            if (subSrcDir.find("/entries") != std::string::npos || subSrcDir.find("/files") != std::string::npos)
            {
                continue;
            }

            // 如果是目录，则递归调用
            findInfoJsonFiles(subSrcDir, targetFilename, subDestDir, refId, isFound);
        }
        else if (dirEntry.is_regular_file() && dirEntry.path().filename() == targetFilename)
        {
            fs::path infoJsonPath = dirEntry.path();
            // 调用 loadFile() 函数解析info.json文件
            infoJson.loadFile(infoJsonPath);

            // 使用--kind --arch参数时过滤，只输出匹配的应用
            std::string localArch = KMUtil::kmGetArch();
            // 获取信息
            if (infoJson.id == refId)
            {
                localInfo.id = infoJson.id;
                localInfo.completedId = infoJson.base;
                localInfo.runtime = infoJson.runtime;
                localInfo.module = infoJson.module;
                localInfo.size = infoJson.size;
                localInfo.version = infoJson.version;
                localInfo.name = infoJson.name;
                localInfo.branch = infoJson.channel;
                auto it = std::find(infoJson.archs.begin(), infoJson.archs.end(), localArch);
                if (it != infoJson.archs.end()) 
                {
                    localInfo.arch = *it;
                }
                else
                {
                    continue;
                }
                // 获取最大版本号
                if (d->m_localInfo.version.empty() || KMVersion::compare(infoJson.version, d->m_localInfo.version) > 0)
                {
                    // 更新最大版本号和相关信息
                    d->m_localInfo.id = infoJson.id;
                    d->m_localInfo.version = infoJson.version;
                    d->m_localInfo.size = infoJson.size;
                    d->m_localInfo.branch = infoJson.channel;
                    d->m_localInfo.description = infoJson.description;
                    d->m_localInfo.arch = localInfo.arch;
                    d->m_localInfo.name = infoJson.name;
                    d->m_localInfo.module = infoJson.module;
                    d->m_localDebInfo.annotations.debinfo = infoJson.annotations.debinfo;
                }
            }
            else
            {
                continue;
            }

            // 获取当前语言环境
            KMUtil locale;
            std::string localeName = locale.getCurrentLocale();
            if (!infoJson.names["name[" + localeName + "]"].empty())
            {
                localInfo.name = infoJson.names["name[" + localeName + "]"];
            }
            else
            {
                localInfo.name = infoJson.name;
            }

            if (!infoJson.descriptions["description[" + localeName + "]"].empty())
            {
                localInfo.description = infoJson.descriptions["description[" + localeName + "]"];
            }
            else
            {
                localInfo.description = infoJson.description;
            }

            printRefInfo(localInfo);
            isFound = true;
        }
    }
}

/**
 * @brief 获取所有符合要求的文件路径
 *
 *
 * @param directoryPath
 * @param files
 * @return true
 * @return false
 */
void KMInfoCommand::getAllFiles(const std::string &directoryPath, std::set<std::string> &files, const std::string &targetFilename, bool isFoundFile)
{
    try
    {
        for (const auto &dirEntry : fs::directory_iterator(directoryPath))
        {
            if (fs::is_symlink(fs::path(dirEntry)) && dirEntry.path().filename() != targetFilename)
            {
                continue;
            }

            if (isFoundFile)
            {
                // 已找到目标文件
                files.insert(dirEntry.path().string());
                if (dirEntry.is_directory())
                {
                    getAllFiles(dirEntry.path().string(), files, targetFilename, isFoundFile);
                }
            }
            else if (dirEntry.path().filename() == targetFilename)
            {
                isFoundFile = true;
                files.insert(dirEntry.path().string());

                // 逐层向上获取所有父目录，并将它们的路径添加到文件列表中
                for (auto parentPath = dirEntry.path().parent_path(); !parentPath.empty() && parentPath != parentPath.root_path(); parentPath = parentPath.parent_path())
                {
                    files.insert(parentPath.string());
                }

                if (dirEntry.is_directory())
                {
                    getAllFiles(dirEntry.path().string(), files, targetFilename, isFoundFile);
                }
            }
            else if (dirEntry.is_directory())
            {
                // 是目录则继续向下遍历
                getAllFiles(dirEntry.path().string(), files, targetFilename, isFoundFile);
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        kmlogger.error("Skipping directory due to access error.");
    }
}

/**
 * @brief 实现kaiming info -L 功能,显示应用安装的所有文件
 *
 * @param name
 */
void KMInfoCommand::listFilesDirectory(const std::string &targetFilename)
{
    std::set<std::string> files;

    fs::path layersPath = d->m_folder->getLayersPath();
    fs::path exportsPath = d->m_folder->getExportsPath();
    fs::path binPath = d->m_folder->getExportsBinPath();
    fs::path sharePath = d->m_folder->getExportsSharePath();

    bool isFoundFile = false;

    if (fs::exists(binPath))
    {
        getAllFiles(binPath, files, targetFilename, isFoundFile);
    }

    if (fs::exists(sharePath))
    {
        getAllFiles(sharePath, files, targetFilename, isFoundFile);
    }

    getAllFiles(layersPath, files, targetFilename, isFoundFile);
    getAllFiles(exportsPath, files, targetFilename, isFoundFile);

    for (const auto &file : files)
    {
        // 输出获取到的文件列表
        std::cout << file << std::endl;
    }
}

void KMInfoCommand::fileInListFile(const fs::path &listFilePath)
{
    if (listFilePath.empty())
    {
        KMDebug("Provided file path is empty.");
        return;
    }

    std::ifstream file(listFilePath);
    if (!file.is_open())
    {
        KMDebug("Error opening file:  " + listFilePath.string());
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << line << '\n';
    }
}

void KMInfoCommand::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--listfiles", "-L",
        "--loglevel",
        "--verbose", "-v",
    };

    for (int i = 1; i < argc; ++i) 
    {
        std::string arg(argv[i]);

        if (arg.size() >= 1 && arg[0] == '-') 
        {
            std::string opt = arg;
            auto eq_pos = opt.find('=');
            if (eq_pos != std::string::npos)
                opt = opt.substr(0, eq_pos);

            if (validOptions.find(opt) == validOptions.end()) 
            {
                std::cerr << "\033[1;31m" << _("E:") << "\033[0m " << _("Unrecognized option “") << arg << "”" << std::endl;
                std::cerr << _("Please use ") << ("'kaiming info --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}