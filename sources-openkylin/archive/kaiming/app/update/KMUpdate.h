/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMUPDATE_H
#define KMUPDATE_H

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

typedef struct _UpdateProgressStatus
{
    std::string refId;
    int progress;
    std::string speed;
    int timeout;
} UpdateProgressStatus;

typedef function<void(std::string updateProgressStatus)> UpdateCallBack;

/**
 * @brief update command
 */
class KMUpdate : public KMSubCommand
{
    DECLARE_DYNCREATE(KMUpdate);

public:
    KMUpdate();
    ~KMUpdate() override;

    int dispose(int argc, char **argv) override;
    void init();
    int start(const std::vector<std::string> &vecName);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // KMUPDATE_H
