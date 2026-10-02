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

#include "../../ukui-greeter/common/configuration.h"
#include "../../ukui-greeter/common/glibinterface.h"
#include "../../ukui-greeter/common/globalv.h"
#include "../../ukui-greeter/greeter/greeterutils.h"
#include "../../ukui-greeter/greeter/language.h"
#include "../../ukui-greeter/greeter/imageutil.h"
#include "../../ukui-greeter/BiometricAuth/giodbus.h"
#include <QPixmap>
#include <QIcon>
#include "stubext.h"
using namespace stub_ext;

class CommonTest : public testing::Test
{
protected:
//    static void SetUpTestSuite()
//    {
//        m_pServiceManagerDbus = new ServiceManager();
//    }

//    static void TearDownTestSuite()
//    {
//        delete m_pServiceManagerDbus;
//        m_pServiceManagerDbus = nullptr;
//    }

//    static ServiceManager *m_pServiceManagerDbus;
};

//ServiceManager *ServiceManagerTest::m_pServiceManagerDbus = nullptr;

TEST_F(CommonTest, getValue)
{
    if (Configuration::instance()) {
        QString color = Configuration::instance()->getValue("background-color").toString();
        ASSERT_EQ(color.isEmpty(), false);
    }
}

TEST_F(CommonTest, getCurrentUser)
{
    if (Configuration::instance()) {
        Configuration::instance()->getCurrentUser(getenv("USER"));
    }
}

TEST_F(CommonTest, setValue)
{
    if (Configuration::instance()) {
        QString color = Configuration::instance()->getValue("backgroundPath").toString();
        QString oldvalue = Configuration::instance()->getUserConfig("backgroundPath").toString();
        Configuration::instance()->setValue("backgroundPath", oldvalue);
        ASSERT_EQ(oldvalue.isEmpty(), false);
    }
}

TEST_F(CommonTest, hasValue)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->hasValue("backgroundPath");
        ASSERT_EQ(value, false);
    }
}

TEST_F(CommonTest, getLogo)
{
    if (Configuration::instance()) {
        QPixmap value = Configuration::instance()->getLogo();
        ASSERT_EQ(value.isNull(), false);
    }
}

TEST_F(CommonTest, getIs990)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getIs990();
        ASSERT_EQ(value, false);
    }
}

TEST_F(CommonTest, getLastNumLock)
{
    if (Configuration::instance()) {
        bool value = Configuration::instance()->getLastNumLock();
        Configuration::instance()->saveLastNumLock(value);
        ASSERT_EQ(value, false);
    }
}

TEST_F(CommonTest, getRootBackgroundOption)
{
    if (Configuration::instance()) {
        int value = Configuration::instance()->getRootBackgroundOption(getenv("USER"));
        ASSERT_EQ(value, 1);
    }
}

TEST_F(CommonTest, getDefaultBackgroundName)
{
    if (Configuration::instance()) {
        QString background = Configuration::instance()->getDefaultBackgroundName();
        ASSERT_EQ(background.isEmpty(), false);
    }
}

TEST_F(CommonTest, setUsersFont)
{
    if (Configuration::instance()) {
//        Configuration::instance()->setUsersFont();
    }
}

TEST_F(CommonTest, setUsersThemeColor)
{
    if (Configuration::instance()) {
        Configuration::instance()->setUsersThemeColor();
    }
}

TEST_F(CommonTest, getPtToPx)
{
    if (Configuration::instance()) {
        Configuration::instance()->getPtToPx();
    }
}

TEST_F(CommonTest, getLastLoginUser)
{
    if (Configuration::instance()) {
        QString background = Configuration::instance()->getLastLoginUser();
    }
}

TEST_F(CommonTest, saveLastLoginUser)
{
    if (Configuration::instance()) {
        Configuration::instance()->saveLastLoginUser(Configuration::instance()->getLastLoginUser());
    }
}

TEST_F(CommonTest, saveLastLoginUser1)
{
    if (Configuration::instance()) {
        Configuration::instance()->saveLastLoginUser1(Configuration::instance()->getLastLoginUser());
    }
}

TEST_F(CommonTest, getUseFirstDevice)
{
    if (Configuration::instance()) {
        Configuration::instance()->getUseFirstDevice();
    }
}

TEST_F(CommonTest, scaledPixmap)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newPixmap = scaledPixmap(1920, 1080, Configuration::instance()->getUserConfig("backgroundPath").toString());
    ASSERT_EQ(newPixmap.isNull(), false);
}

TEST_F(CommonTest, PixmapToRound)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newPixmap = PixmapToRound(pixmap, 4);
    ASSERT_EQ(newPixmap.isNull(), false);
}

TEST_F(CommonTest, loadSvg)
{
    QPixmap iconPixmap = ImageUtil::loadSvg(":/image/assets/show-password.png", "white", 16);
    ASSERT_EQ(iconPixmap.isNull(), false);
}

TEST_F(CommonTest, drawSymbolicColoredPixmap)
{
    QPixmap retryIcon = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    ASSERT_EQ(ImageUtil::drawSymbolicColoredPixmap(retryIcon, "white").isNull(), false);
    ASSERT_EQ(ImageUtil::drawSymbolicColoredPixmap(retryIcon, "black").isNull(), false);
    ASSERT_EQ(ImageUtil::drawSymbolicColoredPixmap(retryIcon, "gray").isNull(), false);
    ASSERT_EQ(ImageUtil::drawSymbolicColoredPixmap(retryIcon, "blue").isNull(), false);
    ASSERT_EQ(ImageUtil::drawSymbolicColoredPixmap(retryIcon, "pick").isNull(), false);
//    ASSERT_EQ(getLoadingIcon(16).width(), 16);
}

TEST_F(CommonTest, scaleBlurPixmap)
{
    QPixmap *pixmap = new QPixmap(scaleBlurPixmap(16, 16, Configuration::instance()->getUserConfig("backgroundPath").toString()));
    ASSERT_EQ(pixmap->width(), 16);
}

TEST_F(CommonTest, blurPixmap)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newpixmap = blurPixmap(pixmap);
    ASSERT_EQ(newpixmap.width(), 16);
}

TEST_F(CommonTest, cutPixmap)
{
    QPixmap pixmap = QIcon::fromTheme("view-refresh-symbolic").pixmap(48, 48);
    QPixmap newpixmap = cutPixmap(pixmap);
    ASSERT_EQ(newpixmap.width(), 16);
}

TEST_F(CommonTest, isPicture)
{
    bool value = isPicture(Configuration::instance()->getUserConfig("backgroundPath").toString());
    ASSERT_EQ(value, false);
}

TEST_F(CommonTest, isCommunity)
{
    bool value = UKUIGreeter::Utils::isCommunity();
    ASSERT_EQ(value, false);
}

TEST_F(CommonTest, getDefaultFace)
{
    QString value = UKUIGreeter::Utils::getDefaultFace();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(CommonTest, getLanguage)
{
    Language lang = getLanguage("zh_CN.UTF-8");
//    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(CommonTest, giodbus)
{
    ASSERT_EQ(get_server_gvariant_stdout(32), 0);
}

TEST_F(CommonTest, getDefaultFontSize)
{
    ASSERT_EQ(getDefaultFontSize(), 1);
}
