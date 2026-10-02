/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABPrintDataDir.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMOABMetadata.h"
#include "KMOABContext.h"
#include "KMOABElf.h"

namespace fs = std::filesystem;

class KMOABPrintDataDir::Options : public KMOption::Options
{
public:
    Options() = default;
    virtual ~Options() = default;

protected:
    void preParseHook();
    void postParseHook();

private:
    void addOptions();

private:
    bool m_help = false;
};

void KMOABPrintDataDir::Options::preParseHook()
{
    addOptions();
}

void KMOABPrintDataDir::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

void KMOABPrintDataDir::Options::addOptions()
{
    setDescription("\nUsage:\n \t" + KMOABContext::instance().appName() + " data-dir\n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");
}

class KMOABPrintDataDir::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
};

REGISTER_SUBCOMMAND_DYNCREATE(data-dir, KMOABPrintDataDir)

KMOABPrintDataDir::KMOABPrintDataDir()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_appFile = fs::canonical(KMOABContext::instance().appName());
}

KMOABPrintDataDir::~KMOABPrintDataDir()
{
}

int KMOABPrintDataDir::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOABPrintDataDir::init(int argc, char **argv)
{
    // 只能输出错误级别日志，因为本身结果要输出到终端
    kmlogger.setLogLevel(KMLogLevel::err);

    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMOABPrintDataDir::run()
{
    KMOABElf elf(d->m_appFile);
    if (!elf.open())
    {
        throw KMException(elf.errorMsg());
    }

    std::string appDataDir = elf.appDataDir();
    if (appDataDir.empty())
    {
        throw KMException("When there are multiple layers, only output the path of the app is supported");
    }

    fs::path app(KMOABContext::instance().appName());
    std::string baseName = app.filename().string();
    std::size_t pos = baseName.find_last_of('.');
    if (pos != std::string::npos)
    {
        baseName = baseName.substr(0, pos);
    }
    
    std::cout << appDataDir << std::endl;

    return EXIT_SUCCESS;
}
