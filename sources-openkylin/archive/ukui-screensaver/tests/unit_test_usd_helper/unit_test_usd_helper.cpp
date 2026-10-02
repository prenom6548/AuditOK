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

#include "../../src/dbusifs/usdhelper.h"

#include "stubext.h"
using namespace stub_ext;

class UsdHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pUsdHelperDbus = new UsdHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pUsdHelperDbus;
        m_pUsdHelperDbus = nullptr;
    }

    static UsdHelper *m_pUsdHelperDbus;
};

UsdHelper *UsdHelperTest::m_pUsdHelperDbus = nullptr;

TEST_F(UsdHelperTest, usdExternalDoAction)
{
    bool value = m_pUsdHelperDbus->usdExternalDoAction(-1);
    ASSERT_EQ(value, false);
}
