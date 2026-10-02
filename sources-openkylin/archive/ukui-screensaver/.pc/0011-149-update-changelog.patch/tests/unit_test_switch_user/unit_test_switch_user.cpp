#include <gtest/gtest.h>

#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../src/common/global_utils.h"
#include "../../src/lock-backend/switchuserutils.h"

#include "stubext.h"
using namespace stub_ext;

class SwitchUserUtilsTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pSwitchUserUtilsDbus = new SwitchUserUtils();
    }

    static void TearDownTestSuite()
    {
        delete m_pSwitchUserUtilsDbus;
        m_pSwitchUserUtilsDbus = nullptr;
    }

    static SwitchUserUtils *m_pSwitchUserUtilsDbus;
};

SwitchUserUtils *SwitchUserUtilsTest::m_pSwitchUserUtilsDbus = nullptr;


// 测试 GetCurUserName 函数
TEST_F(SwitchUserUtilsTest, GetCurUserName) {
    QString userName = m_pSwitchUserUtilsDbus->GetCurUserName();
    EXPECT_EQ(userName, "testuser");
}


// 测试 GetUidByName 函数
TEST_F(SwitchUserUtilsTest, GetUidByName) {
    int uid = m_pSwitchUserUtilsDbus->GetUidByName(getenv("USER"));
    EXPECT_EQ(uid, 1000);
}


// 测试 GetUserUDII 函数
TEST_F(SwitchUserUtilsTest, GetUserUDII) {
    UserDisplayIfInfo userDisplayIfInfo = m_pSwitchUserUtilsDbus->GetUserUDII(getenv("USER"));


    EXPECT_EQ(userDisplayIfInfo.strSeatPath, "/org/freedesktop/DisplayManager/Seat1");
    EXPECT_EQ(userDisplayIfInfo.strUserName, "testuser");
    EXPECT_EQ(userDisplayIfInfo.strSessionPath, "/org/freedesktop/DisplayManager/Session1");
}


// 测试 SwitchToUserSession 函数
TEST_F(SwitchUserUtilsTest, SwitchToUserSession) {
    UserDisplayIfInfo userDisplayIfInfo;
    userDisplayIfInfo.strSeatPath = "/org/freedesktop/DisplayManager/Seat1";
    userDisplayIfInfo.strUserName = "testuser";
    userDisplayIfInfo.strSessionPath = "/org/freedesktop/DisplayManager/Session1";
    int result = m_pSwitchUserUtilsDbus->SwitchToUserSession("/org/freedesktop/DisplayManager/Seat1", userDisplayIfInfo);
    EXPECT_EQ(result, 0);
}


// 测试 SwitchToUserLock 函数
TEST_F(SwitchUserUtilsTest, SwitchToUserLock) {
    bool result = m_pSwitchUserUtilsDbus->SwitchToUserLock();
    EXPECT_TRUE(result);
}
