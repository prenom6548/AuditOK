/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABMETADATA_H__
#define __KMOABMETADATA_H__

#include "KMJsonHelper.h"
#include "KMInfoJson.h"

#define KMOABMETADATA_PREFIX "OAB"

#define BUNDLE_FILESYSTE_EROFS "erofs"
#define BUNDLE_FILESYSTE_SQUASHFS "squashfs"

class KMOABMetadata
{
public:
    typedef struct Section
    {
        // 只读文件系统名称，形如：{prefix}.bundle
        std::string bundle;

        // 图标文件段名称，形如：{prefix}.icon
        std::string icon;

        // 额外元信息文件段名称，形如：{prefix}.extra-meta
        std::string extra_meta;

    } Section;

public:
    KMOABMetadata() = default;
    ~KMOABMetadata() = default;

    void parse(const std::string &jsonContent);

    std::string dump();

     /**
     * @brief : 保存json对象到文件
     * @param : [in] jsonFile, json文件
     * @throw : KMException
     */ 
    void saveFile(const std::string& jsonFile);

private:
    void fromJson();

    void toJson();

public:
    // meta属性域

    // meta文件的版本号，默认值为“1”
    std::string version = "1";

    // 该elf文件包含的额外字段清单
    Section sections;

    // 创建OK文件时指定的UUID，用于识别该OK包
    std::string uuid;

    // bundle段的hash，采用sha256算法
    std::string digest;

    // bundle段包含的组件（layer）信息
    std::vector<KMInfoJson> layers;

    // bundle的文件系统类型
    std::string bundleFilesystem;

private:
    // 数据成员域
    nlohmann::json m_root;
};

#endif // !__KMOABMETADATA_H__
