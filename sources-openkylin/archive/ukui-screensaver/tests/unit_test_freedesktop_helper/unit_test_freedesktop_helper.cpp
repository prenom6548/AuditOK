/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#include <gtest/gtest-death-test.h>

#include "../../src/dbusifs/freedesktophelper.h"

#include "stubext.h"
using namespace stub_ext;

class FreedesktopHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pFreedesktopHelperDbus = new FreedesktopHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pFreedesktopHelperDbus;
        m_pFreedesktopHelperDbus = nullptr;
    }

    static FreedesktopHelper *m_pFreedesktopHelperDbus;
};

FreedesktopHelper *FreedesktopHelperTest::m_pFreedesktopHelperDbus = nullptr;

TEST_F(FreedesktopHelperTest, NameHasOwner)
{
    bool value = m_pFreedesktopHelperDbus->NameHasOwner("org.ukui.Biometric");
    ASSERT_EQ(value, false);
}

TEST_F(FreedesktopHelperTest, isServiceActivable)
{
    bool value = m_pFreedesktopHelperDbus->isServiceActivable("org.ukui.UniauthBackend");
    ASSERT_EQ(value, false);
}
