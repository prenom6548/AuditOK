/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMContainer.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <unordered_set>
#include <uuid/uuid.h>

#include "KMOverlay.h"
#include "KMConfigData.h"
#include "KMCrun.h"

extern char **environ;

using namespace std;

class KMContainer::Private
{
public:
    KMConfigDbata cfg;
    std::string id;
    std::string baseDir;
};

KMContainer::KMContainer()
    : d_ptr(make_unique<Private>())
{
    // 设置模板参数
    initTemplate();
    // 设置基本环境变量
    initBaseEnv();
    // 设置process默认参数
    initProcessDefault();
}

KMContainer::~KMContainer()
{
}

/**
 * @brief 初始化模板
 */
void KMContainer::initTemplate()
{
    std::string hostname = getHostName();
    std::string domainname = getHostDomainName();
    d_ptr->cfg.ociVersion = "1.0.1";
    d_ptr->cfg.hostname = hostname;
    d_ptr->cfg.domainname = domainname;
    d_ptr->cfg.root = Root{ "rootfs", false };
    d_ptr->cfg.linuxConfig = Linux{};
    IDMap uidMap = IDMap{ 0, 0, 65536 };
    IDMap gidMap = IDMap{ 0, 0, 65536 };
    d_ptr->cfg.linuxConfig->uidMappings = std::vector<IDMap>{ uidMap };
    d_ptr->cfg.linuxConfig->gidMappings = std::vector<IDMap>{ uidMap };
    d_ptr->cfg.linuxConfig->namespaces = { { "mount" }, { "uts" } };
}

/**
 * @brief 设置uid映射
 * @param uidMappings uid映射数组
 */
void KMContainer::setUidMappings(const std::vector<IDMap> &uidMappings)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
        d_ptr->cfg.linuxConfig->namespaces = {
            { "mount" },
            { "uts" },
            { "user" }
        };
    }

    d_ptr->cfg.linuxConfig->uidMappings = uidMappings;
}

/**
 * @brief 设置gid映射
 * @param gidMappings gid映射数组
 */
void KMContainer::setGidMappings(const std::vector<IDMap> &gidMappings)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
        d_ptr->cfg.linuxConfig->namespaces = {
            { "mount" },
            { "uts" },
            { "user" }
        };
    }

    d_ptr->cfg.linuxConfig->gidMappings = gidMappings;
}

/**
 * @brief 获取命名空间
 */
std::vector<NameSpace> KMContainer::getNameSpaces()
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
    }

    return d_ptr->cfg.linuxConfig->namespaces;
}

/**
 * @brief 设置命名空间
 * @param nss 命名空间数组
 */
void KMContainer::setNameSpace(const std::vector<std::string> &nss)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
    }

    for (const auto &ns : nss)
    {
        d_ptr->cfg.linuxConfig->namespaces.emplace_back(NameSpace{ ns });
    }
}

/**
 * @brief 添加命名空间
 * @param ns 命名空间
 */
void KMContainer::addNameSpace(const std::string &ns)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
    }

    d_ptr->cfg.linuxConfig->namespaces.emplace_back(NameSpace{ ns });
}

/**
 * @brief 移除命名空间
 * @param ns 命名空间
 */
void KMContainer::rmNameSpace(const std::string &ns)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
    }

    auto &def = d_ptr->cfg.linuxConfig->namespaces;
    auto rm = std::remove_if(def.begin(), def.end(), [&](const NameSpace &it) {
        return (it.linuxNamespace == ns);
    });
    def.erase(rm, def.end());
}

/**
 * @brief 设置/根挂载传播属性
 * @param 传播类型
 */
void KMContainer::setRootfsPropagation(const std::string &propagation)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
    }

    d_ptr->cfg.linuxConfig->rootfsPropagation = propagation;
}

/**
 * @brief 获取/根挂载传播属性
 * @param 传播类型
 * @return true 成功 false 失败
 */
bool KMContainer::getRootfsPropagation(std::string &propagation)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        return false;
    }
    if (d_ptr->cfg.linuxConfig->rootfsPropagation.empty())
    {
        return false;
    }
    propagation = d_ptr->cfg.linuxConfig->rootfsPropagation;
    return true;
}

/**
 * @brief 初始化环境变量
 */
void KMContainer::initBaseEnv()
{
    d_ptr->cfg.process = Process{};
    d_ptr->cfg.process->env = std::vector<std::string>{};
    const std::unordered_set<std::string> envs = { "DISPLAY",
                                                   "SHELL",
                                                   "TEMP",
                                                   "TEMPDIR",
                                                   "PWD",
                                                   "EMAIL_ADDRESS",
                                                   "HOME",
                                                   "HOSTNAME",
                                                   "LOGNAME",
                                                   "REAL_NAME",
                                                   "TERM",
                                                   "USER",
                                                   "USERNAME",
                                                   "XAUTHORITY",
                                                   "XDG_SESSION_CLASS",
                                                   "XDG_SESSION_ID",
                                                   "XDG_SESSION_DESKTOP",
                                                   "D_DISABLE_RT_SCREEN_SCALE",
                                                   "XMODIFIERS",
                                                   "DESKTOP_SESSION",
                                                   "DEEPIN_WINE_SCALE",
                                                   "XDG_CURRENT_DESKTOP",
                                                   "XDG_DATA_HOME",
                                                   "XIM",
                                                   "XDG_SESSION_TYPE",
                                                   "XDG_RUNTIME_DIR",
                                                   "CLUTTER_IM_MODULE",
                                                   "QT4_IM_MODULE",
                                                   "GTK_IM_MODULE",
                                                   "auto_proxy",
                                                   "http_proxy",
                                                   "https_proxy",
                                                   "ftp_proxy",
                                                   "SOCKS_SERVER",
                                                   "no_proxy",
                                                   "QT_IM_MODULE",
                                                   "WAYLAND_DISPLAY",
                                                   "QT_QPA_PLATFORM",
                                                   "QT_WAYLAND_SHELL_INTEGRATION",
                                                   "GDMSESSION",
                                                   "QT_WAYLAND_FORCE_DPI",
                                                   "QMLSCENE_DEVICE",
                                                   "GNOME_DESKTOP_SESSION_ID",
                                                   "PYTHONPATH",
                                                   "PERLLIB",
                                                   "PERL5LIB",
                                                   "XCURSOR_PATH",
                                                   "GST_PLUGIN_PATH_1_0",
                                                   "GST_REGISTRY",
                                                   "GST_REGISTRY_1_0",
                                                   "GST_PLUGIN_PATH",
                                                   "GST_PLUGIN_SYSTEM_PATH",
                                                   "GST_PLUGIN_SCANNER",
                                                   "GST_PLUGIN_SCANNER_1_0",
                                                   "GST_PLUGIN_SYSTEM_PATH_1_0",
                                                   "GST_PRESET_PATH",
                                                   "GST_PTP_HELPER",
                                                   "GST_PTP_HELPER_1_0",
                                                   "GST_INSTALL_PLUGINS_HELPER",
                                                   "KRB5CCNAME",
                                                   "XKB_CONFIG_ROOT",
                                                   "GIO_EXTRA_MODULES",
                                                   "GDK_BACKEND",
                                                   "UEM_SERVICE_LAUNCH_APP" };

    for (char **env = environ; *env != nullptr; ++env)
    {
        std::string kv(*env);
        auto pos = kv.find('=');
        if (pos == std::string::npos)
        {
            continue;
        }
        std::string key = kv.substr(0, pos);
        if (envs.find(key) != envs.end())
        {
            d_ptr->cfg.process->env.emplace_back(kv);
        }
    }
}

/**
 * @brief 设置appID
 * @param id appID
 */
void KMContainer::setAppID(const std::string &id)
{
    d_ptr->id = id;
}

/**
 * @brief 设置base目录,必须设置用于修订挂载点信息
 * @param baseDir 目录
 */
void KMContainer::setBaseDir(const std::string &baseDir)
{
    d_ptr->baseDir = baseDir;
}

/**
 * @brief 获取appID
 * @return appID
 */
std::string KMContainer::getAppID()
{
    return d_ptr->id;
}

/**
 * @brief 设置oci版本
 * @param version 版本号
 */
void KMContainer::setOciVersion(const std::string &version)
{
    d_ptr->cfg.ociVersion = version;
}

/**
 * @brief 获取版本
 * @return string 版本号
 */
std::string KMContainer::getOciVersion()
{
    if (d_ptr->cfg.ociVersion)
    {
        return d_ptr->cfg.ociVersion.value();
    }

    return {};
}

/**
 * @brief 设置主机名
 * @param hostName 主机名
 */
void KMContainer::setHostName(const std::string &hostName)
{
    d_ptr->cfg.hostname = hostName;
}

/**
 * @brief 获取容器主机名
 * @return string 主机名
 */
std::string KMContainer::setHostName()
{
    if (d_ptr->cfg.hostname)
    {
        return d_ptr->cfg.hostname.value();
    }

    return {};
}

/**
 * @brief 设置域名
 * @param domainName 域名
 */
void KMContainer::setDomainName(const std::string &domainName)
{
    d_ptr->cfg.domainname = domainName;
}

/**
 * @brief 获取域名
 * @return string 域名
 */
std::string KMContainer::getDomainName()
{
    if (d_ptr->cfg.domainname)
    {
        return d_ptr->cfg.domainname.value();
    }

    return {};
}

/**
 * @brief 设置根目录
 * @param root 根目录
 * @param readonly 是否只读系统
 */
void KMContainer::setRoot(const std::string &root, bool readonly)
{
    if (!d_ptr->cfg.root)
    {
        d_ptr->cfg.root = Root{};
    }
    d_ptr->cfg.root->path = root;
    d_ptr->cfg.root->readonly = readonly;
}

/**
 * @brief 获取根结构数据
 * @param Root 数据
 */
Root KMContainer::getRoot()
{
    if (d_ptr->cfg.root)
    {
        return d_ptr->cfg.root.value();
    }

    return Root{};
}

/**
 * @brief 添加注释；包含任意传递给容器的元数据
 * @param key 键
 * @param value 值
 */
void KMContainer::addAnnotation(const std::string &key,
                                const std::string &value)
{
    if (d_ptr->cfg.annotations)
    {
        d_ptr->cfg.annotations.value()[key] = value;
    }
    else
    {
        d_ptr->cfg.annotations = std::map<std::string, std::string>{ { key, value } };
    }
}

/**
 * @brief 获取注释；包含任意传递给容器的元数据
 * @return 注释
 */
std::map<std::string, std::string> KMContainer::getAnnotations()
{
    if (d_ptr->cfg.annotations)
    {
        return d_ptr->cfg.annotations.value();
    }

    return std::map<std::string, std::string>{};
}

/**
 * @brief 删除注释
 * @param key 键
 */
void KMContainer::delAnnotation(const std::string &key)
{
    if (d_ptr->cfg.annotations)
        d_ptr->cfg.annotations->erase(key);
}

/**
 * @brief 设置挂载点
 * @param mounts 挂载点
 */
void KMContainer::setMounts(const std::vector<Mount> &mounts)
{
    d_ptr->cfg.mounts = mounts;
}

/**
 * @brief 添加挂载点
 * @param mount 挂载点
 */
void KMContainer::addMount(const Mount &mount)
{
    if (d_ptr->cfg.mounts)
    {
        d_ptr->cfg.mounts->emplace_back(mount);
    }
    else
    {
        d_ptr->cfg.mounts = std::vector<Mount>{ mount };
    }
}

/**
 * @brief 获取Mounts
 * @return mounts 挂载数据
 */
std::vector<Mount> KMContainer::getMounts()
{
    if (d_ptr->cfg.mounts)
    {
        return d_ptr->cfg.mounts.value();
    }
    else
    {
        return std::vector<Mount>{};
    }
}

/**
 * @brief 设置进程启动参数
 * @param args 启动参数
 */
void KMContainer::setProcessArgs(const std::vector<std::string> &args)
{
    if (!d_ptr->cfg.process)
    {
        d_ptr->cfg.process = Process{};
    }

    d_ptr->cfg.process->args = args;
}

/**
 * @brief 设置进程当前工作目录
 * @param cwd 当前工作目录
 */
void KMContainer::setProcessCwd(const std::string &cwd)
{
    if (!d_ptr->cfg.process)
    {
        d_ptr->cfg.process = Process{};
    }

    d_ptr->cfg.process->cwd = cwd;
}

/**
 * @brief 设置进程默认参数
 */
void KMContainer::initProcessDefault()
{
    if (!d_ptr->cfg.process)
    {
        d_ptr->cfg.process = Process{};
    }

    if (isatty(fileno(stdin)) != 0)
    {
        d_ptr->cfg.process->terminal = true;
    }
    else
    {
        d_ptr->cfg.process->terminal = false;
    }

    d_ptr->cfg.process->user = User{ .gid = getgid(), .uid = getuid() };

    // 工作目录默认为根目录
    d_ptr->cfg.process->cwd = string("/");
}

/**
 * @brief 设置进程默认参数
 */
void KMContainer::setProcessTerminal(const bool &terminal)
{
    if (!d_ptr->cfg.process)
    {
        d_ptr->cfg.process = Process{};
    }
    d_ptr->cfg.process->terminal = terminal;
}

/**
 * @brief 获取进程启动参数
 * @return args 启动参数
 */
std::vector<std::string> KMContainer::getProcessArgs()
{
    if (d_ptr->cfg.process)
    {
        return d_ptr->cfg.process->args;
    }

    return std::vector<std::string>{};
}

/**
 * @brief 设置进程环境变量
 * @param env 环境变量
 */
void KMContainer::setProcessEnv(const std::vector<std::string> &env)
{
    if (!d_ptr->cfg.process)
        d_ptr->cfg.process = Process{};

    d_ptr->cfg.process->env = env;
}

/**
 * @brief 添加进程环境变量
 * @param env 环境变量
 */
void KMContainer::addProcessEnv(const std::vector<std::string> &env)
{
    if (!d_ptr->cfg.process)
        d_ptr->cfg.process = Process{};

    d_ptr->cfg.process->env.insert(d_ptr->cfg.process->env.end(),
                                   std::make_move_iterator(env.begin()),
                                   std::make_move_iterator(env.end()));
}

/**
 * @brief 获取进程启动环境变量
 * @return env 环境变量
 */
std::vector<std::string> KMContainer::getProcessEnv()
{
    if (d_ptr->cfg.process)
    {
        return d_ptr->cfg.process->env;
    }

    return std::vector<std::string>{};
}

/**
 * @brief 设置进程配置
 * @param process 进程配置
 */
void KMContainer::setProcess(const Process &process)
{
    d_ptr->cfg.process = process;
}

/**
 * @brief 获取进程配置
 * @return Process 进程配置
 */
Process KMContainer::getProcess()
{
    if (d_ptr->cfg.process)
    {
        return d_ptr->cfg.process.value();
    }

    return Process{};
}

/**
 * @brief 设置工作目录
 * @param cwd 工作目录
 */
void KMContainer::setCwd(const std::string &cwd)
{
    if (!d_ptr->cfg.process)
    {
        d_ptr->cfg.process = Process{};
    }

    d_ptr->cfg.process->cwd = cwd;
}

/**
 * @brief 设置hooks挂载
 * @param hooks hooks挂载
 */
void KMContainer::setHooks(const Hooks &hooks)
{
    d_ptr->cfg.hooks = hooks;
}

/**
 * @brief 获取Hooks挂载数据
 * @return hooks 挂载数据
 */
Hooks KMContainer::getHooks()
{
    if (d_ptr->cfg.hooks)
    {
        return d_ptr->cfg.hooks.value();
    }

    return Hooks{};
}

/**
 * @brief 设置prestart钩子
 * @param perstart prestart结构
 */
void KMContainer::setPrestart(const std::vector<Hook> &prestart)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }

    d_ptr->cfg.hooks->prestart = prestart;
}

/**
 * @brief 添加createRuntime钩子
 * @param createRuntime createRuntime结构
 */
void KMContainer::setCreateRuntime(const std::vector<Hook> &createRuntime)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }
    d_ptr->cfg.hooks->createRuntime = createRuntime;
}

/**
 * @brief 设置createContainer钩子
 * @param createContainer createContainer结构
 */
void KMContainer::setCreateContainer(const std::vector<Hook> &createContainer)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }
    d_ptr->cfg.hooks->createContainer = createContainer;
}

/**
 * @brief 添加startContainer钩子
 * @param startContainer startContainer结构
 */
void KMContainer::setStartContainer(const std::vector<Hook> &startContainer)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }
    d_ptr->cfg.hooks->startContainer = startContainer;
}

/**
 * @brief 设置poststart钩子
 * @param postStart postStart结构
 */
void KMContainer::setPoststart(const std::vector<Hook> &postStart)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }
    d_ptr->cfg.hooks->postStart = postStart;
}

/**
 * @brief 添加poststop钩子
 * @param postStop postStop结构
 */
void KMContainer::setPostStop(const std::vector<Hook> &postStop)
{
    if (!d_ptr->cfg.hooks)
    {
        d_ptr->cfg.hooks = Hooks{};
    }
    d_ptr->cfg.hooks->postStop = postStop;
}

/**
 * @brief 设置linux平台配置
 * @param linuxConfig linux配置
 */
void KMContainer::setLinux(const Linux &linuxConfig)
{
    d_ptr->cfg.linuxConfig = linuxConfig;
}

/**
 * @brief 获取linux平台配置
 * @return Linux 平台信息
 */
Linux KMContainer::getLinux()
{
    if (d_ptr->cfg.linuxConfig)
    {
        return d_ptr->cfg.linuxConfig.value();
    }
    return Linux{};
}

/**
 * @brief 运行前准备
 * @param isBuild 是否是编译工作
 * @param runDir 工作目录
 * @param containerID 容器ID
 * @return 运行成功返回true，否则返回false
 */
bool KMContainer::preRun(std::string &runDir, std::string &containerid, const bool &isBuild)
{
    if (!fs::exists(KAIMING_BOX))
    {
        KMError("crun do not exist, please install crun first");
        return false;
    }

    if (!genRunDir(runDir, containerid, isBuild))
    {
        KMError("generate run directory failed");
        return false;
    }

    if (!genConfig(runDir))
    {
        KMError("generate config.json failed");
        return false;
    }

    return true;
}

/**
 * @brief 运行容器,同步方式,前后端不分离
 * @param isBuild 是否是编译工作运行, 默认为true是为了兼容旧版本代码
 * @return 成功true，失败 false
 */
bool KMContainer::run(const bool &isBuild)
{
    string runDir, containerID;
    if (!preRun(runDir, containerID, isBuild))
    {
        return false;
    }

    vector<string> args = { "--cgroup-manager=disabled", "run", "-b", runDir };
    args.emplace_back(containerID);
    if (!KMCrun::exec(args))
    {
        clearRunDir(runDir, isBuild);
        return false;
    }

    if (!clearRunDir(runDir, isBuild))
    {
        KMError("clear run dir failed");
    }

    return true;
}

std::string KMContainer::generateUUID()
{
    uuid_t uuidtime;
    // 生成基于时间的UUID
    uuid_generate_time(uuidtime);

    // 将UUID转换为字符串格式
    std::array<char, 64> uuidarray;
    uuid_unparse(uuidtime, uuidarray.data());
    return std::string(uuidarray.data());
}

/**
 * @brief 生成运行参数
 * @return true 成功，false 失败
 */
bool KMContainer::genRunDir(std::string &dir, std::string &uuid, bool isBuild)
{
    // 编包目录和运行目录不同
    if (isBuild)
    {
        dir = string("/run/user/") + to_string(getuid()) + string("/kaiming-builder/") + d_ptr->id;
    }
    else
    {
        dir = string("/run/user/") + to_string(getuid()) + string("/kaiming/") + d_ptr->id;
    }

    if (isBuild || !fs::exists(dir))
    {
        uuid = generateUUID();
        if (uuid.empty())
        {
            KMError("generate uuid failed");
            return false;
        }
    }
    else
    {
        for (const auto &entry : fs::directory_iterator(dir))
        {
            if (fs::is_directory(entry.status()))
            {
                uuid = entry.path().filename().string();
                break;
            }
        }
        if (uuid.empty())
        {
            uuid = generateUUID();
        }
    }

    dir = dir + "/" + uuid;
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec)
    {
        KMError("create /run/user/ directory failed: " + ec.message());
        return false;
    }

    return true;
}

/**
 * @brief 生成配置文件
 * @param dir bundle目录
 * @return true 成功，false 失败
 */
bool KMContainer::genConfig(const std::string &dir)
{
    if (!fs::exists(dir))
    {
        std::error_code ec;
        fs::create_directories(dir, ec);
        if (ec)
        {
            KMError("create bundle directory failed: " + ec.message());
            return false;
        }
    }

    string rootPath = dir + string("/") + d_ptr->cfg.root->path;
    if (!fs::exists(rootPath))
    {
        std::error_code ec;
        fs::create_directories(rootPath, ec);
    }

    std::ofstream ofs(dir + "/config.json");
    if (!ofs.is_open())
    {
        KMError("create config.json in bundle directory failed");
        return false;
    }

    try
    {
        nlohmann::json json = d_ptr->cfg;
        ofs << std::setw(4) << json << std::endl;
        ofs.close();
    }
    catch (...)
    {
        ofs.close();
        KMError("generate config.json failed");
        return false;
    }

    return true;
}

/**
 * @brief 清空容器运行目录
 * @param dir 运行目录
 * @return true 成功 false 失败
 */
bool KMContainer::clearRunDir(const std::string &dir, const bool &isBuild)
{
    if (dir.empty())
    {
        return true;
    }

    if (!fs::exists(dir))
    {
        return true;
    }

    std::string clDir = dir;
    if (!isBuild)
    {
        clDir = fs::path(dir).parent_path().string();
    }

    std::error_code ec;
    if (!fs::remove_all(clDir, ec))
    {
        KMError("clear run directory failed: " + ec.message());
        return false;
    }

    return true;
}

static std::map<int, std::string> allCapabilitiesMap = {
    { 0, "CAP_CHOWN" },
    { 1, "CAP_DAC_OVERRIDE" },
    { 2, "CAP_DAC_READ_SEARCH" },
    { 3, "CAP_FOWNER" },
    { 4, "CAP_FSETID" },
    { 5, "CAP_KILL" },
    { 6, "CAP_SETGID" },
    { 7, "CAP_SETUID" },
    { 8, "CAP_SETPCAP" },
    { 9, "CAP_LINUX_IMMUTABLE" },
    { 10, "CAP_NET_BIND_SERVICE" },
    { 11, "CAP_NET_BROADCAST" },
    { 12, "CAP_NET_ADMIN" },
    { 13, "CAP_NET_RAW" },
    { 14, "CAP_IPC_LOCK " },
    { 15, "CAP_IPC_OWNER" },
    { 16, "CAP_SYS_MODULE" },
    { 17, "CAP_SYS_RAWIO" },
    { 18, "CAP_SYS_CHROOT" },
    { 19, "CAP_SYS_PTRACE" },
    { 20, "CAP_SYS_PACCT" },
    { 21, "CAP_SYS_ADMIN" },
    { 22, "CAP_SYS_BOOT" },
    { 23, "CAP_SYS_NICE" },
    { 24, "CAP_SYS_RESOURCE" },
    { 25, "CAP_SYS_TIME" },
    { 26, "CAP_SYS_TTY_CONFIG" },
    { 27, "CAP_MKNOD" },
    { 28, "CAP_LEASE" },
    { 29, "CAP_AUDIT_WRITE" },
    { 30, "CAP_AUDIT_CONTROL" },
    { 31, "CAP_SETFCAP" },
    { 32, "CAP_MAC_OVERRIDE" },
    { 33, "CAP_MAC_ADMIN" },
    { 34, "CAP_SYSLOG" },
    { 35, "CAP_WAKE_ALARM" },
    { 36, "CAP_BLOCK_SUSPEND" },
    { 37, "CAP_AUDIT_READ" },
    { 38, "CAP_PERFMON" },
    { 39, "CAP_BPF" },
    { 40, "CAP_CHECKPOINT_RESTORE" } //
};

/**
 * @brief 根据给定的权限 ID 列表，获取对应权限能力的字符串表示
 * @param ids 权限ID的列表
 * @return 权限字符串的列表，表示输入ID对应的权限能力
 */
std::vector<std::string>
KMContainer::getCapabilities(const std::vector<int> &ids)
{
    std::vector<std::string> result;
    for (int id : ids)
    {
        auto it = allCapabilitiesMap.find(id);
        if (it != allCapabilitiesMap.end())
        {
            result.push_back(it->second);
        }
    }
    return result;
}

/**
 * @brief 设置Linux的capabilities的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setLinuxCapabilities(const std::vector<int> &ids)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux();
    }
    d_ptr->cfg.linuxConfig->capabilities = getCapabilities(ids);
}

/**
 * @brief 设置process的capabilities的bounding的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setProcessBoundingCapabilities(const std::vector<int> &ids)
{
    if (!d_ptr->cfg.process->capabilities)
    {
        d_ptr->cfg.process->capabilities = Capabilities();
    }
    d_ptr->cfg.process->capabilities->bounding = getCapabilities(ids);
}

/**
 * @brief 设置process的capabilities的permitted的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setProcessPermittedCapabilities(const std::vector<int> &ids)
{
    if (!d_ptr->cfg.process->capabilities)
    {
        d_ptr->cfg.process->capabilities = Capabilities();
    }
    d_ptr->cfg.process->capabilities->permitted = getCapabilities(ids);
}

/**
 * @brief 设置process的capabilities的inheritable的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setProcessInheritableCapabilities(
    const std::vector<int> &ids)
{
    if (!d_ptr->cfg.process->capabilities)
    {
        d_ptr->cfg.process->capabilities = Capabilities();
    }
    d_ptr->cfg.process->capabilities->inheritable = getCapabilities(ids);
}

/**
 * @brief 设置process的capabilities的effective的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setProcessEffectiveCapabilities(const std::vector<int> &ids)
{
    if (!d_ptr->cfg.process->capabilities)
    {
        d_ptr->cfg.process->capabilities = Capabilities();
    }
    d_ptr->cfg.process->capabilities->effective = getCapabilities(ids);
}

/**
 * @brief 设置process的capabilities的ambient的权限能力
 * @param ids 权限 ID 的列表
 */
void KMContainer::setProcessAmbientCapabilities(const std::vector<int> &ids)
{
    if (!d_ptr->cfg.process->capabilities)
    {
        d_ptr->cfg.process->capabilities = Capabilities();
    }
    d_ptr->cfg.process->capabilities->ambient = getCapabilities(ids);
}

/**
 * @brief 生成config.json到指定目录
 * @param dir 指定目录
 */
void KMContainer::genConfigTo(const std::string &dir)
{
    std::ofstream ofs(dir + "/config.json");
    if (!ofs.is_open())
    {
        KMError("create config.json in bundle directory failed");
    }
    try
    {
        nlohmann::json json = d_ptr->cfg;
        ofs << std::setw(4) << json << std::endl;
        ofs.close();
    }
    catch (...)
    {
        ofs.close();
        KMError("generate config.json failed");
    }
}

/**
 * @brief 获取KMConfigDbata对象
 * @return KMConfigDbata
 */
KMConfigDbata KMContainer::getConfig()
{
    return d_ptr->cfg;
}

/**
 * @brief 修改KMConfigDbata对象cfg
 * @param config, KMConfigDbata对象
 */
void KMContainer::setCfg(const KMConfigDbata &config)
{
    d_ptr->cfg = config;
}

/**
 * @brief overlay支持rootless挂载处理
 */
void KMContainer::overlayRootless()
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux{};
        return;
    }

    if (!KMOverlay::isOverlayRootless())
    {
        rmNameSpace("user");
    }
}

/**
 * @brief 设置seccomp配置
 * @param seccomp seccomp配置
 * @return true 成功 false 失败
 */
bool KMContainer::setSeccomp(const Seccomp &seccomp)
{
    if (!d_ptr->cfg.linuxConfig)
    {
        d_ptr->cfg.linuxConfig = Linux();
    }
    if (!d_ptr->cfg.linuxConfig->seccomp)
    {
        d_ptr->cfg.linuxConfig->seccomp = Seccomp();
    }
    d_ptr->cfg.linuxConfig->seccomp = seccomp;
    return true;
}

/**
 * @brief 获取宿主主机名
 * @return string 主机名
 */
std::string KMContainer::getHostName()
{
    std::string name;
    char hostname[65] = { 0 };
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        name = "kaiming";
    }
    else
    {
        name = hostname;
    }

    return name;
}

/**
 * @brief 获取主机域名
 * @return string 主机名
 */
std::string KMContainer::getHostDomainName()
{
    std::string name;
    char domainname[65] = { 0 };
    if (getdomainname(domainname, sizeof(domainname)) != 0)
    {
        name = "kaiming";
    }
    else
    {
        name = domainname;
    }

    return name;
}
