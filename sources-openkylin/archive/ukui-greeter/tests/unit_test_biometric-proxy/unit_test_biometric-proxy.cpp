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
#include <QDBusAbstractInterface>

#include "../../ukui-greeter/BiometricAuth/biometricdeviceinfo.h"
#include "../../ukui-greeter/BiometricAuth/biometricproxy.h"

#include "stubext.h"
using namespace stub_ext;

class BiometricProxyTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pBiometricProxyDbus = new BiometricProxy();
    }

    static void TearDownTestSuite()
    {
        delete m_pBiometricProxyDbus;
        m_pBiometricProxyDbus = nullptr;
    }

    static BiometricProxy *m_pBiometricProxyDbus;
};

BiometricProxy *BiometricProxyTest::m_pBiometricProxyDbus = nullptr;

TEST_F(BiometricProxyTest, getDeviceType)
{
    QString value = DeviceType::getDeviceType(DeviceType::FingerPrint);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, getDeviceType_tr)
{
    QString value = DeviceType::getDeviceType_tr(DeviceType::FingerPrint);
    DeviceType::getDeviceType_tr(DeviceType::FingerVein);
    DeviceType::getDeviceType_tr(DeviceType::Iris);
    DeviceType::getDeviceType_tr(DeviceType::Face);
    DeviceType::getDeviceType_tr(DeviceType::VoicePrint);
    DeviceType::getDeviceType_tr(REMOTE_QRCODE_TYPE);
    DeviceType::getDeviceType_tr(DeviceType::__MAX_NR_TYPES);
    registerMetaType();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, GetDefaultDevice)
{
    QString value = GetDefaultDevice(getenv("USER"));
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BiometricProxyTest, GetMaxFailedAutoRetry)
{
    int value = GetMaxFailedAutoRetry(getenv("USER"));
    GetMaxTimeoutAutoRetry(getenv("USER"));
    ASSERT_EQ(value, 5);
}

TEST_F(BiometricProxyTest, DEgetDeviceType)
{
    QString value = DeviceType::getDeviceType(DeviceType::FingerPrint);
    ASSERT_EQ(value.isEmpty(), false);
}


TEST_F(BiometricProxyTest, biometricdeviceinfo)
{
    int oldLastDevice = GetLastDevice("USER");
    SetLastDevice(getenv("User"), 101);
    SetLastDevice(getenv("User"), oldLastDevice);
    GetHiddenSwitchButton();
    GetAuthEnable();
    GetQRCodeEnable();
    GetFailedTimes();
    GetUseFirstDevice();
    GetMaxFailedAutoRetry(getenv("User"));
    GetMaxTimeoutAutoRetry(getenv("User"));
    ASSERT_EQ(GetDefaultDevice(getenv("USER")).isEmpty(), true);
}

TEST_F(BiometricProxyTest, Identify)
{
    m_pBiometricProxyDbus->Identify(32, getuid(), -1, 0);
//    ASSERT_EQ(connectToService, true);
}

TEST_F(BiometricProxyTest, GetFeatureCount)
{
    int value = m_pBiometricProxyDbus->GetFeatureCount(getuid(), -1, 0);
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricProxyTest, StopOps)
{
    int value = m_pBiometricProxyDbus->StopOps(32, 0);
    ASSERT_EQ(value, 0);
}


TEST_F(BiometricProxyTest, GetDevList)
{
    DeviceList value = m_pBiometricProxyDbus->GetDevList();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, GetDevCount)
{
    int value = m_pBiometricProxyDbus->GetDevCount();
    ASSERT_EQ(value, 0);
}

TEST_F(BiometricProxyTest, GetDevMesg)
{
    QString value = m_pBiometricProxyDbus->GetDevMesg(32);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, GetNotifyMesg)
{
    QString value = m_pBiometricProxyDbus->GetNotifyMesg(32);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, GetOpsMesg)
{
    QString value = m_pBiometricProxyDbus->GetOpsMesg(32);
    m_pBiometricProxyDbus->UpdateStatus(32);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BiometricProxyTest, GetUserFeatures)
{
    ASSERT_EQ(m_pBiometricProxyDbus->GetUserFeatures(1000).count(), 0);
}

TEST_F(BiometricProxyTest, GetUserDevCount)
{
    ASSERT_EQ(m_pBiometricProxyDbus->GetUserDevCount(1000), 0);
}

TEST_F(BiometricProxyTest, GetFrameFd)
{
    ASSERT_EQ(m_pBiometricProxyDbus->GetFrameFd(1000), 0);
}

TEST_F(BiometricProxyTest, GetHasUkeyFeature)
{
    ASSERT_EQ(m_pBiometricProxyDbus->GetHasUkeyFeature(32), false);
}
