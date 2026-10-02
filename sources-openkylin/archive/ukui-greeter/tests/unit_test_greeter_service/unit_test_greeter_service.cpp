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

#include "../../ukui-greeter/greeter/greeterservice.h"

#include "stubext.h"
using namespace stub_ext;

class ServiceManagerTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pServiceManagerDbus = GreeterService::instance();
    }

    static void TearDownTestSuite()
    {
        delete m_pServiceManagerDbus;
        m_pServiceManagerDbus = nullptr;
    }

    static GreeterService *m_pServiceManagerDbus;
};

GreeterService *ServiceManagerTest::m_pServiceManagerDbus = nullptr;

TEST_F(ServiceManagerTest, SwitchToUser)
{
    m_pServiceManagerDbus->SwitchToUser(getenv("USER"));
}

TEST_F(ServiceManagerTest, SwitchToGreeterUser)
{
    m_pServiceManagerDbus->SwitchToGreeterUser(getenv("USER"));
}

TEST_F(ServiceManagerTest, getWillSwitchUser)
{
    m_pServiceManagerDbus->getWillSwitchUser();
}

TEST_F(ServiceManagerTest, isUserNameValid)
{
    bool value = m_pServiceManagerDbus->isUserNameValid(getenv("USER"));
    EXPECT_EQ(value, false);
}
