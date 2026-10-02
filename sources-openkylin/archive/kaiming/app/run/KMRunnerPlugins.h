/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMRUNNERPLUGINS_H__
#define __KMRUNNERPLUGINS_H__

#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include "common/KMInfoJson.h"

class KMContainer;
/**
 * @brief 插件挂载处理模块
 */
class KMRunnerPlugins
{
public:
    /**
     * @brief 挂载插件
     */
    bool mountPlugins(KMContainer *container, const std::string &appDataPath, const Annotations &annotations, const std::string &appDir);

    /**
    * @brief 处理插件依赖并进行挂载
    * @return true 存在 false 不存在
    */
    void handlePluginDependencies(KMContainer *container, const std::string &appDataPath);

private:
    /**
    * @brief 扫描插件目录中的软链接，并返回解析后的真实路径列表
    * @return 所有软链接的真实路径
    */
    std::vector<std::string> scanSymlinkParents();

    /**
     * @brief 处理/opt是软链接
     * @return true 存在 false 不存在
     */
    std::string normalizeOptPath(const std::string &path);

    /**
     * @brief 叠加宿主机插件目录
     * @return true 挂载成功 false 挂载失败
     */
    bool overlayHostPlugins(KMContainer *container, const std::string &appDataPath, const std::string &appDir);

    /**
     * @brief 挂载 VFS 预加载库
     */
    bool mountVfsPreloadLibrary(KMContainer *container, const std::string &appDataPath);

    /**
     * @brief 处理 opt 目录下的插件
     */
    static void processOptDirectory(KMContainer *container, const std::string &appDataPath, const std::string &appDir, const std::string &pluginsRoot, const std::string &targetDir);

    /**
     * @brief 挂载插件目录下的插件
     */
    static void mountOverlayForDir(KMContainer *container, const std::string &appDataPath, const std::string &appDir, const std::string &rootDir, const std::string &targetDir);
};

#endif
