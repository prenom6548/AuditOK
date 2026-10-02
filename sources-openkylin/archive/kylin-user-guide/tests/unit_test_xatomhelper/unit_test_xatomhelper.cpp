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

#include "../../src/xatom-helper.h"

#include <QX11Info>

#include <X11/Xlib.h>

class XAtomHelperTest : public ::testing::Test {

};

TEST_F(XAtomHelperTest, TestGetInstance) {

    XAtomHelper::getInstance(); 
}

TEST_F(XAtomHelperTest, TestIsFrameLessWindow) {

    XAtomHelper::getInstance()->isFrameLessWindow(0);
}

TEST_F(XAtomHelperTest, TestIsWindowDecorateBorderOnly) {

    XAtomHelper::getInstance()->isWindowDecorateBorderOnly(0);
}

// 准备70%
TEST_F(XAtomHelperTest, TestIsUKUICsdSupported) {

    XAtomHelper::getInstance()->isUKUICsdSupported();
}

TEST_F(XAtomHelperTest, TestGetWindowBorderRadius) {

    XAtomHelper::getInstance()->getWindowBorderRadius(0);
}

TEST_F(XAtomHelperTest, TestGetWindowMotifHint) {

    XAtomHelper::getInstance()->getWindowMotifHint(0);
}

TEST_F(XAtomHelperTest, TestSetWindowBorderRadius) {

    XAtomHelper::getInstance()->setWindowBorderRadius(0,6,6,6,6);
}

TEST_F(XAtomHelperTest, TestRegisterUKUICsdNetWmSupportAtom) {

    XAtomHelper::getInstance()->registerUKUICsdNetWmSupportAtom();
}
