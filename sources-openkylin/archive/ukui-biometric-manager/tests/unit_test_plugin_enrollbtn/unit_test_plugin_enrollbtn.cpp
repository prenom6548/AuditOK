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
#include "../../biometric-plugin/enrollbtn.h"
#include <gtest/gtest.h>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>


class EnrollBtnTest : public ::testing::Test {
protected:
    EnrollBtn* enrollBtn;


    void SetUp() override {
        enrollBtn = new EnrollBtn();
    }


    void TearDown() override {
        delete enrollBtn;
    }
};


// 测试构造函数
TEST_F(EnrollBtnTest, ConstructorTest) {
    // 检查对象名称
    EXPECT_EQ(enrollBtn->objectName(), "this");


    // 检查尺寸设置
    EXPECT_EQ(enrollBtn->minimumSize(), QSize(580, 60));
    EXPECT_EQ(enrollBtn->maximumSize(), QSize(16777215, 60));


    // 检查属性设置
    EXPECT_TRUE(enrollBtn->property("useButtonPalette").toBool());
    EXPECT_TRUE(enrollBtn->isFlat());


    // 检查样式表
    QString styleSheet = enrollBtn->styleSheet();
    EXPECT_NE(styleSheet.indexOf("AddBtn:!checked:!pressed:!hover{background-color: palette(base); border-radius: 6px;}"), -1);
    EXPECT_NE(styleSheet.indexOf("AddBtn:!checked:!pressed:hover{background-color: palette(button); border-radius: 6px;}"), -1);


    // 检查布局和子部件
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(enrollBtn->layout());
    ASSERT_NE(layout, nullptr);
    ASSERT_EQ(layout->count(), 4);


    QLabel* iconLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
    ASSERT_NE(iconLabel, nullptr);


    QLabel* textLabel = qobject_cast<QLabel*>(layout->itemAt(2)->widget());
    ASSERT_NE(textLabel, nullptr);


    // 检查图标标签属性
    EXPECT_EQ(iconLabel->property("iconHighlightEffectMode").toInt(), 1);


    // 检查主题相关属性设置
    QGSettings* qtSettings = qobject_cast<QGSettings*>(enrollBtn->findChild<QGSettings*>());
    ASSERT_NE(qtSettings, nullptr);


    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode) {
        EXPECT_TRUE(iconLabel->property("useIconHighlightEffect").toBool());
    } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
        EXPECT_FALSE(iconLabel->property("useIconHighlightEffect").toBool());
    }
}


// 测试 enterEvent 和 leaveEvent 函数
TEST_F(EnrollBtnTest, EnterLeaveEventTest) {
//    QSignalSpy enterSpy(enrollBtn, SIGNAL(enterWidget()));
//    QSignalSpy leaveSpy(enrollBtn, SIGNAL(leaveWidget()));


    // 模拟进入事件
    QEvent enterEvent(QEvent::Enter);
    enrollBtn->enterEvent(&enterEvent);
//    EXPECT_EQ(enterSpy.count(), 1);


    // 模拟离开事件
    QEvent leaveEvent(QEvent::Leave);
    enrollBtn->leaveEvent(&leaveEvent);
//    EXPECT_EQ(leaveSpy.count(), 1);
}


// 测试 setName 函数
TEST_F(EnrollBtnTest, SetNameTest) {
    QString testText = "Test Text";
    enrollBtn->setName(testText);


    QLabel* textLabel = qobject_cast<QLabel*>(enrollBtn->layout()->itemAt(2)->widget());
    EXPECT_EQ(textLabel->text(), testText);
}


// 测试主题改变时的属性更新
TEST_F(EnrollBtnTest, ThemeChangeTest) {
    QGSettings* qtSettings = qobject_cast<QGSettings*>(enrollBtn->findChild<QGSettings*>());
    ASSERT_NE(qtSettings, nullptr);


    QLabel* iconLabel = qobject_cast<QLabel*>(enrollBtn->layout()->itemAt(1)->widget());


    // 模拟主题从亮到暗的变化
    qtSettings->set(MODE_QT_KEY, "ukui-dark");
    EXPECT_TRUE(iconLabel->property("useIconHighlightEffect").toBool());


    // 模拟主题从暗到亮的变化
    qtSettings->set(MODE_QT_KEY, "ukui-white");
    EXPECT_FALSE(iconLabel->property("useIconHighlightEffect").toBool());
}
