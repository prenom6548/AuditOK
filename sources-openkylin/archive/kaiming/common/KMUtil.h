/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMUTIL_H
#define KMUTIL_H

#include <glib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <filesystem>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <locale>
#include <optional>
#include <nlohmann/json.hpp>

#include "KMStringUtils.h"
#include "KMProcessUtils.h"
#include "KMRemoteSummary.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

// 默认环境变量
static std::map<std::string, std::string> globalEnvDefault = {
    { "PATH", "/app/bin:/usr/bin" },
    { "LD_LIBRARY_PATH", "" },
    { "XDG_CONFIG_DIRS", "/app/etc/xdg:/etc/xdg" },
    // {"XDG_DATA_DIRS", "/app/share:/usr/share"},
    { "XDG_DATA_DIRS", "/app/share:/usr/share:/usr/share/runtime/share:/run/host/user-share:/run/host/share" },
    { "SHELL", "/bin/sh" },
    { "TEMP", "" },
    { "TEMPDIR", "" },
    { "TMP", "" },
    { "TMPDIR", "" },
    { "XDG_RUNTIME_DIR", "" },
    { "PYTHONPATH", "" },
    { "PERLLIB", "" },
    { "PERL5LIB", "" },
    { "XCURSOR_PATH", "" },
    { "GST_PLUGIN_PATH_1_0", "" },
    { "GST_REGISTRY", "" },
    { "GST_REGISTRY_1_0", "" },
    { "GST_PLUGIN_PATH", "" },
    { "GST_PLUGIN_SYSTEM_PATH", "" },
    { "GST_PLUGIN_SCANNER", "" },
    { "GST_PLUGIN_SCANNER_1_0", "" },
    { "GST_PLUGIN_SYSTEM_PATH_1_0", "" },
    { "GST_PRESET_PATH", "" },
    { "GST_PTP_HELPER", "" },
    { "GST_PTP_HELPER_1_0", "" },
    { "GST_INSTALL_PLUGINS_HELPER", "" },
    { "KRB5CCNAME", "" },
    { "XKB_CONFIG_ROOT", "" },
    { "GIO_EXTRA_MODULES", "" },
    { "GDK_BACKEND", "" },
};

static std::map<std::string, std::string> globalEnvDevel = {
    { "ACLOCAL_PATH", "/app/share/aclocal" },
    { "C_INCLUDE_PATH", "/app/include" },
    { "CPLUS_INCLUDE_PATH", "/app/include" },
    { "LDFLAGS", "-L/app/lib " },
    { "PKG_CONFIG_PATH", "/app/lib/pkgconfig:/app/share/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig" },
    { "LC_ALL", "en_US.utf8" }
};

static std::set<std::string> globalEnvCopy = { "PWD", "GDMSESSION", "XDG_CURRENT_DESKTOP", "XDG_SESSION_DESKTOP", "DESKTOP_SESSION", "EMAIL_ADDRESS", "HOME", "HOSTNAME", "LOGNAME", "REAL_NAME", "TERM", "USER", "USERNAME" };

static std::set<std::string> globalEnvCopyNoDevel = { "LANG", "LANGUAGE", "LC_ALL", "LC_ADDRESS", "LC_COLLATE", "LC_CTYPE", "LC_IDENTIFICATION", "LC_MEASUREMENT", "LC_MESSAGES", "LC_MONETARY", "LC_NAME", "LC_NUMERIC", "LC_PAPER", "LC_TELEPHONE", "LC_TIME" };

static std::vector<std::string> exportDirs = { "share/applications", "share/icons", "share/dbus-1/services", "share/gnome-shell/search-providers", "share/mime/packages", "share/metainfo", "bin" };

static std::vector<std::string> globalOfflinePackageSourceType = { ".ok", ".layer" };

// 创建枚举值与字符串的绑定映射
static std::unordered_map<KMPackageSourceType, std::string> packageSourceTypeWithString = { { KMPACKAGE_SOURCE_TYPE_ONLINE, "online" },
                                                                                            { KMPACKAGE_SOURCE_TYPE_OK, ".ok" },
                                                                                            { KMPACKAGE_SOURCE_TYPE_LAYER, ".layer" },
                                                                                            { KMPACKAGE_SOURCE_TYPE_OFFLINE, "offline" } };

class KMUtil
{
public:
    KMUtil();

    /**
     * @brief : 获取当前主机对应的开明包arch信息
     * @note  : 注意此方法仅用于开明包arch使用，不是真正的主机架构值；如主机架构aarch64会转换为arm64
     */
    static const std::string kmGetArch();
    static const std::string kmGetDefaultModule();
    static const std::string kmGetDefaultChannel();
    static const std::string kmGetDBusProxy();
    static const std::string kmGetTimeZone();
    static bool kmStrIsInteger(const char *str);
    static bool kmValidatePathCharacters(const char *path);
    static KMRef kmString2Ref(std::string &str);
    static std::map<std::string, std::string> kmGetMinimalEnv(bool devel, bool useLdSoCache);
    static int kmRandom(int rangeStart, int rangeEnd);
    static std::list<std::string> split(std::string str, char c);
    static std::string getKernelArch();

    /**
     * @brief 判断文件是否是 json 文件
     * 
     * @param filePath 文件路径
     * @return true 是 json 文件
     * @return false 不是 json 文件
     */
    static bool isJsonFile(const std::string &filePath);

    /**
     * @brief 获取当前工作目录
     * 
     * @param cwd 当前工作目录
     * @return true 成功 false 失败 
     */
    static bool getCwdDir(std::string &cwd);

    /**
     * @brief 判断一个 double 类型值是否是0
     * 
     * @param value 判断值
     * @param tolerance double的精度 
     * @return true  double 类型值是0
     * @return false double 类型值不是0
     */
    static bool isZero(double value, double tolerance = 1e-6);

    /**
     * @brief 格式化安装大小
     * 
     * @param size 安装大小
     * @return std::string 格式化安装工大小
     */
    static std::string formatInstalledSize(double size);

    /**
     * @brief 获取当前语言环境，并进行截断符合 desktop 规范
     * 
     * @return std::string 返回当前的语言环境字符串，取值如下：
     * 
     *  "en_US", "zh_CN", "C" 等
     */
    static std::string getCurrentLocale();

    /**
     * @brief 去除 ref 向量中的重复值
     * 
     * @param refVector 去除向量中重复值后的 ref 向量
     */
    static void removeDuplicateValues(std::vector<std::string> &refVector);

    /**
     * @brief 当前 ref 是否是离线包
     * 
     * @param ref 判断的 ref
     * @return true  是离线包
     * @return false 不是离线包
     */
    static bool isOfflinePackageSourceType(const std::string &ref);

    /**
     * @brief 获取包来源类型
     * 
     * @param ref 判断的 ref
     * @return KMPackageSourceType 返回来源类型为：在线包、离线包（.layer, .ok）
     */
    static KMPackageSourceType getPackageSourceType(const std::string &ref);

    /**
     * @brief 通过枚举 KMPackageSourceType 获取对应的字符串
     * 
     * @param type KMPackageSourceType 变量
     * @return std::string 获取对应的变量
     */
    static std::string packageSourceTypeToString(KMPackageSourceType type);

    /**
     * @brief 分割字符串函数，通过/分割输入字符串，将分割后的结果保存在KMRef中
     * 
     * 不同格式处理：根据分割后的向量的大小来判断字符串的格式并进行相应的解析。
     * 
     * 以下是包含的格式：
     * 
     *  - 格式${id}：仅有id部分，其他字段使用默认值。
     * 
     *  - 格式${id}/${version}：channel、module 使用默认值。
     * 
     *  - 格式${id}/${version}/${module}：channel 使用默认值。
     * 
     *  - 格式${id}/${version}/${module}/${channel}/ ：完整格式。
     * 
     *  - 格式${id}/${version}/${module}/${channel}/${arch} ：完整格式。
     * 
     * 版本验证：对version字段的格式进行验证，确保符合Semver规范。
     * 
     * @param ref 用户命令行输入的ref
     * @return KMPMRef 返回组装的 KMPMRef 结构体
     */
    static KMPMRef parseKMPMRef(const std::string &ref);

    /**
     * @brief 解析 summary.json module 模块中获取的字符串，将结果保存在 KMPMRef中
     * 
     * @param ref summary.json中获取的字符串, 如 base, runtime 中的  "stable:top.openkylin.base/2.0.0/x86_64",其中版本号大部分是缺的
     * @return KMPMRef 返回组装的 KMPMRef 结构体
     */
    static KMPMRef parseSummaryKMPMRef(const std::string &ref);

    /**
     * @brief 解析 summary.json 中获取的逐层拼接字符串，将结果保存在 KMPMRef中
     * 
     * @param ref summary.json中获取的字符串, 如 "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0"
     * @return KMPMRef 返回组装的 KMPMRef 结构体
     */
    static KMPMRef parseSummaryFuzzyKMPMRef(const std::string &ref);

    /**
     * @brief 根据 ref 构造完整的 ostree ref，并和summary.json中层次一样
     * 
     * @param pmRef 传入的 KMPMRef结构体
     * @return std::string 返回完整的 ostree ref, 和 summary.json 保持一致
     */
    static std::string dupKMPMCompletedRef(const KMPMRef &pmRef);

    /**
     * @brief 打印 KMRef 结构体
     * 
     * @param kmRef KMRef 结构体
     */
    static void printKMRef(const KMRef &kmRef);

    /**
     * @brief  打印解析前 ref 和 解析后 KMPMRef 结构体
     * 
     * @param ref 解析前 ref
     * @param kmRef  解析后 KMPMRef 结构体
     */
    static void printKMPMRef(const std::string &ref, const KMPMRef &kmRef);

    /**
     * @brief 根据三段式规范构造最新的Ref
     * 
     * @param name 软件包标识
     * @param channel 发布渠道
     * @param version 软件版本，4 位数字，使用.分隔，遵循语义化版本控制规范（SemVer）规范
     * @param arch 系统架构描述字符串
     * @param module  模块，为了方便软件包分发而拆分的文件集合
     * @return KMRef 返回组装的 KMRef 结构体
     */
    static KMRef constructRef(const std::string &name, const std::string &channel, const std::string &version, const std::string &arch, const std::string &module);

    static KMRef constructRef(KaimingKinds kind, const std::string &name, const std::string &arch, const std::string &branch);
    static std::string dupPref(const KMRef &ref);
    static std::string dupRef(const KMRef &ref);

    static bool isCharSafe(gunichar c);

    /**
     * @brief 通过 appid 获取默认的 ref 类型
     *
     * @param pref appid，类型如 org.kde.kclock
     * @return std::string 返回获取的 ref 类型: "app", "runtime"
     */
    static std::string getDefaultTypeForType(std::string pref);

    /**
     * @brief : std::vector<std::string>转换成std::list<std::string>
     */
    static std::list<std::string> vectorToList(const std::vector<std::string> &vector);

    static std::string getLibraryDir();

    /**
     * @brief : 获取路径的真实路径
     * @param : [in] path，路径
     * @return: 真实路径,软链接会替换为其连接路径
     */
    static std::string realpath(const std::string &path);

    /**
     * @brief : 将后面的std::vector中的元素追加到前面std::vector中
     * @param : [out] src，追加元素后的std::vector
     * @param : [in] dest，待追加的std::vector
     */
    static void append(std::vector<KMRef> &src, std::vector<KMRef> &&dest);

    /**
     * @brief : 规范化文件路径
     * @param : [in] path，原始文件路径
     * @return: 规范化后的文件路径
     */
    static std::string canonicalizeFilename(const std::string &path);

    static GBytes *readAllBytesByFd(int fd, std::string &errMsg);

    /* 有时这是/var/run，它是一个符号链接，当我们将其作为路径传递到沙箱时会引起奇怪的问题 */
    static std::string getRealXdgRuntimeDir();

    /* 获取PulseAudio用于其套接字的运行时目录。 */
    static std::string getPulseRuntimeDir();

    /*  获取PulseAudio用于其配置的目录。*/
    static std::string getPulseHome();

    /* 获取PulseAudio使用的机器ID。这是systemd/D-Bus机器ID，否则是主机名。 */
    static std::string getPulseMachineId();

    static bool argumentNeedsQuoting(const char *arg);

    /**
     * @brief : 转移T*指针的所有权
     */
    template<typename T>
    static T *stealPointer(T **p)
    {
        if (p == nullptr || *p == nullptr)
            return nullptr;

        T *r = *p;
        *p = nullptr;
        return r;
    }

    /**
     * @brief : 将字节大小转换为GB等表示的字符串
     * @param : [in] size, 字节大小
     * @return: GB/MB/KB等表示的字符串型大小
     */
    static std::string formatSize(std::uint64_t size);

    /**
     * @brief : 判断是否数字字符串
     * @return: true-表示是数字字符串；false-表示不是纯数字字符串
     */
    static bool isInteger(const char *str);

    static bool isStringEqual(std::string str1, std::string str2);

    /**
     * @brief 计算字符串中以空格为分隔符的参数个数
     *
     * @param str 待计算字符串，如 "kigo %u"
     * @return int 返回符串中以空格为分隔符的参数个数，"kigo %u" 参数个数为2
     */
    static int countArgsNumber(const std::string &str);

    /**
     * @brief 格式化desktop文件Exec字段
     *
     * @param input 输入待格式化的Exec字段，如 "kigo %u -qwindowicon kigo -qwindowtitle %c"
     * @param refId 输入的refId, 如 "org.kde.kigo"
     * @return std::string 格式化后的字段，如 "kigo --file-forwarding org.kde.kigo @@u %u @@ -qwindowicon kigo -qwindowtitle %c"
     */
    static std::string formatExecString(const std::string &input, const std::string &refId);

    /**
     * @brief 判断输入是否为 'y' 或 'Y', 或者只输入回车键
     *
     * @param input 输入的字符
     * @return true 当前是y, Y, 空格键，允许执行后续操作
     * @return false 当前非默认或y,Y, 不允许执行后续操作
     */
    static bool isYes(const char &input);

    /**
     * @brief 判断输入是否为 'n' 或 'N'
     *
     * @param input 输入的字符
     * @return true 当前是n, N
     * @return false 当前不是 n, N
     */
    static bool isNo(const char &input);

    /**
     * @brief : 将变量param数值由from更改为to
     * @param : [in | param int64_t 变量
     * @param : [in] from, 需要更改的数值
     * @param : [in] to, 更改后的数值
     */
    static void replaceInt(int64_t &param, const int64_t &from, const int64_t &to);
    static void replaceInt(uint64_t &param, const int64_t &from, const int64_t &to);
    static void replaceInt(std::optional<int64_t> &param, const int64_t &from, const int64_t &to);
    static void replaceInt(std::optional<uint64_t> &param, const int64_t &from, const int64_t &to);
    static void replaceInt(std::optional<std::vector<int64_t>> &param, const int64_t &from, const int64_t &to);

    /**
     * @brief: 安全转义路径,避免特殊字符（如 (、)、空格）被 shell 错误解析
     * @return 返回转义后的路径
     */
    template<typename T>
    static std::string escapePath(const T &path)
    {
        std::string str;

        if constexpr (std::is_convertible_v<T, std::filesystem::path>)
        {
            str = std::filesystem::path(path).string();
        }
        else
        {
            str = path;
        }

        if (!str.empty() && str.back() != '/')
        {
            str += '/';
        }

        return "\"" + str + "\"";
    }
};

#endif // KMUTIL_H
