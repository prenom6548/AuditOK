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

#include "../../src/dbusifs/kglobalaccelhelper.h"

#include "stubext.h"
using namespace stub_ext;

class KglobalAccelHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pKglobalAccelHelperDbus = new KglobalAccelHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pKglobalAccelHelperDbus;
        m_pKglobalAccelHelperDbus = nullptr;
    }

    static KglobalAccelHelper *m_pKglobalAccelHelperDbus;
};

KglobalAccelHelper *KglobalAccelHelperTest::m_pKglobalAccelHelperDbus = nullptr;

TEST_F(KglobalAccelHelperTest, blockShortcut)
{
    bool oldValue = m_pKglobalAccelHelperDbus->blockShortcut(true);
    ASSERT_EQ(oldValue, true);

    bool newValue = m_pKglobalAccelHelperDbus->blockShortcut(false);
    ASSERT_EQ(newValue, true);
}

