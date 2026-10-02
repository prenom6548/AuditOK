/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMINFOJSON_H__
#define __KMINFOJSON_H__

#include "KMNlohmannJsonHelper.h"

/**
 * @brief : 读写info.json配置文件的类
 */ 
class KMInfoJson
{
public:
    /**
     * 清单内部类，用于描述permission filesystem属性
     */ 
    struct Filesystem
    {
        bool m_xdgUserDirs = false;
        std::vector<std::string> m_filesystemDirs;
    };

    /**
     * 清单内部类，用于描述permission net属性
     */
    struct Net
    {
        bool m_network = false;
        bool m_scanning = false;
    };

   /**
     * 清单内部类，用于描述permission device属性
     */
    struct Device
    {
        bool m_bluetooth = false;
        bool m_microphone = false;
        bool m_camera = false;
        bool m_usb = false;
        bool m_serial = false;
    };

    /**
     * 清单内部类，用于描述permission processManager属性
     */
    struct ProcessManager
    {
        bool m_autostart = false;
    };

    /**
     * 清单内部类，用于描述permission others属性
     */
    struct Others
    {
        bool m_systemNotice = false;
        bool m_systemTime = false;
        bool m_systemServer = false;
        bool m_windowTop = false;
        bool m_preventSleep = false;
        bool m_systemDisplayResolution = false;
        bool m_systemDisplayBrightness = false;
        bool m_systemAudioSetting = false;
        bool m_systemAudioSize = false;
        bool m_systemCrontab = false;
        bool m_systemPowerMode = false;
        bool m_systemPowerPolicy = false;
    };

    /**
     * 清单内部类，用于描述permission属性
     */ 
    struct Permission
    {
        bool m_none = false;
        Filesystem m_filesystem;
        Net m_net;
        Device m_device;
        ProcessManager m_processManager;
        Others m_others;
    };

    struct InstallActions
    {
        /*
            开明软件包在安装 / 卸载时要启动的服务
            示例：
            "annotations": {
                "kylin.install": {
                    "start_service": [
                        "docker.service",
                        "nm.service"
                    ]
                }
            }
        */
        std::vector<std::string> start_service;

        /*
            开明软件包在安装 / 卸载时要停止的服务
            示例：
            "annotations": {
                "kylin.install": {
                    "stop_service": [
                        "docker.service",
                        "nm.service"
                    ]
                }
            }
        */
        std::vector<std::string> stop_service;

        /*
            开明软件包在安装 / 卸载时要停止的进程
            示例：
            "annotations": {
                "kylin.install": {
                    "stop_process": [
                        "kylin-video",
                        "kylin-music"
                    ]
                }
            }
        */
        std::vector<std::string> stop_process;

        /*
            开明软件包在安装时要修改文件或目录权限
            示例：
            "annotations": {
                "kylin.install": {
                    "chmod": [
                        "777 /etc/qemu-delete-bridge",
                        "755 /etc/xxx"
                    ]
                }
            }
        */
        std::vector<std::string> chmod;

        /*
            开明软件包在安装时要创建的用户组
            示例：
            "annotations": {
                "kylin.install": {
                    "add_group": [
                        "docker"
                    ]
                }
            }
        */
        std::vector<std::string> add_group;

        /*
            开明软件包在安装时要开机自启的 desktop 文件
            示例：
            "annotations": {
                "kylin.install": {
                    "autostart": [
                        "/opt/apps/xx.xx.video/share/application/video.desktop",
                        "/opt/apps/xx.xx.video/share/application/music.desktop"
                    ]
                }
            }
        */
        std::vector<std::string> autostart;

        /*
            开明软件包在安装 / 卸载时要删除的文件或目录
            示例：
            "annotations": {
                "kylin.install": {
                    "remove": [
                        "/etc/qemu-delete-bridge",
                        "$HOME/.config/xx.xx"
                    ]
                }
            }
        */
        std::vector<std::string> remove;
    };

    struct UninstallActions
    {
        std::vector<std::string> stop_service;
        std::vector<std::string> stop_process;
        std::vector<std::string> remove;
    };

    /**
     * 扩展字段
     */
    struct Annotations
    {
        // 是否提权运行（即需要超级用户权限）
        bool kylin_privilege = false;

        // 是否系统应用（系统应用,即应用安装在/usr、/etc等系统目录中，而不是遵循规范安装到/opt/apps/${id}/files为前缀的目录中）
        bool kylin_sysapp = false;

        // 是否需要使用gpu驱动
        bool kylin_gpu = false;

        // 是否需要polkit鉴权弹窗
        bool kylin_polkit = false;

        /*
            依赖的其他应用的程序，每个数组元素都是一个可执行程序。可以是绝对路径，也可以只写程序名，建议和本应用中使用方式一致。
            如：
            开明应用A需要调用开明应用B的可执行程序b，
                - 如果在A中使用程序b是使用的绝对路径，如：/usr/bin/b [args]，那么在此配置中也配置为/usr/bin/b
                - 如果在A中使用程序b是只用程序名，如：b [args]，那么在此处配置中也建议配置为b
        */ 
        std::vector<std::string> kylin_depend_apps;

        /*
            安装时，需要创建桌面快捷方式的desktop文件名列表。这些desktop文件必须安装到标准路径/opt/apps/${id}/file/share/applications/目录
            （kylin.sysapp=true时安装到/usr/share/applications/目录）下此功能才能生效。
        */
        std::vector<std::string> kylin_desktops;

        /*
            卸载时，需关闭的程序列表
        */
        std::vector<std::string> kylin_uninstallkills;

        /*
            安装时，需停止的服务列表
        */
        std::vector<std::string> kylin_stopservices;

        /*
            卸载时需要额外删除的目录列表（宿主机的路径，非容器内路径）。
            注意：若需要删除家目录下的子目录，可以参考如下配置
            kylin.rmdir:
                - $HOME/.config/kylin-aiassistant
        */
        std::vector<std::string> kylin_rmdir;

        /*
            卸载时需要额外删除的文件列表（宿主机的路径，非容器内路径）。
            注意：若需要删除家目录下的文件，可以参考如下配置
            kylin.rmdir:
                - $HOME/.config/kylin-aiassistant/config
        */
        std::vector<std::string> kylin_unlink;

        /*
            dbus服务的白名单列表。每个数组元素都是一个可执行程序完整路径。dbus管控的判断逻辑是：
            (1)判断/proc/进程id/exe 和白名单中的名字是否一致
            (2)判断白名单中的文件是否存在
         */
        std::vector<std::string> kylin_dbus_whitelist;

        // 安装动作
        InstallActions kylin_install;

        // 卸载动作
        UninstallActions kylin_uninstall;

        /////////// 原生部署属性 ////////////////////////

        // 是否需要部署到宿主环境（即原生部署，不在容器中运行）
        bool kylin_hostapp = false;
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
    void loadFile(const std::string& jsonFile);

    /**
     * @brief : 保存json对象到文件
     * @param : [in] jsonFile, json文件
     * @throw : KMException
     */ 
    void saveFile(const std::string& jsonFile);
    void saveModulesFile(const std::string& jsonFile);

private:
    void fromJson();

    void toJson();

    void determineModule();

    void parsePermissionFromJson();

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
    std::map<std::string, std::string> modules;
    std::string module;

    // 安装大小
    uint64_t size ;

    // 版本
    std::string version;

    // 环境变量
    std::map<std::string, std::string> envs;

    // 权限
    std::vector<std::string> permissions;
    Permission permission;

    // 许可证信息，SPDX格式
    std::string license;

    // 首页
    std::string homepage;

    // 维护者
    std::vector<std::string> maintainers;

    // 扩展字段
    Annotations annotations;

    /*
        导出的插件路径列表。此路径以构建结果的files目录为根。
        如：
        export_plugins: [
            "/usr/lib/mozilla/plugin",
            "/opt/kingsoft/wps/office6"
        ]
        1）构建出开明包后，安装时会将构建结果目录${build-dir}/files/usr/lib/mozilla/plugin和${build-dir}/files/opt/kingsoft/wps/office6导出到宿主机的目录/opt/kaiming/exports/plugins/usr/lib/mozilla/plugin和/opt/kaiming/exports/plugins/opt/kingsoft/wps/office6。
        2）安装时同时将导出插件目录记录到/opt/kaiming/exports/plugins/plugins.list中
        3）应用运行时，根据2）中文件记录的目录列表挂载到容器中
     */
    std::vector<std::string> export_plugins;

private:
    // 数据成员域
    nlohmann::json m_root;
};

#endif // !__KMINFOJSON_H__
