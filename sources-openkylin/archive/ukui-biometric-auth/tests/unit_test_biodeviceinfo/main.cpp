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
#include "../../uniauth-backend/src/biodeviceinfo.h"
#include <QByteArray>
#include <QDataStream>
 
// gtest 测试用例
TEST(BioDeviceInfoTest, SerializationAndDeserialization) {
    registerCustomTypes();

    DeviceInfo original;
    original.device_id = 1;
    original.device_shortname = "TestDevice";
    original.device_fullname = "Test Device Full Name";
    original.driver_enable = 1;
    original.device_available = 1;
    original.biotype = BIOTYPE_FINGERPRINT;
    original.stotype = 0;
    original.eigtype = 0;
    original.vertype = 0;
    original.idtype = 0;
    original.bustype = 0;
    original.dev_status = 0;
    original.ops_status = 0;
 
    QDBusArgument argument;
    argument<<original;

    DeviceInfo deserialized;
    argument >> deserialized;
 
    EXPECT_EQ(original.device_id, deserialized.device_id);
    EXPECT_EQ(original.device_shortname, deserialized.device_shortname);
    EXPECT_EQ(original.device_fullname, deserialized.device_fullname);
    EXPECT_EQ(original.driver_enable, deserialized.driver_enable);
    EXPECT_EQ(original.device_available, deserialized.device_available);
    EXPECT_EQ(original.biotype, deserialized.biotype);
    EXPECT_EQ(original.stotype, deserialized.stotype);
    EXPECT_EQ(original.eigtype, deserialized.eigtype);
    EXPECT_EQ(original.vertype, deserialized.vertype);
    EXPECT_EQ(original.idtype, deserialized.idtype);
    EXPECT_EQ(original.bustype, deserialized.bustype);
    EXPECT_EQ(original.dev_status, deserialized.dev_status);
    EXPECT_EQ(original.ops_status, deserialized.ops_status);
}
 
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
