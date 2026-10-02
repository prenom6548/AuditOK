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
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <KSharedConfig>
#include <KConfigGroup>
#include "../../registeredSession/plugins/default/defaultinterface.h"

class DefaultInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
       m_pDefaultInterface = new DefaultInterface();
    }

    static void TearDownTestSuite()
    {
       if (m_pDefaultInterface) {
           delete m_pDefaultInterface;
           m_pDefaultInterface = nullptr;
       }
    }

    static DefaultInterface* m_pDefaultInterface;
};

DefaultInterface *DefaultInterfaceTest::m_pDefaultInterface = nullptr;

TEST_F(DefaultInterfaceTest, getDefaultApp)
{
    QVariantList ret = m_pDefaultInterface->getDefaultApp("x-scheme-handler/http");
    ASSERT_GT(ret.size(), 0);
}

TEST_F(DefaultInterfaceTest, getAppList)
{
    QVariantList ret = m_pDefaultInterface->getAppList("x-scheme-handler/http");
    ASSERT_GT(ret.size(), 0);
}

TEST_F(DefaultInterfaceTest, setDefaultApp)
{
    bool ret = m_pDefaultInterface->setDefaultApp("kylin-music.desktop", "audio/x-vorbis+ogg");
    ASSERT_EQ(ret, true);
}

TEST_F(DefaultInterfaceTest, getSystemDefaultAppList)
{
    QMap<QString, QVariant> ret = m_pDefaultInterface->getSystemDefaultAppList();
    ASSERT_GT(ret.size(), 0);
}

TEST_F(DefaultInterfaceTest, setSelfDefaultAppInvalidDesktop)
{
    bool ret = m_pDefaultInterface->setSelfDefaultApp("ukui-not-exist.desktop", "x-scheme-handler/http");
    ASSERT_FALSE(ret);
}

TEST_F(DefaultInterfaceTest, saveMimeTypeAssociation)
{
    const QString mimeType = "text/plain";
    const QString storageId = "kylin-text-editor.desktop";
    m_pDefaultInterface->saveMimeTypeAssociation(mimeType, storageId);

    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"),
                                                            KConfig::NoGlobals,
                                                            QStandardPaths::GenericConfigLocation);
    KConfigGroup defaultApp(profile, "Default Applications");
    QStringList apps = defaultApp.readXdgListEntry(mimeType);
    ASSERT_TRUE(apps.contains(storageId));
}

TEST_F(DefaultInterfaceTest, getDefaultAppInvalidMimeType)
{
    QVariantList ret = m_pDefaultInterface->getDefaultApp("ukcc/not-exist-mime");
    ASSERT_EQ(ret.size(), 0);
}

TEST_F(DefaultInterfaceTest, setDefaultAppAllMimeTypeBranches)
{
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "x-scheme-handler/http"));
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "x-scheme-handler/mailto"));
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "image/png"));
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "audio/x-vorbis+ogg"));
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "video/mp4"));
    ASSERT_TRUE(m_pDefaultInterface->setDefaultApp("kylin-video.desktop", "text/plain"));
}

TEST_F(DefaultInterfaceTest, privateSetDefaultProgramPaths)
{
    QVector<Appinfo> browserApps = DefaultInterface::_getAppList("x-scheme-handler/http");
    if (!browserApps.isEmpty() && browserApps.first().item && g_app_info_get_id(browserApps.first().item)) {
        m_pDefaultInterface->setWebBrowsersDefaultProgram(const_cast<char *>(g_app_info_get_id(browserApps.first().item)));
    }
    QVector<Appinfo> mailApps = DefaultInterface::_getAppList("x-scheme-handler/mailto");
    if (!mailApps.isEmpty() && mailApps.first().item && g_app_info_get_id(mailApps.first().item)) {
        m_pDefaultInterface->setMailReadersDefaultProgram(const_cast<char *>(g_app_info_get_id(mailApps.first().item)));
    }
    QVector<Appinfo> imageApps = DefaultInterface::_getAppList("image/png");
    if (!imageApps.isEmpty() && imageApps.first().item && g_app_info_get_id(imageApps.first().item)) {
        m_pDefaultInterface->setImageViewersDefaultProgram(const_cast<char *>(g_app_info_get_id(imageApps.first().item)));
    }
    QVector<Appinfo> audioApps = DefaultInterface::_getAppList("audio/x-vorbis+ogg");
    if (!audioApps.isEmpty() && audioApps.first().item && g_app_info_get_id(audioApps.first().item)) {
        m_pDefaultInterface->setAudioPlayersDefaultProgram(const_cast<char *>(g_app_info_get_id(audioApps.first().item)));
    }
    QVector<Appinfo> videoApps = DefaultInterface::_getAppList("video/x-ogm+ogg");
    if (!videoApps.isEmpty() && videoApps.first().item && g_app_info_get_id(videoApps.first().item)) {
        m_pDefaultInterface->setVideoPlayersDefaultProgram(const_cast<char *>(g_app_info_get_id(videoApps.first().item)));
    }
    QVector<Appinfo> textApps = DefaultInterface::_getAppList("text/plain");
    if (!textApps.isEmpty() && textApps.first().item && g_app_info_get_id(textApps.first().item)) {
        m_pDefaultInterface->setTextEditorsDefautlProgram(const_cast<char *>(g_app_info_get_id(textApps.first().item)));
    }
    SUCCEED();
}

TEST_F(DefaultInterfaceTest, resetDefaultWithoutLocalFile)
{
    QString oldLocalFile = m_pDefaultInterface->mLocalMimefile;
    m_pDefaultInterface->mLocalMimefile = QDir::tempPath() + "/ukcc-not-exist-mimeapps.list";
    QFile::remove(m_pDefaultInterface->mLocalMimefile);
    m_pDefaultInterface->resetDefault();
    ASSERT_FALSE(QFile::exists(m_pDefaultInterface->mLocalMimefile));
    m_pDefaultInterface->mLocalMimefile = oldLocalFile;
}

TEST_F(DefaultInterfaceTest, serviceDbusCodec)
{
    qRegisterMetaType<Service>("Service");
    qDBusRegisterMetaType<Service>();
    Service in;
    in.icon = "icon";
    in.name = "name";
    in.storageId = "name.desktop";
    QDBusArgument argument;
    argument << in;
    const QDBusArgument &readArg = argument;
    Service out;
    readArg >> out;
    SUCCEED();
}
