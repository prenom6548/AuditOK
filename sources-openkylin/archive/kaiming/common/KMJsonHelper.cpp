/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMJsonHelper.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "KMStringUtils.h"

namespace fs = std::filesystem;

nlohmann::json KMJsonHelper::loadFile(const std::string &jsonFile)
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

nlohmann::json KMJsonHelper::toJson(const std::vector<std::string> &list)
{
    nlohmann::json array = nlohmann::json::array();
    for (auto &item : list)
    {
        array.push_back(item);
    }

    return array;
}

nlohmann::json KMJsonHelper::toJson(const std::map<std::string, std::string> &map)
{
    nlohmann::json object = nlohmann::json::object();
    for (auto &pair : map)
    {
        object[pair.first] = pair.second;
    }

    return object;
}

std::string KMJsonHelper::getStringValue(const std::string &key, const std::string &defaultValue)
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

uint64_t KMJsonHelper::getUInt64Value(const std::string &key, uint64_t defaultValue)
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

bool KMJsonHelper::getBoolValue(const std::string &key, bool defaultValue)
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

int64_t KMJsonHelper::getInt64Value(const std::string &key, int64_t defaultValue)
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

int KMJsonHelper::getIntValue(const std::string &key, int defaultValue)
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

std::vector<std::string> KMJsonHelper::getStringListValue(const std::string &key)
{
    std::vector<std::string> defaultValue;

    nlohmann::json value = m_node[key];
    if (value.is_null())
    {
        return defaultValue;
    }

    if (value.is_array())
    {
        for (auto &item : value)
        {
            if (item.is_string())
            {
                defaultValue.push_back(item.get<std::string>());
            }
        }
    }

    return defaultValue;
}

nlohmann::json KMJsonHelper::getObjectValue(const std::string &key)
{
    nlohmann::json node = m_node[key];
    if (node.is_null())
    {
        return {};
    }

    return node;
}

/**
 * @brief : 获取对象键值对
 * @param : [in] key, json对象键
 * @return: std::map<std::string, std::string>, 键值对
 */
std::map<std::string, std::string> KMJsonHelper::getObjectKeyValue(const std::string &key)
{
    nlohmann::json node = m_node[key];
    if (node.is_null())
    {
        return {};
    }

    std::map<std::string, std::string> result;
    for (auto &[key, value] : node.items())
    {
        result[key] = value;
    }

    return result;
}

std::map<std::string, std::string> KMJsonHelper::getStringMapValue(const std::string &key)
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

std::map<std::string, std::string> KMJsonHelper::fromPrefix(const std::string &prefix)
{
    std::map<std::string, std::string> defaultValue;

    if (m_node.is_object())
    {
        for (auto it = m_node.begin(); it != m_node.end(); it++)
        {
            std::string key = it.key();
            if (KMStringUtils::startsWith(key, prefix))
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

void KMJsonHelper::setBoolValue(const std::string &key, bool value, bool enable)
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

void KMJsonHelper::setStringListValue(const std::string &key, const std::vector<std::string> &value)
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

void KMJsonHelper::setStringMapValue(const std::string &key, const std::map<std::string, std::string> &value)
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

void KMJsonHelper::setStringValue(const std::string &key, const std::string &value)
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

void KMJsonHelper::setUInt64Value(const std::string &key, uint64_t value)
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

void KMJsonHelper::add(const std::map<std::string, std::string> &map)
{
    for (auto &pair : map)
    {
        m_node[pair.first] = pair.second;
    }
}

void KMJsonHelper::toJson(const std::string &jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code e;
        if (!fs::create_directories(baseDir, e))
        {
            throw KMException(e.message());
        }
    }

    std::ofstream o(jsonFile);
    o << std::setw(4) << m_node << std::endl;
}
