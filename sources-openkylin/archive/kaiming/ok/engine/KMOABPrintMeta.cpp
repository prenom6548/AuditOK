/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABPrintMeta.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMOABMetadata.h"
#include "KMOABContext.h"
#include "KMOABElf.h"

namespace fs = std::filesystem;

class KMOABPrintMeta::Options : public KMOption::Options
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

void KMOABPrintMeta::Options::preParseHook()
{
    addOptions();
}

void KMOABPrintMeta::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}

void KMOABPrintMeta::Options::addOptions()
{
    setDescription("\nUsage:\n \t" + KMOABContext::instance().appName() + " print-meta\n");

    addOption("help", "h", KMOption::value(&m_help), "Show help options");
}

class KMOABPrintMeta::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
};

REGISTER_SUBCOMMAND_DYNCREATE(print-meta, KMOABPrintMeta)

KMOABPrintMeta::KMOABPrintMeta()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_appFile = fs::canonical(KMOABContext::instance().appName());
}

KMOABPrintMeta::~KMOABPrintMeta()
{
}

int KMOABPrintMeta::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOABPrintMeta::init(int argc, char **argv)
{
    // 只能输出错误级别日志，因为本身结果要输出到终端
    kmlogger.setLogLevel(KMLogLevel::err);

    d->m_kmOptions->parseCommandLine(argc, argv);
}

int KMOABPrintMeta::run()
{
    KMOABElf elf(d->m_appFile);
    if (!elf.open())
    {
        throw KMException(elf.errorMsg());
    }

    std::string metaInfo = elf.metaContent();
    std::cout << metaInfo << std::endl;

    return EXIT_SUCCESS;
}
