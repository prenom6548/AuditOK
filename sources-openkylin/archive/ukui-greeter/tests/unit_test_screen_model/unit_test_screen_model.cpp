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
#include "../../ukui-greeter/greeter/screenmodel.h"
#include <gtest/gtest.h>
#include <QDesktopWidget>
#include <QApplication>


class ScreenModelTest : public ::testing::Test {
protected:
    ScreenModel* screenModel;


    void SetUp() override {
        screenModel = new ScreenModel();
    }


    void TearDown() override {
        delete screenModel;
    }
};


// 测试构造函数
TEST_F(ScreenModelTest, ConstructorTest) {
    // 重新创建，确保信号连接正常
    screenModel = new ScreenModel();



    // 模拟信号发射
    Q_EMIT screenModel->_desktop->workAreaResized(0);
    Q_EMIT screenModel->_desktop->resized(0);
    Q_EMIT screenModel->_desktop->screenCountChanged(1);
}


// 测试 rowCount 函数
TEST_F(ScreenModelTest, RowCountTest) {
    // 正常情况
    QModelIndex parent;
    int rowCount = screenModel->rowCount(parent);
    EXPECT_EQ(rowCount, screenModel->m_screen.size());
}


// 测试 data 函数
TEST_F(ScreenModelTest, DataTest) {
    QModelIndex validIndex;
    QModelIndex invalidIndex;


    // 有效索引且 role 为 Qt::UserRole 的情况
    QVariant validData = screenModel->data(validIndex, Qt::UserRole);
    if (!screenModel->m_screen.isEmpty()) {
        EXPECT_EQ(validData, screenModel->m_screen[0]);
    } else {
        EXPECT_EQ(validData, QVariant());
    }


    // 无效索引的情况
    QVariant invalidData = screenModel->data(invalidIndex, Qt::UserRole);
    EXPECT_EQ(invalidData, QVariant());


    // 有效索引但 role 不为 Qt::UserRole 的情况
    validData = screenModel->data(validIndex, Qt::DisplayRole);
    EXPECT_EQ(validData, QVariant());
}


// 测试 onScreenResized 函数
TEST_F(ScreenModelTest, OnScreenResizedTest) {
    // 正常情况
    int screen = 0;
    screenModel->onScreenResized(screen);

    // 超出范围的情况
    screen = 999;
    screenModel->onScreenResized(screen);


//    // 确保没有额外的数据改变信号
//    EXPECT_EQ(dataChangedSpy.count(), 1);
}


// 测试 onScreenCountChanged 函数
TEST_F(ScreenModelTest, OnScreenCountChangedTest) {
    int newCount = 5;
    screenModel->onScreenCountChanged(newCount);


//    EXPECT_EQ(beginResetModelSpy.count(), 1);
//    EXPECT_EQ(endResetModelSpy.count(), 1);
}


// 测试 loadScreens 函数
TEST_F(ScreenModelTest, LoadScreensTest) {
    screenModel->loadScreens();



    // 检查 m_screen 内容
    int screenCount = screenModel->_desktop->screenCount();
    EXPECT_EQ(screenModel->m_screen.size(), screenCount);
}

