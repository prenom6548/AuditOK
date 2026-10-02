/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABELF_H__
#define __KMOABELF_H__

#include <memory>
#include <string>
#include <libelf.h>
#include <gelf.h>

#include "common/KMOABMetadata.h"

class KMOABElf
{
public:
    KMOABElf() = default;
    KMOABElf(const std::string &elf);
    virtual ~KMOABElf();

    /**
     * @brief : 打开elf文件准备读取
     * @note  : 做其它操作前，需要首先调用此方法
     * @return: true-正常打开；false-打开失败
     */ 
    bool open();

    /**
     * @brief : 获取ok包的meta内容
     */ 
    std::string metaContent() const;
    KMOABMetadata metadata() const;

    /**
     * @brief : 解压bundle段的数据到目录dest
     * @param : [in] dest，解压到的目标目录
     * @return: true-解压成功; false-解压失败
     */ 
    // bool extractBundle(const std::string &dest);

    /**
     * @brief : 挂载bundle段到挂载点mountPoint
     * @param : [in] mountPoint，挂载点
     * @param : [in] isAutoUmount，是否在对象解析时自动卸载
     * @return: true-挂载成功; false-挂载失败
     */ 
    bool mountBundle(const std::string &mountPoint, bool isAutoUmount = true);

    /**
     * @brief : 卸载bundle段（前面已经使用mountBundle挂载）
     * @return: true-挂载成功; false-挂载失败
     */ 
    bool umountBundle();
    static bool umountBundle(const std::string &mountPoint);

    /**
     * @brief : 重新打包bundle，并更新当前ok文件的bundle段
     * @param : [in] bundleDir, bundle数据
     * @param : [in] newFilesystem, 如果传入，则打包bundle时使用新的文件系统类型
     * @return: true-成功; false-失败
     */ 
    bool repackageBundle(const std::string &bundleDir, const std::string &newFilesystem = "");

    std::string appDataDir() const;

    /**
     * @brief : 最近错误信息
     */ 
    std::string errorMsg() const;

private:
    /**
     * @brief : 根据名称获取段/节信息
     * @param : [in] sectionName, 段名
     * @return: Elf_Scn *, 短信息结构体指针；获取失败返回nullptr
     */
    Elf_Scn * getSection(const std::string &sectionName);

    /**
     * @brief : 根据名称获取段/节头信息
     * @param : [in] section, 段/节
     * @return: GElf_Shdr, 节头信息
     */
    GElf_Shdr getSectionHeader(Elf_Scn *section);

    /**
     * @brief : 根据名称获取段/节字符串类型的数据
     * @param : [in] sectionName, 段名
     * @param : [out] data, 段数据
     * @return: true-成功获取；false-失败
     */
    // bool getSectionData(const std::string &sectionName, std::string &data);
    bool getSectionData(Elf_Scn * scn, std::string &data);

    // bool extractErofsBundle(const std::string &dest);
    // bool extractSquashfsBundle(const std::string &dest);

    bool mountErofsBundle();
    bool mountSquashfsBundle();

    bool generateErofsBundle(const std::string &bundleFile, const std::string &bundleDir);
    bool generateSquashfsBundle(const std::string &bundleFile, const std::string &bundleDir);

    /**
     * @brief : 根据数据文件dataFile更新appCopy的sectionName段
     * @param : [in] appCopy, 本执行文件的copy
     * @param : [in] sectionName, 待修改的段名
     * @param : [in] dataFile, 新的段内容所在文件
     * @return: true-成功; false-失败
     * @note  : 在使用 objcopy --update-section 命令时，如果新段内容的大小与原段大小不匹配，可能会导致解析错误。
     *          这是因为 ELF 文件格式对每个段的大小有严格的要求，如果段的大小发生变化，ELF 文件的结构可能会被破坏，从而导致解析错误。
                为了避免这种情况，确保新段内容的大小与原段大小一致。如果新段内容较小，可以使用填充（padding）来使其达到所需的大小；
                如果较大，可能需要重新设计你的程序逻辑以适应新的段大小。
     */ 
    // bool updateSection(const std::string &appCopy, const std::string &sectionName, const std::string &dataFile);

    /**
     * @brief : 根据数据文件dataFile增加appCopy的sectionName段
     * @param : [in] appCopy, 本执行文件的copy
     * @param : [in] sectionName, 待增加的段名
     * @param : [in] dataFile, 新的段内容所在文件
     * @return: true-成功; false-失败
     */ 
    bool addSection(const std::string &appCopy, const std::string &sectionName, const std::string &dataFile);

    /**
     * @brief : 根据数据文件dataFile增加appCopy的sectionName段
     * @param : [in] appCopy, 本执行文件的copy
     * @param : [in] sectionName, 待增加的段名
     * @param : [in] dataFile, 新的段内容所在文件
     * @return: true-成功; false-失败
     */ 
    // bool copySection(const std::string &sectionName, const std::string &srcElf, const std::string &destElf);

    /**
     * @brief : 删除elf文件appCopy的sectionName段
     * @param : [in] appCopy, 本执行文件的copy
     * @param : [in] sectionName, 待增加的段名
     * @return: true-成功; false-失败
     */ 
    bool removeSection(const std::string &appCopy, const std::string &sectionName);

private:
    class Private;
    std::shared_ptr<Private> d;
};

#endif // !__KMOABELF_H__
