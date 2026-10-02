/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMREFERENCE_H__
#define __KMREFERENCE_H__

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "common/KMInfoJson.h"

class KMReference final
{
public:
    KMReference();
    ~KMReference() = default;

    /**
     * @brief 获取目录
     * @param id 包名
     * @return true 成功，false 失败
     */
    bool getLayerDir(const std::string &id, bool isDevel);

    /**
     * @brief 获取app目录
     * @param appDir app目录
     * @return app目录
     */
    bool getAppDir(std::string &appDir);

    /**
     * @brief 获取base目录
     * @param baseDir base目录
     * @return base目录
     */
    bool getBaseDir(std::string &baseDir);

    /**
     * @brief 获取runtime目录
     * @param runtimeDir runtime目录
     * @return runtime目录
     */
    bool getRuntimeDir(std::string &runtimeDir);

    /**
    * @brief 获取Version
    * @param version 版本号
    * @return true 成功，false 失败
     */
    bool getVersion(std::string &version) const;

    /**
     * @brief 获取Commands命令
     * @param commands 命令列表
     * @return true 成功，false 失败
     */
    bool getCommands(std::vector<std::string> &commands);

    /**
     * @brief 获取appId
     * @param appid app名称
     * @return true 成功，false 失败
     */
    bool getAppId(std::string &appid);

    /**
     * @brief 获取架构
     * @param appid 架构名称
     * @return true 成功，false 失败
     */
    bool getArch(std::string &arch);

    /**
     * @brief 获取环境变量列表
     * @param envs 环境变量列表
     * @return true 成功，false 失败
     */
    bool getEnvs(std::map<std::string, std::string> &envs);

    /**
     * @brief 环境变量合并，环境变量优先级：base > runtime > app且处理追加环境变量例如$PATH=/usr:$PATH
     * @param envs 环境变量列表
     * @param newEnvs 追加环境变量列表
     */
    void appendEnvs(std::map<std::string, std::string> &envs, const std::map<std::string, std::string> &newEnvs);

    /**
     * @brief 获取权限管控信息
     * @return 权限管控
     */
    Permissions getPermissions() const;

    /**
     * @brief 获取自定义信息
     * @return 自定义结构
     */
    Annotations getAnnotations() const;

private:
    /**
     * @brief 解析包名
     * @param id 包名
     * @return KMReference
     */
    bool parse(const std::string &id);

    /**
     * @brief 获取app, base, runtime的目录
     * @return true 成功，false 失败
     */
    bool getPackageDir(bool isDevel);

    /**
     * @brief 检查权限管控信息
     * @return true 成功，false 失败
     */
    bool ensurePerms();

private:
    class Private;
    std::shared_ptr<Private> d_ptr;
};

#endif // __KMREFERENCE_H__