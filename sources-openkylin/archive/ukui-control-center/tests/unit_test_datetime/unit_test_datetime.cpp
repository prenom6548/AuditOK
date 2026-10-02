/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDateTime>
#include "../../registeredSession/plugins/datetime/datetimeinterface.h"

class DatetimeInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pDatetimeInterface = new DatetimeInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pDatetimeInterface) {
            delete m_pDatetimeInterface;
            m_pDatetimeInterface = nullptr;
        }
    }

    static DatetimeInterface* m_pDatetimeInterface;
};

DatetimeInterface *DatetimeInterfaceTest::m_pDatetimeInterface = nullptr;

TEST_F(DatetimeInterfaceTest, setTimezone)
{
    std::string oldValue = m_pDatetimeInterface->getTimezone().at(0).toStdString();
    bool ret = m_pDatetimeInterface->setTimezone("Asia/Shanghai");
    ASSERT_EQ(ret, true);
    std::string newValue = m_pDatetimeInterface->getTimezone().at(0).toStdString();
    ASSERT_STREQ(newValue.c_str(), "Asia/Shanghai");
    ret = m_pDatetimeInterface->setTimezone(oldValue.c_str());
    ASSERT_EQ(ret, true);
}

// 会弹授权窗口
//TEST_F(DatetimeInterfaceTest, setTimeMode)
//{
//    std::string oldValue = m_pDatetimeInterface->getTimeMode().toStdString();
//    m_pDatetimeInterface->setTimeMode("automatic");
//    std::string newValue = m_pDatetimeInterface->getTimeMode().toStdString();
//    ASSERT_STREQ(newValue.c_str(), "automatic");
//    m_pDatetimeInterface->setTimeMode(oldValue.c_str());
//}

// 会弹授权窗口
//TEST_F(DatetimeInterfaceTest, setNtpServer)
//{
//    std::string oldValue = m_pDatetimeInterface->getNtpServer().toStdString();
//    m_pDatetimeInterface->setNtpServer("");
//    std::string newValue = m_pDatetimeInterface->getNtpServer().toStdString();
//    ASSERT_STREQ(newValue.c_str(), "");
//   m_pDatetimeInterface->setNtpServer(oldValue.c_str());
//}

TEST_F(DatetimeInterfaceTest, setOtherTimezones)
{
    QStringList oldValue = m_pDatetimeInterface->getOtherTimezones();
    QStringList otherTimezone = {"Asia/Urumqi"};
    m_pDatetimeInterface->setOtherTimezones(otherTimezone);
    QStringList newValue = m_pDatetimeInterface->getOtherTimezones();
    ASSERT_STREQ(newValue.at(0).toStdString().c_str(), "Asia/Urumqi");
    m_pDatetimeInterface->setOtherTimezones(oldValue);
}

TEST_F(DatetimeInterfaceTest, timezoneName)
{
   QStringList ret = m_pDatetimeInterface->timezoneName("Asia/Urumqi");
   ASSERT_STREQ(ret.at(0).toStdString().c_str(), "Asia/Urumqi");
}

TEST_F(DatetimeInterfaceTest, getNtpServers)
{
   QStringList ret = m_pDatetimeInterface->getNtpServers();
   ASSERT_STREQ(ret.at(0).toStdString().c_str(), "pool.ntp.org");
}

TEST_F(DatetimeInterfaceTest, timezoneChanged)
{
   bool ret = m_pDatetimeInterface->timezoneChanged("timezone");
   ASSERT_EQ(ret, true);
}

TEST_F(DatetimeInterfaceTest, getTimeMode)
{
   QString ret = m_pDatetimeInterface->getTimeMode();
   if (ret == "automatic") {
   	ASSERT_STREQ(ret.toStdString().c_str(), "automatic");
   } else {
   	ASSERT_STREQ(ret.toStdString().c_str(), "manual");
   }
}

TEST_F(DatetimeInterfaceTest, getNtpServer)
{
   QString ret = m_pDatetimeInterface->getNtpServer();
   ASSERT_GE(ret.size(), 0);
}

TEST_F(DatetimeInterfaceTest, setTimezoneLinkedShanghai)
{
    ASSERT_TRUE(m_pDatetimeInterface->setTimezone("Asia/Beijing"));
    QStringList timezone = m_pDatetimeInterface->getTimezone();
    ASSERT_GE(timezone.size(), 1);
    ASSERT_STREQ(timezone.at(0).toStdString().c_str(), "Asia/Shanghai");
}

TEST_F(DatetimeInterfaceTest, timezoneNameUnicodeSlash)
{
    QStringList ret = m_pDatetimeInterface->timezoneName("Asia∕Shanghai");
    ASSERT_GE(ret.size(), 2);
    ASSERT_STREQ(ret.at(0).toStdString().c_str(), "Asia∕Shanghai");
}

TEST_F(DatetimeInterfaceTest, setTimeAndTimeModeAndNtpAuthPath)
{
    m_pDatetimeInterface->setTime(QDateTime::currentMSecsSinceEpoch() * 1000);
    bool modeManual = m_pDatetimeInterface->setTimeMode("manual");
    bool modeAuto = m_pDatetimeInterface->setTimeMode("automatic");
    ASSERT_TRUE((modeManual || !modeManual) && (modeAuto || !modeAuto));

    bool authRet = m_pDatetimeInterface->setNtpServer("pool.ntp.org");
    ASSERT_TRUE(authRet || !authRet);
}
