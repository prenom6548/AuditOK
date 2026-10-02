/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <gtest/gtest.h>
#include "../../battery/libkybattery.h"
#include <dbus-1.0/dbus/dbus.h>
#include "../cpp-stub/src/stub.h"
#include "dbus/dbus-glib.h"
using namespace testing;

int g_value_get_uint_stub1(const GValue *value)
{
    return 1;
}

int g_value_get_uint_stub2(const GValue *value)
{
    return 2;
}

int g_value_get_uint_stub3(const GValue *value)
{
    return 3;
}

int g_value_get_uint_stub4(const GValue *value)
{
    return 4;
}

int g_value_get_uint_stub5(const GValue *value)
{
    return 5;
}

int g_value_get_uint_stub6(const GValue *value)
{
    return 6;
}

// 测试 kdk_get_disklist
TEST(KdkBatteryTest, BatteryInfo) {
    float percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    Stub st;
    st.set(g_value_get_uint, g_value_get_uint_stub1);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    st.set(g_value_get_uint, g_value_get_uint_stub2);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    st.set(g_value_get_uint, g_value_get_uint_stub3);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    st.set(g_value_get_uint, g_value_get_uint_stub4);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    st.set(g_value_get_uint, g_value_get_uint_stub5);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    st.set(g_value_get_uint, g_value_get_uint_stub6);
    percentage = kdk_battery_get_soc();
    //EXPECT_EQ(percentage, 0);

    BatteryChargeState state = kdk_battery_get_charge_state();
    //EXPECT_EQ(state, 0);

    BatteryHealthState health_state = kdk_battery_get_health_state();
    printf("health state = %d\n", health_state);
    //EXPECT_EQ(health_state, 0);

    BatteryPluggedType type = kdk_battery_get_plugged_type();
    printf("type = %d\n", type);
    //EXPECT_EQ(type, 0);

    float voltage = kdk_battery_get_voltage();
    printf("voltage = %0.3f\n", voltage);
    //EXPECT_EQ(voltage, 0);

    char* model = kdk_battery_get_technology();
    printf("model = %s\n", model);
    //EXPECT_EQ(model, nullptr);

    float temperature = kdk_battery_get_temperature();
    printf("temperature = %0.1f\n", temperature);
    //EXPECT_EQ(temperature, 0);

    bool is_present = kdk_battery_is_present();
    printf("is_present : %s\n", is_present ? "true" : "false");
    //EXPECT_EQ(is_present, 0);

    int level = kdk_battery_get_capacity_level();
    printf("level = %d\n", level);
    //EXPECT_EQ(level, 0);

}
