/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMApplication.h"

#include <iomanip>
#include <iostream>
#include <locale>
#include <string>
#include <vector>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "common/KMSubCommand.h"
#include "common/KMInstalledAppQuery.h"

namespace fs = std::filesystem;

class KMApplication::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

    const std::string &logLevel() const
    {
        return m_loglevel;
    }

    const std::string &subCommand() const
    {
        return m_subcmd;
    }

protected:
    void preParseHook() override;
    void postParseHook() override;
    void showUsage() const override;

private:
    void addOptions();

private:
    std::string m_programName;
    std::string m_subcmd;

    bool m_help = false;
    bool m_verbose = false;
    std::string m_loglevel;
    std::vector<std::string> m_othersPositionalOptions;
};

void KMApplication::Options::preParseHook()
{
    addOptions();
}

void KMApplication::Options::postParseHook()
{
    if (m_help && m_subcmd.empty())
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_verbose)
    {
        m_loglevel = "debug";
    }
}

void KMApplication::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming [SUBCOMMAND] [OPTION...] [ARGMENTS]\n"));

    //选项参数
    addOption("help", "h", KMOption::value(&m_help)->defaultValue(false), _("Show help options"));
    addOption("loglevel", "", KMOption::value(&m_loglevel)->defaultValue("info"), _("loglevel: trace, debug, info, warn, error, critical, off"), true);
    addOption("verbose", "v", KMOption::value(&m_verbose), _("Show debug information"));

    //位置参数
    addPositionOption("SUBCOMMAND", KMOption::value(&m_subcmd), 1, _("Subcommand of kaiming, such as 'install', usage: kaiming install ..."));
    addPositionOption("ARGMENTS", KMOption::value(&m_othersPositionalOptions), -1, _("SUBCOMMAND args"));
}

void KMApplication::Options::showUsage() const
{
    KMOption::Options::showUsage();

    std::cout << _("Subcommands:") << std::endl;
    for (auto &entry : KMSubCommandFactory::subCommandDescriptions())
    {
        if (entry.m_name.empty())
        {
            std::cout << entry.m_description << std::endl;
        }
        else
        {
            std::cout << "  " << std::left << std::setw(24) << entry.m_name << entry.m_description << std::endl;
        }
    }
    std::cout << std::endl;
}

class KMApplication::Private
{
public:
    int m_argc;
    char **m_argv;

    std::unique_ptr<Options> m_kmOptions;
};

KMApplication::KMApplication(token, int argc, char **argv)
    : d(std::make_unique<Private>())
{
    d->m_argc = argc;
    d->m_argv = argv;
    d->m_kmOptions = std::make_unique<Options>();
}

KMApplication::~KMApplication() = default;

void KMApplication::init()
{
    /*
        客户端进程和系统助手之间共享的子repo确实需要支持创建其他人可读的文件，因此将umask重写为022。
        理想情况下，这应该在需要时设置，但umask是线程不安全的，所以实际上没有本地方法来解决这个问题。
    */
    mode_t mode = S_IWGRP | S_IWOTH;
    umask(mode);

    std::locale::global(std::locale(""));
    kmI18nInit(GETTEXT_PACKAGE, LOCALEDIR);

    KMSubCommandFactory::setSubCommandDescriptions({ { "", _(" Manage installed applications and runtimes") },
                                                     { "install", _("Install an application or runtime") },
                                                     { "update", _("Update an application or runtime") },
                                                     { "upgrade", _("Update all applications or runtime") },
                                                     { "uninstall", _("Uninstall an application") },
                                                     { "repair", _("Repair an application or runtime") },
                                                     { "info", _("Show info for an app or runtime") },
                                                     { "list", _("Display installed applications or runtime environments") },
                                                     { "search", _("Search for remote package descriptions") },
                                                     { "which", _("Search for which package the specified file belongs to") },
                                                     { "remote", _("Modify remote repo config") },
                                                     { "print-meta", _("Print the ok package's metadata info.") },
                                                     { "changelog", _("Print the ok package's changelog info.") },
                                                     { "compare-versions", _("Compare two versions. Exit status code: 0 indicates established; otherwise indicates not established") },
                                                     { "deltas", _("Build incremental packages") },

                                                     { "", _("\n Manage running applications") },
                                                     { "run", _("Run an application") },
                                                     { "enter", _("Run a command in a running container") },
                                                     { "ps", _("Enumerate running applications") },
                                                     { "kill", _("Stop a running application") },

                                                     { "", _("\n Build applications") },
                                                     { "build-init", _("Initialize a directory for building") },
                                                     { "build", _("Run a build command inside the build dir") },
                                                     { "build-finish", _("Finish a build dir for export") },
                                                     { "build-export", _("Export ok package") } });

    auto *env = getenv("KAIMING_BINARY");
    std::string binary = env ? env : "";
    if (!binary.empty())
    {
        specialHandle(fs::path(binary).filename());
    }

    d->m_kmOptions->parseCommandLine(d->m_argc, d->m_argv);

    KMLogger::instance().setLogLevel(d->m_kmOptions->logLevel());
}

int KMApplication::exec()
{
    int ret = EXIT_FAILURE;

    std::unique_ptr<KMSubCommand> subCommand = KMSubCommandFactory::createKMSubCommand(d->m_kmOptions->subCommand());
    if (subCommand)
    {
        int off = 1;
        try
        {
            ret = subCommand->dispose(d->m_argc - off, d->m_argv + off);
        }
        catch (const std::exception &e)
        {
            KMError(e.what());
        }
        catch (...)
        {
            KMError(_("Unknowned exception"));
        }
    }
    else
    {
        KMError(std::string(_("Unknowned subcommand : ")).append(d->m_kmOptions->subCommand()));
    }

    return ret;
}

/**
  * @brief 处理应用程序调用情况
  * @param fileName 应用程序文件名
  */
void KMApplication::specialHandle(const std::string &fileName)
{
    KMInstalledAppQuery query;
    std::unordered_map<std::string, std::string> unMap = query.getPackageMappingsFromExec(fileName);
    for (auto &[key, value] : unMap)
    {
        std::string appid, command;
        if (!key.empty() && !value.empty())
        {
            appid = value;
            command = std::string("--command=") + key;
            std::vector<std::string> args;
            args.emplace_back(d->m_argv[0]);
            args.emplace_back("run");
            args.emplace_back(command);
            args.emplace_back(appid);
            args.emplace_back("--");
            for (int i = 1; i < d->m_argc; i++)
            {
                args.emplace_back(d->m_argv[i]);
            }
            // 转换未char*[]格式
            static std::vector<std::unique_ptr<char[]>> argvs;
            for (auto &arg : args)
            {
                auto buffer = std::make_unique<char[]>(arg.size() + 1);
                std::copy(arg.c_str(), arg.c_str() + arg.size() + 1, buffer.get());
                argvs.emplace_back(std::move(buffer));
            }
            argvs.emplace_back(nullptr);

            d->m_argc = d->m_argc + 4;
            d->m_argv = (char **)argvs.data();
            break;
        }
        continue;
    }
}
