/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMCHANGELOG_H__
#define __KMCHANGELOG_H__

#include <vector>
#include <string>
#include <istream>
#include <fstream>
#include <memory>

struct ChangelogEntry 
{
    std::string package;
    std::string version;
    std::vector<std::string> distributions;
    std::string urgency;
    std::vector<std::string> changes;
    std::string maintainer;
    std::string date;
};

class KMChangelog
{
public:
    explicit KMChangelog(const std::string& changelog);
    ~KMChangelog();

    /**
     * @brief : 打开changelog文件，准备读取
     * @note  : 仅供读取changelog时使用
     */
    bool open();

    /**
     * @brief : 新建changelog文件后，添加的第一条entry
     * @note  : changelog文件添加第一条记录时调用
     */
    bool addFirst(const ChangelogEntry& entry);

    /**
     * @brief : 添加一条changelog版本变更记录
     */
    bool add(const ChangelogEntry& entry);

    /**
     * @brief : 修改最后一条changelog版本变更记录
     */
    bool update(const ChangelogEntry& entry);

    /**
     * @brief : 逐条解析changelog记录
     * @note  : 先调用open方法
     */
    bool parseNext(ChangelogEntry& entry);

    /**
     * @brief : 解析changelog全部内容
     * @return: changelog记录列表
     * @note  : 先调用open方法
     */
    std::vector<ChangelogEntry> parseAll();

private:
    bool parseHeader(const std::string& line, ChangelogEntry& entry);
    bool parseChangeLine(const std::string& line, ChangelogEntry& entry);

private:
    std::string m_changelog;
    std::unique_ptr<std::ifstream> m_input;
    int m_lineNo = 0;
};

#endif // !__KMCHANGELOG_H__
