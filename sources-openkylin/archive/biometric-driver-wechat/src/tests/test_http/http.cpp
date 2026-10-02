#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>
#include "ksb_http.h"
#include <curl/curl.h>

extern "C"{
    #define REQUIRE_TIMEOUT_S 20
}

// 测试 write_data 函数
TEST(WriteDataTest, HandlesValidInput) {
    Get_Data data;
    data.size = 0;
    data.data = (char *)malloc(1); // 初始分配
    data.data[0] = '\0';

    char test_str[] = "Hello, World!";
    size_t result = write_data(test_str, 1, strnlen(test_str,1024), &data);

    // 验证返回值
    EXPECT_EQ(result, strlen(test_str));

    // 验证数据内容
    EXPECT_STREQ(data.data, test_str);

    // 验证大小
    EXPECT_EQ(data.size, strlen(test_str));

    // 释放分配的内存
    free(data.data);
}


TEST(DoHttpGetTest, HandlesValidInput) {
    char url[] = "http://example.com";
    char getcontent[1024*1024];
    Get_Data ret_data;
    ret_data.data = NULL;
    // Mock curl_easy_init
    CURL *handler = curl_easy_init();
    ASSERT_NE(handler, nullptr);

    // Mock curl_easy_setopt
    curl_easy_setopt(handler, CURLOPT_URL, url);
    curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handler, CURLOPT_WRITEDATA, &ret_data);
    curl_easy_setopt(handler, CURLOPT_TIMEOUT, REQUIRE_TIMEOUT_S);

    // Mock curl_easy_perform
    CURLcode ops_res = curl_easy_perform(handler);
    ASSERT_EQ(ops_res, CURLE_OK);

    // Call Do_Http_Get
    int rc = Do_Http_Get(url, getcontent);
    EXPECT_EQ(rc, 0);

    // Cleanup
    curl_easy_cleanup(handler);
}

TEST(HttpPostTest, HandlesValidInput) {
    char url[] = "http://example.com";
    char postcontent[] = "{\"key\":\"value\"}";
    char getcontent[1024*1024];

    int rc = Do_Http_Post(url, postcontent, getcontent);

    // 验证返回值
    EXPECT_EQ(rc, 0);

}

TEST(DoDownloadTest, HandlesValidInput) {
    char url[] = "http://example.com/test.jpg";
    char filename[] = "test.jpg";

    // Mock curl_easy_init
    CURL *download_handler = curl_easy_init();
    ASSERT_NE(download_handler, nullptr);

    // Mock curl_easy_setopt
    curl_easy_setopt(download_handler, CURLOPT_URL, url);
    FILE* file = fopen(filename, "wb");
    ASSERT_NE(file, nullptr);
    curl_easy_setopt(download_handler, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(download_handler, CURLOPT_TIMEOUT, REQUIRE_TIMEOUT_S);

    // Mock curl_easy_perform
    CURLcode ops_res = curl_easy_perform(download_handler);
    ASSERT_EQ(ops_res, CURLE_OK);

    // Call Do_Download
    int rc = Do_Download(url, filename);
    EXPECT_EQ(rc, 0);

    // 验证文件是否存在且大小大于0
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    EXPECT_GT(file_size, 0);

    // Cleanup
    fclose(file);
    curl_easy_cleanup(download_handler);
    remove(filename); // 删除测试文件
}


