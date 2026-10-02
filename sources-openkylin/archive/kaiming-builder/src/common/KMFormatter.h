/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMFORMATTER_H
#define KMFORMATTER_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <regex>
#include <locale>
#include <codecvt>
#include <string>

typedef struct KMEntry
{
    std::string id;
    std::string name;
    std::string kind;
    std::string version;
    std::string arch;
    std::string channel;
    std::string module;
    std::string description;
    std::string size;
    std::string base;
    std::string runtime;
    std::string repo;
} KMEntry;

class KMFormatter
{
public:
    KMFormatter();
    ~KMFormatter();

    /**
     * @brief 截断字符串，如果宽度超出则截断并添加 "..." 
     * 
     * @param str 待截断的字符串
     * @param width 截断宽度
     * @return std::string 截断后的字符串
     */
    static std::string truncate(const std::string &str, size_t width);

    /**
     * @brief 填充字符串，如果宽度超出则截断并添加 "..." 
     * 
     * @param str 待填充的字符串
     * @param width 填充宽度
     * @return std::string 填充后的字符串
     */
    static std::string pad(const std::string &str, size_t width);
    

    /**
     * @brief 设置列宽度
     * 
     * @param idW id 列宽度
     * @param nameW name 列宽度
     * @param versionW version 列宽度
     * @param archW arch 列宽度
     * @param channelW channel 列宽度
     * @param moduleW module 列宽度
     * @param descriptionW description 列宽度
     */
    void setColumnWidth(size_t idW, size_t nameW, size_t versionW, size_t archW, size_t channelW, size_t moduleW, size_t descriptionW, size_t sizeW, size_t baseW, size_t runtimeW);

    /**
     * @brief 获取列头
     * 
     * @param column 列名
     * @return std::string 列头
     */
    std::string getColumnHead(const std::string &column);

    /**
     * @brief 打印表头
     */
    void printHeader();

    /**
     * @brief 打印单个条目
     * 
     * @param entry 条目
     */
    void printEntry(const KMEntry &entry);

    /**
     * @brief 打印多个条目
     * 
     * @param entries 条目列表
     */
    void printEntries(const std::vector<KMEntry> &entries);

    /**
     * @brief 设置是否显示 channel 列
     * 
     * @param show 是否显示
     */ 
    void setShowChannel(bool show);

    /**
     * @brief 设置是否显示 arch 列
     * 
     * @param show 是否显示
     */
    void setShowArch(bool show);

    /**
     * @brief 设置是否显示 size 列
     * 
     * @param show 是否显示
     */
    void setShowSize(bool show);

    /**
     * @brief 设置是否显示 base 列
     * 
     * @param show 是否显示
     */
    void setShowBase(bool show);

    /**
     * @brief 设置是否显示 runtime 列
     * 
     * @param show 是否显示
     */
    void setShowRuntime(bool show);

    /**
     * @brief 打印表尾
     * 
     */
    void printFooter();

private:
    size_t idWidth = 35;          // id 列宽度
    size_t nameWidth = 15;        // name 列宽度
    size_t kindWidth = 15;
    size_t versionWidth = 20;     // version 列宽度
    size_t archWidth = 10;        // arch 列宽度
    size_t channelWidth = 15;     // channel 列宽度
    size_t moduleWidth = 12;      // module 列宽度
    size_t repoWidth = 20;        //repo 列宽度
    size_t descriptionWidth = 30; // description 列宽度
    size_t sizeWidth = 10;        // size 列宽度
    size_t baseWidth = 30;      // base 列宽度
    size_t runtimeWidth = 30;      // runtime 列宽度

    bool showChannel = true; // 是否显示 channel 列
    bool showArch = true;    // 是否显示 arch 列
    bool showSize = true;    // 是否显示 size 列
    bool showBase = false;    // 是否显示 base 列
    bool showRuntime = false;    // 是否显示 runtime 列
};

#endif