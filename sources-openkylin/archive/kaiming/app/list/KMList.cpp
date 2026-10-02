/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMList.h"

#include <sys/personality.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <unordered_set>
#include <gio/gio.h>

#include "common/KMDbusJson.h"
#include "common/KMInfoJson.h"
#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/KMUtil.h"
#include "common/KMVersion.h"
#include "common/KMHostAppDeploy.h"

/**
 * @brief list命令行选项
 */
class KMList::Options : public KMOption::Options
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
    string m_sideload;
    string m_arch;
    string m_kind;
    string m_module;
};

void KMList::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMList::Options::postParseHook()
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
void KMList::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming list [options…]  - display installed applications or runtime\n"));
    //CHANGE帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display Help Options"));

    //应用选项
    addOption("kind", "", KMOption::value<std::string>(&m_kind), _("specify the type, such as app, runtime, base, extension"));
    addOption("module", "", KMOption::value<std::string>(&m_module), _("specify the module, such as binary, devel"));
}

class KMList::Private
{
public:
    std::shared_ptr<Options> m_options;
    std::shared_ptr<KMFolder> m_folder;
    std::shared_ptr<KMFormatter> m_formatter; // 格式化输出
    std::vector<KMlocalAppsInfo> m_localAppsInfo;
    std::vector<KMEntry> m_localInfo;
    size_t m_maxIdWidth;
    size_t m_maxNameWidth;
    size_t m_maxVersionWidth;
};

REGISTER_SUBCOMMAND_DYNCREATE(list, KMList)

KMList::KMList()
    : d(std::make_unique<Private>())
{
    init();
}

KMList::~KMList() = default;

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
int KMList::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv); // 检查未知选项

    vector<string> vecList;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            vecList.emplace_back(argv[i]);
            continue;
        }
        else if (!vecList.empty())
        {
            kmlogger.error("args error");
            return KM_ERROR_INVALID_ARGS;
        }
    }

    d->m_options->parseCommandLine(argc, argv);

    if (!vecList.empty())
    {
        kmlogger.error("Args number error, not support args.");
        printf("%s\n", _("Args number error, not support args."));
        return KM_ERROR_INVALID_ARGS;
    }
    vecList.clear();

    int ret = start(vecList);

    return ret;
}

/**
 * @brief 列出所有的已安装软件包
 *
 *
 * @param vecName
 * @return int
 */
int KMList::start(const vector<string> &vecName)
{
    if (vecName.empty())
    {
        kmlogger.info("Args is empty, so list all packages.");

        // 通过 DBus 调用 system 服务执行驱动清理（无 root 权限也可触发）
        {
            GError *error = NULL;
            GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
            if (!connection)
            {
                if (error)
                {
                    kmlogger.error("Failed to connect system bus: %s", error->message);
                    g_clear_error(&error);
                }
            }
            else
            {
                GVariant *result = g_dbus_connection_call_sync(
                    connection,
                    "org.kylin.kaiming",
                    "/org/kylin/kaiming",
                    "org.kylin.kaiming.interface",
                    "checkAndCleanupUninstalledDrivers",
                    NULL,
                    G_VARIANT_TYPE_TUPLE,
                    G_DBUS_CALL_FLAGS_NONE,
                    -1,
                    NULL,
                    &error);

                if (!result)
                {
                    if (error)
                    {
                        kmlogger.error("DBus cleanup call failed: %s", error->message);
                        g_clear_error(&error);
                    }
                }
                else
                {
                    gint cleanedCount = 0;
                    g_variant_get(result, "(i)", &cleanedCount);
                    if (cleanedCount > 0)
                    {
                        kmlogger.info("Cleaned up %d orphaned driver package(s) via DBus", cleanedCount);
                    }
                    g_variant_unref(result);
                }
                g_object_unref(connection);
            }
        }

        if (d->m_options->m_module == "devel")
        {
            d->m_formatter->setShowBase(true);
            d->m_formatter->setShowRuntime(true);
        }
        
        // 初始化最大宽度
        d->m_maxIdWidth = 0;
        d->m_maxNameWidth = 0;
        d->m_maxVersionWidth = 0;
        
        d->m_formatter->setShowArch(true);
        d->m_formatter->setShowChannel(true);
        
        auto layersPath = d->m_folder->getLayersPath();
        auto develPath = fs::path(KMStorageDir::getUserDevelDataDirLocation()) / "kaiming";
        if (fs::exists(layersPath))
        {
            findInfoJsonFiles(layersPath, "info.json", layersPath, false); // 第一次遍历不打印，只计算宽度
        }
        if (fs::exists(develPath))
        {
            findInfoJsonFiles(develPath, "info.json", develPath, false); // 第一次遍历不打印，只计算宽度
        }
        
        // 设置最小宽度
        size_t minIdWidth = 10;
        size_t minNameWidth = 20;
        size_t minVersionWidth = 15;
        
        // 确保宽度不小于标题长度
        d->m_maxIdWidth = std::max(d->m_maxIdWidth, minIdWidth)  + 5;
        d->m_maxNameWidth = std::max(d->m_maxNameWidth, minNameWidth) + 5;
        d->m_maxVersionWidth = std::max(d->m_maxVersionWidth, minVersionWidth) + 5;
        
        // 设置列宽
        d->m_formatter->setColumnWidth(d->m_maxIdWidth, d->m_maxNameWidth, d->m_maxVersionWidth, 12, 12, 12, 20, 15, 25, 25);
        d->m_formatter->printHeader();
        
        // 第二次遍历实际打印
        if (fs::exists(layersPath))
        {
            findInfoJsonFiles(layersPath, "info.json", layersPath, true);
        }
        if (fs::exists(develPath))
        {
            findInfoJsonFiles(develPath, "info.json", develPath, true);
        }
        d->m_formatter->printFooter();
    }

    return 0;
}

void KMList::init()
{
    d->m_options = std::make_shared<Options>();
    d->m_folder = std::make_shared<KMFolder>("/opt/kaiming", false);
    d->m_formatter = std::make_shared<KMFormatter>();
}

/**
 * @brief 格式化文件大小
 *
 * @param size
 * @return string
 */
std::string KMList::formatFileSize(double size) const
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

void KMList::findInfoJsonFiles(const fs::path &dirPath, const std::string &targetFilename, const std::string &destDir, const bool &isPrint)
{
    KMInfoJson infoJson;
    bool found = false;
    std::error_code ec;

    // 先遍历当前目录下的所有文件
    for (const auto &dirEntry : fs::directory_iterator(dirPath, ec))
    {
        if (ec)
        {
            continue;
        }

        if (dirEntry.is_symlink())
        {
            continue;
        }

        if (dirEntry.is_regular_file() && dirEntry.path().filename() == targetFilename)
        {
            fs::path parentDir = dirEntry.path().parent_path();
            fs::path deployFile = parentDir / "deploy";

            if (!fs::exists(deployFile) && !fs::is_regular_file(deployFile))
            {
                continue;
            }

            KMEntry localInfo;
            KMLocalInfoList localList;
            
            fs::path infoJsonPath = dirEntry.path();

            // 调用 loadFile() 函数解析info.json文件
            try
            {
                infoJson.loadFile(infoJsonPath);
            }
            catch (const std::exception& e)
            {
                // 捕获标准异常
                kmlogger.error("json parse error : %s", infoJsonPath.c_str());
                continue;
            }

            // 获取信息
            localInfo.id = infoJson.id;
            localInfo.channel = infoJson.channel;
            localInfo.module = infoJson.module;
            localInfo.version = infoJson.version;
            localInfo.size = std::to_string(infoJson.size);
            localInfo.name = infoJson.name;
            localInfo.description = infoJson.description;
            localInfo.base = infoJson.base;
            localInfo.runtime = infoJson.runtime;
            
            // 检查是否已存在相同的 id
            auto it = std::find_if(d->m_localAppsInfo.begin(), d->m_localAppsInfo.end(),
                [&infoJson](const KMlocalAppsInfo& appInfo) 
                {
                    return appInfo.id == infoJson.id;
                });

            if (it == d->m_localAppsInfo.end()) 
            {
                // 如果不存在，直接插入
                d->m_localAppsInfo.push_back({infoJson.id, infoJson.version, infoJson.module});
            } 
            else 
            {
                if (infoJson.module != it->module)
                {
                    // module不一致，直接插入
                    d->m_localAppsInfo.push_back({infoJson.id, infoJson.version, infoJson.module});
                }
                else
                {
                    // 如果已存在，比较版本号，保留更高的版本
                    KMVersion version1 = KMVersion::parse(infoJson.version, false);
                    KMVersion version2 = KMVersion::parse(it->version, false);
                    if (version1 > version2) // version1 > version2
                    {
                        // 更新现有项的版本
                        it->version = infoJson.version;
                    }
                }
            }
            localInfo.channel = infoJson.channel;

            std::ostringstream oss;
            for (size_t i = 0; i < infoJson.archs.size(); ++i)
            {
                oss << infoJson.archs[i];
                if (i != infoJson.archs.size() - 1)
                {
                    oss << ", "; // 分隔符
                }
            }
            localInfo.arch = oss.str();
            localList.kind = infoJson.kind;
            localList.archs = infoJson.archs;
            localList.module = infoJson.module;

            // 获取当前语言环境，输出对应语言环境下的name和description
            std::string localeName = KMUtil::getCurrentLocale();
            localInfo.name = !infoJson.names["name[" + localeName + "]"].empty() ? infoJson.names["name[" + localeName + "]"] : infoJson.name;
            KMStringUtils::removeTrailingNewline(localInfo.name);
            localInfo.description = !infoJson.descriptions["description[" + localeName + "]"].empty() ? infoJson.descriptions["description[" + localeName + "]"] : infoJson.description;
            KMStringUtils::removeTrailingNewline(localInfo.description);

            // 使用--kind --arch参数时过滤，只输出匹配的应用
            if (d->m_options->m_arch.empty())
            {
                d->m_options->m_arch = KMUtil::kmGetArch();
            }

            bool matchArch = std::find(localList.archs.begin(), localList.archs.end(), d->m_options->m_arch) != localList.archs.end();
            if (matchArch)
            {
                localInfo.arch = d->m_options->m_arch;
            }

            if (localList.kind == "base" || localList.kind == "runtime")
            {
                localInfo.arch = localList.archs[0];
                matchArch = true;
            }

            d->m_localInfo.push_back(localInfo);
            bool matchKind = d->m_options->m_kind.empty() || localList.kind == d->m_options->m_kind;

            bool matchModule = d->m_options->m_module.empty() || localList.module == d->m_options->m_module;
            if (matchArch && matchKind && matchModule)
            {
                localInfo.size = formatFileSize(infoJson.size);
                // 计算最大宽度
                size_t idWidth = localInfo.id.length();
                size_t nameWidth = localInfo.name.length();
                size_t versionWidth = localInfo.version.length();
                
                if (idWidth > d->m_maxIdWidth) d->m_maxIdWidth = idWidth;
                if (nameWidth > d->m_maxNameWidth) d->m_maxNameWidth = nameWidth;
                if (versionWidth > d->m_maxVersionWidth) d->m_maxVersionWidth = versionWidth;
                
                if (isPrint)
                {
                    d->m_formatter->printEntry(localInfo);
                }
            }
            found = true;
            break; // 处理完一个文件后跳出当前目录的文件遍历
        }
    }

    if (!found)
    { 
        // 再遍历当前目录下的所有子目录
        std::error_code ec2; // 使用新的 error_code 避免与第一个循环冲突
        for (const auto &dirEntry : fs::directory_iterator(dirPath, ec2))
        {
            if (ec2)
            {
                continue; // 跳过无法访问的目录
            }

            if (dirEntry.is_symlink())
            {
                continue;
            }

            if (dirEntry.is_directory())
            {
                std::string subSrcDir = dirEntry.path().string();
                std::string subDestDir = destDir + "/" + dirEntry.path().filename().string();
                
                // 过滤 "entires", "files" 目录，避免过深遍历或遍历到 bluetooth 这种 700权限目录导致读取权限有问题
                if (subSrcDir.find("/entries") != std::string::npos || subSrcDir.find("/files") != std::string::npos)
                {
                    continue;
                }

                // 创建目标目录（使用 error_code 避免异常）
                std::error_code create_ec;
                fs::create_directory(subDestDir, create_ec);
                if (create_ec) {
                    continue;
                }

                // 递归处理子目录
                findInfoJsonFiles(subSrcDir, targetFilename, subDestDir, isPrint);
            }
        }
    }
}
const std::vector<KMlocalAppsInfo> &KMList::getLocalAppIds(const bool &isPrint)
{
    auto layersPath = d->m_folder->getLayersPath();
    auto develPath = fs::path(KMStorageDir::getUserDevelDataDirLocation()) / "kaiming";
    if (fs::exists(layersPath))
    {
        findInfoJsonFiles(layersPath, "info.json", layersPath, isPrint);
    }
    if (fs::exists(develPath))
    {
        findInfoJsonFiles(develPath, "info.json", develPath, isPrint);
    }
    return d->m_localAppsInfo;
}

std::vector<std::string> KMList::getAllLocalInfo()
{
    std::vector<std::string> resultJson;
    auto layersPath = d->m_folder->getLayersPath();
    auto develPath = fs::path(KMStorageDir::getUserDevelDataDirLocation()) / "kaiming";
    if (fs::exists(layersPath))
    {
        findInfoJsonFiles(layersPath, "info.json", layersPath, false);
    }
    if (fs::exists(develPath))
    {
        findInfoJsonFiles(develPath, "info.json", develPath, false);
    }

    std::string packInfoToJson;

    for (const auto &entry : d->m_localInfo)
    {
        packInfoToJson = KMDbusJson::packLocalInfoToJson(
            entry.id,
            entry.name,
            entry.version,
            entry.size,
            entry.channel,
            entry.arch,
            entry.module,
            entry.description
        );

        resultJson.push_back(packInfoToJson);
    }
    d->m_localInfo.clear();
    return resultJson;
}
void KMList::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--kind",
        "--module",
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
                std::cerr << _("Please use ") << ("'kaiming list --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}