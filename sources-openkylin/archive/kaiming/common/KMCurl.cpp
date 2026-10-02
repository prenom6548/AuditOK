/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMCurl.h"

#include "common/KMLogger.h"

KMCurl::KMCurl()
{

}

KMCurl::~KMCurl()
{

}

bool KMCurl::curlApiDownload(const string& url, const string& outputFile) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        kmlogger.error("Failed to initialize libcurl.");
        return false;
    }

    string tempFile = outputFile + ".tmp";

    FILE *file = fopen(tempFile.c_str(), "wb");
    if (!file)
    {
        kmlogger.error("Failed to open %s for writing.", outputFile.c_str());
        curl_easy_cleanup(curl);
        return false;
    }

    // 设置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);    // 允许重定向
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);   // 连接超时 10s
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);          // 传输超时 30s
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "KMCurl/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);    // 启用 HTTPS 验证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);

    // 先检查文件关闭是否成功
    if (fclose(file) != 0) {
        kmlogger.error("Failed to close file: %s", outputFile.c_str());
        curl_easy_cleanup(curl);
        fs::remove(tempFile);
        return false;
    }

    bool success = false;

    // 检查下载结果
    if (res != CURLE_OK) {
        kmlogger.error("Download failed: %s", curl_easy_strerror(res));
        fs::remove(tempFile);
    }else {
        if (rename(tempFile.c_str(), outputFile.c_str()) != 0) {
            kmlogger.error("Failed to rename temp file to: %s", outputFile.c_str());
            fs::remove(tempFile);
        } else {
            success = true;
        }
    }

    curl_easy_cleanup(curl);
    return success;
}

bool KMCurl::checkURL(const std::string &url, int timeout)
{
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // 设置超时时间为2秒
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
        // 设置请求不输出
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        // 执行请求
        res = curl_easy_perform(curl);

        // 清理 curl 资源
        curl_easy_cleanup(curl);

        // 判断请求是否成功
        if (res == CURLE_OK)
        {
            return true;
        }
        else
        {
            std::cerr << "Check http url error: " << curl_easy_strerror(res) << std::endl;
        }
    }
    return false;
}
