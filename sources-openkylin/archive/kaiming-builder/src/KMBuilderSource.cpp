/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSource.h"
#include "KMBuilderContext.h"
#include "common/KMLogger.h"

/**
 * @brief : 源工厂方法，根据源类型创建对应的源类型对象
 * @param : [in]，sourceType 源类型
 * @return: std::shared_ptr<KMBuilderSource>，根据源类型创建对应的源类型对象或空
 */ 
std::shared_ptr<KMBuilderSource> KMBuilderSource::createSource(const std::string &sourceType)
{
    KMBuilderSource *pObjetct = dynamic_cast<KMBuilderSource *>(KMDynamicCreator::createObjectById("SourceType_" + sourceType));
    return std::shared_ptr<KMBuilderSource>(pObjetct);
}

/**
 * @brief : 反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSource::deserialize(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    m_dest = helper.getStringValue("dest");
    m_onlyArches = helper.getStringListValue("only-arches");
    m_skipArches = helper.getStringListValue("skip-arches");
    m_command = helper.getStringValue("command");

    deserializeEx(sourceNode);
}

KMBuilderSource::KMBuilderSource()
{
}

KMBuilderSource::~KMBuilderSource()
{
}

void KMBuilderSource::setBaseDir(const std::string &baseDir)
{
    m_baseDir = baseDir;
}

std::string KMBuilderSource::getBaseDir() const
{
    return m_baseDir;
}

std::string KMBuilderSource::getDest() const
{
    return m_dest;
}

std::vector<std::string> KMBuilderSource::getOnlyArches() const
{
    return m_onlyArches;
}

std::vector<std::string> KMBuilderSource::getSkipArches() const
{
    return m_skipArches;
}

std::string KMBuilderSource::getCommand() const
{
    return m_command;
}

void KMBuilderSource::setCommand(std::string path)
{
    std::srand(std::time(nullptr));
    int randomNumber = std::rand() % 10000;
    std::string autoFileName = "command_" + std::to_string(randomNumber) + ".sh";

    m_scriptFile = KMBuilderUtils::buildFilename(path, autoFileName);

    std::ofstream outfile(m_scriptFile);
    if (!outfile.is_open())
    {
        throw KMException("Failed to open file for writing command");
    }

    //写入脚本头部
    outfile << "#!/bin/bash\n";

    //写入命令
    outfile << m_command << "\n";
    outfile.close();
    
    if (chmod(m_scriptFile.c_str(), 0755) != 0)    
    {        
        throw KMException("Failed to set source command");   
    }
}

void KMBuilderSource::executeCommand()
{
    if (!m_scriptFile.empty())
    {
        KMDebug("Executing command: \"" + m_command + "\" in script file: \"" + m_scriptFile + "\"");
        int ret = std::system(m_scriptFile.c_str());
        if (ret != 0)
        {
            throw KMException("Source Command execution failed with code" + ret);
        }
    }
}

bool KMBuilderSource::isEnable()
{
    if (!m_onlyArches.empty() && !KMBuilderUtils::contains(m_onlyArches, builderContext.getArch()))
    {
        return false;
    }

    if (!m_skipArches.empty() && KMBuilderUtils::contains(m_skipArches, builderContext.getArch()))
    {
        return false;
    }

    return true;
}

bool KMBuilderSource::downloadSource(const std::string &destDir)
{
    return processNode(destDir);
}

