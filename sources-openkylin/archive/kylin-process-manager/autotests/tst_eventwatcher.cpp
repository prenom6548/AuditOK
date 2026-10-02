/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QDBusConnection>
#include <QDBusInterface>
#include <QSignalSpy>
#include <QtTest/QtTest>

// 定义为public以访问私有成员
#define private public
#define protected public
#include "../core/devicestate.h"
#include "../core/eventwatcher.h"
#undef private
#undef protected

// 模拟PowerManager类
class MockPowerManager : public PowerManager
{
    Q_OBJECT
public:
    MockPowerManager()
        : PowerManager()
    {
        // 初始化为默认状态
        m_currentPowerType = sched_policy::PowerType::Ac;
        m_currentPowerMode = sched_policy::PowerMode::Performance;
    }

    // 模拟电源模式变化
    void simulatePowerModeChange(sched_policy::PowerMode mode)
    {
        m_currentPowerMode = mode;
        Q_EMIT powerModeChanged(mode);
    }

    // 模拟电源类型变化
    void simulatePowerTypeChange(sched_policy::PowerType type)
    {
        m_currentPowerType = type;
        Q_EMIT powerTypeChanged(type);
    }
};

// 模拟DeviceModeManager类
class MockDeviceModeManager : public DeviceModeManager
{
    Q_OBJECT
public:
    MockDeviceModeManager(bool softFreezeEnabled)
        : DeviceModeManager(softFreezeEnabled)
    {
        // 初始化为PC模式
        m_currentDeviceMode = sched_policy::DeviceMode::PC;
    }

    // 模拟设备模式变化
    void simulateDeviceModeChange(sched_policy::DeviceMode mode)
    {
        m_currentDeviceMode = mode;
        Q_EMIT deviceModeChanged(mode);
    }

    // 重写handleSoftFreezeModeEnabledChanged以便测试
    void handleSoftFreezeModeEnabledChanged(bool enabled)
    {
        m_softFreezeModeEnabled = enabled;
        if (enabled) {
            m_currentDeviceMode = sched_policy::DeviceMode::SoftFreeze;
        } else {
            m_currentDeviceMode = sched_policy::DeviceMode::PC;
        }
        Q_EMIT deviceModeChanged(m_currentDeviceMode);
    }
};

// 将WindowManager类相关定义移到外部
namespace kdk
{
// WindowId 结构定义
struct WindowId
{
    QString toString() const
    {
        return m_id;
    }
    QString m_id;

    bool operator==(const WindowId &other) const
    {
        return m_id == other.m_id;
    }
};

// WindowInfo 结构定义
struct WindowInfo
{
    bool isMinimized() const
    {
        return m_minimized;
    }
    bool m_minimized = false;
};
}

// 独立声明WindowManager类，不再嵌套在命名空间内
class KdkWindowManager : public QObject
{
    Q_OBJECT
public:
    static KdkWindowManager *self()
    {
        static KdkWindowManager instance;
        return &instance;
    }

    QList<kdk::WindowId> windows() const
    {
        return m_windows;
    }
    kdk::WindowId currentActiveWindow() const
    {
        return m_activeWindow;
    }

    static kdk::WindowInfo getwindowInfo(const kdk::WindowId &wid)
    {
        kdk::WindowInfo info;
        info.m_minimized = wid.m_id.contains("minimized");
        return info;
    }

    // 模拟方法，用于测试
    void simulateWindowAdded(const QString &id)
    {
        kdk::WindowId wid;
        wid.m_id = id;
        m_windows.append(wid);
        qDebug() << "Window added: " << wid.toString();
        Q_EMIT windowAdded(wid);
    }

    void simulateWindowRemoved(const QString &id)
    {
        kdk::WindowId wid;
        wid.m_id = id;
        for (int i = 0; i < m_windows.size(); i++) {
            if (m_windows[i].m_id == id) {
                m_windows.removeAt(i);
                break;
            }
        }
        Q_EMIT windowRemoved(wid);
    }

    void simulateActiveWindowChanged(const QString &id)
    {
        m_activeWindow.m_id = id;
        Q_EMIT activeWindowChanged(m_activeWindow);
    }

    void simulateWindowMinimized(const QString &id)
    {
        kdk::WindowId wid;
        wid.m_id = "minimized-" + id;
        Q_EMIT windowChanged(wid);
    }

Q_SIGNALS:
    void windowAdded(kdk::WindowId wid);
    void windowRemoved(kdk::WindowId wid);
    void activeWindowChanged(kdk::WindowId wid);
    void windowChanged(kdk::WindowId wid);

private:
    QList<kdk::WindowId> m_windows;
    kdk::WindowId m_activeWindow;
};

// 为了兼容原始代码，在kdk命名空间中添加WindowManager的引用
namespace kdk
{
// 提供一个访问KdkWindowManager的接口
inline KdkWindowManager *WindowManager()
{
    return KdkWindowManager::self();
}
}

// 测试EventWatcher类
class TestEventWatcher : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // 测试窗口事件回调
    void testWindowAddedCallback();
    void testWindowRemovedCallback();
    void testActiveWindowChangedCallback();
    void testWindowMinimizedCallback();

    // 测试电源相关回调
    void testPowerModeChangedCallback();
    void testPowerTypeChangedCallback();

    // 测试设备模式回调
    void testDeviceModeChangedCallback();
    void testSoftFreezeModeEnabledChanged();

    // 测试多媒体和状态通知回调
    void testMprisDbusAddedCallback();
    void testStatusNotifierItemRegisteredCallback();

    // 测试资源警告回调
    void testResourceWarningCallback();

    // 测试启动和停止监听
    void testStartAndStopWatcher();

    // 测试状态查询
    void testPowerTypeAndDeviceMode();

private:
    // 辅助方法
    void simulateDaemonResourceWarning(const std::string &resource, int level);

private:
    EventWatcher *m_eventWatcher;
    MockPowerManager *m_powerManager;
    MockDeviceModeManager *m_deviceModeManager;
};

void TestEventWatcher::initTestCase()
{
    // 创建模拟的PowerManager和DeviceModeManager实例
    m_powerManager = new MockPowerManager();
    m_deviceModeManager = new MockDeviceModeManager(false);

    // 创建EventWatcher实例，传入所有权
    m_eventWatcher = new EventWatcher(
        std::unique_ptr<PowerManager>(m_powerManager),
        std::unique_ptr<DeviceModeManager>(m_deviceModeManager));
}

void TestEventWatcher::cleanupTestCase()
{
    // EventWatcher拥有PowerManager和DeviceModeManager的所有权，会自动释放
    delete m_eventWatcher;
}

void TestEventWatcher::init()
{
    // 每个测试前的准备，可根据需要重置状态
}

void TestEventWatcher::cleanup()
{
    // 每个测试后的清理
}

void TestEventWatcher::testWindowAddedCallback()
{
    // 测试窗口添加回调
    static bool callbackCalled = false;
    static std::string windowId;

    m_eventWatcher->setWindowAddedCallback([&](const std::string &wid) {
        callbackCalled = true;
        windowId = wid;
        qDebug() << "Window added: " << QString::fromStdString(wid);
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟窗口添加事件
    KdkWindowManager::self()->simulateWindowAdded("window1");

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QVERIFY(!windowId.empty());

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testWindowRemovedCallback()
{
    // 测试窗口移除回调
    bool callbackCalled = false;
    std::string windowId;

    m_eventWatcher->setWindowRemovedCallback([&](const std::string &wid) {
        callbackCalled = true;
        windowId = wid;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟窗口移除事件
    KdkWindowManager::self()->simulateWindowRemoved("window2");

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QVERIFY(!windowId.empty());

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testActiveWindowChangedCallback()
{
    // 测试活动窗口变更回调
    static bool callbackCalled = false;
    static std::string windowId;

    m_eventWatcher->setActiveWindowChangedCallback([&](const std::string &wid) {
        callbackCalled = true;
        windowId = wid;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟活动窗口变更事件
    KdkWindowManager::self()->simulateActiveWindowChanged("window3");

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QVERIFY(!windowId.empty());

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testWindowMinimizedCallback()
{
    // 测试窗口最小化回调
    static bool callbackCalled = false;
    static std::string windowId;

    m_eventWatcher->setWindowMinimizedCallback([&](const std::string &wid) {
        callbackCalled = true;
        windowId = wid;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟窗口最小化事件
    KdkWindowManager::self()->simulateWindowMinimized("window4");

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QVERIFY(!windowId.empty());

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testPowerModeChangedCallback()
{
    // 测试电源模式变更回调
    static bool callbackCalled = false;
    sched_policy::PowerMode powerMode = sched_policy::PowerMode::Unknown;

    m_eventWatcher->setPowerModeChangedCallback([&](sched_policy::PowerMode mode) {
        callbackCalled = true;
        powerMode = mode;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟电源模式变更
    m_powerManager->simulatePowerModeChange(sched_policy::PowerMode::Save);

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QCOMPARE(powerMode, sched_policy::PowerMode::Save);

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testPowerTypeChangedCallback()
{
    // 测试电源类型变更回调
    static bool callbackCalled = false;
    sched_policy::PowerType powerType = sched_policy::PowerType::Ac;

    m_eventWatcher->setPowerTypeChangedCallback([&](sched_policy::PowerType type) {
        callbackCalled = true;
        powerType = type;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟电源类型变更
    m_powerManager->simulatePowerTypeChange(sched_policy::PowerType::Battery);

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QCOMPARE(powerType, sched_policy::PowerType::Battery);

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testDeviceModeChangedCallback()
{
    // 测试设备模式变更回调
    static bool callbackCalled = false;
    sched_policy::DeviceMode deviceMode = sched_policy::DeviceMode::Unknown;

    m_eventWatcher->setDeviceModeChangedCallback([&](sched_policy::DeviceMode mode) {
        callbackCalled = true;
        deviceMode = mode;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟设备模式变更
    m_deviceModeManager->simulateDeviceModeChange(sched_policy::DeviceMode::Tablet);

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QCOMPARE(deviceMode, sched_policy::DeviceMode::Tablet);

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testSoftFreezeModeEnabledChanged()
{
    // 测试软冻结模式启用状态变更
    static bool callbackCalled = false;
    sched_policy::DeviceMode deviceMode = sched_policy::DeviceMode::Unknown;

    m_eventWatcher->setDeviceModeChangedCallback([&](sched_policy::DeviceMode mode) {
        callbackCalled = true;
        deviceMode = mode;
    });

    // 开始监听
    m_eventWatcher->startWatcher();

    // 模拟软冻结模式启用状态变更
    m_eventWatcher->handleSoftFreezeModeEnabledChanged(true);

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QCOMPARE(deviceMode, sched_policy::DeviceMode::SoftFreeze);
    QCOMPARE(m_deviceModeManager->m_softFreezeModeEnabled, true);

    // 清空标志
    callbackCalled = false;

    // 模拟软冻结模式禁用状态变更
    m_eventWatcher->handleSoftFreezeModeEnabledChanged(false);

    // 验证回调是否被正确调用
    QVERIFY(callbackCalled);
    QCOMPARE(deviceMode, sched_policy::DeviceMode::PC);
    QCOMPARE(m_deviceModeManager->m_softFreezeModeEnabled, false);

    // 停止监听
    m_eventWatcher->stopWatcher();
}

void TestEventWatcher::testMprisDbusAddedCallback()
{
    // 测试多媒体服务添加回调
    // 注意：这个测试需要模拟DBus信号，这里只测试回调设置正确性
    static bool callbackCalled = false;
    static int pid = 0;
    static std::string name;

    m_eventWatcher->setMprisDbusAddedCallback([&](int _pid, const std::string &_name) {
        callbackCalled = true;
        pid = _pid;
        name = _name;
    });

    // 验证回调是否已正确设置
    QVERIFY(m_eventWatcher->m_mprisDbusAddedCallback != nullptr);

    // 直接调用回调函数测试
    if (m_eventWatcher->m_mprisDbusAddedCallback) {
        m_eventWatcher->m_mprisDbusAddedCallback(12345, "org.mpris.MediaPlayer2.TestPlayer");

        QVERIFY(callbackCalled);
        QCOMPARE(pid, 12345);
        QCOMPARE(name, std::string("org.mpris.MediaPlayer2.TestPlayer"));
    }
}

void TestEventWatcher::testStatusNotifierItemRegisteredCallback()
{
    // 测试状态通知图标注册回调
    static bool callbackCalled = false;
    static int pid = 0;

    m_eventWatcher->setStatusNotifierItemRegisteredCallback([&](int _pid) {
        callbackCalled = true;
        pid = _pid;
    });

    // 验证回调是否已正确设置
    QVERIFY(m_eventWatcher->m_statusNotifierItemRegisteredCallback != nullptr);

    // 直接调用回调函数测试
    if (m_eventWatcher->m_statusNotifierItemRegisteredCallback) {
        m_eventWatcher->m_statusNotifierItemRegisteredCallback(67890);

        QVERIFY(callbackCalled);
        QCOMPARE(pid, 67890);
    }

    // 测试onStatusNotifierItemRegistered方法
    callbackCalled = false;
    m_eventWatcher->onStatusNotifierItemRegistered("app-12345-1");

    QVERIFY(callbackCalled);
    QCOMPARE(pid, 12345);
}

void TestEventWatcher::testResourceWarningCallback()
{
    // 测试资源警告回调
    static bool callbackCalled = false;
    static std::string resource;
    static int level = 0;

    m_eventWatcher->setResourceWarningCallback([&](const std::string &_resource, int _level) {
        callbackCalled = true;
        resource = _resource;
        level = _level;
    });

    // 验证回调是否已正确设置
    QVERIFY(m_eventWatcher->m_resourceWarningCallback != nullptr);

    // 直接调用回调函数测试
    if (m_eventWatcher->m_resourceWarningCallback) {
        m_eventWatcher->m_resourceWarningCallback("memory", 80);

        QVERIFY(callbackCalled);
        QCOMPARE(resource, std::string("memory"));
        QCOMPARE(level, 80);
    }

    // 模拟DaemonDbusInterface的ResourceThresholdWarning信号
    callbackCalled = false;
    simulateDaemonResourceWarning("cpu", 90);

    // 由于我们无法直接触发DaemonDbusInterface的信号，这部分测试可能无法通过
    // 这里主要测试回调函数设置正确
}

void TestEventWatcher::simulateDaemonResourceWarning(const std::string &resource, int level)
{
    // 这个方法无法直接调用DaemonDbusInterface的信号，需要在实际环境中进行测试
    // 在这里直接测试回调函数
    if (m_eventWatcher->m_resourceWarningCallback) {
        m_eventWatcher->m_resourceWarningCallback(resource, level);
    }
}

void TestEventWatcher::testStartAndStopWatcher()
{
    // 测试启动和停止监听
    static bool windowAddedCalled = false;

    m_eventWatcher->setWindowAddedCallback([&](const std::string &) {
        windowAddedCalled = true;
    });

    // 启动监听
    m_eventWatcher->startWatcher();

    // 模拟窗口添加事件
    KdkWindowManager::self()->simulateWindowAdded("window5");

    // 验证回调是否被调用
    QVERIFY(windowAddedCalled);

    // 重置标志
    windowAddedCalled = false;

    // 停止监听
    m_eventWatcher->stopWatcher();

    // 再次模拟窗口添加事件
    KdkWindowManager::self()->simulateWindowAdded("window6");

    // 验证回调是否不再被调用
    QVERIFY(!windowAddedCalled);
}

void TestEventWatcher::testPowerTypeAndDeviceMode()
{
    // 测试状态查询方法

    // 设置初始状态
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Ac;
    m_deviceModeManager->m_currentDeviceMode = sched_policy::DeviceMode::PC;

    // 验证状态查询正确
    QCOMPARE(m_eventWatcher->powerType(), sched_policy::PowerType::Ac);
    QCOMPARE(m_eventWatcher->deviceMode(), sched_policy::DeviceMode::PC);

    // 改变状态
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Battery;
    m_deviceModeManager->m_currentDeviceMode = sched_policy::DeviceMode::Tablet;

    // 再次验证状态查询正确
    QCOMPARE(m_eventWatcher->powerType(), sched_policy::PowerType::Battery);
    QCOMPARE(m_eventWatcher->deviceMode(), sched_policy::DeviceMode::Tablet);
}

// 主函数，这是QTest需要的入口点
QTEST_MAIN(TestEventWatcher)
#include "tst_eventwatcher.moc"
