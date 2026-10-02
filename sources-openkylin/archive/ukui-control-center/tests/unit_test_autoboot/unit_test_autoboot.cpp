/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include <QDBusReply>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QFileInfo>
#include <QDBusMetaType>
#include <QDBusArgument>
#include "../../registeredSession/plugins/autoboot/autostartinterface.h"

static QString createTempDesktopFile(QString* outDesktopName)
{
    QString unique = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString desktopName = QString("ukcc-autoboot-test-%1.desktop").arg(unique);
    QString desktopPath = QDir::tempPath() + "/" + desktopName;
    QFile file(desktopPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        *outDesktopName = "";
        return "";
    }
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=UKCC Autoboot Test " << unique << "\n";
    stream << "Exec=/bin/true\n";
    stream << "Icon=application-x-executable\n";
    stream << "NoDisplay=false\n";
    file.close();
    *outDesktopName = desktopName;
    return desktopPath;
}

static QString createTempDesktopFileWithName(const QString &desktopName)
{
    QString desktopPath = QDir::tempPath() + "/" + desktopName;
    QFile file(desktopPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return "";
    }
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=UKCC Temp " << desktopName << "\n";
    stream << "Exec=/bin/true\n";
    stream << "Icon=application-x-executable\n";
    stream << "NoDisplay=false\n";
    file.close();
    return desktopPath;
}

class AutobootInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAutoStartInterface = new AutoStartInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pAutoStartInterface) {
            delete m_pAutoStartInterface;
            m_pAutoStartInterface = nullptr;
      	}
    }

    static AutoStartInterface* m_pAutoStartInterface;
};

AutoStartInterface *AutobootInterfaceTest::m_pAutoStartInterface = nullptr;

TEST_F(AutobootInterfaceTest, addAutobootApp)
{
    QString desktopName;
    QString sourceDesktop = createTempDesktopFile(&desktopName);
    ASSERT_FALSE(sourceDesktop.isEmpty());
    QString localAutostartDesktop = QDir::homePath() + "/.config/autostart/" + desktopName;

    m_pAutoStartInterface->deleteLocalFile(desktopName);
    bool ret = m_pAutoStartInterface->addAutobootApp(sourceDesktop);
    ASSERT_EQ(ret, true);
    m_pAutoStartInterface->deleteLocalFile(desktopName);
    QFile::remove(localAutostartDesktop);
    QFile::remove(sourceDesktop);
}

TEST_F(AutobootInterfaceTest, getAppInfo)
{
    AutoApp ret = m_pAutoStartInterface->getAppInfo("/usr/share/applications/kylin-video.desktop");
    ASSERT_STREQ(ret.bname.toStdString().c_str(), "kylin-video.desktop");
}

TEST_F(AutobootInterfaceTest, saveAppStatusToggleHidden)
{
    QString desktopName;
    QString sourceDesktop = createTempDesktopFile(&desktopName);
    ASSERT_FALSE(sourceDesktop.isEmpty());
    QString localAutostartDesktop = QDir::homePath() + "/.config/autostart/" + desktopName;

    m_pAutoStartInterface->deleteLocalFile(desktopName);
    ASSERT_TRUE(m_pAutoStartInterface->addAutobootApp(sourceDesktop));
    bool ret = m_pAutoStartInterface->saveAppStatus(desktopName, false);
    ASSERT_EQ(ret, true);
    m_pAutoStartInterface->deleteLocalFile(desktopName);
    QFile::remove(localAutostartDesktop);
    QFile::remove(sourceDesktop);
}

TEST_F(AutobootInterfaceTest, saveAppStatus)
{
    bool ret = m_pAutoStartInterface->saveAppStatus("/usr/share/applications/kylin-video.desktop", false);
    ASSERT_EQ(ret, false);
}

TEST_F(AutobootInterfaceTest, setApplist)
{
    QStringList oldValue = m_pAutoStartInterface->getAppList();
    QStringList strList = {"kylin-weather.desktop", "sogouImeService.desktop", "kylin-video.desktop"};
    bool ret = m_pAutoStartInterface->setApplist(strList);
    ASSERT_EQ(ret, true);
    QStringList newValue = m_pAutoStartInterface->getAppList();
    ASSERT_STREQ(newValue.at(0).toStdString().c_str(), "kylin-weather.desktop");
    m_pAutoStartInterface->setApplist(oldValue);
}

TEST_F(AutobootInterfaceTest, getDisplayStatus)
{
    bool ret = m_pAutoStartInterface->getDisplayStatus("/usr/share/applications/kylin-video.desktop");
    ASSERT_EQ(ret, false);
}

TEST_F(AutobootInterfaceTest, deleteLocalFile)
{
    QString desktopName;
    QString sourceDesktop = createTempDesktopFile(&desktopName);
    ASSERT_FALSE(sourceDesktop.isEmpty());
    QString localAutostartDesktop = QDir::homePath() + "/.config/autostart/" + desktopName;

    m_pAutoStartInterface->deleteLocalFile(desktopName);
    ASSERT_TRUE(m_pAutoStartInterface->addAutobootApp(sourceDesktop));
    bool ret = m_pAutoStartInterface->deleteLocalFile(desktopName);
    ASSERT_EQ(ret, true);
    ASSERT_FALSE(QFile::exists(localAutostartDesktop));
    QFile::remove(sourceDesktop);
}

TEST_F(AutobootInterfaceTest, deleteLocalFileFalse)
{
    bool ret = m_pAutoStartInterface->deleteLocalFile("");
    ASSERT_EQ(ret, false);
}

TEST_F(AutobootInterfaceTest, getAllAppMap)
{
    QMap<QString, AutoApp> ret = m_pAutoStartInterface->getAllAppMap();
    ASSERT_GT(ret.size(), 1);
}

TEST_F(AutobootInterfaceTest, getServiceMap)
{
    QMap<QString, AutoApp> ret = m_pAutoStartInterface->getServiceMap();
    ASSERT_GT(ret.size(), 0);
}

TEST_F(AutobootInterfaceTest, getLocalAppMap)
{
    QMap<QString, AutoApp> ret = m_pAutoStartInterface->getLocalAppMap();
    ASSERT_GE(ret.size(), 0);
}

TEST_F(AutobootInterfaceTest, getStatusMap)
{
    QMap<QString, QVariant> ret = m_pAutoStartInterface->getStatusMap();
    ASSERT_GE(ret.size(), 0);
}

TEST_F(AutobootInterfaceTest, saveAppStatusInvalid)
{
    bool ret = m_pAutoStartInterface->saveAppStatus("", false);
    ASSERT_FALSE(ret);
}

TEST_F(AutobootInterfaceTest, getDisplayStatusInvalidFile)
{
    bool ret = m_pAutoStartInterface->getDisplayStatus("/tmp/ukcc-not-exist.desktop");
    ASSERT_FALSE(ret);
}

TEST_F(AutobootInterfaceTest, setAutobootBlacklist)
{
    QStringList list;
    list << "ukcc-not-exist.desktop";
    bool ret = m_pAutoStartInterface->setAutobootBlacklist(list);
    ASSERT_TRUE(ret);
}

TEST_F(AutobootInterfaceTest, setAutobootAllBlackAndWhitelist)
{
    ASSERT_TRUE(m_pAutoStartInterface->setAutobootAllBlack());
    QStringList whiteList;
    ASSERT_TRUE(m_pAutoStartInterface->setAutobootWhitelist(whiteList));
}

TEST_F(AutobootInterfaceTest, setAutobootWhitelistNonExistentDesktop)
{
    QStringList whiteList;
    whiteList << "ukcc-not-exist.desktop";
    ASSERT_TRUE(m_pAutoStartInterface->setAutobootWhitelist(whiteList));
}

TEST_F(AutobootInterfaceTest, keyFileToTile)
{
    QString outPath = QDir::tempPath() + "/ukcc-autoboot-keyfile.desktop";
    GKeyFile *keyfile = g_key_file_new();
    g_key_file_set_string(keyfile, "Desktop Entry", "Name", "UKCC Auto Test");
    g_key_file_set_string(keyfile, "Desktop Entry", "Exec", "/bin/true");
    gboolean ret = m_pAutoStartInterface->keyFileToTile(keyfile, outPath.toUtf8().constData());
    ASSERT_TRUE(ret);
    ASSERT_TRUE(QFile::exists(outPath));
    QFile::remove(outPath);
    g_key_file_free(keyfile);
}

TEST_F(AutobootInterfaceTest, setInformationShownFilter)
{
    QString filePath = QDir::tempPath() + "/ukcc-autoboot-show-filter.desktop";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=UKCCShowFilter\n";
    stream << "Exec=/bin/true\n";
    stream << "OnlyShowIn=GNOME;\n";
    file.close();
    AutoApp app = m_pAutoStartInterface->setInformation(filePath);
    ASSERT_FALSE(app.shown);
    QFile::remove(filePath);
}

TEST_F(AutobootInterfaceTest, getAppInfoSogouIconPath)
{
    QString desktopPath = createTempDesktopFileWithName("sogouImeService.desktop");
    ASSERT_FALSE(desktopPath.isEmpty());
    AutoApp app = m_pAutoStartInterface->getAppInfo(desktopPath);
    ASSERT_STREQ(app.icon.toStdString().c_str(), "/opt/sogouimebs/files/share/resources/skin/logo/logo.png");
    QFile::remove(desktopPath);
}

TEST_F(AutobootInterfaceTest, setInformationSogouIconPath)
{
    QString desktopPath = createTempDesktopFileWithName("sogouImeService.desktop");
    ASSERT_FALSE(desktopPath.isEmpty());
    AutoApp app = m_pAutoStartInterface->setInformation(desktopPath);
    ASSERT_STREQ(app.bname.toStdString().c_str(), "sogouImeService.desktop");
    ASSERT_FALSE(app.icon.isEmpty());
    QFile::remove(desktopPath);
}

TEST_F(AutobootInterfaceTest, addAutobootAppDuplicate)
{
    QString desktopName;
    QString sourceDesktop = createTempDesktopFile(&desktopName);
    ASSERT_FALSE(sourceDesktop.isEmpty());
    ASSERT_TRUE(m_pAutoStartInterface->addAutobootApp(sourceDesktop));
    ASSERT_FALSE(m_pAutoStartInterface->addAutobootApp(sourceDesktop));
    m_pAutoStartInterface->deleteLocalFile(desktopName);
    QFile::remove(sourceDesktop);
}

TEST_F(AutobootInterfaceTest, saveAppStatusUnsafeName)
{
    bool ret = m_pAutoStartInterface->saveAppStatus("..", false);
    ASSERT_FALSE(ret);
}

TEST_F(AutobootInterfaceTest, getAllAppMapInsertLocalOnly)
{
    QString desktopName = "ukcc-local-only.desktop";
    QString localPath = QDir::homePath() + "/.config/autostart/" + desktopName;
    QDir().mkpath(QDir::homePath() + "/.config/autostart");
    QFile file(localPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QTextStream stream(&file);
    stream << "[Desktop Entry]\nType=Application\nName=UKCCLocalOnly\nExec=/bin/true\n";
    file.close();

    m_pAutoStartInterface->mSysServiceMap.clear();
    QMap<QString, AutoApp> map = m_pAutoStartInterface->getAllAppMap();
    ASSERT_TRUE(map.contains(desktopName));
    QFile::remove(localPath);
}

TEST_F(AutobootInterfaceTest, saveAppStatusSystemPosFlow)
{
    QString desktopName;
    QString srcDesktop = createTempDesktopFile(&desktopName);
    ASSERT_FALSE(srcDesktop.isEmpty());
    QString localPath = QDir::homePath() + "/.config/autostart/" + desktopName;
    m_pAutoStartInterface->deleteLocalFile(desktopName);

    AutoApp app;
    app.bname = desktopName;
    app.path = srcDesktop;
    app.hidden = false;
    app.position = SYSTEMPOS;
    m_pAutoStartInterface->mAllAppMap[desktopName] = app;
    m_pAutoStartInterface->mSysServiceMap[desktopName] = app;

    ASSERT_TRUE(m_pAutoStartInterface->saveAppStatus(desktopName, false));
    ASSERT_TRUE(QFile::exists(localPath));
    ASSERT_TRUE(m_pAutoStartInterface->saveAppStatus(desktopName, true));
    QFile::remove(srcDesktop);
}

TEST_F(AutobootInterfaceTest, autoAppDbusCodec)
{
    qRegisterMetaType<AutoApp>("AutoApp");
    qDBusRegisterMetaType<AutoApp>();
    AutoApp in;
    in.bname = "name.desktop";
    in.icon = "icon";
    in.name = "name";
    in.hidden = true;
    in.position = LOCALPOS;
    in.path = "/tmp/name.desktop";
    in.no_display = false;
    in.shown = true;
    in.comment = "comment";
    in.exec = "/bin/true";
    in.description = "desc";
    QDBusArgument argument;
    argument << in;
    const QDBusArgument &readArg = argument;
    AutoApp out;
    readArg >> out;
    SUCCEED();
}
