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

#include "../../biometric-manager/src/configuration.h"
#include "../../biometric-manager/src/customtype.h"
#include "../../biometric-manager/src/giodbus.h"
#include "../../biometric-manager/src/imageutil.h"
#include <QIcon>
#include "stubext.h"
using namespace stub_ext;

class ConfigurationTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pImageUtilDbus = new ImageUtil();
    }

    static void TearDownTestSuite()
    {
        delete m_pImageUtilDbus;
        m_pImageUtilDbus = nullptr;
    }

    static ImageUtil *m_pImageUtilDbus;

};

ImageUtil *ConfigurationTest::m_pImageUtilDbus = nullptr;

TEST_F(ConfigurationTest, getDefaultDevice)
{
    if (Configuration::instance()) {
        QString value = Configuration::instance()->getDefaultDevice();
        ASSERT_EQ(value.isEmpty(), false);
    }
}

TEST_F(ConfigurationTest, setDefaultDevice)
{
    if (Configuration::instance()) {
        QString oldValue = Configuration::instance()->getDefaultDevice();
        Configuration::instance()->setDefaultDevice("uru4000");
        Configuration::instance()->setDefaultDevice(oldValue);
    }
}

TEST_F(ConfigurationTest, getLoginSwitch)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getLoginSwitch();
        ASSERT_EQ(value, true);
    }
}

TEST_F(ConfigurationTest, setLoginSwitch)
{
    if (Configuration::instance()) {
        bool oldValue = Configuration::instance()->getLoginSwitch();
        Configuration::instance()->setLoginSwitch(!oldValue);
        Configuration::instance()->setLoginSwitch(oldValue);
    }
}

TEST_F(ConfigurationTest, getSaverSwitch)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getSaverSwitch();
        ASSERT_EQ(value, true);
    }
}

TEST_F(ConfigurationTest, setSaverSwitch)
{
    if (Configuration::instance()) {
        bool oldValue = Configuration::instance()->getSaverSwitch();
        Configuration::instance()->setSaverSwitch(!oldValue);
        Configuration::instance()->setSaverSwitch(oldValue);
    }
}

TEST_F(ConfigurationTest, getSystemSwitch)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getSystemSwitch();
        ASSERT_EQ(value, true);
    }
}

TEST_F(ConfigurationTest, setSystemSwitch)
{
    if (Configuration::instance()) {
        bool oldValue = Configuration::instance()->getSystemSwitch();
        Configuration::instance()->setSystemSwitch(!oldValue);
        Configuration::instance()->setSystemSwitch(oldValue);
    }
}

TEST_F(ConfigurationTest, get_current_version)
{
    if (Configuration::instance()) {
        QString value = Configuration::instance()->get_current_version();
        ASSERT_EQ(value.isEmpty(), false);
    }
}

TEST_F(ConfigurationTest, get_same_str_start)
{
    if (Configuration::instance()) {
        QString s1 = "ssss";
        QString s2 = "ssss1";
        QString value = Configuration::instance()->get_same_str_start(s1, s2);
        ASSERT_EQ(value.isEmpty(), false);
    }
}

TEST_F(ConfigurationTest, isNum)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->isNum("999");
        ASSERT_EQ(value, false);
    }
}

TEST_F(ConfigurationTest, registerCustomTypes)
{
    registerCustomTypes();
}

TEST_F(ConfigurationTest, transferBioType)
{
    EnumToString::transferBioType(BIOTYPE_FINGERPRINT);
    EnumToString::transferBioType(BIOTYPE_FINGERVEIN);
    EnumToString::transferBioType(BIOTYPE_IRIS);
    EnumToString::transferBioType(BIOTYPE_FACE);
    QString value = EnumToString::transferBioType(BIOTYPE_VOICEPRINT);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, lowerTransferBioType)
{
    EnumToString::lowerTransferBioType(BIOTYPE_FINGERPRINT);
    EnumToString::lowerTransferBioType(BIOTYPE_FINGERVEIN);
    EnumToString::lowerTransferBioType(BIOTYPE_IRIS);
    EnumToString::lowerTransferBioType(BIOTYPE_FACE);
    QString value = EnumToString::lowerTransferBioType(BIOTYPE_VOICEPRINT);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, transferVerifyType)
{
    EnumToString::transferVerifyType(VERIFY_HARDWARE);
    EnumToString::transferVerifyType(VERIFY_SOFTWARE);
    EnumToString::transferVerifyType(VERIFY_MIX);
    QString value = EnumToString::transferVerifyType(VERIFY_OTHER);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, transferStorageType)
{
    EnumToString::transferStorageType(STORAGE_DEVICE);
    EnumToString::transferStorageType(STORAGE_OS);
    QString value = EnumToString::transferStorageType(STORAGE_MIX);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, transferBusType)
{
    EnumToString::transferBusType(BUS_SERIAL);
    EnumToString::transferBusType(BUS_USB);
    EnumToString::transferBusType(BUS_PCIE);
    EnumToString::transferBusType(BUS_ANY);
    QString value = EnumToString::transferBusType(BUS_OTHER);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, transferIdentifyType)
{
    EnumToString::transferIdentifyType(VERIFY_HARDWARE);
    EnumToString::transferIdentifyType(VERIFY_SOFTWARE);
    EnumToString::transferIdentifyType(VERIFY_MIX);
    QString value = EnumToString::transferIdentifyType(VERIFY_OTHER);
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(ConfigurationTest, isAdmin)
{
    isAdmin(1000);
}

TEST_F(ConfigurationTest, getBioType)
{
    getBioType(BIOTYPE_FACE);
    getBioType(BIOTYPE_FINGERPRINT);
    getBioType(BIOTYPE_FINGERVEIN);
    getBioType(BIOTYPE_IRIS);
    int value = getBioType(BIOTYPE_VOICEPRINT);
    ASSERT_EQ(value, 4);
}

TEST_F(ConfigurationTest, get_server_gvariant_stdout)
{
    get_server_gvariant_stdout(32);
}

TEST_F(ConfigurationTest, loadSvg)
{
    QPixmap iconPixmap = m_pImageUtilDbus->loadSvg("/usr/share/background/house.png", "white", 16);
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "white");
    ASSERT_EQ(iconPixmap.isNull(), false);
}

TEST_F(ConfigurationTest, drawSymbolicColoredPixmap)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap colorPixmap = m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "white");
    m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "black");
    m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "gray");
    m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "blue");
    m_pImageUtilDbus->drawSymbolicColoredPixmap(pixmap, "");
    ASSERT_EQ(colorPixmap.isNull(), false);
}
