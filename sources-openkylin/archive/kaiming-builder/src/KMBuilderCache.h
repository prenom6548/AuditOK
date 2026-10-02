/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDERCACHE_H__
#define __KMBUILDERCACHE_H__

#include <string>
#include <memory>
#include <vector>

#include "common/KMSingleton.h"

class KMBuilderCache : public KMSingleton<KMBuilderCache>
{
public:
    KMBuilderCache(token);
    virtual ~KMBuilderCache();

    void open(const std::string &cacheDir);

    bool lookup(const std::string &stage);

    void recordStage(const std::string &stage);
    void recordStage(const std::string &stage, const std::vector<std::string> &files);

private:
    void saveFile(const std::string& jsonFile);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDERCACHE_H__
