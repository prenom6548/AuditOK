/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMYAMLNodeHelper.h"

#include <assert.h>
#include <fstream>

#include "KMException.h"
#include "../KMBuilderUtils.h"

// YAML::Node KMYAMLNodeHelper::toYAMLNode(const Json::Value &json)
// {
//     if (json.isNull())
//     {
//         return YAML::Node();
//     }
    
//     if (json.isString() || json.isBool() || json.isNumeric())
//     {
//         return YAML::Load(json.asString());
//     }
    
//     if (json.isObject())
//     {
//         YAML::Node object;
//         for (auto it = json.begin(); it != json.end(); it++) 
//         {
//             object[it.name()] = KMYAMLNodeHelper::toYAMLNode(*it);
//         }

//         return object;
//     }

//     if (json.isArray())
//     {
//         YAML::Node array;
//         for (const auto &item : json)
//         {
//             array.push_back(KMYAMLNodeHelper::toYAMLNode(item));
//         }

//         return array;
//     }

//     assert(false);
//     return YAML::Node();
// }

YAML::Node KMYAMLNodeHelper::toYAMLNode(const nlohmann::json &json)
{
    if (json.is_null())
    {
        return YAML::Node();
    }

    if (json.is_string() || json.is_boolean() || json.is_number())
    {
        return YAML::Load(json.dump());
    }

    if (json.is_object())
    {
        YAML::Node object;
        for (auto it = json.begin(); it != json.end(); it++)
        {
            object[it.key()] = KMYAMLNodeHelper::toYAMLNode(it.value());
        }
        return object;
    }

    if (json.is_array())
    {
        YAML::Node array;
        for (const auto &item : json)
        {
            array.push_back(KMYAMLNodeHelper::toYAMLNode(item));
        }
        return array;
    }

    assert(false);
    return YAML::Node();
}

YAML::Node KMYAMLNodeHelper::loadFromYAML(const std::string& yamlFile)
{
    try
    {
        YAML::Node rootNode = YAML::LoadFile(yamlFile);
        return rootNode;
    }
    catch (const YAML::Exception& e)
    {
         throw KMException(e.what());
    }

    assert(false);
    return YAML::Node();
}

// YAML::Node KMYAMLNodeHelper::loadFromJson(const std::string& jsonFile)
// {
//     std::ifstream infile;
//     infile.open(jsonFile);
//     if (!infile.is_open())
//     {
//         throw KMException("Failed to open json file : " + jsonFile);
//     }

//     Json::CharReaderBuilder reader;
//     std::string error;
//     Json::Value node;
//     if (!Json::parseFromStream(reader, infile, &node, &error))
//     {
//         throw KMException(error);
//     }

//     return KMYAMLNodeHelper::toYAMLNode(node);
// }

YAML::Node KMYAMLNodeHelper::loadFromJson(const std::string& jsonFile)
{
    std::ifstream infile;
    infile.open(jsonFile);
    if (!infile.is_open())
    {
        throw KMException("Failed to open json file : " + jsonFile);
    }

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

    return KMYAMLNodeHelper::toYAMLNode(json);
}

std::string KMYAMLNodeHelper::getStringValue(const std::string &key, const std::string &defaultValue)
{
    YAML::Node value = m_node[key];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<std::string>();
    }

    return defaultValue;
}

long KMYAMLNodeHelper::getLongValue(const std::string &key, long defaultValue)
{
    YAML::Node value = m_node[key];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<long>();
    }

    return defaultValue;
}

int KMYAMLNodeHelper::getIntValue(const std::string &key, int defaultValue)
{
    YAML::Node value = m_node[key];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<int>();
    }

    return defaultValue;
}

bool KMYAMLNodeHelper::getBoolValue(const std::string &key, bool defaultValue)
{
    YAML::Node value = m_node[key];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsScalar())
    {
        return value.as<bool>();
    }

    return defaultValue;
}

std::vector<std::string> KMYAMLNodeHelper::getStringListValue(const std::string &key)
{
    std::vector<std::string> defaultValue;

    YAML::Node value = m_node[key];
    if (!value.IsDefined())
    {
        return defaultValue;
    }

    if (value.IsSequence())
    {
        for (YAML::Node::const_iterator iter = value.begin(); iter != value.end(); iter++)
        {
            if ((*iter).IsScalar())
            {
                defaultValue.push_back((*iter).as<std::string>());
            }
        }
    }

    return defaultValue;
}

std::map<std::string, std::string> KMYAMLNodeHelper::getStringMapValue(const std::string &key)
{
    std::map<std::string, std::string> defaultValue;

    YAML::Node node = m_node[key];
    if (!node.IsDefined())
    {
        return defaultValue;
    }
    
    if (node.IsMap())
    {
        for(YAML::Node::const_iterator iter = node.begin() ; iter != node.end() ; iter++)
        {
            std::string key = iter->first.as<std::string>();
            YAML::Node value = iter->second;
            if (value.IsScalar())
            {
                defaultValue[key] = value.as<std::string>();
            }
        }
    }
    return defaultValue;
}

std::map<std::string, std::string> KMYAMLNodeHelper::fromPrefix(const std::string &pre)
{
    std::map<std::string, std::string> defaultValue;
    
    if (!m_node.IsNull())
    {
        for (YAML::Node::const_iterator iter = m_node.begin() ; iter != m_node.end() ; iter++)
        {
            std::string key = iter->first.as<std::string>();
            YAML::Node value = iter->second;
            if (key.find(pre) == 0 && value.IsScalar())
            {
                defaultValue[key] = value.as<std::string>();
            }
        }
    }
    return defaultValue;
}