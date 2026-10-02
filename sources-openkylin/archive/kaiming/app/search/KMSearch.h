/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

// #ifndef KMSEARCH_H
// #define KMSEARCH_H

#include <dbus/dbus.h>
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
#include <sys/stat.h> // 包含 chmod 函数的头文件

#include "common/KMSubCommand.h"
#include "common/KMContext.h"
#include "common/KMEnv.h"
#include "common/KMFolder.h"
#include "common/KMOSTreeHandler.h"
#include "common/KMPackageManager.h"

typedef struct _KMSearchInfo
{
    string id;            // 包id, 如 "org.kde.kclock"
    string description;   // 包描述,如 "Set alarms and timers, use a stopwatch, and manage world clocks"
    string completedId;   // 包完整id, 如 "app/org.kde.kclock/x86_64/stable"
    string name;          // 包实际名称, 如 闹钟，"KDE"
    string arch;          // 包架构，如 "x86_64"
    string branch;        // 分支名，如 "stable"
    string module;        // 包所在模块: app, runtime
    string version;       // 包版本，如 "24.02.2"
    string license;       // 包证书，如 "GPL-2.0-or-later"
    string origin;        // 软件源仓库名称，如 "kaiming-repo"
    string collection;    // 软件 collection_id, 如 "org.kylin.kaiming"，和离线导入有关
    double size;          // 安装大小
    string installedSize; // 软件包大小
    string runtime;       // 运行时，如 "org.kde.Platform/x86_64/6.6"
    string sdk;           // SDK,如 "org.kde.Sdk/x86_64/6.6"
    string commit;        // 目前对应的commit
    GVariant *commitData; // 目前对应的commit data
    string parent;        // 上一个commit
    string subject;       // commit 提交的内容改变
    string date;          // commit 修改的时间
} KMSearchInfo;

/**
 * @brief kaiming search 命令
 * @note 展示软件包信息
 */
class KMSearch : public KMSubCommand
{
    DECLARE_DYNCREATE(KMSearch);

public:
    KMSearch();
    ~KMSearch() override;

    int dispose(int argc, char **argv) override;
    void init();

    int start(const std::vector<std::string> &name);
    int dbusStart(const std::vector<std::string> &name);

    /* Todo: 软件商店回调接口 */
    int getRemoteInfo(const std::string &name, std::string &strRemoteInfoJson, std::vector<std::string> &allRemoteInfo, std::string &remoteKind);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};
