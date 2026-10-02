/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceScript.h"
#include "KMSourceProcess.h"
#include "KMBuilderUtils.h"

REGISTER_SOURCE_TYPE_DYNCREATE(script, KMBuilderSourceScript)

class KMBuilderSourceScript::Private
{
public:
    std::vector<std::string> m_commands;
    std::string m_destFilename;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceScript::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // 将被放入shell脚本文件中的shell命令数组
    m_ptr->m_commands = helper.getStringListValue("commands");
    // 要在源目录中使用的文件名，默认为autogen.sh
    m_ptr->m_destFilename = helper.getStringValue("dest-filename", "autogen.sh");

    // 校验
    if (m_ptr->m_commands.empty())
    {
        throw KMException("In manifest file,  'script' type source must config 'commands'");
    }

    // 检查 m_destFilename 是否包含 '/'
    if (m_ptr->m_destFilename.find('/') != std::string::npos)    
    {   
        throw KMException("The dest-filename cannot contain '/'");    
    }

    
}

KMBuilderSourceScript::KMBuilderSourceScript()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    this->m_type = SRC_Script;
}

bool saveCmd(std::vector<std::string> commands, std::string fileName)
{
    try
    {
        fs::path filePath(fileName);
        fs::path parentPath = filePath.parent_path();
        if (!fs::exists(parentPath))
        {
            fs::create_directories(parentPath);
        }
        std::ofstream outfile(fileName);
        if (!outfile.is_open())
        {
            return false;
        }

        //写入脚本头部
        outfile << "#!/bin/bash\n";

        //写入命令
        for (const auto &command : commands)
        {
            outfile << command << std::endl;
        }
        outfile.close();
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

KMBuilderSourceScript::~KMBuilderSourceScript()
{
}

bool KMBuilderSourceScript::ensureSource()
{
    return false;
}

bool KMBuilderSourceScript::processNode(std::string path)
{
    std::string fileName = KMBuilderUtils::buildFilename(path, m_ptr->m_destFilename);

    if (!saveCmd(m_ptr->m_commands,fileName))
    {
        return false ;
    }

    //设置文件权限755
    try
    {
        fs::path filePath(fileName);
        if (fs::exists(filePath))
        {
            fs::permissions(filePath, 
                            fs::perms::owner_all | 
                            fs::perms::group_read | fs::perms::group_exec |
                            fs::perms::others_read | fs::perms::others_exec, 
                            fs::perm_options::replace);    
        }
        return true;
    } catch (const std::exception &e){
        return false;
    }
}

void KMBuilderSourceScript::toYaml(YAML::Node& sourceNode)
{
    sourceNode["type"] = "script";
    if (!m_ptr->m_commands.empty())
    {
        KMBuilderUtils::vectorToYaml(sourceNode, "commands", m_ptr->m_commands);
    }
    if (!m_ptr->m_destFilename.empty())
    {
        sourceNode["dest-filename"] = m_ptr->m_destFilename;
    }
}