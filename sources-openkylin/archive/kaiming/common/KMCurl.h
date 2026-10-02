/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef KMCURL_H
#define KMCURL_H

#include <curl/curl.h>
#include <zlib.h>

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "common/KMConfig.h"
#include "common/KMFolder.h"
#include "common/KMMacros.h"
#include "common/KMStorageDir.h"
#include "common/KMUtil.h"

/**
 * @brief 封装 Curl 下载接口
 * 
 */
class KMCurl
{
public:
    KMCurl();
    ~KMCurl();

    /**
     * @brief 通过 curl api 下载文件
     *
     * @param url 下载的文件链接
     * @param outputFile 存储到本地的文件名
     * @return true 下载成功
     * @return false 下载失败
     */
    static bool curlApiDownload(const string& url, const string& outputFile);

    /**
     * @brief 检测指定网址是否可以访问，并设置超时时间为 2 秒
     * 
     * @param url 检测网址
     * @param timeout 超时时间
     * @return true 可以访问
     * @return false 不可访问
     */
    static bool checkURL(const std::string& url, int timeout);

};

#endif