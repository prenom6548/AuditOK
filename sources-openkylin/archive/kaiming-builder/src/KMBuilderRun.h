/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDERRUN_H__
#define __KMBUILDERRUN_H__

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include "KMSubCommand.h"

#include "KMBuilderManifest.h"

namespace fs = std::filesystem;

#define STATE_DIR ".kaiming-builder"

class KMBuilderRun : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuilderRun);

public:
    KMBuilderRun();
    virtual ~KMBuilderRun();

    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);
    int run();

    void checkOptions();
    std::string getCcacheDir(); 
    std::string getBuildDir(); 
    std::string getAppDir(); 

    /**
     * @brief : 初始化参数
     * @return : true-成功; false-失败
     */
    bool initArgs(std::vector<std::string> &args);
    void getBuildEnv(std::vector<std::string> &envs);
    void getModuleBuildOptions(const std::shared_ptr<KMBuildOptions> &options, std::vector<std::string> &buildOptions);


    /**
     * @brief: 下载项目源码
     */
    void downloadsFiles();
    void buildInit();
    bool extract(const shared_ptr<KMBuilderModule> &component, const string &sourcedir);
    void expandComponents(const std::vector<std::shared_ptr<KMBuilderModule>>components, std::vector<std::shared_ptr<KMBuilderModule>> &expandComponents);
    

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDERRUN_H__