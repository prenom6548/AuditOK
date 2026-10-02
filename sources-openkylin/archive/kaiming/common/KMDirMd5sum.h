/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMDIRMD5SUM_H__
#define __KMDIRMD5SUM_H__

#include <string>

class KMDirMd5sum
{
public:
    KMDirMd5sum(const std::string& workingDirectory);
    ~KMDirMd5sum();

    /**
     * @brief : 对工作目录的某子目录求md5sum
     * @param : subDir，子目录名，不带路径
     */ 
    void md5sum(const std::string& subDir);

    /**
     * @brief : 对工作目录的某子目录校验md5值
     * @param : subDir，子目录名，不带路径
     */
    bool checksum(const std::string& subDir);

private:
    std::string m_workingDirectory; // 工作目录
};

#endif // !__KMDIRMD5SUM_H__
