/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */



#ifndef __BUILDER_UTILS_H__
#define __BUILDER_UTILS_H__

#include <list>
#include <vector>
#include <string>
#include <filesystem>

#include <sstream>
#include <iostream>
#include <algorithm>
#include <system_error>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <yaml-cpp/yaml.h>

#include <unordered_map>
#include <regex>

#include "common/KMException.h"
#include "common/KMRef.h"
#include "common/KMVersion.h"

namespace fs = std::filesystem;
using namespace std;

class KMBuilderUtils
{
public:
    KMBuilderUtils();
    ~KMBuilderUtils();

    static list<string> split(string str, char c);
    static const string getArch();
    static bool ensureDir(fs::path dir);

    /**
     * @brief : 替换id字符串前缀-
     * @param origId : id字符串
     * @return : 
     */
    static string replaceIdPrefix(const string &origId);
    /**
     * @brief : 利用重载函数,执行参数拼接
     * @param prefix : 前缀字符串
     * @param content : 字符串正文
     * @param suffix : 后缀字符串
     * @return : 拼接之后的字符串
     */
    static string argSplice(string prefix, string content);
    static string argSplice(string prefix, string content, string suffix);
    static string argSplice(string prefix, int content);

    /**
     * @brief : 获取子路径
     * @param parentPath : 父路径
     * @param childName : 子路径
     * @return : 拼接之后的全路径
     */
    static std::string getChildPath(const std::string &parentPath, const std::string &childName);
    /**
     * @brief : 判断路径是否存在
     * @param path : 路径
     * @return : true-存在; false-不存在
     */
    static bool pathExists(const std::string &path);

    /**
     * @brief : 拼接命令行参数并提交执行
     * @param appDir : 路径
     * @param envVars : 环境变量
     * @param extraArgs : 运行参数
     * @param commandline : 命令
     * @return : true-成功; false-失败
     */
    static bool command(string appDir, vector<string> envVars, vector<string> extraArgs, string commandLine);

    /**
     * @brief : 执行外部程序
     * @param : [in] workingDirectory, 工作目录
     * @param : [in] args, 执行外部程序需要传入的参数列表，包括外部程序本身
     * @param : [in] ignoreExitCode，是否忽略退出码
     * @return: true - 成功执行；false - 出现错误
    */
    static bool spawn(const std::string &workingDirectory, const std::vector<std::string> &args, bool ignoreExitCode = false);

    /**
     * @brief : 清除文件的硬链接
     * @filePath : 文件路径
     * @return: true-成功；false-失败
     */
    static bool breakHardlink(const std::filesystem::path &filePath);

    /**
     * @brief : 创建目录软连接
     * @param : target 目标目录
     * @param : link 连接符号
     * @param : ec 错误信息
     * @return: true-创建成功；false-创建失败
     */
    static bool createDirectorySymlink(const std::string &target, const std::string &link, std::error_code &ec);

    /**
     * pathMatchPrefix:
     * @brief : 判断路径string是否匹配pattern前缀，返回匹配结束后的部分字符串
     * @param : [in] pattern, 前缀模式
     * @param : [in] string, 路径
     * @return: 返回匹配结束后的部分字符串，不匹配返回null；
     */
    static const char *pathMatchPrefix(const char *pattern, const char *string);

    /**
     * @brief : 获取目录下所有文件
     * @param : dir 查找的目标目录
     * @param : files 所有的文件
     */
    static void getFilesInDir(const std::string &dir, vector<string> &files);

    static const string getCommandResoult(const string &exe, const list<string> &args);

    /**
     * @brief : 路径模式匹配
     * @param : pattern 正则
     * @param : path 文件路径
     * @return : 匹配到的内容
     */
    static bool matchesPathPattern(string path, string pattern);

    /**
     * @brief : 路径前缀模式匹配
     * @param : pattern 正则
     * @param : string 文件路径
     * @return : 匹配到的内容
     */
    static string pathMatchPrefix(const std::string &pattern, const std::string &string);

    /**
     * @brief : 获取文件名
     */
    static string getBasename(string path);

    /**
     * @brief : 获取目录名
     */
    static string getDirname(const string &path);

    /**
     * @brief : 获取子目录名
     */
    static string getChildDir(const string &base, const string &child);

    /**
     * @brief : 将文件内容存储到string中
     */
    static std::string fileGetContents(const std::string &filePath);

    /**
     * @brief : 将配置文件内容写入文件（添加）
     */
    static bool fileSetContents(const string &filePath, const string &content);

    /**
     * @brief : 将配置文件内容写入文件（重写）
     */
    static bool fileReSetContents(const string &filePath, const string &content);

    /**
     * @brief : 转移T*指针的所有权
     */
    template<typename T>
    static T *stealPointer(T **p)
    {
        if (p == nullptr || *p == nullptr)
            return nullptr;

        T *r = *p;
        *p = nullptr;
        return r;
    }

    /**
     * @brief : 根据分隔符拆分字符串
     * @param : [in] str，源字符串
     * @param : [in] split，分隔符
     * @return: std::vector<std::string>，拆分后的字符串数组
     */
    static std::vector<std::string> splitString(const std::string &str, const std::string &split);

    /**
     * @brief : 将字符串数组合并成一个字符串，各个元素之间用split分割
     * @param : [in] vector，待合并的字符串数组
     * @param : [in] split，分隔符
     * @return: 合并后的长字符串
     */
    static std::string join(const std::vector<std::string>& vector, const std::string &split);

    /**
     * @brief : 将字符串str中连续的ch字符缩减成1个
     * @param : [in], str, 待处理的字符串
     * @param : [in], ch，待瘦身的字符
     * @return: 处理完后的字符串
     */
    static std::string removeDuplicateChars(const std::string& str, char ch);

    static bool argumentNeedsQuoting(const char *arg);

    /**
     * isEmptyDirectory:
     * @brief : 判断路径是否空目录，使用前先判断是否是目录
     * @param : [in] path，路径
     * @return: true-空目录或不存在; false-不是空目录或不是目录
     */
    static bool isEmptyDirectory(const std::string &path);

    /**
     * @brief : 获取目录下所有的文件列表（包含子目录下的文件）
     * @param : [in] dir，路径
     */
    static std::vector<std::string> getDirFiles(const fs::path &dir);

    /**
     * removeEmptyDirectory:
     * @brief : 递归删除空子目录
     * @param : [in] path，路径
     * @return: true-操作成功; false-操作失败
     */
    static bool removeEmptyDirectory(const std::string &path, std::error_code &ec);

    /**
     * @brief : 递归删除目录或文件
     * @param : [in] path, 待目录或文件
     * @param : [out] std::error_code，保存错误信息
     * @return : true-成功；false-失败
     */
    static bool removeAll(const std::string &path, std::error_code &ec);
    static bool removeAll(const std::string &path);
    static bool rmRfAll(const std::string &path);

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
     * @brief : 组装文件名
     * @param : [in] path, 前缀路径
     * @param : [in] name, 后续文件或目录名
     * @return: 组装后的文件路径
     */
    static std::string buildFilename(const std::string &path, const std::string &name);
    static std::string buildFilename(const std::string &path, const std::string &name1, const std::string &name2);
    static std::string buildFilename(const std::string &path, const std::string &name1, const std::string &name2, const std::string &name3);

    /**
     * @brief : 将字符串str中的子串from替换为to
     * @param : [in | out] str, 原字符串
     * @param : [in] from, 需要替换的旧字符子串
     * @param : [in] to, 替换后的新子串
     * @return: 替换后的字符串引用，同str
     */
    static std::string &replace(std::string &str, const std::string &from, const std::string &to);

    /**
     * @brief : 递归创建目录
     * @param : [in] path, 目录
     * @param : [out] std::error_code，保存错误信息
     * @return : true-存在；false-不存在
     */
    static bool mkpath(const std::string &path, std::error_code &ec);
    static bool mkpath(const std::string &path);
    static bool mkpath(const std::string &path, unsigned int mode);

    /**
     * @brief : 在字符串数组v中查找是否包含字符串e
     * @param : [in] v, 字符串数组
     * @param : [in] e, 字符串
     * @return: true-包含；false-不包含
     */
    static bool contains(const std::vector<string> &v, const std::string &e);
    static bool contains(const std::string &src, const std::string &sub);

    /**
     * @brief : 下载url数据到文件dest
     * @param : [in] url，支持HTTP、HTTPS、FTP、FTPS协议
     * @param : [in] 目标文件名，默认则为url的basename
     * @return: true-下载成功；false-下载失败
     * @note  : 使用wget命令实现，优势：可利用其断点续传和代理
     */ 
    static bool downloadUrl(const std::string &url, const std::string &dest = "");
    static bool downloadUrlNoPrint(const std::string &url, const std::string &dest = "");

    /**
    * @brief : 查找可执行文件路径
    * @param : [in] exe，可执行文件名
    * @return: 可执行文件绝对路径
    * @note  : 只探索下面目录，如果需要请自行添加
    *          /usr/local/sbin
    *          /usr/local/bin
    *          /usr/bin
    *          /usr/sbin
    *          /bin
    *          /sbin
    */ 
    static std::string searchExecutable(const std::string &exe);

    /**
     * @brief :获取HASH MAP中的key是否存在
     */
    static bool ifContains(const std::unordered_map<string, bool> &map, string key);

    /**
     * @brief : 列举出路径下所有文件(不包含目录)
     * @param path : 
     * @return : 
     */
    static vector<string> enumerateDirectoryPath(const string &path, bool ignoreSymlink = false);

    /**
     * @brief :组合两个vector为一个
     */
    static vector<string> concatVectors(const vector<string> &vec1, const vector<string> &vec2);

    /**
     * @brief :获取符号链接（symlink）的目标路径
     */
    static std::string getSymlinkTarget(const std::string &path);

    /**
     * @brief :查看路径是否属于链接
     */
    static bool isStringSymlink(const std::string &pathString);

    /**
     * @brief 获取modlue name
     */
    static std::string getModuleName(const std::string path);

    /**
     * @brief 封装函数：将脚本内容写入 .sh 文件并设置可执行权限
     */
    static bool writeScriptToFile(const std::string& scriptContent, const std::string& filePath);
    
    /**
     * @brief 获取ASCII文档中的特定前缀的文本内容
     */
    static std::string readFile(const std::string &path, const std::string &prefix);

    /**
     * @brief: 组成KMRef结构体
     */
    static KMRef comToRef(const std::string &id, const std::string &kind, const std::string &channel, const std::string &name, const std::string &version, const std::string &module);

    static void concatVectors(std::vector<KMRef> &ref1, const std::vector<KMRef> &ref2);

    /**
     * @brief: 校验base或runtime是否匹配。字符串格式为{channel}:{id}/{version}
     */
    static bool checkEnvironment(const std::string &base1, const std::string &base2);

    /**
     * @brief:兼容版本号
     */
    static bool matchVersion(const std::string &version, const std::string &version_standard);

    /**
     * @brief: 判断字符串是否为数字
     */
    static bool isNumeric(const std::string &str);

    /**
     *@brief: 判断当前字符串是否为合法的现行版
     */
    static bool isValidPrerelease(const std::string &str);

    /**
     * @brief: 获取家目录
     */
    static std::string getHomeDir();

    /**
     * @brief: ID匹配
     */
    static bool matchID(const std::string &text, const std::string &pattern);

    /**
    * @brief : 校验appid是否合法
    * @param : [in] appName, appid
    * @param : [out] errMsg, 错误信息
    */ 
    static bool isValidAppName(const std::string &appName, std::string &errMsg);
    
    /*
     * @brief: 解析ASCII文件
     */
    static std::vector<std::string> parseTxt(const std::string &path);

    /**
     * @brief: 在字符串数组中找到特定前缀的字符串并返回
     */
    static std::string matchPreStr(const std::vector<std::string> &vers, const std::string &pre);

    /**
     * @brief: 删除字符串前面的空格或制表符
     */
    static std::string trimLeftStr(const std::string &str);

    /**
     * @brief: 删除字符串后面的逗号和空格
     */
    static std::string trimRightStr(const std::string &str);
    /**
     *@brief: 删除字符串前后空格或制表符或逗号
     */
    static std::string trimStr(const std::string &str);

    /**
     * @brief: 依据空格或制表符划分字符串
     */
    static std::vector<std::string> splitStrBlank(const std::string &str);

    /**
     * @brief: 获取字典的所有关键字key
     */
    static std::vector<std::string> keysFromMap(const std::map<std::string, std::string>&values);

    /**
     * @brief: 字符串数组写入yaml节点
     */
    static void vectorToYaml(YAML::Node &yamlNode, std::string key, std::vector<std::string> values);

    /**
     * @brief: map写入yaml节点
     */
    static void mapToYaml(YAML::Node &yamlNode, std::map<std::string, std::string>values);

    /**
     * @brief: 拷贝文件到指定目录，且跳过部分文件
     */

    static bool cpAll(const std::string &src, const std::string &dest);
    static bool copyFile(const std::string &src, const std::string &dest, const std::string &skip);

    /**
     * @brief : 使用rsync同步目录中的内容
     * @param : [in] src, 源目录
     * @param : [in] dest, 目标目录
     * @param : [in] excludes, 排除同步路径
     * @return : true-成功；false-失败
     */
    static bool rsyncDir(const std::string &src, const std::string &dest, const std::vector<std::string>& excludes = {});

    /**
     * @brief: 获取shell脚本执行命令的输出结果
     */
    static std::string executeCommand(const std::string &command);

    /**
     * @brief: 获取系统内核
     */
    static std::string getSysCore();

    /**
     * @brief 字符串转换为小写
     */
    static std::string toLowerUtf8(const std::string& str);

    /**
     * @brief: 判断是否为V1系统
     */
    static bool isV11();

    /**
     * @brief: 获取文件时间戳
     */
    static int getFileTimestamp(const std::string &filePath);

    /**
     * @brief: 根据时间戳判断文件是否过期
     * @param: filePath 文件路径
     * @param: expiredTime 过期时间
     */
    static bool isFileExpired(const std::string &filePath, int expiredTime);

    /**
     * @brief: deb版本号转换为kaiming版本号
     */
    static std::string versionDebToKaiming(std::string version);

    /**
     * @brief: 提取字符串符号前字符串
     */
    static std::string extractPre(const std::string &str);

    /**
     * @brief: 提取字符串前面数字
     */
    static std::string extractNumPre(const std::string &str);


};

#endif // !__BUILDER_FLOW_H__
