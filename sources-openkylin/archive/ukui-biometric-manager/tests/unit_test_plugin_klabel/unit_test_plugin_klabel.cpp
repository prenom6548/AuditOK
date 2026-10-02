/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "../../biometric-plugin/kalabel.h"
#include "../../biometric-plugin/namelabel.h"
#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QFont>
#include <QApplication>


class KALabelTest : public ::testing::Test {
protected:
    KALabel* label;
    NameLabel* nameLabel;

    void SetUp() override {
        label = new KALabel();
        nameLabel = new NameLabel();
    }


    void TearDown() override {
        delete label;
        delete nameLabel;
    }
};


// 测试构造函数
TEST_F(KALabelTest, ConstructorTest) {
    // 测试默认构造函数
    KALabel* defaultLabel = new KALabel();
    EXPECT_EQ(defaultLabel->m_strText, "");


    // 测试带参数的构造函数
    KALabel* textLabel = new KALabel("Test Text");
    EXPECT_EQ(textLabel->m_strText, "Test Text");
    delete defaultLabel;
    delete textLabel;
}


// 测试 setText 函数
TEST_F(KALabelTest, SetTextTest) {
    QString testText = "Hello, World!";
    label->setText(testText);
    EXPECT_EQ(label->m_strText, testText);
    EXPECT_EQ(label->text(), testText);
}


// 测试 paintEvent 函数
TEST_F(KALabelTest, PaintEventTest) {
    // 模拟需要截断文本的情况
    label->resize(50, 20);
    label->setText("This is a very long text that needs to be truncated");
    QPaintEvent paintEvent(label->rect());


    // 模拟 paintEvent 调用
    label->paintEvent(&paintEvent);


    QString expectedText = label->getElidedText(label->font(), label->width(), label->m_strText);
    EXPECT_EQ(label->text(), expectedText);
    EXPECT_EQ(label->toolTip(), "This is a very long text that needs to be truncated");


    // 模拟不需要截断文本的情况
    label->resize(200, 20);
    label->setText("Short Text");
    label->paintEvent(&paintEvent);


    EXPECT_EQ(label->text(), "Short Text");
    EXPECT_EQ(label->toolTip(), "");
}


// 测试 getElidedText 函数
TEST_F(KALabelTest, GetElidedTextTest) {
    QFont font;
    int width = 100;


    // 测试需要截断的情况
    QString longText = "This is a very long text that needs to be truncated";
    QString elidedText = label->getElidedText(font, width, longText);
    QFontMetrics fontMetrics(font);
    EXPECT_EQ(elidedText, fontMetrics.elidedText(longText, Qt::ElideRight, width));


    // 测试不需要截断的情况
    QString shortText = "Short Text";
    elidedText = label->getElidedText(font, width, shortText);
    EXPECT_EQ(elidedText, shortText);
}
