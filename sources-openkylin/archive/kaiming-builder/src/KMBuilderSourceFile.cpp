/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceFile.h"
#include "KMBuilderUtils.h"
#include "KMSourceProcess.h"
#include "KMBuilderContext.h"

REGISTER_SOURCE_TYPE_DYNCREATE(file, KMBuilderSourceFile)

class KMBuilderSourceFile::Private
{
public:
    std::string m_path;
    std::string m_url;
    std::vector<std::string> m_mirrorUrls;
    std::string m_md5;
    std::string m_sha1;
    std::string m_sha256;
    std::string m_sha512;
    std::string m_destFilename;
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceFile::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // 将被复制到源目录中的本地文件的路径
    m_ptr->m_path = helper.getStringValue("path");
    // 将下载并复制到源目录中的远程文件的URL。如果同时指定了path，则会覆盖path
    m_ptr->m_url = helper.getStringValue("url");
    // 下载后验证的文件的md5校验和; 请注意，md5不再被视为安全校验和，我们建议您至少使用sha256
    m_ptr->m_md5 = helper.getStringValue("md5");
    // 下载后验证的文件的sha1校验和；请注意，sha1不再被视为安全校验和，我们建议您至少使用sha256
    m_ptr->m_sha1 = helper.getStringValue("sha1");
    // 文件的sha256校验和，下载后验证
    m_ptr->m_sha256 = helper.getStringValue("sha256");
    // 文件的sha512校验和，下载后验证
    m_ptr->m_sha512 = helper.getStringValue("sha512");
    // 要在源目录中使用的文件名，默认为路径的basename
    m_ptr->m_destFilename = helper.getStringValue("dest-filename");
    
    // 下面是未启用属性
    // 如果主url失败，则使用的替代url的列表
    m_ptr->m_mirrorUrls = helper.getStringListValue("mirror-urls");

    // 校验
    if (m_ptr->m_path.empty() && m_ptr->m_url.empty())
    {
        throw KMException("In manifest file, 'file' type source at least config one of 'url' or 'path'");
    }

    if(!m_ptr->m_url.empty())
    {
        if (getChecksum().empty())
        {
            throw KMException("In manifest file, 'file' type source at least config one of 'sha256' or 'sha512'");
        }
    }
    
    // 如果未设置，默认为url的basename
    if (m_ptr->m_destFilename.empty())
    {
        std::string file = m_ptr->m_url.empty() ? m_ptr->m_path : m_ptr->m_url;
        std::size_t pos = file.find_last_of('/');
        if (pos == std::string::npos)
        {
            m_ptr->m_destFilename = file;
        }
        else
        {
            m_ptr->m_destFilename = file.substr(pos + 1);
        }
    }

    if(m_ptr->m_destFilename.find("/")!=std::string::npos)
    {
        throw KMException("The dest-filename cannot contain '/'");
    }
}

KMBuilderSourceFile::KMBuilderSourceFile()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    m_type = SRC_File;
}

KMBuilderSourceFile::~KMBuilderSourceFile()
{
}
std::string KMBuilderSourceFile::getChecksum()
{
     return m_ptr->m_sha256 != "" ? m_ptr->m_sha256 : m_ptr->m_sha512 != "" ? m_ptr->m_sha512 : m_ptr->m_sha1 != "" ? m_ptr->m_sha1 : m_ptr->m_md5 != "" ? m_ptr->m_md5 : "";
}

bool KMBuilderSourceFile::processNode(std::string path)
{  
    if (!m_ptr->m_url.empty())
    {
        std::string destFile = KMBuilderUtils::buildFilename(builderContext.getDownloadsDir(), getChecksum(), m_ptr->m_destFilename);

        if (!KMBuilderUtils::downloadUrl(m_ptr->m_url, destFile))
        {
            throw KMException("Failed to download file : " + m_ptr->m_url);
        }

        std::string resultMessage = checkSum(m_ptr->m_sha512, m_ptr->m_sha256, m_ptr->m_sha1, m_ptr->m_md5, destFile);
        if (!resultMessage.empty())
        {
            throw KMException("Failed to checksum file : " + m_ptr->m_url);
        }

        bool filecopyResult = fileCopy(destFile, path);

        if (!m_command.empty())
        {
            setCommand(path);
            executeCommand();
        }

        return filecopyResult;  
    }
    else if (!m_ptr->m_path.empty())
    {
        std::string sourcePath = KMBuilderUtils::buildFilename(m_baseDir, m_ptr->m_path);
        std::string targetPath = KMBuilderUtils::buildFilename(path,m_ptr->m_destFilename);

        //找不到文件file
        if (!fileCopy(sourcePath, targetPath))
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

void KMBuilderSourceFile::toYaml(YAML::Node& sourceNode)
{
    sourceNode["type"] = "file";
    if (!m_ptr->m_path.empty())
    {
        sourceNode["path"] = m_ptr->m_path;
    }
    if (!m_ptr->m_url.empty())
    {
        sourceNode["url"] = m_ptr->m_url;
    }
    if (!m_ptr->m_sha256.empty())
    {
        sourceNode["sha256"] = m_ptr->m_sha256;
    }
    if (!m_ptr->m_sha512.empty())
    {
        sourceNode["sha512"] = m_ptr->m_sha512;
    }
    if (!m_ptr->m_destFilename.empty())
    {
        sourceNode["dest-filename"] = m_ptr->m_destFilename;
    }
}