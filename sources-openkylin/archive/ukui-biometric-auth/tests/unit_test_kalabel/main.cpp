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
#include "../../polkit-agent/src/kalabel.h"
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
 
// 辅助函数，用于创建QFontMetrics对象，避免在测试中重复代码
QFontMetrics createFontMetrics(const QFont& font) {
    return QFontMetrics(font);
}
 
// 测试getElidedText方法是否正确地省略了文本
TEST(KALabelTest, GetElidedText_ShouldElideTextIfWidthIsExceeded) {
    // 创建一个QFont对象，并设置其属性（这里使用默认属性）
    QFont font;
    QFontMetrics fontMetrics = createFontMetrics(font);
 
    // 设置测试字符串和宽度
    QString testString = "This is a very long text that needs to be elided.";
    int width = fontMetrics.width("This is a short text."); // 假设这个宽度是合适的测试宽度
 
    // 创建KALabel对象（仅为了访问其getElidedText方法）
    KALabel kalabel;
 
    // 调用getElidedText方法
    QString elidedText = kalabel.getElidedText(font, width, testString);
 
    // 检查文本是否被省略
    EXPECT_TRUE(elidedText.endsWith("..."));
    EXPECT_LT(fontMetrics.width(elidedText), width);
}
 
// 测试getElidedText方法在没有超过宽度时是否返回原始文本
TEST(KALabelTest, GetElidedText_ShouldNotElideTextIfWidthIsNotExceeded) {
    // 创建一个QFont对象，并设置其属性（这里使用默认属性）
    QFont font;
    QFontMetrics fontMetrics = createFontMetrics(font);
 
    // 设置测试字符串和宽度（确保宽度足够大以容纳整个字符串）
    QString testString = "Short text";
    int width = fontMetrics.width(testString) + 10; // 加一些额外的宽度以确保不会省略
 
    // 创建KALabel对象（仅为了访问其getElidedText方法）
    KALabel kalabel;
 
    // 调用getElidedText方法
    QString elidedText = kalabel.getElidedText(font, width, testString);
 
    // 检查文本是否未被省略
    EXPECT_EQ(elidedText, testString);
}
 
TEST(KALabelTest, ShowTest) {
    KALabel kalabel;
    KALabel kalabel1("test");
    kalabel1.setText("test1");
    kalabel1.setFixedWidth(80);
    kalabel1.paintEvent(nullptr);
    kalabel1.setText("long long long long long test text");
    kalabel1.paintEvent(nullptr);
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
