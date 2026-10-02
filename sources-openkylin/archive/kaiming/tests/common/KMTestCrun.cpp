/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include <sys/resource.h>
#include <sys/time.h>

#define private public

#include "common/KMCrun.h"

class KMCrunTest : public ::testing::Test
{
protected:
    KMCrun runner; // 被测试对象
};

TEST_F(KMCrunTest, TestPipeFailure)
{
    // 1. 准备正确参数
    std::vector<const char *> args = { "/bin/ls", "-l", nullptr };
    std::variant<bool, std::string> output = true; // 注意：这里是 bool 分支

    // 2. 模拟 FD 耗尽，迫使源码里的 pipe(pipes) 返回 -1
    struct rlimit old_lim, new_lim;
    getrlimit(RLIMIT_NOFILE, &old_lim);
    new_lim.rlim_cur = 4; // 限制只有 4 个 FD（0,1,2 已占用），pipe 必失败
    new_lim.rlim_max = old_lim.rlim_max;
    setrlimit(RLIMIT_NOFILE, &new_lim);

    // 3. 执行
    bool result = runner.doCmd(args, output, true);

    // 4. 恢复限制
    setrlimit(RLIMIT_NOFILE, &old_lim);

    // 5. 验证
    // 如果 pipe 失败，源码应该直接 return false
    // 且不应该看到 "ls: 写入错误" 或 "execvp failed"，因为根本没走到 fork
    ASSERT_FALSE(result);
}

TEST_F(KMCrunTest, DoCmd_Wait_Success)
{
    std::vector<const char *> args = { "/bin/ls", "-l", nullptr };
    
    std::variant<bool, std::string> output = true; // 或者是 std::string("")
    
    bool isWait = true; 

    bool success = runner.doCmd(args, output, isWait);
    
    EXPECT_TRUE(success);
}
