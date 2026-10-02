/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMUtil.h"

#include <sys/personality.h>
#include <sys/utsname.h>
#include <fstream>
#include <random>
#include <array>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

#include "KMLogger.h"

static map<string, string> globalAppRefTypeMap = {
    { "org.kde.ark", "app" },
    { "org.kde.elisa", "app" },
    { "org.kde.filelight", "app" },
    { "org.kde.haruna", "app" },
    { "org.kde.isoimagewriter", "app" },
    { "org.kde.itinerary", "app" },
    { "org.kde.kamoso", "app" },
    { "org.kde.kapman", "app" },
    { "org.kde.kcalc", "app" },
    { "org.kde.kclock", "app" },
    { "org.kde.kigo", "app" },
    { "org.kde.kolourpaint", "app" },
    { "org.kde.konsole", "app" },
    { "org.kde.krdc", "app" },
    { "org.kde.kronometer", "app" },
    { "org.kde.kweather", "app" },
    { "org.kde.okular", "app" },
    { "org.kde.palapeli", "app" },
    { "top.openkylin.Clock", "app" },
    { "top.openkylin.PhotoViewer", "app" },
};

static map<string, string> globalRuntimeRefTypeMap = {
    { "top.openkylin.Platform", "runtime" },
    { "top.openkylin.Sdk", "runtime" },
    { "org.kde.Sdk.Docs", "runtime" },
    { "org.freedesktop.Platform.Compat.i386", "runtime" },
    { "org.freedesktop.Platform.VAAPI.Intel", "runtime" },
    { "org.freedesktop.Platform", "runtime" },
    { "org.gtk.Gtk3theme.Yaru", "runtime" },
    { "top.openkylin.Sdk.Doc", "runtime" },
    { "top.wine.Platform", "runtime" },
    { "top.wine.Sdk.Docs", "runtime" },
    { "top.wine.Sdk.gecko", "runtime" },
    { "top.wine.Sdk.mono", "runtime" },
    { "top.wine.Sdk", "runtime" },
};

struct JsonValidator : nlohmann::json::json_sax_t 
{
    // 处理null值
    bool null() override { return true; }
    
    // 处理布尔值
    bool boolean(bool) override { return true; }
    
    // 处理整数
    bool number_integer(number_integer_t) override { return true; }
    
    // 处理无符号整数
    bool number_unsigned(number_unsigned_t) override { return true; }
    
    // 处理浮点数（第二个参数是字符串表示）
    bool number_float(number_float_t, const string_t&) override { return true; }
    
    // 处理字符串
    bool string(string_t&) override { return true; }
    
    // 开始处理对象（参数是元素数量）
    bool start_object(std::size_t) override { return true; }
    
    // 处理对象的键
    bool key(string_t&) override { return true; }
    
    // 结束处理对象
    bool end_object() override { return true; }
    
    // 开始处理数组（参数是元素数量）
    bool start_array(std::size_t) override { return true; }
    
    // 结束处理数组
    bool end_array() override { return true; }

    // 处理二进制数据
#if NLOHMANN_JSON_VERSION_MAJOR > 3 || \
    (NLOHMANN_JSON_VERSION_MAJOR == 3 && NLOHMANN_JSON_VERSION_MINOR >= 8)
    bool binary(binary_t&) override { return true; }
#endif

    bool parse_error(std::size_t, const std::string&, const nlohmann::detail::exception& e) override 
    {
        kmlogger.error("JSON parse error: %s", e.what());
        return false; // 出错时停止解析
    }
};

KMUtil::KMUtil()
{
}

/**
 * @brief : 查找可执行文件路径
 * @param : [in] exe，可执行文件名
 * @return: 可执行文件绝对路径
 * @note  : 只探索下面目录，如果需要请自行添加
 *          /usr/local/sbin
 *          /usr/local/bin
 *          /usr/bin
 *          /usr/sbin
 *          /bin
 *          /sbin
 */
static std::string searchExecutable(const std::string &exe)
{
    char *path = std::getenv("PATH");
    std::vector<std::string> binDirs;
    if (path && path[0] != '\0')
    {
        binDirs = KMStringUtils::splitString(path, ":");
    }
    else
    {
        std::vector<std::string> binDirsDefault = {
            "/usr/local/sbin",
            "/usr/local/bin",
            "/usr/sbin",
            "/usr/bin",
            "/sbin",
            "/bin"
        };

        binDirs = binDirsDefault;
    }

    std::string result;

    for (const std::string &binDir : binDirs)
    {
        std::string execPath = KMStringUtils::buildFilename(binDir, exe);
        fs::path p(execPath);
        if (!fs::exists(p))
        {
            continue;
        }

        if (!fs::is_regular_file(p))
        {
            continue;
        }

        return execPath;
    }

    return result;
}

static std::string packageArch(const std::string &platformArch)
{
    std::string arch = platformArch;

    if (platformArch == "loongarch64")
    {
        std::string exe = searchExecutable("dpkg-architecture");
        if (!exe.empty())
        {
            std::string result;
            std::vector<std::string> args;
            args.push_back("-q");
            args.push_back("DEB_HOST_ARCH");
            if (KMProcessUtils::spawn(exe, args, result))
            {
                KMStringUtils::replace(result, "\n", "");
                if (!result.empty())
                {
                    arch = result;
                }
            }
        }
    }
    else if (platformArch == "sw_64")
    {
        arch = "sw64";
    }

    return arch;
}

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__aarch64__) && !defined(__arm__)
static string get_kernel_arch()
{
    static struct utsname buf;
    static char *arch = nullptr;

    if (arch != nullptr)
        return arch;

    if (uname(&buf))
    {
        arch = "unknown";
        return arch;
    }

    arch = buf.machine;

    /* Override for some arches */
    string mstr = buf.machine;
    if (mstr.length() == 4 && mstr.at(0) == 'i' && mstr.at(2) == '8' && mstr.at(3) == '6')
    {
        arch = "i386";
    }
    else if (mstr == "mips")
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mipsel";
#endif
    }
    else if (mstr == "mips64")
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mips64el";
#endif
    }

    return arch;
}
#endif

/**
 * @brief : 获取当前主机对应的开明包arch信息
 * @note  : 注意此方法仅用于开明包arch使用，不是真正的主机架构值；如主机架构aarch64会转换为arm64
 */
const string KMUtil::kmGetArch()
{
#if defined(__i386__)
    return "i386";
#elif defined(__x86_64__)
    return "x86_64";
#elif defined(__aarch64__)
    return "arm64";
#elif defined(__arm__)
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    return "arm";
#else
    return "armeb";
#endif
#else
    return packageArch(get_kernel_arch());
#endif
}

const string KMUtil::kmGetDefaultModule()
{
    return string("binary");
}

bool KMUtil::isJsonFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) 
    {
        kmlogger.error("Failed to open file: %s", filePath.c_str());
        return false;
    }

    JsonValidator validator;
    bool ok = json::sax_parse(file, &validator, nlohmann::json::input_format_t::json, true /* strict 模式 */);
    return ok;
}

bool KMUtil::isZero(double value, double tolerance)
{
    return std::abs(value) < tolerance;
}

std::string KMUtil::formatInstalledSize(double size)
{
    vector<string> units = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
    int index = 0;

    while (size >= 1024.0 && index < units.size())
    {
        size /= 1024.0;
        index++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[index];
    return oss.str();
}

std::string KMUtil::getCurrentLocale()
{
    // 获取全局的语言环境
    std::locale currentLocale = std::locale();

    // 获取当前语言环境的名称
    std::string localeName = currentLocale.name();

    // 查找'.'的位置
    std::size_t pos = localeName.find('.');

    // 如果找到了'.'，则返回第一个部分，否则返回整个字符串
    if (pos != std::string::npos)
    {
        return localeName.substr(0, pos);
    }
    else
    {
        return localeName;
    }
}

void KMUtil::removeDuplicateValues(vector<string> &refVector)
{
    // set 是一个自动排序容器,只存储唯一值,可以达到去重效果

    set<string> refSet(refVector.begin(), refVector.end());

    refVector.assign(refSet.begin(), refSet.end());
}

bool KMUtil::isOfflinePackageSourceType(const std::string &ref)
{
    if (KMStringUtils::endsWithForRef(ref, ".layer") || KMStringUtils::endsWithForRef(ref, ".ok"))
    {
        return true;
    }
    return false;
}

KMPackageSourceType KMUtil::getPackageSourceType(const std::string &ref)
{
    if (KMStringUtils::endsWith(ref, ".layer"))
    {
        return KMPACKAGE_SOURCE_TYPE_LAYER;
    }

    if (KMStringUtils::endsWithForRef(ref, ".ok") || KMStringUtils::endsWithForRef(ref, ".okx") || KMStringUtils::endsWithForRef(ref, ".okinstaller"))
    {
        return KMPACKAGE_SOURCE_TYPE_OK;
    }

    return KMPACKAGE_SOURCE_TYPE_ONLINE;
}

std::string KMUtil::packageSourceTypeToString(KMPackageSourceType type)
{
    if (packageSourceTypeWithString.find(type) != packageSourceTypeWithString.end())
    {
        return packageSourceTypeWithString[type];
    }
    else
    {
        return "Unknown Type";
    }
}

const string KMUtil::kmGetDefaultChannel()
{
    // 先返回这个
    return "stable";
}

bool KMUtil::kmStrIsInteger(const char *str)
{
    if (str == NULL || *str == '\0')
    {
        return false;
    }

    for (; *str != '\0'; str++)
    {
        if (!g_ascii_isdigit(*str))
        {
            return false;
        }
    }

    return true;
}

bool KMUtil::isCharSafe(gunichar c)
{
    return g_unichar_isgraph(c) || c == ' ';
}

bool KMUtil::kmValidatePathCharacters(const char *path)
{
    while (*path)
    {
        gunichar c = g_utf8_get_char_validated(path, -1);
        if (c == (gunichar)-1 || c == (gunichar)-2)
        {
            return false;
        }
        else if (!isCharSafe(c))
        {
            return false;
        }

        path = g_utf8_find_next_char(path, NULL);
    }

    return true;
}

KMRef KMUtil::kmString2Ref(string &str)
{
    KMRef ref;
    int index1 = str.find('/', 0);
    int index2 = str.find('/', index1 + 1);

    if (index1 <= 0 || index2 <= 0)
    {
        return KMRef();
    }

    ref.id = str.substr(0, index1);
    ref.arch = str.substr(index1 + 1, index2 - index1 - 1);
    ref.branch = str.substr(index2 + 1, str.length() - index2 - 1);

    return ref;
}

std::map<std::string, std::string> KMUtil::kmGetMinimalEnv(bool devel, bool useLdSoCache)
{
    std::map<std::string, std::string> env;
    for (auto &pair : globalEnvDefault)
    {
        if (!pair.second.empty())
        {
            env.insert(pair);
        }
    }

    if (!useLdSoCache)
    {
        env["LD_LIBRARY_PATH"] = "/app/lib";
    }

    if (devel)
    {
        for (auto &pair : globalEnvDevel)
        {
            if (!pair.second.empty())
            {
                env.insert(pair);
            }
        }
    }

    for (auto &envKey : globalEnvCopy)
    {
        const char *envVal = getenv(envKey.c_str());
        if (envVal)
        {
            env.insert(pair<string, string>(envKey, string(envVal)));
        }
    }

    if (!devel)
    {
        for (auto &envKey : globalEnvCopyNoDevel)
        {
            const char *envVal = getenv(envKey.c_str());
            if (envVal)
            {
                env.insert(pair<string, string>(envKey, string(envVal)));
            }
        }
    }
    return env;
}

int KMUtil::kmRandom(int rangeStart, int rangeEnd)
{
    random_device r;
    default_random_engine e(r());
    uniform_int_distribution<int> uniformDist(rangeStart, rangeEnd);
    return uniformDist(e);
}

list<string> KMUtil::split(string str, char c)
{
    list<string> retList;

    int indexStart = 0;
    int indexEnd = -1;
    do
    {
        indexEnd = str.find(c, indexStart);

        if (indexEnd < 0)
        {
            retList.push_back(str.substr(indexStart, str.length() - indexStart));
            break;
        }

        // 针对  "20.03;askkd;qwecc;"
        if (indexEnd == str.length() - 1)
        {
            retList.push_back(str.substr(indexStart, str.length() - indexStart - 1));
            break;
        }

        retList.push_back(str.substr(indexStart, indexEnd - indexStart));
        indexStart = indexEnd + 1;
    } while (true);

    return retList;
}

const string KMUtil::kmGetDBusProxy()
{
    const char *e = g_getenv("KAIMING_DBUSPROXY");

    if (e != NULL)
    {
        return e;
    }
    return DBUSPROXY;
}

const string KMUtil::kmGetTimeZone()
{
    std::string localtime("/etc/localtime");
    /* Resolve relative path */
    std::string canonical = KMUtil::realpath(localtime);
    if (!canonical.empty())
    {
        /* Strip the prefix and slashes if possible. */
        const char *tzdir = getenv("TZDIR");
        if (tzdir != nullptr && KMStringUtils::startsWith(canonical, tzdir))
        {
            std::string_view tzdirView(tzdir);
            std::string canonicalSuffix = canonical.substr(tzdirView.size());
            while (canonicalSuffix.at(0) == '/')
            {
                canonicalSuffix = canonicalSuffix.substr(1);
            }

            return canonicalSuffix;
        }

        std::string defaultTzpath("/usr/share/zoneinfo");
        if (KMStringUtils::startsWith(canonical, defaultTzpath))
        {
            std::string canonicalSuffix = canonical.substr(defaultTzpath.length());
            while (canonicalSuffix.at(0) == '/')
            {
                canonicalSuffix = canonicalSuffix.substr(1);
            }

            return canonicalSuffix;
        }
    }

    if (fs::exists("/etc/timezone"))
    {
        std::fstream fileStream("/etc/timezone", std::ios::in);
        if (fileStream.is_open())
        {
            std::string ret;
            std::getline(fileStream, ret);
            return ret;
        }
    }

    return std::string("UTC");
}

KMPMRef KMUtil::parseKMPMRef(const std::string &ref)
{
    KMPMRef kmRef;
    std::vector<std::string> tokens;
    std::stringstream ss(ref);
    std::string token;

    KMPackageSourceType packageSourceType = getPackageSourceType(ref);

    kmRef.refSourceType = packageSourceType;
    if (kmRef.refSourceType == KMPACKAGE_SOURCE_TYPE_OK)
    {
        kmRef.ref.id = ref;
        return kmRef;
    }

    // 按照斜杠分割字符串
    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }

    // 根据tokens的数量判断引用字符串格式
    if (tokens.size() >= 1)
    {
        kmRef.ref.id = tokens[0];
        // kmRef.ref.kind = KMStringUtils::parseType(kmRef.ref.id); // 此处不能根据 id 后缀判断类型
    }
    else
    {
        //throw std::invalid_argument("Invalid reference format: id is required.");
        kmlogger.error("Invalid reference format: id is required.");
    }

    if (tokens.size() == 2)
    {
        // ${id}/${version} 格式
        kmRef.ref.version = tokens[1];
    }
    else if (tokens.size() == 3)
    {
        // ${id}/${version}/${module} 格式
        kmRef.ref.version = tokens[1];
        kmRef.ref.module = tokens[2];
    }
    else if (tokens.size() == 4)
    {
        // ${id}/${version}/${module}/${channel}/ 格式
        kmRef.ref.version = tokens[1];
        kmRef.ref.module = tokens[2];
        kmRef.ref.channel = tokens[3];
    }
    else if (tokens.size() == 5)
    {
        // ${id}/${version}/${module}/${channel}/${arch} 格式
        kmRef.ref.version = tokens[1];
        kmRef.ref.module = tokens[2];
        kmRef.ref.channel = tokens[3];
        kmRef.ref.arch = tokens[4];
    }

    if (kmRef.ref.arch.empty())
    {
        kmRef.ref.arch = kmGetArch();
    }

    // // 如果需要对version进行Semver规范校验，可以用正则表达式
    // if (!kmRef.ref.version.empty() && !KMStringUtils::isValidSemver(kmRef.ref.version))
    // {
    //     throw std::invalid_argument("Invalid version format: version does not follow Semver specification.");
    // }

    return kmRef;
}

KMPMRef KMUtil::parseSummaryKMPMRef(const std::string &ref)
{
    KMPMRef kmRef;
    std::vector<std::string> tokens;
    std::stringstream ss(ref); // "stable:top.openkylin.base/2.0.0/x86_64",
    std::string token;

    KMPackageSourceType packageSourceType = getPackageSourceType(ref);

    kmRef.refSourceType = packageSourceType;

    // 按照 '/' 分割字符串
    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }

    // 根据tokens的数量判断引用字符串格式
    if (tokens.size() >= 1)
    {
        // 按照  ':' 分割字符串
        std::stringstream ssChannel(tokens[0]);
        std::string tokenChannel;
        std::vector<std::string> tokensChannel;
        while (std::getline(ssChannel, tokenChannel, ':'))
        {
            tokensChannel.push_back(tokenChannel);
        }

        if (tokensChannel.size() == 2)
        {
            kmRef.ref.channel = tokensChannel[0];
            kmRef.ref.id = tokensChannel[1];
        }
        else
        {
            throw std::invalid_argument("Invalid reference format: channel:id is required.");
        }
    }
    else
    {
        throw std::invalid_argument("Invalid reference format: id is required.");
    }

    if (tokens.size() == 2)
    {
        // stable:top.openkylin.base/2.0.0
        kmRef.ref.version = tokens[1];
    }
    else if (tokens.size() == 3)
    {
        // stable:top.openkylin.base/2.0.0/x86_64
        kmRef.ref.version = tokens[1];
        kmRef.ref.arch = tokens[2];
    }

    return kmRef;
}

KMPMRef KMUtil::parseSummaryFuzzyKMPMRef(const std::string &ref)
{
    KMPMRef kmRef;
    std::vector<std::string> tokens;
    std::stringstream ss(ref); // "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0", "stable/x86_64//top.openkylin.base//2.0.0"
    std::string token;

    KMPackageSourceType packageSourceType = getPackageSourceType(ref);

    kmRef.refSourceType = packageSourceType;

    // 按照斜杠分割字符串
    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }

    // 根据tokens的数量判断引用字符串格式
    if (tokens.size() >= 6)
    {
        kmRef.ref.channel = tokens[0];
        kmRef.ref.arch = tokens[1];
        kmRef.ref.kind = tokens[2];
        kmRef.ref.id = tokens[3];
        kmRef.ref.module = tokens[4];
        kmRef.ref.version = tokens[5];
    }
    else
    {
        throw std::invalid_argument("Invalid reference format: id is required.");
    }

    return kmRef;
}

std::string KMUtil::dupKMPMCompletedRef(const KMPMRef &pmRef)
{
    string completedRef;

    completedRef.append(pmRef.ref.channel)
        .append("/")
        .append(pmRef.ref.arch)
        .append("/")
        .append(pmRef.ref.kind)
        .append("/")
        .append(pmRef.ref.id)
        .append("/")
        .append(pmRef.ref.module)
        .append("/")
        .append(pmRef.ref.version);

    return completedRef;
}

void KMUtil::printKMRef(const KMRef &kmRef)
{
    kmlogger.info("<KMRefInfo>: Id:%s channel:%s version:%s arch:%s module:%s", kmRef.id.c_str(), kmRef.channel.c_str(), kmRef.version.c_str(), kmRef.arch.c_str(), kmRef.module.c_str());
}

void KMUtil::printKMPMRef(const string &ref, const KMPMRef &kmRef)
{
    kmlogger.info("<KMPMRefInfo>: Id:%s channel:%s version:%s arch:%s module:%s sourceType:%s", kmRef.ref.id.c_str(), kmRef.ref.channel.c_str(), kmRef.ref.version.c_str(), kmRef.ref.arch.c_str(),
                  kmRef.ref.module.c_str(), packageSourceTypeToString(kmRef.refSourceType).c_str());
}

KMRef KMUtil::constructRef(const std::string &name, const std::string &channel, const std::string &version, const std::string &arch, const std::string &module)
{
    KMRef ref;

    ref.id = name;

    if (channel.empty())
    {
        ref.channel = KMUtil::kmGetDefaultChannel();
        ref.branch = ref.channel;
    }
    else
    {
        ref.branch = channel;
    }

    ref.version = version;

    if (arch.empty())
    {
        ref.arch = KMUtil::kmGetArch();
    }
    else
    {
        ref.arch = arch;
    }

    if (module.empty())
    {
        ref.module = KMUtil::kmGetDefaultModule();
    }
    else
    {
        ref.module = module;
    }

    return ref;
}

KMRef KMUtil::constructRef(KaimingKinds kind, const std::string &name, const std::string &arch, const std::string &branch)
{
    KMRef ref;

    if (KaimingKinds::KAIMING_KINDS_APP == kind)
    {
        ref.kind = BASE_TYPE_APP;
    }
    else
    {
        ref.kind = BASE_TYPE_RUNTIME;
    }

    if (arch.empty())
    {
        ref.arch = KMUtil::kmGetArch();
    }
    else
    {
        ref.arch = arch;
    }

    if (branch.empty())
    {
        ref.branch = KMUtil::kmGetDefaultChannel();
    }
    else
    {
        ref.branch = branch;
    }

    ref.id = name;

    return ref;
}

std::string KMUtil::dupPref(const KMRef &ref)
{
    if (ref.id.empty())
    {
        return "";
    }

    return ref.id + "/" + ref.arch + "/" + ref.branch;
}

std::string KMUtil::dupRef(const KMRef &ref)
{
    if (ref.id.empty())
    {
        return "";
    }

    return ref.kind + "/" + ref.id + "/" + ref.arch + "/" + ref.branch;
}

/**
 * @brief 通过 appid 获取默认的 ref 类型
 *
 * @param pref appid，类型如 org.kde.kclock
 * @return std::string 返回获取的 ref 类型: "app", "runtime"
 */
std::string KMUtil::getDefaultTypeForType(string pref)
{
    kmlogger.info("pref:  %s", pref.c_str());
    for (const auto &ref : globalAppRefTypeMap)
    {
        if (ref.first == pref)
        {
            kmlogger.debug("app ref => first:%s second:%s", ref.first.c_str(), ref.second.c_str());
            return ref.second;
        }
    }

    for (const auto &ref : globalRuntimeRefTypeMap)
    {
        if (ref.first == pref || pref.find(ref.first) != std::string::npos)
        {
            kmlogger.debug("runtime ref => first:%s second:%s", ref.first.c_str(), ref.second.c_str());
            return ref.second;
        }
    }

    if (KMStringUtils::endsWith(pref, ".Debug") || KMStringUtils::endsWith(pref, ".Locale") || KMStringUtils::endsWith(pref, ".Sdk") || KMStringUtils::endsWith(pref, ".Platform") || KMStringUtils::endsWith(pref, ".default") || KMStringUtils::endsWith(pref, ".Sources"))
    {
        kmlogger.debug("runtime ref endsWith .Debug, .Locale, .Sdk, .Platform, .Sources");
        return "runtime";
    }

    kmlogger.info("Use default type: app");
    return "app";
}

/**
 * @brief : std::vector<std::string>转换成std::list<std::string>
 */
std::list<std::string> KMUtil::vectorToList(const std::vector<std::string> &vector)
{
    return std::list<std::string>(vector.cbegin(), vector.cend());
}

std::string KMUtil::getKernelArch()
{
    static struct utsname buf;
    static const char *arch = NULL;
    char *m;

    if (arch != NULL)
    {
        return arch;
    }

    if (::uname(&buf))
    {
        arch = "unknown";
        return arch;
    }

    /* By default, just pass on machine, good enough for most arches */
    arch = buf.machine;

    /* Override for some arches */

    m = buf.machine;
    /* i?86 */
    std::string_view mView(m);
    if (mView.size() == 4 && m[0] == 'i' && m[2] == '8' && m[3] == '6')
    {
        arch = "i386";
    }
    else if (g_str_has_prefix(m, "arm"))
    {
        if (g_str_has_suffix(m, "b"))
        {
            arch = "armeb";
        }
        else
        {
            arch = "arm";
        }
    }
    else if (strcmp(m, "mips") == 0)
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mipsel";
#endif
    }
    else if (strcmp(m, "mips64") == 0)
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        arch = "mips64el";
#endif
    }

    return arch;
}

// 返回对应架构的库目录
std::string KMUtil::getLibraryDir()
{
    // std::string exe = searchExecutable("dpkg-architecture");
    // if (!exe.empty())
    // {
    //     std::string result;
    //     std::vector<std::string> args;
    //     args.push_back("-q");
    //     args.push_back("DEB_HOST_MULTIARCH");
    //     if (KMProcessUtils::spawn(exe, args, result))
    //     {
    //         KMStringUtils::replace(result, "\n", "");
    //         if (!result.empty())
    //         {
    //             return result;
    //         }
    //     }
    // }

#if defined(__x86_64__)
    return "x86_64-linux-gnu";
#elif defined(__i386__)
    return "i386-linux-gnu";
#elif defined(__arm__) && defined(__ARMEL__)
    return "arm-linux-gnueabi";
#elif defined(__arm__) && defined(__ARMHF__)
    return "arm-linux-gnueabihf";
#elif defined(__aarch64__)
    return "aarch64-linux-gnu";
#elif defined(__powerpc64__) && defined(__LITTLE_ENDIAN__)
    return "powerpc64le-linux-gnu";
#elif defined(__powerpc64__) && !defined(__LITTLE_ENDIAN__)
    return "powerpc64-linux-gnu";
#else
    return getKernelArch() + "-linux-gnu";
#endif
}

/**
 * @brief : 获取路径的真实路径
 * @param : [in] inpath，路径
 * @return: 真实路径,软链接会替换为其连接路径
 */
std::string KMUtil::realpath(const std::string &inpath)
{
    if (inpath.empty())
    {
        return "";
    }

    std::string path = KMUtil::canonicalizeFilename(inpath);

    fs::path filepath(path);
    if (!fs::exists(filepath))
    {
        return "";
    }

    if (fs::is_symlink(filepath))
    {
        fs::path link = fs::read_symlink(filepath);
        fs::path targetPath = link;
        if (!link.is_absolute())
        {
            std::string target = link.string();
            targetPath = filepath.parent_path() / target;
        }
        if (!fs::exists(targetPath))
        {
            return "";
        }

        return targetPath.string();
    }

    return filepath.string();
}

/**
 * @brief : 将后面的std::vector中的元素追加到前面std::vector中
 * @param : [out] src，追加元素后的std::vector
 * @param : [in] dest，待追加的std::vector
 */
void KMUtil::append(std::vector<KMRef> &src, std::vector<KMRef> &&dest)
{
    for (auto &item : dest)
    {
        src.push_back(item);
    }
}

/**
 * @brief : 规范化文件路径
 * @param : [in] path，原始文件路径
 * @return: 规范化后的文件路径
 */
std::string KMUtil::canonicalizeFilename(const std::string &path)
{
    // g_autoptr(GFile) file = g_file_new_for_path(path.c_str());
    // return g_file_get_path(file);
    char *canonicalize = canonicalize_file_name(path.c_str());
    std::string ret;
    if (canonicalize != nullptr)
    {
        ret = canonicalize;
        ::free(canonicalize);
    }

    return ret;
}

GBytes *KMUtil::readAllBytesByFd(int fd, std::string &errMsg)
{
    g_autoptr(GMappedFile) mfile = NULL;
    g_autoptr(GError) error = NULL;
    mfile = g_mapped_file_new_from_fd(fd, FALSE, &error);
    if (!mfile)
    {
        errMsg = error->message;
        return nullptr;
    }

    return g_mapped_file_get_bytes(mfile);
}

#define DECIMAL_STR_MAX(type) (2 + (sizeof(type) <= 1 ? 3 : sizeof(type) <= 2 ? 5  \
                                                        : sizeof(type) <= 4   ? 10 \
                                                        : sizeof(type) <= 8   ? 20 \
                                                                              : sizeof(int[-2 * (sizeof(type) > 8)])))

/* 有时这是/var/run-->/run，它是一个符号链接，当我们将其作为路径传递到沙箱时会引起奇怪的问题 */
std::string KMUtil::getRealXdgRuntimeDir()
{
    const char *userRuntimeDir = g_get_user_runtime_dir();
    if (userRuntimeDir != nullptr)
    {
        return KMUtil::realpath(userRuntimeDir);
    }
    else
    {
        return std::string("/run/user/") + std::to_string(::getuid());
    }
}

/* 获取PulseAudio用于其套接字的运行时目录。*/
std::string KMUtil::getPulseRuntimeDir()
{
    const char *val = ::getenv("PULSE_RUNTIME_PATH");
    if (val != nullptr)
    {
        return KMUtil::realpath(val);
    }

    {
        const char *userRuntimeDir = g_get_user_runtime_dir();
        if (userRuntimeDir != nullptr)
        {
            std::string dir = KMStringUtils::buildFilename(userRuntimeDir, "pulse");
            if (fs::is_directory(dir))
            {
                return KMUtil::realpath(dir);
            }
        }
    }

    {
        std::string pulseHome = KMUtil::getPulseHome();
        std::string machineId = KMUtil::getPulseMachineId();
        if (!pulseHome.empty() && !machineId.empty())
        {
            /* This is usually a symlink, but we take its realpath() anyway */
            std::string dir = pulseHome + "/" + machineId + "-runtime";
            if (fs::is_directory(dir))
            {
                return KMUtil::realpath(dir);
            }
        }
    }

    return "";
}

/*
 * 获取PulseAudio用于其配置的目录。
 */
std::string KMUtil::getPulseHome()
{
    /* Legacy path ~/.pulse is tried first, for compatibility */
    {
        std::string ret = KMStringUtils::buildFilename(g_get_home_dir(), ".pulse");
        if (fs::is_directory(ret))
        {
            return ret;
        }
    }

    /* The more modern path, usually ~/.config/pulse */
    {
        std::string ret = KMStringUtils::buildFilename(g_get_user_config_dir(), "pulse");
        if (fs::is_directory(ret))
        {
            return ret;
        }
    }

    return "";
}

/*
 * 获取PulseAudio使用的机器ID。这是systemd/D-Bus机器ID，否则是主机名。
 */
std::string KMUtil::getPulseMachineId()
{
    std::vector<std::string> machineIds = { "/etc/machine-id", "/var/lib/dbus/machine-id" };

    for (auto &machineId : machineIds)
    {
        std::ifstream file(machineId);
        if (file.is_open())
        {
            std::string line;
            std::getline(file, line);
            line = KMStringUtils::strstrip(line);
            size_t i = 0;
            for (; i < line.length(); i++)
            {
                if (!isxdigit(line.at(i)))
                {
                    break;
                }
            }

            if (!line.empty() && i == line.length())
            {
                return line;
            }
        }
    }

    return g_get_host_name();
}

bool KMUtil::argumentNeedsQuoting(const char *arg)
{
    if (*arg == '\0')
    {
        return true;
    }

    while (*arg != 0)
    {
        char c = *arg;
        if (!std::isalnum(c) && !(c == '-' || c == '/' || c == '~' || c == ':' || c == '.' || c == '_' || c == '=' || c == '@'))
        {
            return true;
        }
        arg++;
    }
    return false;
}

/**
 * @brief : 将字节大小转换为GB等表示的字符串
 * @param : [in] size, 字节大小
 * @return: GB/MB/KB等表示的字符串型大小
 */
std::string KMUtil::formatSize(std::uint64_t size)
{
#define GB (1024 * 1024 * 1024)
#define MB (1024 * 1024)
#define KB (1024)

    char str[16] = { 0 };
    double fcount = size * 1.0;

    if (size > GB)
    {
        snprintf(str, sizeof(str), "%.2f GB", fcount / GB);
    }
    else if (size > MB)
    {
        snprintf(str, sizeof(str), "%.2f MB", fcount / MB);
    }
    else if (size > KB)
    {
        snprintf(str, sizeof(str), "%.2f KB", fcount / KB);
    }
    else
    {
        snprintf(str, sizeof(str), "%lu B", size);
    }

    return str;
}

/**
 * @brief : 判断是否数字字符串
 * @return: true-表示是数字字符串；false-表示不是纯数字字符串
 */
bool KMUtil::isInteger(const char *s)
{
    if (s == nullptr || *s == '\0')
    {
        return false;
    }

    for (; *s != '\0'; s++)
    {
        if (!std::isdigit(*s))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief 判断字符串是否相等， `==` 运算符会存在警告
 *
 * @param str1
 * @param str2
 * @return true
 * @return false
 */
bool KMUtil::isStringEqual(std::string str1, std::string str2)
{
    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end());
}

/**
 * @brief 计算字符串中以空格为分隔符的参数个数
 *
 * @param str 待计算字符串，如 "kigo %u"
 * @return int 返回符串中以空格为分隔符的参数个数，"kigo %u" 参数个数为2
 */
int KMUtil::countArgsNumber(const std::string &str)
{
    int count = 0;
    for (char c : str)
    {
        if (c == ' ')
        {
            count++;
        }
    }

    return count + 1;
}

/**
 * @brief 格式化desktop文件Exec字段
 *
 * @param input 输入待格式化的Exec字段，如 "kigo %u -qwindowicon kigo -qwindowtitle %c", "kamoso -qwindowtitle %c"
 * @param refId 输入的refId, 如 "org.kde.kigo"
 * @return std::string 格式化后的字段，如 "kigo --file-forwarding org.kde.kigo @@u %u @@ -qwindowicon kigo -qwindowtitle %c", "kamoso org.kde.kamoso -qwindowtitle %c"
 */
std::string KMUtil::formatExecString(const std::string &input, const std::string &refId)
{
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::vector<std::string> tokensInput;
    std::string token;
    std::string current;
    bool insideQuotes = false;

    int count = countArgsNumber(input);
    kmlogger.info("input: %s command args count: %d", input.c_str(), count);

    while (iss >> std::noskipws >> token[0])
    {
        char ch = token[0];
        if (ch == '"')
        {
            insideQuotes = !insideQuotes;
            if (!insideQuotes)
            {
                tokensInput.push_back("'\"" + current + "\"'");
                current.clear();
            }
        }
        else if (insideQuotes)
        {
            current += ch;
        }
        else if (std::isspace(ch))
        {
            if (!current.empty())
            {
                tokensInput.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += ch;
        }
    }
    if (!current.empty())
    {
        tokensInput.push_back(current);
    }

    size_t argsLen = tokensInput.size();

    std::string commandArg = string("--command=").append(tokensInput[0]);

    if (argsLen >= 1)
    {
        tokens.push_back(commandArg);

        for (size_t i = 1; i < argsLen; i++)
        {
            token = tokensInput[i];

            if (token == "%f" || token == "%u")
            {
                tokens.push_back("--file-forwarding");
                break;
            }
        }

        tokens.push_back(refId);
        tokens.push_back("--");

        for (size_t i = 1; i < argsLen; i++)
        {
            token = tokensInput[i];

            if (token == "%f")
            {
                tokens.push_back("@@");
                tokens.push_back(token);
                tokens.push_back("@@");
            }
            else if (token == "%u")
            {
                tokens.push_back("@@u");
                tokens.push_back(token);
                tokens.push_back("@@");
            }
            else
            {
                tokens.push_back(token);
            }
        }
    }
    else
    {
        tokens.push_back(" ");
        tokens.push_back(refId);
    }

    std::ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        oss << tokens[i];
        if (i < tokens.size() - 1)
        {
            oss << ' ';
        }
    }

    return oss.str();
}

/**
 * @brief 判断输入是否为 'y' 或 'Y', 或者只输入回车键
 *
 * @param input 输入的字符
 * @return true 当前是y, Y, 空格键，允许执行后续操作
 * @return false 当前非默认或y,Y, 不允许执行后续操作
 */
bool KMUtil::isYes(const char &input)
{
    return input == 'y' || input == 'Y' || input == '\n';
}

/**
 * @brief 判断输入是否为 'n' 或 'N'
 *
 * @param input 输入的字符
 * @return true 当前是n, N
 * @return false 当前不是 n, N
 */
bool KMUtil::isNo(const char &input)
{
    return input == 'n' || input == 'N';
}

/**
 * @brief : 将变量param数值由from更改为to
 * @param : [in | param int64_t 变量
 * @param : [in] from, 需要更改的数值
 * @param : [in] to, 替换后的新子串
 */
void KMUtil::replaceInt(int64_t &param, const int64_t &from, const int64_t &to)
{
    if (param == from)
    {
        param = to;
    }
}

void KMUtil::replaceInt(uint64_t &param, const int64_t &from, const int64_t &to)
{
    std::uint64_t u_from = static_cast<uint64_t>(from);
    std::uint64_t u_to = static_cast<uint64_t>(to);
    if (param == u_from)
    {
        param = u_to;
    }
}

void KMUtil::replaceInt(std::optional<int64_t> &param, const int64_t &from, const int64_t &to)
{
    if (param.has_value())
    {
        int64_t &param_tmp = param.value();
        if (param_tmp == from)
        {
            param_tmp = to;
        }
    }
}

void KMUtil::replaceInt(std::optional<uint64_t> &param, const int64_t &from, const int64_t &to)
{
    std::uint64_t u_from = static_cast<uint64_t>(from);
    std::uint64_t u_to = static_cast<uint64_t>(to);
    if (param.has_value())
    {
        uint64_t &param_tmp = param.value();
        if (param_tmp == u_from)
        {
            param_tmp = u_to;
        }
    }
}

void KMUtil::replaceInt(std::optional<std::vector<int64_t>> &param, const int64_t &from, const int64_t &to)
{
    if (param.has_value())
    {
        std::vector<int64_t> param_tmp = param.value();
        for (int i = 0; i < param_tmp.size(); i++)
        {
            if (param_tmp[i] == from)
            {
                param_tmp[i] = to;
            }
        }
    }
}

/**
 * @brief 获取当前工作目录
 * @param cwd 当前工作目录
 * @return true 成功 false 失败 
*/
bool KMUtil::getCwdDir(std::string &cwd)
{
    try
    {
        cwd = fs::current_path().string();
    }
    catch (const fs::filesystem_error &e)
    {
        return false;
    }

    return true;
}
