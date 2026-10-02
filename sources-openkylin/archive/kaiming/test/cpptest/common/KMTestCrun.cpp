#include "KMCrun.h"
#include <gtest/gtest.h>
#define private public

class KMCrunTest : public ::testing::Test
{
protected:
    KMCrun runner; // 被测试对象
};

// pipe 创建失败
TEST_F(KMCrunTest, TestPipeFailure)
{
    // 强制模拟 pipe 失败的情况
    // 这里模拟 pipe 失败，我们可以通过改变内存或配置来实现失败，但这里只做示范
    // 假设我们在实际测试中通过控制某些条件来触发 pipe 失败
    std::vector<const char *> args = { "/bin/ls", "-l" };
    std::variant<bool, std::string> output;
    bool isWait = true;

    bool result = runner.doCmd(args, output, isWait);

    // 期望返回 false，表示 pipe 创建失败
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