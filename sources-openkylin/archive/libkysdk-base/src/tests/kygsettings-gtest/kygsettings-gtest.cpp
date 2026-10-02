/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include "../cpp_stub/src/stub.h"
#include "../../gsettings/libkygsetting.h"
#include "glib.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

TEST(TestSuite1, kdk_settings_reset_1)
{
	int ret = kdk_settings_reset(NULL, NULL);
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite2, kdk_settings_reset_2)
{
    int ret = kdk_settings_reset("org.gnome.evolution-data-server.calendar", "notify-custom-snooze-minutes");
    EXPECT_EQ(ret, 0);
}

TEST(TestSuite3, kdk_settings_reset_3)
{
    int ret = kdk_settings_reset("org.ukui.control-center.notice", "enable-notice");
    EXPECT_EQ(ret, 1);
}

TEST(TestSuite4, kdk_gsettings_set_1)
{
	double a[] = {0.0,2.0};  
	size_t s = 2;
	int ret = kdk_gsettings_set(NULL, NULL, "ad", a, s);
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite5, kdk_gsettings_set_2)
{
    double a[] = {0.0,2.0};  
	size_t s = 2;
	
	int ret = kdk_gsettings_set("org.onboard.window", "docking-aspect-change-range", "ad", a, s);
	EXPECT_EQ(ret, 1);
}

// TEST(TestSuite6, kdk_gsettings_set_3)
// {
//     char* a[] = {"2dfew", "wfwgrte"};  
// 	size_t s = 2;
	
// 	int ret = kdk_gsettings_set("org.onboard.window", "docking-aspect-change-range", "ad", a, s);
// 	EXPECT_EQ(ret, 1);
// }

// TEST(TestSuite7, kdk_gsettings_set_4)
// {
//     int a[] = {0,2};  
// 	size_t s = 2;
	
// 	int ret = kdk_gsettings_set("org.onboard.window", "docking-aspect-change-range", "ad", a, s);	//当类型为数组时，请输入数组大小，size_t
// 	EXPECT_EQ(ret, 1);
// }

TEST(TestSuite8, kdk_settings_set_string_1)
{
	int ret = kdk_settings_set_string(NULL, "version", "3.36.1");
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite9, kdk_settings_set_string_2)
{	
	int ret = kdk_settings_set_string("org.ukui.control-center.apt.proxy", "xxx", "");
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite10, kdk_settings_set_string_3)
{	
	int ret = kdk_settings_set_string("org.ukui.control-center.apt.proxy", "host", "");
	EXPECT_EQ(ret, 1);
}

TEST(TestSuite11, kdk_settings_set_int_1)
{
	int ret = kdk_settings_set_int(NULL, "output-volume", 67);
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite12, kdk_settings_set_int_2)
{
	int ret = kdk_settings_set_int("org.ukui.audio", "output-volume", 67);
	EXPECT_EQ(ret, 0);
}

TEST(TestSuite13, kdk_settings_set_int_3)
{
	int ret = kdk_settings_set_int("org.ukui.control-center.apt.proxy", "port", 8081);
	EXPECT_EQ(ret, 1);
}

TEST(TestSuite14, kdk_settings_get_string_1)
{
	char* res = kdk_settings_get_string(NULL, "version");
	EXPECT_FALSE(res != NULL);
}

TEST(TestSuite14, kdk_settings_get_string_2)
{
	char* res = kdk_settings_get_string("org.gnome.evolution", "version");
	EXPECT_FALSE(res != NULL);
}

TEST(TestSuite15, kdk_settings_get_string_3)
{
	char* res = kdk_settings_get_string("org.ukui.control-center.apt.proxy", "host");
	EXPECT_FALSE(res == NULL);
}

TEST(TestSuite16, kdk_settings_get_int_1)
{
	int key = kdk_settings_get_int(NULL, "output-volume");
}

TEST(TestSuite17, kdk_settings_get_int_2)
{
	int key = kdk_settings_get_int("org.ukui.audio", "output-volume");
}

TEST(TestSuite18, kdk_settings_get_int_3)
{
	int key = kdk_settings_get_int("org.ukui.control-center.apt.proxy", "port");
}

TEST(TestSuite19, kdk_gsettings_get_1)
{
	bool enabled;
	GVariantIter iter;
	kdk_gsettings_get(NULL, "blacklist", "as", &iter);
}

TEST(TestSuite20, kdk_gsettings_get_2)
{
	bool enabled;
	GVariantIter iter;
	kdk_gsettings_get("org.ukui.control-center.notice", "xxxx", "as", &iter);
}

TEST(TestSuite21, kdk_gsettings_get_3)
{
	bool enabled;
	GVariantIter iter;
	kdk_gsettings_get("org.ukui.control-center.notice", "blacklist", "as", &iter);
}

TEST(TestSuite22, kdk_settings_get_double_1)
{
	double dde = kdk_settings_get_double(NULL, "scaling-factor");
}

TEST(TestSuite23, kdk_settings_get_double_2)
{
	double dde = kdk_settings_get_double("org.ukui.SettingsDaemon.plugins.xsettings", "xxxx");
}

TEST(TestSuite24, kdk_settings_get_double_3)
{
	double dde = kdk_settings_get_double("org.ukui.SettingsDaemon.plugins.xsettings", "scaling-factor");
}