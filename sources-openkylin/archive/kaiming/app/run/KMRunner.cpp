/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMRunner.h"

#include <set>
#include <regex>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>

#include "KMReference.h"
#include "KMRunnerOptions.h"
#include "KMRunnerCommon.h"
#include "KMRunnerOnce.h"
#include "KMRunnerPlugins.h"
#include "KMRunnerMount.h"
#include "common/KMConfigData.h"
#include "common/KMContainer.h"
#include "common/KMLogger.h"
#include "common/KMDirUtils.h"
#include "common/KMUnixSocketServer.h"
#include "common/KMOverlay.h"
#include "common/KMBrowserUtils.h"
#include "common/KMProcessUtils.h"
#include "common/KMStringUtils.h"

static const std::vector<std::string> OVERLAY_DIRS = { "/usr", "/lib", "/lib32", "/lib64", "/bin", "/sbin", "/etc", "/opt" };
static const std::string KM_ORIGIN_PLUGINS_DIR = "/opt/kaiming/exports/plugins";
static const std::string KM_PLUGINS_DIR = "/opt/system/resource/export/kaiming";
static const std::string KARE_PLUGINS_DIR = "/opt/system/resource/export/kare";

REGISTER_SUBCOMMAND_DYNCREATE(run, KMRunner)

class KMRunner::Private
{
public:
    bool isBindHostHome;
    std::string appId;
    std::string appDir;
    std::string baseDir;
    std::string runtimeDir;
    std::string appDataPath;
    std::string optionCommand;
    std::string runCommands;
    std::string arch;
    std::string runDir;
    std::string containerId;
    std::string homeDir;
    std::string cacheDir;
    std::string version;
    std::string sockPath;
    std::string pulseSocket;
    std::string xdgRuntimeDir;
    std::vector<std::string> commandArgs;
    std::vector<std::string> infoCommands;
    std::vector<std::string> envs;
    std::map<std::string, std::string> infoEnvs;
    std::unique_ptr<KMContainer> container;
    std::unique_ptr<KMRunnerOptions> options;
    Permissions perms;
    Annotations annotations;
    KMUnixSocketServer server;
};

/**
 * @brief 构造函数
 */
KMRunner::KMRunner()
    : d_ptr(std::make_shared<KMRunner::Private>())
{
    d_ptr->container = std::make_unique<KMContainer>();
    d_ptr->options = std::make_unique<KMRunnerOptions>();
}

/**
 * @brief 开明子命令运行
 * @param argc 参数个数
 * @param argv 参数列表
 * @return 0 成功 非0 失败
 */
int KMRunner::dispose(int argc, char **argv)
{
    if (!parseArgs(argc, argv))
    {
        return false;
    }

    return start() ? EXIT_SUCCESS : -1;
}

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数列表
 * @return true 成功 false 失败
 */
bool KMRunner::parseArgs(int argc, char **argv)
{
    // 参数解析
    d_ptr->options->parseCommandLine(argc, argv);

    d_ptr->appId = d_ptr->options->getAppId();
    if (d_ptr->appId.empty())
    {
        KMError("app id is empty");
        return false;
    }

    d_ptr->optionCommand = d_ptr->options->getCommand();

    for (auto &arg : d_ptr->options->getArgs())
    {
        std::string completeArg = "\"" + arg + "\"";
        if (!d_ptr->options->isFileWarding())
        {
            d_ptr->commandArgs.emplace_back(completeArg);
            continue;
        }

        if (arg == "@@" || arg == "@@u")
        {
            continue;
        }

        d_ptr->commandArgs.emplace_back(completeArg);
    }

    KMDebug("start app " + d_ptr->appId);

    return true;
}

/**
 * @brief 开始运行
 * @return true 成功 false 失败
 */
bool KMRunner::start()
{
    if (!getLayersDirs())
    {
        KMError("get layers dirs failed");
        return false;
    }

    if (!ensureAppDataDir())
    {
        KMError("create app data directory failed");
        return false;
    }

    if (!setupInterTty())
    {
        KMError("setup inter tty failed");
        return false;
    }

    if (!assembleCommand())
    {
        KMError("assemble command failed");
        return false;
    }

    if (!assembleProcEnv())
    {
        KMError("assemble process env failed");
        return false;
    }

    // 判断是否已经运行,若已经运行则进入已运行的容器环境内运行
    if (KMRunnerOnce::run(d_ptr->appId, d_ptr->runCommands, d_ptr->sockPath, d_ptr->envs))
    {
        return d_ptr->server.wait();
    }

    // 挂载base/runtime/app
    if (!mountLayers())
    {
        KMError("mount layers failed");
        return false;
    }

    // 挂载基础环境
    if (!mountBase())
    {
        KMError("mount others failed");
        return false;
    }

    // 挂载数据目录
    if (!mountData())
    {
        KMError("mount data failed");
        return false;
    }

    // 挂载gpu显卡驱动
    if (!mountGpu())
    {
        KMError("mount gpu failed");
        return false;
    }

    // 权限管控
    if (!permissionControl())
    {
        KMError("permission control failed");
        return false;
    }

    // 处理二进制依赖
    if (!mountDepandApps())
    {
        KMError("mount depend apps failed");
        return false;
    }

    // 挂载插件
    if (!mountPlugins())
    {
        KMError("mount plugins failed");
        return false;
    }

    // 挂载box
    if (!mountBox())
    {
        KMError("mount box failed");
        return false;
    }

    // 挂载covers
    if (!mountCover())
    {
        KMError("mount cover failed");
        return false;
    }

    // 处理运行参数
    if (!setProcessCmd())
    {
        KMError("set process failed");
        return false;
    }

    // 运行
    if (!run())
    {
        if (!d_ptr->options->isDetach() && !d_ptr->server.exitCode.empty() && d_ptr->server.exitCode != "0")
        {
            kmlogger.error(d_ptr->server.errorInfo.c_str());
            KMError("kaiming application exit code: " + d_ptr->server.exitCode);
        }

        KMError("kaiming run failed");
        return false;
    }

    return true;
}

/**
 * @brief 设置交互tty
 * @return true 成功 false 失败
 */
bool KMRunner::setupInterTty()
{
    // wait(true阻塞等待，false不阻塞),与detach相反
    if (!d_ptr->options->isDetach())
    {
        d_ptr->container->setProcessTerminal(true);
        startUnixSocketServer();
    }

    return true;
}

/**
 * @brief 运行
 * @return true 成功 false 失败
 */
bool KMRunner::run()
{
    std::string runDir, containerId, crunDir, cmd;
    if (!d_ptr->container->preRun(runDir, containerId))
    {
        return false;
    }

    if (!KMRunnerOnce::runCmd({}, assembleRunOptions(runDir), containerId))
    {
        d_ptr->server.stop();
        return false;
    }

    // 等待 socket server 完成
    return d_ptr->server.wait();
}

/**
 * @brief 构建run参数
 * @param containerId 容器ID
 * @param runDir 运行目录
 * @return 返回参数列表
 */
std::vector<std::string> KMRunner::assembleRunOptions(const std::string &runDir)
{
    std::vector<std::string> args;
    args.emplace_back("--detach");
    if (!d_ptr->sockPath.empty())
    {
        args.emplace_back("--console-socket=" + d_ptr->sockPath);
    }
    args.emplace_back("-b");
    args.emplace_back(runDir);
    return args;
}

/**
 * @brief 获取app,base,runtime目录
 * @return true 成功 false 失败
 */
bool KMRunner::getLayersDirs()
{
    KMReference ref;
    if (!ref.getLayerDir(d_ptr->appId, d_ptr->options->isDevel()))
    {
        KMError("get layer dir failed");
        return false;
    }

    if (!ref.getAppId(d_ptr->appId))
    {
        KMError("appid is empty");
        return false;
    }

    if (!ref.getArch(d_ptr->arch))
    {
        KMError("get arch failed");
        return false;
    }

    if (!ref.getAppDir(d_ptr->appDir))
    {
        KMError("get app dir failed");
        return false;
    }

    if (!ref.getBaseDir(d_ptr->baseDir))
    {
        KMError("get base dir failed");
        return false;
    }

    if (!ref.getRuntimeDir(d_ptr->runtimeDir))
    {
        KMError("get runtime dir failed");
        return false;
    }

    if (!ref.getVersion(d_ptr->version))
    {
        KMError("get version failed");
        return false;
    }

    if (!ref.getCommands(d_ptr->infoCommands))
    {
        KMError("get commands failed");
        return false;
    }

    if (!ref.getEnvs(d_ptr->infoEnvs))
    {
        KMError("get envs failed");
        return false;
    }

    // 获取权限信息，且权限管控可以是空
    d_ptr->perms = ref.getPermissions();
    // 获取自定义字段
    d_ptr->annotations = ref.getAnnotations();
    // 设置默认权限全部开启
    d_ptr->perms.all = true;

    KMDebug("app dir: " + d_ptr->appDir + ", base dir: " + d_ptr->baseDir + ", runtime dir: " + d_ptr->runtimeDir);

    return true;
}

/**
 * @brief 创建appdate子目录
 * @return true 成功 false 失败
 */
bool KMRunner::ensureAppDataDir()
{
    if (!isExistHomeDir())
    {
        KMError("get home dir failed");
        return false;
    }

    if (!isExistsXdgRuntimeDir())
    {
        KMError("get XDG_RUNTIME_DIR failed");
        return false;
    }

    if (!getAppDataPath())
    {
        return false;
    }

    std::error_code ec;
    fs::path appDataPath(d_ptr->appDataPath);
    d_ptr->cacheDir = d_ptr->homeDir + "/.var/app/kaiming/cache";

    if (!fs::exists(appDataPath, ec) && !fs::create_directories(appDataPath, ec))
        return false;

    if (!fs::exists(appDataPath / "upperdir", ec) && !fs::create_directories(appDataPath / "upperdir", ec))
        return false;

    if (!fs::exists(appDataPath / "workdir", ec) && !fs::create_directories(appDataPath / "workdir", ec))
        return false;

    if (!fs::exists(d_ptr->cacheDir, ec) && !fs::create_directories(d_ptr->cacheDir, ec))
        return false;

    return true;
}

/**
 *@brief 获取appdata路径
 *@return true 成功 false 失败
 */
bool KMRunner::getAppDataPath()
{
    d_ptr->appDataPath = d_ptr->homeDir + "/.var/app/" + d_ptr->appId;
    return true;
}

/**
 *@brief 设置容器annotation信息
 */
void KMRunner::setAnnotationInfo()
{
    d_ptr->container->setAppID(d_ptr->appId);
    d_ptr->container->setBaseDir(d_ptr->baseDir);
    d_ptr->container->addAnnotation("kaiming.app.id", d_ptr->appId);
    d_ptr->container->addAnnotation("kaiming.app.dir", d_ptr->appDir);
    d_ptr->container->addAnnotation("kaiming.base.dir", d_ptr->baseDir);
    d_ptr->container->addAnnotation("kaiming.runtime.dir", d_ptr->runtimeDir);
}

/**
 * @brief 挂载app,base,runtime目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountLayers()
{
    setAnnotationInfo();
    // 系统目录叠加挂载
    sysOverlay();
    // 应用目录挂载
    appBindMount();
    // 持久化存储家目录
    PerStorageHome();

    return true;
}

/**
 * @brief 系统目录叠加挂载
 */
void KMRunner::sysOverlay()
{
    /* mount overlayfs /proc /sys /dev /run属于特殊的内存文件系统,不属于具体的硬件设备,无须叠加
        /media /cdrom /mnt /home /root 属于数据保存目录，无需叠加  */
    for (auto &entry : fs::directory_iterator(d_ptr->baseDir + "/files"))
    {
        // 非目录不叠加
        if (!entry.is_directory())
        {
            continue;
        }
        std::string dir = "/" + entry.path().filename().string();
        if (std::find(OVERLAY_DIRS.begin(), OVERLAY_DIRS.end(), dir) != OVERLAY_DIRS.end())
        {
            overlayDir(dir);
        }
    }

    // 叠加share目录
    if (!overlayShare())
    {
        KMError("overlay share failed");
    }

    // 叠加挂载applications目录
    overlayApplications();

    // 叠加挂载libexec目录
    overlaylibexec();

    // 创建usr子目录软连接
    createUsrSymlinkSubdir("share", "../share");
    createUsrSymlinkSubdir("lib", "../lib");
    createUsrSymlinkSubdir("lib32", "../lib32");
    createUsrSymlinkSubdir("lib64", "../lib64");
    createUsrSymlinkSubdir("bin", "../bin");
    createUsrSymlinkSubdir("sbin", "../sbin");
    createUsrSymlinkSubdir("libexec", "../libexec");

    // 叠加挂载var目录
    overlayVar();
}

/**
 * @brief 叠加libexec目录
 */
void KMRunner::overlayApplications()
{
    // 叠加share目录
    std::vector<std::string> shareDirs = {
        "/opt/kaiming/share/applications",
        d_ptr->appDir + "/files/share/applications",
        d_ptr->appDir + "/files/usr/share/applications",
        d_ptr->appDir + "/depends/usr/share/applications",
        d_ptr->appDir + "/depends/share/applications",
        d_ptr->runtimeDir + "/files/usr/share/applications",
        d_ptr->baseDir + "/files/usr/share/applications",
        "/opt/kaiming/exports/share/applications",
        "/opt/system/resource/applications",
        "/opt/kare/usr/share/applications"
    };
        
    if (fs::exists("/tmp/ostree/usr-normal-mode"))
    {
        shareDirs.emplace_back("/tmp/ostree/usr-normal-mode/share/applications");
    }
    else
    {
        shareDirs.emplace_back("/usr/share/applications");
    }
    // 构造浏览器相关路径并插入到开头
    auto all = KMBrowserUtils::loadExecutablesFromFile();
    std::vector<std::string> browserPaths;
    for (auto &e : all)
    {
        // 只处理 appId 不为空的浏览器，并且排除当前应用本身
        if (!e.appId.empty() && d_ptr->appId != e.appId && !e.desktopPath.empty())
        {
            browserPaths.emplace_back(e.desktopPath);
        }
    }
    // 将所有浏览器路径插入到 shareDirs 的开头
    shareDirs.insert(shareDirs.begin(), browserPaths.begin(), browserPaths.end());

    mountOverlay(shareDirs, "/shareApplications", "/share/applications");
}

/**
 * @brief 叠加libexec目录
 */
void KMRunner::overlaylibexec()
{
    const std::vector<std::string> lowers = {
        KM_ORIGIN_PLUGINS_DIR + "/usr/libexec",
        KM_PLUGINS_DIR + "/usr/libexec",
        KARE_PLUGINS_DIR + "/usr/libexec",
        d_ptr->appDir + "/files/usr/libexec",
        d_ptr->appDir + "/depends/usr/libexec",
        d_ptr->runtimeDir + "/files/usr/libexec",
        d_ptr->baseDir + "/files/usr/libexec"
    };

    mountOverlay(lowers, "/libexec", "/libexec");
}

/**
 * @brief 叠加/var目录
 */
void KMRunner::overlayVar()
{
    const std::vector<std::string> lowers = {
        d_ptr->appDir + "/files/var",
        d_ptr->appDir + "/depends/var",
        d_ptr->runtimeDir + "/files/var",
        d_ptr->baseDir + "/files/var"
    };

    mountOverlay(lowers, "/var", "/var");

    // 特殊处理/var/run,/var/lock目录
    std::error_code ec;
    std::string upper = d_ptr->appDataPath + "/upperdir/var";
    if (!fs::exists(upper, ec))
    {
        fs::create_directories(upper, ec);
    }
    if (!fs::exists(upper + "/run", ec) && !fs::exists(upper + "/lock", ec))
    {
        // run目录需要特殊处理，此目录必须得是软链接
        fs::create_symlink("/run", upper + "/run", ec);
        fs::create_symlink("/run/lock", upper + "/lock", ec);
    }
}

/**
 * @brief 应用目录挂载
 */
void KMRunner::appBindMount()
{
    std::error_code ec;
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string appDir = d_ptr->appDir + "/files";
    if (fs::exists(appDir, ec))
    {
        // 使用app数据配置目录
        std::vector<std::string> dirs = { "/srv" };
        for (auto dir : dirs)
        {
            std::string upper = d_ptr->appDataPath + "/upperdir" + dir;
            if (!fs::exists(upper, ec))
            {
                fs::create_directories(upper, ec);
            }

            // 挂载目录
            mountDir(upper, dir, type, options);

            std::string srcDir = appDir + dir;
            if (!fs::exists(srcDir, ec))
            {
                continue;
            }

            for (const auto &entry : fs::directory_iterator(srcDir))
            {
                std::string filename = entry.path().filename().string();
                std::string subdir = upper + "/" + filename;
                std::string dstdir = dir + "/" + filename;

                // 软链接不挂载
                if (entry.is_symlink())
                {
                    std::string link = fs::read_symlink(entry.path(), ec);
                    fs::create_symlink(link, subdir, ec);
                }
                else // 非软链接才挂载
                {
                    mountDir(entry.path().string(), dstdir, type, options);
                }
            }
        }
        // 新标准化规范挂载
        if (!d_ptr->annotations.sysapp)
        {
            // 按照标准规范绑定挂载目录
            std::string dstDir = "/opt/apps/" + d_ptr->appId + "/files";
            d_ptr->container->addMount(Mount{ .source = appDir, .destination = dstDir, .type = type, .options = options });
        }
    }
}

/**
 * @brief 创建usr子目录软连接
 * @param link 软连接文件
 * @param target 软连接目标
 * @return true 成功 false 失败
 */
void KMRunner::createUsrSymlinkSubdir(const std::string &link, const std::string &target)
{
    // 创建挂载软连接
    std::error_code ec;
    std::string linkDir = d_ptr->appDataPath + "/upperdir/usr/" + link;
    if (fs::is_symlink(linkDir, ec))
    {
        return;
    }

    if (fs::is_directory(linkDir, ec))
    {
        fs::remove_all(linkDir, ec);
    }

    fs::create_symlink(target, linkDir, ec);
}

/**
 * @brief 持久化存储家目录信息
 * @return true 成功 false 失败
 */
bool KMRunner::PerStorageHome()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    mountDir("/root", "/root", type, options);

    uid_t uid = getuid();
    if (uid == 0)
    {
        // root级服务需要可以访问home,run目录
        mountDir("/home", "/home", type, options);
        d_ptr->isBindHostHome = true;
        return true;
    }

    if (d_ptr->perms.all || d_ptr->perms.xdgUserDirs)
    {
        mountDir(d_ptr->homeDir, d_ptr->homeDir, type, options);
        d_ptr->isBindHostHome = true;
        return true;
    }

    // 持久化存储不挂载家目录
    std::error_code ec;
    std::string homedir = d_ptr->appDataPath + "/upperdir" + d_ptr->homeDir;
    if (!fs::exists(homedir, ec))
    {
        fs::create_directories(homedir, ec);
    }
    d_ptr->container->addMount(Mount{ .source = homedir, .destination = d_ptr->homeDir, .type = type, .options = options });
    d_ptr->isBindHostHome = false;

    return true;
}

/**
 * @brief 设置process
 * @return true 成功 false 失败
 */
bool KMRunner::setProcessCmd()
{
    if (!setProcessCmdArgs())
    {
        KMError("set process command failed");
        return false;
    }

    if (!setProcessCap(defCap))
    {
        KMError("set default capability failed");
        return false;
    }

    if (!privileged())
    {
        KMError("privilege failed");
    }

    return true;
}

/**
 * @brief 挂载运行启动程序
 * @return true 成功 false 失败
 */
bool KMRunner::mountStartsh()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string dst = "/opt/apps/kaiming/bin/" + d_ptr->appId;
    std::string src = "/opt/kaiming-tools/bin/start.sh";
    mountDir(src, dst, type, options);
    return true;
}

/**
 * @brief 设置进程运行命令
 * @return true 成功 false 失败
 */
bool KMRunner::setProcessCmdArgs()
{
    std::string cwd;
    if (KMDirUtils::getCwdDir(cwd))
    {
        d_ptr->container->setProcessCwd(cwd);
    }
    else
    {
        d_ptr->container->setProcessCwd(d_ptr->homeDir);
    }

    if (d_ptr->runCommands.empty())
    {
        KMError("run command is empty");
        return false;
    }

    d_ptr->container->genRunDir(d_ptr->runDir, d_ptr->containerId);

    return true;
}

/**
 * @brief 提权
 * @return true 成功 false 失败
 */
bool KMRunner::privileged()
{
    // 是否提权
    if (!d_ptr->annotations.privilege)
    {
        return true;
    }

    // 去除user命名空间才能插入ko,fanotify等功能
    d_ptr->container->rmNameSpace("user");

    setProcessCap(allCap);

    return true;
}

/**
 * @brief 设置process的cap能力
 * @param cap 能力列表
 * @return true 成功 false 失败
 */
bool KMRunner::setProcessCap(const std::vector<int> &cap)
{
    d_ptr->container->setLinuxCapabilities(cap);
    d_ptr->container->setProcessBoundingCapabilities(cap);
    d_ptr->container->setProcessAmbientCapabilities(cap);
    d_ptr->container->setProcessEffectiveCapabilities(cap);

    if (d_ptr->annotations.privilege)
    {
        d_ptr->container->setProcessInheritableCapabilities(cap);
    }

    d_ptr->container->setProcessPermittedCapabilities(cap);

    return true;
}

/**
 * @brief 组装运行命令
 * @return true 成功 false 失败
 */
bool KMRunner::assembleCommand()
{
    std::string command, subCmd;
    if (!d_ptr->optionCommand.empty())
    {
        // optionCommand 本身保持原样
        command = d_ptr->optionCommand;
        for (const auto &arg : d_ptr->commandArgs)
        {
            subCmd = subCmd.empty() ? subCmd.append(arg) : subCmd.append(" ").append(arg);
        }
    }
    else
    {
        for (const auto &arg : d_ptr->infoCommands)
        {
            command = command.empty() ? command.append(arg) : command.append(" ").append(arg);
        }
    }

    if (command.empty())
    {
        KMError("command is empty");
        return false;
    }

    command.append(" ").append(parseRunCommand(subCmd));
    d_ptr->runCommands = command;

    int polkit = d_ptr->annotations.polkit ? 1 : 0;
    std::vector<std::string> args = { "/bin/bash", "-c" };
    std::string additionalCommand = "/opt/apps/kaiming/bin/" + d_ptr->appId + " " + d_ptr->appId + " " + std::to_string(polkit) + " " + d_ptr->runCommands;
    args.emplace_back(additionalCommand);
    d_ptr->container->setProcessArgs(args);

    return true;
}

/**
 * @brief 组装libgtk-3.so.0 提取版本号
 * @return true 成功 false 失败
 */
bool KMRunner::adaptGtkTheme()
{
    //组装libgtk-3.so.0路径
    std::string archDir = KMDirUtils::getLibraryDir();
    std::string linkPath = "/usr/lib/" + archDir + "/libgtk-3.so.0";

    //检查路径是否为软连接
    std::error_code ec;
    std::string gtkOutput = fs::is_symlink(linkPath, ec) ? fs::read_symlink(linkPath).string() : linkPath;
    std::vector<std::string> parts = KMStringUtils::splitString(gtkOutput, ".");

    if (parts.size() >= 4) //确保文件名格式正确，至少有4段
    {
        //取第三段的前两个数字
        std::string verNumstr = parts[3];
        int gtkVersion = std::stoi(verNumstr);
        if (gtkVersion < 2000)
        {
            return true;
        }
    }
    return false;
}

static std::vector<std::string> parseShellArgs(const std::string &cmd)
{
    std::vector<std::string> args;
    std::string current;
    bool inSingle = false;
    bool inDouble = false;
    for (size_t i = 0; i < cmd.size(); ++i)
    {
        char c = cmd[i];

        if (c == '"' && !inSingle)
        {
            inDouble = !inDouble;
            continue;
        }
        if (c == '\'' && !inDouble)
        {
            inSingle = !inSingle;
            continue;
        }
        if (c == ' ' && !inSingle && !inDouble)
        {
            if (!current.empty())
            {
                args.push_back(current);
                current.clear();
                inSingle = false;
                inDouble = false;
            }
            continue;
        }
        current.push_back(c);
    }
    if (!current.empty())
        args.push_back(current);

    return args;
}

/**
 * @brief 解析命令行，拆分可执行路径与参数，并处理路径前缀
 * @param commandLine 原始命令行
 * @return 拼装好的 runCommands
 */
std::string KMRunner::parseRunCommand(const std::string &commandLine)
{
    // 去掉前后空格
    std::string cleanCommand = commandLine;
    cleanCommand.erase(0, cleanCommand.find_first_not_of(" \t"));
    cleanCommand.erase(cleanCommand.find_last_not_of(" \t") + 1);

    // 用新的解析器（不会把双引号参数拆碎
    std::vector<std::string> argList;
    std::vector<std::string> rawArgs = parseShellArgs(cleanCommand);
    for (auto &token : rawArgs)
    {
        argList.emplace_back(token);
    }

    std::string args;
    for (auto &arg : argList)
    {
        std::string newArg = arg;
        // 路径前缀处理
        if (!newArg.empty() && KMStringUtils::startsWith(newArg, "/") && !(KMStringUtils::startsWith(newArg, "/home")) && !(KMStringUtils::startsWith(newArg, "/data")) && !(KMStringUtils::startsWith(newArg, "/run")))
        {
            if (fs::exists(newArg))
            {
                newArg = "/run/host/rootfs" + newArg;
            }
        }

        std::string escapedArg;
        for (char c : newArg)
        {
            if (c == '\'')
            {
                escapedArg += "'\\''";
            }
            else
            {
                escapedArg += c;
            }
        }

        // 使用单引号包裹参数，处理特殊字符$ `等
        args += " '" + escapedArg + "'";
    }
    return args;
}

/**
 * @brief 组装运行环境变量
 * @return true 成功 false 失败
 */
bool KMRunner::assembleProcEnv()
{
    Process process = d_ptr->container->getProcess();
    // 获取系统环境变量
    std::map<std::string, std::string> envs;
    for (auto &env : process.env)
    {
        auto key = env.substr(0, env.find_first_of('='));
        auto value = env.substr(env.find_first_of('=') + 1);
        envs[key] = value;
    }

    // 设置dbus环境变量
    std::error_code ec;
    std::map<std::string, std::string> newEnvs;
    auto *sysAddress = getenv("DBUS_SYSTEM_BUS_ADDRESS");
    newEnvs["DBUS_SYSTEM_BUS_ADDRESS"] = (sysAddress == nullptr) ? "unix:path=/run/dbus/system_bus_socket" : sysAddress;

    // 增加XDG相关环境变量
    auto homeDir = d_ptr->homeDir;
    auto *pXdgDataHome = getenv("XDG_DATA_HOME");
    newEnvs["XDG_DATA_HOME"] = (pXdgDataHome == nullptr) ? (homeDir + "/.local/share") : pXdgDataHome;
    newEnvs["XDG_CONFIG_HOME"] = homeDir + "/.config";
    newEnvs["XDG_CACHE_HOME"] = homeDir + "/.cache";
    newEnvs["XDG_STATE_HOME"] = homeDir + "/.local/state";
    std::string xdgConfig = "/opt/apps/" + d_ptr->appId + "/files/etc/xdg";
    std::string xdgData = "/opt/apps/" + d_ptr->appId + "/files/share";
    newEnvs["XDG_CONFIG_DIRS"] = xdgConfig + ":/etc/xdg:/etc/xdg/xdg-ukui";
    newEnvs["XDG_DATA_DIRS"] = xdgData + ":/usr/share:/usr/local/share:/usr/share/ukui:/share:/run/host/share:/run/host/local-share:/run/host/user-share";
    newEnvs["XDG_RUNTIME_DIR"] = d_ptr->xdgRuntimeDir;
    // 有些环境中/usr/bin/sensible-browser调用/usr/bin/x-www-browser失效，无法打开网页，故用环境变量BROWSER嫁接到使用/usr/bin/xdg-open打开
    newEnvs["BROWSER"] = "/usr/bin/xdg-open";

    // 设置认证相关环境变量
    auto *pXauthFile = getenv("XAUTHORITY");
    if (pXauthFile == nullptr)
    {
        std::string defaultXauth = homeDir + "/.Xauthority";
        if (fs::exists(defaultXauth, ec))
        {
            newEnvs["XAUTHORITY"] = defaultXauth;
        }
    }
    else
    {
        newEnvs["XAUTHORITY"] = pXauthFile;
    }

    // 设置桌面文件路径
    if (isatty(fileno(stdin)) == 1)
    {
        if (std::string appDesktopPath = "/opt/kaiming/exports/share/applications/" + d_ptr->appId + ".desktop"; fs::exists(appDesktopPath, ec))
        {
            newEnvs["PANSHI_APP_DESKTOP"] = appDesktopPath;
        }
        else if (appDesktopPath = "/opt/kaiming/share/applications/" + d_ptr->appId + ".desktop"; fs::exists(appDesktopPath, ec))
        {
            newEnvs["PANSHI_APP_DESKTOP"] = appDesktopPath;
        }
    }
    else
    {
        auto desktopFile = getenv("GIO_LAUNCHED_DESKTOP_FILE");
        if (desktopFile != nullptr)
        {
            newEnvs["PANSHI_APP_DESKTOP"] = desktopFile;
            newEnvs["GIO_LAUNCHED_DESKTOP_FILE"] = desktopFile;
        }
    }

    // 设置Display
    auto *display = getenv("DISPLAY");
    newEnvs["DISPLAY"] = (display == nullptr) ? ":0" : display;

    // 设置SessionManager环境变量
    auto *sessionManagerEnv = getenv("SESSION_MANAGER");
    if (sessionManagerEnv != nullptr)
    {
        newEnvs["SESSION_MANAGER"] = sessionManagerEnv;
    }

    // 设置Version环境变量
    newEnvs["APP_VERSION"] = d_ptr->version;

    // 主要编包环境变量
    newEnvs["KAIMING_ID"] = d_ptr->appId;

    // 增加QT相关环境变量
    newEnvs["QML2_IMPORT_PATH"] = std::string("/lib64/qml:") + "/lib/qml";
    std::string platLibDir = "/usr/lib/" + KMDirUtils::getLibraryDir();
    std::string glExtendDir = "/etc/egl/egl_external_platform.d:" + platLibDir + "/GL/egl/egl_external_platform.d:" + "/usr/share/egl/egl_external_platform.d";
    newEnvs["__EGL_EXTERNAL_PLATFORM_CONFIG_DIRS"] = glExtendDir;
    newEnvs["QT_ACCESSIBILITY"] = "1";
    newEnvs["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1";

    // 区分root和普通用户登录时挂载信息
    if (uid_t uid; KMDirUtils::getLoginUid(uid))
    {
        std::string xdgRun = "/run/user/" + std::to_string(uid);
        std::string sock = "/tmp/kmsessdbusproxy-" + std::to_string(uid) + ".sock";
        if (d_ptr->annotations.tray && getuid() != 0 && fs::exists(sock))
        {
            newEnvs["DBUS_SESSION_BUS_ADDRESS"] = "unix:path=" + sock;
        }
        else
        {
            auto sessionAddress = getenv("DBUS_SESSION_BUS_ADDRESS");
            newEnvs["DBUS_SESSION_BUS_ADDRESS"] = (sessionAddress == nullptr) ? ("unix:path=" + xdgRun + "/bus") : sessionAddress;
        }

        // pulse 相关配置
        std::string pulseDir = xdgRun + "/pulse";
        std::string pulseSocket = pulseDir + "/native";
        if (!fs::exists(pulseSocket, ec))
        {
            if (fs::exists("/var/run/pulse/native", ec))
            {
                pulseSocket = "/var/run/pulse/native";
            }
            else
            {
                pulseSocket = "";
            }
        }

        if (!pulseSocket.empty())
        {
            d_ptr->pulseSocket = pulseSocket;
            newEnvs["PULSE_SERVER"] = pulseSocket;
            if (std::string pulseDir; KMDirUtils::getPulseRuntimeDir(pulseDir))
            {
                newEnvs["PULSE_CLIENTCONFIG"] = pulseDir.append("/config");
            }
        }
    }

    // 增加app应用中的环境变量
    std::map<std::string, std::string> infoEnvs = d_ptr->infoEnvs;

    // 合并处理环境变量
    KMReference ref;
    ref.appendEnvs(envs, newEnvs);
    ref.appendEnvs(envs, infoEnvs);

    // 处理环境变量中的变量引用
    for (auto &[k, v] : envs)
    {
        auto it = v.find("$" + k);
        if (it != std::string::npos)
        {
            if (v.find("$" + k + ":") != std::string::npos)
            {
                v.replace(it, k.length() + 2, "");
            }
            else
            {
                v.replace(it, k.length() + 1, "");
            }
        }
    }

    // 后续的PATH和LD_LIBRARY_PATH不能完全被用户设置的替换
    newEnvs.clear();

    // 设置PATH环境变量
    std::string valueOfpath;
    for (const auto &[k, v] : envs)
    {
        if (k == "PATH")
        {
            valueOfpath = v;
        }
    }
    std::string pathEnv = "/usr/bin:/usr/sbin:/bin:/sbin:/usr/local/bin:/usr/local/sbin:/usr/libexec:/usr/games:/opt/kaiming/bin";
    std::string standardOptPath = "/opt/apps/" + d_ptr->appId + "/files/bin" + ":" + "/opt/apps/" + d_ptr->appId + "/files/local/bin:";
    std::string extendPath = valueOfpath.empty() ? "" : (valueOfpath + ":");
    newEnvs["PATH"] = extendPath + pathEnv;
    if (!d_ptr->annotations.sysapp)
    {
        newEnvs["PATH"] = standardOptPath + extendPath + pathEnv;
    }

    // 设置LD_LIBRARY_PATH
    std::string valueOfldlibrarypath;
    for (const auto &[k, v] : envs)
    {
        if (k == "LD_LIBRARY_PATH")
        {
            valueOfldlibrarypath = v;
        }
    }

    //根据GTK版本设置GTK_THEME环境变量
    if (adaptGtkTheme())
    {
        newEnvs["GTK_THEME"] = "Adwaita";
    }

    std::string defLibraryPath = "/usr/lib:/lib:/usr/local/lib:/usr/lib64:/usr/lib32:/lib64:/lib32";
    if (!valueOfldlibrarypath.empty())
    {
        defLibraryPath = valueOfldlibrarypath + ":" + defLibraryPath;
    }
    newEnvs["LD_LIBRARY_PATH"] = defLibraryPath;
    if (!d_ptr->annotations.sysapp)
    {
        std::string standard = "/opt/apps/" + d_ptr->appId + "/files/lib" + ":" + "/opt/apps/" + d_ptr->appId + "/files/local/lib:";
        newEnvs["LD_LIBRARY_PATH"] = standard + defLibraryPath;
    }

    // 更新环境变量,主要修改PATH和LD_LIBRARY_PATH
    for (const auto &[k, v] : newEnvs)
    {
        envs[k] = v;
    }

    // 语言类环境变量设置
    std::vector<std::string> langEnvs;
    auto *envLang = getenv("LANG");
    auto *envLanguage = getenv("LANGUAGE");

    if (envLang && envLanguage)
    {
        langEnvs.emplace_back(std::string("LANG=") + envLang);
        langEnvs.emplace_back(std::string("LANGUAGE=") + envLanguage);
    }

    if (langEnvs.empty())
    {
        std::ifstream file("/etc/locale.conf");
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                KMStringUtils::replace(line, "\"", "");
                langEnvs.emplace_back(line);
            }
        }
    }

    for (auto &lang : langEnvs)
    {
        d_ptr->envs.emplace_back(lang);
    }

    // 修改环境变量格式
    for (const auto &[k, v] : envs)
    {
        d_ptr->envs.emplace_back(k + "=" + v);
    }

    // 如果需要宿主机插件覆盖，添加 LD_PRELOAD 环境变量
    if (d_ptr->annotations.hostPlugins)
    {
        std::error_code ec;
        std::string preloadPath = "/opt/kaiming-tools/lib/libvfspreload.so";
        if (fs::exists(preloadPath, ec))
        {
            d_ptr->envs.emplace_back("LD_PRELOAD=/usr/lib/libvfspreload.so");
        }
        else
        {
            KMInfo("/usr/lib/libvfspreload.so not found");
        }
    }

    d_ptr->container->setProcessEnv(d_ptr->envs);

    return true;
}

/**
 * @brief 挂载基础目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountBase()
{
    std::string dst, src, type;
    std::vector<std::string> options;

    dst = "/proc";
    src = "/proc";
    type = "bind";
    options = { "rbind", "rprivate" };
    d_ptr->container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = options });

    dst = "/tmp";
    src = "/tmp";
    type = "bind";
    options = { "rbind", "rprivate" };
    mountDir(src, dst, type, options);

    dst = "/sys";
    src = "/sys";
    type = "bind";
    options = { "rbind", "rprivate" };
    mountDir(src, dst, type, options);

    // 挂载用户运行信息
    src = "/run";
    dst = src;
    options = { "rbind", "rprivate" };
    mountDir(src, dst, type, options);

    return true;
}

/**
 * @brief 挂载var基础目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountVar()
{
    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string type = "bind";

    // 挂载var临时目录
    mountDir("/var/tmp", "/var/tmp", type, options);

    // 挂载var缓存目录
    mountDir("/var/cache", "/var/cache", type, options);

    // 挂载var日志目录
    mountDir("/var/log", "/var/log", type, options);

    return true;
}

/**
 * @brief 挂载dev设备
 * @return true 成功 false 失败
 */
bool KMRunner::mountDev()
{
    std::string src, dst, type;
    std::vector<std::string> options;
    // 挂载所有设备需要按需挂载
    dst = "/dev";
    src = "/dev";
    type = "bind";
    options = { "rbind", "rprivate" };
    mountDir(src, dst, type, options);

    if ((d_ptr->perms.all || d_ptr->perms.microphone) && (!mountPulseaudio() || !mountAlsa()))
    {
        KMError("mount audio config failed");
        return false;
    }

    if (!d_ptr->perms.all)
    {
        // 去除管控不需要挂载的设备
        std::vector<std::string> controlDevice{};
        if (!d_ptr->perms.camera)
        {
            controlDevice.emplace_back("/dev/video");
        }
        if (!d_ptr->perms.serial)
        {
            controlDevice.emplace_back("/dev/ttyS");
            controlDevice.emplace_back("/dev/ttyUSB");
        }
        if (!d_ptr->perms.microphone)
        {
            controlDevice.emplace_back("/dev/snd");
        }

        for (auto const &entry : fs::directory_iterator{ "/dev" })
        {
            std::string dev = entry.path().string();
            bool isControlDevice = false;
            for (auto const &control : controlDevice)
            {
                if (dev.find(control) != std::string::npos)
                {
                    isControlDevice = true;
                    break;
                }
            }
            if (!isControlDevice)
            {
                continue;
            }
            permDev(dev);
        }
    }

    return true;
}

/**
 * @brief 管控设备文件
 * @return true 成功 false 失败
 */
bool KMRunner::permDev(std::string const &dev)
{
    std::error_code ec;
    std::string devDir = d_ptr->appDataPath + "/upperdir/dev";
    if (!fs::exists(devDir, ec))
    {
        fs::create_directories(devDir, ec);
    }

    std::vector<std::string> options;
    std::string type = "bind";
    if (fs::is_directory(dev))
    {
        options = { "nosuid", "strictatime", "mode=0777" };
        d_ptr->container->addMount(Mount{ .source = "tmpfs", .destination = dev, .type = "tmpfs", .options = options });
    }
    else
    {
        std::string ovl = d_ptr->appDataPath + "/upperdir" + dev;
        if (!fs::exists(ovl, ec))
        {
            std::ofstream(ovl, std::ios::trunc);
        }
        options = { "rbind", "rprivate" };
        d_ptr->container->addMount(Mount{ .source = ovl, .destination = dev, .type = type, .options = options });
    }

    return true;
}

/**
 * @brief 挂载socket
 * @return true 成功 false 失败
 */
bool KMRunner::mountSocket()
{
    if (!mountX11())
    {
        KMError("mount x11 failed");
        return false;
    }

    if (!mountWayland())
    {
        KMError("mount wayland failed");
        return false;
    }

    if (!mountDbus())
    {
        KMError("mount dbus failed");
        return false;
    }

    return true;
}

/**
 * @brief 挂载overlay 
 * @param lowers目录
 * @param upper目录
 * @param merge目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountOverlay(const std::vector<std::string> &lowers, const std::string &upper, const std::string &merge)
{
    return KMRunnerMount::mountOverlay(d_ptr->container.get(), d_ptr->appDataPath, lowers, upper, merge);
}

/**
 * @brief 挂载share目录
 * @return true 成功 false 失败
 */
bool KMRunner::overlayShare()
{
    // 叠加share目录
    std::vector<std::string> shareDirs = {
        d_ptr->appDir + "/files/share",
        d_ptr->appDir + "/files/usr/share",
        d_ptr->appDir + "/depends/usr/share",
        d_ptr->appDir + "/depends/share",
        d_ptr->runtimeDir + "/files/usr/share",
        d_ptr->baseDir + "/files/usr/share",
        "/opt/kaiming/exports/share",
        "/opt/kaiming/share",
        "/opt/system/resource",
        "/opt/kare/usr/share"
    };

    if (fs::exists("/tmp/ostree/usr-normal-mode"))
    {
        shareDirs.emplace_back("/tmp/ostree/usr-normal-mode/share");
    }
    else
    {
        shareDirs.emplace_back("/usr/share");
    }

    return mountOverlay(shareDirs, "/share", "/share");
}

/**
 * @brief 挂载share,包含字体,icon等
 * @return true 成功 false 失败
 */
bool KMRunner::mountShare()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };

    /* 宿主机/usr/share已叠加，故不需要再次绑定挂载 */

    // 挂载字体配置
    mountDir("/etc/fonts", "/etc/fonts", type, options);

    // 挂载字体
    mountDir("/usr/local/share/fonts", "/usr/local/share/fonts", type, options);

    // 挂载用户配置
    std::string userlocalShare = KMDirUtils::getUserDataDir();
    mountDir(userlocalShare, userlocalShare, type, options);

    std::string homeFontPath = d_ptr->homeDir + "/.fonts";
    mountDir(homeFontPath, homeFontPath, type, options);

    // 挂载体缓存
    std::string homecache = KMDirUtils::getUserCacheDir();
    std::vector<std::string> fontDirs = { "/var/cache/fontconfig", "/usr/lib/fontconfig/cache", homecache.append("/fontconfig") };
    for (auto const &config : fontDirs)
    {
        mountDir(config, config, type, options);
    }

    // 挂载用户目录缓存
    if (!d_ptr->perms.all)
    {
        std::string cacheDir = d_ptr->homeDir + "/.cache";
        mountDir(cacheDir, cacheDir, type, options);
    }

    // 挂载主机配置到容器内,方便xdg桌面应用使用
    mountDir("/usr/share", "/run/host/share", type, options);
    mountDir("/usr/local/share", "/run/host/local-share", type, options);
    mountDir("/", "/run/host/rootfs", type, options);
    mountDir(userlocalShare, "/run/host/user-share", type, options);

    return true;
}

/**
 * @brief 叠加目录
 * @param dir overlay目录路径
 * @param isHost 是否叠加宿主机目录
 */
void KMRunner::overlayDir(const std::string &dir)
{
    const std::vector<std::string> dirs = {
        KM_ORIGIN_PLUGINS_DIR + dir,
        KM_ORIGIN_PLUGINS_DIR + "/usr" + dir,
        KM_PLUGINS_DIR + dir,
        KM_PLUGINS_DIR + "/usr" + dir,
        KARE_PLUGINS_DIR + dir,
        KARE_PLUGINS_DIR + "/usr" + dir,
        d_ptr->appDir + "/files" + dir,
        d_ptr->appDir + "/files/usr" + dir,
        d_ptr->appDir + "/depends" + dir,
        d_ptr->appDir + "/depends/usr" + dir,
        d_ptr->runtimeDir + "/files" + dir,
        d_ptr->runtimeDir + "/files/usr" + dir,
        d_ptr->baseDir + "/files" + dir,
        d_ptr->baseDir + "/files/usr" + dir
    };
    std::vector<std::string> lowers;
    std::error_code ec;
    for (auto &dir : dirs)
    {
        if (!fs::is_symlink(dir, ec))
        {
            lowers.emplace_back(dir);
        }
    }

    mountOverlay(lowers, dir, dir);
}

/**
 * @brief 挂载pulseaudio配置
 * @return true 成功 false 失败
 */
bool KMRunner::mountPulseaudio()
{
    if (!d_ptr->pulseSocket.empty())
    {
        // 设置挂载
        std::string parentDir = fs::path(d_ptr->pulseSocket).parent_path().string();
        std::string pulseConfig = "/opt/kaiming-tools/config/pulse-config";
        std::string configPath = parentDir + "/config";
        std::string type = "bind";
        std::vector<std::string> options = { "rbind", "rprivate" };
        mountDir(d_ptr->pulseSocket, d_ptr->pulseSocket, type, options);
        mountDir(pulseConfig, configPath, type, options);
    }

    return true;
}

/**
 * @brief 挂载alsa配置
 * @return true 成功 false 失败
 */
bool KMRunner::mountAlsa()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    mountDir("/usr/share/alsa", "/usr/share/alsa", type, options);
    mountDir("/etc/alsa", "/etc/alsa", type, options);
    return true;
}

/**
 * @brief 挂载dbus配置
 * @return true 成功 false 失败
 */
bool KMRunner::mountDbus()
{
    auto *systemBusEnv = getenv("DBUS_SYSTEM_BUS_ADDRESS");
    std::string systemBus = "/var/run/dbus/system_bus_socket";
    if (systemBusEnv != nullptr && std::filesystem::exists(systemBusEnv))
    {
        systemBus = systemBusEnv;
    }

    if (!std::filesystem::exists(systemBus))
    {
        KMError("dbus system bus socket not found at " + systemBus);
        return false;
    }

    std::string type = "bind";
    std::vector<std::string> roOptions = { "rbind", "ro", "rprivate" };
    d_ptr->container->addMount(Mount{ .source = systemBus, .destination = "/run/dbus/system_bus_socket", .type = type, .options = roOptions });

    // 只有普通用户才能挂载session bus
    uid_t uid = getuid();
    if (uid != 0)
    {
        // 挂载user session bus地址
        auto sessionAddress = getenv("DBUS_SESSION_BUS_ADDRESS");
        if (sessionAddress != nullptr)
        {
            std::string sessionAddrStr = sessionAddress;

            // 检查是否为抽象地址
            std::regex abstractRegex(R"(unix:abstract=(/?.+))");
            std::smatch match;
            if (!std::regex_search(sessionAddrStr, match, abstractRegex))
            {
                // 非抽象地址，解析为路径
                std::string sessionPath = pathFromAddress(sessionAddrStr);
                if (sessionPath.empty() || !std::filesystem::exists(sessionPath))
                {
                    KMError("DBus session bus path: " + sessionPath + " not found");
                    return false;
                }

                d_ptr->container->addMount(Mount{ .source = sessionPath, .destination = sessionPath, .type = type, .options = roOptions });
            }
        }
        else
        {
            std::error_code ec;
            std::string session = d_ptr->xdgRuntimeDir + "/bus";
            if (!fs::exists(session, ec))
            {
                KMInfo("DBUS_SESSION_BUS_ADDRESS not found");
            }
            else
            {
                mountDir(session, session, type, roOptions);
            }
        }
    }

    return true;
}

/**
 * @brief : 挂载x11
 * @return true 成功 false 失败
 */
bool KMRunner::mountX11()
{
    // 因为/tmp默认挂载,所以此挂载注释掉即可
    // string x11Socket = "/tmp/.X11-unix";
    return true;
}

/**
 * @brief 挂载wayland
 * @return true 成功 false 失败
 */
bool KMRunner::mountWayland()
{
    auto *display = getenv("WAYLAND_DISPLAY");
    if (display == nullptr)
    {
        return true;
    }

    std::string waylandDisplay = display;
    std::string waylandSocker;
    if (waylandDisplay.at(0) == '/')
    {
        waylandSocker = waylandDisplay;
    }
    else
    {
        waylandSocker = d_ptr->xdgRuntimeDir + "/" + waylandDisplay;
    }

    std::vector<std::string> options = { "rbind", "ro", "rprivate" };
    mountDir(waylandSocker, waylandSocker, "bind", options);

    // 解决wayland兼容x11问题
    mountX11Wayland();

    return true;
}

/**
 * @brief 挂载x11-wayland兼容层
 */
void KMRunner::mountX11Wayland()
{
    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string type = "bind";
    std::string src = "/opt/kaiming-tools/bin/x11-ext.sh";
    std::string dst = "/opt/x11-wayland/x11-ext.sh";
    mountDir(src, dst, type, options);
}

/**
 * @brief 挂载数据配置
 * @return true 成功 false 失败
 */
bool KMRunner::mountData()
{
    if (!mountVar())
    {
        KMError("mount var failed");
        return false;
    }

    if (!mountEtc())
    {
        KMError("mount etc failed");
        return false;
    }

    if (!mountSocket())
    {
        KMError("mount socket failed");
        return false;
    }

    if (!mountShare())
    {
        KMError("mount share failed");
        return false;
    }

    if (!mountDconf())
    {
        KMError("mount dconf failed");
        return false;
    }

    if (!mountXauthority())
    {
        KMError("mount xauthority failed");
        return false;
    }

    if (!mountShareData())
    {
        KMError("mount share data directory failed");
        return false;
    }

    if (!mountDependAppsDir())
    {
        // 非重要致命性挂载,即使失败也不应该返回
        KMError("mount depend apps directory failed");
    }

    if (!mountStartsh())
    {
        KMError("mount crun config dir failed");
        return false;
    }

    return true;
}

/**
 * @brief 挂载依赖应用目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountDependAppsDir()
{
    // 挂载依赖应用目录
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "ro", "rprivate" };
    std::string src = "/opt/kaiming/bin";
    mountDir(src, src, type, options);

    // 挂载dbus客户端
    std::string srcCli = "/opt/kaiming-tools/bin/dbus-cli.sh";
    std::string dst = "/opt/kaiming-tools/bin/kaiming";
    mountDir(srcCli, dst, type, options);

    return true;
}

/**
 * @brief 处理依赖应用问题
 * @return true 成功 false 失败
 */
bool KMRunner::mountDepandApps()
{
    // xdg-open 默认兼容
    d_ptr->annotations.dependApps.emplace_back("/usr/bin/xdg-open");
    d_ptr->annotations.dependApps.emplace_back("/usr/bin/boxsm");
    d_ptr->annotations.dependApps.emplace_back("/usr/bin/boxmount");
    d_ptr->annotations.dependApps.emplace_back("/usr/bin/boxumount");

    // 浏览器默认兼容
    auto all = KMBrowserUtils::loadExecutablesFromFile();
    for (auto &e : all)
    {
        if (d_ptr->appId == e.appId)
        {
            continue;
        }
        d_ptr->annotations.dependApps.emplace_back(e.execPath);
    }

    // 挂载依赖
    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string type = "bind";
    std::string proxyCli = "/opt/kaiming-tools/bin/dbus-cli.sh";
    for (auto &app : d_ptr->annotations.dependApps)
    {
        // 若不是绝对路径，则添加默认路径
        if (app.at(0) != '/')
        {
            app = "/usr/bin/" + app;
        }

        mountDir(proxyCli, app, type, options);
    }

    return true;
}

/**
 * @brief 挂载共享目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountShareData()
{
    // 挂载用户数据目录
    std::error_code ec;
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    if (fs::exists("/data", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/data", .destination = "/data", .type = type, .options = options });
    }
    if (fs::exists("/srv", ec) && !fs::exists(d_ptr->appDir + "/files/srv", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/srv", .destination = "/srv", .type = type, .options = options });
    }

    // 处理移动存储设备
    d_ptr->container->setRootfsPropagation("shared"); // 设置/根挂载属性为从属,即只能接收传播,不能对外传播
    options = { "rbind", "rshared" };
    if (fs::exists("/cdrom", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/cdrom", .destination = "/cdrom", .type = type, .options = options });
    }
    if (fs::exists("/media", ec) && !fs::is_symlink("/media", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/media", .destination = "/media", .type = type, .options = options });
    }
    if (fs::exists("/run/media", ec) && !fs::is_symlink("/run/media", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/run/media", .destination = "/run/media", .type = type, .options = options });
    }
    if (fs::exists("/mnt", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/mnt", .destination = "/mnt", .type = type, .options = options });
    }

    return true;
}

/**
 * @brief 挂载许可
 * @return true 成功 false 失败
 */
bool KMRunner::mountXauthority()
{
    // 此配置不应该影响应用的正常启动
    if (getuid() == 0)
        return true;

    if (d_ptr->isBindHostHome)
        return true;

    auto *pXauthFile = getenv("XAUTHORITY");
    std::string xauthFile = (pXauthFile == nullptr) ? (d_ptr->homeDir + "/.Xauthority") : pXauthFile;

    std::vector<std::string> options = { "rbind", "rprivate" };
    mountDir(xauthFile, xauthFile, "bind", options);

    return true;
}

/**
 * @brief 权限管控
 * @return true 成功 false 失败
 */
bool KMRunner::permissionControl()
{
    if (!mountFileSystem())
    {
        KMError("mount specified path failed");
        return false;
    }

    if (!mountAutoStart())
    {
        KMError("mount autostart failed");
        return false;
    }

    if (!setNetwork())
    {
        KMError("mount autostart failed");
        return false;
    }

    if (!mountDev())
    {
        KMError("mount dev failed");
        return false;
    }

    if (!bluetoothRestricted())
    {
        KMError("bluetooth control failed");
        return false;
    }

    return true;
}

bool KMRunner::bluetoothRestricted()
{
    if (d_ptr->perms.all || d_ptr->perms.bluetooth)
    {
        return true;
    }

    Seccomp seccomp;
    seccomp.defaultAction = "SCMP_ACT_ALLOW";
    seccomp.architectures = { "SCMP_ARCH_X86_64", "SCMP_ARCH_ARM", "SCMP_ARCH_AARCH64" };

    Syscall syscall;
    syscall.names = { "socket" };
    syscall.action = "SCMP_ACT_ERRNO";
    syscall.errnoRet = 1;

    SyscallArg arg;
    arg.index = 0;
    arg.op = "SCMP_CMP_EQ";
    arg.value = 31;

    syscall.args.emplace_back(arg);
    seccomp.syscalls.emplace_back(syscall);

    if (!d_ptr->container->setSeccomp(seccomp))
    {
        KMError("failed to set seccomp for bluetooth");
    }

    return true;
}

/**
 * @brief 设置网络访问权限
 * @return true 成功 false 失败
 */
bool KMRunner::setNetwork()
{
    if (!d_ptr->perms.all && !d_ptr->perms.network)
    {
        d_ptr->container->addNameSpace("network");
    }
    return true;
}

/**
 * @brief 挂载指定目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountFileSystem()
{
    // 绑定指定路径挂载
    std::error_code ec;
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    for (auto &specdir : d_ptr->perms.filesystemDirs)
    {
        if (specdir[0] == '~')
        {
            if (specdir.length() > 1)
            {
                specdir = d_ptr->homeDir + specdir.substr(1);
            }
            else
            {
                specdir = d_ptr->homeDir;
            }
        }

        if (!fs::exists(specdir, ec))
        {
            continue;
        }

        if ((specdir == d_ptr->homeDir || specdir == (d_ptr->homeDir + "/")))
        {
            if (!d_ptr->isBindHostHome)
            {
                mountDir(specdir, specdir, type, options);
                d_ptr->isBindHostHome = true;
            }
            continue;
        }

        mountDir(specdir, specdir, type, options);
    }

    return true;
}

/**
 * @brief 挂载box目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountBox()
{
    std::string box = d_ptr->homeDir + "/.box";
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rshared" };
    mountDir(box, box, type, options);
    return true;
}

/**
 * @brief 挂载covers目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountCover()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    for (const auto &[key, value] : d_ptr->annotations.covers)
    {
        mountDir(key, value, type, options);
    }

    return true;
}

/**
 * @brief 挂载自动启动目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountAutoStart()
{
    if (!d_ptr->perms.all && !d_ptr->perms.autostart)
    {
        return true;
    }

    std::vector<std::string> options = { "rbind", "rprivate" };
    std::string type = "bind";
    std::string etcAutoStart = "/etc/xdg/autostart";
    mountDir(etcAutoStart, etcAutoStart, type, options);
    std::string homeAutoStart = d_ptr->homeDir + "/.config/autostart";
    mountDir(homeAutoStart, homeAutoStart, type, options);

    return true;
}

/**
 * @brief 挂载dconf目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountDconf()
{
    std::string type = "bind";
    std::vector<std::string> options = { "rbind", "rprivate" };
    if (getuid() == 0)
    {
        return true;
    }

    // 挂载 dconf

    std::string dconf = d_ptr->homeDir + "/.config/dconf";
    std::error_code ec;
    if (fs::exists(dconf, ec))
    {
        d_ptr->container->addMount(Mount{ .source = dconf, .destination = dconf, .type = type, .options = options });

        std::string userDirs = d_ptr->homeDir + "/.config/user-dirs.dirs";
        mountDir(userDirs, userDirs, type, options);

        std::string userDirsLocale = d_ptr->homeDir + "/.config/user-dirs.locale";
        mountDir(userDirsLocale, userDirsLocale, type, options);
    }

    return true;
}

/**
 * @brief 挂载etc目录
 * @return true 成功 false 失败
 */
bool KMRunner::mountEtc()
{
    std::string type = "bind";
    std::vector<std::string> roOptions = { "rbind", "ro", "rprivate" };
    // 挂载账户信息
    mountDir("/etc/passwd", "/etc/passwd", type, roOptions);
    mountDir("/etc/group", "/etc/group", type, roOptions);

    std::error_code ec;
    if (!fs::exists(d_ptr->baseDir + "/files/pkcs11/pkcs11.conf", ec))
    {
        std::string pkcsconf = d_ptr->cacheDir + "/pkcs11.conf";
        if (!fs::exists(pkcsconf, ec))
        {
            std::ofstream ofs(pkcsconf, std::ios::out | std::ios::trunc);
            if (!ofs.is_open())
            {
                ofs.close();
                KMError("create pkcs11.conf file failed");
                return false;
            }
            ofs << "# Disable user pkcs11 config, because the host modules don't work in the runtime\n"
                << std::endl;
            ofs << "user-config: none" << std::endl;
            ofs.close();
        }

        mountDir(pkcsconf, "/etc/pkcs11/pkcs11.conf", type, roOptions);
    }

    // 处理/etc/mtab
    std::string mtab = d_ptr->appDataPath + "/upperdir/etc/mtab";
    fs::create_symlink("../proc/self/mounts", mtab, ec);

    // 挂载文管配置
    mountDir("/etc/xdg/peony-data.conf", "/etc/xdg/peony-data.conf", type, roOptions);

    // 挂载机器ID
    mountDir("/etc/machine-id", "/etc/machine-id", type, roOptions);
    mountDir("/var/lib/dbus/machine-id", "/var/lib/dbus/machine-id", type, roOptions);

    // 挂载resolv路由
    if (fs::is_symlink("/etc/resolv.conf", ec)) // 软链接不挂载
    {
        std::string resolv = d_ptr->appDataPath + "/upperdir/etc/resolv.conf";
        if (!fs::is_symlink(resolv, ec))
        {
            fs::remove(resolv, ec);
            std::string link = fs::read_symlink("/etc/resolv.conf", ec);
            fs::create_symlink(link, resolv, ec);
        }
    }
    else // 非软链接才挂载
    {
        mountDir("/etc/resolv.conf", "/etc/resolv.conf", type, roOptions);
    }

    // 挂载host信息
    mountDir("/etc/host.conf", "/etc/host.conf", type, roOptions);
    mountDir("/etc/hosts", "/etc/hosts", type, roOptions);
    mountDir("/etc/hostname", "/etc/hostname", type, roOptions);

    // 挂载gai.conf
    mountDir("/etc/gai.conf", "/etc/gai.conf", type, roOptions);

    // 设置系统时间挂载参数，如果系统的存在用系统的，系统的不存在，用 runtime/files 下的
    std::string rawTimeZone = KMDirUtils::getTimeZone();
    std::string localtimeZone = "/usr/share/zoneinfo/" + rawTimeZone;
    if (fs::exists("/usr/share/zoneinfo", ec))
    {
        d_ptr->container->addMount(Mount{ .source = "/usr/share/zoneinfo", .destination = "/usr/share/zoneinfo", .type = type, .options = roOptions });
        if (fs::exists(localtimeZone, ec))
        {
            std::string upperLocaltime = d_ptr->appDataPath + "/upperdir/etc";
            if (fs::exists(upperLocaltime, ec))
            {
                fs::create_symlink(localtimeZone, upperLocaltime + "/localtime", ec);
            }
        }
    }

    if (fs::exists("/etc/timezone, ec"))
    {
        d_ptr->container->addMount(Mount{ .source = "/etc/timezone", .destination = "/etc/timezone", .type = type, .options = roOptions });
    }
    else
    {
        std::string src = "/var/db/zoneinfo";
        std::string dst = "/etc/timezone";
        if (fs::exists(src, ec))
        {
            d_ptr->container->addMount(Mount{ .source = src, .destination = dst, .type = type, .options = roOptions });
        }
        else
        {
            // 挂载/etc/timezone
            std::string timezoneFile = d_ptr->cacheDir + "/timezone";
            if (!fs::exists(timezoneFile, ec))
            {
                std::ofstream ofs(timezoneFile, std::ios::out | std::ios::trunc);
                if (!ofs.is_open())
                {
                    ofs.close();
                    KMError("create timezone file failed");
                    return false;
                }
                ofs << rawTimeZone << std::endl;
                ofs.close();
            }

            mountDir(timezoneFile, "/etc/timezone", type, roOptions);
        }
    }

    return true;
}

/**
 *@brief 解析dbus地址
 *@return string 路径
 */
std::string KMRunner::pathFromAddress(const std::string &address) const
{
    if (address.empty())
        return "";

    if (address.find("unix:") == std::string::npos)
        return "";

    if (address.find("path=") == std::string::npos)
        return "";

    std::string path = address.substr(address.find("path=") + strlen("path="), address.length() - address.find("path="));
    if (path.empty())
        return "";

    if (path.find(",") != std::string::npos)
        path = path.substr(0, path.find(","));

    return path;
}

/**
 *@brief 挂载指定目录
 */
void KMRunner::mountDir(const std::string &hostDir, const std::string &containDir, const std::string &type, std::vector<std::string> &options)
{
    KMRunnerMount::mountDir(d_ptr->container.get(), d_ptr->appDataPath, d_ptr->isBindHostHome, hostDir, containDir, type, options);
}

/**
 * @brief 家目录是否存在
 * @return true 存在 false 不存在
 */
bool KMRunner::isExistHomeDir()
{
    return KMDirUtils::getHomeDir(d_ptr->homeDir);
}

/**
 * @brief XDG_RUNTIME_DIR是否存在
 * @return true 存在 false 不存在
 */
bool KMRunner::isExistsXdgRuntimeDir()
{
    return KMDirUtils::getXdgRuntime(d_ptr->xdgRuntimeDir);
}

/**
 * @brief 启动Unix Socket服务器
 * @return true 存在 false 不存在
 */
bool KMRunner::startUnixSocketServer()
{
    if (!d_ptr->server.create())
        return false;

    d_ptr->sockPath = d_ptr->server.getPath();

    return d_ptr->server.run();
}

/**
 * @brief 挂载GPU显卡驱动 
 * @return true 存在 false 不存在
 */
bool KMRunner::mountGpu()
{
    if (!d_ptr->annotations.gpu)
    {
        return true;
    }

    std::string archDir = KMDirUtils::getLibraryDir();
    std::string upperDir = d_ptr->appDataPath + "/upperdir";
    std::string hostDir = "/run/host/rootfs";

    for (auto &gpu : gpus)
    {
        std::string dir = gpu;
        KMStringUtils::replace(dir, "myarch-linux-gnu", archDir);

        std::string link = upperDir + dir;
        std::string real = hostDir + dir;

        std::error_code ec;
        if (!fs::exists(dir, ec))
        {
            continue;
        }

        if (fs::exists(link, ec))
        {
            if (fs::is_symlink(link, ec))
            {
                continue;
            }
            else
            {
                fs::remove(link, ec);
            }
        }

        fs::path parent = fs::path(link).parent_path();
        if (!fs::exists(parent, ec))
        {
            fs::create_directories(parent, ec);
        }

        // 创建软链接
        fs::create_symlink(real, link, ec);
    }

    return true;
}

/**
 * @brief 挂载插件
 * @return true 存在 false 不存在
 */
bool KMRunner::mountPlugins()
{
    KMRunnerPlugins plugins;
    return plugins.mountPlugins(d_ptr->container.get(), d_ptr->appDataPath, d_ptr->annotations, d_ptr->appDir);
}
