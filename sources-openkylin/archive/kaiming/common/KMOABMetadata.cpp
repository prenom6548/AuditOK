/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABMetadata.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void KMOABMetadata::parse(const std::string &jsonContent)
{
    m_root = nlohmann::json::parse(jsonContent);

    fromJson();
}

std::string KMOABMetadata::dump()
{
    toJson();

    return m_root.dump(4);
}

void KMOABMetadata::fromJson()
{
    KMJsonHelper helper(m_root);
    version = helper.getStringValue("version", "1");
    uuid = helper.getStringValue("uuid");
    digest = helper.getStringValue("digest");
    bundleFilesystem = helper.getStringValue("bundle-filesystem");

    nlohmann::json sectionsNode = m_root["sections"];
    if (sectionsNode.is_object())
    {
        KMJsonHelper sectionsHelper(sectionsNode);
        sections.bundle = sectionsHelper.getStringValue("bundle");
        sections.icon = sectionsHelper.getStringValue("icon");
        sections.extra_meta = sectionsHelper.getStringValue("extra-meta");
    }

    nlohmann::json layersNode = m_root["layers"];
    if (layersNode.is_array())
    {
        for (auto& item : layersNode)
        {
            if (item.is_object())
            {
                nlohmann::json info = item["info"];
                if (info.is_object())
                {
                    KMInfoJson infoJson;
                    infoJson.setRoot(info);
                    layers.push_back(infoJson);
                }
            }
        }
    }
}

void KMOABMetadata::toJson()
{
    m_root["version"] = version;
    m_root["uuid"] = uuid;
    m_root["digest"] = digest;
    m_root["bundle-filesystem"] = bundleFilesystem;

    nlohmann::json sectionsNode;
    sectionsNode["bundle"] = sections.bundle;
    if (!sections.icon.empty())
    {
        sectionsNode["icon"] = sections.icon;
    }
    if (!sections.extra_meta.empty())
    {
        sectionsNode["extra-meta"] = sections.extra_meta;
    }
    m_root["sections"] = sectionsNode;

    nlohmann::json layersNode;
    for (auto &layer : layers)
    {
        nlohmann::json layerNode;
        layerNode["info"] = layer.root();
        layersNode.push_back(layerNode);
    }
    m_root["layers"] = layersNode;
}

void KMOABMetadata::saveFile(const std::string &jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code ec;
        bool result = fs::create_directories(baseDir.string(), ec);
        if (!result && 0 != ec.value())
        {
            throw KMException(ec.message());
        }
    }

    toJson();

    std::ofstream o(jsonFile);
    o << std::setw(4) << m_root << std::endl;
}
