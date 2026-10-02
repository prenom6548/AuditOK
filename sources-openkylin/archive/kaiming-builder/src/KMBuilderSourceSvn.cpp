/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceSvn.h"
#include "KMSourceProcess.h"

REGISTER_SOURCE_TYPE_DYNCREATE(svn, KMBuilderSourceSvn)

class KMBuilderSourceSvn::Private
{
public:
    std::string m_url;
    std::string m_revision;
    std::string m_username;
    std::string m_password;
    std::string m_tag;
    std::string m_path;
    std::string m_depth;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceSvn::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // svn存储库的URL，包括分支/标记部分
    m_ptr->m_url = helper.getStringValue("url");
    // 要使用的特定修订号
    m_ptr->m_revision = helper.getStringValue("revision");

    // 下面是原开发人员私自添加的属性，规范中没有，也不可能会出现将用户名、密码写到yaml配置文件中的现象
    m_ptr->m_username = helper.getStringValue("username");
    m_ptr->m_password = helper.getStringValue("password");
    m_ptr->m_tag = helper.getStringValue("tag");
    m_ptr->m_path = helper.getStringValue("path");
    m_ptr->m_depth = helper.getStringValue("depth");

    // 校验
    if (m_ptr->m_url.empty())
    {
        throw KMException("In manifest file, 'svn' type source must config 'url'");
    }
}

void KMBuilderSourceSvn::toYaml(YAML::Node& sourceNode)
{
    sourceNode["type"] = "svn";
    if (!m_ptr->m_path.empty())
    {
        sourceNode["path"] = m_ptr->m_path;
    }
    if (!m_ptr->m_url.empty())
    {
        sourceNode["url"] = m_ptr->m_url;
    }
    if (!m_ptr->m_tag.empty())
    {
        sourceNode["tag"] = m_ptr->m_tag;
    }
    if (!m_ptr->m_username.empty())
    {
        sourceNode["username"] = m_ptr->m_username;
    }
    if (!m_ptr->m_password.empty())
    {
        sourceNode["password"] = m_ptr->m_password;
    }
    if (!m_ptr->m_depth.empty())
    {
        sourceNode["depth"] = m_ptr->m_depth;
    }
    if (!m_ptr->m_revision.empty())
    {
        sourceNode["revision"] = m_ptr->m_revision;
    }
}

KMBuilderSourceSvn::KMBuilderSourceSvn()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    this->m_type = SRC_Svn;
}

KMBuilderSourceSvn::~KMBuilderSourceSvn()
{
}

std::string KMBuilderSourceSvn::generateSvnCommand(std::string path)
{
    std::ostringstream command;

    command << "svn checkout ";

    if (!this->m_ptr->m_revision.empty())
    {
        command << "-r " << this->m_ptr->m_revision << " ";
    }

    if (!this->m_ptr->m_username.empty() && !this->m_ptr->m_password.empty())
    {
        command << "--username " << this->m_ptr->m_username << " --password " << this->m_ptr->m_password << " ";
    }

    if (!this->m_ptr->m_depth.empty())
    {
        command << "--depth " << this->m_ptr->m_depth << " ";
    }

    command << this->m_ptr->m_url;

    if (!path.empty())
    {
        command << " " << path;
    }

    return command.str();
}

bool KMBuilderSourceSvn::processNode(std::string path)
{
    std::string svnOrder = generateSvnCommand(path);
    return executeShellScript(svnOrder);
}
