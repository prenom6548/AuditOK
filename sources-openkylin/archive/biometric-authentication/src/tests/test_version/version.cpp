#include <gtest/gtest.h>
#include <gmock/gmock.h>  // 添加 gmock 头文件以支持模拟对象
#include <fstream>
using namespace testing;
extern "C" {
#include "../../lib/biometric_version.h"
}

// 新增测试用例：测试 bio_check_drv_api_version 函数
TEST(VersionTest, BioCheckDrvApiVersion) {
    // 测试驱动版本与框架版本完全匹配
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR, BIO_DRV_API_VERSION_MINOR, BIO_DRV_API_VERSION_FUNC), 0);

    // 测试驱动版本主版本号大于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR + 1, BIO_DRV_API_VERSION_MINOR, BIO_DRV_API_VERSION_FUNC), 3);

    // 测试驱动版本主版本号小于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR - 1, BIO_DRV_API_VERSION_MINOR, BIO_DRV_API_VERSION_FUNC), -3);

    // 测试驱动版本从版本号大于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR, BIO_DRV_API_VERSION_MINOR + 1, BIO_DRV_API_VERSION_FUNC), 2);

    // 测试驱动版本从版本号小于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR, BIO_DRV_API_VERSION_MINOR - 1, BIO_DRV_API_VERSION_FUNC), -2);

    // 测试驱动版本功能版本号大于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR, BIO_DRV_API_VERSION_MINOR, BIO_DRV_API_VERSION_FUNC + 1), 1);

    // 测试驱动版本功能版本号小于框架版本
    EXPECT_EQ(bio_check_drv_api_version(BIO_DRV_API_VERSION_MAJOR, BIO_DRV_API_VERSION_MINOR, BIO_DRV_API_VERSION_FUNC - 1), 0);
}

// 新增测试用例：测试 bio_check_app_api_version 函数
TEST(VersionTest, BioCheckAppApiVersion) {
    // 测试应用版本与框架版本完全匹配
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR, BIO_APP_API_VERSION_MINOR, BIO_APP_API_VERSION_FUNC), 0);

    // 测试应用版本主版本号大于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR + 1, BIO_APP_API_VERSION_MINOR, BIO_APP_API_VERSION_FUNC), 3);

    // 测试应用版本主版本号小于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR - 1, BIO_APP_API_VERSION_MINOR, BIO_APP_API_VERSION_FUNC), -3);

    // 测试应用版本从版本号大于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR, BIO_APP_API_VERSION_MINOR + 1, BIO_APP_API_VERSION_FUNC), 2);

    // 测试应用版本从版本号小于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR, BIO_APP_API_VERSION_MINOR - 1, BIO_APP_API_VERSION_FUNC), -2);

    // 测试应用版本功能版本号大于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR, BIO_APP_API_VERSION_MINOR, BIO_APP_API_VERSION_FUNC + 1), 1);

    // 测试应用版本功能版本号小于框架版本
    EXPECT_EQ(bio_check_app_api_version(BIO_APP_API_VERSION_MAJOR, BIO_APP_API_VERSION_MINOR, BIO_APP_API_VERSION_FUNC - 1), 0);
}

// 新增测试用例：测试 bio_get_drv_api_version_by_framework_side 和 bio_get_app_api_version_by_framework_side
TEST(VersionTest, BioGetApiVersions) {
    int major, minor, function;

    // 测试 bio_get_drv_api_version_by_framework_side
    bio_get_drv_api_version_by_framework_side(&major, &minor, &function);
    EXPECT_EQ(major, BIO_DRV_API_VERSION_MAJOR);
    EXPECT_EQ(minor, BIO_DRV_API_VERSION_MINOR);
    EXPECT_EQ(function, BIO_DRV_API_VERSION_FUNC);

    // 测试 bio_get_app_api_version_by_framework_side
    bio_get_app_api_version_by_framework_side(&major, &minor, &function);
    EXPECT_EQ(major, BIO_APP_API_VERSION_MAJOR);
    EXPECT_EQ(minor, BIO_APP_API_VERSION_MINOR);
    EXPECT_EQ(function, BIO_APP_API_VERSION_FUNC);
}


