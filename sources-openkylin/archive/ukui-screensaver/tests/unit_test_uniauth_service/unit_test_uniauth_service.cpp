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

#include "../../src/dbusifs/uniauthservice.h"

#include "stubext.h"
using namespace stub_ext;

class UniAuthServiceTest : public testing::Test
{
//protected:
//    static void SetUpTestSuite()
//    {
//        m_pUniAuthServiceDbus = new UniAuthService();
//    }

//    static void TearDownTestSuite()
//    {
//        delete m_pUniAuthServiceDbus;
//        m_pUniAuthServiceDbus = nullptr;
//    }

//    static UniAuthService *m_pUniAuthServiceDbus;
};

//UniAuthService *UniAuthServiceTest::m_pUniAuthServiceDbus = nullptr;

TEST_F(UniAuthServiceTest, DefaultDevice)
{
    if (UniAuthService::instance()) {
        QString oldValue = UniAuthService::instance()->getDefaultDevice(getenv("USER"), 0);
        UniAuthService::instance()->setDefaultDevice(0, "testValue");
        QString newValue = UniAuthService::instance()->getDefaultDevice(getenv("USER"), 0);
        std::string str = newValue.toStdString();
        const char* cstr = str.data();
        ASSERT_STREQ(cstr, "testValue");
        UniAuthService::instance()->setDefaultDevice(0, oldValue);
    }
}

TEST_F(UniAuthServiceTest, BioAuthStatus)
{
    if (UniAuthService::instance()) {
        bool oldValue = UniAuthService::instance()->getBioAuthStatus(getenv("USER"), 0);
        UniAuthService::instance()->setBioAuthStatus(0, !oldValue);
        bool newValue = UniAuthService::instance()->getBioAuthStatus(getenv("USER"), 0);
        ASSERT_EQ(newValue, !oldValue);
        UniAuthService::instance()->setBioAuthStatus(0, oldValue);
    }
}

TEST_F(UniAuthServiceTest, AllDefaultDevice)
{
    if (UniAuthService::instance()) {
        QStringList defaultDeviceList = UniAuthService::instance()->getAllDefaultDevice(getenv("USER"));
        ASSERT_EQ(defaultDeviceList.isEmpty(), false);
    }
}

TEST_F(UniAuthServiceTest, MaxFailedTimes)
{
    if (UniAuthService::instance()) {
        int maxFailedTimes = UniAuthService::instance()->getMaxFailedTimes();
        ASSERT_EQ(maxFailedTimes, 5);
    }
}

TEST_F(UniAuthServiceTest, QRCodeEnable)
{
    if (UniAuthService::instance()) {
        bool qRCodeEnable = UniAuthService::instance()->getQRCodeEnable();
        ASSERT_EQ(qRCodeEnable, true);
    }
}

TEST_F(UniAuthServiceTest, DoubleAuth)
{
    if (UniAuthService::instance()) {
        bool doubleAuth = UniAuthService::instance()->getDoubleAuth();
        ASSERT_EQ(doubleAuth, false);
    }
}

TEST_F(UniAuthServiceTest, UserBind)
{
    if (UniAuthService::instance()) {
        bool userBind = UniAuthService::instance()->getUserBind();
        ASSERT_EQ(userBind, false);
    }
}

TEST_F(UniAuthServiceTest, IsShownInControlCenter)
{
    if (UniAuthService::instance()) {
        bool isShownInControlCenter = UniAuthService::instance()->getIsShownInControlCenter();
        ASSERT_EQ(isShownInControlCenter, false);
    }
}

TEST_F(UniAuthServiceTest, HiddenSwitchButton)
{
    if (UniAuthService::instance()) {
        bool hiddenSwitchButton = UniAuthService::instance()->getHiddenSwitchButton();
        ASSERT_EQ(hiddenSwitchButton, false);
    }
}

TEST_F(UniAuthServiceTest, getFTimeoutTimes)
{
    if (UniAuthService::instance()) {
        int hiddenSwitchButton = UniAuthService::instance()->getFTimeoutTimes();
        ASSERT_EQ(hiddenSwitchButton, 5);
    }
}

TEST_F(UniAuthServiceTest, isActivatable)
{
    if (UniAuthService::instance()) {
        bool hiddenSwitchButton = UniAuthService::instance()->isActivatable();
        ASSERT_EQ(hiddenSwitchButton, true);
    }
}

TEST_F(UniAuthServiceTest, isUserNameValid)
{
    if (UniAuthService::instance()) {
        bool hiddenSwitchButton = UniAuthService::instance()->isUserNameValid(getenv("USER"));
        ASSERT_EQ(hiddenSwitchButton, true);
    }
}

TEST_F(UniAuthServiceTest, GetLastLoginUser)
{
    if (UniAuthService::instance()) {
        QString hiddenSwitchButton = UniAuthService::instance()->GetLastLoginUser();
        ASSERT_EQ(hiddenSwitchButton.isEmpty(), false);
    }
}

TEST_F(UniAuthServiceTest, GetQuickLoginUser)
{
    if (UniAuthService::instance()) {
        QString hiddenSwitchButton = UniAuthService::instance()->GetQuickLoginUser();
        ASSERT_EQ(hiddenSwitchButton.isEmpty(), false);
    }
}

TEST_F(UniAuthServiceTest, getUseFirstDevice)
{
    if (UniAuthService::instance()) {
        bool value = UniAuthService::instance()->getUseFirstDevice();
        UniAuthService::instance()->SwitchToUser(getenv("USER"));
        UniAuthService::instance()->SwitchToGreeterUser(getenv("USER"));
        UniAuthService::instance()->getWillSwitchUser();
        UniAuthService::instance()->SaveLastLoginUser(UniAuthService::instance()->GetLastLoginUser());
        UniAuthService::instance()->SaveQuickLoginUser(UniAuthService::instance()->GetQuickLoginUser());
        ASSERT_EQ(value, false);
    }
}
