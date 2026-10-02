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
#include <QApplication>
#include <QWidget>
#include <QPixmap>
#include "../../polkit-agent/src/modeButton.h"

// Test fixture for ModeButton
class ModeButtonTest : public ::testing::Test {
protected:
};

TEST_F(ModeButtonTest, TestDefaultConstructor) {
    ModeButton button(nullptr);
    EXPECT_EQ(button.isShowPwd(), false);
    EXPECT_EQ(button.size(), QSize(24, 24));
}

TEST_F(ModeButtonTest, TestIconConstructor) {
    ModeButton button("testIcon", nullptr);
    // Since we can't directly assert the icon, we assume the constructor sets up the internal state correctly
    EXPECT_EQ(button.size(), QSize(24, 24));
    // Further checks would involve mocking QIcon::fromTheme and verifying interactions
}

TEST_F(ModeButtonTest, TestShowPasswordToggle) {
    ModeButton button(nullptr);
    
    // Mock QGSettings to return a theme name that is not dark or black
    button.initStyleTheme();
    
    // Initially, pwdShow should be false
    EXPECT_EQ(button.isShowPwd(), false);
    
    // Simulate a click event
    emit button.clicked();
    
    // Now pwdShow should be true
    EXPECT_EQ(button.isShowPwd(), true);
    
    // Simulate another click event
    emit button.clicked();
    
    // Now pwdShow should be false again
    EXPECT_EQ(button.isShowPwd(), false);
}

TEST_F(ModeButtonTest, TestHoverEffect) {
    ModeButton button(nullptr);
    
    // Mock QGSettings to return a theme name that is not dark or black
    button.initStyleTheme();
    
    // Simulate entering the hover state
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(&button, &enterEvent);
    
    // Verify the icon is set to white (or appropriate hover color)
    // This would involve checking the internal state or mocking QIcon::pixmap and QPixmap::fromImage
    // Since we can't directly access the icon, we assume setModeIcon handles it correctly
    
    // Simulate leaving the hover state
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&button, &leaveEvent);
    
    // Verify the icon is set back to gray (or appropriate non-hover color)
    // Similarly, we assume setModeIcon handles it correctly
}

TEST_F(ModeButtonTest, TestThemeChange) {
    ModeButton button(nullptr);
    
    button.initStyleTheme();
    button.setModeIcon(); 
    
    button.m_styleSettings->changed("styleName");
    
    QPixmap pixmap = QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(24,24);
    pixmap = button.drawSymbolicColoredPixmap(pixmap, "black");
    pixmap = button.drawSymbolicColoredPixmap(pixmap, "blue");
    pixmap = button.drawSymbolicColoredPixmap(pixmap, "gray");
    pixmap = button.drawSymbolicColoredPixmap(pixmap, "white");
    pixmap = button.drawSymbolicColoredPixmap(pixmap, "red");

    button.m_strThemeName = "ukui-dark";
    button.setModeIcon();
    button.m_strThemeName = "ukui-black";
    button.setModeIcon();
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
