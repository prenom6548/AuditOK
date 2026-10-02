/*
 * @Author: caoxiansheng caoxiansheng@kylinos.cn
 * @Date: 2025-03-28 09:33:31
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-03-31 17:38:39
 * @FilePath: /study/home/cc/work/kaiming/app/which/KMWhich.h
 * @Description: 
 * 
 * Copyright (c) by KylinSoft  Co., Ltd. 2025. All Rights Reserved.
 */

#ifndef __KMWHICH_H_
#define __KMWHICH_H_

#include "common/KMSubCommand.h"
#include "common/KMEnv.h"
#include "common/KMFolder.h"
#include "common/KMBuildinOptions.h"

class KMWhich : public KMSubCommand
{
    DECLARE_DYNCREATE(KMWhich);

public:
    KMWhich();
    ~KMWhich() override;

    int dispose(int argc, char **argv) override;

private:
    /**
     * @brief: 扫描指定目录查询指定文件包名
     * @param {string} &infoDir  缓存文件路径
     * @param {string} &filePath 待检测文件路径
     * @return {*}
     */
    void searchPackageByFile(const std::string &infoDir, const std::string &filePath);

    /**
     * @brief: 检查文件路径是否在.list文件中
     * @param {path} &listFilePath 匹配的文件路径
     * @param {string} &filePath   待检测文件路径
     * @return {*} true: 找到 false: 没有找到
     */
    bool fileExistsInListFile(const fs::path &listFilePath, const std::string &searchFilePath);

    /**
     * @brief: 规范化路径字符串 
     * @param {string} &path 待处理的路径
     * @return {*} 返回规范化后的路径字符串
     */
    std::string normalizePath(const std::string &path);

private:
    class Options;
    class Private;
    std::shared_ptr<Private> d;
};

#endif //__KMWHICH_H_