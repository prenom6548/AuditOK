/*
*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "settings-properties-info-test.h"
#include "settings-properties-info.h"

#include <QTest>

void SettingsPropertiesInfoTest::testName()
{
    UkuiNotification::SettingsPropertiesInfo info(UkuiNotification::SettingsProperty::Invalid);
    QCOMPARE(info.name(), "Invalid");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ScheduleTurnOnDND);
    QCOMPARE(info.name(), "ScheduleTurnOnDND");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ScheduleTurnOnDNDTime);
    QCOMPARE(info.name(), "ScheduleTurnOnDNDTime");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ScheduleTurnOffDNDTime);
    QCOMPARE(info.name(), "ScheduleTurnOffDNDTime");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::DNDWhileMultiScreen);
    QCOMPARE(info.name(), "DNDWhileMultiScreen");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::DNDWhileFullScreen);
    QCOMPARE(info.name(), "DNDWhileFullScreen");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::NotifyAlarmWhileDND);
    QCOMPARE(info.name(), "NotifyAlarmWhileDND");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ReceiveNotificationsFromApps);
    QCOMPARE(info.name(), "ReceiveNotificationsFromApps");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::AllowNotify);
    QCOMPARE(info.name(), "AllowNotify");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::AllowSound);
    QCOMPARE(info.name(), "AllowSound");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ShowContentOnLockScreen);
    QCOMPARE(info.name(), "ShowContentOnLockScreen");

    info = UkuiNotification::SettingsPropertiesInfo(UkuiNotification::SettingsProperty::ShowNotificationOnLockScreen);
    QCOMPARE(info.name(), "ShowNotificationOnLockScreen");
}
QTEST_GUILESS_MAIN(SettingsPropertiesInfoTest)
