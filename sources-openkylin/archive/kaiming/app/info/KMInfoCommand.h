/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef KMINFOCOMAND_H
#define KMINFOCOMAND_H

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

using namespace std;

/**
 * @brief kaiming info 展示的软件包信息
 *
 */
typedef struct _KMLocalInfo
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
} KMLocalInfo;

/**
 * @brief kaiming info 命令
 * @note 展示软件包信息
 */
class KMInfoCommand : public KMSubCommand
{
    DECLARE_DYNCREATE(KMInfoCommand);

public:
    KMInfoCommand();
    ~KMInfoCommand() final;

    int dispose(int argc, char **argv) override;
    void init();

    int start(const std::string &name);

    // 软件商店回调接口
    string getLocalInfo(const std::string &name, int &errorCode);
    string getLocalDebInfo(const string &name, int &errorCode);

private:
    bool isValidRefId(const std::string &refId) const;
    void printRefInfo(const KMLocalInfo &localInfo);
    string formatFileSize(double size) const;
    void printBoldColorHead(int color, const char *head, const char *text) const;
    void findInfoJsonFiles(const fs::path &dirPath, const std::string &targetFilename, const std::string &destDir, const std::string &refId, bool &isFound);
    void listFilesDirectory(const std::string &name);
    void getAllFiles(const std::string &directoryPath, std::set<std::string> &files, const std::string &targetFilename, bool isFoundFile);

    /**
     * @brief: 从指定的list文件中读取内容并输出
     * @param {path} &listFilePath list文件路径
     * @return {*}
     */
    void fileInListFile(const fs::path &listFilePath);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // KMINFOCOMAND_H
