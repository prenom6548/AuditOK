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
#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "guide-widget.h" 

class GuideWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试之前运行
        widget = new GuideWidget();
    }

    void TearDown() override {
        // 在每个测试之后运行
        delete widget;
    }

    GuideWidget* widget;
};

TEST_F(GuideWidgetTest, TestInitialState) {
    // 测试初始状态
    EXPECT_TRUE(widget->isVisible());
    EXPECT_EQ(widget->getTitle(), "Default Title");
}

TEST_F(GuideWidgetTest, TestSetTitle) {
    // 测试设置标题
    QString newTitle = "New Title";
    widget->setTitle(newTitle);
    EXPECT_EQ(widget->getTitle(), newTitle);
}

TEST_F(GuideWidgetTest, TestShowAndHide) {
    // 测试显示和隐藏功能
    widget->hide();
    EXPECT_FALSE(widget->isVisible());

    widget->show();
    EXPECT_TRUE(widget->isVisible());
}

TEST_F(GuideWidgetTest, TestResize) {
    // 测试调整大小功能
    QSize newSize(200, 100);
    widget->resize(newSize);
    EXPECT_EQ(widget->size(), newSize);
}

