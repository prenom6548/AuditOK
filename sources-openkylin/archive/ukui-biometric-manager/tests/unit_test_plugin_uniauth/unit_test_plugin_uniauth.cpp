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

#include "../../biometric-plugin/uniauthservice.h"

#include "stubext.h"
using namespace stub_ext;

class UniAuthServiceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pUniAuthServiceDbus = new UniAuthService();
    }

    static void TearDownTestSuite()
    {
        delete m_pUniAuthServiceDbus;
        m_pUniAuthServiceDbus = nullptr;
    }

    static UniAuthService *m_pUniAuthServiceDbus;
};

UniAuthService *UniAuthServiceTest::m_pUniAuthServiceDbus = nullptr;

TEST_F(UniAuthServiceTest, DefaultDevice)
{
    QString oldValue = m_pUniAuthServiceDbus->getDefaultDevice(getenv("USER"), 0);
    m_pUniAuthServiceDbus->setDefaultDevice(0, "testValue");
    QString newValue = m_pUniAuthServiceDbus->getDefaultDevice(getenv("USER"), 0);
    std::string str = newValue.toStdString();
    const char* cstr = str.data();
    ASSERT_STREQ(cstr, "testValue");
    m_pUniAuthServiceDbus->setDefaultDevice(0, oldValue);
}

TEST_F(UniAuthServiceTest, BioAuthStatus)
{
    bool oldValue = m_pUniAuthServiceDbus->getBioAuthStatus(getenv("USER"), 0);
    m_pUniAuthServiceDbus->setBioAuthStatus(0, !oldValue);
    bool newValue = m_pUniAuthServiceDbus->getBioAuthStatus(getenv("USER"), 0);
    ASSERT_EQ(newValue, !oldValue);
    m_pUniAuthServiceDbus->setBioAuthStatus(0, oldValue);
}

TEST_F(UniAuthServiceTest, AllDefaultDevice)
{
    QStringList defaultDeviceList = m_pUniAuthServiceDbus->getAllDefaultDevice(getenv("USER"));
    ASSERT_EQ(defaultDeviceList.isEmpty(), false);
}

TEST_F(UniAuthServiceTest, MaxFailedTimes)
{
    int maxFailedTimes = m_pUniAuthServiceDbus->getMaxFailedTimes();
    ASSERT_EQ(maxFailedTimes, 5);
}

TEST_F(UniAuthServiceTest, QRCodeEnable)
{
    bool qRCodeEnable = m_pUniAuthServiceDbus->getQRCodeEnable();
    ASSERT_EQ(qRCodeEnable, true);
}

TEST_F(UniAuthServiceTest, DoubleAuth)
{
    bool doubleAuth = m_pUniAuthServiceDbus->getDoubleAuth();
    ASSERT_EQ(doubleAuth, false);
}

TEST_F(UniAuthServiceTest, UserBind)
{
    bool userBind = m_pUniAuthServiceDbus->getUserBind();
    ASSERT_EQ(userBind, false);
}

TEST_F(UniAuthServiceTest, IsShownInControlCenter)
{
    bool isShownInControlCenter = m_pUniAuthServiceDbus->getIsShownInControlCenter();
    ASSERT_EQ(isShownInControlCenter, false);
}

TEST_F(UniAuthServiceTest, HiddenSwitchButton)
{
    bool hiddenSwitchButton = m_pUniAuthServiceDbus->getHiddenSwitchButton();
    ASSERT_EQ(hiddenSwitchButton, false);
}

TEST_F(UniAuthServiceTest, getUseFirstDevice)
{
    bool getUseFirstDevice = m_pUniAuthServiceDbus->getUseFirstDevice();
    ASSERT_EQ(getUseFirstDevice, false);
}

TEST_F(UniAuthServiceTest, isActivatable)
{
    bool isActivatable = m_pUniAuthServiceDbus->isActivatable();
    ASSERT_EQ(isActivatable, false);
}



