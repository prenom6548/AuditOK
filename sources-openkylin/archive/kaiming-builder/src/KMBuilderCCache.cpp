/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderCCache.h"

#include <filesystem>
#include <fstream>

#include "KMBuilderContext.h"
#include "common/KMLogger.h"

namespace fs = std::filesystem;

/**
 * @brief: 设置ccache目录
 * @param: 是否启用
 * @param: ccache缓存目录
 */
bool KMBuilderCCache::setCcache(const bool enable, const string &ccacheDir)
{
    if (enable)
    {
        std::error_code ec;
        string binDir = ccacheDir + "/bin";
        if (!fs::exists(binDir, ec) && !fs::create_directories(binDir, ec))
        {
            KMError(ec.message());
            return false;
        }

        vector<string> compilers = { "cc", "c++", "gcc", "g++" };
        for (auto &compiler : compilers)
        {
            string symlinkPath = binDir + string("/") + compiler;
            if (fs::exists(symlinkPath, ec) && !fs::remove(symlinkPath, ec))
            {
                KMError(ec.message());
                return false;
            }
            fs::create_symlink("/usr/bin/ccache", symlinkPath, ec);
        }
    }
    else
    {
        std::error_code ec;
        string disabledDir = ccacheDir + "/disabled";
        if (!fs::exists(disabledDir, ec) && !fs::create_directories(disabledDir, ec))
        {
            KMError(ec.message());
            return false;
        }

        string ccacheConfiPath = disabledDir + "/ccache.conf";
        ofstream file(ccacheConfiPath, ios::out);
        file << "disable = true\n";
        file.close();
    }
    return true;
}
