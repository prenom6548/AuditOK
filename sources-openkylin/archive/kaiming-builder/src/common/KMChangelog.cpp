/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMChangelog.h"

#include <regex>
#include <sstream>
#include <filesystem>

#include "KMLogger.h"
#include "KMException.h"

namespace fs = std::filesystem;

KMChangelog::KMChangelog(const std::string& changelog)
    : m_changelog(changelog)
{}

KMChangelog::~KMChangelog()
{
    if (m_input && m_input->is_open())
    {
        m_input->close();
    }
}

bool KMChangelog::open()
{
    if (m_input)
    {
        if (m_input->is_open())
        {
            m_input->close();
        }
    }

    m_input = std::make_unique<std::ifstream>(m_changelog);
    if (m_input->is_open())
    {
        m_lineNo = 0;
        return true;
    }

    return false;
}

bool KMChangelog::addFirst(const ChangelogEntry& entry)
{
    std::ofstream outFile(m_changelog);

    // Write header
    outFile << entry.package << " (" << entry.version << ")";
    for (const auto& dist : entry.distributions)
    {
        outFile << " " << dist;
    }
    outFile << "; urgency=" << entry.urgency << "\n\n";

    // Write changes
    for (const auto& change : entry.changes)
    {
        outFile << "  * " << change << "\n";
    }

    // Write footer
    outFile << "\n -- " << entry.maintainer << "  " << entry.date << "\n\n";

    return true;
}

bool KMChangelog::add(const ChangelogEntry& entry)
{
    std::string tmp_changelog = fs::path(m_changelog).parent_path() / "~changelog";
    std::ofstream outFile(tmp_changelog);

    // Write header
    outFile << entry.package << " (" << entry.version << ")";
    for (const auto& dist : entry.distributions)
    {
        outFile << " " << dist;
    }
    outFile << "; urgency=" << entry.urgency << "\n\n";

    // Write changes
    for (const auto& change : entry.changes)
    {
        outFile << "  * " << change << "\n";
    }

    // Write footer
    outFile << "\n -- " << entry.maintainer << "  " << entry.date << "\n\n";

    // 打开原文件
    std::ifstream inFile(m_changelog);

    // 写入原内容
    outFile << inFile.rdbuf();

    // 关闭文件句柄
    inFile.close();
    outFile.close();

    // 替换原文件
    fs::rename(tmp_changelog, m_changelog);

    return true;
}

bool KMChangelog::update(const ChangelogEntry& entry)
{
    std::string tmp_changelog = fs::path(m_changelog).parent_path() / "~changelog";
    std::ofstream outFile(tmp_changelog);

    // Write header
    outFile << entry.package << " (" << entry.version << ")";
    for (const auto& dist : entry.distributions)
    {
        outFile << " " << dist;
    }
    outFile << "; urgency=" << entry.urgency << "\n\n";

    // Write changes
    for (const auto& change : entry.changes)
    {
        outFile << "  * " << change << "\n";
    }

    // Write footer
    outFile << "\n -- " << entry.maintainer << "  " << entry.date << "\n\n";

    // 写入原内容
    outFile << m_input->rdbuf();

    // 关闭文件句柄
    m_input->close();
    outFile.close();

    // 替换原文件
    fs::rename(tmp_changelog, m_changelog);

    return true;
}

bool KMChangelog::parseNext(ChangelogEntry& entry)
{
    entry = ChangelogEntry();
    std::string line;
    
    // Parse header line
    while (++m_lineNo && std::getline(*m_input, line) && line.empty());
    if (line.empty())
    {
        // no more entry
        return false;
    }

    if (!parseHeader(line, entry))
    {
        throw KMException("Parse header error, at line : " + std::to_string(m_lineNo));
    }

    ++m_lineNo;
    // Parse blank line after header
    if (!std::getline(*m_input, line) || !line.empty())
    {
        throw KMException("This should be a blank line, at line : " + std::to_string(m_lineNo));
    }

    // Parse change lines
    while (++m_lineNo && std::getline(*m_input, line) && !line.empty())
    {
        if (!parseChangeLine(line, entry))
        {
            throw KMException("This line should start with ' ', '\t', '* ', at line : " + std::to_string(m_lineNo));
        }
    }

    // Parse trailer line (maintainer and date)
    if (++m_lineNo && !std::getline(*m_input, line))
    {
        throw KMException("Parse maintainer and date error, at line : " + std::to_string(m_lineNo));
    }

    if (line.find(" -- ") != 0)
    {
        throw KMException("Maintainer and date line should start with ' -- ', at line : " + std::to_string(m_lineNo));
    }

    entry.maintainer = line.substr(4, line.find_last_of("  "));
    entry.date = line.substr(line.find_last_of("  ") + 1);

    return true;
}

bool KMChangelog::parseHeader(const std::string& line, ChangelogEntry& entry)
{
    static const std::regex header_regex(
        R"(^(\w[^\s]*) \(([^)]+)\)\s+([^;]+);\s+urgency=([^\s]+))");
    
    std::smatch matches;
    if (!std::regex_match(line, matches, header_regex))
    {
        return false;
    }

    entry.package = matches[1].str();
    entry.version = matches[2].str();
    
    std::istringstream dist_stream(matches[3].str());
    std::string dist;
    while (std::getline(dist_stream, dist, ' '))
    {
        if (!dist.empty())
        {
            entry.distributions.push_back(dist);
        }
    }

    entry.urgency = matches[4].str();
    return true;
}

bool KMChangelog::parseChangeLine(const std::string& line, ChangelogEntry& entry)
{
    if (line.empty())
    {
        return false;
    }

    if (line.find("  * ") == 0)
    {
        entry.changes.push_back(line.substr(4));
    }
    else if (line[0] == ' ' || line[0] == '\t')
    {
        entry.changes.back() = entry.changes.back() + line;
    }
    else if (line[0] == '*')
    {
        entry.changes.push_back(line.substr(2));
    }
    else
    {
        return false;
    }

    return true;
}

std::vector<ChangelogEntry> KMChangelog::parseAll()
{
    std::vector<ChangelogEntry> entries;
    ChangelogEntry entry;
    while (parseNext(entry))
    {
        entries.push_back(entry);
    }
    return entries;
}

