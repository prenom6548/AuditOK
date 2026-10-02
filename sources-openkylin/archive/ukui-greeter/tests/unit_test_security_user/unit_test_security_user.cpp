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

#include "../../ukui-greeter/greeter/securityuser.h"

#include "stubext.h"
using namespace stub_ext;

class SecurityUserTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pSecurityUserDbus = new SecurityUser();
    }

    static void TearDownTestSuite()
    {
        delete m_pSecurityUserDbus;
        m_pSecurityUserDbus = nullptr;
    }

    static SecurityUser *m_pSecurityUserDbus;
};

SecurityUser *SecurityUserTest::m_pSecurityUserDbus = nullptr;

TEST_F(SecurityUserTest, isSecrityUser)
{
    bool value = m_pSecurityUserDbus->isSecrityUser(getenv("USER"));
    ASSERT_EQ(value, true);
}

TEST_F(SecurityUserTest, getMode)
{
    int mode = m_pSecurityUserDbus->getMode();
    ASSERT_EQ(mode, 0);
}

TEST_F(SecurityUserTest, getBlackList)
{
    QStringList blackList = m_pSecurityUserDbus->getBlackList();
    ASSERT_EQ(blackList.isEmpty(), true);
}

TEST_F(SecurityUserTest, getWhiteList)
{
    QStringList whiteList = m_pSecurityUserDbus->getWhiteList();
    ASSERT_EQ(whiteList.isEmpty(), true);
}
