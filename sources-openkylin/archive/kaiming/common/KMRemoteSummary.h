/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMINSTALLREMOTESUMMARY_H
#define KMINSTALLREMOTESUMMARY_H

#include <curl/curl.h>
#include <zlib.h>

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "common/KMConfig.h"
#include "common/KMFolder.h"
#include "common/KMMacros.h"
#include "common/KMStorageDir.h"
#include "common/KMUtil.h"

using namespace std;

typedef struct _KMCommonRefApplication
{
    string name;
    string runtime;
    string sdk;
    string command;
} KMCommonRefApplication;

typedef struct _KMCommonRefContext
{
    vector<string> shared;
    vector<string> sockets;
    vector<string> devices;
    vector<string> filesystems;
} KMCommonRefContext;

typedef struct _KMCommonRefSessionBusPolicy
{
    string appmenuRegistrar;
    string notify;
    string globalSettings;
    string owned;
    string desktopNotifications;
    string powerManagement;
} KMCommonRefSessionBusPolicy;

typedef struct _KMCommonRefExtension
{
    string name;
    string directory;
    string autodelete;
    string localeSubset;
    string noAutodownload;
} KMCommonRefExtension;

typedef struct _KMCommonRefBuild
{
    vector<string> builtExtensions;
} KMCommonRefBuild;

typedef struct _KMCommonRef
{
    bool isYes = false;           // 同时识别用户的输入参数，true 代表自动确认，用户不用选择，在列出依赖列表进行安装时自动安装
    std::string remote;           // 远程仓库名，目前直接设为 "kaiming-repo", 后续需要自动获取仓库名
    std::string repoSummaryPath;  // 解析的repo summary路径, *-ref.sub
    std::string summaryIdxPath;   // 解析的 summary.idx 路径
    std::string repoSummaryIndex; // summary.idx 中的 commit index
    bool resolved;
    guint64 downloadSize;      // 运行时大小，当前单位为B
    guint64 installedSizeInt;     // 软件包大小，当前为字节B单位
    guint64 realInstalledSize;  // 软件包真正大小,包括不同运行时和软件包
    string installedSize;       // 真正的安装大小

    GVariant *commitDataGVariant;     // 从ostree中获取的commitData
    GVariant *commitMetadataGVariant; // 根据commit_data 获取的 commitMetadata
    GBytes *deployData;       // 每个ref 部署的deploy字节流
    GFile *deployDataFile;   // 每个ref部署的deploy文件

    std::string pref;                 // 用户输入的ref，需要解析出需要的信息。目前支持2种单个 ref 格式：  org.kde.kclock， app/org.kde.kclock/x86_64/master
    std::string baseType;             // ref 的两种基本类型: "app", "runtime"
    std::string completedId;          // 完整 refId, 如 "app/org.kde.kclock/x86_64/master", "runtime/top.openkylin.Platform/x86_64/0.5"
    std::string id;                   // ref 中的id，如 "org.kde.kclock"
    std::string arch;                 // 架构，如 "x86_64"
    std::string branch;               // app分支名，默认 "master", 或者由subsummary中指定的versions或version对应的值
    std::string defaultRuntimeBranch; // runtime分支名，默认 "master", 或者由subsummary中指定的versions或version对应的值
    std::string commit = "active";    // ref 对应的commit，默认active, 否则就是从subsummary中获取的commit。如 ref info=> id: app/org.kde.kclock/x86_64/master commit: f3b1314b51f2b8865089f13cbae5b7cd449b169f0b36530d27da74beb68aa492
    vector<string> matchRefs;         // 从 summary 中获取的匹配refs，供用户选择
    string choiceRef;                 // 用户选择安装的ref
    std::string runtime;              // ref对应的runtime字段值

    vector<string> subpaths;
    bool hasDownload = false; // ref是否需要下载，添加到下载列表中
    bool autoDelete = false;  // ref是否需要删除，避免在下载列表中
    bool autoPrune = false;
    std::map<std::string, std::string> dependencies; // runtime依赖列表: <runtimeid, commitid>

    KMCommonRefApplication refApplication; // 应用的Application信息，包括command字段
    KMCommonRefContext refContext;
    KMCommonRefSessionBusPolicy refSessionBusPolicy;
    KMCommonRefExtension refExtension;
    KMCommonRefBuild refBuild;
} KMCommonRef;


/**
 * @brief parse summary 
 */
class KMRemoteSummary
{
public:
    KMRemoteSummary();
    ~KMRemoteSummary();

    static bool curlCmdDownload();

private:
    class Private;
    std::shared_ptr<Private> d;
};

#endif // !KMINSTALLREMOTESUMMARY_H