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
