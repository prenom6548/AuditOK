/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderCache.h"

#include <filesystem>

#include "common/KMNlohmannJsonHelper.h"
#include "common/KMProcessUtils.h"
#include "KMBuilderUtils.h"

namespace fs = std::filesystem;

class KMBuilderCache::Private
{
public:
    std::string m_cacheDir;
    std::string m_stageCache;

    nlohmann::json m_root;
};

KMBuilderCache::KMBuilderCache(token)
    : d(std::make_unique<Private>())
{
}

KMBuilderCache::~KMBuilderCache()
{
}

void KMBuilderCache::open(const std::string &cacheDir)
{
    d->m_cacheDir = cacheDir;
    d->m_stageCache = KMBuilderUtils::buildFilename(cacheDir, "stage-cache.json");

    if (!KMBuilderUtils::pathExists(d->m_stageCache))
    {
        saveFile(d->m_stageCache);
    }

    d->m_root = KMNlohmannJsonHelper::loadFile(d->m_stageCache);
}

bool KMBuilderCache::lookup(const std::string &stage)
{
    // KMNlohmannJsonHelper helper(d->m_root);
    // return helper.getBoolValue(stage);
    nlohmann::json value = d->m_root[stage];
    return !value.is_null();
}

void KMBuilderCache::recordStage(const std::string &stage)
{
    KMNlohmannJsonHelper helper(d->m_root);
    helper.setBoolValue(stage, true);

    saveFile(d->m_stageCache);
}

void KMBuilderCache::recordStage(const std::string &stage, const std::vector<std::string> &files)
{
    KMNlohmannJsonHelper helper(d->m_root);
    helper.setStringListValue(stage, files);

    saveFile(d->m_stageCache);
}

void KMBuilderCache::saveFile(const std::string& jsonFile)
{
    fs::path baseDir = fs::path(jsonFile).parent_path();
    if (!fs::exists(baseDir))
    {
        std::error_code e;
        if (!KMBuilderUtils::mkpath(baseDir.string(), e))
        {
            throw KMException(e.message());
        }
    }

    std::ofstream o(jsonFile);
    o << std::setw(4) << d->m_root << std::endl;
}
