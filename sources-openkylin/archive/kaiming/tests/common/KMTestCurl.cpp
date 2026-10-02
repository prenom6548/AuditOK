/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <regex>

#include "common/KMCurl.h"

class KMTestCurl
{
public:
    KMTestCurl() = default;
    ~KMTestCurl() = default;

public:
    KMCurl m_curl;
};

// 从配置文件读取 URL
std::string getRepoUrlFromConfig() {
    std::string configPath = "/opt/kaiming/repo/config";
    std::ifstream configFile(configPath);
    std::string line;
    std::string url;
    
    if (!configFile.is_open()) {
        std::cout << "Failed to open config file: " << configPath << std::endl;
        return "";
    }
    
    std::regex urlRegex(R"(url\s*=\s*(.+)$)");
    
    while (std::getline(configFile, line)) {
        std::smatch match;
        if (std::regex_search(line, match, urlRegex)) {
            url = match[1];
            // 去除可能的空格
            url.erase(0, url.find_first_not_of(" \t"));
            url.erase(url.find_last_not_of(" \t") + 1);
            break;
        }
    }
    
    configFile.close();
    return url;
}

TEST(KMTestCurl, test_curl_curlApiDownload)
{
    KMTestCurl curlApiDownload;
    
    // 从配置文件动态获取 URL
    std::string url = getRepoUrlFromConfig();
    if (url.empty()) {
        GTEST_SKIP() << "Failed to get repo URL from /opt/kaiming/repo/config";
    }
    
    // 确保 URL 以 / 结尾
    if (url.back() != '/') {
        url += '/';
    }
    
    std::string outputFile = "/home/summary.json";
    std::cout << "Downloading from: " << url << " to: " << outputFile << std::endl;
    
    bool result = curlApiDownload.m_curl.curlApiDownload(url, outputFile);
    EXPECT_EQ(result, true);
}

TEST(KMTestCurl, test_curl_checkURL)
{
    KMTestCurl checkURL;
    
    // 从配置文件动态获取 URL
    std::string url = getRepoUrlFromConfig();
    if (url.empty()) {
        GTEST_SKIP() << "Failed to get repo URL from /opt/kaiming/repo/config";
    }
    
    std::cout << "Checking URL: " << url << std::endl;
    bool result = checkURL.m_curl.checkURL(url, 2);
    EXPECT_EQ(result, true);
}