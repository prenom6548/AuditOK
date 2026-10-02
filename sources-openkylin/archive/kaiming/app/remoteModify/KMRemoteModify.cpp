
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "KMRemoteModify.h"
#include "common/KMError.h"
#include "common/KMLogger.h"
#include "common/kmtranslation.h"
#include "common/KMFolder.h"

static const std::string CONFIG_PATH = "/opt/kaiming/repo/config";

class KMRemoteModify::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    void checkUnknownOptions(int argc, char **argv);

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
    std::string m_repoName;
    std::string m_repoUrl;
    std::string m_operation;
    std::string m_mode;
    std::string m_minFreeSpace;
    std::string m_repoVersion;
};

void KMRemoteModify::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMRemoteModify::Options::postParseHook()
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
void KMRemoteModify::Options::addOptions()
{
    setDescription(_("\nUsage:\n"
                     "\tkaiming remote <add|remove|modify|list|show> [options]\n\n"
                     "Examples:\n"
                     "\tkaiming remote add kaiming-repo --repo=https://archive2.kylinos.cn/KM/OPENKYLIN/repo\n"
                     "\tkaiming remote modify kaiming-repo --repo=https://archive2.kylinos.cn/KM/OPENKYLIN/repo\n"
                     "\tkaiming remote remove kaiming-repo\n"
                     "\tkaiming remote list\n"
                     "\tkaiming remote show\n"
                     //  "\tkaiming remote set-core --mode=bare-user --min-free-space=500MB\n"
                     ));

    //CHANGE帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_bHelp)->defaultValue(false), _("display Help Options"));

    // 基本参数
    //addPositionOption("operation", KMOption::value<std::string>(&m_operation), 1, _("Operation name (add/remove/modify/list/show/set-core)"));
    addPositionOption("operation", KMOption::value<std::string>(&m_operation), 1, _("Operation name (add/remove/modify/list/show)"));
    addPositionOption("reponame", KMOption::value<std::string>(&m_repoName), 1, _("Repository name"));

    // 可选参数
    addOption("repo", "", KMOption::value<std::string>(&m_repoUrl), _("Remote repo url"));

    // addOption("repo-version", "", KMOption::value<std::string>(&m_repoVersion), _("Core repo version"));
    // addOption("mode", "", KMOption::value<std::string>(&m_mode), _("Core mode"));
    // addOption("min-free-space", "", KMOption::value<std::string>(&m_minFreeSpace), _("Core min free space"));
}

class KMRemoteModify::Private
{
public:
    std::unique_ptr<Options> m_options;
    std::unique_ptr<KMEnv> m_env;
    std::unique_ptr<KMFolder> m_folder;
};

REGISTER_SUBCOMMAND_DYNCREATE(remote, KMRemoteModify)

KMRemoteModify::KMRemoteModify()
    : d(std::make_unique<Private>())
{
    d->m_options = std::make_unique<Options>();
    d->m_env = std::make_unique<KMEnv>();
    d->m_folder = std::make_unique<KMFolder>();
}

KMRemoteModify::~KMRemoteModify() = default;

int KMRemoteModify::dispose(int argc, char **argv)
{
    d->m_options->checkUnknownOptions(argc, argv);
    d->m_options->parseCommandLine(argc, argv);

    // create repo
    d->m_folder->ensureRepoForSudo(false);

    const auto &op = d->m_options->m_operation;
    bool needPrivilege = (op == "add" || op == "remove" || op == "modify" || op == "set-core");

    if (needPrivilege && !d->m_env->passEnvCheck())
        return KM_ERROR_CHECK_ENV_FAILED;

    if (op == "add")
        return addRemote(CONFIG_PATH, d->m_options->m_repoName, d->m_options->m_repoUrl).code;
    else if (op == "remove")
        return removeRemote(CONFIG_PATH, d->m_options->m_repoName).code;
    else if (op == "modify")
        return modifyRemoteUrl(CONFIG_PATH, d->m_options->m_repoName, d->m_options->m_repoUrl).code;
    else if (op == "list")
    {
        std::vector<RemoteRepo> repos;
        auto res = listRemotes(CONFIG_PATH, repos);
        if (res.success)
        {
            std::cout << "远程仓库列表:" << "\n";
            for (auto &r : repos)
                std::cout << "  " << r.name << " -> " << r.url << "\n";
        }
        return res.code;
    }
    else if (op == "show")
    {
        OstreeConfig cfg;
        auto res = parseConfig(CONFIG_PATH, cfg);
        if (res.success)
        {
            std::cout << "[core]" << "\n";
            std::cout << "repo_version=" << cfg.repoVersion << "\n";
            std::cout << "mode=" << cfg.mode << "\n";
            std::cout << "min-free-space-size=" << cfg.minFreeSpaceSize << "\n";
            std::cout << "\n";

            for (auto &r : cfg.remoteRepos)
            {
                std::cout << "[remote \"" << r.name << "\"]" << "\n";
                std::cout << "url=" << r.url << "\n";
                std::cout << "gpg-verify=" << (r.gpgVerify ? "true" : "false") << "\n";
                std::cout << "gpg-verify-summary=" << (r.gpgVerifySummary ? "true" : "false") << "\n";
                std::cout << "\n";
            }
        }
        return res.code;
    }
    else if (op == "set-core")
        return modifyCoreConfig(CONFIG_PATH, d->m_options->m_repoVersion,
                                d->m_options->m_mode, d->m_options->m_minFreeSpace)
            .code;

    std::cerr << "Unknown operation: " << op << "\n";
    return KM_ERROR_INVALID_ARGS;
}

void KMRemoteModify::Options::checkUnknownOptions(int argc, char **argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
        "--repo", "--repo-version", "--mode", "--min-free-space", "--config"};

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
                std::cerr << "Unrecognized option: " << arg << std::endl;
                std::cerr << "Use 'kaiming remote --help' for usage." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

static bool readFileLines(const std::string &path, std::vector<std::string> &lines)
{
    std::ifstream inFile(path);
    if (!inFile)
        return false;
    std::string line;
    while (std::getline(inFile, line))
        lines.push_back(line);
    return true;
}

static bool writeFileLines(const std::string &path, const std::vector<std::string> &lines)
{
    std::ofstream outFile(path);
    if (!outFile)
        return false;
    for (const auto &l : lines)
        outFile << l << "\n";
    return true;
}

KMRemoteModify::ModifyResult KMRemoteModify::addRemote(
    const std::string &configPath,
    const std::string &repoName,
    const std::string &repoUrl,
    bool gpgVerify,
    bool gpgVerifySummary)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    for (auto &l : lines)
    {
        if (l.find("[remote \"" + repoName + "\"]") != std::string::npos)
            return {false, "仓库已存在", KM_ERROR_REMOTE_ALREADY_EXISTS};
    }

    lines.push_back("");
    lines.push_back("[remote \"" + repoName + "\"]");
    lines.push_back("gpg-verify=" + std::string(gpgVerify ? "true" : "false"));
    lines.push_back("gpg-verify-summary=" + std::string(gpgVerifySummary ? "true" : "false"));
    lines.push_back("url=" + repoUrl);

    if (!writeFileLines(configPath, lines))
        return {false, "写入配置失败", KM_ERROR_CONFIG_WRITE_FAILED};

    return {true, "添加成功", KM_ERROR_NO};
}

KMRemoteModify::ModifyResult KMRemoteModify::removeRemote(
    const std::string &configPath,
    const std::string &repoName)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    std::vector<std::string> newLines;
    bool inTarget = false, removed = false;

    for (auto &l : lines)
    {
        if (l.find("[remote \"" + repoName + "\"]") != std::string::npos)
        {
            inTarget = true;
            removed = true;
            continue;
        }
        if (inTarget && l.find('[') == 0)
            inTarget = false;
        if (!inTarget)
            newLines.push_back(l);
    }

    if (!removed)
        return {false, "未找到指定仓库", KM_ERROR_REMOTE_NOT_FOUND};
    if (!writeFileLines(configPath, newLines))
        return {false, "写入配置失败", KM_ERROR_CONFIG_WRITE_FAILED};
    return {true, "删除成功", KM_ERROR_NO};
}

KMRemoteModify::ModifyResult KMRemoteModify::modifyRemoteUrl(
    const std::string &configPath,
    const std::string &repoName,
    const std::string &newUrl)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    bool inTarget = false, modified = false;
    for (auto &l : lines)
    {
        if (l.find("[remote \"" + repoName + "\"]") != std::string::npos)
        {
            inTarget = true;
            continue;
        }
        if (inTarget && l.find("url=") == 0)
        {
            l = "url=" + newUrl;
            modified = true;
            inTarget = false;
        }
    }

    if (!modified)
        return {false, "未找到目标仓库或未修改", KM_ERROR_REMOTE_NOT_FOUND};
    if (!writeFileLines(configPath, lines))
        return {false, "写入配置失败", KM_ERROR_CONFIG_WRITE_FAILED};
    return {true, "修改成功", KM_ERROR_NO};
}

KMRemoteModify::ModifyResult KMRemoteModify::listRemotes(
    const std::string &configPath,
    std::vector<RemoteRepo> &repos)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    RemoteRepo cur;
    bool inRemote = false;
    for (auto &l : lines)
    {
        if (l.rfind("[remote \"", 0) == 0)
        {
            if (inRemote)
                repos.push_back(cur);
            cur = RemoteRepo();
            size_t s = l.find('"') + 1;
            size_t e = l.find_last_of('"');
            cur.name = l.substr(s, e - s);
            inRemote = true;
        }
        else if (inRemote && l.find("url=") == 0)
            cur.url = l.substr(4);
        else if (inRemote && l.find("gpg-verify=") == 0)
            cur.gpgVerify = (l.find("true") != std::string::npos);
        else if (inRemote && l.find("gpg-verify-summary=") == 0)
            cur.gpgVerifySummary = (l.find("true") != std::string::npos);
    }
    if (inRemote)
        repos.push_back(cur);
    return {true, "成功", KM_ERROR_NO};
}

KMRemoteModify::ModifyResult KMRemoteModify::modifyCoreConfig(
    const std::string &configPath,
    const std::string &repoVersion,
    const std::string &mode,
    const std::string &minFreeSpace)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    bool inCore = false;
    for (auto &l : lines)
    {
        if (l.find("[core]") != std::string::npos)
        {
            inCore = true;
            continue;
        }
        if (inCore)
        {
            if (l.find("repo_version=") == 0 && !repoVersion.empty())
                l = "repo_version=" + repoVersion;
            else if (l.find("mode=") == 0 && !mode.empty())
                l = "mode=" + mode;
            else if (l.find("min-free-space-size=") == 0 && !minFreeSpace.empty())
                l = "min-free-space-size=" + minFreeSpace;
            else if (l.find('[') == 0)
                inCore = false;
        }
    }

    if (!writeFileLines(configPath, lines))
        return {false, "写入配置失败", KM_ERROR_CONFIG_WRITE_FAILED};
    return {true, "修改核心配置成功", KM_ERROR_NO};
}

KMRemoteModify::ModifyResult KMRemoteModify::parseConfig(
    const std::string &configPath,
    OstreeConfig &cfg)
{
    std::vector<std::string> lines;
    if (!readFileLines(configPath, lines))
        return {false, "无法读取配置文件", KM_ERROR_CONFIG_NOT_FOUND};

    bool inCore = false, inRemote = false;
    RemoteRepo cur;

    for (auto &l : lines)
    {
        if (l.find("[core]") != std::string::npos)
        {
            inCore = true;
            inRemote = false;
        }
        else if (l.rfind("[remote \"", 0) == 0)
        {
            if (inRemote)
                cfg.remoteRepos.push_back(cur);
            inRemote = true;
            inCore = false;
            cur = RemoteRepo();
            size_t s = l.find('"') + 1;
            size_t e = l.find_last_of('"');
            cur.name = l.substr(s, e - s);
        }
        else if (inCore)
        {
            if (l.find("repo_version=") == 0)
                cfg.repoVersion = l.substr(13);
            else if (l.find("mode=") == 0)
                cfg.mode = l.substr(5);
            else if (l.find("min-free-space-size=") == 0)
                cfg.minFreeSpaceSize = l.substr(20);
        }
        else if (inRemote)
        {
            if (l.find("url=") == 0)
                cur.url = l.substr(4);
            else if (l.find("gpg-verify=") == 0)
                cur.gpgVerify = (l.find("true") != std::string::npos);
            else if (l.find("gpg-verify-summary=") == 0)
                cur.gpgVerifySummary = (l.find("true") != std::string::npos);
        }
    }
    if (inRemote)
        cfg.remoteRepos.push_back(cur);
    return {true, "解析成功", KM_ERROR_NO};
}
