#include "test_datetimeutils.h"

// 测试构造函数
TEST_F(DateTimeUtilsTest, ConstructorTest) {
    ASSERT_NE(datetimeUtils, nullptr);
    
    // 验证初始状态
    QString oldFormat = datetimeUtils->getOldDateFormat();
    EXPECT_FALSE(oldFormat.isEmpty());
}

// 测试TranslateDateFormat(const QDateTime &datetime)方法
TEST_F(DateTimeUtilsTest, TranslateDateFormatFromQDateTimeTest) {
    QDateTime datetime = QDateTime::currentDateTime();
    QString result = datetimeUtils->TranslateDateFormat(datetime);
    EXPECT_FALSE(result.isEmpty());
    
    // 测试特定日期
    QDateTime specificDate = QDateTime(QDate(2023, 1, 1), QTime(12, 0, 0));
    QString result2 = datetimeUtils->TranslateDateFormat(specificDate);
    EXPECT_FALSE(result2.isEmpty());
}

// 测试TranslateDateFormat(const QString &dateTime, const QString &oldFormat)方法
TEST_F(DateTimeUtilsTest, TranslateDateFormatFromStringTest) {
    // 测试标准格式转换
    QString dateString = "2023-01-01 12:00:00";
    QString result = datetimeUtils->TranslateDateFormat(dateString);
    EXPECT_FALSE(result.isEmpty());
    
    // 测试无效日期字符串
    QString invalidDateString = "invalid-date";
    QString result2 = datetimeUtils->TranslateDateFormat(invalidDateString);
    EXPECT_EQ(result2, invalidDateString);
    
    // 测试自定义格式
    QString customDateString = "01/01/2023 12:00:00";
    QString result3 = datetimeUtils->TranslateDateFormat(customDateString, "MM/dd/yyyy hh:mm:ss");
    EXPECT_FALSE(result3.isEmpty());
}

// 测试getOldDateFormat方法
TEST_F(DateTimeUtilsTest, GetOldDateFormatTest) {
    QString oldFormat = datetimeUtils->getOldDateFormat();
    EXPECT_FALSE(oldFormat.isEmpty());
    // 默认应该包含日期和时间格式
    EXPECT_TRUE(oldFormat.contains("yyyy-MM-dd"));
    EXPECT_TRUE(oldFormat.contains("hh:mm:ss") || oldFormat.contains("ap hh:mm:ss"));
}

// 测试DateFormatChange槽函数
TEST_F(DateTimeUtilsTest, DateFormatChangeTest) {
    // 调用槽函数
    datetimeUtils->DateFormatChange("test-format");
    
    // 验证函数能正常调用不崩溃
    SUCCEED();
}

// 测试TimeFormatChange槽函数
TEST_F(DateTimeUtilsTest, TimeFormatChangeTest) {
    // 测试12小时制
    datetimeUtils->TimeFormatChange("12");
    
    // 测试24小时制
    datetimeUtils->TimeFormatChange("24");
    
    // 验证函数能正常调用不崩溃
    SUCCEED();
}

// 测试QDateTimeToCtm私有方法（通过公共方法间接测试）
TEST_F(DateTimeUtilsTest, QDateTimeToCtmTest) {
    QDateTime datetime(QDate(2023, 1, 1), QTime(12, 30, 45));
    QString result = datetimeUtils->TranslateDateFormat(datetime);
    EXPECT_FALSE(result.isEmpty());
    // 验证年份信息是否正确转换
    EXPECT_TRUE(result.contains("2023") || result.contains("23"));
}