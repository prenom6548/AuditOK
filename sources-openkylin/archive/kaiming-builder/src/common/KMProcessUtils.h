/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMPROCESS_H__
#define __KMPROCESS_H__

#include <string>
#include <vector>

/**
 * @brief : 封装子进程相关函数，以替代boost库
 */ 
class KMProcessUtils
{
public:
    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @return: true - 成功执行；false - 出现错误
    */
    static bool spawn(const std::string &exec, const std::vector<std::string> &args = {}, bool ignoreExitCode = true);

    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序
     * @param : [in] workingDirectory, 工作目录
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @return: true - 成功执行；false - 出现错误
    */
    static bool spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args = {}, bool ignoreExitCode = true);

    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序名称，不包含路径
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
     * @return: true - 成功执行；false - 出现错误
     */
    static bool spawn(const std::string &exec, const std::vector<std::string> &args, std::string &result, bool ignoreExitCode = false);

    /**
     * @brief : 执行外部程序
     * @param : [in] exec, 外部程序名称，不包含路径
     * @param : [in] workingDirectory, 执行外部程序工作目录
     * @param : [in] args, 执行外部程序需要传入的参数列表
     * @param : [out] result, 外部程序执行时的标准输出和标准错误信息
     * @return: true - 成功执行；false - 出现错误
     */
    static bool spawn(const std::string &exec, const std::string &workingDirectory, const std::vector<std::string> &args, std::string &result, bool ignoreExitCode = false);

    /**
     * @brief : 判断参数是否需要用引号括起来
     * @return: true - 需要；false - 不需要
     */ 
    static bool argumentNeedsQuoting(const char *arg);
};

#endif // !__KMPROCESS_H__
