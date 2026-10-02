/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMSubPackagesJson.h"

#include <fstream>
#include <filesystem>

#include "KMException.h"
#include "KMFileUtils.h"

namespace fs = std::filesystem;

KMSubPackagesJson::KMSubPackagesJson()
{}

KMSubPackagesJson::KMSubPackagesJson(nlohmann::json rootNode)
{
    m_root = rootNode;
}

KMSubPackagesJson::~KMSubPackagesJson()
{}

std::shared_ptr<KMSubPackagesJson> KMSubPackagesJson::loadFile(const std::string& jsonFile)
{
    std::shared_ptr<KMSubPackagesJson> json;
    nlohmann::json rootNode = KMJsonHelper::loadFile(jsonFile);
    json.reset(new KMSubPackagesJson(rootNode));

    json->fromJson();

    return json;
}

void KMSubPackagesJson::saveFile(const std::string& jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code e;
        if (!KMFileUtils::mkpath(baseDir.string(), e))
        {
            throw KMException(e.message());
        }
    }

    toJson();

    std::ofstream o(jsonFile);
    o << std::setw(4) << m_root << std::endl;
}

void KMSubPackagesJson::fromJson()
{
    parseSubpackages();
}

void KMSubPackagesJson::toJson()
{
    subpackagesToJson();
}

void KMSubPackagesJson::parseSubpackages()
{
    if (!m_root.contains("subpackages"))
    {
        return ;
    }

    nlohmann::json& subpackagesNode = m_root["subpackages"];
    if (!subpackagesNode.is_array())
    {
        return ;
    }

    for (auto& element : subpackagesNode)
    {
        KMJsonHelper helper(element);
        SubPackage sub;
        sub.id = helper.getStringValue("id");
        sub.name = helper.getStringValue("name");
        sub.kind = helper.getStringValue("kind");
        sub.version = helper.getStringValue("version");
        sub.module = helper.getStringValue("module");
        sub.description = helper.getStringValue("description");

        sub.names = helper.fromPrefix("name[");
        sub.descriptions = helper.fromPrefix("description[");

        sub.commands = helper.getStringListValue("command");
        sub.includes = helper.getStringListValue("includes");
        sub.depends = helper.getStringListValue("depends");

        subpackages.push_back(sub);
    }
}

void KMSubPackagesJson::subpackagesToJson()
{
    if (subpackages.empty())
    {
        return ;
    }

    nlohmann::json array = nlohmann::json::array();
    for (auto& subpackage : subpackages)
    {
        nlohmann::json object = nlohmann::json::object();
        KMJsonHelper helper(object);
        helper.setStringValue("id", subpackage.id);
        helper.setStringValue("name", subpackage.name);
        helper.setStringValue("kind", subpackage.kind);
        helper.setStringValue("version", subpackage.version);
        helper.setStringValue("module", subpackage.module);
        helper.setStringValue("description", subpackage.description);

        for (auto& pair : subpackage.names)
        {
            object[pair.first] = pair.second;
        }

        for (auto& pair : subpackage.descriptions)
        {
            object[pair.first] = pair.second;
        }

        helper.setStringListValue("command", subpackage.commands);
        helper.setStringListValue("includes", subpackage.includes);
        helper.setStringListValue("depends", subpackage.depends);

        array.push_back(object);
    }

    m_root["subpackages"] = array;
}
