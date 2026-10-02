/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildinsUtils.h"

#include <string.h>
#include <algorithm>
#include <cstdlib>
#include <regex>

#include "config.h"
#include "common/kmtranslation.h"

/**
 * @brief : 校验appid是否合法
 * @param : [in] appName, appid
 * @param : [out] errMsg, 错误信息
 */ 
bool KMBuildinsUtils::isValidAppName(const std::string &appName, std::string &errMsg)
{
    if (appName.length() > 255)
    {
        errMsg = _("Id can't be longer than 255 characters");
        return false;
    }

    // std::regex regexStr("(\\w+\\.)+(\\w+\\.)+([\\w-]+)$");
    std::regex regexStr("([\\w-]+\\.)+([\\w-]+\\.)+([\\w-]+)$");
    std::smatch matchResult;
    if (std::regex_match(appName, matchResult, regexStr))
    {
        // std::regex regexDigit("^\\d.*");
        // std::smatch matchDigit;
        // for (auto element : matchResult)
        // {
        //     std::string str = element.str();
        //     if (std::regex_match(str, matchDigit, regexDigit))
        //     {
        //         errMsg = _("Id is wrong! Elements may not begin with a digit.");
        //         return false;
        //     }
        // }
    }
    else
    {
        errMsg = _("Id is wrong! Notes:\n"
            "    a) Ids are composed of 3 or more elements separated by a period('.') character.\n"
            "    b) Each element must only contain the ASCII characters '[A-Z][a-z][0-9]_-'.\n"
        //    "    c) All elements must contain at least one character.\n"
        //    "       Elements may not begin with a digit.\n"
        //    "       Additionally '-' is only allowed in the last element."
        );
        return false;
    }

    return true;
}

/**
 * @brief : 是否合法的命名字符
 * @param : [in] c, 待判断字符
 * @param : [in] allowDash，是否运行横线连接符'-'
 */
bool KMBuildinsUtils::isValidInitialNameCharacter(char c, bool allowDash)
{
    return (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z') || (c == '_')
        || (allowDash && c == '-');
}

bool KMBuildinsUtils::isValidNameCharacter(char c, bool allowDash)
{
    return KMBuildinsUtils::isValidInitialNameCharacter(c, allowDash) || (c >= '0' && c <= '9');
}

