
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABMount.h"

#include <iostream>
#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMStringUtils.h"
#include "KMOABContext.h"
#include "KMOABElf.h"

namespace fs = std::filesystem;

class KMOABMount::Options : public KMOption::Options
{
public:
    Options() = default;
    virtual ~Options() = default;

protected:
    void preParseHook();
    void postParseHook();

private:
    void addOptions();

public:
    bool m_help = false;
    std::string m_directory;

    std::string m_baseName; // 去掉路径和扩展名
};

void KMOABMount::Options::preParseHook()
{
    addOptions();
}

void KMOABMount::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_directory.empty())
    {
        throw KMException("DIRECTORY must be specified");
    }
    m_directory = fs::absolute(m_directory);
}

void KMOABMount::Options::addOptions()
{
    setDescription("\nUsage:\n \t" + KMOABContext::instance().appName() + " mount [DIRECTORY] \n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");

    addPositionOption("DIRECTORY", KMOption::value(&m_directory), 1, "Mount to this directory.");
}

class KMOABMount::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
};

REGISTER_SUBCOMMAND_DYNCREATE(mount, KMOABMount)

KMOABMount::KMOABMount()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_appFile = fs::canonical(KMOABContext::instance().appName());
}

KMOABMount::~KMOABMount()
{
}

int KMOABMount::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOABMount::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMOABMount::run()
{
    KMOABElf elf(d->m_appFile);
    if (!elf.open())
    {
        throw KMException(elf.errorMsg());
    }

    if (!elf.mountBundle(d->m_kmOptions->m_directory, false))
    {
        throw KMException(elf.errorMsg());
    }

    return EXIT_SUCCESS;
}

