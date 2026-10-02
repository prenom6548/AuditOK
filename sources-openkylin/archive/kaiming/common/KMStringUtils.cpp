/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMStringUtils.h"

#include <uuid/uuid.h>
#include <array>

/**
 * @brief : 判断字符串前缀
 * @param : [in] src，原始字符串
 * @param : [in] prefix，字符串前缀
 * @return : true-字符串src是以prefix为前缀；false-字符串src不是以prefix为前缀
 */
bool KMStringUtils::startsWith(const std::string &src, const std::string &prefix)
{
    return src.find(prefix) == 0;
}

/**
 * @brief : 判断字符串后缀
 * @param : [in] src，原始字符串
 * @param : [in] sufix，字符串后缀
 * @return : true-字符串src是以sufix为后缀；false-字符串src不是以sufix为后缀
 */
bool KMStringUtils::endsWith(const std::string &src, const std::string &sufix)
{
    size_t pos = src.rfind(sufix);
    if (pos == std::string::npos)
    {
        return false;
    }
    size_t sufixLen = sufix.length();
    size_t srcLen = src.length();

    return pos + sufix.length() == src.length();
}

/**
 * @brief : 生成uuid
 * @return: uuid
 */
std::string KMStringUtils::generateUUID()
{
    uuid_t uuidtime;
    // 生成基于时间的UUID
    uuid_generate_time(uuidtime);

    // 将UUID转换为字符串格式
    std::array<char, 64> uuidarray;
    uuid_unparse(uuidtime, uuidarray.data());
    return std::string(uuidarray.data());
}

/**
 * @brief 判断字符串是否以某个子串为开始，比如"app", "runtime"
 *
 * @param str 待判断的字符串
 * @param suffix 匹配的子串
 * @return true
 * @return false
 */
bool KMStringUtils::startsWithForRef(const std::string &str, const std::string &prefix)
{
    if (str.length() < prefix.length())
    {
        return false;
    }

    std::string substring = str.substr(0, prefix.length());

    // 判断截取的部分是否与前缀相等
    return substring == prefix;
}

/**
 * @brief 判断字符串是否以某个子串为结尾，比如".Debug", ".Locale"
 *
 * @param str 待判断的字符串
 * @param suffix 匹配的子串
 * @return true
 * @return false
 */
bool KMStringUtils::endsWithForRef(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
    {
        return false;
    }

    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

/**
 * @brief : 根据分隔符拆分字符串
 * @param : [in] str，源字符串
 * @param : [in] split，分隔符
 * @return: std::vector<std::string>，拆分后的字符串数组
 */
std::vector<std::string> KMStringUtils::splitString(const std::string &str, const std::string &split)
{
    std::vector<std::string> values;
    if (str.empty())
    {
        return values;
    }

    int indexStart = 0;
    int indexEnd = -1;
    do
    {
        indexEnd = str.find(split, indexStart);

        if (indexEnd == std::string::npos)
        {
            values.push_back(str.substr(indexStart, str.length() - indexStart));
            break;
        }
        else
        {
            values.push_back(str.substr(indexStart, indexEnd - indexStart));
        }

        indexStart = indexEnd + split.length();
    } while (indexStart < str.length());

    return values;
}

/**
 * @brief 判断字符串是否是数字
 * 
 * @param text 待判断的字符串
 * @return true 当前字符串是数字
 * @return false 当前字符串不是数字
 */
bool KMStringUtils::isNumeric(const std::string &str)
{
    const std::string numbers = "0123456789";
    return str.find_first_not_of(numbers) == std::string::npos;
}

/**
 * @brief 判断当前字符串是否合法的先行版本号
 * 
 * @param text 待判断的字符串
 * @return true 当前字符串是否合法的先行版本号
 * @return false 当前字符串不是合法的先行版本号
 */
bool KMStringUtils::isValidPrerelease(const std::string &str)
{
    const std::string numbers = "0123456789";
    const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-";
    return str.find_first_not_of(numbers + allowedChars) == std::string::npos;
}

/**
 * @brief : 去掉字符串两端的空白符，返回一个新的字符串
 * @param : [in] str, 原始字符串
 * @return: 去掉两端的空白符后的新字符串
 */
std::string KMStringUtils::strstrip(const std::string &str)
{
    if (str.empty())
    {
        return str;
    }

    size_t startPos = 0;
    size_t len = str.length();

    for (; startPos < len; ++startPos)
    {
        if (!std::isspace(str.at(startPos)))
        {
            break;
        }
    }

    if (startPos == len)
    {
        return "";
    }

    size_t endPos = len - 1;
    for (; endPos > startPos; --endPos)
    {
        if (!std::isspace(str.at(endPos)))
        {
            break;
        }
    }

    return str.substr(startPos, endPos - startPos + 1);
}

/**
 * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
 * @param : [in] vector，待合并的字符串数组
 * @param : [in] split，分隔符
 * @return: 合并后的长字符串
 */
std::string KMStringUtils::join(std::vector<std::string> vector, const std::string &split)
{
    std::string value;
    for (auto &e : vector)
    {
        if (!value.empty())
        {
            value += split;
        }
        value += e;
    }

    return value;
}

/**
 * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
 * @param : [in] arr, 以NULL结尾的的字符串数组
 * @param : [in] split，分隔符
 * @return: 合并后的长字符串
 */
std::string KMStringUtils::join(const char **arr, const std::string &split)
{
    std::string value;
    if (arr == nullptr)
    {
        return value;
    }

    for (; *arr != nullptr; arr++)
    {
        if (!value.empty())
        {
            value += split;
        }
        value += *arr;
    }

    return value;
}

/**
 * @brief : 将字符串列表合并成一个字符串，各个元素之间用split分割
 * @param : [in] list，待合并的字符串列表
 * @param : [in] split，分隔符
 * @return: 合并后的长字符串
 */
std::string KMStringUtils::join(std::list<std::string> list, const std::string &split)
{
    std::string value;
    for (auto &e : list)
    {
        if (!value.empty())
        {
            value += split;
        }
        value += e;
    }

    return value;
}

/**
 * @brief : 在字符串数组v中查找是否包含字符串e
 * @param : [in] v, 字符串数组
 * @param : [in] e, 字符串
 * @return: true-包含；false-不包含
 */
bool KMStringUtils::contains(const std::vector<std::string> &v, const std::string &e)
{
    for (auto &element : v)
    {
        if (e == element)
        {
            return true;
        }
    }

    return false;
}

bool KMStringUtils::contains(const std::string &src, const std::string &sub)
{
    return std::string::npos != src.find(sub);
}

/**
 * @brief : 将字符串str中的子串from替换为to
 * @param : [in | out] str, 原字符串
 * @param : [in] from, 需要替换的旧字符子串
 * @param : [in] to, 替换后的新子串
 * @return: 替换后的字符串引用，同str
 */
std::string & KMStringUtils::replace(std::string &str, const std::string &from, const std::string &to)
{
    // 替换所有出现的from为to
    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // 防止无限循环
    }

    return str;
}

void KMStringUtils::replace(std::optional<std::string> &str, const std::string &from, const std::string &to)
{
    if (str.has_value())
    {
        size_t startPos = 0;
        std::string &str_tmp = str.value();
        while ((startPos = str_tmp.find(from, startPos)) != std::string::npos)
        {
            str_tmp.replace(startPos, from.length(), to);
            startPos += to.length(); // 防止无限循环
        }
    }
}

/**
 * @brief : 将字符串数组中的所有字符串中的子串from替换为to
 * @param : [in | out] str, 原字符串
 * @param : [in] from, 需要替换的旧字符子串
 * @param : [in] to, 替换后的新子串
 * @return: 替换后的字符串引用，同str
 */
void KMStringUtils::replace(std::vector<std::string> &strs, const std::string &from, const std::string &to)
{
    for (int i = 0 ; i < strs.size() ; i++)
    {
        replace(strs[i], from, to);
    }
}

void KMStringUtils::replace(std::optional<std::vector<std::string>> &strs, const std::string &from, const std::string &to)
{
    if (strs.has_value())
    {
        std::vector<std::string> & strs_tmp = strs.value();
        for (int i = 0 ; i < strs_tmp.size() ; i++)
        {
            replace(strs_tmp[i], from, to);
        }
    }
}

/**
 * @brief : 组装文件名
 * @param : [in] path, 前缀路径
 * @param : [in] name, 后续文件或目录名
 * @return: 组装后的文件路径
 */
std::string KMStringUtils::buildFilename(const std::string &path, const std::string &name)
{
    std::string filename = path + "/" + name;
    KMStringUtils::replace(filename, "//", "/");

    return filename;
}

std::string KMStringUtils::buildFilename(const std::string &path, const std::string &name1, const std::string &name2)
{
    std::string filename = path + "/" + name1 + "/" + name2;
    KMStringUtils::replace(filename, "//", "/");

    return filename;
}

std::string KMStringUtils::buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3)
{
    std::string filename = path + "/" + name1 + "/" + name2 + "/" + name3;
    KMStringUtils::replace(filename, "//", "/");

    return filename;
}

std::string KMStringUtils::buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3, const std::string &name4)
{
    std::string filename = path + "/" + name1 + "/" + name2 + "/" + name3 + "/" + name4;
    KMStringUtils::replace(filename, "//", "/");

    return filename;
}


/**
 * @brief 分割字符串
 * 
 * @param str 待分割的字符串
 * @param delimiter 分隔符，如 '/', '.', ',' 等
 * @return std::vector<std::string> 分割之后的向量 
 */
std::vector<std::string> KMStringUtils::splitStringrByDelimiter(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief 检查字符串的最后一个字符是否为 '\n'，如果是则将其移除。
 * 
 * 比如 description 会包含'\n'
 * 
 * @param str 返回去除 '\n' 的字符串
*/
void KMStringUtils::removeTrailingNewline(std::string &str)
{
    // 如果字符串不为空，且最后一个字符为 '\n'，则移除它
    if (!str.empty() && str.back() == '\n') {
        str.pop_back();
    }
}

/**
 * @brief 根据正则表达式判断版本号格式是否符合规范
 * 
 * @param version 版本号，如 "1.0.0.0"
 * @return true 符合4段式版本规范
 * @return false 不符合4段式版本规范
 */
bool KMStringUtils::isValidVersion(const std::string &version)
{
    // 正则表达式匹配格式：主版本.次版本.修订版本.扩充安全更新版本 (数字格式)

    std::regex versionPattern(R"(^\d+\.\d+\.\d+\.\d+$)");

    return std::regex_match(version, versionPattern);
}

/**
 * @brief 分割版本号字符串为整数数组
 * 
 * @param version 版本号，如 "1.0.0.0"
 * @return std::vector<int> 分割后的整数数组
 */
std::vector<int> KMStringUtils::splitVersion(const std::string &version)
{
    std::vector<int> versionNumbers;
    std::string::size_type start = 0;
    std::string::size_type end = version.find('.');

    while (end != std::string::npos)
    {
        versionNumbers.push_back(std::stoi(version.substr(start, end - start)));
        start = end + 1;
        end = version.find('.', start);
    }

    // 将最后一部分数字加入数组
    versionNumbers.push_back(std::stoi(version.substr(start)));

    return versionNumbers;
}

/**
 * @brief 比较版本号大小
 * 
 * 对两个版本号进行逐段比较，从主版本到扩充安全更新版本。比较规则如下：
 * 
 * - 如果 version1 的某一段小于 version2 的对应段，则返回 -1 表示 version1 小于 version2。
 * - 如果 version1 的某一段大于 version2 的对应段，则返回  1 表示 version1 大于 version2。
 * - 如果所有段都相等，则返回 0 表示两个版本号相等。
 * 
 * @param version1 第一个版本号
 * @param version2 第二个版本号
 * @return int 比较后的值。
 *     -1：第一个版本号 < 第二个版本号
 *      1：第一个版本号 > 第二个版本号
 *      0：第一个版本号 = 第二个版本号
 */
int KMStringUtils::compareVersions(const std::string &version1, const std::string &version2)
{
    if (!isValidVersion(version1) || !isValidVersion(version2))
    {
        throw std::invalid_argument("Invalid version format");
    }

    std::vector<int> v1 = splitVersion(version1);
    std::vector<int> v2 = splitVersion(version2);

    // 逐段比较版本号
    for (size_t i = 0; i < v1.size(); ++i)
    {
        if (v1[i] < v2[i])
        {
            return -1; // version1 < version2
        }

        if (v1[i] > v2[i])
        {
            return 1; // version1 > version2
        }
    }

    return 0; // version1 == version2
}

/**
 * @brief 解析 id 的类型
 * 
 * @param id id类型，如 org.kde.kclock, dev.kylin.runtime, dev.kylin.base
 * @return std::string id中的type类型
 */
std::string KMStringUtils::parseType(const std::string &id)
{
    auto idParts = splitStringrByDelimiter(id, '.');
    if (idParts.size() > 2)
    {
        const std::string &lastPart = idParts.back();
        if (lastPart == "runtime" || lastPart == "platform" || lastPart == "Platform")
        {
            return BASE_TYPE_RUNTIME;
        }
        else if (lastPart == "base")
        {
            return BASE_TYPE_BASE;
        }
    }
    return BASE_TYPE_APP; // 默认为 BASE_TYPE_APP
}

 /**
 * @brief 验证版本号是否符合 Semver 规范
 * 
 * @param version 版本号
 * @return true 符合 Semver 规范
 * @return false 不符合 Semver 规范
 */
bool KMStringUtils::isValidSemver(const std::string &version)
{
    if (version.empty())
    {
        return false;
    }
    // 使用正则表达式验证 Semver 格式
    const std::regex semverRegex(R"(^\d+\.\d+\.\d+(\.\d+)?(-[a-zA-Z0-9]+(\.[a-zA-Z0-9]+)*)?$)");
    return std::regex_match(version, semverRegex);
}

/**
 * @brief 获取ref最后以 "/" 结尾的部分
 *
 * @param str 字符串，如运行时
 * @return std::string
 */
std::string KMStringUtils::getLastPartWithDelimiter(const std::string &str)
{
    // 查找最后一个 "/" 的位置
    size_t lastSlashPos = str.find_last_of('/');
    if (lastSlashPos != std::string::npos)
    {
        // 如果找到了 "/"，则返回其后面的部分
        return str.substr(lastSlashPos + 1);
    }
    else
    {
        // 如果没有找到 "/"，则返回空字符串
        return "stable";
    }
}

/**
 * @brief 截取ref 从开始到最后以 "/" 结尾的部分
 *
 * @param ref
 * @return std::string
 */
std::string KMStringUtils::removeLastPartWithDelimiter(const std::string &ref)
{
    size_t pos = ref.find_last_of('/');
    if (pos != std::string::npos)
    {
        return ref.substr(0, pos + 1);
    }
    else
    {
        // 如果没有找到 "/", 返回原始字符串
        return ref;
    }
}

std::string KMStringUtils::extractPackageId(const std::string &path)
{
    const std::vector<std::string> kinds = {"/base/", "/runtime/", "/app/", "/depend/"};

    for (const auto &kind : kinds)
    {
        size_t pos = path.find(kind);
        if (pos != std::string::npos)
        {
            size_t start = pos + kind.length();
            size_t end = path.find('/', start);
            if (end != std::string::npos)
            {
                return path.substr(start, end - start);
            }
        }
    }
    return "";
}