/*
 * Copyright (c) KylinSoft  Co., Ltd. 2026. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include "KMDeferredCleanupStore.h"

#include "KMJsonHelper.h"
#include "KMLogger.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace {
constexpr const char *kDeferredRefsFileName = "pending_deferred_cleanup_refs.json";
constexpr const char *kLastBootIdFileName = "last_deferred_cleanup_boot_id";
}

KMDeferredCleanupStore::KMDeferredCleanupStore(const fs::path &infoDirectory)
    : m_deferredRefsPath((infoDirectory / kDeferredRefsFileName).string())
    , m_lastBootIdPath((infoDirectory / kLastBootIdFileName).string())
{
}

KMDeferredCleanupStore::KMDeferredCleanupStore(std::string deferredRefsPath, std::string lastBootIdPath)
    : m_deferredRefsPath(std::move(deferredRefsPath))
    , m_lastBootIdPath(std::move(lastBootIdPath))
{
}

std::string KMDeferredCleanupStore::trimCopy(std::string s)
{
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t'))
    {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
    {
        ++i;
    }
    return s.substr(i);
}

std::string KMDeferredCleanupStore::readKernelBootId()
{
    std::ifstream in("/proc/sys/kernel/random/boot_id");
    if (!in.is_open())
    {
        return {};
    }
    std::string line;
    std::getline(in, line);
    return trimCopy(std::move(line));
}

std::string KMDeferredCleanupStore::readLastCleanupBootId() const
{
    std::ifstream in(m_lastBootIdPath);
    if (!in.is_open())
    {
        return {};
    }
    std::string line;
    std::getline(in, line);
    return trimCopy(std::move(line));
}

void KMDeferredCleanupStore::writeLastCleanupBootId(const std::string &bootId) const
{
    std::error_code ec;
    fs::create_directories(fs::path(m_lastBootIdPath).parent_path(), ec);
    if (ec)
    {
        kmlogger.warn("Failed to create dir for %s: %s", m_lastBootIdPath.c_str(), ec.message().c_str());
        return;
    }
    std::ofstream out(m_lastBootIdPath, std::ios::trunc);
    if (!out.is_open())
    {
        kmlogger.warn("Failed to write boot id file: %s", m_lastBootIdPath.c_str());
        return;
    }
    out << bootId;
}

std::vector<std::string> KMDeferredCleanupStore::loadDeferredRefs() const
{
    std::vector<std::string> refs;
    std::error_code ec;
    if (!fs::exists(m_deferredRefsPath, ec))
    {
        return refs;
    }

    try
    {
        nlohmann::json root = KMJsonHelper::loadFile(m_deferredRefsPath);
        if (!root.is_object() || !root.contains("refs") || !root["refs"].is_array())
        {
            return refs;
        }

        for (const auto &item : root["refs"])
        {
            if (item.is_string())
            {
                refs.emplace_back(item.get<std::string>());
            }
        }
    }
    catch (...)
    {
        kmlogger.warn("Failed to parse deferred old refs file: %s", m_deferredRefsPath.c_str());
    }

    return refs;
}

void KMDeferredCleanupStore::saveDeferredRefs(const std::vector<std::string> &refs) const
{
    if (refs.empty())
    {
        std::error_code ec;
        fs::remove(m_deferredRefsPath, ec);
        return;
    }

    std::error_code ec;
    fs::create_directories(fs::path(m_deferredRefsPath).parent_path(), ec);
    if (ec)
    {
        kmlogger.warn("Failed to create deferred old refs dir: %s", ec.message().c_str());
        return;
    }

    nlohmann::json root;
    root["refs"] = refs;

    std::ofstream out(m_deferredRefsPath, std::ios::trunc);
    if (!out.is_open())
    {
        kmlogger.warn("Failed to open deferred old refs file: %s", m_deferredRefsPath.c_str());
        return;
    }
    out << root.dump(2);
}
