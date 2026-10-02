/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMCRUN_H__
#define __KMCRUN_H__

#include <string>
#include <optional>
#include <vector>
#include <map>
#include <variant>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "common/KMLogger.h"
#ifdef CPPUNIT_TEST
#define private public
#endif

namespace fs = std::filesystem;

class KMCrun final
{
public:
    KMCrun() = default;
    virtual ~KMCrun() = default;

public:
    /**
     * @brief 执行run命令 
     * @param options 命令行参数
     * @return true 成功 false 失败
     */
    static bool exec(const std::vector<std::string> &options, const bool &isWait = true);

    /**
     * @brief 执行run命令 
     * @param options 命令行参数
     * @param output 输出
     * @return true 成功 false 失败
     */
    template<typename T>
    static bool exec(const std::vector<std::string> &options, T &output)
    {
        std::vector<const char *> args;
        if (!getArgs(options, args))
            return false;

        bool ret = false;
        std::variant<bool, std::string> varOutput = std::string("");
        if (!(ret = doCmd(args, varOutput, true)))
            return ret;

        std::string outputStr = std::get<std::string>(varOutput);
        if (outputStr.empty())
        {
            output = {};
            return true;
        }

        try
        {
            nlohmann::json j = nlohmann::json::parse(outputStr);
            //    output = j;
            from_json(j, output);
        }
        catch (const std::exception &e)
        {
            KMError("json pase error: " + std::string(e.what()));
            return false;
        }

        return ret;
    }

    /**
     * @brief 获取crun环境
     *
     * @return std::string 类型crun可执行文件地址，如“/usr/bin/crun”
     */
    static std::string getCrun();

private:
    /**
     * @brief 执行命令
     * @param processArgs 进程参数
     * @param output 输出
     * @return true 成功 false 失败
     */
    static bool doCmd(const std::vector<const char *> &processArgs, std::variant<bool, std::string> &output, const bool &isWait);

    /**
     * @brief 获取进程参数
     * @param options 命令行参数
     * @param args 进程参数
     * @return true 成功 false 失败
     */
    static bool getArgs(const std::vector<std::string> &options, std::vector<const char *> &args);
};

#endif // __KMCRUN_H__
