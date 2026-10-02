/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include <stdbool.h>

// 获取软件包标签
typedef struct {
    const char *pattern;
    const char *value;
    int order;
} VersionPattern;

static const VersionPattern version_patterns[] = {
    {"v11", "shadow", 1},
    {"ok2", "shadow", 2},
    {"v10sp1", "v10sp1", 3},
    {"v10", "v10", 4},
    {"shadow", "shadow", 5},
    {NULL, NULL, 0}
};

 // 枚举类型定义
typedef enum
{
    CHECK_RESULT_UNDEF,
    CHECK_RESULT_V10SP1,
    CHECK_RESULT_V11
} CheckResult;

CheckResult _kdk_dmvc_check_os_version(const char *deb_path);

/**
 * @brief 检测kare环境
 */
int check_kare_environment(const char *expected_value, char *output_value, size_t output_size);

/**
 * @brief 检查字符串是否包含通配符
 */
bool contains_wildcard(const char *pattern);

/**
 * @brief 验签
 */
int kdk_verify_signature(char *uri, char *dir, char *typebm, int callflag);

/**
 * 通配符匹配函数
 * @param pattern 模式字符串（可以包含*通配符）
 * @param str 要匹配的字符串
 * @return 匹配成功返回true，失败返回false
 */
bool wildcard_match(const char *pattern, const char *str);
