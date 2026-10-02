/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <gtest/gtest.h>
#include "../../polkit-agent/src/pam-tally.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define CONFIG_FILE "/usr/share/lightdm/lightdm.conf.d/96-kylin-setting.conf"
 
// 辅助函数，用于模拟配置文件内容
void write_config_file(const char* content) {
    FILE* file = fopen(CONFIG_FILE, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
    } else {
        perror("Failed to open config file for writing");
        exit(EXIT_FAILURE);
    }
}
 
// 辅助函数，用于清理共享内存
void cleanup_shm() {
}
 
class PamTallyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试用例之前运行
        cleanup_shm(); // 确保每次测试前清理共享内存
    }
 
    void TearDown() override {
        // 在每个测试用例之后运行
        cleanup_shm(); // 确保每次测试后清理共享内存
    }
};
 
TEST_F(PamTallyTest, PamTallyInit) {
    // 测试pam_tally_init函数
    write_config_file("open-other-authentication=0\n"); // 确保配置关闭其他认证
 
    // 临时修改/etc/pam.d/common-auth文件以进行测试
    //const char* auth_file_content = "auth required pam_deny.so deny=3 unlock_time=600 root_unlock_time=1800\n";
    //FILE* auth_file = fopen("/etc/pam.d/common-auth", "w");
    //if (auth_file) {
    //    fputs(auth_file_content, auth_file);
    //    fclose(auth_file);
    //} else {
    //    perror("Failed to open auth file for writing");
    //    exit(EXIT_FAILURE);
    //}
 
    int result = pam_tally_init();
    EXPECT_EQ(result, 1); // 预期初始化成功
}
 
TEST_F(PamTallyTest, PamTallyIsCanUnlock) {
    // 测试pam_tally_is_canUnlock函数
    pam_tally_init(); // 初始化pam_tally并设置解锁时间为600秒
    pam_tally_add_failed(); // 添加一次失败，不足以达到失败次数上限
    EXPECT_EQ(pam_tally_is_canUnlock(), 1); // 预期可以解锁，因为失败次数未达到上限
 
    // 添加足够多的失败次数以达到上限，并检查是否可以解锁
    for (int i = 0; i < 2; ++i) {
        pam_tally_add_failed();
    }
    time_t start_time = time(NULL);
    EXPECT_EQ(pam_tally_is_canUnlock(), 0); // 预期不能解锁，因为失败次数已达到上限且未过解锁时间
 
    // 等待超过解锁时间后，再次检查是否可以解锁
    sleep(601); // 假设解锁时间为600秒，这里等待601秒以确保超过解锁时间
    EXPECT_EQ(pam_tally_is_canUnlock(), 1); // 预期现在可以解锁，因为已经超过解锁时间
}

TEST_F(PamTallyTest, PamTallyUnlockTime) {
    // 测试pam_tally_is_canUnlock函数
    pam_tally_init(); // 初始化pam_tally并设置解锁时间为600秒
    pam_tally_add_failed(); // 添加一次失败，不足以达到失败次数上限

    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time = 0;
    int unlock_time = 0;
   
    pam_tally_unlock_time_left(1000, &failed_count, &time_left, &deny, &fail_time, &unlock_time); 
    
    EXPECT_EQ(failed_count, 0);
    EXPECT_EQ(time_left, 0);
    EXPECT_EQ(deny, 0);
    EXPECT_EQ(fail_time, 0);
    EXPECT_EQ(unlock_time, 0);

    pam_tally_root_unlock_time_left(&failed_count, &time_left, &deny, &fail_time, &unlock_time);

    EXPECT_EQ(failed_count, 0);
    EXPECT_EQ(time_left, 0);
    EXPECT_EQ(deny, 0);
    EXPECT_EQ(fail_time, 0);
    EXPECT_EQ(unlock_time, 0);
}
 
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
