
/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABExtract.h"

#include <iostream>
#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMStringUtils.h"
#include "KMOABContext.h"
#include "KMOABElf.h"
#include "KMOABUtils.h"

namespace fs = std::filesystem;

class KMOABExtract::Options : public KMOption::Options
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

void KMOABExtract::Options::preParseHook()
{
    addOptions();
}

void KMOABExtract::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    fs::path app(KMOABContext::instance().appName());
    std::string baseName = app.filename().string();
    std::size_t pos = baseName.find_last_of('.');
    if (pos != std::string::npos)
    {
        baseName = baseName.substr(0, pos);
    }
    m_baseName = baseName;

    if (m_directory.empty())
    {
        m_directory = fs::current_path();
    }
    m_directory = fs::absolute(m_directory);
}

void KMOABExtract::Options::addOptions()
{
    setDescription("\nUsage:\n \t" + KMOABContext::instance().appName() + " extract [DIRECTORY] \n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");

    addPositionOption("DIRECTORY", KMOption::value(&m_directory), 1, "Extract to this directory.");
}

class KMOABExtract::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
    std::string m_mountDirPrefix;
};

REGISTER_SUBCOMMAND_DYNCREATE(extract, KMOABExtract)

KMOABExtract::KMOABExtract()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_appFile = fs::canonical(KMOABContext::instance().appName());
    d->m_mountDirPrefix = "/tmp/.ok/mountpoint/";
}

KMOABExtract::~KMOABExtract()
{
}

int KMOABExtract::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOABExtract::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMOABExtract::run()
{
    KMOABElf elf(d->m_appFile);
    if (!elf.open())
    {
        throw KMException(elf.errorMsg());
    }

    fs::path appfile(d->m_appFile);
    std::string mountPoint = d->m_mountDirPrefix + appfile.filename().string() + "/" + KMOABUtils::generateUUID();

    if (!KMOABUtils::spawn(d->m_appFile, { "mount", mountPoint }))
    {
        throw KMException("Failed to mount " + d->m_appFile);
    }

    std::string dest = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, d->m_kmOptions->m_baseName);
    KMOABUtils::mkpath(dest);
    std::string destPath = KMOABUtils::canonical(dest);

    if (!KMOABUtils::isEmptyDirectory(destPath))
    {
        KMOABElf::umountBundle(mountPoint);
        KMOABUtils::rmRfAll(mountPoint);
        throw KMException(destPath + " isn't empty");
    }

    std::vector<std::string> args;
    args.push_back("-rlptHv");
    args.push_back("--ignore-errors");
    args.push_back(mountPoint + "/");
    args.push_back(destPath + "/");

    if (!KMOABUtils::spawn("rsync", args, true))
    {
        KMOABElf::umountBundle(mountPoint);
        KMOABUtils::rmRfAll(mountPoint);
        throw KMException("Failed to rsync bundle to " + destPath);
    }

    KMOABUtils::spawn("sync");
    KMOABElf::umountBundle(mountPoint);
    KMOABUtils::rmRfAll(mountPoint);

    return EXIT_SUCCESS;
}

