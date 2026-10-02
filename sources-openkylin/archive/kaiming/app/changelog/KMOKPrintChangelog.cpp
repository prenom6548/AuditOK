/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOKPrintChangelog.h"

#include <filesystem>

#include "common/KMBuildinOptions.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMOABMetadata.h"
#include "common/KMOKFile.h"
#include "common/kmtranslation.h"
#include "common/KMProcessUtils.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"

namespace fs = std::filesystem;

class KMOKPrintChangelog::Options : public KMOption::Options
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

void KMOKPrintChangelog::Options::preParseHook()
{
    addOptions();
}

void KMOKPrintChangelog::Options::postParseHook()
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

    m_oabPackage = fs::absolute(m_oabPackage);
}

void KMOKPrintChangelog::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming changelog OK-PACKAGE\n"));

    addOption("help", "h", KMOption::value(&m_help), _("Show help options"));

    addPositionOption("OK-PACKAGE", KMOption::value(&m_oabPackage), 1, "OK Package.");
}

class KMOKPrintChangelog::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_appFile;
    std::string m_mountPointPrefix;
};

REGISTER_SUBCOMMAND_DYNCREATE(changelog, KMOKPrintChangelog)

KMOKPrintChangelog::KMOKPrintChangelog()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
    d->m_mountPointPrefix = "/tmp/.ok/kaiming/mountpoint/";
}

KMOKPrintChangelog::~KMOKPrintChangelog()
{
}

int KMOKPrintChangelog::dispose(int argc, char **argv)
{
    init(argc, argv);

    return run();
}

void KMOKPrintChangelog::init(int argc, char **argv)
{
    // 只能输出错误级别日志，因为本身结果要输出到终端
    kmlogger.setLogLevel(KMLogLevel::err);

    d->m_kmOptions->parseCommandLine(argc, argv);
    d->m_appFile = fs::canonical(d->m_kmOptions->m_oabPackage);
}

int KMOKPrintChangelog::run()
{
    KMOKFile elf;
    if (!elf.loadOKFile(d->m_appFile))
    {
        return EXIT_FAILURE;
    }

    const std::string& metaInfo = elf.getMetaInfo();
    if (metaInfo.empty())
    {
        elf.closeOKFile();
        return EXIT_FAILURE;
    }
    KMOABMetadata meta;
    meta.parse(metaInfo);

    std::string mountPonit = d->m_mountPointPrefix + meta.uuid + "/" + KMStringUtils::generateUUID();
    KMFileUtils::mkpath(mountPonit);
    if (!elf.mountBundle(mountPonit))
    {
        elf.closeOKFile();
        fs::remove(mountPonit);
        return EXIT_FAILURE;
    }

    bool hasChangelog = false;
    for (auto& layer : meta.layers)
    {
        std::string changelog = mountPonit + "/layers/" + layer.id + "/" + layer.module + "/KAIMING/changelog";
        if (fs::exists(changelog))
        {
            hasChangelog = true;

            std::vector<std::string> args;
            args.push_back(changelog);
            KMProcessUtils::spawn("cat", args);
        }
    }

    elf.umountBundle(mountPonit);
    elf.closeOKFile();

    if (!hasChangelog)
    {
        throw KMException("Not found KAIMING/changelog");
    }

    fs::remove(mountPonit);

    return EXIT_SUCCESS;
}
