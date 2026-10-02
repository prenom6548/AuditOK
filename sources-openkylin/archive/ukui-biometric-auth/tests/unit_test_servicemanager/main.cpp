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
#include "../../uniauth-backend/src/servicemanager.h"
#include <QByteArray>
#include <QDataStream>

#define SERVICE "biometric-authentication.service"
#define DBUS_SERVICE "org.ukui.Biometric"
#define DBUS_PATH "/org/ukui/Biometric"
#define DBUS_INTERFACE "org.ukui.Biometric"

#define FD_DBUS_SERVICE     "org.freedesktop.DBus"
#define FD_DBUS_PATH        "/org/freedesktop/DBus"
#define FD_DBUS_INTERFACE   "org.freedesktop.DBus"
 
// gtest 测试用例
TEST(ServiceManagerTest, SerializationAndDeserialization) {
    ServiceManager *sm = ServiceManager::instance();

    sm->connectToService();
    
    sm->serviceStatusChanged(true);
    sm->serviceStatusChanged(false);

    sm->dbusNameOwnerChanged(DBUS_SERVICE,DBUS_SERVICE,DBUS_SERVICE);

    sm->onDBusNameOwnerChanged(DBUS_SERVICE,DBUS_SERVICE,DBUS_SERVICE);
    
    sm->serviceExists();
 
}
 
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
