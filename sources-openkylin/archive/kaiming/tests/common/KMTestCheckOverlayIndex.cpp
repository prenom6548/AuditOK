/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/utsname.h>

#include "common/KMOverlay.h"

namespace fs = std::filesystem;

class KMCheckOverlayIndexTest : public ::testing::Test
{
protected:
    std::string tempBootDir;
    std::string tempOptDir;
    std::string originalBoot;
    std::string originalOpt;

    void SetUp() override
    {
        // 创建临时目录模拟 /boot 和 /opt/kaiming-tools
        char bootTemplate[] = "/tmp/test_bootXXXXXX";
        char optTemplate[] = "/tmp/test_optXXXXXX";
        tempBootDir = mkdtemp(bootTemplate);
        tempOptDir = mkdtemp(optTemplate);

        ASSERT_FALSE(tempBootDir.empty());
        ASSERT_FALSE(tempOptDir.empty());

        // 获取 uname 信息
        struct utsname uts;
        ASSERT_EQ(uname(&uts), 0);
        std::string unameInfo = uts.release;

        // 写入 /boot/config-$(uname -r)
        std::ofstream ofs(tempBootDir + "/config-" + unameInfo);
        ofs << "CONFIG_OVERLAY_FS_INDEX=y" << std::endl;
        ofs.close();

        // 创建模拟路径
        fs::create_directories(tempOptDir + "/kaiming-tools/config");

        // 绑定挂载
        system(("mount --bind " + tempBootDir + " /boot").c_str());
        system(("mount --bind " + tempOptDir + " /opt/kaiming-tools").c_str());
    }

    void TearDown() override
    {
        system("umount /boot");
        system("umount /opt/kaiming-tools");
        fs::remove_all(tempBootDir);
        fs::remove_all(tempOptDir);
    }
};

TEST_F(KMCheckOverlayIndexTest, OverlayIndexConfigShouldBeTrue)
{
    KMOverlay checker;
    checker.checkOverlayConfig();

    std::string outputFile = "/opt/kaiming-tools/config/overlay_index.config";
    std::ifstream ifs(outputFile);
    ASSERT_TRUE(ifs.is_open());

    std::string line;
    std::getline(ifs, line);
    EXPECT_EQ(line, "CONFIG_OVERLAY_FS_INDEX=true");
}
