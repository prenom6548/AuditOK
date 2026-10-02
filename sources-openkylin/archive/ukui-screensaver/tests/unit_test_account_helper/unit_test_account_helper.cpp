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

#include "../../src/dbusifs/accountshelper.h"

#include "stubext.h"

using namespace stub_ext;

class AccountsHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAccountsHelperDbus = new AccountsHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pAccountsHelperDbus;
        m_pAccountsHelperDbus = nullptr;
    }

    static AccountsHelper *m_pAccountsHelperDbus;
};

AccountsHelper *AccountsHelperTest::m_pAccountsHelperDbus = nullptr;

TEST_F(AccountsHelperTest, UserLanguageByName)
{
    QString value = m_pAccountsHelperDbus->getUserLanguageByName(getenv("USER"));
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_STREQ(cstr, "zh_CN");
}

TEST_F(AccountsHelperTest, UserBackgroundByName)
{
    QString value = m_pAccountsHelperDbus->getUserBackgroundByName(getenv("USER"));
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_STREQ(cstr, "zh_CN");
}

TEST_F(AccountsHelperTest, UserSessionByName)
{
    QString value = m_pAccountsHelperDbus->getUserSessionByName(getenv("USER"));
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_STREQ(cstr, "zh_CN");
}

TEST_F(AccountsHelperTest, AccountBackground)
{
    QString value = m_pAccountsHelperDbus->getAccountBackground(1000);
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_STREQ(cstr, "zh_CN");
}

TEST_F(AccountsHelperTest, UserList)
{
    QStringList value = m_pAccountsHelperDbus->getUserList();
    ASSERT_EQ(value.isEmpty(), false);
}
