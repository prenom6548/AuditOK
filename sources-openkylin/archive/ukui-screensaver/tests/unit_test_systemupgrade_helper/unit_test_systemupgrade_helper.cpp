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

#include "../../src/dbusifs/systemupgradehelper.h"

#include "stubext.h"

using namespace stub_ext;

class SystemUpgradeHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pSystemUpgradeHelperDbus = new SystemUpgradeHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pSystemUpgradeHelperDbus;
        m_pSystemUpgradeHelperDbus = nullptr;
    }

    static SystemUpgradeHelper *m_pSystemUpgradeHelperDbus;
};

SystemUpgradeHelper *SystemUpgradeHelperTest::m_pSystemUpgradeHelperDbus = nullptr;

TEST_F(SystemUpgradeHelperTest, checkSystemUpgrade)
{
    bool value = m_pSystemUpgradeHelperDbus->checkSystemUpgrade();
    ASSERT_EQ(value, false);
}

TEST_F(SystemUpgradeHelperTest, doUpgradeThenRboot)
{
    bool value = m_pSystemUpgradeHelperDbus->doUpgradeThenRboot();
    ASSERT_EQ(value, false);
}

TEST_F(SystemUpgradeHelperTest, doUpgradeThenShutdown)
{
    bool value = m_pSystemUpgradeHelperDbus->doUpgradeThenShutdown();
    ASSERT_EQ(value, false);
}
