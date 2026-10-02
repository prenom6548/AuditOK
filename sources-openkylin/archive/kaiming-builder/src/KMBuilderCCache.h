/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_CCACHE_H__
#define __BUILDER_CCACHE_H__

#include <string>

using namespace std;

class KMBuilderCCache
{
public:
    KMBuilderCCache() = default;
    ~KMBuilderCCache() = default;

    /**
     * @brief: 设置ccache目录
     * @param: 是否启用
     * @param: ccache缓存目录
     */
    static bool setCcache(const bool enable, const string &ccacheDir);
};

#endif /* __BUILDER_CCACHE_H__ */