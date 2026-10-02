/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KM_DIR_UTILS_H_
#define KM_DIR_UTILS_H_

#include <string>

#include "KMCommonTypePrivate.h"

/**
 * @brief : 用于封装环境变量相关操作
 */
class KMDirUtils
{
public:
    KMDirUtils() = default;
    ~KMDirUtils() = default;

public:
    /**
     * @brief 获取当前工作目录
     * @param dir 返回当前工作目录路径
     * @return true 成功, false 失败
     */
    static bool getCwdDir(std::string &dir);

    /**
     * @brief 获取系统架构目录 
     * @return 系统架构目录字符串
     */
    static std::string getLibraryDir();

    /**
     * @brief 获取家目录
     * @param home 家目录 
     * @return true 成功 false 失败 
    */
    static bool getHomeDir(std::string &homeDir);

    /**
     * @brief 获取开明包目录
     * @return 开明包目录
     */
    static std::string getKmDefDir();

    /**
     * @brief 根据ref信息获取开明包部署路径
     * @param ref开明包基础信息
     * @return true 成功 false 失败
     */
    static bool getDeployedDir(const KMRef &ref, std::string &dir);

    /**
     * @brief 将后面的std::vector中的元素追加到前面std::vector中
     * @param src追加元素后的std::vector
     * @param dest待追加的std::vector
     */
    static void append(std::vector<KMRef> &src, std::vector<KMRef> &&dest);

    /**
     * @brief 获取xdg runtime路径
     * @param dir, 返回xdg runtime路径
     * @return true 成功 false 失败
     */
    static bool getXdgRuntime(std::string &dir);

    /**
     * @brief 获取pulse runtime路径 
     * @param dir, 返回pulse runtime路径
     * @return true 成功 false 失败
     */
    static bool getPulseRuntimeDir(std::string &dir);

    /**
     * @brief 获取时区
     * @return 返回时区信息
     */
    static std::string getTimeZone();

    /**
     * @brief 获取用户data目录
     * @return 返回用户data目录
     */
    static std::string getUserDataDir();

    /**
     * @brief 获取用户缓存目录
     * @return 返回用户缓存目录
     */
    static std::string getUserCacheDir();

    /**
     * @brief 获取登录用的uid
     * @param uid 用户ID
     * @return true 存在 false 不存在
     */
    static bool getLoginUid(uid_t &uid);

private:
    /**
     * @brief 获取系统内核架构
     * @return 系统内核架构字符串
     */
    static std::string getKernelArch();
};

#endif // KM_DIR_UTILS_H_