/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMLIST_H
#define KMLIST_H

#include <dbus/dbus.h>
#include <sys/stat.h> // 包含 chmod 函数的头文件

#include <cctype>     // 包含 isupper 函数
#include <cstdlib>    // 用于 std::system 函数
#include <functional> // 进度条函数
#include <initializer_list>
#include <iomanip> // For std::fixed and std::setprecision
#include <list>
#include <set>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>

#include "common/KMSubCommand.h"
#include "common/KMFormatter.h"
#include "common/KMContext.h"
#include "common/KMEnv.h"
#include "common/KMFolder.h"
#include "common/KMOSTreeHandler.h"
#include "common/KMBuildinOptions.h"
#include "common/KMHostAppDeploy.h"

typedef struct _KMLocalInfoList
{
    std::string id;                 // 包id, 如 org.kde.kclock
    std::string name;               // 包实际名称, 如 闹钟
    std::string version;            // 包版本
    std::vector<std::string> archs; // 包架构
    std::string kind;               // 包类型
    std::string module;             // 包所在模块: app, runtime
    std::string channel;            // 包所在厂商,保留
    std::string description;        // 包描述
    double size;                    // 安装大小
    double packageSize;             // 包大小
    std::string branch;             // 分支名
    list<string> files;             // 安装的文件列表,类似于 dpkg -L
} KMLocalInfoList;

typedef struct KMlocalAppsInfo
{
    std::string id;
    std::string version;
    std::string module;
} KMlocalAppsInfo;

/**
 * @brief ``kaiming list`` 命令可以列出所有已安装的应用。
 */
class KMList : public KMSubCommand
{
    DECLARE_DYNCREATE(KMList);

public:
    KMList();
    ~KMList() override;

    int dispose(int argc, char **argv) override;
    void init();
    int start(const std::vector<string> &vec_name);
    const std::vector<KMlocalAppsInfo>& getLocalAppIds(const bool &isPrint);
    std::vector<std::string> getAllLocalInfo();

private:
    std::string formatFileSize(double size) const;
    void findInfoJsonFiles(const fs::path &dirPath, const std::string &targetFilename, const std::string &destDir, const bool &isPrint);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // KMLIST_H
