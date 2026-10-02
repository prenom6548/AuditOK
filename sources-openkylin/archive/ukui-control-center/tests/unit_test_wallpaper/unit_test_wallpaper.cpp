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
#include <QGSettings>
#include <QFile>
#include "../../registeredSession/plugins/wallpaper/wallpaperinterface.h"

class WallpaperInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pWallpaperInterface = new WallpaperInterface();
    }

    static void TearDownTestSuite()
    {
	if (m_pWallpaperInterface) {
	     delete m_pWallpaperInterface;
	     m_pWallpaperInterface = nullptr;
	}
    }

    static WallpaperInterface* m_pWallpaperInterface;
};

WallpaperInterface *WallpaperInterfaceTest::m_pWallpaperInterface = nullptr;

TEST_F(WallpaperInterfaceTest, getWallpaperMode)
{
    QGSettings gset("org.mate.background");
    QString oldValue = gset.get("picture-filename").toString();
    m_pWallpaperInterface->getWallpaper();
    std::string curValue = m_pWallpaperInterface->getWallpaperMode().toStdString();
    if (oldValue.isEmpty()) {
        ASSERT_STREQ(curValue.c_str(), "color");
    } else {
        ASSERT_STREQ(curValue.c_str(), "picture");
    }
}

TEST_F(WallpaperInterfaceTest, getWallpaper)
{
    std::string wallpaper = m_pWallpaperInterface->getWallpaper().toStdString();
    ASSERT_GE(wallpaper.size(), 0);
}

TEST_F(WallpaperInterfaceTest, setWallpaperPicture)
{
    QGSettings gset("org.mate.background");
    QString oldValue = gset.get("picture-filename").toString();
    QString mode = "picture";
    QString colorData = "";
    if (oldValue.isEmpty()) {
	mode = "color";
    	colorData = gset.get("primary-color").toString();
    }
    std::string wallpaperMode = m_pWallpaperInterface->getWallpaperMode().toStdString();
    m_pWallpaperInterface->setWallpaper("picture", "/usr/share/backgrounds/city.jpg");
    std::string newValue = gset.get("picture-filename").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "/usr/share/backgrounds/city.jpg");
    if (mode == "color") {
        m_pWallpaperInterface->setWallpaper("color", colorData.toStdString().c_str());
    } else {
        m_pWallpaperInterface->setWallpaper("picture", oldValue.toStdString().c_str());
    }
}

TEST_F(WallpaperInterfaceTest, setWallpaperColor)
{
    QGSettings gset("org.mate.background");
    QString oldValue = gset.get("picture-filename").toString();
    QString mode = "picture";
    QString colorData = "";
    if (oldValue.isEmpty()) {
	mode = "color";
    	colorData = gset.get("primary-color").toString();
    }
    std::string wallpaperMode = m_pWallpaperInterface->getWallpaperMode().toStdString();
    m_pWallpaperInterface->setWallpaper("color", "#2d7d9a");
    std::string newValue = gset.get("primary-color").toString().toStdString();
    ASSERT_STREQ(newValue.c_str(), "#2d7d9a");
    if (mode == "color") {
        m_pWallpaperInterface->setWallpaper("color", colorData.toStdString().c_str());
    } else {
        m_pWallpaperInterface->setWallpaper("picture", oldValue.toStdString().c_str());
    }
}

TEST_F(WallpaperInterfaceTest, setDisplayType)
{
    std::string oldType = m_pWallpaperInterface->getDisplayType().toStdString();
    m_pWallpaperInterface->setDisplayType("centered");
    QGSettings gset("org.mate.background");
    std::string newType = gset.get("picture-options").toString().toStdString();
    ASSERT_STREQ(newType.c_str(), "centered");
    m_pWallpaperInterface->setDisplayType(oldType.c_str());
}

TEST_F(WallpaperInterfaceTest, setDisplayTypeRejectsInvalidValue)
{
    QGSettings gset("org.mate.background");
    QString oldType = gset.get("picture-options").toString();
    m_pWallpaperInterface->setDisplayType("invalid-display-type");
    ASSERT_STREQ(gset.get("picture-options").toString().toStdString().c_str(),
                 oldType.toStdString().c_str());
}

TEST_F(WallpaperInterfaceTest, setWallpaperRejectsInvalidPath)
{
    QGSettings gset("org.mate.background");
    QString oldValue = gset.get("picture-filename").toString();
    m_pWallpaperInterface->setWallpaper("picture", "/etc/passwd");
    ASSERT_STREQ(gset.get("picture-filename").toString().toStdString().c_str(),
                 oldValue.toStdString().c_str());
}

TEST_F(WallpaperInterfaceTest, setWallpaperRejectsPathTraversal)
{
    QGSettings gset("org.mate.background");
    QString oldValue = gset.get("picture-filename").toString();
    m_pWallpaperInterface->setWallpaper("picture", "/usr/share/backgrounds/../../../etc/passwd");
    ASSERT_STREQ(gset.get("picture-filename").toString().toStdString().c_str(),
                 oldValue.toStdString().c_str());
}

TEST_F(WallpaperInterfaceTest, resetDefault)
{
    QGSettings gset("org.mate.background");
    QString oldType = gset.get("picture-options").toString();
    std::string oldPicName = gset.get("picture-filename").toString().toStdString();

    m_pWallpaperInterface->resetDefault();

    std::string newType = gset.get("picture-options").toString().toStdString();
    ASSERT_STREQ(newType.c_str(), "scaled");

    QString newPicName = gset.get("picture-filename").toString();
    ASSERT_FALSE(newPicName.isEmpty());
    ASSERT_TRUE(QFile::exists(newPicName));

    m_pWallpaperInterface->setDisplayType(oldType);
    gset.set("picture-filename", oldPicName.c_str());
}

TEST_F(WallpaperInterfaceTest, getPreviewWallpapers)
{
    int size = m_pWallpaperInterface->property("previewWallpapers").toStringList().size();
    ASSERT_GE(size, 0);
}

TEST_F(WallpaperInterfaceTest, getSourceWallpapers)
{
    int size = m_pWallpaperInterface->property("sourceWallpapers").toStringList().size();
    ASSERT_GE(size, 0);
}

TEST_F(WallpaperInterfaceTest, setNeedChangeCustomTheme)
{
    m_pWallpaperInterface->setNeedChangeCustomTheme(false);
    ASSERT_TRUE(m_pWallpaperInterface->getWallpaperMode() == "color" || m_pWallpaperInterface->getWallpaperMode() == "picture");
    m_pWallpaperInterface->setNeedChangeCustomTheme(true);
}

TEST_F(WallpaperInterfaceTest, changeGlobalTheme)
{
    m_pWallpaperInterface->changeGlobalTheme();
    ASSERT_TRUE(true);
}

TEST_F(WallpaperInterfaceTest, setNeedChangeCustomThemeAndSetWallpaper)
{
    QGSettings gset("org.mate.background");
    QString oldPic = gset.get("picture-filename").toString();
    m_pWallpaperInterface->setNeedChangeCustomTheme(false);
    m_pWallpaperInterface->setWallpaper("picture", "/usr/share/backgrounds/city.jpg");
    ASSERT_STREQ(gset.get("picture-filename").toString().toStdString().c_str(), "/usr/share/backgrounds/city.jpg");
    if (!oldPic.isEmpty()) {
        m_pWallpaperInterface->setWallpaper("picture", oldPic);
    }
    m_pWallpaperInterface->setNeedChangeCustomTheme(true);
}

TEST_F(WallpaperInterfaceTest, gsettingsChangedBranches)
{
    QGSettings gset("org.mate.background");
    QString oldDisplay = gset.get("picture-options").toString();
    QString oldPic = gset.get("picture-filename").toString();

    gset.set("picture-options", "zoom");
    ASSERT_STREQ(gset.get("picture-options").toString().toStdString().c_str(), "zoom");

    m_pWallpaperInterface->setNeedChangeCustomTheme(false);
    gset.set("picture-filename", "/usr/share/backgrounds/city.jpg");
    ASSERT_STREQ(gset.get("picture-filename").toString().toStdString().c_str(), "/usr/share/backgrounds/city.jpg");
    m_pWallpaperInterface->setNeedChangeCustomTheme(true);

    gset.set("picture-options", oldDisplay);
    gset.set("picture-filename", oldPic);
}

TEST_F(WallpaperInterfaceTest, constructorColorModePath)
{
    QGSettings gset("org.mate.background");
    QString oldPic = gset.get("picture-filename").toString();
    QString oldPrimary = gset.get("primary-color").toString();
    gset.set("picture-filename", "");
    gset.set("primary-color", "#1a2b3c");

    WallpaperInterface *instance = new WallpaperInterface();
    ASSERT_STREQ(instance->getWallpaperMode().toStdString().c_str(), "color");
    ASSERT_STREQ(instance->getWallpaper().toStdString().c_str(), "#1a2b3c");
    delete instance;

    gset.set("picture-filename", oldPic);
    gset.set("primary-color", oldPrimary);
}

TEST_F(WallpaperInterfaceTest, setWallpaperWithoutUserEnv)
{
    QByteArray oldUser = qgetenv("USER");
    qunsetenv("USER");
    m_pWallpaperInterface->setWallpaper("picture", "/usr/share/backgrounds/city.jpg");
    ASSERT_STREQ(m_pWallpaperInterface->getWallpaperMode().toStdString().c_str(), "picture");
    if (!oldUser.isEmpty()) {
        qputenv("USER", oldUser);
    }
}

TEST_F(WallpaperInterfaceTest, constructorFallbackWallpaperPath)
{
    QGSettings gset("org.mate.background");
    QString oldPic = gset.get("picture-filename").toString();
    gset.set("picture-filename", "/usr/share/backgrounds/city.jpg");
    UniversalInterface::self()->accountUserDbus()->call("SetBackgroundFile", "");

    WallpaperInterface *instance = new WallpaperInterface();
    std::string wallpaper = instance->getWallpaper().toStdString();
    ASSERT_STREQ(wallpaper.c_str(), "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg");
    delete instance;

    if (!oldPic.isEmpty()) {
        UniversalInterface::self()->accountUserDbus()->call("SetBackgroundFile", oldPic);
    }
    gset.set("picture-filename", oldPic);
}

TEST_F(WallpaperInterfaceTest, invokeGsettingsChangedSignalBranches)
{
    QGSettings gset("org.mate.background");
    QString oldDisplay = gset.get("picture-options").toString();
    QString oldPic = gset.get("picture-filename").toString();
    QString oldPrimary = gset.get("primary-color").toString();

    gset.set("picture-options", "zoom");
    QMetaObject::invokeMethod(UniversalInterface::self()->mateBackgroundGsettings(),
                              "changed",
                              Q_ARG(QString, "pictureOptions"));
    ASSERT_STREQ(m_pWallpaperInterface->getDisplayType().toStdString().c_str(), "zoom");

    gset.set("picture-filename", "");
    gset.set("primary-color", "#334455");
    QMetaObject::invokeMethod(UniversalInterface::self()->mateBackgroundGsettings(),
                              "changed",
                              Q_ARG(QString, "pictureFilename"));
    ASSERT_STREQ(m_pWallpaperInterface->getWallpaperMode().toStdString().c_str(), "color");
    ASSERT_STREQ(m_pWallpaperInterface->getWallpaper().toStdString().c_str(), "#334455");

    gset.set("picture-options", oldDisplay);
    gset.set("picture-filename", oldPic);
    gset.set("primary-color", oldPrimary);
}
