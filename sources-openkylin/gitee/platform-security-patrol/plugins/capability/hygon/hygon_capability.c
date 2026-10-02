/**
 * @file hygon_capability.c
 * @brief 海光平台安全能力库插件
 * 
 * 提供海光 CPU 平台的安全能力定义（CSV1、CSV2、CSV3等）
 * 从系统安装路径加载能力定义JSON文件
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "models/environment.h"
#include "logger.h"
#include "plugin_env_utils.h"
#include "cJSON.h"

#define MODULE "hygon_capability"

const char* plugin_type = "capability";

/* 能力定义文件安装路径 */
/* 实际值由 CMake 根据 CMAKE_INSTALL_PREFIX 注入，兜底值仅用于未经 CMake 的独立编译 */
#ifndef CAPABILITY_DATA_DIR
#define CAPABILITY_DATA_DIR "/usr/local/share/plat-sec-patrol/capabilities"
#endif

#define HYON_CAPABILITY_FILE CAPABILITY_DATA_DIR "/hygon_capabilities.json"

psp_error_t plugin_read_file(const char *file_path, char **content, size_t *size)
{
    if (!file_path || !content) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        logger_error(MODULE, "Failed to open file: %s (error: %d)", file_path, errno);
        return ERR_IO;
    }
    
    /* 尝试获取文件大小（对普通文件有效） */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buffer = NULL;
    size_t bytes_read = 0;
    
    if (file_size > 0) {
        /* 普通文件：预分配缓冲区 */
        buffer = (char*)malloc(file_size + 1);
        if (!buffer) {
            logger_error(MODULE, "Failed to allocate memory for file content");
            fclose(fp);
            return ERR_MEMORY;
        }
        
        bytes_read = fread(buffer, 1, file_size, fp);
        buffer[bytes_read] = '\0';
        
        logger_debug(MODULE, "Read %zu bytes from regular file: %s", bytes_read, file_path);
    }
    
    fclose(fp);
    
    *content = buffer;
    if (size) {
        *size = bytes_read;
    }
    
    return ERR_OK;
}

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
static const char* extract_cpu_model_identifier(const environment_t *env)
{
    if (!env || !env->cpu.model) {
        logger_warn(MODULE, "Environment or CPU model is NULL");
        return NULL;
    }
    
    const char *model = env->cpu.model;
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
                
                logger_info(MODULE, "Matched CPU model: %s", result);
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
 * @brief 从完整JSON中过滤出指定CPU型号的能力数据
 * 
 * @param full_json 完整的JSON字符串
 * @param cpu_model CPU型号标识（如 "Hygon C86-3G"）
 * @return 过滤后的JSON字符串，需调用者释放；失败返回NULL
 */
static char* filter_json_by_cpu_model(const char *full_json, const char *cpu_model)
{
    if (!full_json || !cpu_model) {
        logger_error(MODULE, "Invalid parameters for JSON filtering");
        return NULL;
    }
    
    logger_debug(MODULE, "Filtering JSON for CPU model: %s", cpu_model);
    
    /* 解析完整JSON */
    cJSON *root = cJSON_Parse(full_json);
    if (!root) {
        logger_error(MODULE, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
        return NULL;
    }
    
    if (!cJSON_IsObject(root)) {
        logger_error(MODULE, "JSON root is not an object");
        cJSON_Delete(root);
        return NULL;
    }
    
    /* 查找匹配的CPU型号 */
    cJSON *cpu_model_obj = cJSON_GetObjectItemCaseSensitive(root, cpu_model);
    if (!cpu_model_obj || !cJSON_IsObject(cpu_model_obj)) {
        logger_warn(MODULE, "CPU model '%s' not found in capabilities JSON", cpu_model);
        cJSON_Delete(root);
        return NULL;
    }
    
    /* 创建新的JSON对象，只包含匹配的CPU型号 */
    cJSON *filtered_root = cJSON_CreateObject();
    if (!filtered_root) {
        logger_error(MODULE, "Failed to create filtered JSON object");
        cJSON_Delete(root);
        return NULL;
    }

    /* 保留能力库级元数据（外部工具依赖声明），确保过滤后核心层仍可解析 */
    cJSON *external_tools = cJSON_GetObjectItemCaseSensitive(root, "external_tools");
    if (external_tools) {
        cJSON *tools_copy = cJSON_Duplicate(external_tools, 1);
        if (tools_copy) {
            cJSON_AddItemToObject(filtered_root, "external_tools", tools_copy);
        } else {
            logger_warn(MODULE, "Failed to duplicate external_tools section");
        }
    }

    /* 将匹配的CPU型号对象添加到新JSON中 */
    cJSON *copied_obj = cJSON_Duplicate(cpu_model_obj, 1); /* 深度复制 */
    if (!copied_obj) {
        logger_error(MODULE, "Failed to duplicate CPU model object");
        cJSON_Delete(filtered_root);
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON_AddItemToObject(filtered_root, cpu_model, copied_obj);
    
    /* 转换为JSON字符串 */
    char *filtered_json = cJSON_PrintUnformatted(filtered_root);
    
    /* 清理 */
    cJSON_Delete(filtered_root);
    cJSON_Delete(root);
    
    if (!filtered_json) {
        logger_error(MODULE, "Failed to print filtered JSON");
        return NULL;
    }
    
    // logger_info(MODULE, "Filtered JSON for '%s': %zu bytes", cpu_model, strlen(filtered_json));
    return filtered_json;
}

/**
 * @brief 从环境信息判断是否应该加载此能力库
 */
static bool should_load_capabilities(const environment_t *env)
{
    if (!env || !env->cpu.vendor) {
        return false;
    }
    
    /* 只在海光平台加载 */
    if (strcmp(env->cpu.vendor, "HygonGenuine") == 0) {
        logger_info(MODULE, "Detected Hygon platform, loading capability library");
        return true;
    }
    
    return false;
}

/**
 * @brief 读取 JSON 文件内容并返回
 */
static char* read_capability_json(void)
{
    // logger_debug(MODULE, "Reading capability file: %s", HYON_CAPABILITY_FILE);
    
    /* 读取 JSON 文件内容 */
    char *content = NULL;
    psp_error_t ret = plugin_read_file(HYON_CAPABILITY_FILE, &content, NULL);
    
    if (ret != ERR_OK || !content) {
        logger_error(MODULE, "Failed to read capability file: %s", HYON_CAPABILITY_FILE);
        return NULL;
    }
    
    // logger_info(MODULE, "Read capability file: %s (%zu bytes)", HYON_CAPABILITY_FILE, strlen(content));
    return content;
}

/**
 * @brief 能力加载函数（插件必须实现）
 * 
 * 返回 JSON 格式的字符串，由 core 层解析
 * 插件层负责根据CPU型号过滤JSON数据
 */
const char* load_capabilities_json(const environment_t *env)
{
    logger_info(MODULE, "Loading Hygon capabilities");
    
    /* 检查是否应该加载 */
    if (!should_load_capabilities(env)) {
        return NULL;
    }
    
    /* 读取完整JSON文件 */
    char *full_json = read_capability_json();
    if (!full_json) {
        logger_error(MODULE, "Failed to read capability JSON file");
        return NULL;
    }
    
    /* 提取CPU型号标识 */
    const char *cpu_model = extract_cpu_model_identifier(env);
    if (!cpu_model) {
        logger_warn(MODULE, "Could not determine CPU model, returning full JSON");
        return full_json;  /* 降级：返回完整JSON */
    }
    
    /* 过滤JSON，只保留匹配的CPU型号 */
    char *filtered_json = filter_json_by_cpu_model(full_json, cpu_model);
    free(full_json);  /* 释放原始JSON */
    
    if (!filtered_json) {
        logger_error(MODULE, "Failed to filter JSON for CPU model: %s", cpu_model);
        return NULL;
    }
    
    logger_info(MODULE, "Loaded and filtered Hygon capabilities for %s (%zu bytes)", 
               cpu_model, strlen(filtered_json));
    return filtered_json;  /* 调用者负责释放 */
}

/**
 * @brief 插件清理函数（可选）
 */
void plugin_fini(void)
{
    logger_debug(MODULE, "Hygon capability plugin finalized");
}
