/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test_globaldateformat.h"
#include <QDateTime>

void TestGlobalDateFormat::SetUp()
{
    m_dateFormat = GlobalDateFormat::getInstance();
}

void TestGlobalDateFormat::TearDown()
{
    // Singleton, no need to delete
}

TEST_F(TestGlobalDateFormat, GetInstance)
{
    ASSERT_NE(m_dateFormat, nullptr);
    EXPECT_EQ(m_dateFormat, GlobalDateFormat::getInstance());
}

TEST_F(TestGlobalDateFormat, TranslateDateAndTimeFormat)
{
    // Test a valid translation
    QString input = "2023-06-26 10:30:30";
    QString fromFormat = "yyyy-MM-dd hh:mm:ss";
    QString toFormat = "yyyy/MM/dd HH:mm";
    
    QString result = m_dateFormat->TranslateDateAndTimeFormat(input, fromFormat, toFormat);
    EXPECT_EQ(result, "2023/06/26 10:30");
}

TEST_F(TestGlobalDateFormat, TranslateDateAndTimeFormatInvalid)
{
    // Test an invalid input string
    QString input = "invalid-date";
    QString fromFormat = "yyyy-MM-dd hh:mm:ss";
    QString toFormat = "yyyy/MM/dd HH:mm";
    
    QString result = m_dateFormat->TranslateDateAndTimeFormat(input, fromFormat, toFormat);
    EXPECT_EQ(result, "invalid-date"); // Should return original on failure
}

TEST_F(TestGlobalDateFormat, QDateTimeToCtm)
{
    QDateTime dt(QDate(2023, 6, 26), QTime(10, 30, 30));
    struct tm ctm = m_dateFormat->QDateTimeToCtm(dt);
    
    EXPECT_EQ(ctm.tm_year, 2023);
    EXPECT_EQ(ctm.tm_mon, 6);
    EXPECT_EQ(ctm.tm_mday, 26);
    EXPECT_EQ(ctm.tm_hour, 10);
    EXPECT_EQ(ctm.tm_min, 30);
    EXPECT_EQ(ctm.tm_sec, 30);
}

TEST_F(TestGlobalDateFormat, CoverageAdditions)
{
    // 1. 覆盖 TranslateDateFormat (lines 69-70)
    QDateTime dt = QDateTime::currentDateTime();
    QString formattedDate = m_dateFormat->TranslateDateFormat(dt);
    EXPECT_FALSE(formattedDate.isEmpty());

    // 2. 覆盖 TranslateDateAndTimeFormat 中的调试输出 (lines 97-98, 106-107)
    // 已经在 TranslateDateAndTimeFormat 测试中覆盖，但这里显式调用以确保
    QString input = "2023-06-26 10:30:30";
    QString fromFormat = "yyyy-MM-dd HH:mm:ss";
    QString toFormat = "yyyy/MM/dd HH:mm";
    m_dateFormat->TranslateDateAndTimeFormat(input, fromFormat, toFormat);

    // 3. 覆盖 slotDateFormatChange, slotShortDateFormatChange, slotLongDataFormatChange (lines 139-152)
    QMetaObject::invokeMethod(m_dateFormat, "slotDateFormatChange", Q_ARG(QString, "test"));
    QMetaObject::invokeMethod(m_dateFormat, "slotShortDateFormatChange", Q_ARG(QString, "test"));
    QMetaObject::invokeMethod(m_dateFormat, "slotLongDataFormatChange", Q_ARG(QString, "test"));

    // 4. 覆盖 slotTimeFormatChange (lines 154-159)
    QMetaObject::invokeMethod(m_dateFormat, "slotTimeFormatChange", Q_ARG(QString, "24h"));

    // 5. 覆盖析构函数 (lines 28-32)
    // 注意：GlobalDateFormat 是单例，通常不手动删除。
    // 但为了覆盖析构函数中的 m_dateFormatSessionDbus 删除逻辑，
    // 我们可以尝试创建一个临时对象（如果构造函数允许，虽然它是 private，但测试类可能是 friend）
    // 或者通过 getInstance() 触发的 init() 已经创建了 m_dateFormatSessionDbus。
    // 由于 getInstance() 返回的是静态局部变量或静态成员，我们不能简单 delete。
    // 这里的覆盖通常通过程序退出时的自动清理完成，或者如果源码中有特殊的清理接口。
    // 如果必须手动覆盖，可以考虑在测试结束时（虽然不推荐对单例这么做）：
    // GlobalDateFormat *temp = new GlobalDateFormat(); // 如果测试类是 friend
    // delete temp;
}
