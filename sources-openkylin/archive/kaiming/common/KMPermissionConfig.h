/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMPERMISSIONCONFIG_H
#define KMPERMISSIONCONFIG_H

#include <memory>
#include <set>
#include <nlohmann/json.hpp>

#include "common/KMSingleton.h"

struct AppPermissionConfig
{
    bool xdgUserDirs = false;
    bool filesystemDirs = false;
    bool network = false;
    bool scanning = false;
    bool bluetooth = false;
    bool microphone = false;
    bool camera = false;
    bool usb = false;
    bool serial = false;
    bool autostart = false;
    bool systemNotice = false;
    bool systemTime = false;
    bool systemServer = false;
    bool windowTop = false;
    bool preventsleep = false;
    bool systemDisplayResolution = false;
    bool systemDisplayBrightness = false;
    bool systemAudioSettings = false;
    bool systemAudioSize = false;
    bool systemCrontab = false;
    bool systemPowerMode = false;
    bool systemPowerPolicy = false;
};

/**
 * @brief permission config
 */
class KMPermissionConfig
{
public:
    KMPermissionConfig(const std::string &id);
    ~KMPermissionConfig();

    /**
     * @brief : 加载json文件
     * @param : [in] jsonFile, json文件
     * @return: nlohmann::json, 解析后的json对象
     * @throw : KMException
     */
    bool loadFile(const std::string &metafile);

    /**
     * @brief　加载配置文件
     */
    bool loadConfiguration();

    /**
     * @brief 解析配置文件
     */   
    AppPermissionConfig getPermissionConfig();
    
    /**
     * @brief 是否存在管控配置
     * @return true 成功，false 失败
     */
    bool isExist();

private:
    class Private;
    std::shared_ptr<Private> d_ptr;
    nlohmann::json m_root;
};

#endif
