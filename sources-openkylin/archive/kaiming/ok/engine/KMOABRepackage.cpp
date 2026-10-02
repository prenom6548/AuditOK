/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABRepackage.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMStringUtils.h"
#include "KMOABContext.h"
#include "KMOABElf.h"

namespace fs = std::filesystem;

class KMOABRepackage::Options : public KMOption::Options
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
    std::string m_bundleFilesystem;
    std::string m_directory;
};

void KMOABRepackage::Options::preParseHook()
{
    addOptions();
}

void KMOABRepackage::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_directory.empty())
    {
        m_directory = fs::current_path();
    }

    fs::path app(KMOABContext::instance().appName());
    std::string baseName = app.filename().string();
    std::size_t pos = baseName.find_last_of('.');
    if (pos != std::string::npos)
    {
        baseName = baseName.substr(0, pos);
    }
    m_directory = KMStringUtils::buildFilename(m_directory, baseName);

    if (!fs::exists(m_directory))
    {
        KMError(m_directory + " does't exists");
        exit(EXIT_SUCCESS);
    }

    if (fs::is_empty(m_directory))
    {
        KMError(m_directory + " is empty");
        exit(EXIT_SUCCESS);
    }

    m_directory = fs::canonical(m_directory);
}

void KMOABRepackage::Options::addOptions()
{
    setDescription("\nUsage:\n \t" + KMOABContext::instance().appName() + " repackage [DIRECTORY] \n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");
    addOption("bundle-filesystem","", KMOption::value(&m_bundleFilesystem), "The filesystem for bundle. Such as squashfs、erofs.");

    addPositionOption("DIRECTORY", KMOption::value(&m_directory), 1, "Repackage from this directory.");
}

class KMOABRepackage::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
    std::string m_cacheDir;
};

REGISTER_SUBCOMMAND_DYNCREATE(repackage, KMOABRepackage)

KMOABRepackage::KMOABRepackage()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_appFile = fs::canonical(KMOABContext::instance().appName());
}

KMOABRepackage::~KMOABRepackage()
{
}

int KMOABRepackage::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOABRepackage::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMOABRepackage::run()
{
    KMOABElf elf(d->m_appFile);
    if (!elf.open())
    {
        throw KMException(elf.errorMsg());
    }

    if (!elf.repackageBundle(d->m_kmOptions->m_directory, d->m_kmOptions->m_bundleFilesystem))
    {
        throw KMException(elf.errorMsg());
    }

    return EXIT_SUCCESS;
}

