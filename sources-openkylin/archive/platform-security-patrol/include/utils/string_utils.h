#ifndef UTILS_STRING_UTILS_H
#define UTILS_STRING_UTILS_H

#include "plat_sec_patrol.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字符串复制（安全版本）
 * 
 * @param dest 目标缓冲区
 * @param src 源字符串
 * @param size 目标缓冲区大小
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t string_utils_strncpy(char *dest, const char *src, size_t size);

/**
 * @brief 字符串连接
 * 
 * @param str1 字符串 1
 * @param str2 字符串 2
 * @return 连接后的新字符串（需调用者释放），失败返回 NULL
 */
char* string_utils_concat(const char *str1, const char *str2);

/**
 * @brief 字符串分割
 * 
 * @param str 要分割的字符串
 * @param delimiter 分隔符
 * @param[out] tokens 令牌数组（输出，需调用者释放）
 * @param[out] count 令牌数量（输出）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t string_utils_split(const char *str, const char *delimiter,
                          char ***tokens, size_t *count);

/**
 * @brief 去除字符串两端空白
 * 
 * @param str 输入字符串（可修改）
 * @return 处理后的字符串指针
 */
char* string_utils_trim(char *str);

/**
 * @brief 检查字符串是否以指定前缀开头
 * 
 * @param str 字符串
 * @param prefix 前缀
 * @return 是返回 true，否则返回 false
 */
bool string_utils_starts_with(const char *str, const char *prefix);

/**
 * @brief 检查字符串是否包含子串
 * 
 * @param str 字符串
 * @param substr 子串
 * @return 包含返回 true，否则返回 false
 */
bool string_utils_contains(const char *str, const char *substr);

/**
 * @brief 字符串转整数
 * 
 * @param str 字符串
 * @param[out] value 转换结果
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t string_utils_atoi(const char *str, int *value);

/**
 * @brief 格式化字符串（安全版本）
 * 
 * @param buffer 目标缓冲区
 * @param size 缓冲区大小
 * @param format 格式字符串
 * @param ... 可变参数
 * @return 写入的字符数（不包括终止符），失败返回 -1
 */
int string_utils_snprintf(char *buffer, size_t size, const char *format, ...);

/**
 * @brief 复制字符串（动态分配）
 * 
 * @param str 源字符串
 * @return 新字符串（需调用者释放），失败返回 NULL
 */
char* string_utils_strdup(const char *str);

/**
 * @brief 释放字符串数组
 * 
 * @param array 字符串数组
 * @param count 数组元素数量
 */
void string_utils_free_array(char **array, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_STRING_UTILS_H */
