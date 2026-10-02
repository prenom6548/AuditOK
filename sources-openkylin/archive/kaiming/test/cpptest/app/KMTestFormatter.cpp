#include "KMFormatter.h"
#include <gtest/gtest.h>

// 移除重复的 KMCurl 类定义

class KMTestFormatter
{
public:
    KMTestFormatter() = default;
    ~KMTestFormatter() = default;

public:
    KMFormatter m_formatter;
};

TEST(KMTestFormatter, test_formatter_truncate)
{
    KMTestFormatter truncate;
    size_t width = 8;
    std::string str = "http://172.30.60.17/repo/";
    std::string result = truncate.m_formatter.truncate(str, width);
    EXPECT_EQ(result, "http...");
}

TEST(KMTestFormatter, test_formatter_pad)
{
    KMTestFormatter pad;
    size_t width = 8;
    std::string str = "http://172.30.60.17/repo/";
    std::string result = pad.m_formatter.pad(str, width);
    EXPECT_EQ(result, "http://172.30.60.17/repo/");
}

TEST(KMTestFormatter, test_formatter_getColumnHead)
{
    KMTestFormatter getColumnHead;
    std::string str = "http://172.30.60.17/repo/";
    std::string result = getColumnHead.m_formatter.getColumnHead(str);
    EXPECT_EQ(result, "http://172.30.60.17/repo/");
}