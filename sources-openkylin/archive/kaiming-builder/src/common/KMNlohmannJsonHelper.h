/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMNLOHMANNJSONHELPER_H__
#define __KMNLOHMANNJSONHELPER_H__

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "KMException.h"

/**
 * @brief : 辅助json节点的读写
 */ 
class KMNlohmannJsonHelper
{
public:
    /**
     * @brief : 加载json文件
     * @param : [in] jsonFile, json文件
     * @return: nlohmann::json, 解析后的json对象
     * @throw : KMException
     */ 
    static nlohmann::json loadFile(const std::string& jsonFile);

    static nlohmann::json toJson(const std::vector<std::string>& list);

    static nlohmann::json toJson(const std::map<std::string, std::string>& map);

public:
    KMNlohmannJsonHelper(nlohmann::json& node)
        : m_node(node)
    {}

    ~KMNlohmannJsonHelper() = default;

    std::string getStringValue(const std::string &key, const std::string &defaultValue = "");

    std::vector<std::string> getStringListValue(const std::string &key);

    std::map<std::string, std::string> getStringMapValue(const std::string &key);

    uint64_t getUInt64Value(const std::string &key, uint64_t defaultValue = 0L);
    
    int64_t getInt64Value(const std::string &key, int64_t defaultValue = 0L);

    int getIntValue(const std::string &key, int defaultValue = 0);

    bool getBoolValue(const std::string &key, bool defaultValue = false);

    void setBoolValue(const std::string& key, bool value, bool enable = true);
    
    void setStringListValue(const std::string &key, const std::vector<std::string>& value);

    void setStringMapValue(const std::string &key, const std::map<std::string, std::string>& value);

    void setStringValue(const std::string &key , const std::string &value); 

    void setUInt64Value(const std::string &key , uint64_t value); 

    void add(const std::map<std::string, std::string>& map);

    std::map<std::string, std::string> fromPrefix(const std::string& prefix);

    bool contains(const std::string &key) { return m_node.contains(key); }

private:
    nlohmann::json& m_node;
};

#endif // !__KMNLOHMANNJSONHELPER_H__
