/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderSourceArchive.h"

// #include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "KMBuilderContext.h"
#include "KMSourceProcess.h"
#include "common/KMLogger.h"

REGISTER_SOURCE_TYPE_DYNCREATE(archive, KMBuilderSourceArchive)

namespace fs = std::filesystem;

class KMBuilderSourceArchive::Private
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
    bool m_gitInit;
    std::string m_archiveType;
    uint32_t m_stripComponents = 0; // to dimplify the directorys  ,todo daiyouqi
};

/**
 * @brief : 需要子类实现自身的反序列化
 * @param : [in]，sourceNode，清单中的源节点
 */ 
void KMBuilderSourceArchive::deserializeEx(YAML::Node& sourceNode)
{
    KMYAMLNodeHelper helper(sourceNode);

    // 存档的路径
    m_ptr->m_path = helper.getStringValue("path");
    // 将要下载的远程存档的URL。如果同时指定了path，则此选项将覆盖path
    m_ptr->m_url = helper.getStringValue("url");
    // 下载后验证的文件的md5校验和; 请注意，md5不再被视为安全校验和，我们建议您至少使用sha256
    m_ptr->m_md5 = helper.getStringValue("md5");
    // 下载后验证的文件的sha1校验和；请注意，sha1不再被视为安全校验和，我们建议您至少使用sha256
    m_ptr->m_sha1 = helper.getStringValue("sha1");
    // 文件的sha256校验和，下载后验证
    m_ptr->m_sha256 = helper.getStringValue("sha256");
    // 文件的sha512校验和，下载后验证
    m_ptr->m_sha512 = helper.getStringValue("sha512");
    // 下载文件的文件名为，默认为url的basename
    m_ptr->m_destFilename = helper.getStringValue("dest-filename");
    
    // 下面是未启用属性
    // 如果主url失败，则使用的替代url的列表
    m_ptr->m_mirrorUrls = helper.getStringListValue("mirror-urls");
    // 提取过程中要剥离的初始路径名组件的数量。默认值为1
    m_ptr->m_stripComponents = helper.getIntValue("strip-components", 1);
    // 是否将存储库初始化为git存储库
    m_ptr->m_gitInit = helper.getBoolValue("git-init");
    // 存档的类型（如果无法从路径中猜到）
    m_ptr->m_archiveType = helper.getStringValue("archive-type");

    // 校验
    if (m_ptr->m_path.empty() && m_ptr->m_url.empty())
    {
        throw KMException("In manifest file, 'archive' type source at least config one of 'url' or 'path'");
    }

    if (getChecksum().empty())
    {
        throw KMException("In manifest file, 'archive' type source at least config one of 'sha256' or 'sha512'");
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
}

KMBuilderSourceArchive::KMBuilderSourceArchive()
    : KMBuilderSource()
    , m_ptr(std::make_shared<Private>())
{
    m_type = SRC_Archive;
}

KMBuilderSourceArchive::~KMBuilderSourceArchive() { }

std::string KMBuilderSourceArchive::getChecksum()
{
    return m_ptr->m_sha256 != "" ? m_ptr->m_sha256 : m_ptr->m_sha512 != "" ? m_ptr->m_sha512 : m_ptr->m_sha1 != "" ? m_ptr->m_sha1 : m_ptr->m_md5 != "" ? m_ptr->m_md5 : "";
}

std::string KMBuilderSourceArchive::getUrl()
{
    return m_ptr->m_url;
}

bool KMBuilderSourceArchive::processNode(std::string path)
{
    throw KMException("Please use function : downloadSource");
}

// 源码下载，用于替换方法processNode的，后续有时间再彻底优化
bool KMBuilderSourceArchive::downloadSource(const std::string &destDir)
{
    if (!m_command.empty())
    {
        setCommand(destDir);
    }
    // url的优先级高
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

        m_unzipDestination = destDir;
        m_unzipSource = destFile;

        return true;
    }
    else if (!m_ptr->m_path.empty())
    {
        std::string localSource = KMBuilderUtils::buildFilename(m_baseDir, m_ptr->m_path);
        std::string resultMessage = checkSum(m_ptr->m_sha512, m_ptr->m_sha256, m_ptr->m_sha1, m_ptr->m_md5, localSource);
        if (!resultMessage.empty())
        {
            throw KMException("Failed to checksum file : " + localSource);
        }

        m_unzipDestination = destDir;
        m_unzipSource = localSource;
        
        return true;
    }

    return false;
}

bool KMBuilderSourceArchive::unzipSource()
{
    if (!unZip(m_unzipDestination, m_unzipSource))
        return false;
        
    if (!m_command.empty())
    {
        executeCommand();
    }
    return true;
}

void KMBuilderSourceArchive::toYaml(YAML::Node &sourceNode)
{
    sourceNode["type"] = "archive";
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