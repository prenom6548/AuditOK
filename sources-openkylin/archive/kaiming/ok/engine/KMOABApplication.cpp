/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABApplication.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/KMLogger.h"
#include "KMOABContext.h"

class KMOABApplication::Options : public KMOption::Options
{
public:
    Options() = default;
    virtual ~Options() = default;

    void showUsage() const;

protected:
    void preParseHook();
    void postParseHook();

private:
    void addOptions();

public:
    bool m_verbose = false;
    bool m_help = false;
    bool m_printMeta = false;

    std::string m_appName;
    std::string m_subcmd;
    std::string m_loglevel;
    std::string m_extractPath;
    std::string m_mountPath;
};

void KMOABApplication::Options::preParseHook()
{
    addOptions();
}

void KMOABApplication::Options::postParseHook()
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

void KMOABApplication::Options::addOptions()
{
    m_appName = m_originalArgs.at(0);
    setDescription("\nUsage:\n \t" + m_appName + " [SUBCOMMAND] [OPTION...] \n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");
    addOption("loglevel","", KMOption::value<std::string>(&m_loglevel)->defaultValue("info"), "loglevel: trace, debug, info, warn, error, critical, off", true);
    addOption("verbose", "v", KMOption::value(&m_verbose), "Show debug information");
    addOption("print-meta", "", KMOption::value(&m_printMeta), "Show metadata information. Same to sumcommand : print-meta.");
    addOption("oab-extract", "", KMOption::value(&m_extractPath), "--oab-extract=DIRECTORY. Extract to the specified directory. Same to sumcommand : extract.");
    addOption("oab-mount", "", KMOption::value(&m_mountPath), "--oab-mount=DIRECTORY. Mount to the specified directory. Same to sumcommand : mount.");

    addPositionOption("SUBCOMMAND", KMOption::value(&m_subcmd), 1, "Subcommand of offline package.");
}

void KMOABApplication::Options::showUsage() const
{
    KMOption::Options::showUsage();

    std::cout << "Subcommands:" << std::endl;
    for (auto entry : KMSubCommandFactory::subCommandDescriptions())
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

class KMOABApplication::Private
{
public:
    int m_argc;
    char **m_argv;
    std::vector<char *> m_args; // 去掉了本程序名

    std::unique_ptr<Options> m_okOptions;
};

KMOABApplication::KMOABApplication(token, int argc, char **argv)
    : d(std::make_unique<Private>())
{
    reset(argc, argv);
}

KMOABApplication::~KMOABApplication() { }

void KMOABApplication::reset(int argc, char **argv)
{
    d->m_argc = argc;
    d->m_argv = argv;
    d->m_okOptions = std::make_unique<Options>();

    d->m_args.clear();
    d->m_args.insert(d->m_args.end(), argv+1, argv+argc);
}

void KMOABApplication::init()
{
    KMSubCommandFactory::setSubCommandDescriptions({
        { "data-dir", "Print the app's data directory." },
        { "extract", "Extract to the specified directory." },
        { "mount", "Mount to the specified directory." },
        { "print-meta", "Print the metadata info." },
        { "repackage", "Repackage the offline package. Used for repackaging after extract." },
    });

    d->m_okOptions->parseCommandLine(d->m_argc, d->m_argv);

    KMOABContext::instance().setAppName(d->m_okOptions->m_appName);

    KMLogger::instance().setLogLevel(d->m_okOptions->m_loglevel);

    if (d->m_okOptions->m_subcmd.empty())
    {
        if (d->m_okOptions->m_printMeta)
        {
            d->m_okOptions->m_subcmd = "print-meta";
            d->m_args.insert(d->m_args.begin(), d->m_okOptions->m_subcmd.data());
        }
        else if (!d->m_okOptions->m_extractPath.empty())
        {
            d->m_okOptions->m_subcmd = "extract";
            d->m_args.insert(d->m_args.begin(), d->m_okOptions->m_subcmd.data());
            d->m_args.push_back(d->m_okOptions->m_extractPath.data());
        }
        else if (!d->m_okOptions->m_mountPath.empty())
        {
            d->m_okOptions->m_subcmd = "mount";
            d->m_args.insert(d->m_args.begin(), d->m_okOptions->m_subcmd.data());
            d->m_args.push_back(d->m_okOptions->m_mountPath.data());
        }
    }
}

int KMOABApplication::exec()
{
    int ret = EXIT_FAILURE;

    std::unique_ptr<KMSubCommand> subCommand = KMSubCommandFactory::createKMSubCommand(d->m_okOptions->m_subcmd);
    if (subCommand)
    {
        try
        {
            char ** argv = d->m_args.data();
            int argc = d->m_args.size();
            ret = subCommand->dispose(argc, argv);
        }
        catch (const std::exception &e)
        {
            std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << "Unknowned exception" << std::endl;
        }
    }
    else
    {
        std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << "Unsupported features" << std::endl;
        d->m_okOptions->showUsage();
    }

    return ret;
}
