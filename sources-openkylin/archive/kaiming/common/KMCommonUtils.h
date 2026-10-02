#ifndef KMCOMMON_UTILS_H
#define KMCOMMON_UTILS_H

#include <glib.h>
#include <gio/gio.h>
#include <string>
#include <mutex>
#include <functional>
#include "KMConfigData.h"

typedef std::function<void(void *, const char *)> callback;

class KMCommonUtils
{
public:
    KMCommonUtils() = default;
    ~KMCommonUtils() = default;

public:
    /**
     * @brief 运行gtask命令
     * @param task GTask对象
     * @param command 要执行的命令
     * @param pwd 命令执行的工作目录
     * @param cb 回调函数
     * @param args 回调函数参数
     */
    static void runGtaskCommand(GTask *task, const std::string &command, const std::string &pwd, callback cb, void *args);

    /**
     * @brief 准备命令行
     * @param command 原始命令
     * @return true 成功 false 失败
     */
    static bool prepareCommand(const std::string &command, std::string &cmd);

    /**
     * @brief 准备xdg-open命令
     * @param path 要打开的路径
     * @param appId 应用ID
     * @param appVersion 应用版本
     * @param cmd 输出命令
     * @param true 成功 false 失败
     */
    static bool prepareXdgOpenCmd(const std::string &path, const std::string &appId, const std::string &version, std::string &cmd);

    /**
     * @brief 参数解析
     * @param str 输入字符串
     * @param args 输出参数列表
     * @return true 存在 false 不存在
     */
    static bool parseArgs(const std::string &str, std::vector<std::string> &args);

    /**
     * @brief 获取ps信息
     * @param psInfos 获取的ps信息结构
     */
    static void getPsInfo(KMPSInfoList &psInfos, std::string targetAppid = "", bool isFind = false);

    /**
     * @brief 获取子目录
     * @param path 路径
     * @return 返回子目录路径
     */
    static std::string getSingleSubdirectory(const std::string &path);

    /**
     * @brief brief 获取当前运行的进程信息，并以 JSON 格式返回。
     * @param[out] ret 返回状态，0 表示成功，-1 表示失败。
     * @param[out] msg 如果成功，包含 JSON 字符串；失败时包含错误描述。
     * @return true 表示成功；false 表示失败。
     */
    static bool ps(int &ret, std::string &msg);

    /**
     * @brief 清理后台进程
     * @param nsid 命名空间ID
     */
    static void clearBackendProc(const std::string &nsid);

    /**
     * @brief 停止正在运行的容器
     * @param[out] ret 返回状态码，成功为 0，失败为 -1。
     * @param[out] msg 若成功则为空，失败时包含错误信息（如未找到目标进程）。
     * @param[in] appid 可为应用 ID、进程 ID 或容器 ID
     * @return true 表示成功终止目标进程；false 表示未找到对应进程或执行失败。
     */
    static bool kmkill(int &ret, std::string &msg, const std::string &appid);

    /**
     * @brief 查询运行状态
     * @param[out] ret 返回状态码，成功为 0，失败为 -1。
     * @param[out] msg 若成功则为空，失败时包含错误信息（如未找到目标进程）。
     * @param[in] appid 可为应用 ID、进程 ID 或容器 ID
     * @return true 表示成功终止目标进程；false 表示未找到对应进程或执行失败。
     */
    static bool getRunStatus(const std::string &appid, const std::string &uid, std::string &cid);

    /**
     * @brief 获取子目录
     * @param path 路径
     * @return 返回子目录路径
     */
    static gchar **makeArgv(bool isrun, const gchar *const *options, const gchar *const *soptions, const gchar *cid, const gchar *command);

    /**
     * @brief 重定向标准输入输出到 /dev/null
     */
    static void redirectToNull();
};

#endif // KMCOMMON_UTILS_H
