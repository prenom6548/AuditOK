/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMFILEUTILS_H__
#define __KMFILEUTILS_H__

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include <set>

#include "KMStringUtils.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

typedef enum
{
    FILE_REPLACE_DATASYNC_NEW = (1 << 0),
    FILE_REPLACE_NODATASYNC = (1 << 1),
    FILE_REPLACE_INCREASING_MTIME = (1 << 2),
} FileReplaceFlags;

typedef enum
{
    LINK_TMPFILE_REPLACE,
    LINK_TMPFILE_NOREPLACE,
    LINK_TMPFILE_NOREPLACE_IGNORE_EXIST
} LinkTmpfileReplaceMode;

class KMFileUtils
{
public:
    /**
     * @brief 获取文件时间戳
     * 
     * @param filePath 文件路径
     * @return int 文件时间戳
     */
    static int getFileTimestamp(const std::string &filePath);

    /**
     * @brief 判断文件是否过期
     * 
     * 比较当前时间戳和文件时间戳大小，如果当前时间戳大于文件时间戳，并且时间差大于1小时，则返回true，否则返回false
     * 
     * @param filePath 文件路径
     * @param expiredTime 过期时间，单位为秒
     * @return true 文件过期
     * @return false 文件未过期
     */
    static bool isFileExpired(const std::string &filePath, int expiredTime);

    /**
     * @brief 判断文件是否是 json 文件
     * 
     * @param filePath 文件路径
     * @return true 是 json 文件
     * @return false 不是 json 文件
     */
    static bool isJsonFile(const std::string &filePath);

    /**
     * @brief 判断文件是否存在
     * 
     * @param fileName 文件名
     * @return true 文件存在
     * @return false 文件不存在
     */
    static bool fileExists(const std::string &fileName);

    /**
     * @brief 获取当前时间的函数
     * 
     * @return std::string 
     */
    static std::string getCurrentTime();

    /**
     * @brief 创建文件或者清空已有文件并写入当前时间
     * 
     * @param fileName 
     * @param fileName 文件名
     * @return true 文件操作成功
     * @return false 文件操作失败
     */
    static bool createFileIfNotExist(const std::string &fileName);

    /**
     * isEmptyDirectory:
     * @brief : 判断路径是否空目录，使用前先判断是否是目录
     * @param : [in] path，路径
     * @return: true-空目录或不存在; false-不是空目录或不是目录
     */
    static bool isEmptyDirectory(const std::string &path);

    /**
     * @brief: 递归查询目录是否为空
     * @param: [in] path, 路径
     * @return: true-空目录或不存在; false-不是空目录或不是目录
     */
    static bool isEmptyDir(const std::string &path);

    /**
     * removeEmptyDirectory:
     * @brief : 递归删除空子目录
     * @param : [in] path，路径
     * @return: true-操作成功; false-操作失败
     */
    static bool removeEmptyDirectory(const std::string &path, std::error_code &ec);

    /**
     * @brief : 目录或文件是否存在
     * @param : [in] path, 待目录或文件
     * @return : true-存在；false-不存在
     */
    static bool pathExists(const std::string &path);

    /**
     * @brief : 获取文件或目录中文件大小
     */
    static uint64_t getDirectorySize(const fs::path &dir);

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
     * @brief : 递归删除目录或文件
     * @param : [in] path, 待目录或文件
     * @param : [out] std::error_code，保存错误信息
     * @return : true-成功；false-失败
     */
    static bool removeAll(const std::string &path, std::error_code &ec);
    static bool removeAll(const std::string &path);

    /**
     * @brief : 强制递归删除目录或文件
     * @param : [in] path, 待目录或文件
     * @return : true-成功；false-失败
     * @note  : 有些场景KMFileUtils::removeAll有无法删除的目录，使用KMFileUtils::rmRfAll可以删除。如overlay中的workdir
     */
    static bool rmRfAll(const std::string &path);

    /**
     * @brief : 获取路径的真实路径
     * @param : [in] path，路径
     * @return: 真实路径,软链接会替换为其连接路径
     */
    static std::string realpath(const std::string &path);

    /**
     * @brief : 规范化文件路径
     * @param : [in] path，原始文件路径
     * @return: 规范化后的文件路径
     */
    static std::string canonicalizeFilename(const std::string &path);

    /**
     * @brief : 生成临时文件名
     * @param : [in | out] tmp，临时文件名，必需如下形式的:*.XXXXXX
     * @return: 是否成功
     */
    static bool genTempName(std::string &tmp);

    static std::string kmGenerateTempName(const std::string &tempName);

    /**
     * @brief : 获取目录所有文件md5校验和（包含子孙目录）
     * @param : [in] dir，目录路径
     * @return: 目录所有文件md5校验和
     * @note  : 此方法废弃，替换为KMDirMd5sum::md5sum
     */
    static std::string getDirMd5sum(const std::string &dir);

    /**
     * @brief : 判断是否废弃的md5计算方式的结果
     * @param : md5sumFile，md5校验和文件
     * @return: true-废弃的；false-未废弃的
     */
    static bool isObsolete(const std::string &md5sumFile);

    /**
     * @brief : 获取文件sha256校验和
     * @param : [in] file，文件路径
     * @return: 文件内容校验和
     */
    static std::string getFileSha256sum(const std::string &file);

    /**
     * @brief : 获取文件md5校验和
     * @param : [in] file，文件路径
     * @return: 文件内容校验和
     */
    static std::string getFileMd5sum(const std::string &file);

    /**
     * @brief 读取文件内容
     * @param fileName 文件名，包含路径
     * @param content 文件内容
     * @return bool true成功 false失败
     */
    static bool readFile(const std::string &fileName, std::string &content);

    /**
     * @brief 立即写文件
     * @param fileName 文件名，包含路径
     * @param content 文件内容
     * @return bool
     */
    static bool syncWriteFile(const std::string &fileName, const std::string &content);

    /**
     * @brief : 获取目录下所有的文件列表（包含子目录下的文件）
     * @param : [in] dir，路径
     */
    static std::vector<std::string> getDirFiles(const fs::path &dir);

    /**
     * findFileInTree:
     * @brief : 在目录树中查找文件是否存在
     * @param : [in] baseDir, 查找的目标目录
     * @param : [in] filename，待查找的文件名
     * @return: true-找到; false-未找到
     */
    static bool findFileInTree(const std::string &baseDir, const std::string &filename);

    /**
     * @brief : 使用cp实现目录或文件拷贝
     * @param : [in] src, 源目录或文件
     * @param : [in] dest, 目标目录或文件
     * @return : true-成功；false-失败
     */
    static bool cpRfAll(const std::string &src, const std::string &dest);
    static bool cpRfAllStar(const std::string &src, const std::string &dest);

    /**
     * @brief : 使用rsync同步目录中的内容
     * @param : [in] src, 源目录
     * @param : [in] dest, 目标目录
     * @param : [in] excludes, 排除同步路径
     * @return : true-成功；false-失败
     */
    static bool rsyncDir(const std::string &src, const std::string &dest, const std::vector<std::string>& excludes = {});

    /*
     * @brief: 解析ASCII文件
     * @brief: [in] path, 源文件目录
     */
    static std::vector<std::string> parseTxt(const std::string &path);

    /**
     * @brief: 判断是否为不可变系统
     */
    static bool isImmutableSystem();

    /**
     * @description: 判断是不是可执行文件
     * @param {path} &path 传入的文件路径
     * @return true: 是
     */
    static bool isExecutableByFileCmd(const fs::path &path);

    /**
     * @description: 判断是不是脚本文件
     * @param {path} &path 传入的文件路径
     * @return {*}
     */
    static bool isShellScriptByFileCmd(const fs::path &path);

    /**
     * @description: 排除非可执行文件类型
     * @param {path} &path 传入的文件路径
     * @return {*}
     */
    static bool isNonExecExt(const fs::path &path);

    /**
     * @description: 根据文件类型拷贝
     * @param {path} &sourceDir 源目录
     * @param {path} &destDir  目标目录
     * @param {string} &extension 文件后缀
     * @param {string} &fileType  文件类型
     * @param {set<std::string>} &installedFiles 安装文件列表
     * @return {*}
     */
    static std::string copyFilesByExtension(const fs::path &sourceDir, const fs::path &destDir, const std::string &extension, const std::string &fileType, std::set<std::string> &installedFiles);

    static bool syncWithExclusions(const fs::path& source, const fs::path& destination, const std::vector<std::string>& excludePatterns);
};

#endif //!__KMFILEUTILS_H__
