/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMJsonHelper_H__
#define __KMJsonHelper_H__

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "KMException.h"

/**
 * @brief : 辅助json节点的读写
 */
class KMJsonHelper
{
public:
    /**
     * @brief : 加载json文件
     * @param : [in] jsonFile, json文件
     * @return: nlohmann::json, 解析后的json对象
     * @throw : KMException
     */
    static nlohmann::json loadFile(const std::string &jsonFile);

    static nlohmann::json toJson(const std::vector<std::string> &list);

    static nlohmann::json toJson(const std::map<std::string, std::string> &map);

public:
    KMJsonHelper(nlohmann::json &node)
        : m_node(node)
    {
    }

    ~KMJsonHelper() = default;

    std::string getStringValue(const std::string &key, const std::string &defaultValue = "");

    std::vector<std::string> getStringListValue(const std::string &key);

    std::map<std::string, std::string> getStringMapValue(const std::string &key);

    /**
     * @brief : 获取对象键值对
     * @param : [in] key, json对象键
     * @return: std::map<std::string, std::string>, 键值对
     */
    std::map<std::string, std::string> getObjectKeyValue(const std::string &key);

    nlohmann::json getObjectValue(const std::string &key);

    uint64_t getUInt64Value(const std::string &key, uint64_t defaultValue = 0L);

    int64_t getInt64Value(const std::string &key, int64_t defaultValue = 0L);

    int getIntValue(const std::string &key, int defaultValue = 0);

    bool getBoolValue(const std::string &key, bool defaultValue = false);

    void setBoolValue(const std::string &key, bool value, bool enable = true);

    void setStringListValue(const std::string &key, const std::vector<std::string> &value);

    void setStringMapValue(const std::string &key, const std::map<std::string, std::string> &value);

    void setStringValue(const std::string &key, const std::string &value);

    void setUInt64Value(const std::string &key, uint64_t value);

    void add(const std::map<std::string, std::string> &map);

    std::map<std::string, std::string> fromPrefix(const std::string &prefix);

    void toJson(const std::string &jsonFile);

private:
    nlohmann::json &m_node;
};

#endif // !__KMJsonHelper_H__
