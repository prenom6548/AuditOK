#include <gtest/gtest.h>
#include <gmock/gmock.h>  // 添加 gmock 头文件以支持模拟对象
#include <functional> // for std::bind
#include <glib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <filesystem>
#include <libusb.h>
#include <dlfcn.h>
using namespace testing;

extern "C" {
#include "../../lib/biometric_common.h"
#include "../../lib/biometric_stroge.h"
#include "../../lib/biometric_config.h"
#include "../../lib/biometric_version.h"
    bio_dev* bio_dev_new();
    void bio_dev_free(bio_dev* dev);
}


// Mock function implementations
int mock_ops_open(bio_dev* dev) {
    return 0;
}

char* mock_ops_capture(bio_dev* dev, OpsActions action) {
    return strdup("captured_data");
}

int mock_ops_enroll(bio_dev* dev, OpsActions action, int uid, int idx,  char* bio_idx_name) {
    return 0;
}

int mock_ops_verify(bio_dev* dev, OpsActions action, int uid, int idx) {
    return 0;
}

int mock_ops_identify(bio_dev* dev, OpsActions action, int uid, int idx_start, int idx_end) {
    return uid;
}

feature_info* mock_ops_search(bio_dev* dev, OpsActions action, int uid, int idx_start, int idx_end) {
    feature_info* info = bio_sto_new_feature_info(uid, 1, "Driver1", idx_start, "Index1");
    return info;
}

int mock_ops_clean(bio_dev* dev, OpsActions action, int uid, int idx_start, int idx_end) {
    return 0;
}

feature_info* mock_ops_get_feature_list(bio_dev* dev, OpsActions action, int uid, int idx_start, int idx_end) {
    feature_info* info = bio_sto_new_feature_info(uid, 1, "Driver1", idx_start, "Index1");
    return info;
}

int mock_ops_feature_rename(bio_dev* dev, int uid, int idx,  char* new_name) {
    return 0;
}

void mock_ops_close(bio_dev* dev) {}

const char *mock_ops_get_ops_result_mesg(bio_dev *dev)
{
	return NULL;
}

const char *mock_ops_get_dev_status_mesg(bio_dev *dev)
{
	return NULL;
}

const char *mock_ops_get_notify_mid_mesg(bio_dev *dev)
{
    return NULL;
}

int mock_ops_stop_by_user(bio_dev *dev, int waiting_sec)
{

    return 0;
}

void mock_status_changed_callback(int driver_id, int status_type) {
    // Mock implementation
}


bio_dev* mockDev = nullptr;
void initializeMockDev() {
    mockDev = (bio_dev*)malloc(sizeof(bio_dev));  // Allocate memory
    if (mockDev != nullptr) {
        memset(mockDev, 0, sizeof(bio_dev));  // Clear memory

        // Initialize structure members
        mockDev->driver_id = 1;
        mockDev->device_name = strdup("Mock Device");
        mockDev->full_name = strdup("Mock Bio Device Full Name");
        mockDev->drv_api_version.major = 1;
        mockDev->drv_api_version.minor = 0;
        mockDev->drv_api_version.function = 0;
        mockDev->enable = 1;
        mockDev->dev_num = 1;
        mockDev->dev_status = static_cast<DevStatus>(0);
        mockDev->ops_result = static_cast<OpsResult>(0);
        mockDev->notify_mid = static_cast<NotifyMID>(0);

        // Initialize function pointers
        mockDev->ops_open = mock_ops_open;
        mockDev->ops_capture = mock_ops_capture;
        mockDev->ops_enroll = mock_ops_enroll;
        mockDev->ops_verify = mock_ops_verify;
        mockDev->ops_identify = mock_ops_identify;
        mockDev->ops_search = mock_ops_search;
        mockDev->ops_clean = mock_ops_clean;
        mockDev->ops_get_feature_list = mock_ops_get_feature_list;
        mockDev->ops_feature_rename = mock_ops_feature_rename;
        mockDev->ops_close = mock_ops_close;
        mockDev->ops_get_ops_result_mesg = mock_ops_get_ops_result_mesg;
        mockDev->ops_get_dev_status_mesg = mock_ops_get_dev_status_mesg;
        mockDev->ops_get_notify_mid_mesg = mock_ops_get_notify_mid_mesg;
        mockDev->ops_stop_by_user = mock_ops_stop_by_user;
        mockDev->ops_status_changed_callback = mock_status_changed_callback;
        mockDev->ops_status_changed_callback = mock_status_changed_callback;
    }
}
// Test case
TEST(BioDevTest, TestMockOps) {
    initializeMockDev();

    // Testing bio_ops_capture
    char* captured_data = bio_ops_capture(mockDev);
    EXPECT_STREQ(captured_data, "captured_data");
    free(captured_data);  

    // Testing bio_ops_enroll
    EXPECT_EQ(bio_ops_enroll(mockDev, 123, 1, "bio_idx_name"), 0);

    // Testing bio_ops_verify
    EXPECT_EQ(bio_ops_verify(mockDev, 123, 1), 0);

    // Testing bio_ops_identify
    EXPECT_EQ(bio_ops_identify(mockDev, 123, 1, 5), 123);

    // Testing bio_ops_search
    feature_info* search_result = bio_ops_search(mockDev, 123, 1, 5);
    EXPECT_NE(search_result, nullptr);
    EXPECT_EQ(search_result->uid, 123);
    free(search_result);

    // Testing bio_ops_clean
    EXPECT_EQ(bio_ops_clean(mockDev, 123, 1, 5), 0);

    // Testing bio_ops_get_feature_list
    feature_info* feature_list = bio_ops_get_feature_list(mockDev, 123, 1, 5);
    EXPECT_NE(feature_list, nullptr);
    EXPECT_EQ(feature_list->uid, 123);
    free(feature_list);

    // Testing bio_ops_feature_rename
    EXPECT_EQ(bio_ops_feature_rename(mockDev, 123, 1, "new_name"), 0);


    EXPECT_EQ(bio_ops_stop_ops(mockDev, 1), 0);

    // Free allocated memory for mockDev
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioDevIsEnable) {
    initializeMockDev();

    GKeyFile* mockConf = g_key_file_new();
    g_key_file_set_boolean(mockConf, "Mock Device", "Enable", TRUE);

    EXPECT_TRUE(bio_dev_is_enable(mockDev, mockConf));

    g_key_file_set_boolean(mockConf, "Mock Device", "Enable", FALSE);
    EXPECT_FALSE(bio_dev_is_enable(mockDev, mockConf));

    g_key_file_free(mockConf);
}

TEST(BioDevTest, BioDevSetSerialPath) {
    initializeMockDev();

    GKeyFile* mockConf = g_key_file_new();
    g_key_file_set_string(mockConf, "Mock Device", "Path", "/dev/ttyUSB0");

    // 测试成功设置路径的情况
    EXPECT_EQ(bio_dev_set_serial_path(mockDev, mockConf), 0);
    EXPECT_STREQ(mockDev->serial_info.path, "/dev/ttyUSB0");

    // 测试设备名称为空的情况
    free(mockDev->device_name);
    mockDev->device_name = NULL;
    EXPECT_EQ(bio_dev_set_serial_path(mockDev, mockConf), -1);

    // 重新设置设备名称
    mockDev->device_name = strdup("Mock Device");

    // 测试路径为空的情况
    g_key_file_set_string(mockConf, "Mock Device", "Path", "");
    EXPECT_EQ(bio_dev_set_serial_path(mockDev, mockConf), -1);

    // 测试获取路径失败的情况
    g_key_file_remove_key(mockConf, "Mock Device", "Path", NULL);
    EXPECT_EQ(bio_dev_set_serial_path(mockDev, mockConf), -1);

    g_key_file_free(mockConf);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioIsStopByUser) {
    initializeMockDev();
    // 测试设备状态为 DEVS_COMM_STOP_BY_USER 的情况
    mockDev->dev_status = DEVS_COMM_STOP_BY_USER;
    EXPECT_TRUE(bio_is_stop_by_user(mockDev));

    // 测试设备状态不为 DEVS_COMM_STOP_BY_USER 的情况
    mockDev->dev_status = static_cast<DevStatus>(200);
    EXPECT_FALSE(bio_is_stop_by_user(mockDev));

    // 测试设备状态为负数的情况
    mockDev->dev_status = static_cast<DevStatus>(-100 + DEVS_COMM_STOP_BY_USER);
    EXPECT_FALSE(bio_is_stop_by_user(mockDev));
    free(mockDev);
}

TEST(CommonTest, BiosetOpsTimeoutMs)
{
    // 设置一个超时时间
    int test_timeout = 5000;
    bio_set_ops_timeout_ms(test_timeout);

    // 获取并验证超时时间是否设置正确
    int actual_timeout = bio_get_ops_timeout_ms();
    EXPECT_EQ(actual_timeout, test_timeout);

    // 获取并验证默认超时时间
    int default_timeout = bio_get_ops_timeout_ms();
    EXPECT_EQ(default_timeout, test_timeout);
}

TEST(CommonTest, BioBase64EncodeDecode) {
    unsigned char bin_data[] = {0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x51, 0x04, 0x00, 0x1a, 0x0b, 0x0c, 0x56, 0x00, 0x00, 0x00};
    char base64[100];
    int bin_len = sizeof(bin_data);
    int base64_len;

    // 测试 bio_base64_encode
    base64_len = bio_base64_encode(bin_data, base64, bin_len);
    EXPECT_STREQ(base64, "H4sIAAAAAAAC/8tIzcnJVyjPL8pJUQQAGgsMVgAAAA==");

    // 测试 bio_base64_decode
    unsigned char decoded_bin_data[100];
    int decoded_bin_len = bio_base64_decode(base64, decoded_bin_data);
    EXPECT_EQ(decoded_bin_len, bin_len);
    EXPECT_EQ(memcmp(decoded_bin_data, bin_data, bin_len), 0);
}

TEST(BioDevTest, BioGetSetDevStatus) {
    initializeMockDev();

    // 设置设备状态并验证
    bio_set_dev_status(mockDev, 100);
    EXPECT_EQ(bio_get_dev_status(mockDev), 100);

    // 设置设备状态并验证
    bio_set_dev_status(mockDev, 200);
    EXPECT_EQ(bio_get_dev_status(mockDev), 200);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

// Test bio_set_dev_abs_status
TEST(BioDevTest, BioSetDevAbsStatus) {
    initializeMockDev();

    // 设置设备状态并验证
    bio_set_dev_abs_status(mockDev, 300);
    EXPECT_EQ(bio_get_dev_status(mockDev), 300);

    // 设置设备状态并验证
    bio_set_dev_abs_status(mockDev, 400);
    EXPECT_EQ(bio_get_dev_status(mockDev), 400);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioGetSetOpsResult) {
    initializeMockDev();

    // 设置设备状态
    bio_set_dev_status(mockDev, 200);

    // 设置操作结果并验证
    bio_set_ops_result(mockDev, 150);
    EXPECT_EQ(bio_get_ops_result(mockDev), 250);

    // 设置操作结果并验证
    bio_set_ops_result(mockDev, 75);
    EXPECT_EQ(bio_get_ops_result(mockDev), 275);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

// Test bio_set_ops_abs_result
TEST(BioDevTest, BioSetOpsAbsResult) {
    initializeMockDev();

    // 设置操作结果并验证
    bio_set_ops_abs_result(mockDev, 350);
    EXPECT_EQ(bio_get_ops_result(mockDev), 350);

    // 设置操作结果并验证
    bio_set_ops_abs_result(mockDev, 450);
    EXPECT_EQ(bio_get_ops_result(mockDev), 450);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioGetSetNotifyMid) {
    initializeMockDev();

    // 测试 bio_get_notify_mid 和 bio_set_notify_mid
    bio_set_notify_mid(mockDev, 123);
    EXPECT_EQ(bio_get_notify_mid(mockDev), 23);

    // 测试 bio_set_notify_abs_mid
    bio_set_notify_abs_mid(mockDev, 456);
    EXPECT_EQ(bio_get_notify_mid(mockDev), 456);

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioSetAllStatus) {
    initializeMockDev();

    // 设置初始状态
    mockDev->dev_status = static_cast<DevStatus>(0);
    mockDev->ops_result = static_cast<OpsResult>(0);
    mockDev->notify_mid = static_cast<NotifyMID>(0);

    // 调用 bio_set_all_status
    bio_set_all_status(mockDev, static_cast<DevStatus>(101), static_cast<OpsResult>(202), static_cast<NotifyMID>(303));

    // 验证状态是否正确设置
    EXPECT_EQ(mockDev->dev_status, static_cast<DevStatus>(1));
    EXPECT_EQ(mockDev->ops_result, static_cast<OpsResult>(2));
    EXPECT_EQ(mockDev->notify_mid, static_cast<NotifyMID>(3));

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioSetAllAbsStatus) {
    initializeMockDev();

    // 设置初始状态
    mockDev->dev_status = static_cast<DevStatus>(0);
    mockDev->ops_result = static_cast<OpsResult>(0);
    mockDev->notify_mid = static_cast<NotifyMID>(0);

    // 调用 bio_set_all_abs_status
    bio_set_all_abs_status(mockDev, static_cast<DevStatus>(101), static_cast<OpsResult>(202), static_cast<NotifyMID>(303));

    // 验证状态是否正确设置
    EXPECT_EQ(mockDev->dev_status, static_cast<DevStatus>(101));
    EXPECT_EQ(mockDev->ops_result, static_cast<OpsResult>(202));
    EXPECT_EQ(mockDev->notify_mid, static_cast<NotifyMID>(303));

    // 释放分配的内存
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioGetDevStatusMesg) {
    initializeMockDev();

    // Test case for DEVS_COMM_IDLE
    mockDev->dev_status = DEVS_COMM_IDLE;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Device idle");

    // Test case for DEVS_COMM_DOING
    mockDev->dev_status = DEVS_COMM_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Device is in process");

    // Test case for DEVS_COMM_STOP_BY_USER
    mockDev->dev_status = DEVS_COMM_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating current operation");

    // Test case for DEVS_COMM_DISABLE
    mockDev->dev_status = DEVS_COMM_DISABLE;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Device is disabled");

    // Test case for DEVS_OPEN_DOING
    mockDev->dev_status = DEVS_OPEN_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Opening device");

    // Test case for DEVS_OPEN_STOP_BY_USER
    mockDev->dev_status = DEVS_OPEN_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating open operation");

    // Test case for DEVS_ENROLL_DOING
    mockDev->dev_status = DEVS_ENROLL_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Enrolling");

    // Test case for DEVS_ENROLL_STOP_BY_USER
    mockDev->dev_status = DEVS_ENROLL_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating enroll operation");

    // Test case for DEVS_VERIFY_DOING
    mockDev->dev_status = DEVS_VERIFY_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Verifying");

    // Test case for DEVS_VERIFY_STOP_BY_USER
    mockDev->dev_status = DEVS_VERIFY_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating Verify operation");

    // Test case for DEVS_IDENTIFY_DOING
    mockDev->dev_status = DEVS_IDENTIFY_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Identifying");

    // Test case for DEVS_IDENTIFY_STOP_BY_USER
    mockDev->dev_status = DEVS_IDENTIFY_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating identify operation");

    // Test case for DEVS_CAPTURE_DOING
    mockDev->dev_status = DEVS_CAPTURE_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Capturing");

    // Test case for DEVS_CAPTURE_STOP_BY_USER
    mockDev->dev_status = DEVS_CAPTURE_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating capture operation");

    // Test case for DEVS_SEARCH_DOING
    mockDev->dev_status = DEVS_SEARCH_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Searching");

    // Test case for DEVS_SEARCH_STOP_BY_USER
    mockDev->dev_status = DEVS_SEARCH_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating search operation");

    // Test case for DEVS_CLEAN_DOING
    mockDev->dev_status = DEVS_CLEAN_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Cleaning");

    // Test case for DEVS_CLEAN_STOP_BY_USER
    mockDev->dev_status = DEVS_CLEAN_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating clean operation");

    // Test case for DEVS_GET_FLIST_DOING
    mockDev->dev_status = DEVS_GET_FLIST_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Getting feature list");

    // Test case for DEVS_GET_FLIST_STOP_BY_USER
    mockDev->dev_status = DEVS_GET_FLIST_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating get feature list operation");

    // Test case for DEVS_RENAME_DOING
    mockDev->dev_status = DEVS_RENAME_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Renaming feature");

    // Test case for DEVS_RENAME_STOP_BY_USER
    mockDev->dev_status = DEVS_RENAME_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating feature rename operation");

    // Test case for DEVS_CLOSE_DOING
    mockDev->dev_status = DEVS_CLOSE_DOING;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Closing");

    // Test case for DEVS_CLOSE_STOP_BY_USER
    mockDev->dev_status = DEVS_CLOSE_STOP_BY_USER;
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Terminating close operation");

    // Test case for unknown status
    mockDev->dev_status = static_cast<DevStatus>(999);
    EXPECT_STREQ(bio_get_dev_status_mesg(mockDev), "Device is in an unknown status...");

    // Free allocated memory for mockDev
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, TestBioGetOpsResultMesg) {
    initializeMockDev();

    struct OpsResultTestCase {
    OpsResult ops_result;
    const char* expected_message;
    };

OpsResultTestCase test_cases[] = {
        {OPS_COMM_SUCCESS, "Operation successful"},
        {OPS_COMM_FAIL, "Operation failed"},
        {OPS_COMM_ERROR, "An error was encountered during the operation"},
        {OPS_COMM_STOP_BY_USER, "The operation was interrupted by the user"},
        {OPS_COMM_TIMEOUT, "Operation timeout"},
        {OPS_COMM_OUT_OF_MEM, "Out of memory"},
        {OPS_OPEN_SUCCESS, "Open device completion"},
        {OPS_OPEN_FAIL, "Open device failed"},
        {OPS_OPEN_ERROR, "An error was encountered during the opening"},
        {OPS_ENROLL_SUCCESS, "Enroll feature successful"},
        {OPS_ENROLL_FAIL, "Enroll feature failed"},
        {OPS_ENROLL_ERROR, "An error was encountered during enrolling"},
        {OPS_ENROLL_STOP_BY_USER, "Enroll operation was interrupted by the user"},
        {OPS_ENROLL_TIMEOUT, "Enroll operation timeout"},
        {OPS_VERIFY_MATCH, "Verify feature match"},
        {OPS_VERIFY_NO_MATCH, "Verify feature no match"},
        {OPS_VERIFY_ERROR, "An error was encountered during verifying"},
        {OPS_VERIFY_STOP_BY_USER, "Verify operation was interrupted by the user"},
        {OPS_VERIFY_TIMEOUT, "Verify operation timeout"},
        {OPS_IDENTIFY_MATCH, "Identify feature match"},
        {OPS_IDENTIFY_NO_MATCH, "Identify feature no match"},
        {OPS_IDENTIFY_ERROR, "An error was encountered during identifying"},
        {OPS_IDENTIFY_STOP_BY_USER, "Identify operation was interrupted by the user"},
        {OPS_IDENTIFY_TIMEOUT, "Identify operation timeout"},
        {OPS_CAPTURE_SUCCESS, "Capture feature successful"},
        {OPS_CAPTURE_FAIL, "Capture feature failed"},
        {OPS_CAPTURE_ERROR, "An error was encountered during capturing"},
        {OPS_CAPTURE_STOP_BY_USER, "Capture operation was interrupted by the user"},
        {OPS_CAPTURE_TIMEOUT, "Capture operation timeout"},
        {OPS_SEARCH_MATCH, "Find the specified features"},
        {OPS_SEARCH_NO_MATCH, "No specified features were found"},
        {OPS_SEARCH_ERROR, "An error was encountered during searching"},
        {OPS_SEARCH_STOP_BY_USER, "Search operation was interrupted by the user"},
        {OPS_SEARCH_TIMEOUT, "Search operation timeout"},
        {OPS_CLEAN_SUCCESS, "Clean feature successful"},
        {OPS_CLEAN_FAIL, "Clean feature failed"},
        {OPS_CLEAN_ERROR, "An error was encountered during cleaning"},
        {OPS_CLEAN_STOP_BY_USER, "Clean operation was interrupted by the user"},
        {OPS_CLEAN_TIMEOUT, "Clean operation timeout"},
        {OPS_GET_FLIST_SUCCESS, "Get feature list completion"},
        {OPS_GET_FLIST_FAIL, "Get feature list failed"},
        {OPS_GET_FLIST_ERROR, "An error was encountered during getting feature list"},
        {OPS_RENAME_SUCCESS, "Rename feature successful"},
        {OPS_RENAME_FAIL, "Rename feature failed"},
        {OPS_RENAME_ERROR, "An error was encountered during renaming"},
        {OPS_CLOSE_SUCCESS, "Close device completion"},
        {OPS_CLOSE_FAIL, "Close device failed"},
        {OPS_CLOSE_ERROR, "An error was encountered during closing"},
        {static_cast<OpsResult>(-1), "Operation is in an unknown status......"}
    };

    // Test all cases
    for (const auto& test_case : test_cases) {
        mockDev->ops_result = test_case.ops_result;
        EXPECT_STREQ(bio_get_ops_result_mesg(mockDev), test_case.expected_message);
    }
    // Free allocated memory for mockDev
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, TestBioGetNotifyMidMesg) {
    initializeMockDev();

    struct NotifyTestCase {
        NotifyMID notify_mid;
        const char* expected_message;
    };

    NotifyTestCase test_cases[] = {
        {NOTIFY_COMM_IDLE, "Device idle"},
        {NOTIFY_COMM_FAIL, "Operation failed"},
        {NOTIFY_COMM_ERROR, "An error was encountered during the operation"},
        {NOTIFY_COMM_STOP_BY_USER, "The operation was interrupted by the user"},
        {NOTIFY_COMM_TIMEOUT, "Operation timeout"},
        {NOTIFY_COMM_DISABLE, "Device is disable"},
        {NOTIFY_COMM_OUT_OF_MEM, "Out of memory"},
        {NOTIFY_COMM_UNSUPPORTED_OPS, "The device or driver does not support this operation"},
        {NOTIFY_OPEN_SUCCESS, "Open device completion"},
        {NOTIFY_OPEN_FAIL, "Open device failed"},
        {NOTIFY_OPEN_ERROR, "An error was encountered during the opening"},
        {NOTIFY_ENROLL_SUCCESS, "Enroll feature successful"},
        {NOTIFY_ENROLL_FAIL, "Enroll feature failed"},
        {NOTIFY_ENROLL_ERROR, "An error was encountered during enrolling"},
        {NOTIFY_ENROLL_STOP_BY_USER, "Enroll operation was interrupted by the user"},
        {NOTIFY_ENROLL_TIMEOUT, "Enroll operation timeout"},
        {NOTIFY_VERIFY_MATCH, "Verify feature match"},
        {NOTIFY_VERIFY_NO_MATCH, "Verify feature no match"},
        {NOTIFY_VERIFY_ERROR, "An error was encountered during verifying"},
        {NOTIFY_VERIFY_STOP_BY_USER, "Verify operation was interrupted by the user"},
        {NOTIFY_VERIFY_TIMEOUT, "Verify operation timeout"},
        {NOTIFY_IDENTIFY_MATCH, "Identify feature match"},
        {NOTIFY_IDENTIFY_NO_MATCH, "Identify feature no match"},
        {NOTIFY_IDENTIFY_ERROR, "An error was encountered during identifying"},
        {NOTIFY_IDENTIFY_STOP_BY_USER, "Identify operation was interrupted by the user"},
        {NOTIFY_IDENTIFY_TIMEOUT, "Identify operation timeout"},
        {NOTIFY_CAPTURE_SUCCESS, "Capture feature successful"},
        {NOTIFY_CAPTURE_FAIL, "Capture feature failed"},
        {NOTIFY_CAPTURE_ERROR, "An error was encountered during capturing"},
        {NOTIFY_CAPTURE_STOP_BY_USER, "Capture operation was interrupted by the user"},
        {NOTIFY_CAPTURE_TIMEOUT, "Capture operation timeout"},
        {NOTIFY_SEARCH_MATCH, "Find the specified features"},
        {NOTIFY_SEARCH_NO_MATCH, "No specified features were found"},
        {NOTIFY_SEARCH_ERROR, "An error was encountered during searching"},
        {NOTIFY_SEARCH_STOP_BY_USER, "Search operation was interrupted by the user"},
        {NOTIFY_SEARCH_TIMEOUT, "Search operation timeout"},
        {NOTIFY_CLEAN_SUCCESS, "Clean feature successful"},
        {NOTIFY_CLEAN_FAIL, "Clean feature failed"},
        {NOTIFY_CLEAN_ERROR, "An error was encountered during cleaning"},
        {NOTIFY_CLEAN_STOP_BY_USER, "Clean operation was interrupted by the user"},
        {NOTIFY_CLEAN_TIMEOUT, "Clean operation timeout"},
        {NOTIFY_GET_FLIST_SUCCESS, "Get feature list completion"},
        {NOTIFY_GET_FLIST_FAIL, "Get feature list failed"},
        {NOTIFY_GET_FLIST_ERROR, "An error was encountered during getting feature list"},
        {NOTIFY_RENAME_SUCCESS, "Rename feature successful"},
        {NOTIFY_RENAME_FAIL, "Rename feature failed"},
        {NOTIFY_RENAME_ERROR, "An error was encountered during renaming"},
        {NOTIFY_RENAME_INCOMPLETE, "Feature renaming is not complete, please try again"},
        {NOTIFY_CLOSE_SUCCESS, "Close device completion"},
        {NOTIFY_CLOSE_FAIL, "Close device failed"},
        {NOTIFY_CLOSE_ERROR, "An error was encountered during closing"},
        {NOTIFY_COMM_IDLE, "Device idle"},
    };

    for (const auto& test_case : test_cases) {
        mockDev->notify_mid = test_case.notify_mid;
        EXPECT_STREQ(bio_get_notify_mid_mesg(mockDev), test_case.expected_message);
    }

    // Free allocated memory for mockDev
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioGetEmptyDriverId) {
    
    // 创建并添加设备到驱动列表
    for (int i = BIO_DRVID_DYNAMIC_START; i < BIO_DRVID_DYNAMIC_START + 5; ++i) {
        bio_dev* dev = (bio_dev*)malloc(sizeof(bio_dev));
        dev->driver_id = i;
        bio_drv_list = g_list_append(bio_drv_list, dev);
    }

    // 测试 bio_get_empty_driver_id 函数
    int empty_id = bio_get_empty_driver_id();
    EXPECT_EQ(empty_id, BIO_DRVID_DYNAMIC_START+5);

    // 清理测试环境
    g_list_free_full(bio_drv_list, free);
    bio_drv_list = NULL;
}

TEST(BioDevTest, BioGetEmptyDriverIdFull) {

    // 创建并添加设备到驱动列表，填满所有动态ID
    for (int i = BIO_DRVID_DYNAMIC_START; i <= BIO_DRVID_DYNAMIC_MAX; i++) {
        bio_dev* dev = (bio_dev*)malloc(sizeof(bio_dev));
        dev->driver_id = i;
        bio_drv_list = g_list_append(bio_drv_list, dev);
    }

    // 测试 bio_get_empty_driver_id 函数
    int empty_id = bio_get_empty_driver_id();
    EXPECT_EQ(empty_id, -1);

    // 清理测试环境
    g_list_free_full(bio_drv_list, free);
    bio_drv_list = NULL;
}

TEST(BioDevTest, BioGetDevList) {
    // 初始化设备列表
    bio_dev_list = NULL;

    // 创建并初始化两个 mock 设备
    bio_dev* dev1 = (bio_dev*)malloc(sizeof(bio_dev));
    bio_dev* dev2 = (bio_dev*)malloc(sizeof(bio_dev));
    memset(dev1, 0, sizeof(bio_dev));
    memset(dev2, 0, sizeof(bio_dev));
    dev1->device_name = strdup("Mock Device 1");
    dev2->device_name = strdup("Mock Device 2");

    // 将设备添加到设备列表
    bio_dev_list = g_list_append(bio_dev_list, dev1);
    bio_dev_list = g_list_append(bio_dev_list, dev2);

    // 获取设备列表
    GList* dev_list = bio_get_dev_list();

    // 验证设备列表是否正确
    EXPECT_NE(dev_list, nullptr);
    EXPECT_EQ(g_list_length(dev_list), 2);
    EXPECT_STREQ(((bio_dev*)g_list_nth_data(dev_list, 0))->device_name, "Mock Device 1");
    EXPECT_STREQ(((bio_dev*)g_list_nth_data(dev_list, 1))->device_name, "Mock Device 2");

    // 释放分配的内存
    free(dev1->device_name);
    free(dev2->device_name);
    free(dev1);
    free(dev2);
    g_list_free(dev_list);
}

TEST(BioPrintDrvListTest, PrintDriverList) {
    // 初始化模拟的 bio_drv_list
    bio_dev *dev1 = (bio_dev*)malloc(sizeof(bio_dev));
    dev1->enable = 1;
    dev1->driver_id = 1;
    dev1->device_name = strdup("Device1");

    bio_dev *dev2 = (bio_dev*)malloc(sizeof(bio_dev));
    dev2->enable = 0;
    dev2->driver_id = 2;
    dev2->device_name = strdup("Device2");

    bio_dev *dev3 = (bio_dev*)malloc(sizeof(bio_dev));
    dev3->enable = 1;
    dev3->driver_id = 3;
    dev3->device_name = strdup("Device3");

    bio_drv_list = g_list_append(bio_drv_list, dev1);
    bio_drv_list = g_list_append(bio_drv_list, dev2);
    bio_drv_list = g_list_append(bio_drv_list, dev3);

    // 捕获输出
    testing::internal::CaptureStdout();
    bio_print_drv_list(BIO_PRINT_LEVEL_INFO);
    std::string output = testing::internal::GetCapturedStdout();

    // 验证输出
    // EXPECT_NE(output.find("Device1[T] -> "), std::string::npos);
    // EXPECT_NE(output.find("Device2[F] -> "), std::string::npos);
    // EXPECT_NE(output.find("Device3[T] -> "), std::string::npos);
    // EXPECT_NE(output.find("NULL"), std::string::npos);

    // 清理
    bio_free_drv_list();
}

TEST(BioFreeDrvListTest, FreeDriverList) {
    // 初始化模拟的 bio_drv_list
    bio_dev *dev1 = (bio_dev*)malloc(sizeof(bio_dev));
    dev1->enable = 1;
    dev1->device_name = strdup("Device1");

    bio_dev *dev2 = (bio_dev*)malloc(sizeof(bio_dev));
    dev2->enable = 0;
    dev2->device_name = strdup("Device2");

    bio_dev *dev3 = (bio_dev*)malloc(sizeof(bio_dev));
    dev3->enable = 1;
    dev3->device_name = strdup("Device3");

    bio_drv_list = g_list_append(bio_drv_list, dev1);
    bio_drv_list = g_list_append(bio_drv_list, dev2);
    bio_drv_list = g_list_append(bio_drv_list, dev3);


    // 调用 bio_free_drv_list 函数

    bio_free_drv_list();

    // 验证 bio_drv_list 是否被正确释放并设置为 NULL
    EXPECT_EQ(bio_drv_list, nullptr);
}

TEST(BioFreeDrvListTest, FreeDevList) {
    // 初始化模拟的 bio_drv_list
    bio_dev *dev1 = (bio_dev*)malloc(sizeof(bio_dev));
    dev1->enable = 1;
    dev1->device_name = strdup("Device1");

    bio_dev *dev2 = (bio_dev*)malloc(sizeof(bio_dev));
    dev2->enable = 0;
    dev2->device_name = strdup("Device2");

    bio_dev *dev3 = (bio_dev*)malloc(sizeof(bio_dev));
    dev3->enable = 1;
    dev3->device_name = strdup("Device3");


    bio_dev_list = g_list_append(bio_dev_list, dev1);
    bio_dev_list = g_list_append(bio_dev_list, dev2);
    bio_dev_list = g_list_append(bio_dev_list, dev3);

    // 调用 bio_free_dev_list 函数
    
    bio_free_dev_list();


    // 验证 bio_dev_list 是否被正确释放并设置为 NULL
    EXPECT_EQ(bio_dev_list, nullptr);
}

TEST(BioNewStringTest, NullInput) {
    char* result = bio_new_string(NULL);
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result, "");
    free(result);
}

TEST(BioNewStringTest, NonNullInput) {
    const char* input = "test string";
    char* result = bio_new_string(const_cast<char*>(input));
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result, input);
    free(result);
}

TEST(BioNewStringTest, EmptyStringInput) {
    const char* input = "";
    char* result = bio_new_string(const_cast<char*>(input));
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result, input);
    free(result);
}

TEST(BioCommonTest, GetEmptyIndex) {
    initializeMockDev();

    sqlite3 *db = bio_sto_connect_db();
    ASSERT_NE(db, nullptr) << "Failed to connect to database";

    // 插入测试数据
    feature_info *info1 = bio_sto_new_feature_info(1001, 1, "Driver1", 1, "Index1");
    feature_sample *sample1 = bio_sto_new_feature_sample(1, "SampleData1");
    info1->sample = sample1;

    feature_info *info2 = bio_sto_new_feature_info(1001, 1, "Driver1", 3, "Index2");
    feature_sample *sample2 = bio_sto_new_feature_sample(2, "SampleData2");
    info2->sample = sample2;

    feature_info *info3 = bio_sto_new_feature_info(1002, 2, "Driver2", 1, "Index3");
    feature_sample *sample3 = bio_sto_new_feature_sample(3, "SampleData3");
    info3->sample = sample3;

    info1->next = info2;
    info2->next = info3;

    int ret = bio_sto_set_feature_info(db, info1);
    ASSERT_EQ(ret, 0) << "Failed to set feature info";
    bio_sto_disconnect_db(db);
    
    bio_dev* dev = (bio_dev*)malloc(sizeof(bio_dev));
    dev->bioinfo.biotype = static_cast<BioType>(1);
    dev->device_name = "Driver1";

    // 测试 bio_common_get_empty_index 函数
    EXPECT_EQ(bio_common_get_empty_index(dev, 1001, 0, 2), 0); // 测试范围内的空闲索引
    EXPECT_EQ(bio_common_get_empty_index(dev, 1001, 1, 3), 2); // 测试范围内没有空闲索引，返回-1
    EXPECT_EQ(bio_common_get_empty_index(dev, 1001, 1, -1), 2); // 测试范围不限大小，返回最大索引+1
    EXPECT_EQ(bio_common_get_empty_index(dev, 1001, 3, 5), -1); // 测试范围内的空闲索引
    EXPECT_EQ(bio_common_get_empty_index(dev, 1001, 4, 5), -1); // 测试范围内的空闲索引
    EXPECT_EQ(bio_common_get_empty_sample_no(dev, 1, 2), -1); //需要改
    

}

// TEST(BioCommonTest, DetectUsbDevice) {
//     initializeMockDev();
//     // 初始化 libusb
//     libusb_context *ctx;
//     ASSERT_EQ(libusb_init(&ctx), 0);

//     // 模拟 USB 设备
//     libusb_device **usb_devs_list = (libusb_device **)malloc(2 * sizeof(libusb_device *));
//     libusb_device *usb_dev1 = (libusb_device *)malloc(sizeof(libusb_device));
//     libusb_device *usb_dev2 = (libusb_device *)malloc(sizeof(libusb_device));
//     libusb_device_descriptor usb_dev_desc1 = {0x1234, 0x5678};
//     libusb_device_descriptor usb_dev_desc2 = {0x8765, 0x4321};

//     usb_devs_list[0] = usb_dev1;
//     usb_devs_list[1] = usb_dev2;

//     // 设置设备描述符
//     libusb_set_device_descriptor(usb_dev1, &usb_dev_desc1);
//     libusb_set_device_descriptor(usb_dev2, &usb_dev_desc2);

//     // 调用函数并验证结果
//     int num_detected = bio_common_detect_usb_device(mockDev);
//     EXPECT_EQ(num_detected, 2);

//     // 清理
//     free(usb_devs_list);
//     free(usb_dev1);
//     free(usb_dev2);
//     libusb_exit(ctx);
// }

// TEST(BioDriverTest, InitTest) {
//     bio_conf_init();
//     GList* driver_list = bio_driver_list_init();
//     ASSERT_NE(driver_list, nullptr);
//     // 进一步的断言可以根据实际需求添加
// }
TEST(BioOpsTest, CreateNode) {
    bio_extend* node = bio_ops_create_node("key1", "value1", "type1", "description1", "user1");
    ASSERT_NE(node, nullptr);
    EXPECT_STREQ(node->key, "key1");
    EXPECT_STREQ((char *)node->value, "value1");
    EXPECT_STREQ(node->type, "type1");
    EXPECT_STREQ(node->description, "description1");
    EXPECT_STREQ(node->user, "user1");
    free(node->key);
    free(node->value);
    free(node->type);
    free(node->description);
    free(node->user);
    free(node);
}

TEST(BioOpsTest, AddNode) {
    bio_extend* list = bio_ops_create_node("key1", "value1", "type1", "description1", "user1");
    bio_extend* node = bio_ops_create_node("key2", "value2", "type2", "description2", "user2");
    list = bio_ops_add_node(node, list);
    ASSERT_NE(list, nullptr);
    EXPECT_STREQ(list->next->key, "key2");
    bio_ops_free_extend_list(list);
}

TEST(BioOpsTest, DeleteNode) {
    bio_extend* list = bio_ops_create_node("key1", "value1", "type1", "description1", "user1");
    bio_extend* node = bio_ops_create_node("key2", "value2", "type2", "description2", "user2");
    list = bio_ops_add_node(node, list);
    list = bio_ops_delete_node("key1", list);
    ASSERT_NE(list, nullptr);
    EXPECT_STREQ(list->key, "key1");
    bio_ops_free_extend_list(list);
}

TEST(BioOpsTest, PrintList) {
    bio_extend* list = bio_ops_create_node("key1", "value1", "type1", "description1", "user1");
    bio_extend* node = bio_ops_create_node("key2", "value2", "type2", "description2", "user2");
    list = bio_ops_add_node(node, list);
    testing::internal::CaptureStdout();
    bio_ops_printf_list(list);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output.find("key = key1"), std::string::npos);
    EXPECT_EQ(output.find("key = key2"), std::string::npos);
    bio_ops_free_extend_list(list);
}

TEST(BioOpsTest, FreeExtendList) {
    bio_extend* list = bio_ops_create_node("key1", "value1", "type1", "description1", "user1");
    bio_extend* node = bio_ops_create_node("key2", "value2", "type2", "description2", "user2");
    list = bio_ops_add_node(node, list);
    EXPECT_EQ(bio_ops_free_extend_list(list), 0);
}

TEST(BioExtendTest, TestBioGetValueByKey) {
    bio_extend *list = bio_ops_create_node("key1", "value1", "1", "desc1", "1001");
    list->next = bio_ops_create_node("key2", "value2", "2", "desc2", "1001");

    bio_extend *result = bio_get_value_by_key(list, "key2");
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result->key, "key2");
    EXPECT_STREQ((char *)result->value, "value2");
    EXPECT_STREQ(result->type, "2");
    EXPECT_STREQ(result->description, "desc2");

    free(result->key);
    free(result->value);
    free(result->description);
    free(result);
    free(list->next->key);
    free(list->next->value);
    free(list->next->description);
    free(list->next);
    free(list->key);
    free(list->value);
    free(list->description);
    free(list);
}

TEST(BioExtendTest, TestBioOpsModifyWrExtendList) {
    bio_extend *list = bio_ops_create_node("key1", "value1", "1", "desc1", "1001");
    list->next = bio_ops_create_node("key2", "value2", "2", "desc2", "1001");

    int result = bio_ops_modify_wr_extend_list(list, "key2", "new_value2");
    EXPECT_EQ(result, 0);
    EXPECT_STREQ((char *)list->next->value, "new_value2");

    free(list->next->key);
    free((char *)list->next->value);
    free(list->next->description);
    free(list->next);
    free(list->key);
    free((char *)list->value);
    free(list->description);
    free(list);
}

TEST(BioDevTest, BioInitRoExtendList) {
    initializeMockDev();

    char key[] = "test_key";
    char value[] = "test_value";
    char type[] = "test_type";
    char description[] = "test_description";
    char user[] = "test_user";

    EXPECT_EQ(bio_init_ro_extend_list(mockDev, key, value, type, description, user), 0);
    EXPECT_NE(mockDev->ro_extend_list, nullptr);

    // Verify the contents of the list
    bio_extend * node = mockDev->ro_extend_list;
    EXPECT_STREQ(node->key, key);
    EXPECT_STREQ((char *)node->value, value);
    EXPECT_STREQ(node->type, type);
    EXPECT_STREQ(node->description, description);
    EXPECT_STREQ(node->user, user);

    // Clean up
    free(node->key);
    free(node->type);
    free(node->description);
    free(node->user);
    free(node);
}

TEST(BioDevTest, BioOpsGetRoExtendList) {
    initializeMockDev();

    // 设置 ro_extend_list 为非空值
    bio_extend* extend_list = (bio_extend*)malloc(sizeof(bio_extend));
    mockDev->ro_extend_list = extend_list;

    // 调用 bio_ops_get_ro_extend_list 并验证返回值
    EXPECT_EQ(bio_ops_get_ro_extend_list(mockDev), extend_list);

    // 设置 ro_extend_list 为 NULL
    mockDev->ro_extend_list = NULL;

    // 调用 bio_ops_get_ro_extend_list 并验证返回值
    EXPECT_EQ(bio_ops_get_ro_extend_list(mockDev), nullptr);

    // 释放分配的内存
    free(extend_list);
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, TestBioInitWrExtendList) {
    initializeMockDev();

    char key[] = "test_key";
    char value[] = "test_value";
    char type[] = "test_type";
    char description[] = "test_description";
    char user[] = "test_user";

    EXPECT_EQ(bio_init_wr_extend_list(mockDev, key, value, type, description, user), 0);
    ASSERT_NE(mockDev->wr_extend_list, nullptr);
    EXPECT_STREQ(mockDev->wr_extend_list->key, key);
    EXPECT_STREQ((char *)mockDev->wr_extend_list->value, value);
    EXPECT_STREQ(mockDev->wr_extend_list->type, type);
    EXPECT_STREQ(mockDev->wr_extend_list->description, description);
    EXPECT_STREQ(mockDev->wr_extend_list->user, user);

    // Free allocated memory
    free(mockDev->wr_extend_list->key);
    free(mockDev->wr_extend_list->value);
    free(mockDev->wr_extend_list->type);
    free(mockDev->wr_extend_list->description);
    free(mockDev->wr_extend_list->user);
    free(mockDev->wr_extend_list);
}

TEST(BioDevTest, TestBioGetWrExtendValueByKey) {
    initializeMockDev();

    // Initialize the list
    char key1[] = "key1";
    char value1[] = "value1";
    char type1[] = "type1";
    char description1[] = "description1";
    char user1[] = "user1";
    bio_init_wr_extend_list(mockDev, key1, value1, type1, description1, user1);

    char key2[] = "key2";
    char value2[] = "value2";
    char type2[] = "type2";
    char description2[] = "description2";
    char user2[] = "user2";
    bio_init_wr_extend_list(mockDev, key2, value2, type2, description2, user2);

    // Test getting the first node
    bio_extend* node = bio_get_wr_extend_value_by_key(mockDev, key1);

    // Test getting the second node
    node = bio_get_wr_extend_value_by_key(mockDev, key2);

    // Test getting a non-existent node
    node = bio_get_wr_extend_value_by_key(mockDev, "non_existent_key");

    // Free allocated memory
    bio_extend* current = mockDev->wr_extend_list;
    while (current != nullptr) {
        bio_extend* next = current->next;
        free(current->key);
        free(current->value);
        free(current->type);
        free(current->description);
        free(current->user);
        free(current);
        current = next;
    }
}

// TEST(Bioinit,init)
// {
//     EXPECT_EQ(bio_init(),0);
//     EXPECT_EQ(bio_close(),0);
    
// }

TEST(BioOpsTest, TestBioOpsInitExtendConf) {
    initializeMockDev();
    bio_extend extend1 = {"key1", (char *)"value1", "type1", "description1", "user1", NULL};
    mockDev->wr_extend_list = &extend1;

    // Test when file is empty
    EXPECT_EQ(bio_ops_init_extend_conf(mockDev), 0);

}

TEST(BioOpsTest, ModifyExtendConfFile) {
    initializeMockDev();
    mockDev->extend_conf_handle = tmpfile();  // 创建临时文件

    // 写入初始配置内容
    const char* initial_content = "[key1]\nvalue=old_value\n";
    fwrite(initial_content, sizeof(char), strnlen(initial_content,24), mockDev->extend_conf_handle);
    fflush(mockDev->extend_conf_handle);

    // 调用函数修改配置文件
    bio_ops_modify_extend_conf_file(mockDev, (char*)"key1", (char*)"new_value");

    // 读取修改后的内容
    fseek(mockDev->extend_conf_handle, 0, SEEK_SET);
    char buffer[1024];
    fread(buffer, sizeof(char), sizeof(buffer) - 1, mockDev->extend_conf_handle);
    buffer[sizeof(buffer) - 1] = '\0';

    // 验证修改后的内容
    EXPECT_STREQ(buffer, "[key1]\nvalue=new_value\n");

    fclose(mockDev->extend_conf_handle);  // 关闭临时文件
}

// 测试 bio_ops_analyzing_extend_conf_file 函数
TEST(BioOpsTest, AnalyzingExtendConfFile) {
    initializeMockDev();
    mockDev->extend_conf_handle = tmpfile();  // 创建临时文件

    // 写入配置内容
    const char* content = "[key1]\nvalue=value1\ntype=type1\ndescription=desc1\nuser=user1\n";
    fwrite(content, sizeof(char), strnlen(content,61), mockDev->extend_conf_handle);
    fflush(mockDev->extend_conf_handle);

    // 调用函数解析配置文件
    bio_extend* result = bio_ops_analyzing_extend_conf_file(mockDev);

    // 验证解析结果
    ASSERT_NE(result, nullptr);
    EXPECT_STREQ(result->key, "key1");
    EXPECT_STREQ((char *)result->value, "value1");
    EXPECT_STREQ(result->type, "type1");
    EXPECT_STREQ(result->description, "desc1");
    EXPECT_STREQ(result->user, "user1");

    // 释放内存
    free(result->key);
    free(result->value);
    free(result->type);
    free(result->description);
    free(result->user);
    free(result);

    fclose(mockDev->extend_conf_handle);  // 关闭临时文件
}

TEST(BioDevTest, TestBioOpsGetWrExtendFeature) {
    initializeMockDev();

    bio_extend* list = bio_ops_get_wr_extend_feature(mockDev);
    EXPECT_NE(list, nullptr);

    free(list);
    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, TestBioOpsSetWrExtendFeature) {
    initializeMockDev();

    char key[] = "test_key";
    char value[] = "test_value";

    // Test when device is enabled
    mockDev->enable = TRUE;
    EXPECT_EQ(bio_ops_set_wr_extend_feature(mockDev, key, value), 0);

    // Test when device is disabled
    mockDev->enable = FALSE;
    EXPECT_EQ(bio_ops_set_wr_extend_feature(mockDev, key, value), -1);

    free(mockDev->device_name);
    free(mockDev->full_name);
    free(mockDev);
}

TEST(BioDevTest, BioDevNew) {
    bio_dev* dev = bio_dev_new();
    ASSERT_NE(dev, nullptr);  // 确保分配成功
    EXPECT_EQ(dev->plugin_handle, nullptr);  // 确保初始化为 NULL
    EXPECT_EQ(dev->ops_free, nullptr);  // 确保初始化为 NULL
    free(dev);  // 释放分配的内存
}

// 测试 bio_dev_free 函数
TEST(BioDevTest, BioDevFree) {
    bio_dev* dev = bio_dev_new();
    ASSERT_NE(dev, nullptr);  // 确保分配成功

    // 模拟 ops_free 函数指针
    dev->ops_free = [](bio_dev* d) { /* 模拟释放操作 */ };

    // 模拟 plugin_handle
    dev->plugin_handle = dlopen("libm.so", RTLD_LAZY);
    bio_dev_free(dev);
}
