/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */


#include <gtest/gtest.h>

#include "app/run/KMRunnerPlugins.h"
#include "common/KMContainer.h"

class KMTestRunnerPlugins
{
public:
    KMRunnerPlugins m_runnerPlugins;
};

TEST(KMTestRunnerPlugins, test_mountPlugins_with_hostPlugins)
{
    KMTestRunnerPlugins testPlugins;
    KMContainer container; // 创建一个 KMContainer 对象
    std::string appDataPath = "/tmp/testAppData";
    Annotations annotations;
    annotations.hostPlugins = true; // 测试 hostPlugins 为 true 的情况
    std::string appDir = "/tmp/testAppDir";

    // 测试 mountPlugins 函数是否能正常执行
    EXPECT_TRUE(testPlugins.m_runnerPlugins.mountPlugins(&container, appDataPath, annotations, appDir));
}


