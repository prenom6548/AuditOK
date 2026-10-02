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
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <security/pam_appl.h>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QLightDM/Greeter>
#include <QSocketNotifier>
#include <QDebug>
#include <QSignalSpy>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "../../src/lock-backend/authpamthread.h"
#include "../../src/lock-backend/pamauthenticate.h"
#include "../../src/lock-backend/lightdmhelper.h"
#include "../../src/lock-backend/dbusupperinterface.h"
#include "../../src/dbusifs/accountshelper.h"

#include "stubext.h"

using namespace stub_ext;

class DbusUpperInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAccountsHelperDbus = QSharedPointer<AccountsHelper>(new AccountsHelper());
        StubExt st;
        st.set_lamda(&LightDMHelper::connectToDaemonSync, []() { return true; });
        st.set_lamda(&isGreeterMode, []() { return true; });
        m_pLightDMHelperDbus = new LightDMHelper(m_pAccountsHelperDbus, Configuration::instance());
        m_pDbusUpperInterfaceDbus = new DbusUpperInterface();
        m_pDbusUpperInterfaceDbus->init();
    }

    static void TearDownTestSuite()
    {
        delete m_pDbusUpperInterfaceDbus;
        m_pDbusUpperInterfaceDbus = nullptr;

        delete m_pLightDMHelperDbus;
        m_pLightDMHelperDbus = nullptr;

        m_pAccountsHelperDbus.reset();
    }

    static DbusUpperInterface *m_pDbusUpperInterfaceDbus;

    static LightDMHelper *m_pLightDMHelperDbus;

    static QSharedPointer<AccountsHelper> m_pAccountsHelperDbus;
};

DbusUpperInterface *DbusUpperInterfaceTest::m_pDbusUpperInterfaceDbus = nullptr;
LightDMHelper *DbusUpperInterfaceTest::m_pLightDMHelperDbus = nullptr;
QSharedPointer<AccountsHelper> DbusUpperInterfaceTest::m_pAccountsHelperDbus = nullptr;


TEST_F(DbusUpperInterfaceTest, GetSlpState)
{
    m_pDbusUpperInterfaceDbus->onNameLost("test");
    bool value = m_pDbusUpperInterfaceDbus->GetSlpState();

    m_pLightDMHelperDbus->getLoginUserCount();
    m_pLightDMHelperDbus->findUserByUid(getuid());

    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, GetBlankState)
{
    bool value = m_pDbusUpperInterfaceDbus->GetBlankState();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, GetLockState)
{
    bool value = m_pDbusUpperInterfaceDbus->GetLockState();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, CheckAppVersion)
{
    bool value = m_pDbusUpperInterfaceDbus->CheckAppVersion();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, checkScreenDialogRunning)
{
    m_pDbusUpperInterfaceDbus->LockStartupMode();
    m_pDbusUpperInterfaceDbus->SessionTools();
    m_pDbusUpperInterfaceDbus->Lock();
    m_pDbusUpperInterfaceDbus->SwitchUser();
    m_pDbusUpperInterfaceDbus->AppBlockWindow("Suspend");
    m_pDbusUpperInterfaceDbus->MultiUserBlockWindow("Reboot");
    m_pDbusUpperInterfaceDbus->onShowBlankScreensaver();
    m_pDbusUpperInterfaceDbus->ShowScreensaver();
    m_pDbusUpperInterfaceDbus->LockScreensaver();
    //    m_pDbusUpperInterfaceDbus->Suspend();
    bool value = m_pDbusUpperInterfaceDbus->checkScreenDialogRunning();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, GenerateBatteryArgsList)
{
    QJsonArray value = m_pDbusUpperInterfaceDbus->GenerateBatteryArgsList();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, getSleepLockCheck)
{
    QJsonArray value = m_pDbusUpperInterfaceDbus->getSleepLockCheck();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(DbusUpperInterfaceTest, getShutdownLockcheck)
{
    QJsonArray value = m_pDbusUpperInterfaceDbus->getShutdownLockcheck();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(DbusUpperInterfaceTest, getSaverTheme)
{
    QJsonArray value = m_pDbusUpperInterfaceDbus->getSaverTheme();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(DbusUpperInterfaceTest, getBatteryIconName)
{
    QString value = m_pDbusUpperInterfaceDbus->getBatteryIconName();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, getIsBattery)
{
    bool value = m_pDbusUpperInterfaceDbus->getIsBattery();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, GetDefaultAuthUser)
{
    StubExt st;
    st.set_lamda(&isGreeterMode, []() { return true; });
    QString value = m_pDbusUpperInterfaceDbus->GetDefaultAuthUser();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, GetCurrentSession)
{
    QString value = m_pDbusUpperInterfaceDbus->GetCurrentSession();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, GenerateSessionInfoList)
{
    QJsonArray value = m_pDbusUpperInterfaceDbus->GenerateSessionInfoList();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, GetCurrentUser)
{
    QString value = m_pDbusUpperInterfaceDbus->GetCurrentUser();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(DbusUpperInterfaceTest, getCurTabletMode)
{
    bool value = m_pDbusUpperInterfaceDbus->getCurTabletMode();
    ASSERT_EQ(value, false);
}

TEST_F(DbusUpperInterfaceTest, onPamShowMessage)
{
    m_pDbusUpperInterfaceDbus->onPamShowMessage("",1);
}

TEST_F(DbusUpperInterfaceTest, onPamShowPrompt)
{
    m_pDbusUpperInterfaceDbus->onPamShowPrompt("",1);
}

TEST_F(DbusUpperInterfaceTest, onPamShowBinaryPrompt)
{   
    m_pDbusUpperInterfaceDbus->onPamShowBinaryPrompt("",1);
}

TEST_F(DbusUpperInterfaceTest, onPamAuthCompleted)
{   
    m_pDbusUpperInterfaceDbus->onPamAuthCompleted();
}

TEST_F(DbusUpperInterfaceTest, onBioAuthShowMessage)
{   
    m_pDbusUpperInterfaceDbus->onBioAuthShowMessage("");
}

TEST_F(DbusUpperInterfaceTest, GenerateUserInfoList)
{
    m_pDbusUpperInterfaceDbus->GenerateUserInfoList();
    m_pDbusUpperInterfaceDbus->onUsersInfoChanged();
    m_pDbusUpperInterfaceDbus->onLogin1ReqLock();
    m_pDbusUpperInterfaceDbus->onLogin1ReqUnLock();
    m_pDbusUpperInterfaceDbus->onLogin1PrepareForSleep(true);
    m_pDbusUpperInterfaceDbus->onLogin1PrepareForSleep(false);
    m_pDbusUpperInterfaceDbus->onLogin1SessionActiveChanged(true);
    m_pDbusUpperInterfaceDbus->onBlockInhibitedChanged("ShutDown");
    m_pDbusUpperInterfaceDbus->delayLockScreen();
    m_pDbusUpperInterfaceDbus->stopDelayLockScreen();
    m_pDbusUpperInterfaceDbus->onBatteryStatusChanged("test");
    m_pDbusUpperInterfaceDbus->onSessionIdleReceived();
    m_pDbusUpperInterfaceDbus->onSessionIdleExit();
    m_pDbusUpperInterfaceDbus->onLockScreenTimeout();
    m_pDbusUpperInterfaceDbus->onBatteryChanged(QStringList());
    m_pDbusUpperInterfaceDbus->onLidStateChanged(false);
}

TEST_F(DbusUpperInterfaceTest, SendUpdateInfoSig)
{
    QJsonObject getjsonCmd;
    QJsonParseError jsonParseError;
    QJsonObject objRes;
    QVariant varValue;

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF;
    getjsonCmd["Key"] = KEY_IDLE_DELAY;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc.object();
    varValue = objRes.value(KEY_IDLE_DELAY);
    m_pDbusUpperInterfaceDbus->onLockScreenConfigChanged(KEY_IDLE_DELAY, varValue.toDouble());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF;
    getjsonCmd["Key"] = KEY_SHOW_REST_TIME;
    const QJsonDocument jsonDoc1 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc1.object();
    varValue = objRes.value(KEY_SHOW_REST_TIME);
    m_pDbusUpperInterfaceDbus->onScreenSaverConfigChanged(KEY_SHOW_REST_TIME, varValue.toDouble());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF;
    getjsonCmd["Key"] = KEY_SLEEP_COMPUTER_AC;
    const QJsonDocument jsonDoc2 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc2.object();
    varValue = objRes.value(KEY_SLEEP_COMPUTER_AC);
    m_pDbusUpperInterfaceDbus->onPowerManagerConfigChanged(KEY_SLEEP_COMPUTER_AC, varValue.toDouble());

    //    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF;
    //    getjsonCmd["Key"] = KEY_PICTURE_FILENAME;
    //    const QJsonDocument jsonDoc3 =
    //    QJsonDocument::fromJson(m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
    //    &jsonParseError); objRes = jsonDoc3.object(); varValue = objRes.value(KEY_PICTURE_FILENAME);
    //    m_pDbusUpperInterfaceDbus->onMateBgConfigChanged(KEY_PICTURE_FILENAME, varValue.toString());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF;
    getjsonCmd["Key"] = KEY_HOUR_SYSTEM;
    const QJsonDocument jsonDoc4 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc4.object();
    varValue = objRes.value(KEY_HOUR_SYSTEM);
    m_pDbusUpperInterfaceDbus->onUkccPluginsConfigChanged(KEY_HOUR_SYSTEM, varValue.toDouble());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF;
    getjsonCmd["Key"] = KEY_SYSTEM_FONT_SIZE;
    const QJsonDocument jsonDoc5 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc5.object();
    varValue = objRes.value(KEY_SYSTEM_FONT_SIZE);
    m_pDbusUpperInterfaceDbus->onThemeStyleConfigChanged(KEY_SYSTEM_FONT_SIZE, varValue.toDouble());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF;
    getjsonCmd["Key"] = KEY_SESSION_IDLE;
    const QJsonDocument jsonDoc6 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc6.object();
    varValue = objRes.value(KEY_SESSION_IDLE);
    m_pDbusUpperInterfaceDbus->onSessionConfigChanged(KEY_SESSION_IDLE, varValue.toDouble());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF;
    getjsonCmd["Key"] = KEY_CAPSLOCK_STATUS;
    const QJsonDocument jsonDoc7 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc7.object();
    varValue = objRes.value(KEY_CAPSLOCK_STATUS);
    m_pDbusUpperInterfaceDbus->onKeyboardConfigChanged(KEY_CAPSLOCK_STATUS, varValue.toBool());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF;
    getjsonCmd["Key"] = KEY_AREA_SCREENSHOT;
    const QJsonDocument jsonDoc8 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc8.object();
    varValue = objRes.value(KEY_AREA_SCREENSHOT);
    m_pDbusUpperInterfaceDbus->onUsdMediaKeysConfigChanged(KEY_AREA_SCREENSHOT, varValue.toString());

    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF;
    getjsonCmd["Key"] = KEY_RFKILL_STATE;
    const QJsonDocument jsonDoc9 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    objRes = jsonDoc9.object();
    varValue = objRes.value(KEY_RFKILL_STATE);
    m_pDbusUpperInterfaceDbus->onUsdMediaStateKeysConfigChanged(KEY_RFKILL_STATE, varValue.toDouble());
}

TEST_F(DbusUpperInterfaceTest, GetInformation)
{
    //    QList<QVariant> argumentList;
    //    argumentList << QVariant::fromValue(QString(QJsonDocument(jsonCmd).toJson()));
    QList<int> list{ /*LOCK_CMD_ID_GET_USERINFO_LIST, */ LOCK_CMD_ID_GET_DEFAULT_AUTH_USER,
                     LOCK_CMD_ID_GET_CURRENT_USER,
                     LOCK_CMD_ID_GET_SESSIONS_LIST,
                     LOCK_CMD_ID_GET_CURRENT_SESSION,
                     LOCK_CMD_ID_LOGIN1_IS_SESSION_ACTIVE,
                     LOCK_CMD_ID_GET_AGREEMENT,
                     LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF,
                     /*LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF,*/ LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF,
                     LOCK_CMD_ID_PAMAUTH_IS_AUTHENTICATED,
                     LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION,
                     LOCK_CMD_ID_PAMAUTH_GET_AUTHUSER,
                     LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF,
                     LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANHIBERNATE,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANPOWEROFF,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANREBOOT,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSUSPEND,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOGOUT,
                     LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOCKSCREEN,
                     LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK,
                     LOCK_CMD_ID_UPOWER_BATTERY_STATUS,
                     LOCK_CMD_ID_UPOWER_IS_BATTERY,
                     LOCK_CMD_ID_UPOWER_BATTERY,
                     LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK,
                     LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK,
                     LOCK_CMD_ID_LOCK_SCREEN_GET_THEMES,
                     LOCK_CMD_ID_BIOAUTH_GET_AVAILABLE_DEVICES,
                     LOCK_CMD_ID_BIOAUTH_GET_DISABLED_DEVICES,
                     LOCK_CMD_ID_BIOAUTH_GET_STATE,
                     LOCK_CMD_ID_BIOAUTH_GET_CURDEVICE,
                     LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_ID,
                     LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_NAME,
                     LOCK_CMD_ID_BIOAUTH_GET_DEFAULT_DEVICE,
                     LOCK_CMD_ID_GET_PUBLIC_KEY,
                     LOCK_CMD_ID_TABLET_MODE,
                     -1 };

    for (int i = 0; i < list.count(); i++) {
        QJsonObject jsonCmd;
        jsonCmd["CmdId"] = list.at(i);
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    }
}

TEST_F(DbusUpperInterfaceTest, SetInformation)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SET_USER;
    jsonCmd["Content"] = getenv("USER");
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd).toJson()).toUtf8());

    QJsonObject jsonCmd1;
    jsonCmd1["CmdId"] = LOCK_CMD_ID_SWITCH_TO_USER;
    jsonCmd1["Content"] = getenv("USER");
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd1).toJson()).toUtf8());

    QJsonObject jsonCmd2;
    jsonCmd2["CmdId"] = LOCK_CMD_ID_SET_SESSION;
    jsonCmd2["Content"] = "testSession";
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd2).toJson()).toUtf8());

    QJsonObject jsonCmd3;
    jsonCmd3["CmdId"] = LOCK_CMD_ID_START_SESSION;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd3).toJson()).toUtf8());

    QJsonObject getjsonCmd;
    getjsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF;
    getjsonCmd["Key"] = KEY_IDLE_DELAY;
    QJsonParseError jsonParseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd).toJson())).toUtf8(),
        &jsonParseError);
    QJsonObject objRes = jsonDoc.object();
    QVariant varValue = objRes.value(KEY_IDLE_DELAY);
    QJsonObject jsonCmd4;
    jsonCmd4["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_LOCKSCREEN_CONF;
    jsonCmd4["Content"] = varValue.toDouble();
    jsonCmd4["Key"] = KEY_IDLE_DELAY;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd4).toJson()).toUtf8());

    QJsonObject getjsonCmd5;
    getjsonCmd5["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF;
    getjsonCmd5["Key"] = KEY_VIDEO_SIZE;
    QJsonParseError jsonParseError5;
    const QJsonDocument jsonDoc5 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd5).toJson())).toUtf8(),
        &jsonParseError5);
    QJsonObject objRes5 = jsonDoc5.object();
    QVariant varValue5 = objRes5.value(KEY_VIDEO_SIZE);
    QJsonObject jsonCmd5;
    jsonCmd5["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_SCREENSAVER_CONF;
    jsonCmd5["Content"] = varValue5.toDouble();
    jsonCmd5["Key"] = KEY_VIDEO_SIZE;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd5).toJson()).toUtf8());

    QJsonObject getjsonCmd6;
    getjsonCmd6["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF;
    getjsonCmd6["Key"] = KEY_SLEEP_COMPUTER_AC;
    QJsonParseError jsonParseError6;
    const QJsonDocument jsonDoc6 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd6).toJson())).toUtf8(),
        &jsonParseError6);
    QJsonObject objRes6 = jsonDoc6.object();
    QVariant varValue6 = objRes6.value(KEY_SLEEP_COMPUTER_AC);
    QJsonObject jsonCmd6;
    jsonCmd6["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_POWERMANAGER_CONF;
    jsonCmd6["Content"] = varValue6.toDouble();
    jsonCmd6["Key"] = KEY_SLEEP_COMPUTER_AC;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd6).toJson()).toUtf8());

    QJsonObject getjsonCmd7;
    getjsonCmd7["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF;
    getjsonCmd7["Key"] = KEY_HOUR_SYSTEM;
    QJsonParseError jsonParseError7;
    const QJsonDocument jsonDoc7 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd7).toJson())).toUtf8(),
        &jsonParseError7);
    QJsonObject objRes7 = jsonDoc7.object();
    QVariant varValue7 = objRes7.value(KEY_HOUR_SYSTEM);
    QJsonObject jsonCmd7;
    jsonCmd7["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_UKCCPLUGINS_CONF;
    jsonCmd7["Content"] = varValue7.toDouble();
    jsonCmd7["Key"] = KEY_HOUR_SYSTEM;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd7).toJson()).toUtf8());

    QJsonObject getjsonCmd8;
    getjsonCmd8["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF;
    getjsonCmd8["Key"] = KEY_SYSTEM_FONT_SIZE;
    QJsonParseError jsonParseError8;
    const QJsonDocument jsonDoc8 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd8).toJson())).toUtf8(),
        &jsonParseError8);
    QJsonObject objRes8 = jsonDoc8.object();
    QVariant varValue8 = objRes8.value(KEY_SYSTEM_FONT_SIZE);
    QJsonObject jsonCmd8;
    jsonCmd8["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_THEMESTYLE_CONF;
    jsonCmd8["Content"] = varValue8.toDouble();
    jsonCmd8["Key"] = KEY_SYSTEM_FONT_SIZE;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd8).toJson()).toUtf8());

    QJsonObject getjsonCmd9;
    getjsonCmd9["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF;
    getjsonCmd9["Key"] = KEY_SESSION_IDLE;
    QJsonParseError jsonParseError9;
    const QJsonDocument jsonDoc9 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd9).toJson())).toUtf8(),
        &jsonParseError9);
    QJsonObject objRes9 = jsonDoc9.object();
    QVariant varValue9 = objRes9.value(KEY_SESSION_IDLE);
    QJsonObject jsonCmd9;
    jsonCmd9["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_SESSION_CONF;
    jsonCmd9["Content"] = varValue9.toDouble();
    jsonCmd9["Key"] = KEY_SESSION_IDLE;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd9).toJson()).toUtf8());

    QJsonObject jsonCmd10;
    jsonCmd10["CmdId"] = LOCK_CMD_ID_PAMAUTH_AUTHENTICATE;
    jsonCmd10["Content"] = getenv("USER");
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd10).toJson()).toUtf8());

    QJsonObject jsonCmd11;
    jsonCmd11["CmdId"] = LOCK_CMD_ID_PAMAUTH_RESPOND;
    jsonCmd11["Content"] = "test";
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd11).toJson()).toUtf8());

    QJsonObject jsonCmd12;
    jsonCmd12["CmdId"] = LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL;
    //    jsonCmd12["Content"] = "test";
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd12).toJson()).toUtf8());

    QJsonObject jsonCmd13;
    jsonCmd13["CmdId"] = LOCK_CMD_ID_USD_MEDIAKEYS;
    jsonCmd13["Content"] = VOLUME_DOWN_KEY;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd13).toJson()).toUtf8());

    QJsonObject jsonCmd14;
    jsonCmd14["CmdId"] = LOCK_CMD_ID_LOGIN1_SET_POWER_MANAGER;
    jsonCmd14["Content"] = "LockScreen";
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd14).toJson()).toUtf8());

    QJsonObject jsonCmd15;
    jsonCmd15["CmdId"] = LOCK_CMD_ID_LOCK_STATE_CHANGED;
    jsonCmd15["Content"] = false;
    jsonCmd15["SessionTools"] = false;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd15).toJson()).toUtf8());

    QJsonObject jsonCmd16;
    jsonCmd16["CmdId"] = LOCK_CMD_ID_BIOAUTH_STARTAUTH;
    jsonCmd16["UserId"] = 1000;
    jsonCmd16["DevId"] = 101;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd16).toJson()).toUtf8());

    QJsonObject jsonCmd17;
    jsonCmd17["CmdId"] = LOCK_CMD_ID_BIOAUTH_STOPAUTH;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd17).toJson()).toUtf8());

    QJsonObject jsonCmd18;
    jsonCmd18["CmdId"] = LOCK_CMD_ID_KWIN_BLOCK_SHORTCUT;
    jsonCmd18["Content"] = true;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd18).toJson()).toUtf8());

    QJsonObject jsonCmd19;
    jsonCmd19["CmdId"] = LOCK_CMD_ID_KWIN_BLOCK_SHORTCUT;
    jsonCmd19["Content"] = false;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd19).toJson()).toUtf8());

    QJsonObject getjsonCmd20;
    getjsonCmd20["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF;
    getjsonCmd20["Key"] = KEY_RFKILL_STATE;
    QJsonParseError jsonParseError20;
    const QJsonDocument jsonDoc20 = QJsonDocument::fromJson(
        m_pDbusUpperInterfaceDbus->GetInformation(QString(QJsonDocument(getjsonCmd20).toJson())).toUtf8(),
        &jsonParseError20);
    QJsonObject objRes20 = jsonDoc20.object();
    QVariant varValue20 = objRes20.value(KEY_RFKILL_STATE);
    QJsonObject jsonCmd20;
    jsonCmd20["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_USD_MEDIA_STATE_KEYS_CONF;
    jsonCmd20["Content"] = varValue20.toDouble();
    jsonCmd20["Key"] = KEY_RFKILL_STATE;
    m_pDbusUpperInterfaceDbus->SetInformation(QString(QJsonDocument(jsonCmd20).toJson()).toUtf8());
}
