/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMDBUSJSON_H__
#define __KMDBUSJSON_H__

#include <string>
#include <nlohmann/json.hpp>
#include "KMJsonHelper.h"

/**
 * @brief : 处理dbus json业务类
 */ 
class KMDbusJson
{
public:
    KMDbusJson() = default;
    ~KMDbusJson() = default;

    /**
     * @brief : 封装本地包的信息为json字符串
     */
    static std::string packLocalInfoToJson(const std::string &id, const std::string &name, const std::string &version, const std::string &size, const std::string &branch, const std::string &arch, const std::string &module, const std::string &description);
    /**
     * @brief : 封装在线进度条数据为 JSON 字符串
     */
    static std::string packProgressInfoToJson(const std::string &refId, int progress, const std::string &speed, int timeout);
    /**
     * @brief : 封装离线进度条数据为 JSON 字符串
     */
    static std::string packProgressOffInfoToJson(const std::string &refId, int progress);
 
    /**
     * @brief : 封装本地不存在的包信息数据为 空JSON 字符串
     */
    static std::string errorLocalJson();
    /**
     * @brief : 封装远程包 路径 信息数据为 JSON 字符串
     */
    static std::string parseRemotePathToJson(std::string& stringPath);
    /**
     * @brief : 封装远程不存在的包信息数据为 空JSON 字符串
     */
    static std::string errorRemoteJson();

    static std::string packRemoteStatusToJson(const std::string &updateTime, bool signStatus, const std::string &signCN);

};

#endif // !__KMDBUSJSON_H__
