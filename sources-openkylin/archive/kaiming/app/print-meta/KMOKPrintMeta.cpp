/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOKPrintMeta.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMOABMetadata.h"
#include "common/KMOKFile.h"
#include "common/kmtranslation.h"

namespace fs = std::filesystem;

class KMOKPrintMeta::Options : public KMOption::Options
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
    std::string m_oabPackage;
};

void KMOKPrintMeta::Options::preParseHook()
{
    addOptions();
}

void KMOKPrintMeta::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_oabPackage.empty())
    {
        throw KMException("OK-PACKAGE must be specified");
    }

    fs::path app(m_oabPackage);
    if (!fs::exists(app))
    {
        throw KMException("OK-PACKAGE must exists.");
    }
}

void KMOKPrintMeta::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming print-meta OK-PACKAGE\n"));

    addOption("help", "h", KMOption::value(&m_help), _("Show help options"));

    addPositionOption("OK-PACKAGE", KMOption::value(&m_oabPackage), 1, "OK Package.");
}

class KMOKPrintMeta::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
};

REGISTER_SUBCOMMAND_DYNCREATE(print-meta, KMOKPrintMeta)

KMOKPrintMeta::KMOKPrintMeta()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMOKPrintMeta::~KMOKPrintMeta()
{
}

int KMOKPrintMeta::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOKPrintMeta::init(int argc, char **argv)
{
    // 只能输出错误级别日志，因为本身结果要输出到终端
    kmlogger.setLogLevel(KMLogLevel::err);

    d->m_kmOptions->parseCommandLine(argc, argv);
    d->m_appFile = fs::canonical(d->m_kmOptions->m_oabPackage);
}

int KMOKPrintMeta::run()
{
    KMOKFile elf;
    if (!elf.loadOKFile(d->m_appFile))
    {
        return EXIT_FAILURE;
    }

    const std::string& metaInfo = elf.getMetaInfo();
    std::cout << metaInfo << std::endl;

    elf.closeOKFile();

    if (metaInfo.empty())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
