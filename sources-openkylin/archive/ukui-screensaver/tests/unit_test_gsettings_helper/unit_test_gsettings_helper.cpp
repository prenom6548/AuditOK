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

#include "../../src/lock-backend/gsettingshelper.h"
#include "../../src/common/definetypes.h"

#include "stubext.h"

using namespace stub_ext;

class GSettingsHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pGSettingsHelperDbus = new GSettingsHelper();
    }

    static void TearDownTestSuite()
    {
        delete m_pGSettingsHelperDbus;
        m_pGSettingsHelperDbus = nullptr;
    }

    static GSettingsHelper *m_pGSettingsHelperDbus;
};

GSettingsHelper *GSettingsHelperTest::m_pGSettingsHelperDbus = nullptr;

TEST_F(GSettingsHelperTest, GetScreenSaverConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_SHOW_REST_TIME);
    bool value1 = result1.toBool();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_SHOW_REST_TIME);

    QVariant result2 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_SHOW_CUSTOM_REST_TIME);
    bool value2 = result2.toBool();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_SHOW_CUSTOM_REST_TIME);

    QVariant result3 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_SHOW_UKUI_REST_TIME);
    bool value3 = result3.toBool();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_SHOW_UKUI_REST_TIME);

    QVariant result4 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_CYCLE_TIME);
    int value4 = result4.toInt();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_CYCLE_TIME);

    QVariant result5 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE);
    bool value5 = result5.toBool();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_AUTOMATIC_SWITCHING_ENABLE);

    QVariant result6 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_BACKGROUND_PATH);
    QString value6 = result6.toString();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_BACKGROUND_PATH);

    QVariant result7 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_MYTEXT);
    QString value7 = result7.toString();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_MYTEXT);

    QVariant result8 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_TEXT_IS_CENTER);
    bool value8 = result8.toBool();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_TEXT_IS_CENTER);

    QVariant result9 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_SHOW_MESSAGE_ENABLED);
    bool value9 = result9.toBool();

    QVariant result10 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_MESSAGE_NUMBER);
    int value10 = result10.toInt();

    QVariant result11 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_VIDEO_FORMAT);
    QString value11 = result11.toString();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_VIDEO_FORMAT);

    QVariant result12 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_VIDEO_PATH);
    QString value12 = result12.toString();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_VIDEO_PATH);

    QVariant result13 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_VIDEO_SIZE);
    int value13 = result13.toInt();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_VIDEO_SIZE);

    QVariant result14 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_VIDEO_WIDTH);
    int value14 = result14.toInt();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_VIDEO_WIDTH);

    QVariant result15 = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_VIDEO_HEIGHT);
    int value15 = result15.toInt();
    m_pGSettingsHelperDbus->onScreenSaverConfigChanged(KEY_VIDEO_HEIGHT);

    ASSERT_EQ(value1, true);
    ASSERT_EQ(value2, true);
    ASSERT_EQ(value3, true);
    ASSERT_EQ(value4, 300);
    ASSERT_EQ(value5, false);
    ASSERT_EQ(value6.isEmpty(), false);
    ASSERT_EQ(value7.isEmpty(), true);
    ASSERT_EQ(value8, true);
    ASSERT_EQ(value9, false);
    ASSERT_EQ(value10, 0);
    ASSERT_EQ(value11.isEmpty(), false);
    ASSERT_EQ(value12.isEmpty(), false);
    ASSERT_EQ(value13, 100);
    ASSERT_EQ(value14, 1920);
    ASSERT_EQ(value15, 1080);
}

TEST_F(GSettingsHelperTest, SetScreenSaverConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetScreenSaverConf(KEY_SHOW_REST_TIME);
    bool oldValue = result.toBool();

    bool value = m_pGSettingsHelperDbus->SetScreenSaverConf(KEY_SHOW_REST_TIME, true);

    m_pGSettingsHelperDbus->SetScreenSaverConf(KEY_SHOW_REST_TIME, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetLockScreenConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_DELAY);
    int value1 = result1.toInt();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_DELAY);

    QVariant result2 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_LOCK);
    int value2 = result2.toInt();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_LOCK);

    QVariant result3 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_LOCK);
    bool value3 = result3.toBool();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_LOCK);

    QVariant result4 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_LOCK);
    bool value4 = result4.toBool();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_LOCK);

    QVariant result5 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_LOCK);
    int value5 = result5.toInt();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_LOCK);

    QVariant result6 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_LOCK);
    bool value6 = result6.toBool();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IDLE_LOCK);

    QVariant result7 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_SLEEP_ACTIVATION_ENABLED);
    bool value7 = result7.toBool();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_SLEEP_ACTIVATION_ENABLED);

    QVariant result8 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_LOCK_ENABLED);
    bool value8 = result8.toBool();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_LOCK_ENABLED);

    QVariant result9 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_BACKGROUND);
    QString value9 = result9.toString();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_BACKGROUND);

    QVariant result10 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_MODE);
    QString value10 = result10.toString();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_MODE);

    QVariant result11 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_THEMES);
    QStringList value11 = result11.toStringList();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_THEMES);

    QVariant result12 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IMAGE_TRANSITION_EFFECT);
    int value12 = result12.toInt();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IMAGE_TRANSITION_EFFECT);

    QVariant result13 = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IMAGE_SWITCH_INTERVAL);
    int value13 = result13.toInt();
    m_pGSettingsHelperDbus->onLockScreenConfigChanged(KEY_IMAGE_SWITCH_INTERVAL);

    ASSERT_EQ(value1, 5);
    ASSERT_EQ(value2, -1);
    ASSERT_EQ(value3, true);
    ASSERT_EQ(value4, true);
    ASSERT_EQ(value5, 10);
    ASSERT_EQ(value6, true);
    ASSERT_EQ(value7, true);
    ASSERT_EQ(value8, true);
    ASSERT_EQ(value9.isEmpty(), false);
    ASSERT_EQ(value10.isEmpty(), false);
    ASSERT_EQ(value11.isEmpty(), false);
    ASSERT_EQ(value12, 0);
    ASSERT_EQ(value13, 60);
}

TEST_F(GSettingsHelperTest, SetLockScreenConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetLockScreenConf(KEY_IDLE_DELAY);
    int oldValue = result.toInt();

    bool value = m_pGSettingsHelperDbus->SetLockScreenConf(KEY_IDLE_DELAY, 1);
    m_pGSettingsHelperDbus->SetLockScreenConf(KEY_IDLE_DELAY, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetPowerManagerConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_LOCK_SUSPEND);
    bool value1 = result1.toBool();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_LOCK_SUSPEND);

    QVariant result2 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_LOCK_HIBERNATE);
    bool value2 = result2.toBool();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_LOCK_HIBERNATE);

    QVariant result3 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_LOCK_BLANKSCREEN);
    bool value3 = result3.toBool();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_LOCK_BLANKSCREEN);

    QVariant result4 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_SLEEP_COMPUTER_AC);
    int value4 = result4.toInt();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_SLEEP_COMPUTER_AC);

    QVariant result5 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_SLEEP_DISPLAY_AC);
    int value5 = result5.toInt();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_SLEEP_DISPLAY_AC);

    QVariant result6 = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_BUTTON_LID_AC);
    int value6 = result6.toInt();
    m_pGSettingsHelperDbus->onPowerManagerConfigChanged(KEY_BUTTON_LID_AC);

    ASSERT_EQ(value1, false);
    ASSERT_EQ(value2, false);
    ASSERT_EQ(value3, true);
    ASSERT_EQ(value4, -1);
    ASSERT_EQ(value5, -1);
    ASSERT_EQ(value6, 0);
}

TEST_F(GSettingsHelperTest, SetPowerManagerConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetPowerManagerConf(KEY_LOCK_SUSPEND);
    bool oldValue = result.toBool();

    bool value = m_pGSettingsHelperDbus->SetPowerManagerConf(KEY_LOCK_SUSPEND, false);
    m_pGSettingsHelperDbus->SetPowerManagerConf(KEY_LOCK_SUSPEND, oldValue);
    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetMateBgConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetMateBgConf(KEY_PICTURE_FILENAME);
    QString value1 = result1.toString();
//    m_pGSettingsHelperDbus->onMateBgConfigChanged(KEY_PICTURE_FILENAME);

    QVariant result2 = m_pGSettingsHelperDbus->GetMateBgConf(KEY_PICTURE_OPTIONS);
    QString value2 = result2.toString();
//    m_pGSettingsHelperDbus->onMateBgConfigChanged(KEY_PICTURE_OPTIONS);

    QVariant result3 = m_pGSettingsHelperDbus->GetMateBgConf(KEY_PRIMARY_COLOR);
    QString value3 = result3.toString();
//    m_pGSettingsHelperDbus->onMateBgConfigChanged(KEY_PRIMARY_COLOR);

    ASSERT_EQ(value1.isEmpty(), false);
    ASSERT_EQ(value2.isEmpty(), false);
    ASSERT_EQ(value3.isEmpty(), false);
}

TEST_F(GSettingsHelperTest, SetMateBgConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetMateBgConf(KEY_PICTURE_FILENAME);
    QString oldValue = result.toString();

    bool value = m_pGSettingsHelperDbus->SetMateBgConf(KEY_PICTURE_FILENAME, "ssss");
    m_pGSettingsHelperDbus->SetMateBgConf(KEY_PICTURE_FILENAME, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetUkccPluginsConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetUkccPluginsConf(KEY_HOUR_SYSTEM);
    int value1 = result1.toInt();
    m_pGSettingsHelperDbus->onUkccPluginsConfigChanged(KEY_HOUR_SYSTEM);

    QVariant result2 = m_pGSettingsHelperDbus->GetUkccPluginsConf(KEY_DATE);
    QString value2 = result2.toString();
    m_pGSettingsHelperDbus->onUkccPluginsConfigChanged(KEY_DATE);

    ASSERT_EQ(value1, 24);
    ASSERT_EQ(value2.isEmpty(), false);
}

TEST_F(GSettingsHelperTest, SetUkccPluginsConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetUkccPluginsConf(KEY_HOUR_SYSTEM);
    int oldValue = result.toInt();

    bool value = m_pGSettingsHelperDbus->SetUkccPluginsConf(KEY_HOUR_SYSTEM, 100);
    m_pGSettingsHelperDbus->SetUkccPluginsConf(KEY_HOUR_SYSTEM, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetThemeStyleConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_SYSTEM_FONT_SIZE);
    double value1 = result1.toDouble();
    m_pGSettingsHelperDbus->onThemeStyleConfigChanged(KEY_SYSTEM_FONT_SIZE);

    QVariant result2 = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_THEME_COLOR);
    QString value2 = result2.toString();
    m_pGSettingsHelperDbus->onThemeStyleConfigChanged(KEY_THEME_COLOR);

    QVariant result3 = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_MENU_TRANSPARENCY);
    int value3 = result3.toInt();
    m_pGSettingsHelperDbus->onThemeStyleConfigChanged(KEY_MENU_TRANSPARENCY);

    QVariant result4 = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_STYLE_NAME);
    QString value4 = result4.toString();
    m_pGSettingsHelperDbus->onThemeStyleConfigChanged(KEY_STYLE_NAME);

    QVariant result5 = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_SYSTEM_FONT);
    QString value5 = result5.toString();
    m_pGSettingsHelperDbus->onThemeStyleConfigChanged(KEY_SYSTEM_FONT);

    ASSERT_EQ(value1, 11);
    ASSERT_EQ(value2.isEmpty(), false);
    ASSERT_EQ(value3, 35);
    ASSERT_EQ(value4.isEmpty(), false);
    ASSERT_EQ(value5.isEmpty(), false);
}

TEST_F(GSettingsHelperTest, SetThemeStyleConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetThemeStyleConf(KEY_SYSTEM_FONT_SIZE);
    int oldValue = result.toInt();

    bool value = m_pGSettingsHelperDbus->SetThemeStyleConf(KEY_SYSTEM_FONT_SIZE, 14);
    m_pGSettingsHelperDbus->SetThemeStyleConf(KEY_SYSTEM_FONT_SIZE, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetSessionConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetSessionConf(KEY_SESSION_IDLE);
    int value1 = result1.toInt();
    m_pGSettingsHelperDbus->onSessionConfigChanged(KEY_SESSION_IDLE);

    QVariant result2 = m_pGSettingsHelperDbus->GetSessionConf(KEY_SESSION_LOGOUT_MUSIC);
    bool value2 = result2.toBool();
    m_pGSettingsHelperDbus->onSessionConfigChanged(KEY_SESSION_LOGOUT_MUSIC);

    QVariant result3 = m_pGSettingsHelperDbus->GetSessionConf(KEY_SESSION_POWEROFF_MUSIC);
    bool value3 = result3.toBool();
    m_pGSettingsHelperDbus->onSessionConfigChanged(KEY_SESSION_POWEROFF_MUSIC);

    ASSERT_EQ(value1, 1);
    ASSERT_EQ(value2, false);
    ASSERT_EQ(value3, false);
}

TEST_F(GSettingsHelperTest, SetSessionConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetSessionConf(KEY_SESSION_LOGOUT_MUSIC);
    bool oldValue = result.toBool();

    bool value = m_pGSettingsHelperDbus->SetSessionConf(KEY_SESSION_LOGOUT_MUSIC, false);
    m_pGSettingsHelperDbus->SetSessionConf(KEY_SESSION_LOGOUT_MUSIC, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetKeyboardConf)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetKeyboardConf(KEY_CAPSLOCK_STATUS);
    bool value1 = result1.toBool();
    m_pGSettingsHelperDbus->onKeyboardConfigChanged(KEY_CAPSLOCK_STATUS);
    ASSERT_EQ(value1, false);
}

TEST_F(GSettingsHelperTest, SetKeyboardConf)
{
    QVariant result = m_pGSettingsHelperDbus->GetKeyboardConf(KEY_CAPSLOCK_STATUS);
    bool oldValue = result.toBool();

    bool value = m_pGSettingsHelperDbus->SetKeyboardConf(KEY_CAPSLOCK_STATUS, false);
    m_pGSettingsHelperDbus->SetKeyboardConf(KEY_CAPSLOCK_STATUS, oldValue);

    ASSERT_EQ(value, true);
}

TEST_F(GSettingsHelperTest, GetUsdMediaKeys)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetUsdMediaKeys(KEY_AREA_SCREENSHOT);
    QString value1 = result1.toString();
    m_pGSettingsHelperDbus->onUsdMediaKeysConfigChanged(KEY_AREA_SCREENSHOT);

    QVariant result2 = m_pGSettingsHelperDbus->GetUsdMediaKeys(KEY_AREA_SCREENSHOT2);
    QString value2 = result2.toString();
    m_pGSettingsHelperDbus->onUsdMediaKeysConfigChanged(KEY_AREA_SCREENSHOT2);

    QVariant result3 = m_pGSettingsHelperDbus->GetUsdMediaKeys(KEY_SCREEN_SHOT);
    QString value3 = result3.toString();
    m_pGSettingsHelperDbus->onUsdMediaKeysConfigChanged(KEY_SCREEN_SHOT);

    QVariant result4 = m_pGSettingsHelperDbus->GetUsdMediaKeys(KEY_SCREEN_SHOT2);
    QString value4 = result4.toString();
    m_pGSettingsHelperDbus->onUsdMediaKeysConfigChanged(KEY_SCREEN_SHOT2);

    QVariant result5 = m_pGSettingsHelperDbus->GetUsdMediaKeys(KEY_WINDOW_SCREENSHOT);
    QString value5 = result5.toString();
    m_pGSettingsHelperDbus->onUsdMediaKeysConfigChanged(KEY_WINDOW_SCREENSHOT);

    ASSERT_EQ(value1.isEmpty(), false);
    ASSERT_EQ(value2.isEmpty(), false);
    ASSERT_EQ(value3.isEmpty(), false);
    ASSERT_EQ(value4.isEmpty(), false);
    ASSERT_EQ(value5.isEmpty(), false);
}

TEST_F(GSettingsHelperTest, GetUsdMediaStateKeys)
{
    QVariant result1 = m_pGSettingsHelperDbus->GetUsdMediaStateKeys(KEY_RFKILL_STATE);
    bool value1 = result1.toBool();
    m_pGSettingsHelperDbus->onUsdMediaStateKeysConfigChanged(KEY_RFKILL_STATE);
    ASSERT_EQ(value1, false);
}

TEST_F(GSettingsHelperTest, SetUsdMediaStateKeys)
{
    QVariant result = m_pGSettingsHelperDbus->GetUsdMediaStateKeys(KEY_RFKILL_STATE);
    bool oldValue = result.toBool();

    bool value = m_pGSettingsHelperDbus->SetUsdMediaStateKeys(KEY_RFKILL_STATE, false);
    m_pGSettingsHelperDbus->SetUsdMediaStateKeys(KEY_RFKILL_STATE, oldValue);

    ASSERT_EQ(value, true);
}




