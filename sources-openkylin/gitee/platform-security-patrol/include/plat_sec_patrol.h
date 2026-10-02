/**
 * @file plat_sec_patrol.h
 * @brief Platform Security Patrol - 主头文件，包含所有公共定义
 * 
 * 平台安全能力检测与使能工具
 */

#ifndef PLAT_SEC_PATROL_H
#define PLAT_SEC_PATROL_H

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 版本信息 ==================== */
#define PLAT_SEC_PATROL_VERSION_MAJOR 0
#define PLAT_SEC_PATROL_VERSION_MINOR 1
#define PLAT_SEC_PATROL_VERSION_PATCH 0
#define PLAT_SEC_PATROL_VERSION_STRING "0.1.0"

/* ==================== 错误码定义 ==================== */
typedef enum {
    ERR_OK = 0,
    ERR_GENERIC = -1,
    ERR_PERMISSION = -2,
    ERR_NOT_FOUND = -3,
    ERR_INVALID_PARAM = -4,
    ERR_MEMORY = -5,
    ERR_IO = -6,
    ERR_PLUGIN_LOAD = -7,
    ERR_PARSE = -8,
    ERR_NOT_SUPPORTED = -9,
    ERR_EXECUTION = -10,
    ERR_DEPENDENCY = -11,
    ERR_VALIDATION = -12
} psp_error_t;

/* ==================== 日志级别 ==================== */
typedef enum {
    LOG_LEVEL_ERROR = 0,     /**< 错误级别 */
    LOG_LEVEL_WARN = 1,      /**< 警告级别 */
    LOG_LEVEL_INFO = 2,      /**< 信息级别 */
    LOG_LEVEL_DEBUG = 3      /**< 调试级别 */
} log_level_t;

/* ==================== 能力状态 ==================== */
typedef enum {
    CAP_STATUS_SUPPORTED = 0,     /**< 硬件支持 */
    CAP_STATUS_UNSUPPORTED = 1,   /**< 硬件不支持 */
    CAP_STATUS_DISABLED = 2,      /**< 支持但当前禁用 */
    CAP_STATUS_AVAILABLE = 3,     /**< 配置完备，可用 */
    CAP_STATUS_UNAVAILABLE = 4,   /**< 配置不完备，不可用 */
    CAP_STATUS_UNKNOWN = 5        /**< 无法判定 */
} cap_status_t;

/* ==================== 层次状态 ==================== */
typedef enum {
    LAYER_STATUS_SUPPORTED = 0,     /**< 支持 */
    LAYER_STATUS_UNSUPPORTED = 1,   /**< 不支持 */
    LAYER_STATUS_DISABLED = 2,      /**< 支持但当前禁用 */
    LAYER_STATUS_AVAILABLE = 3,     /**< 可用 */
    LAYER_STATUS_UNAVAILABLE = 4,   /**< 不可用 */
    LAYER_STATUS_UNKNOWN = 5        /**< 未知 */
} layer_status_t;

/* ==================== 插件类型 ==================== */
typedef enum {
    PLUGIN_TYPE_COLLECTOR = 0,    /* 环境感知插件 */
    PLUGIN_TYPE_CAPABILITY = 1    /* 能力库插件 */
} plugin_type_t;

/* ==================== 操作命令类型 ==================== */
typedef enum {
    CMD_CHECK = 0,      /* 检测命令 */
    CMD_ENABLE = 1,     /* 使能命令 */
    CMD_DISABLE = 2,    /* 禁用命令 */
    CMD_LIST = 3        /* 列表命令 */
} command_t;

/* ==================== 输出格式 ==================== */
typedef enum {
    FORMAT_TEXT = 0,
    FORMAT_JSON = 1
} output_format_t;

/* ==================== 工具函数宏 ==================== */

/** 获取数组元素个数 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/** 结构体成员偏移量（使用标准 offsetof） */

/** 从成员指针获取结构体指针（容器指针） */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/** 最大值 */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/** 最小值 */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** 安全释放内存并置 NULL */
#define SAFE_FREE(ptr) do { \
    if ((ptr) != NULL) { \
        free((void*)(ptr)); \
        (ptr) = NULL; \
    } \
} while(0)

/** 字符串化 */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/* ==================== 工具函数 ==================== */

/**
 * @brief 将能力状态转换为字符串
 */
static inline const char* cap_status_to_string(cap_status_t status)
{
    switch (status) {
        case CAP_STATUS_SUPPORTED: return "supported";
        case CAP_STATUS_UNSUPPORTED: return "unsupported";
        case CAP_STATUS_DISABLED: return "disabled";
        case CAP_STATUS_AVAILABLE: return "available";
        case CAP_STATUS_UNAVAILABLE: return "unavailable";
        case CAP_STATUS_UNKNOWN: return "unknown";
        default: return "invalid";
    }
}

/**
 * @brief 将层次状态转换为字符串
 */
static inline const char* layer_status_to_string(layer_status_t status)
{
    switch (status) {
        case LAYER_STATUS_SUPPORTED: return "supported";
        case LAYER_STATUS_UNSUPPORTED: return "unsupported";
        case LAYER_STATUS_DISABLED: return "disabled";
        case LAYER_STATUS_AVAILABLE: return "available";
        case LAYER_STATUS_UNAVAILABLE: return "unavailable";
        case LAYER_STATUS_UNKNOWN: return "unknown";
        default: return "invalid";
    }
}

/**
 * @brief 将日志级别转换为字符串
 */
static inline const char* log_level_to_string(log_level_t level)
{
    switch (level) {
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARN: return "WARN";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* PLAT_SEC_PATROL_H */
