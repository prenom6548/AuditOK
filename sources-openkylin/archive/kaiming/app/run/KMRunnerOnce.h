/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMRUNNER_ONCEMORE_H__
#define __KMRUNNER_ONCEMORE_H__

#include <vector>
#include <string>

/**
 * @brief 开明多次运行类
 */
class KMRunnerOnce final
{
public:
    KMRunnerOnce() = default;
    ~KMRunnerOnce() = default;

    /**
     * @brief 再次运行应用
     * @param appid 应用ID
     * @param commandArgs 运行命令及参数
     * @param sockPath 套接字路径
     * @param envs 环境变量
     * @return true 成功 false 失败
    */
    static bool run(const std::string &appid, const std::string &commandArgs, const std::string &sockPath, const std::vector<std::string> &envs);

    /**
    * @brief 应用是否已运行
    * @param appid 应用ID
    * @param containerid 容器ID
    * @return true 成功 false 失败
    */
    static bool isAlreadyRun(const std::string &appid, std::string &containerid);

    /**
     * @brief 运行命令
     * @param options 主命令参数
     * @param soptions 子命令参数
     * @param cid 容器ID
     * @param cmd 运行命令
     * @return true 成功 false 失败
     */
    static bool runCmd(const std::vector<std::string> &options, const std::vector<std::string> &soptions, const std::string &cid, const std::string &cmd = "");

    /**
     * @brief 组装二次运行命令
     * @param sockPath 套接字路径
     * @param envs 环境变量
     * @param cmd 输出的命令字符串
     * @return true 成功 false 失败
    */
    static bool assembleRunOnceOptions(const std::string &sockPath, const std::vector<std::string> &envs, std::vector<std::string> &args);
};

#endif