#include "utils/string_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>

#define MODULE "string_utils"

psp_error_t string_utils_strncpy(char *dest, const char *src, size_t size)
{
    if (!dest || !src || size == 0) {
        return ERR_INVALID_PARAM;
    }
    
    size_t src_len = strlen(src);
    if (src_len >= size) {
        // 源字符串太长，无法安全复制（包括终止符）
        return ERR_INVALID_PARAM;
    }
    
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
    
    return ERR_OK;
}

char* string_utils_concat(const char *str1, const char *str2)
{
    if (!str1 || !str2) {
        return NULL;
    }
    
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t total_len = len1 + len2 + 1;
    
    char *result = (char*)malloc(total_len);
    if (!result) {
        logger_error(MODULE, "Failed to allocate memory for concatenation");
        return NULL;
    }
    
    memcpy(result, str1, len1);
    memcpy(result + len1, str2, len2 + 1); /* 包括终止符 */
    
    return result;
}

psp_error_t string_utils_split(const char *str, const char *delimiter,
                          char ***tokens, size_t *count)
{
    if (!str || !delimiter || !tokens || !count) {
        return ERR_INVALID_PARAM;
    }
    
    /* 临时分配，后续可能需要优化 */
    char *str_copy = string_utils_strdup(str);
    if (!str_copy) {
        return ERR_MEMORY;
    }
    
    /* 第一次遍历计算令牌数量 */
    size_t token_count = 0;
    char *saveptr = NULL;
    char *token = strtok_r(str_copy, delimiter, &saveptr);
    
    while (token) {
        token_count++;
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    
    free(str_copy);
    
    if (token_count == 0) {
        *tokens = NULL;
        *count = 0;
        return ERR_OK;
    }
    
    /* 分配令牌数组 */
    char **token_array = (char**)calloc(token_count + 1, sizeof(char*));
    if (!token_array) {
        logger_error(MODULE, "Failed to allocate memory for tokens");
        return ERR_MEMORY;
    }
    
    /* 第二次遍历复制令牌 */
    str_copy = string_utils_strdup(str);
    if (!str_copy) {
        free(token_array);
        return ERR_MEMORY;
    }
    
    token_count = 0;
    saveptr = NULL;
    token = strtok_r(str_copy, delimiter, &saveptr);
    
    while (token) {
        token_array[token_count++] = string_utils_strdup(token);
        if (!token_array[token_count - 1]) {
            /* 清理已分配的令牌 */
            for (size_t i = 0; i < token_count; i++) {
                free(token_array[i]);
            }
            free(token_array);
            free(str_copy);
            return ERR_MEMORY;
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    
    free(str_copy);
    
    *tokens = token_array;
    *count = token_count;

    return ERR_OK;
}

char* string_utils_trim(char *str)
{
    if (!str) {
        return NULL;
    }
    
    /* 去除前导空白 */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    /* 如果全是空白 */
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    /* 去除尾部空白 */
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    /* 移动字符串到开头（如果需要） */
    if (start != str) {
        memmove(str, start, end - start + 2); /* 包括终止符 */
    }
    
    return str;
}

bool string_utils_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix) {
        return false;
    }
    
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);
    
    if (prefix_len > str_len) {
        return false;
    }
    
    return strncmp(str, prefix, prefix_len) == 0;
}

bool string_utils_contains(const char *str, const char *substr)
{
    if (!str || !substr) {
        return false;
    }
    
    return strstr(str, substr) != NULL;
}

psp_error_t string_utils_atoi(const char *str, int *value)
{
    if (!str || !value) {
        return ERR_INVALID_PARAM;
    }
    
    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);
    
    /* 检查转换是否成功 */
    if (endptr == str || *endptr != '\0') {
        logger_debug(MODULE, "Invalid integer string: %s", str);
        return ERR_PARSE;
    }
    
    /* 检查溢出 */
    if (val < INT_MIN || val > INT_MAX) {
        logger_debug(MODULE, "Integer overflow: %ld", val);
        return ERR_GENERIC;
    }
    
    *value = (int)val;
    return ERR_OK;
}

int string_utils_snprintf(char *buffer, size_t size, const char *format, ...)
{
    if (!buffer || !format || size == 0) {
        return -1;
    }
    
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(buffer, size, format, args);
    va_end(args);
    
    return ret;
}

char* string_utils_strdup(const char *str)
{
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char *dup = (char*)malloc(len);
    if (!dup) {
        logger_error(MODULE, "Failed to duplicate string: %s", str);
        return NULL;
    }
    
    memcpy(dup, str, len);
    return dup;
}

void string_utils_free_array(char **array, size_t count)
{
    if (!array) return;
    
    for (size_t i = 0; i < count; i++) {
        if (array[i]) {
            free(array[i]);
        }
    }
    free(array);
}
