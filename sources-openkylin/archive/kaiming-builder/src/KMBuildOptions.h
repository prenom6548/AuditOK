/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_OPTIONS_H__
#define __BUILDER_OPTIONS_H__

#include <map>
#include <string>
#include <vector>
#include <memory>

using namespace std;

// typedef struct
// {
//     bool m_strip = false;
//     bool m_noDebuginfo = false;
//     bool m_noDebuginfoCompression = false;
//     string m_cFlags = "";
//     bool m_cflagsOverride = false;
//     string m_cppFlags = "";
//     bool m_cppFlagsOverride = false;
//     string m_cxxFlags = "";
//     bool m_cxxFlagsOverride = false;
//     string m_ldFlags = "";
//     bool m_ldFlagsOverride = false;
//     string m_appendPath = "";
//     string m_prependPath = "";
//     string m_appendLdLibraryPath = "";
//     string m_prependLdLibraryPath = "";
//     string m_appendPkgConfigPath = "";
//     string m_prependPkgConfigPath = "";
//     string m_prefix = "";
//     string m_libDir = "";
//     vector<string> m_buildArgs;
//     vector<string> m_testArgs;
//     vector<string> m_configOpts;
//     vector<string> m_secretOpts;
//     vector<string> m_makeArgs;
//     vector<string> m_makeInstallArgs;
//     map<string, string> m_secretEnv;
//     map<string, string> m_env;
// } KMBuildOptions;

class KMBuildOptions 
{
public:  
   
    // 静态方法获取 buildArgs
    static void getBuildArgs(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& args);

    /**
     * @brief : 处理环境变量中路径变量的前面插入和后面追加
     * @param : [in | out] envs，环境变量map
     * @param : [in] key, 路径类变量的key，如：PATH
     * @param : [in] prepend, 前面追加
     * @param : [in] append, 后面追加
     */ 
    static void concatEnvironmentPath(std::map<std::string, std::string> &envs, const std::string &key, const std::string &prepend, const std::string &append);

    // 静态方法获取 env，格式化为 "key=value"
    static void getBuildEnv(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& env);

    // 静态方法获取 configOpts
    static void getBuildConfigOpts(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& opts);

    // 静态方法获取 secretOpts
    static void getBuildSecretOpts(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& opts);

    // 静态方法获取 secretEnv，格式化为 "key=value"
    static void getBuildSecretEnv(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& env);
    

    bool m_strip = false;
    bool m_noDebuginfo = false;
    bool m_noDebuginfoCompression = false;
    string m_cFlags = "";
    bool m_cflagsOverride = false;
    string m_cppFlags = "";
    bool m_cppFlagsOverride = false;
    string m_cxxFlags = "";
    bool m_cxxFlagsOverride = false;
    string m_ldFlags = "";
    bool m_ldFlagsOverride = false;
    string m_appendPath = "";
    string m_prependPath = "";
    string m_appendLdLibraryPath = "";
    string m_prependLdLibraryPath = "";
    string m_appendPkgConfigPath = "";
    string m_prependPkgConfigPath = "";
    string m_prefix = "";
    string m_libDir = "";
    std::vector<string> m_buildArgs;
    std::vector<string> m_testArgs;
    std::vector<string> m_configOpts;
    std::vector<string> m_secretOpts;
    std::vector<string> m_makeArgs;
    std::vector<string> m_makeInstallArgs;
    map<string, string> m_secretEnv;
    map<string, string> m_env;
};

#endif // !__BUILDER_OPTIONS_H__