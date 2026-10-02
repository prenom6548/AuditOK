/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <fstream>
#include <filesystem>
#include "KMRunnerMount.h"
#include "common/KMOverlay.h"
#include "common/KMContainer.h"

namespace fs = std::filesystem;

/**
 * @brief 挂载overlay
 * @param lowers目录
 * @param upper目录
 * @param merge目录
 * @return true 成功 false 失败
 */
bool KMRunnerMount::mountOverlay(KMContainer *container, const std::string &appDataPath, const std::vector<std::string> &lowers, const std::string &upper, const std::string &merge, const std::vector<std::string> &mountOptions)
{
    // 多个挂载不能使用同一upperdir,workdir,否则会导致未知风险问题
    std::error_code ec;
    std::string workdir = appDataPath + "/workdir" + upper;
    std::string upperdir = appDataPath + "/upperdir" + upper;
    if (!fs::exists(upperdir, ec) || !fs::exists(workdir, ec))
    {
        fs::create_directories(upperdir, ec);
        fs::create_directories(workdir, ec);
    }

    std::string upperOpt = "upperdir=" + upperdir;
    std::string workOpt = "workdir=" + workdir;
    std::string lowerOpt = "lowerdir=";

    for (auto const &dir : lowers)
    {
        appendLowerDir(lowerOpt, dir);
    }

    // 判断是否存在lower层
    if (lowerOpt == "lowerdir=")
    {
        return true;
    }

    std::vector<std::string> options = { lowerOpt, upperOpt, workOpt };
    options.insert(options.end(), mountOptions.begin(), mountOptions.end());

    if (KMOverlay::isOverlayFsIndexOff())
    {
        options.emplace_back("index=off");
    }

    // 判断内核版本，如果支持userxattr，则添加到options中
    if (KMOverlay::isOverlayUserXAttr())
    {
        options.emplace_back("userxattr");
    }

    container->addMount(Mount{ .source = "overlay", .destination = merge, .type = "overlay", .options = options });

    return true;
}

/**
 * @brief 挂载指定目录
 */
void KMRunnerMount::mountDir(KMContainer *container, const std::string &appDataPath, bool isBindHostHome, const std::string &hostDir, const std::string &containDir, const std::string &type, std::vector<std::string> &options)
{
    // 挂载指定目录
    if (hostDir.empty() || containDir.empty())
        return;

    std::error_code ec;
    if (!fs::exists(hostDir, ec))
        return;

    size_t pos = containDir.find('/', 1);
    std::string prefix = (pos == std::string::npos) ? containDir : containDir.substr(0, pos);
    std::string dst = appDataPath + "/upperdir" + containDir;
    if (prefix == "/root")
    {
        dst = containDir;
    }
    if (prefix == "/home" && isBindHostHome)
    {
        dst = containDir;
    }
    if (fs::exists(dst, ec))
    {
        container->addMount(Mount{ .source = hostDir, .destination = containDir, .type = type, .options = options });
        return;
    }

    std::vector<std::string> dirs = { "/usr", "/lib", "/lib32", "/lib64", "/bin", "/sbin", "/etc", "/opt", "/var", "/srv", "/home", "/root" };

    if (std::find(dirs.begin(), dirs.end(), prefix) != dirs.end())
    {
        // 创建overlay需要的upperdir目录
        if (!fs::is_directory(hostDir, ec))
        {
            fs::path parent = fs::path(dst).parent_path();
            if (!fs::exists(parent, ec))
            {
                fs::create_directories(parent, ec);
            }
            std::ofstream(dst, std::ios::trunc);
        }
        else if (!fs::exists(dst, ec))
        {
            fs::create_directories(dst, ec);
        }
    }
    container->addMount(Mount{ .source = hostDir, .destination = containDir, .type = type, .options = options });
}

/**
 * @brief 追加lowerdir
 * @param lowerdir lowerdir
 * @param dir 追加的目录
 */
void KMRunnerMount::appendLowerDir(std::string &lowerdir, const std::string &dir)
{
    std::error_code ec;
    if (!dir.empty() && fs::exists(dir, ec))
    {
        if (lowerdir.length() > strlen("lowerdir="))
        {
            lowerdir.append(":");
        }
        lowerdir.append(dir);
    }
}
