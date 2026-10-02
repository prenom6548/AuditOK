#include <gtest/gtest.h>
#include <gmock/gmock.h>  // 添加 gmock 头文件以支持模拟对象
#include <fstream>
using namespace testing;

extern "C" {
#include "../../lib/biometric_stroge.h"
#include "../../lib/biometric_common.h"  
#include "../../lib/biometric_config.h"  // 添加 biometric_config.h 头文件
#include "../../lib/biometric_version.h"
int internal_utf8_char_len(char firstByte);
int internal_utf8_str_len(char *str);
int internal_utf8_str_width(char *str);
char * internal_newstr(char * old_str);
int internal_create_dir(const char *new_dir);
void internal_get_uuid_by_uid(int uid, char * uuid);
int internal_create_datebase_info(sqlite3 *db);
int internal_create_eigen_info_table(sqlite3 *db);
}

TEST(StorageTest, BioStoNewStr) {
    // 测试 bio_sto_new_str 函数
    const char* test_str = "Hello, World!";
    char* result = bio_sto_new_str((char*)test_str);
    EXPECT_STREQ(result, test_str);
    free(result);

    // 测试 bio_sto_new_str 函数处理空字符串
    result = bio_sto_new_str(NULL);
    EXPECT_STREQ(result, "");
    free(result);

    // 测试 bio_sto_new_str 函数处理空字符串
    result = bio_sto_new_str("");
    EXPECT_STREQ(result, "");
    free(result);
}

TEST(StorageTest, BioStoFeatureSampleAndInfo) {
    // 测试 bio_sto_new_feature_sample 和 bio_sto_free_feature_sample
    feature_sample *sample1 = bio_sto_new_feature_sample(1, "SampleData1");
    ASSERT_NE(sample1, nullptr) << "Failed to create feature sample";
    EXPECT_EQ(sample1->no, 1);
    EXPECT_STREQ(sample1->data, "SampleData1");
    bio_sto_free_feature_sample(sample1);

    feature_sample *sample2 = bio_sto_new_feature_sample(2, nullptr);
    ASSERT_NE(sample2, nullptr) << "Failed to create feature sample with null data";
    EXPECT_EQ(sample2->no, 2);
    EXPECT_EQ(sample2->data, nullptr);
    bio_sto_free_feature_sample(sample2);

    // 测试 bio_sto_new_feature_info 和 bio_sto_free_feature_info
    feature_info *info1 = bio_sto_new_feature_info(1001, 1, "Driver1", 1, "Index1");
    ASSERT_NE(info1, nullptr) << "Failed to create feature info";
    EXPECT_EQ(info1->uid, 1001);
    EXPECT_EQ(info1->biotype, 1);
    EXPECT_STREQ(info1->driver, "Driver1");
    EXPECT_EQ(info1->index, 1);
    EXPECT_STREQ(info1->index_name, "Index1");
    EXPECT_EQ(info1->sample, nullptr);
    EXPECT_EQ(info1->next, nullptr);
    bio_sto_free_feature_info(info1);

    feature_info *info2 = bio_sto_new_feature_info(1001, 2, "Driver1", 2, "Index2");
    ASSERT_NE(info2, nullptr) << "Failed to create feature info";
    EXPECT_EQ(info2->uid, 1001);
    EXPECT_EQ(info2->biotype, 2);
    EXPECT_STREQ(info2->driver, "Driver1");
    EXPECT_EQ(info2->index, 2);
    EXPECT_STREQ(info2->index_name, "Index2");
    EXPECT_EQ(info2->sample, nullptr);
    EXPECT_EQ(info2->next, nullptr);
    bio_sto_free_feature_info(info2);

    feature_info *info3 = bio_sto_new_feature_info(1002, 2, "Driver2", 1, "Index3");
    ASSERT_NE(info3, nullptr) << "Failed to create feature info";
    EXPECT_EQ(info3->uid, 1002);
    EXPECT_EQ(info3->biotype, 2);
    EXPECT_STREQ(info3->driver, "Driver2");
    EXPECT_EQ(info3->index, 1);
    EXPECT_STREQ(info3->index_name, "Index3");
    EXPECT_EQ(info3->sample, nullptr);
    EXPECT_EQ(info3->next, nullptr);
    bio_sto_free_feature_info(info3);

    // 测试 bio_sto_free_feature_info_list
    info1 = bio_sto_new_feature_info(1001, 1, "Driver1", 1, "Index1");
    info2 = bio_sto_new_feature_info(1001, 2, "Driver1", 2, "Index2");
    info3 = bio_sto_new_feature_info(1002, 2, "Driver2", 1, "Index3");

    info1->next = info2;
    info2->next = info3;

    bio_sto_free_feature_info_list(info1);
}

// 新增测试用例：测试 bio_sto_get_feature_info, bio_sto_set_feature_info, bio_sto_clean_feature_info
TEST(StorageTest, BioStoFeatureInfoFunctions) {
    sqlite3 *db = bio_sto_connect_db();
    ASSERT_NE(db, nullptr) << "Failed to connect to database";

    // 插入测试数据
    feature_info *info1 = bio_sto_new_feature_info(1001, 1, "Driver1", 1, "Index1");
    feature_sample *sample1 = bio_sto_new_feature_sample(1, "SampleData1");
    info1->sample = sample1;

    feature_info *info2 = bio_sto_new_feature_info(1001, 1, "Driver1", 2, "Index2");
    feature_sample *sample2 = bio_sto_new_feature_sample(2, "SampleData2");
    info2->sample = sample2;

    feature_info *info3 = bio_sto_new_feature_info(1002, 2, "Driver2", 1, "Index3");
    feature_sample *sample3 = bio_sto_new_feature_sample(3, "SampleData3");
    info3->sample = sample3;

    info1->next = info2;
    info2->next = info3;

    int ret = bio_sto_set_feature_info(db, info1);
    ASSERT_EQ(ret, 0) << "Failed to set feature info";

    // 测试 bio_sto_get_feature_info
    feature_info *retrieved_info = bio_sto_get_feature_info(db, 1001, 1, "Driver1", 1, 2);
    ASSERT_NE(retrieved_info, nullptr) << "Failed to get feature info";

    feature_info *current_info = retrieved_info;
    EXPECT_EQ(current_info->uid, 1001);
    EXPECT_EQ(current_info->biotype, 1);
    EXPECT_STREQ(current_info->driver, "Driver1");
    EXPECT_EQ(current_info->index, 1);
    EXPECT_STREQ(current_info->index_name, "Index1");
    EXPECT_EQ(current_info->sample->no, 1);
    EXPECT_STREQ(current_info->sample->data, "SampleData1");

    current_info = current_info->next;
    EXPECT_EQ(current_info->uid, 1001);
    EXPECT_EQ(current_info->biotype, 1);
    EXPECT_STREQ(current_info->driver, "Driver1");
    EXPECT_EQ(current_info->index, 2);
    EXPECT_STREQ(current_info->index_name, "Index2");
    EXPECT_EQ(current_info->sample->no, 2);
    EXPECT_STREQ(current_info->sample->data, "SampleData2");

    bio_sto_free_feature_info_list(retrieved_info);

    // 测试 bio_sto_clean_feature_info
    ret = bio_sto_clean_feature_info(db, 1001, 1, "Driver1", 1, 2);
    ASSERT_EQ(ret, 0) << "Failed to clean feature info";

    retrieved_info = bio_sto_get_feature_info(db, 1001, 1, "Driver1", 1, 2);
    EXPECT_EQ(retrieved_info, nullptr) << "Feature info should be cleaned";

    // 清理数据库
    bio_sto_free_feature_info_list(info1);
    bio_sto_disconnect_db(db);
}

TEST(StorageTest, bioStoUpdateFeatureInfoByDbid) {
    sqlite3 *db = bio_sto_connect_db();
    ASSERT_NE(db, nullptr) << "Failed to connect to database";
    int ret = bio_sto_update_feature_info_by_dbid(db, 9, 1001, 1, "Driver1",2,"index4", 4);
    ASSERT_EQ(ret, 0) << "Failed to update feature info by dbid";
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

// 新增测试用例：测试 bio_conf_init 函数
TEST(ConfigTest, BioConfInit) {
    // 创建一个临时配置文件
    std::ofstream config_file(DRIVERS_CONFIG_FILE);
    config_file << "[Settings]\nkey=value\n";
    config_file.close();

    // 初始化配置
    int result = bio_conf_init();
    EXPECT_EQ(result, 0) << "bio_conf_init should return 0 on success";

    // 检查配置文件是否正确加载
    GKeyFile *conf = get_bio_conf();
    EXPECT_NE(conf, nullptr) << "Configuration file should be loaded";

    // 检查配置文件中的键值对
    GError **error;
    char *value = g_key_file_get_string(conf, "Settings", "key", error);
    // EXPECT_STREQ(value, "value") << "Configuration key 'key' should have value 'value'";
    g_free(value);
    
    // 清理配置文件
    bio_conf_free();

    // 删除临时配置文件
    std::remove(DRIVERS_CONFIG_FILE);
}

// 测试 internal_utf8_char_len 函数
TEST(InternalTest, Utf8CharLen) {
    EXPECT_EQ(internal_utf8_char_len('a'), 1);
    EXPECT_EQ(internal_utf8_char_len(0xC2), 2);
    EXPECT_EQ(internal_utf8_char_len(0xE0), 3);
    EXPECT_EQ(internal_utf8_char_len(0xF0), 4);
}

// 测试 internal_utf8_str_len 函数
TEST(InternalTest, Utf8StrLen) {
    char str1[] = "Hello";
    char str2[] = "你好";
    EXPECT_EQ(internal_utf8_str_len(str1), 5);
    EXPECT_EQ(internal_utf8_str_len(str2), 2);
}

// 测试 internal_utf8_str_width 函数
TEST(InternalTest, Utf8StrWidth) {
    char str1[] = "Hello";
    char str2[] = "你好";
    EXPECT_EQ(internal_utf8_str_width(str1), 5);
    EXPECT_EQ(internal_utf8_str_width(str2), 4);
}

// 测试 internal_newstr 函数
TEST(InternalTest, NewStr) {
    char str1[] = "Hello";
    char *new_str1 = internal_newstr(str1);
    ASSERT_NE(new_str1, nullptr);
    EXPECT_STREQ(new_str1, str1);
    free(new_str1);

    char *new_str2 = internal_newstr(nullptr);
    ASSERT_NE(new_str2, nullptr);
    EXPECT_STREQ(new_str2, "");
    free(new_str2);
}

// 测试 internal_create_dir 函数
TEST(InternalTest, CreateDir) {
    const char *dir = "./test_dir";
    EXPECT_EQ(internal_create_dir(dir), 0);
    struct stat st;
    EXPECT_EQ(stat(dir, &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    rmdir(dir);
}

TEST(InternalGetUuidByUidTest, GetPwUidReturnsNull) {
    char uuid[37];
    internal_get_uuid_by_uid(-1, uuid);
    EXPECT_STREQ(uuid, "01234567-0123-0123-0123-0123456789AB");
}

TEST(InternalGetUuidByUidTest, AccessReturnsNonZero) {
    char uuid[37];
    internal_get_uuid_by_uid(1000, uuid);
    EXPECT_STREQ(uuid, "76772da7-f97b-4d18-82b4-61f3240a81df");
}

TEST(InternalGetUuidByUidTest, UuidGenerateAndParse) {
    char uuid[37];
    internal_get_uuid_by_uid(1000, uuid);
    EXPECT_STREQ(uuid, "76772da7-f97b-4d18-82b4-61f3240a81df");
}

TEST(DatabaseTest, CreateEigenInfoTable) {
    sqlite3 *db;
    int ret = sqlite3_open(":memory:", &db);
    ASSERT_EQ(ret, SQLITE_OK) << "Failed to open database";

    ret = internal_create_eigen_info_table(db);
    EXPECT_EQ(ret, 0) << "Failed to create eigen info table";

    // 检查表是否创建成功
    sqlite3_stmt *stmt;
    const char *sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" TABLE_EIGEN_INFO "';";
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    ASSERT_EQ(ret, SQLITE_OK) << "Failed to prepare statement";

    ret = sqlite3_step(stmt);
    EXPECT_EQ(ret, SQLITE_ROW) << "Table " TABLE_EIGEN_INFO " does not exist";

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

TEST(DatabaseTest, CreateDatabaseInfo) {
    sqlite3 *db;
    int ret = sqlite3_open(":memory:", &db);
    ASSERT_EQ(ret, SQLITE_OK) << "Failed to open database";

    ret = internal_create_datebase_info(db);
    EXPECT_EQ(ret, 0) << "Failed to create database info table";

    // 检查表是否创建成功
    sqlite3_stmt *stmt;
    const char *sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" TABLE_DATABASE_FORMAT "';";
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    ASSERT_EQ(ret, SQLITE_OK) << "Failed to prepare statement";

    ret = sqlite3_step(stmt);
    EXPECT_EQ(ret, SQLITE_ROW) << "Table " TABLE_DATABASE_FORMAT " does not exist";

    sqlite3_finalize(stmt);

    // 检查插入的数据是否正确
    sql = "SELECT FormatMajor, FormatMinor, FormatFunction FROM " TABLE_DATABASE_FORMAT " WHERE ID = 1;";
    ret = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    ASSERT_EQ(ret, SQLITE_OK) << "Failed to prepare statement";

    ret = sqlite3_step(stmt);
    ASSERT_EQ(ret, SQLITE_ROW) << "No data found in " TABLE_DATABASE_FORMAT;

    EXPECT_EQ(sqlite3_column_int(stmt, 0), DATABASE_FORMAT_MAJOR);
    EXPECT_EQ(sqlite3_column_int(stmt, 1), DATABASE_FORMAT_MINOR);
    EXPECT_EQ(sqlite3_column_int(stmt, 2), DATABASE_FORMAT_FUNC);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}