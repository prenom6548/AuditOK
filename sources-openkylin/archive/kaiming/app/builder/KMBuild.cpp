/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuild.h"

#include <gio/gio.h>
#include <glib.h>
#include <sys/ioctl.h>
#include <sys/personality.h>
#include <array>
#include <filesystem>
#include <limits>
#include <string>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <optional>
#include <algorithm>
#include <unordered_set>

#include "common/KMBuildinsUtils.h"
#include "common/KMCommonTypePrivate.h"
#include "common/KMException.h"
#include "common/KMInfoJson.h"
#include "common/KMInstalledAppQuery.h"
#include "common/KMLogger.h"
#include "common/KMBuildinOptions.h"
#include "common/KMUtil.h"
#include "common/KMProcessUtils.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "common/kmtranslation.h"

#include "KMContainerWrap.h"

namespace fs = std::filesystem;

class KMBuild::Options : public KMOption::Options
{
public:
    Options() = default;
    ~Options() override = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();
    void normalizeCommand(std::string &command);

public:
    // application options
    bool m_help = false;
    bool m_dieWithParent = false;
    bool m_withAppdir = false;
    bool m_debug = false;
    std::vector<std::string> m_bindMounts;
    std::string m_buildDir;

    // context options
    std::vector<std::string> m_envs;
    std::vector<std::string> m_unsetenvs;

    // position options
    std::string m_directory;
    std::string m_command;
    std::string m_splitOfArgs;
    std::vector<std::string> m_args;
};

void KMBuild::Options::preParseHook()
{
    addOptions();
}

void KMBuild::Options::postParseHook()
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

    if (m_splitOfArgs.empty() && !m_command.empty())
    {
        // kaiming build [OPTION…] DIRECTORY [COMMAND [--] [ARGUMENT…]]
        int posDirectory = getPositionalOptionIndex("COMMAND");
        int steps = m_splitOfArgs.empty() ? 1 : 2;
        if (posDirectory > 0 && m_originalArgs.size() > posDirectory + steps)
        {
            for (int i = posDirectory + steps; i < m_originalArgs.size(); i++)
            {
                m_args.push_back(m_originalArgs.at(i));
            }
        }
    }

    if (m_command.empty())
    {
        m_command = DEFAULT_SHELL;
    }
}

void KMBuild::Options::addOptions()
{
    setDescription(_("\nUsage:\n \tkaiming build [OPTION…] DIRECTORY [COMMAND [--] [ARGUMENT…]]"
                     "\nExample:\n \tkaiming build ${build-dir} cmake -j8\n"));
    //帮助选项
    addOption("help", "h", KMOption::value<bool>(&m_help), _("Help Options"));
    //应用选项
    addOption("bind-mount", "", KMOption::value<std::vector<std::string>>(&m_bindMounts), _("Add bind mount, --bind-mount=DEST=SRC"));
    addOption("build-dir", "", KMOption::value<std::string>(&m_buildDir), _("Start build in this directory, --build-dir=DIR"));
    addOption("debug", "", KMOption::value<bool>(&m_debug), _("Used for debugging"));
    //context选项
    addOption("env", "", KMOption::value<std::vector<std::string>>(&m_envs), _("Set environment variable, --env=VAR=VALUE"));
    addOption("unset-env", "", KMOption::value<std::vector<std::string>>(&m_unsetenvs), _("Remove variable from environment, --unset-env=VAR"));

    //位置选项
    addPositionOption("DIRECTORY", KMOption::value<std::string>(&m_directory), 1, _("The app dir"));
    addPositionOption("COMMAND", KMOption::value<std::string>(&m_command), 1, _("COMMAND"));
    addPositionOption("--", KMOption::value(&m_splitOfArgs), 1, _("The start flag of command's arguments(not kaiming's options)"));
    addPositionOption("ARGUMENT", KMOption::value<std::vector<std::string>>(&m_args), -1, _("ARGUMENT"));
}

class KMBuild::Private
{
public:
    std::unique_ptr<Options> m_kmOptions;

    std::string m_command;
    std::string m_metadata;
    std::shared_ptr<KMInfoJson> m_infoJson;
    KMRef m_baseRef;
    KMRef m_runtimeRef;
    std::string m_id;

    std::shared_ptr<KMDeploy> m_baseDeploy;
    std::shared_ptr<KMDeploy> m_runtimeDeploy;
    std::string m_baseFiles;
    std::string m_runtimeFiles;
    std::string m_resFiles;
    std::string m_appIdDir;
    std::string m_workdir;
    std::string m_upperdir;

    std::unique_ptr<KMContainerWrap> m_container;
    std::shared_ptr<KMContext> m_appContext;
    std::shared_ptr<KMContext> m_argContext;

    uid_t m_uid = 0;
    gid_t m_gid = 0;
};

REGISTER_SUBCOMMAND_DYNCREATE(build, KMBuild)

KMBuild::KMBuild()
{
    d = std::make_unique<Private>();
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuild::~KMBuild() = default;

int KMBuild::dispose(int argc, char **argv)
{
    KMTrace("KMBuild::dispose invoke begin");

    init(argc, argv);

    int ret = run();

    KMTrace("KMBuild::dispose invoke end");
    return ret;
}

void KMBuild::init(int argc, char **argv)
{
    KMTrace("KMBuild::init invoke begin");

    d->m_kmOptions->parseCommandLine(argc, argv);
    parseOptionsToContext();

    d->m_command = d->m_kmOptions->m_command;
    d->m_metadata = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "info.json");
    d->m_infoJson = std::make_shared<KMInfoJson>();
    d->m_infoJson->loadFile(d->m_metadata);
    d->m_id = d->m_infoJson->id;

    // 解析base、runtime，并校验是否已经安装
    KMRef originalBaseRef = d->m_infoJson->baseRef();
    // 校验base是否已安装
    KMInstalledAppQuery query;
    std::vector<KMRef> bases = query.channel(originalBaseRef.channel)
                                   .arch(originalBaseRef.arch)
                                   .kind(BASE_TYPE_BASE)
                                   .id(originalBaseRef.id)
                                   .module(MODULE_NAME_DEV)
                                   .version(originalBaseRef.version)
                                   .query(KMInstalledAppQuery::User);
    if (bases.empty())
    {
        throw KMException("No matching development base found, please install it : sudo kaiming install " + KMStringUtils::buildFilename(originalBaseRef.id, originalBaseRef.version, MODULE_NAME_DEV, originalBaseRef.channel));
    }
    d->m_baseRef = bases.at(0);
    d->m_baseDeploy = KMStorageDir::getSystemDefaultDir().loadDeployed(d->m_baseRef);
    if (!d->m_baseDeploy)
    {
        throw KMException(_("The base's info.json load failed"));
    }
    d->m_baseFiles = KMStringUtils::buildFilename(d->m_baseDeploy->m_dir, "files");

    // 解析runtime，并校验是否已经安装
    if (!d->m_infoJson->runtime.empty())
    {
        KMRef originalRuntimeRef = d->m_infoJson->runtimeRef();
        std::vector<KMRef> runtimes = query.channel(originalRuntimeRef.channel)
                                          .arch(originalRuntimeRef.arch)
                                          .kind(BASE_TYPE_RUNTIME)
                                          .id(originalRuntimeRef.id)
                                          .module(MODULE_NAME_DEV)
                                          .version(originalRuntimeRef.version)
                                          .query(KMInstalledAppQuery::User);
        if (runtimes.empty())
        {
            throw KMException("No matching development runtime found, please install it : sudo kaiming install " + KMStringUtils::buildFilename(originalRuntimeRef.id, originalRuntimeRef.version, MODULE_NAME_DEV, originalRuntimeRef.channel));
        }
        d->m_runtimeRef = runtimes.at(0);

        d->m_runtimeDeploy = KMStorageDir::getSystemDefaultDir().loadDeployed(d->m_runtimeRef);
        if (!d->m_runtimeDeploy)
        {
            throw KMException(_("The runtime's info.json load failed"));
        }

        d->m_runtimeFiles = KMStringUtils::buildFilename(d->m_runtimeDeploy->m_dir, "files");
    }

    // 获取build上层目录
    d->m_appIdDir = KMStorageDir::getDataDir(d->m_id);
    // 出现/home各个子目录属于不同分区的场景，故修改upperdir和workdir的路径，确保和构建结果build目录在同一文件系统。
    // const char *home = std::getenv("HOME");
    // std::string homeDir;
    // if (nullptr != home && *home != '0')
    // {
    //     homeDir = home;
    // }
    std::string overlaydir;
    // if (d->m_infoJson->annotations.sysapp || !KMStringUtils::startsWith(d->m_kmOptions->m_directory, homeDir))
    // {
        // workdir和upperdir需要在同一文件系统中
        fs::path directory = d->m_kmOptions->m_directory;
        fs::path directory_parent = directory.parent_path();
        overlaydir = KMStringUtils::buildFilename(directory_parent, ".kaiming-builder", "overlaydir");
    // }
    // else
    // {
    //     overlaydir = KMStringUtils::buildFilename(d->m_appIdDir, "overlaydir");
    // }
    if (!fs::exists(overlaydir))
    {
        fs::create_directories(overlaydir);
    }
    d->m_workdir = KMStringUtils::buildFilename(overlaydir, "workdir");
    std::error_code ec;
    if (!KMFileUtils::mkpath(d->m_workdir, ec))
    {
        throw KMException(d->m_workdir + _(" create failed, error: ") + ec.message());
    }

    d->m_resFiles = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "files");
    if (d->m_infoJson->annotations.sysapp)
    {
        d->m_upperdir = d->m_resFiles;
    }
    else
    {
        d->m_upperdir = KMStringUtils::buildFilename(overlaydir, "upperdir");
        std::string linkUsr = KMStringUtils::buildFilename(d->m_upperdir, "usr");
        if (fs::exists(linkUsr) && fs::is_symlink(linkUsr))
        {
            std::vector<std::string> args;
            args.push_back(linkUsr);
            KMProcessUtils::spawn("unlink", args, true);
        }
        std::string linkEtc = KMStringUtils::buildFilename(d->m_upperdir, "etc");
        if (fs::exists(linkEtc) && fs::is_symlink(linkEtc))
        {
            std::vector<std::string> args;
            args.push_back(linkEtc);
            KMProcessUtils::spawn("unlink", args, true);
        }

        // 现在的upperdir不再是app的files目录，里面的内容都是临时用的，每次都清空，以防残留，方便暴露未将构建结果生成到/opt/apps/${appid}/files中去的场景
        if (!KMFileUtils::isEmptyDirectory(d->m_upperdir))
        {
            // KMFileUtils::rmRfAll(d->m_upperdir);
            fs::remove_all(d->m_upperdir);
            KMDebug("Clear cache : " + d->m_upperdir);
        }
        KMDebug("mkpath : " + d->m_upperdir);
        if (!KMFileUtils::mkpath(d->m_upperdir, ec))
        {
            throw KMException(d->m_upperdir + _(" create failed, error: ") + ec.message());
        }

        // if (!isV10Sp1())
        {
            // 创建软连接：upperdir/usr/bin --> build/files，以简化构建清单编写
            std::vector<std::string> args;
            args.push_back("-s");
            args.push_back(d->m_resFiles);
            args.push_back(linkUsr);
            if (!KMProcessUtils::spawn("ln", args))
            {
                throw KMException("Failed to ln -s " + d->m_resFiles + " " + linkUsr);
            }

            std::string dirEtc = KMStringUtils::buildFilename(d->m_resFiles, "etc");
            KMFileUtils::mkpath(dirEtc);
            args.clear();
            args.push_back("-s");
            args.push_back(dirEtc);
            args.push_back(linkEtc);
            if (!KMProcessUtils::spawn("ln", args))
            {
                throw KMException("Failed to ln -s " + dirEtc + " " + linkEtc);
            }
        }
    }

    // TODO : 预留。1、从base和runtime继承权限？2、主机文件系统读写权限？3、从命令行传入的权限？
    d->m_appContext = std::make_shared<KMContext>();
    d->m_appContext->merge(*d->m_argContext);

    KMTrace("KMBuild::init invoke end");
}

int KMBuild::getTerminalSize(int &rows, int &columns)
{
    struct winsize w;
    
    // 检查标准输出是否连接到终端
    if (!isatty(STDIN_FILENO))
    {
        return -1;
    }
    
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &w) == -1)
    {
        return -1;
    }

    rows = w.ws_row;
    columns = w.ws_col;

    return 0;
}

void KMBuild::parseOptionsToContext()
{
    KMTrace("KMBuild::parseOptionsToContext invoke begin");

    d->m_argContext = std::make_shared<KMContext>();

    setOptionEnvContext();

    KMTrace("KMBuild::parseOptionsToContext invoke end");
}

void KMBuild::setOptionEnvContext()
{
    for (const std::string &env : d->m_kmOptions->m_envs)
    {
        std::string::size_type pos = env.find("=");
        if (std::string::npos == pos)
        {
            throw KMException(_("Wrong option --env=") + env);
        }

        std::string var = env.substr(0, pos);
        std::string value = env.substr(pos + 1);
        d->m_argContext->addEnvVar(var, value);
    }

    for (const auto &unenv : d->m_kmOptions->m_unsetenvs)
    {
        if (KMStringUtils::contains(unenv, "="))
        {
            throw KMException(_("Wrong option --unset-env=") + unenv + _(". Environment variable name must not contain '='"));
        }

        d->m_argContext->addEnvVar(unenv, "");
    }
}

void KMBuild::setProcess()
{
    Process p = d->m_container->getProcess();
    p.cwd = d->m_kmOptions->m_buildDir.empty() ? "/app" : d->m_kmOptions->m_buildDir;

    uid_t uid = ::getuid();
    struct passwd *pwd = ::getpwuid(uid);
    if (pwd == nullptr)
    {
        throw KMException("Couldn't getpwuid");
    }
    User user;
    user.uid = uid;
    user.gid = pwd->pw_gid;
    user.username = pwd->pw_name;

    p.user = user;

    if (d->m_command == DEFAULT_SHELL && d->m_kmOptions->m_args.empty())
    {
        int rows=0, columns=0;
        if (KMBuild::getTerminalSize(rows, columns) == 0)
        {
            std::string contend = "#!/bin/bash\n\nstty rows " + std::to_string(rows) + " columns " + std::to_string(columns) + "\n/bin/bash\nexit 0\n";
            d->m_container->addShareMount("/run/build/stty_bash.sh", contend, "/run/build/stty_bash.sh", { "rbind", "ro", "rprivate" });
            d->m_command = "/run/build/stty_bash.sh";
        }
    }

    std::vector<std::string> args;
    args.push_back(d->m_command);
    args.insert(args.end(), d->m_kmOptions->m_args.begin(), d->m_kmOptions->m_args.end());
    p.args = args;

    d->m_container->setProcess(p);
    d->m_container->setupProcessEnvs();
}

void KMBuild::setupAppInfo()
{
    d->m_container->setupAppDataDir(d->m_id);

    // info.json中的env包含了depends依赖引入的环境变量，故此处追加
    for (auto& env : d->m_infoJson->envs)
    {
        d->m_container->addEnv(env.first, env.second);
    }

    if (d->m_infoJson->annotations.sysapp)
    {
        d->m_container->addMount(d->m_resFiles, "/app", "bind", { "rbind", "rprivate" });
    }
    else
    {
        std::string appDest = "/opt/apps/" + d->m_id + "/files";
        d->m_container->addMount(d->m_resFiles, appDest, "bind", { "rbind", "rprivate" });
        d->m_container->addEnv("PATH", appDest + "/local/bin:" + appDest + "/bin:");
        d->m_container->addEnv("LIBRARY_PATH", appDest + "/local/lib:" + appDest + "/local/lib/" + KMUtil::getLibraryDir() + ":" + appDest + "/lib:" + appDest + "/lib/" + KMUtil::getLibraryDir() + ":" + appDest + "/lib64:" + appDest + "/lib32:");
        d->m_container->addEnv("LD_LIBRARY_PATH", appDest + "/local/lib:" + appDest + "/local/lib/" + KMUtil::getLibraryDir() + ":" + appDest + "/lib:" + appDest + "/lib/" + KMUtil::getLibraryDir() + ":" + appDest + "/lib64:" + appDest + "/lib32:");
        d->m_container->addEnv("XDG_CONFIG_DIRS", appDest + "/etc/xdg:");
        d->m_container->addEnv("XDG_DATA_DIRS", appDest + "/local/share:" + appDest + "/share:");
        d->m_container->addEnv("ACLOCAL_PATH", appDest + "/local/share/aclocal:" + appDest + "/share/aclocal:");
        d->m_container->addEnv("C_INCLUDE_PATH", appDest + "/local/include:" + appDest + "/include:");
        d->m_container->addEnv("CPLUS_INCLUDE_PATH", appDest + "/local/include:" + appDest + "/include:");
        d->m_container->addEnv("LDFLAGS", "-L" + appDest + "/local/lib " + "-L" + appDest + "/lib " + "-L" + appDest + "/lib64 " + "-L" + appDest + "/lib32 ");
        d->m_container->addEnv("PKG_CONFIG_PATH", appDest + "/local/lib/pkgconfig:" + appDest + "/local/share/pkgconfig:" + appDest + "/local/lib/" + KMUtil::getLibraryDir() + "/pkgconfig:"
                                                + appDest + "/lib/pkgconfig:" + appDest + "/share/pkgconfig:" + appDest + "/lib/" + KMUtil::getLibraryDir() + "/pkgconfig:");
    }
    d->m_container->addEnv("QML2_IMPORT_PATH", "/lib64/qml:/usr/lib64/qml:/lib/qml:/usr/lib/qml");
    d->m_container->addEnv("QT_PLUGIN_PATH", "/lib64/plugins:/usr/lib64/plugins:/lib/plugins:/usr/lib/plugins:/usr/lib/" + KMUtil::getLibraryDir() + "/qt5/plugins");
    d->m_container->addEnv("__EGL_EXTERNAL_PLATFORM_CONFIG_DIRS", "/etc/egl/egl_external_platform.d:/usr/lib/" + KMUtil::getLibraryDir() + "/GL/egl/egl_external_platform.d:/usr/share/egl/egl_external_platform.d");

    d->m_container->setupAppContext(d->m_appContext);

    if (!d->m_kmOptions->m_buildDir.empty() && fs::exists(d->m_kmOptions->m_buildDir))
    {
        d->m_container->addMount(d->m_kmOptions->m_buildDir, d->m_kmOptions->m_buildDir, "bind", { "rbind", "rprivate" });
    }

    for (const std::string &bindMount : d->m_kmOptions->m_bindMounts)
    {
        size_t pos = bindMount.find_first_of('=');
        if (std::string::npos == pos)
        {
            throw KMException(_("Missing '=' in bind mount option : ") + bindMount);
        }

        d->m_container->addMount(bindMount.substr(pos + 1), bindMount.substr(0, pos), "bind", { "rbind", "rprivate" });
    }

    setProcess();
}

int KMBuild::run()
{
    KMTrace("KMBuild::run invoke begin");

    d->m_container = std::make_unique<KMContainerWrap>();
    d->m_container->setAppID(d->m_id);
    d->m_container->initMinimalEnvs(true, false);
    if (d->m_infoJson->annotations.sysapp)
    {
        d->m_container->addEnv("APP_INSTALL_PREFIX", "/usr");
        d->m_container->addEnv("APP_ID", d->m_id);
        d->m_container->addEnv("APP_VERSION", d->m_infoJson->version);
        d->m_container->addEnv("APP_ARCH", d->m_baseDeploy->m_ref.arch);
        d->m_container->addEnv("APP_DIRECTORY", "/app");
    }
    else
    {
        std::string appDest = "/opt/apps/" + d->m_id + "/files";
        d->m_container->addEnv("APP_DIRECTORY", appDest);
        d->m_container->addEnv("APP_INSTALL_PREFIX", appDest);
        d->m_container->addEnv("APP_ID", d->m_id);
        d->m_container->addEnv("APP_VERSION", d->m_infoJson->version);
        d->m_container->addEnv("APP_ARCH", d->m_baseDeploy->m_ref.arch);
    }
    
    std::string qt6 = KMStringUtils::buildFilename(d->m_runtimeFiles, "/usr/lib/qt6");
    if (fs::exists(qt6))
    {
        // runtime制作的有问题，qt程序报错：qmake: could not find a Qt installation of ''，此处需要做兼容处理
        d->m_container->addEnv("QT_SELECT", "qt6");
    }
    else
    {
        d->m_container->addEnv("QT_SELECT", "qt5");
    }
    
    std::vector<std::string> depends_paths;
    std::string dependsDir = KMStringUtils::buildFilename(d->m_kmOptions->m_directory, "depends");
    if (fs::exists(dependsDir))
    {
        depends_paths.push_back(dependsDir);
    }
    d->m_container->setUpperdir(d->m_upperdir);
    d->m_container->setupOverlayfs(d->m_baseFiles, d->m_runtimeFiles, d->m_upperdir, d->m_workdir, depends_paths);
    d->m_container->setupBase(d->m_baseDeploy);
    d->m_container->setupRuntime(d->m_runtimeDeploy);
    setupAppInfo();

    if (!d->m_container->run())
    {
        throw KMException(_("Failed to run ") + d->m_command);
    }

    if (d->m_kmOptions->m_debug && d->m_infoJson->annotations.sysapp)
    {
        // 调试后需要清理
        cleanup();
    }

    KMTrace("KMBuild::run invoke end");
    return EXIT_SUCCESS;
}

void KMBuild::cleanup() const
{
    KMTrace("KMBuild::cleanup invoke begin");

    // 删除构建结果中的因挂载而生成到构建结果中的文件，需要在删除空目录之前
    std::vector<std::string> defaultCleanups = {
        "/etc/default/locale",
        "/etc/timezone",
        "/etc/passwd",
        "/etc/group",
        "/etc/pkcs11/pkcs11.conf",
        "/etc/shadow",
        "/etc/gshadow",
        "/etc/machine-id",
        "/etc/localtime",
        "/usr/share/zoneinfo",
        "/etc/resolv.conf",
        "/etc/host.conf",
        "/etc/hosts",
        "/etc/gai.conf",
        "/usr/bin/xdg-open"
    };

    for (auto const &cleanup : defaultCleanups)
    {
        std::string remove = KMStringUtils::buildFilename(d->m_resFiles, cleanup);
        KMFileUtils::rmRfAll(remove);
    }

    // 删除构建结果中的空文件夹，这些空文件夹应该都是overlay叠加挂载时创建upperdir时创建的（目的是使得目录可写）
    if (std::error_code ec; !KMFileUtils::removeEmptyDirectory(d->m_resFiles, ec))
    {
        throw KMException(ec.message());
    }

    KMTrace("KMBuild::cleanup invoke end");
}
