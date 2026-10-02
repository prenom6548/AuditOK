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
#include <gtest/gtest-death-test.h>

#include "../../bioauth/include/giodbus.h"

#include "stubext.h"
using namespace stub_ext;

class FreedesktopHelperTest : public testing::Test
{
protected:

};

TEST_F(FreedesktopHelperTest, get_server_gvariant_stdout)
{
    int value = get_server_gvariant_stdout(-1); 
    ASSERT_EQ(value, -1);
}
