/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMINFOJSON_H__
#define __KMINFOJSON_H__

#include "KMJsonHelper.h"
#include "KMCommonTypePrivate.h"

struct Permissions
{
    bool all = false;
    bool xdgUserDirs = false;
    bool network = false;
    bool scanning = false;
    bool bluetooth = false;
    bool microphone = false;
    bool camera = false;
    bool usb = false;
    bool serial = false;
    bool autostart = false;
    bool systemNotice = false;
    bool systemTime = false;
    bool systemServer = false;
    bool windowTop = false;
    bool preventSleep = false;
    bool systemDisplayResolution = false;
    bool systemDisplayBrightness = false;
    bool systemAudioSettings = false;
    bool systemAudioSize = false;
    bool systemCrontab = false;
    bool systemPowerMode = false;
    bool systemPowerPolicy = false;
    std::vector<std::string> filesystemDirs;
};

struct DebInfo
{
    std::string package;
    std::string version;
    std::string section;
    std::string package_type;
    std::string recommends;
    std::string uploaders;
    std::string provides;
    std::string origin;
    std::string multi_arch;
    std::string build_depends;
    std::string source;
    std::string suggests;
    std::string conflicts;
    std::string breaks;
    std::string replaces;
    std::string vcs_git;
    std::string vcs_browser;
};

struct InstallActions
{
    std::vector<std::string> start_service;
    std::vector<std::string> stop_service;
    std::vector<std::string> stop_process;
    std::vector<std::string> chmod;
    std::vector<std::string> add_group;
    std::vector<std::string> autostart;
    std::vector<std::string> remove;
};

struct UninstallActions
{
    std::vector<std::string> stop_service;
    std::vector<std::string> stop_process;
    std::vector<std::string> remove;
};

struct Annotations
{
    // 是否提权运行（即需要超级用户权限）
    bool privilege = false;

    // 是否使用LD_PRELOAD
    bool hostPlugins = false;

    // 是否系统应用（即应用安装在/usr、/etc等系统目录中，而不是遵循规范安装到/opt/apps/${id}/files为前缀的目录中）
    bool sysapp = false;

    // 是否需要polkit鉴权弹窗
    bool polkit = false;

    // 是否需要使用gpu驱动
    bool gpu = false;

    // 是否需要处理不可变系统托盘图标
    bool tray = false;

    /*
        编译依赖的所有包列表。
        格式为：${id}/${version}，其中id为必须说明，version可以没有(没有版本则下载安装最新版本)。
        如：top.openkylin.mpv/0.32.0。
        一般只有depend类型的包才有，用于递归依赖。
    */
    std::vector<std::string> depends;

    /*
        宿主机覆盖容器中对应的文件
    */
    std::map<std::string, std::string> covers;

    /*
        依赖的其他应用的程序，每个数组元素都是一个可执行程序。可以是绝对路径，也可以只写程序名，建议和本应用中使用方式一致。
        如：
        开明应用A需要调用开明应用B的可执行程序b，
            - 如果在A中使用程序b是使用的绝对路径，如：/usr/bin/b [args]，那么在此配置中也配置为/usr/bin/b
            - 如果在A中使用程序b是只用程序名，如：b [args]，那么在此处配置中也建议配置为b
    */
    std::vector<std::string> dependApps;

    /*
        安装时，需要创建桌面快捷方式的desktop文件名列表。这些desktop文件必须安装到标准路径/opt/apps/${id}/file/share/applications/目录
        （kylin.sysapp=true时安装到/usr/share/applications/目录）下此功能才能生效。
    */
    std::vector<std::string> desktops;

    /*
        dbus服务的白名单列表。每个数组元素都是一个可执行程序完整路径。dbus管控的判断逻辑是：
        (1)判断/proc/进程id/exe 和白名单中的名字是否一致
        (2)判断白名单中的文件是否存在
     */
    std::vector<std::string> kylin_dbus_whitelist;

    //是否为增量包
    bool deltas = false;

    //可使用该增量包升级的版本
    std::vector<std::string> preVersion;

    // ------------------原生部署专用的属性begin----------------------------
    // 是否需要部署到宿主环境（即原生部署，不在容器中运行）
    bool hostapp = false;

    // 是否支持脚本编译
    bool hostscript = false;

    // 是否为增量包
    // bool deltas = false;

    /*
        安装时，需要启动的服务列表。如run-wine.mount是kylinwine.wecom要启动的服务
    */
    std::vector<std::string> service;

    /*
        安装时，需要添加的系统用户组列表。需要在服务启动之前执行，如"kylin.addsystemgroup" : [“docker”]
    */
    std::vector<std::string> sysgroup;

    /*
        安装时，设置开机自启列表
    */
    std::vector<std::string> autostart;

    /*
        安装时，设置目录或文件的权限。 权限和文件路径之间用空格分隔，如："kylin.chmod": ["777 /etc/qemu-delete-bridge", "777 /etc/qemu-ifup"]
    */
    std::vector<std::string> chmods;

    /*
        安装时，需关闭的程序列表
    */
    std::vector<std::string> installkills;

    /*
        卸载时，需关闭的程序列表
    */
    std::vector<std::string> uninstallkills;

    /*
        安装时，需停止的服务列表
    */
    std::vector<std::string> stopservices;

    /*
        卸载相关目录列表
    */
    std::vector<std::string> rmdir;

    /*
        deb包信息
    */
    DebInfo debinfo;
    // ------------------原生部署专用的属性end----------------------------

    /*
        卸载相关文件列表
    */
    std::vector<std::string> unlink;

    // 安装动作
    InstallActions install_actions;

    // 卸载动作
    UninstallActions uninstall_actions;
};

/**
 * @brief : 读写info.json配置文件的类
 */
class KMInfoJson
{
public:
    /**
     * @brief : 解析info中的base部分
     */
    class InfoBase
    {
    public:
        InfoBase() = default;
        ~InfoBase() = default;
        /**
         * @brief : 解析base获取id与version
         */
        void baseParse();
        /**
         * @brief : 解析version
         */
        void versionParse();
        /**
         * @brief : 检查版本号是否匹配
         * @param : [in] ver 待比较的版本号
         * @return : true-匹配；false-不匹配
         */
        bool checkVersion(const InfoBase &ver);

    public:
        std::string m_baseString;
        std::string m_baseId;
        std::string m_baseVersion;
        std::string m_versionMajor;
        std::string m_versionMinor;
    };

public:
    KMInfoJson() = default;
    ~KMInfoJson() = default;

    /**
     * @brief : 加载json文件
     * @param : [in] jsonFile, json文件
     * @return: nlohmann::json, 解析后的json对象
     * @throw : KMException
     */
    void loadFile(const std::string &jsonFile);

    /**
     * @brief : 保存json对象到文件
     * @param : [in] jsonFile, json文件
     * @throw : KMException
     */
    void saveFile(const std::string &jsonFile);

    void setRoot(nlohmann::json &root);

    nlohmann::json root();

    /**
     * @brief : 解析JSON中的permission字段
     * @param : 包含权限信息的nlohmann::json对象
     * @return : void
     */
    void parsePermissions();

    /**
     * @brief 获取已解析的权限信息
     * @return const Permissions& 返回对已解析权限结构体的常量引用
     */
    const Permissions &getPermissions() const;

    /**
     * @brief : 解析JSON中的annotations字段
     * @param : 包含自定义信息
     * @return : void
     */
    void parseAnnotations();

    /**
     * @brief 获取已解析的权限信息
     * @return const Permissions& 返回对已解析权限结构体的常量引用
     */
    const Annotations &getAnnotations() const;

    KMRef baseRef() const;

    KMRef runtimeRef() const;

    static KMRef parseKMRef(const std::string &ref);

    std::string &operator[](const std::string &key);

private:
    void fromJson();

    void toJson();

    void permissionToJson();

    void annotationsToJson();

public:
    // info.json属性域，暂时不进行封装

    // 协议版本号
    std::string schema_version = "1.0";

    // 应用id
    std::string id;

    // 架构
    std::vector<std::string> archs;

    // base
    std::string base;

    // 运行时
    std::string runtime;

    std::string depend;

    // 渠道
    std::string channel;

    // kaiming run执行命令
    std::vector<std::string> commands;

    // 应用名称
    std::string name;

    // 多语言应用名称，例：name[zh_CN]
    std::map<std::string, std::string> names;

    // 描述
    std::string description;

    // 多语言描述，例：description[zh_CN]
    std::map<std::string, std::string> descriptions;

    // kind {app/runtime/base}
    std::string kind;

    // module (binary、devel)
    std::string module;

    // 安装大小
    uint64_t size = 0;

    // 版本
    std::string version;

    // 环境变量
    std::map<std::string, std::string> envs;

    // 权限
    Permissions permissions;

    // 许可证信息，SPDX格式
    std::string license;

    // 首页
    std::string homepage;

    // 维护者
    std::vector<std::string> maintainers;

    // 自定义
    Annotations annotations;

    // plugins extend
    std::vector<std::string> export_plugins;

private:
    // 数据成员域
    nlohmann::json m_root;
};

#endif // !__KMINFOJSON_H__
