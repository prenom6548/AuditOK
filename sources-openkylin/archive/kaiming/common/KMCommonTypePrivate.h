/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMCOMMONTYPESPRIVATE_H__
#define __KMCOMMONTYPESPRIVATE_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

enum KaimingKinds
{
    KAIMING_KINDS_APP = 1 << 0,
    KAIMING_KINDS_RUNTIME = 1 << 1,
    KAIMING_KINDS_BASE = 1 << 2,
    KAIMING_KINDS_ALL = KAIMING_KINDS_APP | KAIMING_KINDS_RUNTIME | KAIMING_KINDS_BASE
};

#define BASE_TYPE_BASE "base"
#define BASE_TYPE_RUNTIME "runtime"
#define BASE_TYPE_APP "app"
#define BASE_TYPE_DEPEND "depend"
#define BASE_TYPE_EXTENSION "extension"

#define MODULE_NAME_DEV "devel"
#define MODULE_NAME_BINARY "binary"

typedef struct
{
    std::string id;  /// 软件包标识是识别一个软件包身份的字符串，通常相同标识的软件包由一个厂商进行维护与更新。
                     /// 1. 该字符串由 [a-z] [A-Z] [0-9] [-_.] 字符组成。
                     /// 2. 对于上层应用，其标识必须使用反向域名表示法进行命名。即 org.kde.kclock 形式。
                     /// 3. 举例：org.kde.kclock, org.kde.kcalc

    std::string channel;  // 发布渠道,默认为 "stable" 。由小写字母与下划线组成，额外标识、定制字段（beta、stable）。
    std::string version;  // 软件版本，4 位数字，使用.分隔，遵循语义化版本控制规范（SemVer）规范。上游大部分三段，不遵循规范的根据版本号扩充为四段，第四位保留给安全更新。如 1.0.0.0-beta
    std::string arch;  // 系统架构描述字符串，默认为 "x86_64", 如 x86_64, arm

    std::string module;  /// 模块，为默认为 "binary"，了方便软件包分发而拆分的文件集合。一个软件包可以拆分为binary、devel、locale-zh-cn、locale-en-us、debuginfo等。
                         /// 其中本地化包标识符合ISO 639-1和ISO 3166-1相关定义。dbuginfo默认拆包，其他开发者决定是否拆包，yaml里标记是否拆包，只要安装就会挂载。 

    std::string branch;  // 软件分支, 如 master, stable

    std::string kind;  /// 软件类型, 如 app, runtime，目前通过id以.分隔符的尾部进行确认。
                                                  /// 尾部为runtime,platform,则为 BASE_TYPE_RUNTIME, 尾部为base，则为 BASE_TYPE_BASE，否则为 BASE_TYPE_APP
    bool isSpecialVersion = false;    //是否为指定版本
    std::string baseInstallDir;
    bool isForceInstall = false;
    bool isSysapp = false;
    bool isHostapp = false;
    bool isDeltas = false;
    bool isOfflineRun = false;
    std::vector<std::string> preVersion;
    std::string findPreVersion;
    std::vector<std::string> vecPlugins;
    std::vector<std::string> vecDbusWhitelist;
} KMRef;


/**
 * @brief 包来源枚举
 * 
 */
enum KMPackageSourceType
{
    KMPACKAGE_SOURCE_TYPE_ONLINE = 0,                                                                                  // 在线包
    KMPACKAGE_SOURCE_TYPE_OK = (1 << 0),                                                                               // .ok 离线包
    KMPACKAGE_SOURCE_TYPE_LAYER = (1 << 1),                                                                            // .layer 离线包
    KMPACKAGE_SOURCE_TYPE_OFFLINE = KMPACKAGE_SOURCE_TYPE_LAYER | KMPACKAGE_SOURCE_TYPE_OK // 离线包
};

/**
 * @brief 包管理结构体
 * 
 */
typedef struct _KMPMRef
{
    KMRef ref;

    KMPackageSourceType refSourceType = KMPACKAGE_SOURCE_TYPE_ONLINE; // 区分离线包或在线包，默认 KMPACKAGE_SOURCE_TYPE_ONLINE 表示在线包。离线包格式以  ".layer", ".ok" 为后缀
    std::string completedRef;                                         // 完整的 ref ，如 "stable/x86_64/app/top.openkylin.Music/binary/2.0.0.0"
    uint64_t installedSize;                                           // 安装大小
    std::string name;                                                 // 安装包名称
    std::string description;                                          // 安装描述
} KMPMRef;

enum KMBuildType
{
    KMBUILDTYPE_APP,
    KMBUILDTYPE_RUNTIME,
    KMBUILDTYPE_EXTERNSION,
    KMBUILDTYPE_DEPEND
};

enum KMRunFlags
{
    KAIMING_RUN_FLAG_DEFAULT = 0,
    KAIMING_RUN_FLAG_DEVEL = (1 << 0),
    KAIMING_RUN_FLAG_BACKGROUND = (1 << 1),
    KAIMING_RUN_FLAG_LOG_SESSION_BUS = (1 << 2),
    KAIMING_RUN_FLAG_LOG_SYSTEM_BUS = (1 << 3),
    KAIMING_RUN_FLAG_NO_SESSION_HELPER = (1 << 4),
    KAIMING_RUN_FLAG_MULTIARCH = (1 << 5),
    KAIMING_RUN_FLAG_WRITABLE_ETC = (1 << 6),
    KAIMING_RUN_FLAG_NO_SESSION_BUS_PROXY = (1 << 7),
    KAIMING_RUN_FLAG_NO_SYSTEM_BUS_PROXY = (1 << 8),
    KAIMING_RUN_FLAG_SET_PERSONALITY = (1 << 9),
    KAIMING_RUN_FLAG_FILE_FORWARDING = (1 << 10),
    KAIMING_RUN_FLAG_DIE_WITH_PARENT = (1 << 11),
    KAIMING_RUN_FLAG_LOG_A11Y_BUS = (1 << 12),
    KAIMING_RUN_FLAG_NO_A11Y_BUS_PROXY = (1 << 13),
    KAIMING_RUN_FLAG_SANDBOX = (1 << 14),
    KAIMING_RUN_FLAG_NO_DOCUMENTS_PORTAL = (1 << 15),
    KAIMING_RUN_FLAG_BLUETOOTH = (1 << 16),
    KAIMING_RUN_FLAG_CANBUS = (1 << 17),
    KAIMING_RUN_FLAG_DO_NOT_REAP = (1 << 18),
    KAIMING_RUN_FLAG_NO_PROC = (1 << 19),
    KAIMING_RUN_FLAG_PARENT_EXPOSE_PIDS = (1 << 20),
    KAIMING_RUN_FLAG_PARENT_SHARE_PIDS = (1 << 21),
};

typedef std::basic_string<unsigned char> kmustring;

typedef enum
{
    KAIMING_REPO_UPDATE_FLAG_NONE = 0,
    KAIMING_REPO_UPDATE_FLAG_DISABLE_INDEX = 1 << 0,
} KMRepoUpdateFlags;

// 任务类型定义
enum class TaskType
{
    TASK_TYPE_INSTALL = 0,          // 安装
    TASK_TYPE_UNINSTALL = (1 << 0), // 卸载
    TASK_TYPE_UPDATE = (1 << 1),    // 更新
    TASK_TYPE_REPAIR = (1 << 2),    // 修复
    TASK_TYPE_SEARCH = (1 << 3),    // 查询
    TASK_TYPE_UPGRADE = (1 << 4)    // 更新所有
};


#endif // !__KMCOMMONTYPESPRIVATE_H__
