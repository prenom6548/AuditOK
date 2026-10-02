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
#include "../../src/ukccplugins/sessiondbus/screensaverinterface.h"
#include "../../src/lock-backend/lightdmhelper.h"
#include <qgsettings.h>
#include "stubext.h"

using namespace stub_ext;
class ScreensaverInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        StubExt st;
        st.set_lamda(&LightDMHelper::connectToDaemonSync, []() { return true; });
        st.set_lamda(&isGreeterMode, []() { return true; });
        m_pDbusUpperInterfaceDbus = new DbusUpperInterface();
        m_pDbusUpperInterfaceDbus->init();
        m_pScreenSaverInterface = new ScreensaverInterface(m_pDbusUpperInterfaceDbus, m_pDbusUpperInterfaceDbus);
    }

    static void TearDownTestSuite()
    {
        if (m_pScreenSaverInterface) {
            delete m_pScreenSaverInterface;
            m_pScreenSaverInterface = nullptr;
        }
        delete m_pDbusUpperInterfaceDbus;
        m_pDbusUpperInterfaceDbus = nullptr;
    }

    static ScreensaverInterface *m_pScreenSaverInterface;
    static DbusUpperInterface *m_pDbusUpperInterfaceDbus;
};

DbusUpperInterface *ScreensaverInterfaceTest::m_pDbusUpperInterfaceDbus = nullptr;
ScreensaverInterface *ScreensaverInterfaceTest::m_pScreenSaverInterface = nullptr;

TEST_F(ScreensaverInterfaceTest, setPreentryTime)
{
    int oldValue = m_pScreenSaverInterface->property("preentryTime").toInt();
    m_pScreenSaverInterface->setPreentryTime(10);
    int newValue = m_pScreenSaverInterface->property("preentryTime").toInt();
    ASSERT_EQ(newValue, 10);
    m_pScreenSaverInterface->setPreentryTime(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setScreensaverType)
{
    QGSettings gsetting("org.ukui.screensaver");
    std::string oldValue = m_pScreenSaverInterface->property("screensaverType").toString().toStdString();
    m_pScreenSaverInterface->setScreensaverType("ukui");
    std::string newValue = gsetting.get("mode").toString().toStdString();

    ASSERT_STREQ(newValue.c_str(), "default-ukui");
    m_pScreenSaverInterface->setScreensaverType(oldValue.c_str());
}

TEST_F(ScreensaverInterfaceTest, setShowBreakTimeUkuiCustom)
{
    bool oldValue = m_pScreenSaverInterface->property("showBreakTimeCustom").toBool();
    m_pScreenSaverInterface->setShowBreakTimeCustom(true);
    bool newValue = m_pScreenSaverInterface->property("showBreakTimeCustom").toBool();
    ASSERT_EQ(newValue, true);
    m_pScreenSaverInterface->setShowBreakTimeCustom(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setShowBreakTimeUkui)
{
    bool oldValue = m_pScreenSaverInterface->property("showBreakTimeUkui").toBool();
    m_pScreenSaverInterface->setShowBreakTimeUkui(false);
    bool newValue = m_pScreenSaverInterface->property("showBreakTimeUkui").toBool();
    ASSERT_EQ(newValue, false);
    m_pScreenSaverInterface->setShowBreakTimeUkui(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setScreenLockEnabled)
{
    bool oldValue = m_pScreenSaverInterface->property("screenLockEnabled").toBool();
    m_pScreenSaverInterface->setScreenLockEnabled(false);
    bool newValue = m_pScreenSaverInterface->property("screenLockEnabled").toBool();
    ASSERT_EQ(newValue, false);
    m_pScreenSaverInterface->setScreenLockEnabled(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setCustomPath)
{
    std::string oldValue = m_pScreenSaverInterface->property("customPath").toString().toStdString();
    m_pScreenSaverInterface->setCustomPath("/test/path");
    std::string newValue = m_pScreenSaverInterface->property("customPath").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "/test/path");
    m_pScreenSaverInterface->setCustomPath(oldValue.c_str());
}

TEST_F(ScreensaverInterfaceTest, setSwitchRandom)
{
    bool oldValue = m_pScreenSaverInterface->property("switchRandom").toBool();
    m_pScreenSaverInterface->setSwitchRandom(false);
    bool newValue = m_pScreenSaverInterface->property("switchRandom").toBool();
    ASSERT_EQ(newValue, false);
    m_pScreenSaverInterface->setSwitchRandom(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setCycleTime)
{
    int oldValue = m_pScreenSaverInterface->property("cycleTime").toInt();
    m_pScreenSaverInterface->setCycleTime(60);
    int newValue = m_pScreenSaverInterface->property("cycleTime").toInt();
    ASSERT_EQ(newValue, 60);
    m_pScreenSaverInterface->setCycleTime(oldValue);
}

TEST_F(ScreensaverInterfaceTest, setCustomText)
{
    std::string oldValue = m_pScreenSaverInterface->property("customText").toString().toStdString();
    m_pScreenSaverInterface->setCustomText("test custom text");
    std::string newValue = m_pScreenSaverInterface->property("customText").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "test custom text");
    m_pScreenSaverInterface->setCustomText(oldValue.c_str());
}

TEST_F(ScreensaverInterfaceTest, setCustomTextCentered)
{
    bool oldValue = m_pScreenSaverInterface->property("customTextCentered").toBool();
    m_pScreenSaverInterface->setCustomTextCentered(false);
    bool newValue = m_pScreenSaverInterface->property("customTextCentered").toBool();
    ASSERT_EQ(newValue, false);
    m_pScreenSaverInterface->setCustomTextCentered(oldValue);
}
