/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMDirMd5sum.h"
#include "KMProcessUtils.h"

KMDirMd5sum::KMDirMd5sum(const std::string& workingDirectory)
    : m_workingDirectory(workingDirectory)
{}

KMDirMd5sum::~KMDirMd5sum()
{}

/**
 * @brief : 对工作目录的某子目录求md5sum
 * @param : subDir，子目录名，不带路径
 */
void KMDirMd5sum::md5sum(const std::string& subDir)
{
    // 例：find files -type f | xargs -P 8 -I {} md5sum {} > files.md5
    std::string cmd("find ");
    if (subDir.empty())
    {
        cmd += " ./ -type f | xargs -P 8 -I {} md5sum -b {} > ";
        cmd += "checksum.md5";
    }
    else
    {
        cmd += subDir;
        cmd += " -type f | xargs -P 8 -I {} md5sum -b {} > ";
        cmd += subDir;
        cmd += ".md5";
    }

    KMProcessUtils::spawn("bash", m_workingDirectory, { "-c", cmd });
}

/**
 * @brief : 对工作目录的某子目录校验md5值
 * @param : subDir，子目录名，不带路径
 */
bool KMDirMd5sum::checksum(const std::string& subDir)
{
    std::string md5 = subDir + ".md5";
    if (subDir.empty())
    {
        md5 = "checksum.md5";
    }
    
    return KMProcessUtils::spawn("md5sum", m_workingDirectory, { "-c", md5 });
}
