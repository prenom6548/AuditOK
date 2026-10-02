/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDebug>

#include "../../plugins/component/InfoButton/infobutton.h"

#include "stubext.h"
using namespace stub_ext;

class InfoButtonTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pInfoButtonTest = new InfoButton();
    }

    static void TearDownTestSuite()
    {
        if (m_pInfoButtonTest) {
            delete m_pInfoButtonTest;
            m_pInfoButtonTest = nullptr;
        }
    }
    static InfoButton *m_pInfoButtonTest;
};

InfoButton *InfoButtonTest::m_pInfoButtonTest = nullptr;

TEST_F(InfoButtonTest, testAllFunc)
{
    //    QPaintEvent *p_paintEvent;
    //    m_pInfoButtonTest->paintEvent(p_paintEvent);
    //    QEvent *p_event;
    //    m_pInfoButtonTest->enterEvent(p_event);
    //    m_pInfoButtonTest->leaveEvent(p_event);
    //    QMouseEvent *p_mouseEvent;
    //    m_pInfoButtonTest->mousePressEvent(p_mouseEvent);
    //    m_pInfoButtonTest->mouseReleaseEvent(p_mouseEvent);

    m_pInfoButtonTest->initUI();
    //    m_pInfoButtonTest->onGSettingChaned("key");
}
