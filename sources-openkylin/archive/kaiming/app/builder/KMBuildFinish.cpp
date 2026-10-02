/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildFinish.h"

#include <gio/gio.h>
#include <filesystem>
#include <unordered_set>

#include "common/KMBuildinsUtils.h"
#include "common/KMContext.h"
#include "common/KMException.h"
#include "common/KMLogger.h"
#include "common/KMInfoJson.h"
#include "common/KMBuildinOptions.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/KMFileUtils.h"
#include "common/kmtranslation.h"

namespace fs = std::filesystem;

class KMBuildFinish::Options : public KMOption::Options
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
    // application options
    bool m_help = false;
    bool m_noexports = false;

    // position options
    std::string m_directory;
};

void KMBuildFinish::Options::preParseHook()
{
    addOptions();
}

void KMBuildFinish::Options::postParseHook()
{
    if (m_help)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    if (m_directory.empty())
    {
        KMError(_("DIRECTORY must be specified"));
        showUsage();
        exit(EXIT_FAILURE);
    }

    m_directory = fs::absolute(m_directory);
}

void KMBuildFinish::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming build-finish [OPTION…] DIRECTORY \nExample:\n \tkaiming build-finish /home/kylin/workspace/qtdemo/build\n"));

    addOption("help","h", KMOption::value<bool>(&m_help), _("Show help options"));
    addOption("noexports","", KMOption::value<bool>(&m_noexports), _("Don't export entries"), true);
    addPositionOption("DIRECTORY", KMOption::value<std::string>(&m_directory), 1, _("The build dir"));
}

class KMBuildFinish::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_baseDir;
    std::string m_filesDir;
    std::string m_exportDir;
    std::string m_metadataFile;

    std::shared_ptr<KMInfoJson> m_infoJson;
    std::string m_id;
};

REGISTER_SUBCOMMAND_DYNCREATE(build-finish, KMBuildFinish)

KMBuildFinish::KMBuildFinish()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuildFinish::~KMBuildFinish() = default;

int KMBuildFinish::dispose(int argc, char **argv)
{
    KMTrace("KMBuildFinish::dispose invoke begin");

    init(argc, argv);

    int ret = run();

    KMTrace("KMBuildFinish::dispose invoke end");
    return ret;
}

void KMBuildFinish::init(int argc, char **argv)
{
    KMTrace("KMBuildFinish::init invoke begin");

    d->m_kmOptions->parseCommandLine(argc, argv);

    d->m_baseDir = d->m_kmOptions->m_directory;
    d->m_filesDir = KMStringUtils::buildFilename(d->m_baseDir, "files");
    d->m_exportDir = KMStringUtils::buildFilename(d->m_baseDir, "entries");
    d->m_metadataFile = KMStringUtils::buildFilename(d->m_baseDir, "info.json");

    if (!fs::exists(d->m_filesDir) || !fs::exists(d->m_metadataFile))
    {
        throw KMException(d->m_baseDir + _(" directory not initialized"));
    }

    d->m_infoJson = std::make_shared<KMInfoJson>();
    d->m_infoJson->loadFile(d->m_metadataFile);

    d->m_id = d->m_infoJson->id;
    if (d->m_id.empty())
    {
        throw KMException(_("No name specified in the metadata"));
    }

    KMTrace("KMBuildFinish::init invoke end");
}

void KMBuildFinish::collectExports()
{
    KMTrace("KMBuildFinish::collectExports invoke begin");

    if (!KMFileUtils::mkpath(d->m_exportDir))
    {
        throw KMException(_("Failed to create export dir"));
    }

    if (d->m_kmOptions->m_noexports)
    {
        KMTrace("KMBuildFinish::collectExports invoke end : noexports");
        return;
    }

    std::vector<std::string> paths = {
        "home",
        "etc",
        "etc/ukui/ukui-notification",         /* Copy .json files */
        "share/applications",                 /* Copy desktop files */
        "share/mime/packages",                /* Copy MIME Type files */
        "share/icons",                        /* Icons */
        "share/pixmaps",                      /* Icons */
        "share/dbus-1",                       /* D-Bus service files */
        "etc/dbus-1",                        /* D-Bus service files */
        "share/gnome-shell/search-providers", /* Search providers */
        "share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "share/metainfo",                     /* Copy appdata/metainfo files */
        "lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "etc/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "share/kylin-user-guide/data/guide",  /* .md, .png files */
        "share/glib-2.0/schemas",             /* .xml */
        "etc/xdg/autostart",                  /* .desktop */
        "share/polkit-1",                     /* .rules, .policy */

        "local/share/applications",                 /* Copy desktop files */
        "local/share/mime/packages",                /* Copy MIME Type files */
        "local/share/icons",                        /* Icons */
        "local/share/pixmaps",                      /* Icons */
        "local/share/dbus-1",                       /* D-Bus service files */
        "local/share/gnome-shell/search-providers", /* Search providers */
        "local/share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "local/share/metainfo",                     /* Copy appdata/metainfo files */
        "local/lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "local/share/kylin-user-guide/data/guide",  /* .md, .png files */
        "local/share/glib-2.0/schemas",             /* .xml */
        "local/share/polkit-1",                     /* .rules, .policy */

        // 下面/usr和/usr/local路径是为sysapp做的兼容
        "usr/share/applications",                 /* Copy desktop files */
        "usr/share/mime/packages",                /* Copy MIME Type files */
        "usr/share/icons",                        /* Icons */
        "usr/share/pixmaps",                      /* Icons */
        "usr/share/dbus-1",                       /* D-Bus service files */
        "usr/share/gnome-shell/search-providers", /* Search providers */
        "usr/share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "usr/share/metainfo",                     /* Copy appdata/metainfo files */
        "usr/lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "usr/share/kylin-user-guide/data/guide",  /* .md, .png files */
        "usr/share/glib-2.0/schemas",             /* .xml */
        "usr/share/polkit-1",                     /* .rules, .policy */

        "usr/local/share/applications",                 /* Copy desktop files */
        "usr/local/share/mime/packages",                /* Copy MIME Type files */
        "usr/local/share/icons",                        /* Icons */
        "usr/local/share/pixmaps",                      /* Icons */
        "usr/local/share/dbus-1",                       /* D-Bus service files */
        "usr/local/share/gnome-shell/search-providers", /* Search providers */
        "usr/local/share/appdata",                      /* Copy appdata/metainfo files (legacy path) */
        "usr/local/share/metainfo",                     /* Copy appdata/metainfo files */
        "usr/local/lib/systemd",                        /* .service, .target, .timer, .socket, .conf */
        "usr/local/share/kylin-user-guide/data/guide",  /* .md, .png files */
        "usr/local/share/glib-2.0/schemas",             /* .xml */
        "usr/local/share/polkit-1",                     /* .rules, .policy */
    };

    std::string errMsg;
    for (const std::string &path : paths)
    {
        std::set<std::string> allowed_extensions;
        std::set<std::string> allowed_prefixes;
        bool require_exact_match;

        if (path == "home" || path == "etc" || path == "etc/ukui/ukui-notification")
        {
            std::string src = KMStringUtils::buildFilename(d->m_filesDir, path);
            if ( !KMFileUtils::pathExists(src) || KMFileUtils::isEmptyDir(src))
            {
                continue;
            }
            std::string dest = KMStringUtils::buildFilename(d->m_exportDir, path);
            if (!KMFileUtils::mkpath(dest))
            {
                throw KMException(_("Failed to create dir : ") + dest);
            }
            if (!KMFileUtils::rsyncDir(src, dest))
            {
                errMsg = _("Failed to copy file : ") + src;
                throw KMException(errMsg);
            }
            continue;
        }

        if (!getAllowedExports(path, allowed_extensions, allowed_prefixes, require_exact_match))
        {
            KMInfo("Unexpectedly not allowed to export " + path);
            continue;
        }

        std::string src = KMStringUtils::buildFilename(d->m_filesDir, path);
        if (fs::exists(src))
        {
            KMDebug("Exporting from " + path);

            std::string dest;
            if (path == "share/appdata" || path == "local/share/appdata" || path == "usr/share/appdata" || path == "usr/local/share/appdata")
            {
                dest = KMStringUtils::buildFilename(d->m_exportDir, "share/metainfo");
            }
            else
            {
                dest = KMStringUtils::buildFilename(d->m_exportDir, path);
                KMStringUtils::replace(dest, "/usr/local/share/", "/share/");
                KMStringUtils::replace(dest, "/usr/share/", "/share/");
                KMStringUtils::replace(dest, "/usr/local/lib/", "/lib/");
                KMStringUtils::replace(dest, "/usr/lib/", "/lib/");
                KMStringUtils::replace(dest, "/local/share/", "/share/");
                KMStringUtils::replace(dest, "/local/lib/", "/lib/");
            }

            std::string parentDest = fs::path(dest).parent_path().string();
            KMDebug("Ensuring export/" + path + " parent exists");
            if (!KMFileUtils::mkpath(parentDest))
            {
                throw KMException(_("Failed to create dir : ") + parentDest);
            }

            KMInfo("Copying from files/" + path);
            if (!copyExports(src, dest, path, allowed_prefixes, allowed_extensions, require_exact_match, errMsg))
            {
                throw KMException(errMsg);
            }
        }
    }

    KMTrace("KMBuildFinish::collectExports invoke end");
}

bool KMBuildFinish::copyExports(const std::string &srcDir, const std::string &destDir, const std::string &srcPrefix, const std::set<std::string> &allowedPrefixes,
                                const std::set<std::string> &allowedExtensions, bool requireExactMatch, std::string &errMsg)
{
    KMTrace("KMBuildFinish::copyExports invoke begin");

    if (!KMFileUtils::mkpath(destDir))
    {
        errMsg = _("Failed to create path :") + destDir;
        return false;
    }

    if (!exportDir("", srcDir, srcPrefix, "", destDir, allowedPrefixes, allowedExtensions, requireExactMatch, errMsg))
    {
        return false;
    }

    KMTrace("KMBuildFinish::copyExports invoke end");
    return true;
}

bool KMBuildFinish::exportDir(const std::string &srcDir, const std::string &srcName, const std::string &srcRelPath, const std::string &destDir, const std::string &destName,
                              const std::set<std::string> &allowedPrefixes, const std::set<std::string> &allowedExtensions, bool requireExactMatch, std::string &errMsg)

{
    std::string dest = KMStringUtils::buildFilename(destDir, destName);
    if (!KMFileUtils::mkpath(dest, 0755))
    {
        errMsg = _("Failed to create path : ") + dest;
        return false;
    }

    std::string src = KMStringUtils::buildFilename(srcDir, srcName);
    fs::path srcPath(src);
    if (!fs::exists(srcPath))
    {
        return true;
    }

    for (auto const &entry : fs::directory_iterator{ srcPath })
    {
        fs::path filepath = entry.path();
        std::string filename = filepath.filename();

        /* Don't export any hidden files or backups */
        if (KMStringUtils::startsWith(filename, ".") || KMStringUtils::endsWith(filename, "~"))
        {
            continue;
        }

        if (fs::is_directory(filepath))
        {
            std::string childRelPath = KMStringUtils::buildFilename(srcRelPath, filename);
            if (!exportDir(src, filename, childRelPath, dest, filename, allowedPrefixes, allowedExtensions, requireExactMatch, errMsg))
            {
                return false;
            }
        }
        else if (fs::is_regular_file(filepath) || fs::is_symlink(filepath))
        {
            std::string sourcePrintable = KMStringUtils::buildFilename(srcRelPath, filename);

            std::string extension;
            for (const std::string &allowedExtension : allowedExtensions)
            {
                if (KMStringUtils::endsWith(filename, allowedExtension))
                {
                    extension = allowedExtension;
                    break;
                }
            }

            if (extension.empty())
            {
                errMsg = _("Wrong extension, not exporting ") + sourcePrintable;
                continue;
            }

            KMDebug(_("Exporting ") + sourcePrintable);

            std::string from = KMStringUtils::buildFilename(src, filename);
            if (!KMFileUtils::cpRfAll(from, dest))
            {
                errMsg = _("Failed to copy file : ") + from;
                return false;
            }
        }
        else
        {
            std::string sourcePrintable = KMStringUtils::buildFilename(srcRelPath, filename);
            KMDebug("Not exporting non-regular file " + sourcePrintable);
        }
    }

    // 如果拷贝的是空目录，则移除；不导出空目录
    if (KMFileUtils::isEmptyDirectory(dest))
    {
        KMFileUtils::removeAll(dest);
    }

    return true;
}

bool KMBuildFinish::getAllowedExports(const std::string &path, std::set<std::string> &allowedExtensions, std::set<std::string> &allowedPrefixes, bool &requireExactMatch)
{
    requireExactMatch = false;

    if (path == "share/applications" ||
        path == "local/share/applications" ||
        path == "usr/share/applications" ||
        path == "usr/local/share/applications")
    {
        allowedExtensions.emplace(".desktop");
    }
    else if (KMStringUtils::startsWith(path, "share/icons") ||
             KMStringUtils::startsWith(path, "local/share/icons") ||
             KMStringUtils::startsWith(path, "usr/share/icons") ||
             KMStringUtils::startsWith(path, "usr/local/share/icons"))
    {
        allowedExtensions.emplace(".svgz");
        allowedExtensions.emplace(".png");
        allowedExtensions.emplace(".svg");
        allowedExtensions.emplace(".ico");
    }
    else if (KMStringUtils::startsWith(path, "share/pixmaps") ||
             KMStringUtils::startsWith(path, "local/share/pixmaps") ||
             KMStringUtils::startsWith(path, "usr/share/pixmaps") ||
             KMStringUtils::startsWith(path, "usr/local/share/pixmaps"))
    {
        allowedExtensions.emplace(".svgz");
        allowedExtensions.emplace(".png");
        allowedExtensions.emplace(".svg");
        allowedExtensions.emplace(".ico");
        allowedExtensions.emplace(".xpm");
    }
    else if (path == "share/dbus-1" ||
             path == "local/share/dbus-1" ||
             path == "usr/share/dbus-1" ||
             path == "usr/local/share/dbus-1" ||
             path == "etc/dbus-1")
    {
        allowedExtensions.emplace(".service");
        allowedExtensions.emplace(".conf");
        allowedExtensions.emplace(".xml");
        allowedExtensions.emplace(".limit");
        allowedExtensions.emplace(".verify");
    }
    else if (path == "share/gnome-shell/search-providers" ||
             path == "local/share/gnome-shell/search-providers" ||
             path == "usr/share/gnome-shell/search-providers" ||
             path == "usr/local/share/gnome-shell/search-providers")
    {
        allowedExtensions.emplace(".ini");
    }
    else if (path == "share/mime/packages" ||
             path == "local/share/mime/packages" ||
             path == "usr/share/mime/packages" ||
             path == "usr/local/share/mime/packages")
    {
        allowedExtensions.emplace(".xml");
    }
    else if (path == "share/metainfo" || path == "share/appdata" ||
             path == "local/share/metainfo" || path == "local/share/appdata" ||
             path == "usr/share/metainfo" || path == "usr/share/appdata" ||
             path == "usr/local/share/metainfo" || path == "usr/local/share/appdata")
    {
        allowedExtensions.emplace(".xml");
    }
    else if (path == "lib/systemd" || path == "etc/systemd" ||
             path == "local/lib/systemd" ||
             path == "usr/lib/systemd" ||
             path == "usr/local/lib/systemd")
    {
        allowedExtensions.emplace(".service");
        allowedExtensions.emplace(".target");
        allowedExtensions.emplace(".timer");
        allowedExtensions.emplace(".socket");
        allowedExtensions.emplace(".conf");
    }
    else if (KMStringUtils::startsWith(path, "share/kylin-user-guide/data/guide") ||
             KMStringUtils::startsWith(path, "local/share/kylin-user-guide/data/guide") ||
             KMStringUtils::startsWith(path, "usr/share/kylin-user-guide/data/guide") ||
             KMStringUtils::startsWith(path, "usr/local/share/kylin-user-guide/data/guide"))
    {
        // 麒麟系统应用的用户手册
        allowedExtensions.emplace(".md");
        allowedExtensions.emplace(".png");
    }
    else if (path == "share/glib-2.0/schemas" ||
             path == "local/share/glib-2.0/schemas" ||
             path == "usr/share/glib-2.0/schemas" ||
             path == "usr/local/share/glib-2.0/schemas")
    {
        // gsettings
        allowedExtensions.emplace(".xml");
    }
    else if (path == "share/polkit-1" ||
             path == "local/share/polkit-1" ||
             path == "usr/share/polkit-1" ||
             path == "usr/local/share/polkit-1")
    {
        // polkit
        allowedExtensions.emplace(".rules");
        allowedExtensions.emplace(".policy");
    }
    else if (path == "etc/xdg/autostart")
    {
        allowedExtensions.emplace(".desktop");
    }
    else
    {
        return false;
    }

    return true;
}

int KMBuildFinish::run()
{
    KMTrace("KMBuildFinish::run invoke begin");

    if (fs::exists(d->m_exportDir))
    {
        // throw KMException(_("Build directory already finalized : ") + d->m_kmOptions->m_directory);
        // KMFileUtils::rmRfAll(d->m_exportDir);
        fs::remove_all(d->m_exportDir);
    }

    KMDebug("Collecting exports");
    collectExports();

    KMDebug("Updating info.json");
    updateMetadata();
    KMInfo(_("Please review the exported files and the info.json"));

    if (KMFileUtils::isEmptyDir(d->m_filesDir))
    {
        throw KMException(d->m_filesDir + _(" is empty. Please check if the installation path is correct. Recommend to use the installation path:") + "/opt/apps/"+ d->m_id + "/files");
    }

    KMTrace("KMBuildFinish::run invoke end");
    return EXIT_SUCCESS;
}

std::string KMBuildFinish::findCommandInPath(const std::string &binDir)
{
    std::string command = "";
    if (fs::exists(binDir) && fs::is_directory(binDir))
    {
        for (auto const &entry : fs::directory_iterator{ binDir })
        {
            if (!command.empty())
            {
                KMWarn(_("More than one executable found"));
                break;
            }

            command = entry.path().filename();
        }
    }

    return command;
}

void KMBuildFinish::updateMetadata()
{
    KMTrace("KMBuildFinish::updateMetadata invoke begin");

    if (d->m_infoJson->commands.empty() && d->m_infoJson->kind == BASE_TYPE_APP)
    {
        KMDebug("Looking for executables");

        std::string binDir = KMStringUtils::buildFilename(d->m_filesDir, "bin");
        std::string command = KMBuildFinish::findCommandInPath(binDir);
        if (command.empty())
        {
            binDir = KMStringUtils::buildFilename(d->m_filesDir, "usr/local/bin");
            command = KMBuildFinish::findCommandInPath(binDir);
        }
        if (command.empty())
        {
            binDir = KMStringUtils::buildFilename(d->m_filesDir, "usr/bin");
            command = KMBuildFinish::findCommandInPath(binDir);
        }

        if (command.empty())
        {
            throw KMException("No executable found, can't set 'command' into info.json");
        }
        else
        {
            KMInfo(_("Using command : ") + command);
            d->m_infoJson->commands.push_back(command);
        }
    }

    d->m_infoJson->size = KMFileUtils::getDirectorySize(fs::path(d->m_baseDir));
    d->m_infoJson->saveFile(d->m_metadataFile);

    // 更新更精确
    d->m_infoJson->loadFile(d->m_metadataFile);
    d->m_infoJson->size = KMFileUtils::getDirectorySize(fs::path(d->m_baseDir));
    d->m_infoJson->saveFile(d->m_metadataFile);

    KMTrace("KMBuildFinish::updateMetadata invoke end");
}