/**
 * @file cpu_collector.c
 * @brief CPU 信息采集插件 - 增强版
 * 
 * 从/proc/cpuinfo 和其他来源采集详细的 CPU 信息
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "plugin_env_utils.h"
#include "logger.h"

#define MODULE "cpu_collector"

/* 必须导出的插件类型标识 */
const char* plugin_type = "collector";
/**
 * @brief 从字符串中提取第一个符合范围的连续数字序列
 * 
 * 专门用于提取海光 CPU 型号数字，过滤掉核心数、频率等干扰数字。
 * 有效范围：3000 <= number < 8000
 * 
 * @param str 输入字符串
 * @param out_num 输出提取的数字
 * @return 成功返回指向数字后第一个字符的指针，失败返回NULL
 */
static const char* extract_cpu_model_number(const char *str, int *out_num)
{
    if (!str || !out_num) {
        return NULL;
    }
    
    const char *current = str;
    
    /* 遍历字符串，查找所有数字序列 */
    while (*current) {
        /* 查找数字开始位置 */
        if (*current >= '0' && *current <= '9') {
            /* 提取完整数字 */
            int num = atoi(current);
            
            /* 检查是否在有效范围内 (3000-7999) */
            if (num >= 3000 && num < 8000) {
                *out_num = num;
                
                /* 跳过所有数字字符 */
                while (*current >= '0' && *current <= '9') {
                    current++;
                }
                
                return current;
            }
            
            /* 不在范围内，跳过这个数字序列 */
            while (*current >= '0' && *current <= '9') {
                current++;
            }
        } else {
            current++;
        }
    }
    
    return NULL;  /* 没有找到符合条件的数字 */
}

/**
 * @brief 从 environment_t 中提取简化的 CPU 型号标识
 * 
 * 支持多种格式：
 * - "Hygon C86 7390 32-core Processor" → "Hygon C86-3G"
 * - "Hygon C86-4G (OPN:7490)" → "Hygon C86-4G"
 * - "Hygon C86 7185" → "Hygon C86-1G"
 * 
 * 解析策略（优先级从高到低）：
 * 1. 直接匹配标准格式："Hygon C86-XG"
 * 2. 从 OPN 括号中提取型号数字（范围 3000-7999）
 * 3. 从整个字符串中提取型号数字（范围 3000-7999）
 * 4. 降级策略：返回 NULL
 * 
 * @param env 环境信息结构
 * @return CPU型号标识字符串（静态缓冲区），失败返回NULL
 */
static const char* extract_cpu_model(char *model)
{
    if (!model) {
        logger_warn(MODULE, "Environment or CPU model is NULL");
        return NULL;
    }
    
    logger_debug(MODULE, "Extracting CPU model identifier from: %s", model);
    
    /* 检测是否为海光 C86 系列 */
    bool is_hygon_c86 = (strstr(model, "Hygon") != NULL && strstr(model, "C86") != NULL);
    if (!is_hygon_c86) {
        logger_warn(MODULE, "Not a Hygon C86 CPU: %s", model);
        return NULL;
    }
    
    /* ========== 策略1：直接匹配标准格式 "Hygon C86-XG" ========== */
    const char *c86_dash = strstr(model, "C86-");
    if (c86_dash) {
        const char *gen_start = c86_dash + 4;  /* 跳过 "C86-" */
        
        /* 提取代际标识（如 "3G", "4G"） */
        if (*gen_start >= '0' && *gen_start <= '9') {
            char gen_char = *gen_start;
            
            /* 检查后面是否紧跟 'G' */
            if (*(gen_start + 1) == 'G' || *(gen_start + 1) == 'g') {
                static char result[64];
                snprintf(result, sizeof(result), "Hygon C86-%cG", gen_char);
                
                logger_info(MODULE, "Matched standard format: %s", result);
                return result;
            }
        }
    }
    
    /* ========== 策略2：从 OPN 括号中提取型号数字 ========== */
    const char *opn_start = strstr(model, "OPN:");
    if (!opn_start) {
        opn_start = strstr(model, "opn:");
    }
    
    if (opn_start) {
        int opn_num = 0;
        const char *after_opn = extract_cpu_model_number(opn_start, &opn_num);
        
        if (after_opn && opn_num > 0) {
            /* 提取百位数作为代数标识 */
            int generation = (opn_num / 100) % 10;
            
            if (generation >= 1 && generation <= 9) {
                static char result[64];
                snprintf(result, sizeof(result), "Hygon C86-%dG", generation);
                
                logger_info(MODULE, "Extracted from OPN %d -> %s", opn_num, result);
                return result;
            }
        }
    }
    
    /* ========== 策略3：从整个字符串中提取型号数字（范围过滤） ========== */
    int model_num = 0;
    const char *after_num = extract_cpu_model_number(model, &model_num);
    
    if (after_num && model_num > 0) {
        /* 提取百位数作为代数标识 */
        int generation = (model_num / 100) % 10;
        
        if (generation >= 1 && generation <= 9) {
            static char result[64];
            snprintf(result, sizeof(result), "Hygon C86-%dG", generation);
            
            logger_info(MODULE, "Extracted from model number %d -> %s", model_num, result);
            return result;
        }
    }
    
    /* ========== 策略4：降级 - 无法识别 ========== */
    logger_warn(MODULE, "Could not parse CPU model identifier from: %s", model);
    return NULL;
}


/**
 * @brief 从/proc/cpuinfo 采集更详细的 CPU 信息
 */
static psp_error_t collect_detailed_cpu_info(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        logger_warn(MODULE, "Failed to open /proc/cpuinfo");
        return ERR_IO;
    }
    
    char line[1024];
    int processor_count = 0;
    char *vendor = NULL;
    char *model = NULL;
    int line_count = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        line_count++;
        
        /* 去除行尾换行符 */
        char *newline = strchr(line, '\n');
        if (newline) {
            *newline = '\0';
        }
        
        /* 查找冒号分隔符 */
        char *colon = strchr(line, ':');
        if (!colon) {
            continue;
        }
        
        /* 分割键值对 */
        *colon = '\0';
        char *key = line;
        char *value = colon + 1;
        
        /* 去除key两端空白 */
        while (*key == ' ' || *key == '\t') key++;
        char *key_end = key + strlen(key) - 1;
        while (key_end > key && (*key_end == ' ' || *key_end == '\t')) {
            *key_end = '\0';
            key_end--;
        }
        
        /* 去除value两端空白 */
        while (*value == ' ' || *value == '\t') value++;
        char *value_end = value + strlen(value) - 1;
        while (value_end > value && (*value_end == ' ' || *value_end == '\t')) {
            *value_end = '\0';
            value_end--;
        }
        
        /* 统计 processor 数量 */
        if (strcmp(key, "processor") == 0) {
            processor_count++;
        }
        /* 提取 vendor_id（只取第一个） */
        else if (strcmp(key, "vendor_id") == 0 && !vendor) {
            vendor = strdup(value);
            if (!vendor) {
                logger_error(MODULE, "Failed to allocate memory for vendor");
                fclose(fp);
                return ERR_MEMORY;
            }
            logger_debug(MODULE, "Extracted vendor: %s", vendor);
        }
        /* 提取 model name */
        else if (strcmp(key, "model name") == 0 && !model) {
            model = strdup(value);
            if (!model) {
                logger_error(MODULE, "Failed to allocate memory for model");
                fclose(fp);
                if (vendor) free(vendor);
                return ERR_MEMORY;
            }
            logger_debug(MODULE, "Extracted model: %s", model);
        }
    }
    
    fclose(fp);
    
    logger_debug(MODULE, "Parsed %d lines from /proc/cpuinfo", line_count);
    
    /* 添加到环境信息列表 */
    if (vendor) {
        plugin_env_list_add(env_list, "vendor", vendor, plugin_name);
        free(vendor);
    } else {
        logger_warn(MODULE, "No vendor information found in /proc/cpuinfo");
    }
    
    if (model) {
        const char *ext_model = extract_cpu_model(model);
        plugin_env_list_add(env_list, "model", ext_model, plugin_name);        
        free(model);
    } else {
        logger_warn(MODULE, "No model information found in /proc/cpuinfo");
    }
    
    return ERR_OK;
}

/**
 * @brief 插件初始化函数（可选）
 */
int plugin_init(void)
{
    logger_debug(MODULE, "CPU collector plugin initialized");
    return 0;
}

/**
 * @brief 采集函数（必须实现）
 * 
 * 返回插件环境信息列表，由 core 层统一管理
 */
plugin_env_list_t* collect(const char *plugin_name)
{
    if (!plugin_name) {
        logger_error(MODULE, "Plugin name is NULL");
        return NULL;
    }
    
    logger_info(MODULE, "Starting CPU collection (plugin: %s)", plugin_name);
    
    /* 创建插件环境列表 */
    plugin_env_list_t *env_list = plugin_env_list_create();
    if (!env_list) {
        logger_error(MODULE, "Failed to create environment list");
        return NULL;
    }
    
    /* 采集详细 CPU 信息 */
    psp_error_t ret = collect_detailed_cpu_info(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect detailed CPU info");
    }
    
    logger_info(MODULE, "CPU collection completed with %zu items", env_list->count);
    return env_list;
}

/**
 * @brief 插件清理函数（可选）
 */
void plugin_fini(void)
{
    logger_debug(MODULE, "CPU collector plugin finalized");
}
