/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef __KMRUNNER_MOUNT_H__
#define __KMRUNNER_MOUNT_H__

#include <string>
#include <vector>

class KMContainer;

class KMRunnerMount
{
public:
    /**
     * @brief 挂载overlay
     * @param lowers目录
     * @param upper目录
     * @param merge目录
     * @return true 成功 false 失败
     */
    static bool mountOverlay(KMContainer *container, const std::string &appDataPath, const std::vector<std::string> &lowers, const std::string &upper, const std::string &merge, const std::vector<std::string> &mountOptions = {});

    /**
     * @brief 挂载指定目
     */
    static void mountDir(KMContainer *container, const std::string &appDataPath, bool isBindHostHome, const std::string &hostDir, const std::string &containDir, const std::string &type, std::vector<std::string> &options);

private:
    /**
     * @brief 追加lowerdir
     * @param lowerdir lowerdir
     * @param dir 追加的目录
     */
    static void appendLowerDir(std::string &lowerdir, const std::string &dir);
};

#endif
