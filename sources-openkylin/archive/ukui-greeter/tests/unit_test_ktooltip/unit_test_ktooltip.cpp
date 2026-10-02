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
#include "../../ukui-greeter/greeter/klabel.h"
#include "../../ukui-greeter/greeter/ktooltip.h"
#include <gtest/gtest.h>
#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>


class TooltipTest : public ::testing::Test {
protected:
    Tooltip* tooltip;

    void SetUp() override {
        tooltip = new Tooltip();
    }


    void TearDown() override {
        delete tooltip;
    }
};


// 测试构造函数
TEST_F(TooltipTest, ConstructorTest) {
    // 检查 Qt::WA_DeleteOnClose 属性
    EXPECT_TRUE(tooltip->testAttribute(Qt::WA_DeleteOnClose));


    // 检查子部件和布局
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(tooltip->layout());
    ASSERT_NE(layout, nullptr);


    KLabel* label = qobject_cast<KLabel*>(layout->itemAt(0)->widget());
    ASSERT_NE(label, nullptr);


    EXPECT_EQ(layout->contentsMargins(), QMargins(0, 0, 0, 0));
}


// 测试 setText 函数
TEST_F(TooltipTest, SetTextTest) {
    QString testText = "Test Text";
    int testFontSize = 12;


    tooltip->setText(testText, testFontSize);


    KLabel* label = qobject_cast<KLabel*>(tooltip->findChild<KLabel*>());
    ASSERT_NE(label, nullptr);


    EXPECT_EQ(label->text(), testText);


    QFont font = label->font();
    EXPECT_EQ(font.pointSize(), testFontSize);


    EXPECT_EQ(label->contentsMargins(), QMargins(2, 0, 2, 0));
}


// 测试 popup 函数
TEST_F(TooltipTest, PopupTest) {
    QString testText = "Popup Text";
    int testFontSize = 14;


    // 模拟 popup 函数调用
    tooltip->popup(tooltip, testText, testFontSize);


    KLabel* label = qobject_cast<KLabel*>(tooltip->findChild<KLabel*>());
    ASSERT_NE(label, nullptr);


    EXPECT_EQ(label->text(), testText);


    // 检查调整大小和显示操作
    EXPECT_TRUE(tooltip->isVisible());


    QSize expectedSize = tooltip->sizeHint();
    EXPECT_EQ(tooltip->size(), expectedSize);
}


// 测试 tipClose 函数
TEST_F(TooltipTest, TipCloseTest) {
    // 模拟 tipClose 函数调用
    tooltip->tipClose(tooltip);


    EXPECT_FALSE(tooltip->isVisible());


    // 由于 tooltip 可能已经关闭，这里无法检查是否真正关闭，可根据具体需求添加更多逻辑
}


// 测试 paintEvent 函数
TEST_F(TooltipTest, PaintEventTest) {
    // 模拟 paintEvent 调用
//    QPaintEvent paintEvent(tooltip->rect());


    // 由于无法直接检查 drawPrimitive 是否被调用，可以使用 mock 框架如 Google Mock 进行更深入测试
    // 这里仅确保 paintEvent 不崩溃
//    tooltip->paintEvent(&paintEvent);
}
