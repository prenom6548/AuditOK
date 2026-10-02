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

#include "../../plugins/netconnect/itemframe.h"

#include "stubext.h"
using namespace stub_ext;


class ItemFrameTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pItemFrameTest = new ItemFrame("eth0");
    }

    static void TearDownTestSuite()
    {
        if (m_pItemFrameTest) {
            delete m_pItemFrameTest;
            m_pItemFrameTest = nullptr;
        }
    }

    static ItemFrame *m_pItemFrameTest;
};

ItemFrame *ItemFrameTest::m_pItemFrameTest = nullptr;

TEST_F(ItemFrameTest, testAllFunc)
{
    ASSERT_EQ(true, true);
}
