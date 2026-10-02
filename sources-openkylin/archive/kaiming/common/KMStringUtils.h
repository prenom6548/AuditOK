/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMSTRINGUTILS_H__
#define __KMSTRINGUTILS_H__

#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <regex>
#include <iostream>
#include <optional>

#include "KMCommonTypePrivate.h"
// #include "KMConfigData.h"

class KMStringUtils
{
public:
    /**
     * @brief : 判断字符串前缀
     * @param : [in] src，原始字符串
     * @param : [in] prefix，字符串前缀
     * @return : true-字符串src是以prefix为前缀；false-字符串src不是以prefix为前缀
     */
    static bool startsWith(const std::string &src, const std::string &prefix);

    /**
     * @brief : 判断字符串后缀
     * @param : [in] src，原始字符串
     * @param : [in] sufix，字符串后缀
     * @return : true-字符串src是以sufix为后缀；false-字符串src不是以sufix为后缀
     */
    static bool endsWith(const std::string &src, const std::string &sufix);

    /**
     * @brief 判断字符串是否以某个子串为开始，比如"app", "runtime"
     *
     * @param str 待判断的字符串
     * @param suffix 匹配的子串
     * @return true
     * @return false
     */
    static bool startsWithForRef(const std::string &str, const std::string &prefix);

    /**
     * @brief : 判断字符串是否以某个子串为结尾，比如".Debug", ".Locale"
     * @param : [in] src，原始字符串
     * @param : [in] sufix，字符串后缀
     * @return : true-字符串src是以sufix为后缀；false-字符串src不是以sufix为后缀
     */
    static bool endsWithForRef(const std::string &src, const std::string &sufix);

    /**
     * @brief : 生成uuid
     * @return: uuid
     */
    static std::string generateUUID();

    /**
     * @brief : 根据分隔符拆分字符串
     * @param : [in] str，源字符串
     * @param : [in] split，分隔符
     * @return: std::vector<std::string>，拆分后的字符串数组
     */
    static std::vector<std::string> splitString(const std::string &str, const std::string &split);

    /**
     * @brief 判断字符串是否是数字
     * 
     * @param text 待判断的字符串
     * @return true 当前字符串是数字
     * @return false 当前字符串不是数字
     */
    static bool isNumeric(const std::string &str);

    /**
     * @brief 判断当前字符串是否合法的先行版本号
     * 
     * @param text 待判断的字符串
     * @return true 当前字符串是否合法的先行版本号
     * @return false 当前字符串不是合法的先行版本号
     */
    static bool isValidPrerelease(const std::string &str);

    /**
     * @brief : 去掉字符串两端的空白符，返回一个新的字符串
     * @param : [in] str, 原始字符串
     * @return: 去掉两端的空白符后的新字符串
     */
    static std::string strstrip(const std::string &str);

    /**
     * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
     * @param : [in] vector，待合并的字符串数组
     * @param : [in] split，分隔符
     * @return: 合并后的长字符串
     */
    static std::string join(std::vector<std::string> vector, const std::string &split);

    /**
     * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
     * @param : [in] arr, 以NULL结尾的的字符串数组
     * @param : [in] split，分隔符
     * @return: 合并后的长字符串
     */
    static std::string join(const char **arr, const std::string &split);

        /**
     * @brief : 将字符串列表合并成一个字符串，各个元素之间用split分割
     * @param : [in] list，待合并的字符串列表
     * @param : [in] split，分隔符
     * @return: 合并后的长字符串
     */
    static std::string join(std::list<std::string> list, const std::string &split);

    /**
     * @brief : 在字符串数组v中查找是否包含字符串e
     * @param : [in] v, 字符串数组
     * @param : [in] e, 字符串
     * @return: true-包含；false-不包含
     */
    static bool contains(const std::vector<std::string> &v, const std::string &e);
    static bool contains(const std::string &src, const std::string &sub);

    /**
     * @brief : 将字符串str中的子串from替换为to
     * @param : [in | out] str, 原字符串
     * @param : [in] from, 需要替换的旧字符子串
     * @param : [in] to, 替换后的新子串
     * @return: 替换后的字符串引用，同str
     */
    static std::string & replace(std::string &str, const std::string &from, const std::string &to);
    static void replace(std::optional<std::string> &str, const std::string &from, const std::string &to);
    /**
     * @brief : 将字符串数组中的所有字符串中的子串from替换为to
     * @param : [in | out] str, 原字符串
     * @param : [in] from, 需要替换的旧字符子串
     * @param : [in] to, 替换后的新子串
     * @return: 替换后的字符串引用，同str
     */
    static void replace(std::vector<std::string> &strs, const std::string &from, const std::string &to);
    static void replace(std::optional<std::vector<std::string>> &strs, const std::string &from, const std::string &to);

    /**
     * @brief : 组装文件名
     * @param : [in] path, 前缀路径
     * @param : [in] name, 后续文件或目录名
     * @return: 组装后的文件路径
     */
    static std::string buildFilename(const std::string &path, const std::string &name);
    static std::string buildFilename(const std::string &path, const std::string &name1, const std::string &name2);
    static std::string buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3);
    static std::string buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3, const std::string &name4);

    /**
     * @brief 分割字符串
     * 
     * @param str 待分割的字符串
     * @param delimiter 分隔符，如 '/', '.', ',' 等
     * @return std::vector<std::string> 分割之后的向量 
     */
    static std::vector<std::string> splitStringrByDelimiter(const std::string& str, char delimiter) ;

    /**
     * @brief 检查字符串的最后一个字符是否为 '\n'，如果是则将其移除。
     * 
     * 比如 description 会包含'\n'
     * 
     * @param str 返回去除 '\n' 的字符串
     */
    static void removeTrailingNewline(std::string &str);

    /**
     * @brief 根据正则表达式判断版本号格式是否符合规范
     * 
     * @param version 版本号，如 "1.0.0.0"
     * @return true 符合4段式版本规范
     * @return false 不符合4段式版本规范
     */
    static bool isValidVersion(const std::string& version);

    /**
     * @brief 分割版本号字符串为整数数组
     * 
     * @param version 版本号，如 "1.0.0.0"
     * @return std::vector<int> 分割后的整数数组
     */
    static std::vector<int> splitVersion(const std::string& version);

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
    static int compareVersions(const std::string& version1, const std::string& version2);

    /**
     * @brief 解析 id 的类型
     * 
     * @param id id类型，如 org.kde.kclock, dev.kylin.runtime, dev.kylin.base
     * @return std::string id中的type类型
     */
    static std::string parseType(const std::string& id);

    /**
     * @brief 验证版本号是否符合 Semver 规范
     * 
     * @param version 版本号
     * @return true 符合 Semver 规范
     * @return false 不符合 Semver 规范
     */
    static bool isValidSemver(const std::string& version);

    /**
     * @brief 获取ref最后以 "/" 结尾的部分
     *
     * @param str 字符串，如运行时
     * @return std::string
     */
    static std::string getLastPartWithDelimiter(const std::string &str);

    /**
     * @brief 截取ref 从开始到最后以 "/" 结尾的部分
     *
     * @param ref
     * @return std::string
     */
    static std::string removeLastPartWithDelimiter(const std::string &ref);

    /**
     * @description: 从路径中提取ID
     * @param {string&} path 路径
     * @return {*}: 返回ID
     */
    static std::string extractPackageId(const std::string& path);
};

#endif // !__KMSTRINGUTILS_H__
