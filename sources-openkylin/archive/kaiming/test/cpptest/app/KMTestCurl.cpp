#include "KMCurl.h"
#include <gtest/gtest.h>

// 移除重复的 KMCurl 类定义

class KMTestCurl
{
public:
    KMTestCurl() = default;
    ~KMTestCurl() = default;

public:
    KMCurl m_curl;
};

TEST(KMTestCurl, test_curl_curlApiDownload)
{
    KMTestCurl curlApiDownload;
    const char *argv[] = {"http://172.30.60.17/repo/", "/home/csy/summary.json"};
    std::string url = argv[0];
    std::string outputFile = argv[1];
    bool result = curlApiDownload.m_curl.curlApiDownload(url, outputFile);
    EXPECT_EQ(result, true);
}

TEST(KMTestCurl, test_curl_checkURL)
{
    KMTestCurl checkURL;
    std::string url = "http://172.30.60.17/repo/";
    bool result = checkURL.m_curl.checkURL(url, 2);
    EXPECT_EQ(result, true);
}