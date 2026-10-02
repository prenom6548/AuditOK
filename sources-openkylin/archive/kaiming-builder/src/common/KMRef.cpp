/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMRef.h"
#include "KMInfoJson.h"
#include "../KMBuilderUtils.h"

void KMRef::loadFromInfoJson(const std::string &path)
{
    KMInfoJson info;
    info.loadFile(path);
    m_id = info.id;
    m_name = info.name;
    m_kind = info.kind;
    m_version = info.version;
    m_channel = info.channel;
    m_module = info.module;
    m_description = info.description;
    m_base = info.base;
    m_runtime = info.runtime;
    m_arch = KMBuilderUtils::getArch();
}

void KMRef::parseInfo(const std::string &strFormat)
{
    std::vector<std::string> channel_id_version = KMBuilderUtils::splitString(strFormat, ":");
    if (channel_id_version.size() != 2)
    {
        throw KMException("The base or runtime is Error.");
    }
    m_channel = channel_id_version[0];
    std::vector<std::string> id_version = KMBuilderUtils::splitString(channel_id_version[1], "/");
    m_id = id_version[0];
    m_version = id_version[1];
}

void KMRef::parseVersion()
{
    std::vector<std::string> versions = KMBuilderUtils::splitString(m_version, ".");
    m_major = std::stoi(versions[0]);
}

bool KMRef::check(KMRef &ref)
{
    ref.parseVersion();
    parseVersion();
    if (m_channel != ref.m_channel || m_id != ref.m_id || m_major != ref.m_major)
    {
        return false;
    }
    return true;
}