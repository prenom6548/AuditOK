/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMDELTASMAIN_H
#define KMDELTASMAIN_H
#define KAIMING_DELTAS (access("/opt/kaiming-tools/bin/xdelta3", F_OK) == 0 ? "/opt/kaiming-tools/bin/xdelta3" : "/usr/bin/xdelta3")

#include <dbus/dbus.h>
#include <sys/stat.h> // 包含 chmod 函数的头文件

#include <cctype>     // 包含 isupper 函数
#include <cstdlib>    // 用于 std::system 函数
#include <functional> // 进度条函数
#include <initializer_list>
#include <iomanip> // For std::fixed and std::setprecision
#include <vector>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <algorithm>
#include <string>

#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/kmtranslation.h"
#include "common/KMPackageManager.h"
#include "common/KMParseSummaryJson.h"
#include "common/KMDirMd5sum.h"
#include "common/KMOKFile.h"


/**
 * @brief 开明安装子命令类
 *
 * @authors yushuoqi
 * @example 增量包构建: kaiming deltas path
 */
class KMDeltas : public KMSubCommand
{
    DECLARE_DYNCREATE(KMDeltas);

public:
    KMDeltas();
    ~KMDeltas() override;

    int dispose(int argc, char **argv) override;
    void init();
    int start(const std::vector<std::string> &vectorAppIds);
    // 获取目录中所有文件的相对路径
    std::vector<fs::path> getFilesInDirectory(const fs::path& directory);
    void createDirectoryStructure(const fs::path& targetDir, const fs::path& relativePath);
    void KMDelcopyDifferentFile(const fs::path& sourceFile, const fs::path& targetDir, const fs::path& relativePath);
    void copyDifferentFile(const fs::path& sourceFile, const fs::path& targetDir, const fs::path& relativePath);
    void addDeltasAnnotation(const fs::path& jsonPath, const fs::path& destPath, vector<std::string> preVersion);
    int deltasOK(const std::string& oldVersionPath, const std::string& newVersionPath, const std::string& outPath);
    void clearDirectory(const fs::path& dir_path);
    bool executeRsyncAndProcess(const std::string& sourceDir, const std::string& targetDir, 
                               const std::string& outputBaseDir);
    std::vector<std::string> executeCommand(const std::string& command);
    bool copyFileWithDirectories(const std::string& sourcePath, const std::string& targetPath);
    bool shouldUseFullCopyForDirectories(const std::string& sourceDir, const std::string& targetDir);

private:
    class Options;
    class Private;
    std::shared_ptr<Private> d;
    std::shared_ptr<KMDirMd5sum> m_md5Checker;
};

#endif