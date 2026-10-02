/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceDir.h"
#include "KMBuilderUtils.h"
#include "KMSourceProcess.h"

namespace fs = std::filesystem;

REGISTER_SOURCE_TYPE_DYNCREATE(dir, KMBuilderSourceDir)

class KMBuilderSourceDir::Private
{
public:
    std::string m_path;
    std::vector<std::string> m_skips;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceDir::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // 本地目录的路径，其内容将被复制到源目录中。请注意，目录源当前不支持缓存，因此每次都会重新生成
    m_ptr->m_path = helper.getStringValue("path");
    // 目录中要忽略的源文件
    m_ptr->m_skips = helper.getStringListValue("skip");

    // 校验
    if (m_ptr->m_path.empty())
    {
        throw KMException("n manifest file, 'dir' type source must config 'path'");
    }

    m_ptr->m_skips.push_back("kaiming");
    m_ptr->m_skips.push_back("kaiming-bundles");
    m_ptr->m_skips.push_back(".kaiming-builder");
}

KMBuilderSourceDir::KMBuilderSourceDir()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    m_type = SRC_Dir;
}

KMBuilderSourceDir::~KMBuilderSourceDir()
{
}

bool KMBuilderSourceDir::ensureSource()
{
    return false;
}

bool KMBuilderSourceDir::processNode(std::string path)
{
    if (!m_ptr->m_path.empty())
    {
        std::string sourcePath = KMBuilderUtils::buildFilename(m_baseDir, m_ptr->m_path);
        //找不到文件dir
        if (!fileCopy(sourcePath, path, m_ptr->m_skips))
        {
            std::string module_name = KMBuilderUtils::getModuleName(path);
            throw KMException("Failed to download sources: component " + module_name + ": Unable to get source file \'" + m_ptr->m_path + "\': File " + sourcePath + " not found");
        }

        if (!m_command.empty())
        {
            setCommand(path);
            executeCommand();
        }
        return true;
    }

    return false;
}

void KMBuilderSourceDir::toYaml(YAML::Node &sourceNode)
{
    sourceNode["type"] = "dir";
    if (!m_ptr->m_path.empty())
    {
        sourceNode["path"] = m_ptr->m_path;
    }
    if (!m_ptr->m_skips.empty())
    {
        KMBuilderUtils::vectorToYaml(sourceNode, "skip", m_ptr->m_skips);
    }
}