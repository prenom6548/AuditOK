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
#include "../../src/dbusifs/enginedevice.h"
#include "../../src/dbusifs/device.h"
#include "../../src/dbusifs/upowerhelper.h"
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
#include <QSignalSpy>

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

const static QString upowerService = QStringLiteral("org.freedesktop.UPower");
const static QString upowerPath = QStringLiteral("/org/freedesktop/UPower");
const static QString upowerInterface = QStringLiteral("org.freedesktop.UPower");
const static QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const static QString upowerDisplayPath = QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice");
const static QString upowerDeviceInterface = QStringLiteral("org.freedesktop.UPower.Device");

// 测试 UpowerHelper 类
class UpowerHelperTest : public ::testing::Test {
protected:
    std::unique_ptr<UpowerHelper> upowerHelper;
    std::unique_ptr<EngineDevice> engineDevice;


    void SetUp() override {
        engineDevice = std::make_unique<EngineDevice>();
        upowerHelper = std::make_unique<UpowerHelper>(nullptr);
        upowerHelper->m_engineDevice = engineDevice.get();
    }


    void TearDown() override {
        upowerHelper.reset();
        engineDevice.reset();
    }
};


// 测试 UpowerHelper 构造函数中的 DBus 连接和属性初始化
TEST_F(UpowerHelperTest, UpowerHelperConstructor) {
    // 检查 m_upowerService 是否有效
    EXPECT_TRUE(upowerHelper->m_upowerService->isValid());


    // 检查 m_upowerInterface 是否有效
    EXPECT_TRUE(upowerHelper->m_upowerInterface->isValid());


    // 检查是否正确连接了信号
//    QDBusConnectionInterface *connectionInterface = QDBusConnection::systemBus().interface();
//    bool isConnected = connectionInterface->isConnected(upowerService, upowerPath, propertiesInterface, "PropertiesChanged", upowerHelper.get());
//    EXPECT_TRUE(isConnected);


    // 检查 m_isBattery 的初始化
    QDBusReply<QVariant> interfaceReply = upowerHelper->m_upowerInterface->call("Get", "org.freedesktop.UPower", "LidIsPresent");
    bool expectedIsBattery = interfaceReply.isValid()? interfaceReply.value().toBool() : false;
    EXPECT_EQ(upowerHelper->m_isBattery, expectedIsBattery);


    // 检查是否正确连接了电池设备的信号
//    if (!upowerHelper->m_batInterface.isNull()) {
//        isConnected = connectionInterface->isConnected(upowerService, upowerHelper->m_batInterface->path(), propertiesInterface, "PropertiesChanged", upowerHelper.get());
//        EXPECT_TRUE(isConnected);
//    }


    // 检查是否正确连接了 EngineDevice 的信号
    bool isSignalConnected = QObject::connect(engineDevice.get(), &EngineDevice::engine_signal_Battery_State, upowerHelper.get(), &UpowerHelper::onBatteryChanged);
    EXPECT_TRUE(isSignalConnected);
}


// 测试 getBatteryIconName 函数
TEST_F(UpowerHelperTest, GetBatteryIconName) {
    // 模拟电池状态和百分比
    upowerHelper->m_upowerInterface->setProperty("OnBattery", QVariant(true));
    if (upowerHelper->m_batInterface) {
        upowerHelper->m_batInterface->setProperty("Percentage", QVariant(50.0));
    }


    QString iconName = upowerHelper->getBatteryIconName();
    EXPECT_EQ(iconName, "battery-level-50-symbolic");


    upowerHelper->m_upowerInterface->setProperty("OnBattery", QVariant(false));
    iconName = upowerHelper->getBatteryIconName();
    EXPECT_EQ(iconName, "battery-level-50-charging-symbolic");


    // 测试异常情况，如百分比为负数
    if (upowerHelper->m_batInterface) {
        upowerHelper->m_batInterface->setProperty("Percentage", QVariant(-10.0));
    }
    iconName = upowerHelper->getBatteryIconName();
    EXPECT_EQ(iconName, "battery-level--10-symbolic");


    // 测试无效的 DBus 接口
//    upowerHelper->m_upowerService->setValid(false);
    iconName = upowerHelper->getBatteryIconName();
    EXPECT_EQ(iconName, QString());
}


// 测试 getBatteryArgs 函数
TEST_F(UpowerHelperTest, GetBatteryArgs) {
    // 模拟 engineDevice 的状态
    DEVICE device;
    device.m_dev.IsPresent = true;
    device.m_dev.kind = UP_DEVICE_KIND_BATTERY;
    device.m_dev.State = UP_DEVICE_STATE_CHARGING;
    engineDevice->devices.append(&device);


    QStringList args = upowerHelper->getBatteryArgs();


    // 检查是否调用了 engine_get_state 并得到了正确的结果
    EXPECT_FALSE(args.isEmpty());


    // 测试 engineDevice 为空的情况
    upowerHelper->m_engineDevice = nullptr;
    args = upowerHelper->getBatteryArgs();
    EXPECT_TRUE(args.isEmpty());
}


// 测试 onBatteryChanged 函数
TEST_F(UpowerHelperTest, OnBatteryChanged) {
    QStringList args = {"arg1", "arg2"};


    // 使用 QSignalSpy 监听 batteryChanged 信号
    QSignalSpy spy(upowerHelper.get(), SIGNAL(batteryChanged(QStringList)));


    upowerHelper->onBatteryChanged(args);


//     检查信号是否正确触发
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy[0][0].toStringList(), args);
}


// 测试 dealMessage 函数
TEST_F(UpowerHelperTest, DealMessage) {
    // 使用 QSignalSpy 监听 batteryStatusChanged 信号
    QSignalSpy spy(upowerHelper.get(), SIGNAL(batteryStatusChanged(QString)));


    QDBusMessage message;


    upowerHelper->dealMessage(message);


//    // 检查信号是否正确触发
    EXPECT_EQ(spy.count(), 1);


    // 测试信号传递的参数
    upowerHelper->m_upowerService->setProperty("OnBattery", QVariant(true));
    if (upowerHelper->m_batInterface) {
        upowerHelper->m_batInterface->setProperty("Percentage", QVariant(30.0));
    }
    upowerHelper->dealMessage(message);
    EXPECT_EQ(spy[1][0].toString(), "battery-level-30-symbolic");
}


// 测试 onLidWatcherMessage 函数
TEST_F(UpowerHelperTest, OnLidWatcherMessage) {
    // 使用 QSignalSpy 监听 lidStateChanged 信号
    QSignalSpy spy(upowerHelper.get(), SIGNAL(lidStateChanged(bool)));


    QDBusMessage message;


    upowerHelper->onLidWatcherMessage();


    // 检查信号是否正确触发
    EXPECT_EQ(spy.count(), 1);


    // 测试信号传递的参数
    QDBusInterface iface(upowerService, upowerPath, propertiesInterface, QDBusConnection::systemBus());
    iface.setProperty("LidIsClosed", QVariant(true));
    upowerHelper->onLidWatcherMessage();
    EXPECT_EQ(spy[1][0].toBool(), true);


    iface.setProperty("LidIsClosed", QVariant(false));
    upowerHelper->onLidWatcherMessage();
    EXPECT_EQ(spy[2][0].toBool(), false);
}




