/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMINSTALLMAIN_H
#define KMINSTALLMAIN_H

#include <dbus/dbus.h>
#include <sys/stat.h> // 包含 chmod 函数的头文件

#include <cctype>     // 包含 isupper 函数
#include <cstdlib>    // 用于 std::system 函数
#include <functional> // 进度条函数
#include <initializer_list>
#include <iomanip> // For std::fixed and std::setprecision
#include <set>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/KMSubCommand.h"
#include "common/KMContext.h"
#include "common/KMEnv.h"
#include "common/KMFolder.h"
#include "common/KMOSTreeHandler.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "common/KMPackageManager.h"
#include "common/KMParseSummaryJson.h"

/**
 * @brief 开明安装子命令类
 *
 * @authors yushuoqi
 * @example 在线安装: sudo kaiming install top.openkylin.Music
 */
class KMInstall : public KMSubCommand
{
    DECLARE_DYNCREATE(KMInstall);

public:
    KMInstall();
    ~KMInstall() override;

    int dispose(int argc, char **argv) override;
    void init();
    int start(const std::vector<std::string> &vectorAppIds);

private:
    class Options;
    class Private;
    std::shared_ptr<Private> d;
};

#endif // KMINSTALLMAIN_H
