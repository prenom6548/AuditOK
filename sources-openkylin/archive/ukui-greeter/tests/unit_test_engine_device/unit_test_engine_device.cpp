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
#include "../../ukui-greeter/greeter/device.h"
#include "../../ukui-greeter/greeter/enginedevice.h"
#include <gtest/gtest.h>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QVariant>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QGSettings>
#include <QDebug>
#include <QCoreApplication>

#include "stubext.h"
using namespace stub_ext;

class EngineDeviceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pEngineDeviceDbus = new EngineDevice();
    }

    static void TearDownTestSuite()
    {
        delete m_pEngineDeviceDbus;
        m_pEngineDeviceDbus = nullptr;
    }

    static EngineDevice *m_pEngineDeviceDbus;
};

EngineDevice *EngineDeviceTest::m_pEngineDeviceDbus = nullptr;


// 测试 power_device_get_devices 函数
TEST_F(EngineDeviceTest, power_device_get_devices) {
    // 首先，确保 QDBus 服务正常运行，这里假设 QDBus 服务可以正常返回一些测试数据
    m_pEngineDeviceDbus->power_device_get_devices();


    // 检查 devices 列表是否不为空
    EXPECT_GT(m_pEngineDeviceDbus->devices.size(), 0);


    // 可以进一步检查 devices 列表中元素的属性，例如：
    if (!m_pEngineDeviceDbus->devices.isEmpty()) {
        DEVICE *device = m_pEngineDeviceDbus->devices.first();
        EXPECT_FALSE(device->m_dev.path.isEmpty());
    }
}


// 测试 getProperty 函数
TEST_F(EngineDeviceTest, getProperty) {
    DEV dev;
    QString testPath = "/test/path";


    // 这里假设 QDBus 服务会正常响应
    m_pEngineDeviceDbus->getProperty(testPath, dev);


    // 检查返回的属性是否符合预期
    EXPECT_EQ(dev.kind, UP_DEVICE_KIND_UNKNOWN);  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.Type, "");  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.Model, "");  // 根据实际情况修改期望的结果
    EXPECT_FALSE(dev.IsPresent);  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.PowerSupply, "");  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.Percentage, 0.0);  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.Online, "");  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.State, UP_DEVICE_STATE_UNKNOWN);  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.TimeToEmpty, 0);  // 根据实际情况修改期望的结果
    EXPECT_EQ(dev.TimeToFull, 0);  // 根据实际情况修改期望的结果
}


// 测试 boolToString 函数
TEST_F(EngineDeviceTest, boolToString) {
    EXPECT_EQ(m_pEngineDeviceDbus->boolToString(true), "yes");
    EXPECT_EQ(m_pEngineDeviceDbus->boolToString(false), "no");
}


// 测试 putAttributes 函数
TEST_F(EngineDeviceTest, putAttributes) {
    QMap<QString, QVariant> map;
    map["TimeToFull"] = QVariant(200);
    map["TimeToEmpty"] = QVariant(100);
    map["State"] = QVariant(2);
    map["Percentage"] = QVariant(50.0);
    map["PowerSupply"] = QVariant(true);
    map["IsPresent"] = QVariant(true);


    DEV btrDetailData;
    m_pEngineDeviceDbus->putAttributes(map, btrDetailData);


    EXPECT_EQ(btrDetailData.TimeToFull, 200);
    EXPECT_EQ(btrDetailData.TimeToEmpty, 100);
    EXPECT_EQ(btrDetailData.State, 2);
    EXPECT_EQ(btrDetailData.Percentage, 5.0);
    EXPECT_EQ(btrDetailData.PowerSupply, "Yes");
    EXPECT_EQ(btrDetailData.IsPresent, true);
}


// 测试 power_device_change_callback 函数
TEST_F(EngineDeviceTest, power_device_change_callback) {
//    DEVICE *device = new DEVICE;
//    device->m_dev.path = "/test/path";
//    engineDevice->devices.append(device);
//    QDBusMessage msg = QDBusMessage::createMethodCall(DBUS_SERVICE, "/test/path", DBUS_INTERFACE_PRO, "PropertiesChanged");
//    msg.setArguments({ QVariant("interface"), QVariant::fromValue(QDBusArgument::fromVariantMap({
//        { "TimeToFull", QVariant(300) }
//    })) });
//    engineDevice->power_device_change_callback(msg, "/test/path");
//    EXPECT_EQ(device->m_dev.TimeToFull, 300);
//    delete device;
}


// 测试 power_device_recalculate_state 函数
TEST_F(EngineDeviceTest, power_device_recalculate_state) {
    // 准备一些测试数据
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    m_pEngineDeviceDbus->devices.append(device);


    // 使用 QSignalSpy 监听信号
//    QSignalSpy summarySpy(m_pEngineDeviceDbus->get(), SIGNAL(engine_signal_summary_change(QString)));
//    QSignalSpy stateSpy(m_pEngineDeviceDbus->get(), SIGNAL(engine_signal_Battery_State(QStringList)));


    m_pEngineDeviceDbus->power_device_recalculate_state();


    // 检查信号是否触发
//    EXPECT_EQ(summarySpy.count(), 1);
//    EXPECT_EQ(stateSpy.count(), 1);


    delete device;
}


// 测试 engine_recalculate_summary 函数
TEST_F(EngineDeviceTest, engine_recalculate_summary) {
    // 准备一些测试数据
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    m_pEngineDeviceDbus->devices.append(device);


    // 使用 QSignalSpy 监听信号
//    QSignalSpy summarySpy(m_pEngineDeviceDbus->get(), SIGNAL(engine_signal_summary_change(QString)));
//    QSignalSpy stateSpy(m_pEngineDeviceDbus->get(), SIGNAL(engine_signal_Battery_State(QStringList)));


    bool result = m_pEngineDeviceDbus->engine_recalculate_summary();


    // 检查函数返回值和信号触发情况
    EXPECT_TRUE(result);
//    EXPECT_EQ(summarySpy.count(), 1);
//    EXPECT_EQ(stateSpy.count(), 1);


    delete device;
}


// 测试 engine_get_state 函数
TEST_F(EngineDeviceTest, engine_get_state) {
    // 准备一些测试数据
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    m_pEngineDeviceDbus->devices.append(device);
    QStringList stateList = m_pEngineDeviceDbus->engine_get_state();
    // 检查返回的状态列表是否符合预期
    EXPECT_FALSE(stateList.isEmpty());
    delete device;
}


// 测试 engine_get_Battery_State 函数
TEST_F(EngineDeviceTest, engine_get_Battery_State) {
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    device->m_dev.Percentage = 50.0;
    device->m_dev.TimeToEmpty = 100;
    QStringList stateList = m_pEngineDeviceDbus->engine_get_Battery_State(device);
    // 检查返回的状态列表元素是否符合预期
    EXPECT_EQ(stateList.size(), 3);
    EXPECT_EQ(stateList[0], "50");
    EXPECT_EQ(stateList[1], QString::number(UP_DEVICE_STATE_CHARGING));
    EXPECT_EQ(stateList[2], "100");


    delete device;
}


// 测试 engine_get_summary 函数
TEST_F(EngineDeviceTest, engine_get_summary) {
    // 准备一些测试数据
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    device->m_dev.Percentage = 50.0;
    device->m_dev.TimeToEmpty = 100;
    m_pEngineDeviceDbus->devices.append(device);
    QString summary = m_pEngineDeviceDbus->engine_get_summary();
    // 检查返回的摘要是否符合预期
    EXPECT_FALSE(summary.isEmpty());


    delete device;
}


// 测试 engine_get_device_summary 函数
TEST_F(EngineDeviceTest, engine_get_device_summary) {
    // 准备一些测试数据
    DEVICE *device = new DEVICE;
    device->m_dev.IsPresent = true;
    device->m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device->m_dev.State = UP_DEVICE_STATE_CHARGING;
    device->m_dev.Percentage = 50.0;
    device->m_dev.TimeToEmpty = 100;
    QString summary = m_pEngineDeviceDbus->engine_get_device_summary(device);
    // 检查返回的摘要是否符合预期
    EXPECT_FALSE(summary.isEmpty());


    delete device;
}


// 测试 engine_kind_to_localised_text 函数
TEST_F(EngineDeviceTest, engine_kind_to_localised_text) {
    EXPECT_EQ(m_pEngineDeviceDbus->engine_kind_to_localised_text(UP_DEVICE_KIND_LINE_POWER, 0), "AC adapter");
    EXPECT_EQ(m_pEngineDeviceDbus->engine_kind_to_localised_text(UP_DEVICE_KIND_BATTERY, 0), "Laptop battery");
    // 测试其他枚举值
}
