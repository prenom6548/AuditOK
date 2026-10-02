/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMHOSTMODEBUILD_H__
#define __KMHOSTMODEBUILD_H__

#include "KMSubCommand.h"

class KMHostModeBuild : public KMSubCommand
{
    DECLARE_DYNCREATE(KMHostModeBuild);

public:
    KMHostModeBuild();
    virtual ~KMHostModeBuild();

int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);

    int run();

    void installBuildDepends();
    void installDebDepends();
    void installKaimingDepends();

    void configure();
    void ensureBuildSystem();
    void getConfigFilename();
    void generateMakefile();
    bool containsOption(const std::vector<std::string>& configOpts, const std::string& option);

    void costomBuild();

    void build();

    void install();

    void postBuild();

    void finish();
    void installScriptFiles();
    void generateMetadata();
    void generateSubPackagesJson();

    void exportPackage();

    /**
     * @brief : 收集子开明包包含的路径列表
     * @param : [in] path, 指定路径; files，指定的文件列表
     * @param : [in] prefix, 前缀，匹配比较时，如果match是相对路径，需要去掉文件/目录的前缀后比较
     * @param : [in] matchs, 匹配规则
     * @param : [out] includes，子开明包包含的路径列表
     */ 
    static void collectSubPackagesPath(const std::string& path, const std::string& prefix, const std::vector<std::string>& matchs, std::vector<std::string> &includes);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMHOSTMODEBUILD_H__

