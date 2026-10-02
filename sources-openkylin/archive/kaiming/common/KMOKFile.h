/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOKFile_H__
#define __KMOKFile_H__

#include <string>
#include <vector>
#include <iostream>
#include <elf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <libelf.h>
#include <memory>
#include <vector>

#include "KMOABMetadata.h"
#include "KMProcessUtils.h"

#define BUNDLE_FILESYSTE_EROFS "erofs"
#define BUNDLE_FILESYSTE_SQUASHFS "squashfs"

class KMOKFile
{
public:
    explicit KMOKFile();
    virtual ~KMOKFile();

    /**
     * @brief 加载 ok 文件
     * 
     * @param filePath 加载 ok 文件路径
     * @return true 加载成功
     * @return false 加载失败
     */
    bool loadOKFile(const std::string &filePath);

    /**
     * @description: 获取ok文件路径
     * @return {*}
     */
    std::string getOKFilePath();

    /**
     * @brief 关闭 ok 文件
     * 
     */
    void closeOKFile();

    /**
     * @brief 读取 ok 文件中的 header section
     * 
     * @param sectionName 读取的 section 名称
     * @return Elf_Scn* 读取到的 section 内容
     */
    Elf_Scn *getHeaderSection(const std::string &sectionName);

    /**
     * @brief 得到 meta section 内容
     * 
     * @param section 读取的 meta section
     * @return std::string 读取到的 meta 内容
     */
    std::string getMetaInfo(Elf_Scn *section);
    std::string getMetaInfo();

    /**
     * @brief 设置 mount point 路径
     * 
     * @param mountPoint 挂载点, 例如 /data/ok
     * @return std::string 挂载点路径
     */
    bool setMountPoint(const std::string &mountPoint);

    void setOKFilePath(const std::string &filePath);

    /**
     * @brief 得到 section 偏移量
     * 
     * @param section section 内容
     * @return Elf64_Off section 偏移量
     */
    Elf64_Off getSectionOffset(Elf_Scn *section);

    /**
     * @brief 挂载 ok 文件
     * 
     * @return true 挂载成功
     * @return false 挂载失败
     */
    bool mountOK(const std::string &strMountPath);

    /**
     * @brief : 挂载bundle
     * @param : [in], mountPath 挂载目录
     * @return: true-挂载成功；false-挂载失败
     * @note  : 
     *          1、和mountOK稍有区别，mountOK只能root用户使用；mountBundle是用pkexec给普通用户提权，从而普通用户也可以挂载
     *          2、需要和umountBundle卸载成对使用
     */
    bool mountBundle(const std::string& mountPath);

    /**
     * @brief  卸载 ok 的 bundle 镜像挂载路径
     * 
     * @return int 命令执行结果
     */
    int umountOK();

    /**
     * @brief  卸载 ok 的 bundle 镜像挂载路径
     * @param  mountPoint 卸载目录
     * @return int 命令执行结果
     */
    int umountOK(const std::string& mountPoint);

    /**
     * @brief : 卸载bundle
     * @param : [in], mountPath 卸载目录
     * @return: true-卸载成功；false-卸载失败
     * @note  : 
     *          1、和umountOK稍有区别，umountOK只能root用户使用；umountBundle是用pkexec给普通用户提权，从而普通用户也可以挂载
     *          2、需要和mountBundle成对使用
     */
    bool umountBundle(const std::string& mountPath);

    std::shared_ptr<KMOABMetadata> getMetadata();

    void clearMountPoint();

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMOKFile_H__
