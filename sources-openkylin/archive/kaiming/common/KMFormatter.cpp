/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMFormatter.h"
#include "kmtranslation.h"

KMFormatter::KMFormatter()
{
}

KMFormatter::~KMFormatter()
{
}

std::string KMFormatter::truncate(const std::string &str, size_t width)
{
    // 判断字符宽度，如果宽度超出则截断并添加 "..."
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);
    size_t len = 0;
    std::wstring truncated;

    for (wchar_t ch : wstr)
    {
        // 中文字符宽度大于1, 英文宽度为1
        len += (ch > 127) ? 2 : 1;

        // 减去4，是因为中文字符占两个宽度，减去4后，可以保证中文字符不会被截断
        if (len <= width - 4)
        {
            truncated += ch;
        }
        else
        {
            truncated += L"...";
            break;
        }
    }
    return converter.to_bytes(truncated);
}

std::string KMFormatter::pad(const std::string &str, size_t width)
{
    // 获取实际宽度，考虑到中英文混排问题
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(str);
    size_t len = 0;

    for (wchar_t ch : wstr)
    {
        len += (ch > 127) ? 2 : 1; // 中文宽度2，英文宽度1
    }

    std::string padded = str;
    if (len < width)
    {
        padded.append(width - len, ' ');
    }
    return padded;
}

void KMFormatter::setColumnWidth(size_t idW, size_t nameW, size_t versionW, size_t archW, size_t channelW, size_t moduleW, size_t descriptionW, size_t sizeW, size_t baseW, size_t runtimeW)
{
    idWidth = idW;
    nameWidth = nameW;
    versionWidth = versionW;
    archWidth = archW;
    channelWidth = channelW;
    moduleWidth = moduleW;
    descriptionWidth = descriptionW;
    sizeWidth = sizeW;
    baseWidth = baseW;
    runtimeWidth = runtimeW;
}

std::string KMFormatter::getColumnHead(const std::string &column)
{
    std::string head = column;
    return head;
}

void KMFormatter::printHeader()
{
    // ANSI 转义序列，用于设置文本颜色为黄色
    //const std::string yellow = "\033[1;33m";
    const std::string headerColor = "\033[1;36m";  // 1;36 = 亮青色
    // ANSI 转义序列，用于重置文本颜色
    const std::string reset = "\033[0m";

    std::cout << std::left << headerColor << pad(truncate(getColumnHead(_("id")), idWidth), idWidth);

    if (showName)
    {
        std::cout << pad(truncate(getColumnHead(_("name")), nameWidth), nameWidth);
    }

    std::cout << pad(truncate(getColumnHead(_("version")), versionWidth), versionWidth);
    if (showArch)
    {
        std::cout << pad(truncate(getColumnHead(_("arch")), archWidth), archWidth);
    }

    if (showChannel)
    {
        std::cout << pad(truncate(getColumnHead(_("channel")), channelWidth), channelWidth);
    }

    if (showSize)
    {
        std::cout << pad(truncate(getColumnHead(_("size")), sizeWidth), sizeWidth);
    }

    std::cout << pad(truncate(getColumnHead(_("module")), moduleWidth), moduleWidth);

    if (showDescription)
    {
        std::cout << pad(truncate(getColumnHead(_("description")), descriptionWidth), descriptionWidth);
    }

    if (showBase)
    {
        std::cout << pad(truncate(getColumnHead(_("base")), baseWidth), baseWidth);
    }

    if (showRuntime)
    {
        std::cout << pad(truncate(getColumnHead(_("runtime")), runtimeWidth), runtimeWidth);
    }

    std::cout << reset << std::endl;
}

void KMFormatter::printEntry(const KMEntry &entry)
{
    KMEntry tempEntry = entry;

    tempEntry.arch = tempEntry.arch.empty() ? "N/A" : tempEntry.arch;
    tempEntry.name = tempEntry.name.empty() ? "N/A" : tempEntry.name;
    tempEntry.id = tempEntry.id.empty() ? "N/A" : tempEntry.id;
    tempEntry.module = tempEntry.module.empty() ? "N/A" : tempEntry.module;
    tempEntry.version = tempEntry.version.empty() ? "N/A" : tempEntry.version;
    tempEntry.size = tempEntry.size.empty() ? "N/A" : tempEntry.size;
    
     // 清理 description 中的转义字符
    std::string cleanDescription = entry.description;
    // 移除所有控制字符，特别是回车符 \r
    cleanDescription.erase(
        std::remove_if(cleanDescription.begin(), cleanDescription.end(),
            [](unsigned char c) { return c == '\r' || c == '\n' || c == '\t' || c == '\b'; }),
        cleanDescription.end()
    );

    tempEntry.description = cleanDescription.empty() ? "N/A" : cleanDescription;
    tempEntry.base = entry.base.empty() ? "N/A" : entry.base;
    tempEntry.runtime = entry.runtime.empty() ? "N/A" : entry.runtime;

    // 逐列输出，确保中文字符对齐
    std::cout << pad(truncate(tempEntry.id, idWidth), idWidth);

    if (showName)
    {
        std::cout << pad(truncate(tempEntry.name, nameWidth), nameWidth);
    }

    std::cout << pad(truncate(tempEntry.version, versionWidth), versionWidth);

    if (showArch)
    {
        std::cout << pad(truncate(tempEntry.arch, archWidth), archWidth);
    }

    if (showChannel)
    {
        std::cout << pad(truncate(tempEntry.channel, channelWidth), channelWidth);
    }

    if (showSize)
    {
        std::cout << pad(truncate(tempEntry.size, sizeWidth), sizeWidth);
    }

    std::cout << pad(truncate(tempEntry.module, moduleWidth), moduleWidth);

    if (showDescription)
    {
        std::cout << pad(truncate(tempEntry.description, descriptionWidth), descriptionWidth);
    }

    if (showBase)
    {
        std::cout << pad(truncate(tempEntry.base, baseWidth), baseWidth);
    }

    if (showRuntime)
    {
        std::cout << pad(truncate(tempEntry.runtime, runtimeWidth), runtimeWidth);
    }

    std::cout << std::endl;
}

void KMFormatter::printEntries(const std::vector<KMEntry> &entries)
{
    printHeader();
    for (const auto &entry : entries)
    {
        printEntry(entry);
    }
}

void KMFormatter::setShowChannel(bool show)
{
    showChannel = show;
}

void KMFormatter::setShowArch(bool show)
{
    showArch = show;
}

void KMFormatter::setShowSize(bool show)
{
    showSize = show;
}

void KMFormatter::setShowBase(bool show)
{
    showBase = show;
}

void KMFormatter::setShowRuntime(bool show)
{
    showRuntime = show;
}

void KMFormatter::setShowDescription(bool show)
{
    showDescription = show;
}

void KMFormatter::setShowName(bool show)
{
    showName = show;
}

void KMFormatter::printFooter()
{
    std::cout << std::endl;
}
