/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMExecCommand.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMProcessUtils.h"
#include "common/kmtranslation.h"
#include "KMBuilderUtils.h"

namespace fs = std::filesystem;

class KMExecCommand::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();

public:
    bool m_help = false;

    std::string m_exec;
    std::string m_splitOfArgs;
    std::vector<std::string> m_args;
    std::string m_cwd;
};

void KMExecCommand::Options::preParseHook()
{
    addOptions();
}

void KMExecCommand::Options::postParseHook()
{
    if (m_help)
    {
        // showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_splitOfArgs.empty())
    {
        if (!m_exec.empty())
        {
            // kaiming-builder exec [OPTION…] COMMAND -- ARGS ...
            int posCommand = getPositionalOptionIndex("COMMAND");
            int steps = m_splitOfArgs.empty() ? 1 : 2;
            if (posCommand > 0 && m_originalArgs.size() > posCommand + steps)
            {
                for (int i = posCommand + steps; i < m_originalArgs.size(); i++)
                {
                    m_args.push_back(m_originalArgs.at(i));
                }
            }
        }
    }

    if (m_exec.empty())
    {
        // showUsage();
        exit(EXIT_FAILURE);
    }
}

void KMExecCommand::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming-builder exec [OPTIONS] COMMAND -- ARGS ..."
                     "\nExample:\n \tkaiming-builder exec ls -- al\n"));

    //帮助选项
    addOption("help","h", KMOption::value<bool>(&m_help), _("Show help options"));

    //应用选项
    addOption("workdirectory", "",KMOption::value(&m_cwd), _("The workdirectory of COMMAND"));

    //位置选项
    addPositionOption("COMMAND", KMOption::value<std::string>(&m_exec), 1, _("The command to exec"));
    addPositionOption("--", KMOption::value(&m_splitOfArgs), 1, _("The start flag of command's arguments"));
    addPositionOption("ARGS", KMOption::value<std::vector<std::string>>(&m_args), -1, _("The args of command"));
}

class KMExecCommand::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

};

REGISTER_SUBCOMMAND_DYNCREATE(exec, KMExecCommand)

KMExecCommand::KMExecCommand()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMExecCommand::~KMExecCommand()
{}

int KMExecCommand::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMExecCommand::init(int argc, char **argv)
{
    // 参数解析
    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMExecCommand::run()
{
    if (!KMProcessUtils::spawn(d->m_kmOptions->m_exec, d->m_kmOptions->m_cwd, d->m_kmOptions->m_args, false))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

