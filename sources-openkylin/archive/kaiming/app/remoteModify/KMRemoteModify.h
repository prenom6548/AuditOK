/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMRemoteModify_H__
#define __KMRemoteModify_H__

#include <string>
#include <vector>
#include <memory>
#include "common/KMEnv.h"
#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"

class KMRemoteModify : public KMSubCommand
{
    DECLARE_DYNCREATE(KMRemoteModify);

public:
    KMRemoteModify();
    ~KMRemoteModify() override;

    int dispose(int argc, char **argv) override;

    /**
     * @brief 修改结果
    */
    struct ModifyResult
    {
        bool success;
        std::string message;
        int code;
    };

    /**
     * @brief 远程仓库配置信息
   */
    struct RemoteRepo
    {
        std::string name;
        std::string url;
        bool gpgVerify{false};
        bool gpgVerifySummary{false};
    };

    /**
      * @brief OSTree 配置信息
    */
    struct OstreeConfig
    {
        std::string repoVersion;
        std::string mode;
        std::string minFreeSpaceSize;
        std::vector<RemoteRepo> remoteRepos;
    };

   /**
    * @brief 添加远程仓库
    * @param configPath 配置文件路径
    * @param repoName 仓库名
    * @param repoUrl 仓库URL
    * @param gpgVerify GPG验证
    * @param gpgVerifySummary GPG摘要验证
    * @return 操作结果
    */
    static ModifyResult addRemote(const std::string &configPath, const std::string &repoName, const std::string &repoUrl, bool gpgVerify = false, bool gpgVerifySummary = false);

   /**
    * @brief 删除远程仓库
    * @param configPath 配置文件路径
    * @param repoName 仓库名
    * @return 操作结果
   */
    static ModifyResult removeRemote(const std::string &configPath, const std::string &repoName);

   /**
    * @brief 修改远程仓库URL
    * @param configPath 配置文件路径
    * @param repoName 仓库名
    * @param newUrl 新的URL
    * @return 操作结果
   */
    static ModifyResult modifyRemoteUrl(const std::string &configPath, const std::string &repoName, const std::string &newUrl);

   /**
    * @brief 列出所有远程仓库
    * @param configPath 配置文件路径
    * @param repos 输出的仓库列表
    * @return 操作结果
   */
    static ModifyResult listRemotes(const std::string &configPath, std::vector<RemoteRepo> &repos);

   /**
    * @brief 修改核心配置
    * @param configPath 配置文件路径
    * @param repoVersion 仓库版本
    * @param mode 模式
    * @param minFreeSpaceSize 最小空闲空间
    * @return 操作结果
   */
    static ModifyResult modifyCoreConfig(const std::string &configPath, const std::string &repoVersion, const std::string &mode, const std::string &minFreeSpace);

   /**
    * @brief 解析 OSTree 配置文件
    * @param configPath 配置文件路径
    * @param config 输出的配置信息
    * @return 解析结果
   */
    static ModifyResult parseConfig(const std::string &configPath, OstreeConfig &config);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // KMRemoteModify_H