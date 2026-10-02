#include <gtest/gtest.h>
#include "KMInstalledAppQuery.h"

class KMInstalledAppQueryTest : public ::testing::Test {
protected:
    KMInstalledAppQuery query;
};

// 测试精确匹配
TEST_F(KMInstalledAppQueryTest, ExactMatch) {
    std::vector<std::string> versions = {"1.0", "2.0", "3.0"};
    std::vector<std::string> result = query.matchVersions(versions, "2.0");
    ASSERT_EQ(result, std::vector<std::string>{"2.0"});
}

// 测试空版本号（返回全部有序版本）
TEST_F(KMInstalledAppQueryTest, EmptyVersion) {
    std::vector<std::string> versions = {"3.0.0", "2.0.0", "1.0.0"};
    std::vector<std::string> result = query.matchVersions(versions, "");
    // 假设 orderVersions 是升序排序
    ASSERT_EQ(result, versions);
}

// 测试非标准版本号（如带 "v" 前缀）
TEST_F(KMInstalledAppQueryTest, NonStandardVersion) {
    std::vector<std::string> versions = {"v1.0", "v2.0-beta", "1.5"};
    std::vector<std::string> result = query.matchVersions(versions, "v1.0");
    // 取决于 KMVersion::parse 是否支持 "v" 前缀
    ASSERT_EQ(result, std::vector<std::string>{"v1.0"});
}