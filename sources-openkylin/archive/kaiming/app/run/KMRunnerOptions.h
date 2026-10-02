/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMRUNNEROPTIONS_H__
#define __KMRUNNEROPTIONS_H__

#include "common/KMBuildinOptions.h"

/**
 * @brief 开明运行命令行参数解析
 */
class KMRunnerOptions : public KMOption::Options
{
public:
    KMRunnerOptions();
    ~KMRunnerOptions() override = default;

    /**
     * @brief 获取启动命令
     * @return string 返回启动命令
     */
    std::string getCommand() const;

    /**
     * @brief 获取appid
     * @return string 返回appid
     */
    std::string getAppId() const;

    /**
     * @brief 获取启动参数
     * @return vector<string> 返回启动参数
     */
    const std::vector<std::string> &getArgs() const;

    /**
     * @brief 获取devel开发运行时
     * @return bool true 开发运行时, false 非开发运行时
     */
    bool isDevel() const;

    /**
     * @brief 获取是否启动文件转发
     * @return bool true 启动, false 不启动
     */
    bool isFileWarding() const;

    /**
     * @brief 获取是否分离参数
     * @return bool true 分离, false 不分离
    */
    bool isDetach() const;

protected:
    /**
     * @brief 命令行参数解析后操作
     */

    void preParseHook() override;
    /**
     * @brief 命令行参数解析后操作
     */

    void postParseHook() override;
    /**
     * @brief 命令行参数添加描述选项
     */
    void addOptions();

private:
    class Private;
    std::shared_ptr<Private> d_ptr;
};
#endif
