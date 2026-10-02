/**
 * @file capability_parser.h
 * @brief 能力库解析模块内部头文件
 */

#ifndef CORE_CAPABILITY_PARSER_H
#define CORE_CAPABILITY_PARSER_H

#include "models/capability.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 从 JSON 字符串解析能力列表
 * 
 * @param json_data JSON 格式的原始数据
 * @return 成功返回能力列表，失败返回 NULL
 */
capability_list_t* capability_parser(const char *json_data);

/**
 * @brief 获取解析错误信息
 * 
 * @return 错误信息字符串 (静态分配)
 */
const char* capability_parser_get_error(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_CAPABILITY_PARSER_H */
