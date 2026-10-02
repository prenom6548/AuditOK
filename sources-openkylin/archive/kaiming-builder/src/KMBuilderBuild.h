/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_BUILD_H__
#define __BUILDER_BUILD_H__

#include <string>

#include "KMBuilderContext.h"
#include "KMBuilderModule.h"

using namespace std;

class KMBuilderBuild
{
public:
    KMBuilderBuild() = default;
    ~KMBuilderBuild() = default;

public:
    /**
     * @brief: 模块构建
     * @param: 缓存
     * @param: 模块module
     * @return: true-成功; false-失败
     */
    bool doBuild(const shared_ptr<KMBuilderModule> &pBuildModule);

private:
    /**
     * @brief: 构建过程
     * @param: pBuilderCache缓存结构
     * @param: sourcedir子目录全路径名
     * @return: true-成功; false-失败
     */
    bool buildHelper(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir);

    /**
     * @brief: 构建
     * @param: setupArgs 构建参数
     * @param: args 构建参数
     * @return: true-成功; false-失败
     */
    bool build(const string &cwdSubdir, const vector<string> &setupArgs, const vector<any> &args);

    /**
     * @brief: 构建编译参数
     * @param: appDir 应用目录全路径名
     * @param: cacheDir 缓存目录全路径名
     * @param: name 模块名称
     * @param: sourceDir 源码目录全路径名
     * @param: cwdSubdir 子目录全路径名
     * @param: opts 编译参数
     * @param: env 环境变量
     * @return: true-成功; false-失败
     */
    vector<string> setupBuildArgs(const string &appDir, const string &cacheDir, const string &name, const string &sourceDir, const string &cwdSubdir, const vector<string> &opts,
                                  const vector<string> &env);

    /**
     * @brief: 解压源码
     * @param: pBuildModule模块module
     * @param: 子目录全路径名
     * @return: true-成功; false-失败
     */
    bool extract(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir);

    /**
     * @brief: 编译安装
     * @param: pBuildModule模块module
     * @param: type 编译类型
     * @param: buildDir 编译目录
     * @param: secEnv 加密环境变量
     * @param: setupArgs 构建参数
     * @param: setupTestArgs 测试参数
     * @return: true-成功，false-失败
     */
    bool makeAndInstall(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &appDir, string &buildDir, const vector<string> &secEnv, const vector<string> &setupArgs,
                        const vector<string> &setupTestArgs);

    /**
     * @brief: 获取make参数
     * @param: KMBuildOptions 构建选项
     * @param: moduleMakeargs 模块make参数
     * @param: args 返回的构建参数
     */
    void getMakeArgs(const shared_ptr<KMBuildOptions> &options, const std::vector<std::string> &moduleMakeargs, vector<string> &args);

    /**
     * @brief: 获取test参数
     * @param: KMBuildOptions 构建选项
     * @param: args 返回的构建参数
     */
    void getTestArgs(const shared_ptr<KMBuildOptions> &options, vector<string> &args);

    /**
     * @brief: 获取install参数
     * @param: KMBuildOptions 构建选项
     * @param: moduleInstallargs 模块install参数
     * @param: args 返回的构建参数
     */
    void getInstallArgs(const shared_ptr<KMBuildOptions> &options, const std::vector<std::string> &moduleInstallargs, vector<string> &args);

    /**
     * @brief: makefile是否存在
     * @param: buildDir 编译目录
     * @param: type 编译类型
     * @return: true-成功; false-失败
     */
    bool isExistMakefile(const string &buildDir, const int &type);

    /**
     * @brief: 生成makefile
     * @param: pBuildModule模块module
     * @param: sourceDir 源码目录全路径名
     * @param: sourceSubdir 子目录全路径名
     * @param: config 配置文件全路径名
     * @param: setupArgs 构建参数
     * @param: confopt 配置参数
     * @param: secopt 安全参数
     * @return: true-成功; false-失败
     */
    bool genMakefile(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &sourceDir, const string &sourceSubdir, const string &config, const vector<string> &confopt,
                     const vector<string> &secopt, string &buildDir, vector<string> &setupArgs);

    /**
     * @brief: 生成配置文件
     * @param: sourceDir 源码目录全路径名
     * @param: sourceSubdir 子目录全路径名
     * @param: config 配置文件全路径名
     * @param: setupArgs 构建参数
     * @return: true-成功; false-失败
     */
    bool genConfigFile(const string &sourceDir, const string &sourceSubdir, const string &config, vector<string> setupArgs);

    /**
     * @brief: 获取配置文件
     * @param: type 构建系统类型
     * @param: sourceSubdir 子目录全路径名
     * @param: config 配置文件全路径名
     * @return: true-成功; false-失败
     */
    bool getConfigFilename(const shared_ptr<KMBuilderModule> &pBuildModule, const int &type, const string &sourceSubdir, string &config);

    /**
     * @brief: 获取编译参数
     * @param: options
     * @param: args编译参数
     */
    void getBuildArgs(const shared_ptr<KMBuildOptions> &options, vector<string> &args);

    /**
     * @brief: 获取环境变量
     * @param: BuildOptions编译选项
     */
    void getBuildEnv(const shared_ptr<KMBuildOptions> &options, vector<string> &env);

    /**
     * @brief: 获取额外的环境变量
     * @param: KMBuildOptions
     * @param: env编译环境变量
     */
    void getExtenv(const shared_ptr<KMBuildOptions> &options, vector<string> &env);

    /**
     * @brief: 获取加密环境变量
     * @param: BuildOptions编译选项
     */
    void getBuildSecretEnv(const shared_ptr<KMBuildOptions> &options, vector<string> &env);

    /**
     * @brief: 获取配置选项
     * @param: BuildOptions编译选项
     */
    void getBuildConfigOpts(const shared_ptr<KMBuildOptions> &options, vector<string> &opts);

    /**
     * @brief: 获取秘密选项
     * @param: mapEnv环境变量
     */
    void getBuildSecretOpts(const shared_ptr<KMBuildOptions> &options, vector<string> &opts);

    /**
     * @brief: 清理编译目录
     * @param: sourceLink，源码目录链接文件
     * @param: sourceDir，源码目录
     * @return: true-成功; false-失败
     */
    bool cleanupBuildbdir(const string &sourceLink, const string &sourceDir);

    /**
     * @brief: 获取编译选项前缀
     * @param: KMBuildOptions 构建选项
     * @return: 编译选项前缀
     */
    string getOptionsPrefix(const shared_ptr<KMBuildOptions> &options);

    /**
     * @brief: 获取编译选项libdir
     * @param: KMBuildOptions 构建选项
     * @return: 编译选项libdir
     */
    string getOptionsLibdir(const shared_ptr<KMBuildOptions> &options);

    /**
     * @brief: 获取是否编译运行时
     * @return: true-是, false-否
     */
    bool getBuildRuntime();

    /**
     * @brief : 判断configOpts中是否包含了选项option。
     * @param : [in] configOpts, 选项列表
     * @param : [in] option, 选项的KEY
     * @return: true-包含; false-不包含
     * @note  : 使用场景：
     *              1. 构建流程中的配置环节的prefix和libdir等是否配置
     */ 
    bool containsOption(const std::vector<std::string>& configOpts, const std::string& option);

public:
    string m_subdir;
};

/**
 * @brief: 创建编译子目录
 * @param: dir目录
 * @param: name模块名
 * @param: 子目录全路径名
 * @return: true-成功; false-失败
 */
bool createSubdir(const string &dir, const string &name, string &subdir);

bool download(const shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir);
#endif