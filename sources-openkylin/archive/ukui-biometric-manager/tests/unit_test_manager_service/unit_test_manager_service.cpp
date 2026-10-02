/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "../../biometric-manager/src/servicemanager.h"

#include "stubext.h"
using namespace stub_ext;

class ServiceManagerTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pServiceManagerDbus = new ServiceManager();
    }

    static void TearDownTestSuite()
    {
        delete m_pServiceManagerDbus;
        m_pServiceManagerDbus = nullptr;
    }

    static ServiceManager *m_pServiceManagerDbus;
};

ServiceManager *ServiceManagerTest::m_pServiceManagerDbus = nullptr;

TEST_F(ServiceManagerTest, connectToService)
{
    bool connectToService = m_pServiceManagerDbus->connectToService();
    ASSERT_EQ(connectToService, true);
}

TEST_F(ServiceManagerTest, serviceExists)
{
    bool serviceExists = m_pServiceManagerDbus->serviceExists();
    ASSERT_EQ(serviceExists, true);
}

TEST_F(ServiceManagerTest, apiCompatible)
{
    bool apiCompatible = m_pServiceManagerDbus->apiCompatible();
    ASSERT_EQ(apiCompatible, true);
}
