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
#include <QtDBus>
#include <QDBusAbstractInterface>

#include "../../src/dbusifs/biometrichelper.h"
#include "../../src/dbusifs/giodbus.h"

#include "stubext.h"
using namespace stub_ext;

class BiometricHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pABiometricHelperDbus = new BiometricHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pABiometricHelperDbus;
        m_pABiometricHelperDbus = nullptr;
    }

    static BiometricHelper *m_pABiometricHelperDbus;
};

BiometricHelper *BiometricHelperTest::m_pABiometricHelperDbus = nullptr;

TEST_F(BiometricHelperTest, Identify)
{
    get_server_gvariant_stdout(-1);
    QDBusPendingCall value = m_pABiometricHelperDbus->Identify(32, 1000, 0, -1);
    ASSERT_EQ(value.isValid(), false);
}

TEST_F(BiometricHelperTest, UkeyIdentify)
{
    QDBusPendingCall value = m_pABiometricHelperDbus->UkeyIdentify(100, 2, 1000);
    ASSERT_EQ(value.isValid(), false);
}

TEST_F(BiometricHelperTest, GetHasUkeyFeature)
{
    bool value = m_pABiometricHelperDbus->GetHasUkeyFeature(1000, 0, -1);
    ASSERT_EQ(value, false);
}

TEST_F(BiometricHelperTest, GetFeatureCount)
{
    bool value = m_pABiometricHelperDbus->GetFeatureCount(1000, 0, -1);
    ASSERT_EQ(value, false);
}

TEST_F(BiometricHelperTest, SetExtraInfo)
{
    int value = m_pABiometricHelperDbus->SetExtraInfo("extra_info", "extra_info");
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricHelperTest, StopOps)
{
    int value = m_pABiometricHelperDbus->StopOps(32, 3000);
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricHelperTest, GetUserDevCount)
{
    int value = m_pABiometricHelperDbus->GetUserDevCount(1000);
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricHelperTest, GetUserDevFeatureCount)
{
    int value = m_pABiometricHelperDbus->GetUserDevFeatureCount(1000, 32);
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricHelperTest, GetDevList)
{
    DeviceList value = m_pABiometricHelperDbus->GetDevList();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricHelperTest, GetUserFeatures)
{
    FeatureMap value = m_pABiometricHelperDbus->GetUserFeatures(1000);
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BiometricHelperTest, GetDevCount)
{
    int value = m_pABiometricHelperDbus->GetDevCount();
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricHelperTest, GetDevMesg)
{
    QString value = m_pABiometricHelperDbus->GetDevMesg(32);
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_EQ(cstr, "");
}

TEST_F(BiometricHelperTest, GetNotifyMesg)
{
    QString value = m_pABiometricHelperDbus->GetNotifyMesg(32);
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_EQ(cstr, "");
}

TEST_F(BiometricHelperTest, GetOpsMesg)
{
    QString value = m_pABiometricHelperDbus->GetOpsMesg(32);
    std::string str = value.toStdString();
    const char* cstr = str.data();
    ASSERT_EQ(cstr, "");
}

TEST_F(BiometricHelperTest, UpdateStatus)
{
    StatusReslut status = m_pABiometricHelperDbus->UpdateStatus(32);
    ASSERT_EQ(status.result , 0);
    ASSERT_EQ(status.enable , 0);
    ASSERT_EQ(status.devNum , 0);
    ASSERT_EQ(status.devStatus , 0);
    ASSERT_EQ(status.opsStatus , 0);
    ASSERT_EQ(status.notifyMessageId , 0);
}
