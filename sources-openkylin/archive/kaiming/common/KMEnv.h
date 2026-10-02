/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMENV_H
#define KMENV_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <gio/gio.h>
#include <string.h>
#include <glib.h>

using namespace std;
namespace fs = std::filesystem;

/**
 * @brief : 用于封装环境变量相关操作
 */
class KMEnv
{
public:
    KMEnv();
    ~KMEnv();

    bool passEnvCheck();

public:
    bool isSudoOrRoot();
    std::string getXdgDataDirs();
    std::string getRepoUrl();

private:
    class Private;
    std::shared_ptr<Private> d;
};

#endif // KMENV_H