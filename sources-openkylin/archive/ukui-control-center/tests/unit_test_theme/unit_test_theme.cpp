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
#include <QGSettings>

#include "../../registeredSession/plugins/theme/themeinterface.h"

class ThemeInterfaceTest:public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pThemeDbus = new ThemeInterface();
    }

    static void TearDownTestSuite()
    {
        delete m_pThemeDbus;
        m_pThemeDbus = nullptr;
    }

    static ThemeInterface *m_pThemeDbus;
};

ThemeInterface *ThemeInterfaceTest::m_pThemeDbus =  nullptr;

TEST_F(ThemeInterfaceTest, initWindowRadius)
{
    bool retValue = m_pThemeDbus->initWindowRadius();
    ASSERT_EQ(retValue, true);
}

TEST_F(ThemeInterfaceTest, isHardwareRendering)
{
    bool retValue = m_pThemeDbus->isHardwareRendering();
    ASSERT_EQ(retValue, true);
}

TEST_F(ThemeInterfaceTest, setEffectStatePersists)
{
    QGSettings gset("org.ukui.control-center.personalise");
    bool oldValue = gset.get("effect").toBool();
    bool newValue = !oldValue;

    ASSERT_TRUE(m_pThemeDbus->setEffectState(newValue));
    ASSERT_EQ(gset.get("effect").toBool(), newValue);

    ASSERT_TRUE(m_pThemeDbus->setEffectState(oldValue));
    ASSERT_EQ(gset.get("effect").toBool(), oldValue);
}
