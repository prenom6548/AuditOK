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
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <glib.h>
#include "../../registeredSession/plugins/font/libfun.h"
#include "../../registeredSession/plugins/font/fontinterface.h"

class FontInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pFontInterface = new FontInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pFontInterface) {
            delete m_pFontInterface;
            m_pFontInterface = nullptr;
        }
    }
    static FontInterface* m_pFontInterface;
};

FontInterface *FontInterfaceTest::m_pFontInterface = nullptr;

TEST_F(FontInterfaceTest, getViewData)
{
    m_pFontInterface->updateFontList();
    QList<ViewData> res = m_pFontInterface->getViewData();
    ASSERT_GE(res.size(), 100);
}

TEST_F(FontInterfaceTest, getInstallViewData)
{
    m_pFontInterface->updateFontList();
    QList<ViewData> res = m_pFontInterface->getInstallViewData();
    ASSERT_GE(res.size(), 0);
}

TEST_F(FontInterfaceTest, getFontDetails)
{
    QList<FontInformation> allFonts = LibFun::getAllFontInformation();
    ASSERT_GT(allFonts.size(), 0);
    m_pFontInterface->updateFontList();
    QString fontPath = allFonts.first().path;
    FontInformation res = m_pFontInterface->getFontDetails(fontPath);
    ASSERT_STREQ(res.path.toStdString().c_str(), fontPath.toStdString().c_str());
}

TEST_F(FontInterfaceTest, resetDefault)
{
    bool res = m_pFontInterface->resetDefault();
    EXPECT_TRUE(res);
}

TEST_F(FontInterfaceTest, installFontList)
{
    QVariantList strList;
    bool res = m_pFontInterface->installFontList(strList);
    EXPECT_TRUE(res);
}

TEST_F(FontInterfaceTest, uninstallFont)
{
    bool res = m_pFontInterface->uninstallFont(QStringList(), "");
    EXPECT_TRUE(res);
}

TEST_F(FontInterfaceTest, applyFont)
{
    bool res = m_pFontInterface->applyFont("Ubuntu Mono", "/usr/share/fonts/truetype/ubuntu/UbuntuMono-Italic[wght].ttf", "Medium Italic");
    EXPECT_TRUE(res);
}

TEST_F(FontInterfaceTest, getFontSize)
{
    double oldValue = m_pFontInterface->getFontSize();
    m_pFontInterface->setFontSize(10);
    double res = m_pFontInterface->getFontSize();
    ASSERT_EQ(res, 10);
    m_pFontInterface->setFontSize(oldValue);
}

TEST_F(FontInterfaceTest, updateFontListAndApplyList)
{
    m_pFontInterface->updateFontList(true);
    m_pFontInterface->updateFontApplyList();
    SUCCEED();
}

TEST_F(FontInterfaceTest, getFontDetailsInvalidPath)
{
    m_pFontInterface->updateFontList();
    FontInformation res = m_pFontInterface->getFontDetails("/tmp/ukcc-font-not-exist.ttf");
    ASSERT_TRUE(res.path.isEmpty());
}

TEST_F(FontInterfaceTest, libfunChooseFontFile)
{
    ASSERT_TRUE(LibFun::chooseFontFile("sample.ttf"));
    ASSERT_TRUE(LibFun::chooseFontFile("sample.otf"));
    ASSERT_TRUE(LibFun::chooseFontFile("sample.ttc"));
    ASSERT_FALSE(LibFun::chooseFontFile("sample.txt"));
}

TEST_F(FontInterfaceTest, libfunInstallAndUninstallWithEmptyPath)
{
    ASSERT_FALSE(LibFun::installFont(""));
    ASSERT_FALSE(LibFun::uninstallFont(""));
}

TEST_F(FontInterfaceTest, libfunCopyFontWithEmptyPath)
{
    ASSERT_FALSE(LibFun::copyFont("", "/tmp"));
    ASSERT_FALSE(LibFun::copyFont("/tmp/font.ttf", ""));
}

TEST_F(FontInterfaceTest, libfunSortAndFontFileType)
{
    FontInformation left;
    left.name = "AFont";
    left.style = "Regular";
    FontInformation right;
    right.name = "BFont";
    right.style = "Bold";
    ASSERT_TRUE(LibFun::caseInsensitiveLessThan(left, right));
    ASSERT_FALSE(LibFun::isFontFile("/tmp/ukcc-not-exist-font"));
}

TEST_F(FontInterfaceTest, libfunRemoveWhitespaceAndVersion)
{
    char *rawWhitespace = g_strdup("  hello\n\n\tworld\n");
    LibFun::remove_whitespace(&rawWhitespace);
    QString whitespaceResult = QString::fromUtf8(rawWhitespace);
    ASSERT_TRUE(whitespaceResult.startsWith("hello"));
    ASSERT_TRUE(whitespaceResult.contains("world"));
    ASSERT_FALSE(whitespaceResult.contains("  hello"));
    g_free(rawWhitespace);

    char *rawVersion = g_strdup("FontName\nVersion 1.2.3\n");
    LibFun::remove_version(&rawVersion);
    ASSERT_STREQ(rawVersion, "1.2.3");
    g_free(rawVersion);
}

TEST_F(FontInterfaceTest, libfunGetFontInfo)
{
    QString realFont = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-Italic[wght].ttf";
    QString ret = LibFun::getFontInfo(realFont);
    ASSERT_TRUE(ret.isEmpty());

    QString invalidPath = "/tmp/ukcc-not-font.txt";
    QFile invalidFile(invalidPath);
    ASSERT_TRUE(invalidFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    invalidFile.write("not a font");
    invalidFile.close();
    QString invalidRet = LibFun::getFontInfo(invalidPath);
    ASSERT_STREQ(invalidRet.toStdString().c_str(), invalidPath.toStdString().c_str());
    QFile::remove(invalidPath);
}

TEST_F(FontInterfaceTest, libfunIsFontFileTrue)
{
    QString realFont = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-Italic[wght].ttf";
    ASSERT_TRUE(LibFun::isFontFile(realFont));
}

TEST_F(FontInterfaceTest, libfunInstallUninstallAndCopySuccess)
{
    QString realFont = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-Italic[wght].ttf";
    QFileInfo info(realFont);
    QString installPath = QDir::homePath() + "/.local/share/fonts/ukui-control-center/" + info.fileName();
    QFile::remove(installPath);
    ASSERT_TRUE(LibFun::installFont(realFont));
    ASSERT_TRUE(QFile::exists(installPath));
    ASSERT_TRUE(LibFun::uninstallFont(installPath));

    QString dstDir = QDir::tempPath() + "/ukcc-font-copy-test";
    QDir().mkpath(dstDir);
    QString copiedPath = dstDir + "/" + info.fileName();
    QFile::remove(copiedPath);
    ASSERT_TRUE(LibFun::copyFont(realFont, dstDir));
    ASSERT_TRUE(QFile::exists(copiedPath));
    QFile::remove(copiedPath);
    QDir().rmdir(dstDir);
}

TEST_F(FontInterfaceTest, libfunGetAllFontInformationAndUpdateList)
{
    QList<FontInformation> list = LibFun::getAllFontInformation();
    ASSERT_GE(list.size(), 0);
    ASSERT_TRUE(LibFun::updateFontList());
}

