/*
 * @Author: caoxiansheng caoxiansheng@kylinos.cn
 * @Date: 2025-03-28 09:34:36
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-04-15 15:54:22
 * @FilePath: /kaiming/app/which/KMWhich.cpp
 * @Description: 
 * 
 * Copyright (c) by KylinSoft  Co., Ltd. 2025. All Rights Reserved.
 */

#include "KMWhich.h"
#include <unordered_set>
#include "common/KMError.h"
#include "common/KMLogger.h"
/**
 * @brief 安装子命令的参数解析
 */
class KMWhich::Options : public KMOption::Options
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
    bool m_help;
    vector<string> m_vecRefs;
};

void KMWhich::Options::preParseHook()
{
    addOptions();
}

/**
 * @brief 命令行参数解析后操作
 */
void KMWhich::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief 命令行参数添加描述选项
 */
void KMWhich::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming which - Search for which package the specified file belongs to\n"));
    addOption("help", "h", KMOption::value<bool>(&m_help)->defaultValue(false), _("display help information"));
    addPositionOption("PATH", KMOption::value<std::vector<std::string>>(&m_vecRefs), -1, _("specify the files path"));
}

class KMWhich::Private
{
public:
    std::unique_ptr<Options> m_options;
    std::unique_ptr<KMFolder> m_folder;
};

REGISTER_SUBCOMMAND_DYNCREATE(which, KMWhich)

KMWhich::KMWhich()
    : d(std::make_unique<Private>())
{
    d->m_options = make_unique<Options>();
    d->m_folder = make_unique<KMFolder>("/opt/kaiming/", false);
}

KMWhich::~KMWhich() = default;

int KMWhich::dispose(int argc, char **argv)
{
    vector<string> vecWhich;

    d->m_options->checkUnknownOptions(argc, argv);
    d->m_options->parseCommandLine(argc, argv);

    if (d->m_options->m_vecRefs.empty())
    {
        KMError(_("Missing required argument PATH"));
        std::cerr << _("Use ") << ("'kaiming which --help'") << _(" for usage.") << std::endl;
        return KM_ERROR_INVALID_ARGS;
    }

    vecWhich = d->m_options->m_vecRefs;

    searchPackageByFile(d->m_folder->getInfoPath(), vecWhich.front());

    return 0;
}

void KMWhich::searchPackageByFile(const std::string &infoDir, const std::string &filePath)
{
    bool found = false;

    try
    {
        for (const auto &entry : fs::directory_iterator(infoDir))
        {
            if (entry.path().extension() == ".list" || entry.path().extension() == ".hostlist")
            {
                if (fileExistsInListFile(entry.path(), filePath))
                {
                    std::cout << entry.path().stem().string() + ": " + filePath << "\n";
                    found = true;
                    break;
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        KMError("Filesystem error: " + std::string(e.what()));
    }

    if (!found)
    {
        std::cout << "No package contains the file: " << filePath << "\n";
    }
}

bool KMWhich::fileExistsInListFile(const fs::path &listFilePath, const std::string &searchFilePath)
{
    std::string normalizedSearchPath = normalizePath(searchFilePath);

    std::ifstream file(listFilePath);
    if (!file)
    {
        KMError("Error opening file:  " + listFilePath.string());
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::string normalizedLine = normalizePath(line);
        if (normalizedLine == normalizedSearchPath)
        {
            return true;
        }
    }

    return false;
}

std::string KMWhich::normalizePath(const std::string &path)
{
    fs::path p(path);
    p = p.lexically_normal();

    if (p != "/" && p.string().back() == '/')
    {
        p = p.parent_path();
    }

    return p.string();
}

void KMWhich::Options::checkUnknownOptions(int argc, char** argv)
{
    std::unordered_set<std::string> validOptions = {
        "--help", "-h",
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
            {
                opt = opt.substr(0, eq_pos);
            }

            if (validOptions.find(opt) == validOptions.end()) 
            {
                KMError(_("Unrecognized option “") + arg + "”");
                std::cerr << _("Please use ") << ("'kaiming which --help'") << _(" to see available options.") << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}