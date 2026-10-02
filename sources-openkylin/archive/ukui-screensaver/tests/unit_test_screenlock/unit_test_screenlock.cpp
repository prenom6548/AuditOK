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
#include "../../src/ukccplugins/sessiondbus/screenlockinterface.h"
#include "../../src/lock-backend/lightdmhelper.h"
#include <qgsettings.h>
#include "stubext.h"

using namespace stub_ext;
class ScreenlockInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        StubExt st;
        st.set_lamda(&LightDMHelper::connectToDaemonSync, []() { return true; });
        st.set_lamda(&isGreeterMode, []() { return true; });
        m_pDbusUpperInterfaceDbus = new DbusUpperInterface();
        m_pDbusUpperInterfaceDbus->init();
        m_pScreenlockInterface = new ScreenlockInterface(m_pDbusUpperInterfaceDbus, m_pDbusUpperInterfaceDbus);
    }

    static void TearDownTestSuite()
    {
        if (m_pScreenlockInterface) {
            delete m_pScreenlockInterface;
            m_pScreenlockInterface = nullptr;
        }
        delete m_pDbusUpperInterfaceDbus;
        m_pDbusUpperInterfaceDbus = nullptr;
    }

    static ScreenlockInterface *m_pScreenlockInterface;
    static DbusUpperInterface *m_pDbusUpperInterfaceDbus;
};

DbusUpperInterface *ScreenlockInterfaceTest::m_pDbusUpperInterfaceDbus = nullptr;
ScreenlockInterface *ScreenlockInterfaceTest::m_pScreenlockInterface = nullptr;

TEST_F(ScreenlockInterfaceTest, setShowOnLogin)
{
    bool oldValue = m_pScreenlockInterface->property("showOnLogin").toBool();
    m_pScreenlockInterface->setShowOnLogin(false);
    bool newValue = m_pScreenlockInterface->property("showOnLogin").toBool();
    ASSERT_EQ(newValue, false);
    m_pScreenlockInterface->setShowOnLogin(oldValue);
}

TEST_F(ScreenlockInterfaceTest, setWallpaper)
{
    QGSettings gset("org.ukui.screensaver");
    std::string oldValue = gset.get("background").toString().toStdString();
    m_pScreenlockInterface->setWallpaper("/usr/share/backgrounds/city.jpg");
    std::string newValue = gset.get("background").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "/usr/share/backgrounds/city.jpg");
    m_pScreenlockInterface->setWallpaper(oldValue.c_str());
}

TEST_F(ScreenlockInterfaceTest, resetDefault)
{
    QGSettings gset("org.ukui.screensaver");
    std::string oldValue = gset.get("background").toString().toStdString();
    m_pScreenlockInterface->resetDefault();
    std::string newValue = gset.get("background").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "/usr/share/backgrounds/1-openkylin.jpg");
    m_pScreenlockInterface->setWallpaper(oldValue.c_str());
}

TEST_F(ScreenlockInterfaceTest, getPreviewWallpapers)
{
    int size = m_pScreenlockInterface->getPreviewWallpapers().size();
    ASSERT_GE(size, 0);
}

TEST_F(ScreenlockInterfaceTest, getSourceWallpapers)
{
    int size = m_pScreenlockInterface->getSourceWallpapers().size();
    ASSERT_GE(size, 0);
}

TEST_F(ScreenlockInterfaceTest, getWallpaper)
{
    int size = m_pScreenlockInterface->getWallpaper().size();
    ASSERT_GE(size, 0);
}
