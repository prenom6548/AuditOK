/**
 * @file system_paths.h
 * @brief 系统路径和常量定义
 * 
 * 集中管理系统中使用的各种路径、魔法数字等常量
 */

#ifndef SYSTEM_PATHS_H
#define SYSTEM_PATHS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 日志相关 ==================== */

/** 默认日志目录 */
#define DEFAULT_LOG_DIR         "/var/log/plat-sec-patrol"

/** 日志文件名前缀 */
#define LOG_FILE_PREFIX         "log-"

/** 日志文件扩展名 */
#define LOG_FILE_EXTENSION      ".txt"

/** 日志文件路径最大长度 */
#define MAX_LOG_PATH_LEN        1024


/* ==================== 插件相关 ==================== */

/** 默认插件目录（可由 CMake 通过 target_compile_definitions 覆盖） */
#ifndef DEFAULT_PLUGIN_DIR
#define DEFAULT_PLUGIN_DIR      "/usr/local/lib/plat-sec-patrol/plugins"
#endif

/** 插件子目录 - 环境感知 */
#define PLUGIN_SUBDIR_ENV       "environment"

/** 插件子目录 - 能力库 */
#define PLUGIN_SUBDIR_CAP       "capability"

/** 插件文件扩展名 */
#define PLUGIN_EXTENSION        ".so"

/** 插件路径最大长度 */
#define MAX_PLUGIN_PATH_LEN     1024


/* ==================== 系统信息文件路径 ==================== */

/* CPU 相关 */
/** /proc/cpuinfo 路径 */
#define PROC_CPUINFO            "/proc/cpuinfo"

/** /proc/version 路径 */
#define PROC_VERSION            "/proc/version"

/** /proc/modules 路径 */
#define PROC_MODULES            "/proc/modules"

/* 固件相关 */
/** EFI 固件目录 */
#define SYS_FIRMWARE_EFI        "/sys/firmware/efi"

/** EFI 平台大小文件 */
#define EFI_FW_PLATFORM_SIZE    "/sys/firmware/efi/fw_platform_size"

// /** DMI BIOS 厂商信息 */
// #define SYS_DMI_BIOS_VENDOR     "/sys/class/dmi/id/bios_vendor"

// /** DMI BIOS 版本信息 */
// #define SYS_DMI_BIOS_VERSION    "/sys/class/dmi/id/bios_version"

// /** DMI BIOS 日期信息 */
// #define SYS_DMI_BIOS_DATE       "/sys/class/dmi/id/bios_date"

// /** DMI 板厂商信息 */
// #define SYS_DMI_BOARD_VENDOR    "/sys/class/dmi/id/board_vendor"

// /** DMI 板名称 */
// #define SYS_DMI_BOARD_NAME      "/sys/class/dmi/id/board_name"

// /** DMI 产品厂商信息 */
// #define SYS_DMI_PRODUCT_VENDOR  "/sys/class/dmi/id/sys_vendor"

// /** DMI 产品名称 */
// #define SYS_DMI_PRODUCT_NAME    "/sys/class/dmi/id/product_name"

/* OS 相关 */
/** OS 发行版信息文件 */
#define ETC_OS_RELEASE          "/etc/os-release"

/** 内核配置（压缩） */
#define PROC_KERNEL_CONFIG_GZ   "/proc/config.gz"

/** 内核配置（boot 目录）格式 */
#define BOOT_CONFIG_FORMAT      "/boot/config-%s"

/** 内核源码目录配置 */
#define SRC_LINUX_CONFIG        "/usr/src/linux/.config"


/* ==================== 缓冲区大小 ==================== */

/** 通用行缓冲区大小 */
#define LINE_BUFFER_SIZE        4096

/** 小缓冲区大小 */
#define SMALL_BUFFER_SIZE       256

/** 中等缓冲区大小 */
#define MEDIUM_BUFFER_SIZE      1024

/** 大缓冲区大小 */
#define LARGE_BUFFER_SIZE       8192


/* ==================== 表格输出相关 ==================== */

/** 表格 Name 列宽度 */
#define TABLE_COL_NAME_WIDTH    25

/** 表格 Overall Status 列宽度 */
#define TABLE_COL_OVERALL_WIDTH 18

/** 表格 CPU Layer 列宽度 */
#define TABLE_COL_CPU_WIDTH     12

/** 表格 Firmware Layer 列宽度 */
#define TABLE_COL_FIRMWARE_WIDTH 16

/** 表格 OS Layer 列宽度 */
#define TABLE_COL_OS_WIDTH      12

/** 表格 Description 列宽度 */
#define TABLE_COL_DESC_WIDTH    35


/* ==================== 其他常量 ==================== */

/** 时间戳缓冲区大小 */
#define TIMESTAMP_BUFFER_SIZE   64

/** 命令输出缓冲区大小 */
#define CMD_OUTPUT_BUFFER_SIZE  4096

/** 最大重试次数 */
#define MAX_RETRY_COUNT         3

/** 默认超时时间（秒） */
#define DEFAULT_TIMEOUT_SEC     30


#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_PATHS_H */
