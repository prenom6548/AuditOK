/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMYAMLNODEHELPER_H__
#define __KMYAMLNODEHELPER_H__

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
// #include <json/json.h>
#include <nlohmann/json.hpp>

/**
 * @brief : 辅助YAML::Node进行标准类型值的获取
 */ 
class KMYAMLNodeHelper
{
public:
    // static YAML::Node toYAMLNode(const Json::Value &json);
    static YAML::Node toYAMLNode(const nlohmann::json &json);
    static YAML::Node loadFromYAML(const std::string& yamlFile);
    static YAML::Node loadFromJson(const std::string& jsonFile);
public:
    KMYAMLNodeHelper(YAML::Node& node)
        : m_node(node)
    {}

    ~KMYAMLNodeHelper() = default;

    std::string getStringValue(const std::string &key, const std::string &defaultValue = "");
    long getLongValue(const std::string &key, long defaultValue = 0L);
    int getIntValue(const std::string &key, int defaultValue = 0);
    bool getBoolValue(const std::string &key, bool defaultValue = false);
    std::vector<std::string> getStringListValue(const std::string &key);
    std::map<std::string, std::string> getStringMapValue(const std::string &key);
    std::map<std::string, std::string> fromPrefix(const std::string &pre);

private:
    YAML::Node &m_node;
};

#endif // !__KMYAMLNODEHELPER_H__
