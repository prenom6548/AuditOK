/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMApplication.h"

#include <iomanip>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include "common/KMLogger.h"
#include "KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "config.h"

#define GETTEXT_PACKAGE "kaiming-builder"

class KMApplication::Options : public KMOption::Options
{
public:
    Options() = default;
    virtual ~Options() = default;

    const std::string &logLevel() const { return m_loglevel; }

    const std::string &subCommand() const { return m_subcmd; }

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
    else if (getPositionalOptionIndex() != 1 && !m_subcmd.empty())
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_verbose)
    {
        m_loglevel = "debug";
    }
    else
    {
        m_loglevel = "info";
    }
}

void KMApplication::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming-builder [SUBCOMMAND] [OPTION…] [ARGMENTS] \n"));

    // 选项参数
    addOption("help","h", KMOption::value(&m_help), _("Show help options"));
    addOption("verbose","v", KMOption::value(&m_verbose), _("Show debug information"));
    addOption("loglevel","", KMOption::value(&m_loglevel)->defaultValue("info"), _("loglevel: trace, debug, info, warn, error, critical, off"), true);
    
    //位置参数
    addPositionOption(_("SUBCOMMAND"), KMOption::value<std::string>(&m_subcmd),1, _("Subcommand of kaiming-builder, usage: kaiming-builder build ..."));
    addPositionOption(_("ARGMENTS"), KMOption::value<std::vector<std::string>>(&m_othersPositionalOptions), -1 ,_("Other args"));
}

void KMApplication::Options::showUsage() const
{
    KMOption::Options::showUsage();

    std::cout << _("Subcommands:") << std::endl;
    for (auto entry : KMSubCommandFactory::subCommandEntrys())
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

KMApplication::~KMApplication() { }

void KMApplication::init()
{
    std::locale::global(std::locale(""));
    kmI18nInit(GETTEXT_PACKAGE, LOCALEDIR);

    d->m_kmOptions->parseCommandLine(d->m_argc, d->m_argv);

    KMLogger::instance().setLogLevel(d->m_kmOptions->logLevel());
}

int KMApplication::exec()
{
    init();

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
            KMError("Unknowned exception");
        }
    }
    else
    {
        KMError(std::string("Unknowned subcommand : ").append(d->m_kmOptions->subCommand()));
    }

    return ret;
}
