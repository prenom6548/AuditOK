/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildOptions.h"
#include "KMBuilderContext.h"


using namespace std;

/**
 * @brief: 获取编译参数
 * @param: options
 * @param: args编译参数
 */
void KMBuildOptions::getBuildArgs(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& args) 
{
    if (!options)
    {
        return;
    }

    for (const auto& arg : options->m_buildArgs) 
    {
        args.emplace_back(arg);
    }
}

/**
 * @brief : 处理环境变量中路径变量的前面插入和后面追加
 * @param : [in | out] envs，环境变量map
 * @param : [in] key, 路径类变量的key，如：PATH
 * @param : [in] prepend, 前面追加
 * @param : [in] append, 后面追加
 */ 
void KMBuildOptions::concatEnvironmentPath(std::map<std::string, std::string> &envs, const std::string &key, const std::string &prepend, const std::string &append)
{
    std::string value = envs[key];
    if (value.empty())
    {
        value = "$" + key;
    }
    
    if (!append.empty())
    {
        value.append(":" + append);
    }
    
    if (!prepend.empty())
    {
        value = prepend + ":" + value;
    }

    if (value != ("$" + key))
    {
        envs[key] = value;
    }
    else
    {
        envs.erase(key);
    }
}

/**
 * @brief: 获取环境变量
 * @param: KMBuildOptions
 * @param: env编译环境变量
 */
void KMBuildOptions::getBuildEnv(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& env) 
{
    std::map<std::string, std::string> envs = builderContext.manifestConfig()->m_envs;
    if (options)
    {
        for (auto [k, v] : options->m_env)
        {
            envs[k] = v;
        }

        if (!options->m_cFlags.empty())
        {
            env.emplace_back("CFLAGS=" + options->m_cFlags);
        }
        if (!options->m_cppFlags.empty())
        {
            env.emplace_back("CPPFLAGS=" + options->m_cppFlags);
        }
        if (!options->m_cxxFlags.empty())
        {
            env.emplace_back("CXXFLAGS=" + options->m_cxxFlags);
        }
        if (!options->m_ldFlags.empty())
        {
            env.emplace_back("LDFLAGS=" + options->m_ldFlags);
        }

        concatEnvironmentPath(envs, "PATH", options->m_prependPath, options->m_appendPath);
        concatEnvironmentPath(envs, "LD_LIBRARY_PATH", options->m_prependLdLibraryPath, options->m_appendLdLibraryPath);
        concatEnvironmentPath(envs, "PKG_CONFIG_PATH", options->m_prependPkgConfigPath, options->m_appendPkgConfigPath);
    }

    std::string ccachedir = "/run/ccache/disabled";
    if (builderContext.isCcache())
    {
        ccachedir = "/run/ccache";
        std::string old = envs["PATH"];
        if (old.empty())
        {
            old = "$PATH";
        }

        std::string path = std::string("/run/ccache/bin:") + old;
        envs["PATH"] = path;
    }
    env.emplace_back("CCACHE_DIR=" + ccachedir);

    for (auto [k, v] : envs)
    {
        env.emplace_back(k + "=" + v);
    }
}

/**
 * @brief: 获取配置选项
 * @param: BuildOptions编译选项
 * @param: 返回的配置选项
 */
void KMBuildOptions::getBuildConfigOpts(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& opts) 
{
    if (!options)
    {
        return;
    }
    
    for (const auto& opt : options->m_configOpts) 
    {
        opts.emplace_back(opt);
    }
}

/**
 * @brief: 获取秘密选项
 * @param: mapEnv环境变量
 * @param: 返回的配置选项
 */
void KMBuildOptions::getBuildSecretOpts(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& opts) 
{
    if (!options)
    {
        return;
    }
    
    for (const auto& opt : options->m_secretOpts) 
    {
        opts.emplace_back(opt);
    }
}

/**
 * @brief: 获取加密环境变量
 * @param: BuildOptions编译选项
 * @param: env编译环境变量
 */
void KMBuildOptions::getBuildSecretEnv(const std::shared_ptr<KMBuildOptions>& options, std::vector<std::string>& env) 
{
    if (!options)
    {
        return;
    }
    
    for (const auto& [k, v] : options->m_secretEnv) 
    {
        env.emplace_back(k + "=" + v);
    }
}

