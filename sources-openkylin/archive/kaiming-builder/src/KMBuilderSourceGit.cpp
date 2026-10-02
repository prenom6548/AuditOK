/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceGit.h"
#include "KMBuilderContext.h"
#include "KMSourceProcess.h"
#include "common/KMProcessUtils.h"

REGISTER_SOURCE_TYPE_DYNCREATE(git, KMBuilderSourceGit)

class KMBuilderSourceGit::Private
{
public:
    std::string m_url;
    std::string m_path;
    std::string m_branch;
    std::string m_tag;
    std::string m_commit;
    bool m_disableFsckobjects;
    bool m_disableShallowClone;
    bool m_disableSubmodules;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceGit::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // git存储库的URL。如果同时指定了路径，则此选项将覆盖路径。通过SSH使用git时，正确的语法为ssh://user@domain/path/to/repo.git
    m_ptr->m_url = helper.getStringValue("url");
    // git存储库中要使用的分支
    m_ptr->m_branch = helper.getStringValue("branch");
    // 要从git存储库中使用的标记
    m_ptr->m_tag = helper.getStringValue("tag");
    // git存储库中要使用的提交
    m_ptr->m_commit = helper.getStringValue("commit");

    // 下面是未启用属性
    // git存储库的本地签出路径。由于git-clone的工作方式，这将比指定url=file:///...更快
    m_ptr->m_path = helper.getStringValue("path");
    // 不要使用transfer.fsckObjects=1来镜像git存储库。对于某些（损坏的）存储库，可能需要这样做
    m_ptr->m_disableFsckobjects = helper.getBoolValue("disable-fsckobjects");
    // 下载git-repo时不要通过制作浅层克隆进行优化
    m_ptr->m_disableShallowClone = helper.getBoolValue("disable-shallow-clone");
    // 克隆存储库时不要签出git子模块
    m_ptr->m_disableSubmodules = helper.getBoolValue("disable-submodules");

    // 校验
    if (m_ptr->m_url.empty())
    {
        throw KMException("In manifest file, 'git' type source at least config one of 'url'");
    }
}

void KMBuilderSourceGit::toYaml(YAML::Node &sourceNode)
{
    sourceNode["type"] = "git";
    if (!m_ptr->m_url.empty())
    {
        sourceNode["url"] = m_ptr->m_url;
    }
    if (!m_ptr->m_branch.empty())
    {
        sourceNode["branch"] = m_ptr->m_branch;
    }
    if (!m_ptr->m_tag.empty())
    {
        sourceNode["tag"] = m_ptr->m_tag;
    }
    if (!m_ptr->m_commit.empty())
    {
        sourceNode["commit"] = m_ptr->m_commit;
    }
}

KMBuilderSourceGit::KMBuilderSourceGit()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    m_type = SRC_Git;
}

KMBuilderSourceGit::~KMBuilderSourceGit()
{
}

bool KMBuilderSourceGit::processNode(std::string path)
{
    if (!m_command.empty())
    {
        setCommand(path);     
    }
    std::string exec = "git";
    std::vector<std::string> bareCloneArgs;
    std::string result;
    m_gitUnzipPath = path;

    std::srand(std::time(nullptr));
    int randomNumber = std::rand();
    std::string repoName = "repository_" + std::to_string(randomNumber) + ".git";
    std::string intermediateDir = builderContext.getDownloadsDir();
    KMBuilderUtils::mkpath(intermediateDir);
    m_gitDownloadRepo = intermediateDir + "/" + repoName;
    KMBuilderUtils::rmRfAll(m_gitDownloadRepo);

    bareCloneArgs = {"clone","--bare"};
    if (!m_ptr->m_branch.empty())
    {
       bareCloneArgs.push_back("--branch");
       bareCloneArgs.push_back(m_ptr->m_branch);
    }
    bareCloneArgs.push_back(m_ptr->m_url);
    bareCloneArgs.push_back(repoName);
    
    bool returnValue = KMProcessUtils::spawn(exec, intermediateDir, bareCloneArgs, result);
    return returnValue;
}

bool KMBuilderSourceGit::unzipSource()
{
    std::string exec = "git";
    std::vector<std::string> args;
    std::string result;
  
    args = {"clone", m_gitDownloadRepo};

    std::string tmpGitPath = m_gitUnzipPath + "/git";
    KMBuilderUtils::mkpath(tmpGitPath);
    args.push_back(tmpGitPath);
    
    if (!KMProcessUtils::spawn(exec, args, result))
    {
        return false;
    }
    
    args.clear();
    if (!m_ptr->m_commit.empty())
    {
        args = {"checkout", m_ptr->m_commit};
    }
    else if (!m_ptr->m_tag.empty())
    {
        args = {"checkout", "tags/" + m_ptr->m_tag};
    }
    
    if (!args.empty() && !KMProcessUtils::spawn(exec, tmpGitPath, args, result))
    {
        return false;
    }

    bool returnValue = fileCopy(tmpGitPath, m_gitUnzipPath);
    if (!returnValue)
    {
        return false;
    }

    args = {"-rf", "git"};
    if (!KMProcessUtils::spawn("rm", m_gitUnzipPath, args, result))
    {
        return false;
    }
    
    if (!m_command.empty())
    {
        executeCommand();
    }
    
    return true;
}

/**
 * @note : TODO：后续优化
 *        1、多次构建会在.kaiming-builder目录中下载多份git仓库
 *        2、路径中不能有空格
 */ 

