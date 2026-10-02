/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceShell.h"
#include "KMBuilderSource.h"
#include "KMBuilderModule.h"
#include "KMBuilderContext.h"
#include "common/KMLogger.h"
#include "KMBuildOptions.h"
#include "common/KMProcessUtils.h"

REGISTER_SOURCE_TYPE_DYNCREATE(shell, KMBuilderSourceShell)

class KMBuilderSourceShell::Private
{
public:
    std::vector<std::string> m_commands;
};


/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceShell::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // shell命令数组
    m_ptr->m_commands = helper.getStringListValue("commands");

    // 校验
    if (m_ptr->m_commands.empty())
    {
        throw KMException("In manifest file, 'shell'  type source must config 'commands'");
    }
}

KMBuilderSourceShell::KMBuilderSourceShell()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    this->m_type = SRC_Shell;
}

/**
 * @brief 运行给定脚本的逻辑
 * @param context 上下文参数
 * @param buildOptions 构建选项
 * @param sourceDir 源文件目录
 * @param commands 脚本命令
 * @return 执行成功与否
 */

bool runScript(const std::shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir, const string command)
{
    try
    {
        std::shared_ptr<KMBuildOptions> buildOptions = pBuildModule->getBuildOptions();
        std::vector<std::string> buildArgs, env;
        KMBuildOptions::getBuildArgs(buildOptions, buildArgs);
        KMBuildOptions::getBuildEnv(buildOptions, env);
        //构建环境和参数
        std::vector<std::string> args=
        {
            "build"
        };

        //加入环境变量到args中
        for (const auto& envVar : env)
        {
            args.push_back("--env=" + envVar);
        }

        args.insert(args.end(), buildArgs.begin(), buildArgs.end());

        std::string appDir = builderContext.getAppDir();//确定路径
        args.push_back(appDir);
        args.push_back("/bin/bash");
        args.push_back("--");
        args.push_back("-c");
        args.push_back(command);

        std::string result;
        if (!KMProcessUtils::spawn("kaiming", sourcedir, args, result))
        {
            KMError(result);
            return false;
        }
        else
        {
            KMDebug(result);
        }

        return true;
    } 
    catch (const std::exception &e)
    {
        KMError(e.what());
        return false;
    }
    
    
}

KMBuilderSourceShell::~KMBuilderSourceShell()
{
}

bool KMBuilderSourceShell::ensureSource()
{
    return false;
}

bool KMBuilderSourceShell::extractShell(const std::shared_ptr<KMBuilderModule> &pBuildModule, const string &sourcedir)
{
    for(const auto& command : m_ptr->m_commands)
    {
        if(!runScript(pBuildModule,sourcedir,command))
        {
            return false;
        }
    }
    
    return true;
}

bool KMBuilderSourceShell::processNode(std::string path)
{
    return true;
}

void KMBuilderSourceShell::toYaml(YAML::Node& sourceNode)
{
    sourceNode["type"] = "shell";
    if (!m_ptr->m_commands.empty())
    {
        KMBuilderUtils::vectorToYaml(sourceNode, "commands", m_ptr->m_commands);
    }
}