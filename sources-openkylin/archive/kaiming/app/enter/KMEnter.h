/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMEnter_H__
#define __KMEnter_H__

#include <optional>
#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/KMDbusProxyCall.h"

/**
 * @brief ``kaiming Enter`` 命令可以查看运行中的应用。
 */
class KMEnter : public KMSubCommand
{
    DECLARE_DYNCREATE(KMEnter);

public:
    KMEnter();
    ~KMEnter() override;

    /**
     * @brief 在正在运行的容器中运行命令
     * @param argc 参数个数
     * @param argv 参数列表
     * @return int 0:成功,非0:失败
     */
    int dispose(int argc, char **argv) final;

    /**
     * @brief 开始运行
     * @return true 成功 false 失败
     */
    bool start();

private:
    /**
     * @brief 初始化
     */
    void init();

    /**
     * @brief 获取容器id
     * @param cid 命令
     * @return true 成功 false 失败
     */
    bool getCid(std::string &cid);

    /**
     * @brief 调用dbus 进入容器 
     * @param cid 容器id
     * @return true 运行成功 false 运行失败
     */
    bool enter(std::string &cid);

    /**
     * @brief 组装运行参数
     * @return 返回参数列表
     */
    std::vector<std::string> assembleEnterOpts();

    /**
     * @brief 等待结束
     * @return true 成功 false 失败
    */
    bool wait();

    /**
      * @brief 解析命令行参数
      * @param argc 参数个数
      * @param argv 参数列表
      * @return true 成功 false 失败
      */
    bool parseArgs(int argc, char **argv);

    /**
      * @brief 创建socket服务套接字
      * @return true 创建成功 false 创建失败
     */
    bool startUnixSocketServer();

private:
    class Options;
    class Private;
    std::shared_ptr<Private> d_ptr;
};

#endif // __KMEnter_H__
