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
#include "../../biometric-manager/src/KAButton.h"
#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QFont>
#include <QApplication>


class KAButtonTest : public ::testing::Test {
protected:
    KAButton* button;


    void SetUp() override {

        button = new KAButton();
    }


    void TearDown() override {
        delete button;
    }
};


// 测试构造函数
TEST_F(KAButtonTest, ConstructorTest) {
    // 测试默认构造函数
    KAButton* defaultButton = new KAButton();
    EXPECT_EQ(defaultButton->m_strText, "");


    // 测试带参数的构造函数
    KAButton* textButton = new KAButton("Test Text");
    EXPECT_EQ(textButton->m_strText, "Test Text");
    delete defaultButton;
    delete textButton;
}


// 测试 setText 函数
TEST_F(KAButtonTest, SetTextTest) {
    QString testText = "Hello, World!";
    button->setText(testText);
    EXPECT_EQ(button->m_strText, testText);
    EXPECT_EQ(button->text(), testText);
}


// 测试 paintEvent 函数
TEST_F(KAButtonTest, PaintEventTest) {
    // 模拟需要截断文本的情况
    button->resize(100, 30);
    button->setText("This is a very long text that needs to be truncated");
    QPaintEvent paintEvent(button->rect());


    // 调用 paintEvent 进行测试
    button->paintEvent(&paintEvent);


    QString expectedText = button->getElidedText(button->font(), button->width() - 28, button->m_strText);
    EXPECT_EQ(button->text(), expectedText);
    EXPECT_EQ(button->toolTip(), "This is a very long text that needs to be truncated");


    // 模拟不需要截断文本的情况
    button->resize(200, 30);
    button->setText("Short Text");
    button->paintEvent(&paintEvent);


    EXPECT_EQ(button->text(), "Short Text");
    EXPECT_EQ(button->toolTip(), "");
}


// 测试 getElidedText 函数
TEST_F(KAButtonTest, GetElidedTextTest) {
    QFont font;
    int width = 100;


    // 测试需要截断的情况
    QString longText = "This is a very long text that needs to be truncated";
    QString elidedText = button->getElidedText(font, width, longText);
    QFontMetrics fontMetrics(font);
    EXPECT_EQ(elidedText, fontMetrics.elidedText(longText, Qt::ElideRight, width));


    // 测试不需要截断的情况
    QString shortText = "Short Text";
    elidedText = button->getElidedText(font, width, shortText);
    EXPECT_EQ(elidedText, shortText);
}

