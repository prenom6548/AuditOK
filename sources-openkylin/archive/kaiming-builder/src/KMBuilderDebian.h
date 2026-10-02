/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_CONTROL_H__
#define __BUILD_CONTROL_H__

#include <string>
#include <vector>
#include <map>

class KMBuilderDebian
{
public:
    std::string m_source;
    std::string m_maintainer;
    std::string m_homepage;
    std::string m_description;
    std::string m_license;

    std::vector<std::map<std::string, std::string>> m_or_depends;
    std::map<std::string, std::string> m_depends;

private:
    std::vector<std::string> m_control;

public:
    KMBuilderDebian();
    virtual ~KMBuilderDebian();

    /**
     * @brief: 解析control
     */
    void parseControl(const std::string &path);

    /**
     * @brief: 找到以某个字符串为前缀的字符串
     */
    std::string getStrFromKey(const std::vector<std::string> &data, const std::string &key);

    /**
     * @brief: 获取build-depends和depends
     */
    void getDepends();
    void getDepends(const std::string &dependKind);

    /**
     * @brief: 解析copyright
     */

    void parseCopyRight(const std::string &path);

    /**
     * @brief: 解析单个depend
     * @param: [in] depend
     * @param: [in] depend是否携带|字符
     */
    std::map<std::string, std::string> parseDepend(const std::string &line, bool isOr = false);


};

#endif