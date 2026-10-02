/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDFINISH_H__
#define __KMBUILDFINISH_H__

#include <set>

#include "common/KMSubCommand.h"

/**
 * @brief 	 完成构建目录
 * @note	 供kaiming-builder构建流程中调用
 */
class KMBuildFinish : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuildFinish);

public:
    KMBuildFinish();
    ~KMBuildFinish() override;

    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);
    int run();

    void collectExports();
    bool getAllowedExports(const std::string &path, std::set<std::string> &allowedExtensions, std::set<std::string> &allowedPrefixes, bool &requireExactMatch);

    bool copyExports(const std::string &srcDir, const std::string &destDir, const std::string &srcPrefix, const std::set<std::string> &allowedPrefixes, const std::set<std::string> &allowedExtensions,
                     bool requireExactMatch, std::string &errMsg);

    static bool exportDir(const std::string &srcDir, const std::string &srcName, const std::string &srcRelPath, const std::string &destDir, const std::string &destName,
                          const std::set<std::string> &allowedPrefixes, const std::set<std::string> &allowedExtensions, bool requireExactMatch, std::string &errMsg);

    static std::string findCommandInPath(const std::string &path);

    void updateMetadata();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDFINISH_H__
