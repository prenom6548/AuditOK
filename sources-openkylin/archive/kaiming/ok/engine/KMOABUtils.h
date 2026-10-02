/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABUTILS_H__
#define __KMOABUTILS_H__

#include <string>
#include <vector>
#include <system_error>

class KMOABUtils
{
public:
    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序名称，不包含路径
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @param : [in] ignoreExitStatus，是否忽略外部程序退出状态；true-忽略; false-不忽略
     * @return: true - 成功执行；false - 出现错误
     */
    static bool spawn(const std::string &exec, const std::vector<std::string> &args = {}, bool ignoreExitStatus = false);

    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序名称，不包含路径
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
     * @return: true - 成功执行；false - 出现错误
     */
    static bool spawn(const std::string &exec, const std::vector<std::string> &args, std::string &result);

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
     * isEmptyDirectory:
     * @brief : 判断路径是否空目录，使用前先判断是否是目录
     * @param : [in] path，路径
     * @return: true-空目录或不存在; false-不是空目录或不是目录
     */
    static bool isEmptyDirectory(const std::string &path);

    /**
     * canonical:
     * @brief : 规范化路径，绝对路径并路径中不包含.和..
     * @param : [in] path，路径
     * @return: 规范化后的路径
     */
    static std::string canonical(const std::string &path);

    /**
     * @brief : 强制递归删除目录或文件
     * @param : [in] path, 待目录或文件
     * @return : true-成功；false-失败
     */
    static bool rmRfAll(const std::string &path);

    /**
     * @brief : 目录或文件拷贝
     * @param : [in] src, 源目录或文件
     * @param : [in] dest, 目标目录或文件
     * @return : true-成功；false-失败
     */
    static bool copyAll(const std::string &src, const std::string &dest);

    /**
     * @brief : 移动文件或重命名文件
     * @param : [in] src, 源目录或文件
     * @param : [in] dest, 目标目录或文件
     * @return : true-成功；false-失败
     */
    static bool mv(const std::string &src, const std::string &dest);

    /**
     * @brief : 获取文件sha256校验和
     * @param : [in] file，文件路径
     * @return: 文件内容校验和
     */ 
    static std::string getFileSha256sum(const std::string &file);

    /**
     * @brief : 查找可执行文件路径
     * @param : [in] exe，可执行文件名
     * @return: 可执行文件绝对路径
     * @note  : 根据环境变量PATH中的路径查询
     */ 
    static std::string searchExecutable(const std::string &exe);

    static std::string generateUUID();
};

#endif // !__KMOABUTILS_H__
