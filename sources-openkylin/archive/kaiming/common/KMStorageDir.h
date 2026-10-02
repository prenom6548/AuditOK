/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMDIR_H__
#define __KMDIR_H__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "KMInfoJson.h"
#include "kmtranslation.h"
#include "KMCommonTypePrivate.h"

namespace fs = std::filesystem;

struct KMDeploy
{
    KMRef m_ref;
    std::string m_dir;
    std::shared_ptr<KMInfoJson> m_infoJson;
};

/**
 * @brief : 主要用于封装kaiming包存储目录的相关操作
 */
class KMStorageDir
{
public:
    KMStorageDir();
    KMStorageDir(const KMStorageDir &other);
    ~KMStorageDir();

    /**
     * @brief : 获取开明包系统级安装的KMStorageDir对象
     * @return: 开明包系统级安装的KMStorageDir对象
     */
    static KMStorageDir getSystemDefaultDir();

    /**
     * @brief : 获取开明包用户安装目录KMStorageDir对象
     * @return: 开明包用户用户安装目录KMStorageDir对象
     */
    static KMStorageDir getUserDir();

    /**
     * @brief : 获取KMStorageDir对象base路径
     */
    std::string getBasePath();

    /**
     * @brief : 判断KMStorageDir对象是否空对象
     */
    explicit operator bool() { return d.use_count() > 0; }

    /**
     * loadDeployed:
     * @brief : 根据ref信息查询开明包是否部署
     * @param : [in] ref, 开明包基础信息
     * @return: 返回部署信息
     */
    std::shared_ptr<KMDeploy> loadDeployed(const KMRef &ref);

public:
    /**
     * @brief : 获取开明包系统级安装的base目录
     * @return: 开明包系统级安装的base目录
     */
    static std::string getSystemDefaultBaseDirLocation();

    /**
     * @brief : 获取开明包用户数据base目录
     * @return: 开明包用户数据base目录
     */
    static std::string getUserBaseDirLocation();

    /**
     *  @brief: 获取当前登录用户
     */
    static std::string getLoginUser();

    /**
     * @brief Get the Login User Primary Group object
     * 
     * @return std::string user primary group
     */
    static std::string getLoginUserPrimaryGroup();

    /**
     * @brief : 获取开明包用户devel数据目录
     * @return: 开明包用户devel数据目录
     */
    static std::string getUserDevelDataDirLocation();
    /**
     * @brief : 获取用户数据目录
     * @return: 用户数据目录
     */
    static std::string getUserDataDirLocation();

    /**
     * @brief : 获取应用数据目录
     * @param : [in] id，应用id
     * @return: 数据目录
     */
    static std::string getDataDir(const std::string &id);

    /**
     * @brief : 根据开明包ref信息获取开明包安装的base目录
     * @param : [in] ref, 开明包ref信息
     * @return: 开明包安装的base目录
     * @note  : 目前开明包仅支持系统级安装
     */
    static std::string getRefBaseDirLocation(const KMRef &ref);

    /**
     * getDeployedDir:
     * @brief : 根据ref信息获取开明包部署路径
     * @param : [in] ref, 开明包基础信息
     * @param : [out] ref, 会更新里面的commit信息（主要是将active>具体的commitid）
     * @return: 开明包部署路径
     */
    static std::string getDeployedDir(const KMRef &ref);

    /**
     * @description: 获取目标用户的 XDG_RUNTIME_DIR
     * @param {string} &username 用户名
     * @return {*}
     */
    static std::string getXdgRuntimeDir(const std::string &username);

    /**
     * @brief : 获取用户目录
     * @return: 用户目录
     */
    static std::string getUserDirLocation();

    /**
     * @brief 更改目录所有者
     * @param path 要更改的目录
     */
    static void modifyDirOwnership(const fs::path &path);

private:
    KMStorageDir(const std::string &path, bool user);

    class Private;
    std::shared_ptr<Private> d;
};

#endif // !__KMDIR_H__
