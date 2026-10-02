#include <gtest/gtest.h>
#include <gmock/gmock.h>  // 添加 gmock 头文件以支持模拟对象
#include <fstream>
using namespace testing;
extern "C" {
#include "../../lib/biometric_config.h"  // 添加 biometric_config.h 头文件
#include "../../lib/biometric_common.h"
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
    char *value = g_key_file_get_string(conf, "Settings", "key", nullptr);
    g_free(value);

    // 清理配置文件
    bio_conf_free();

    // 删除临时配置文件
    std::remove(DRIVERS_CONFIG_FILE);
}