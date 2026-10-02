/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMNlohmannJsonHelper.h"
#include <fstream>
#include <filesystem>
#include "../KMBuilderUtils.h"

nlohmann::json KMNlohmannJsonHelper::loadFile(const std::string& jsonFile)
{
    nlohmann::json json;
    std::ifstream input(jsonFile);

    if (input.is_open())
    {
        json = nlohmann::json::parse(input); // 需要直接赋值，否则会报错
        input.close();
    }
    else
    {
        throw KMException("Failed to load file : " + jsonFile);
    }

    return json;
}

nlohmann::json KMNlohmannJsonHelper::toJson(const std::vector<std::string>& list)
{
    nlohmann::json array = nlohmann::json::array();
    for (auto& item : list)
    {
        array.push_back(item);
    }

    return array;
}

nlohmann::json KMNlohmannJsonHelper::toJson(const std::map<std::string, std::string>& map)
{
    nlohmann::json object = nlohmann::json::object();
    for (auto& pair : map)
    {
        object[pair.first] = pair.second;
    }

    return object;
}

bool KMNlohmannJsonHelper::getBoolValue(const std::string &key, bool defaultValue)
{
    nlohmann::json value = m_node[key];
    
    if (value.is_null())
    {
        return defaultValue;
    }
    
    if (value.is_boolean())
    {
        return value.get<bool>();
    }
   
    return defaultValue;
}

std::string KMNlohmannJsonHelper::getStringValue(const std::string &key, const std::string &defaultValue)
{
    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_primitive())
    {
        return value.get<std::string>();
    }

    return defaultValue;
}

uint64_t KMNlohmannJsonHelper::getUInt64Value(const std::string &key, uint64_t defaultValue)
{
    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_number_unsigned())
    {
        return value.get<uint64_t>();
    }

    return defaultValue;
}

int64_t KMNlohmannJsonHelper::getInt64Value(const std::string &key, int64_t defaultValue)
{
    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_number_integer())
    {
        return value.get<int64_t>();
    }

    return defaultValue;
}

int KMNlohmannJsonHelper::getIntValue(const std::string &key, int defaultValue)
{
    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_number_integer())
    {
        return value.get<int>();
    }

    return defaultValue;
}

std::vector<std::string> KMNlohmannJsonHelper::getStringListValue(const std::string &key)
{
    std::vector<std::string> defaultValue;

    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_array())
    {
        for (auto& item : value)
        {
            if (item.is_string())
            {
                defaultValue.push_back(item.get<std::string>());
            }
        }
    }

    return defaultValue;
}

std::map<std::string, std::string> KMNlohmannJsonHelper::getStringMapValue(const std::string &key)
{
    std::map<std::string, std::string> defaultValue;

    nlohmann::json node = m_node[key];
    if (node.is_null())
    {
        return defaultValue;
    }

    if (node.is_object())
    {
        for (auto it = node.begin(); it != node.end(); it++)
        {
            std::string key = it.key();
            nlohmann::json value = it.value();
            if (value.is_string())
            {
                defaultValue[key] = value.get<std::string>();
            }
        }
    }

    return defaultValue;
}

std::map<std::string, std::string> KMNlohmannJsonHelper::fromPrefix(const std::string& prefix)
{
    std::map<std::string, std::string> defaultValue;

    if (m_node.is_object())
    {
        for (auto it = m_node.begin(); it != m_node.end(); it++)
        {
            std::string key = it.key();
            if (KMBuilderUtils::startsWith(key, prefix))
            {
                nlohmann::json value = it.value();
                if (value.is_string())
                {
                    defaultValue[key] = value.get<std::string>();
                }
            }
        }
    }

    return defaultValue;
}

void KMNlohmannJsonHelper::setStringListValue(const std::string &key, const std::vector<std::string>& value)
{
    if (m_node[key].is_null())
    {
        m_node.erase(key);
    }

    if (!value.empty())
    {
        m_node[key] = toJson(value);
    }
    else
    {
        m_node.erase(key);
    }
}

void KMNlohmannJsonHelper::setStringMapValue(const std::string &key, const std::map<std::string, std::string>& value)
{
    if (m_node[key].is_null())
    {
        m_node.erase(key);
    }

    if (!value.empty())
    {
        m_node[key] = toJson(value);
    }
    else
    {
        m_node.erase(key);
    }
}

void KMNlohmannJsonHelper::setBoolValue(const std::string& key, bool value, bool enable) 
{
    if (m_node[key].is_null())
    {
        m_node.erase(key);
    }
    
    if (!enable)
    {
        return;
    }
    
    m_node[key] = value;
}

void KMNlohmannJsonHelper::setStringValue(const std::string &key, const std::string &value)
{
    if (m_node[key].is_null())
    {
        m_node.erase(key);
    }

    if (!value.empty())
    {
        m_node[key] = value;
    }
    else
    {
        m_node.erase(key);
    }
}

void KMNlohmannJsonHelper::setUInt64Value(const std::string &key, uint64_t value)
{
    if (m_node[key].is_null())
    {
        m_node.erase(key);
    }

    if (value != 0)
    {
        m_node[key] = value;
    }
    else
    {
        m_node.erase(key);
    }
}

void KMNlohmannJsonHelper::add(const std::map<std::string, std::string>& map)
{
    for (auto& pair : map)
    {
        m_node[pair.first] = pair.second;
    }
}

