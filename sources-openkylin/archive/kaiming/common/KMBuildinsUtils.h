/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDINSUTILS_H__
#define __KMBUILDINSUTILS_H__

#include <string>
#include <vector>

/**
 * @brief : 主要用于封装一些子命令通用的业务相关的操作，但又不像KMUtil那么通用
 */
class KMBuildinsUtils
{
    KMBuildinsUtils() = delete;
    ~KMBuildinsUtils() = delete;

public:
    /**
     * @brief : 校验appid是否合法
     * @param : [in] appName, appid
     * @param : [out] errMsg, 错误信息
     */ 
    static bool isValidAppName(const std::string &appName, std::string &errMsg);

    /**
     * @brief : 是否合法的命名字符
     * @param : [in] c, 待判断字符
     * @param : [in] allowDash，是否运行横线连接符'-'
     */
    static bool isValidInitialNameCharacter(char c, bool allowDash);
    static bool isValidNameCharacter(char c, bool allowDash);
};

#endif // !__KMBUILDINSUTILS_H__
