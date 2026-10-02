/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourcePatch.h"
#include "KMSourceProcess.h"
#include "KMBuilderUtils.h"
#include "common/KMProcessUtils.h"

REGISTER_SOURCE_TYPE_DYNCREATE(patch, KMBuilderSourcePatch)

class KMBuilderSourcePatch::Private
{
public:
    std::vector<std::string> m_paths;
    int m_stripComponents = 1;
    std::string m_useGit;
    std::vector<std::string> m_options;
    std::string m_useGitAm;
    std::string m_path;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourcePatch::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // 将在源目录中应用的patch文件的路径
    m_ptr->m_path = helper.getStringValue("path");
    // 将在源目录中应用的patch文件的路径列表，按顺序
    m_ptr->m_paths = helper.getStringListValue("paths");
    // 要修补的-p参数的值，默认为1
    m_ptr->m_stripComponents = helper.getIntValue("strip-components", 1);
    // 当patch文件包含二进制diff时，是否使用“git-apply”而不是“patch”来应用修补程序
    m_ptr->m_useGit = helper.getBoolValue("use-git");
    // 当patch文件包含二进制diff时，是否使用“git-am”而不是“patch”来应用修补程序。您不能在使用git的同时使用它
    m_ptr->m_useGitAm = helper.getBoolValue("use-git-am");
    // 要传递给修补程序命令的额外选项
    m_ptr->m_options = helper.getStringListValue("options");

    // 校验
    if (m_ptr->m_path.empty() && m_ptr->m_paths.empty())
    {
        throw KMException("In manifest file, 'patch' type source at least config one of 'path' or 'paths'");
    }

    // 为了凑旧代码的逻辑添加的逻辑，下面processNode和apply函数中将m_ptr->m_path挪作了他用，代码太垃圾了，TODO：后续有时间时进行优化
    if (!m_ptr->m_path.empty())
    {
        m_ptr->m_paths.push_back(m_ptr->m_path);
    }
}

std::pair<std::string, std::vector<std::string>> applyPatch(const std::string &path, int stripComponents, const std::string &useGit, const std::vector<std::string> &options, const std::string &useGitAm)
{
    std::string exec;
    std::vector<std::string> args;

    if (useGitAm == "true")
    {
        // Use git am to apply the patch
        exec = "git";
        args = {"am", "--keep-cr"};
        args.insert(args.end(), options.begin(), options.end());
    }
    else if (useGit == "true")
    {
        // Use git apply to apply the patch
        exec = "git";
        args = {"apply", "-v"};
        args.insert(args.end(), options.begin(), options.end());
        args.push_back("-p" + std::to_string(stripComponents));
    }
    else
    {
        // Use patch command to apply the patch
        exec = "patch";
        args.insert(args.end(), args.begin(), args.end());
        args.push_back("-p" + std::to_string(stripComponents));
        args.push_back("-i");
    }
    args.push_back(path);
    return {exec,args};
}

KMBuilderSourcePatch::KMBuilderSourcePatch()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    m_type = SRC_Patch;
}

KMBuilderSourcePatch::~KMBuilderSourcePatch()
{
}

bool KMBuilderSourcePatch::apply()
{
    for (auto iter = m_ptr->m_paths.begin(); iter != m_ptr->m_paths.end(); iter++)
    {
        std::string patchPath = KMBuilderUtils::buildFilename(getBaseDir(), *iter);
        auto [exec, patchArgs] = applyPatch(patchPath, m_ptr->m_stripComponents, m_ptr->m_useGit, m_ptr->m_options, m_ptr->m_useGitAm);
        
        std::string result;
        if (!KMProcessUtils::spawn(exec, m_ptr->m_path, patchArgs, result))
        {
            return false;
        }
    }

    if (!m_command.empty())
    {
        executeCommand();
    }

    return true;
}

bool KMBuilderSourcePatch::processNode(std::string path)
{
    m_ptr->m_path = path;
    if (!m_command.empty())
    {
        setCommand(path);
    }
    return true;
}

void KMBuilderSourcePatch::toYaml(YAML::Node& sourceNode)
{
    sourceNode["type"] = "patch";
    if (m_ptr->m_path.empty())
    {
        sourceNode["path"] = m_ptr->m_path;
    }
    if (m_ptr->m_paths.empty())
    {
        KMBuilderUtils::vectorToYaml(sourceNode, "paths", m_ptr->m_paths);
    }
    sourceNode["strip-components"] = m_ptr->m_stripComponents;
}