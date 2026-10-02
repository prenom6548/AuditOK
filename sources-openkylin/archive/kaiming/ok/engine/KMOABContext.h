/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABCONTEXT_H__
#define __KMOABCONTEXT_H__

#include <string>

#include "common/KMSingleton.h"

/**
 * @brief : 上下文信息封装类
 */
class KMOABContext: public KMSingleton<KMOABContext>
{
public:
    KMOABContext(token) {}
    ~KMOABContext() = default;

    void setAppName(const std::string &appName) { m_appName = appName; }

    const std::string& appName() const { return m_appName; } 

private:
    // 当前应用名称
    std::string m_appName;
};

#endif // !__KMOABCONTEXT_H__
