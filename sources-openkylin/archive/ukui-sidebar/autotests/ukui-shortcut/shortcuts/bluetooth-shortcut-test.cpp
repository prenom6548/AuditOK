/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include <QtTest>

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define BLUETOOTH_SERVICE_NAME         "com.ukui.bluetooth.autotest"
#define BLUETOOTH_SERVICE_PATH         "/com/ukui/bluetooth/autotest"
#define BLUETOOTH_SERVICE_INTERFACE    "com.ukui.bluetooth.autotest"

class BluetoothConnector : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothConnector(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void start()
    {
    }

Q_SIGNALS:
    void ready(QDBusInterface *);
    void failed();
};

#define BLUETOOTHCONNECTOR_H
#define private public
#include "../../../ukui-shortcut/shortcuts/bluetooth-shortcut/bluetooth-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kBluetoothTestService[] = "com.ukui.bluetooth.autotest";
const char kBluetoothTestPath[] = "/com/ukui/bluetooth/autotest";
const char kBluetoothTestInterface[] = "com.ukui.bluetooth.autotest";

class FakeBluetoothService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.ukui.bluetooth.autotest")
public:
    QStringList adapters {QStringLiteral("adapter0")};
    QHash<QString, bool> adapterPower {{QStringLiteral("adapter0"), true}};
    QStringList pairedDevices {QStringLiteral("device0")};
    QHash<QString, QVariantMap> deviceAttrs {
        {QStringLiteral("device0"), QVariantMap{{QStringLiteral("Connected"), true},
                                                {QStringLiteral("Name"), QStringLiteral("Keyboard")}}}
    };
    QString lastMethod;
    QVariantMap lastAttr;
    int setDefaultAdapterAttrCount = 0;
    int showTrayWidgetCount = 0;
    int postBluetoothPowerCount = 0;
    bool lastPostBluetoothPower = false;
    QDBusError::ErrorType postBluetoothPowerError = QDBusError::NoError;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(kBluetoothTestPath);
        bus.unregisterService(kBluetoothTestService);
        if (!bus.registerService(kBluetoothTestService)) {
            return false;
        }

        registered = bus.registerObject(kBluetoothTestPath, this, QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(kBluetoothTestPath);
        bus.unregisterService(kBluetoothTestService);
        registered = false;
    }

    void reset()
    {
        adapters = QStringList{QStringLiteral("adapter0")};
        adapterPower = {{QStringLiteral("adapter0"), true}};
        pairedDevices = QStringList{QStringLiteral("device0")};
        deviceAttrs = {{QStringLiteral("device0"), QVariantMap{{QStringLiteral("Connected"), true},
                                                               {QStringLiteral("Name"), QStringLiteral("Keyboard")}}}};
        lastMethod.clear();
        lastAttr.clear();
        setDefaultAdapterAttrCount = 0;
        showTrayWidgetCount = 0;
        postBluetoothPowerCount = 0;
        lastPostBluetoothPower = false;
        postBluetoothPowerError = QDBusError::NoError;
    }

public Q_SLOTS:
    QStringList getAllAdapterAddress()
    {
        return adapters;
    }

    QVariantMap getAdapterAttr(const QString &id, const QString &attr)
    {
        Q_UNUSED(attr)
        return {{QStringLiteral("Powered"), adapterPower.value(id, false)}};
    }

    QStringList getDefaultAdapterPairedDev()
    {
        return pairedDevices;
    }

    QVariantMap getDevAttr(const QString &address)
    {
        return deviceAttrs.value(address);
    }

    bool setDefaultAdapterAttr(const QVariantMap &param)
    {
        lastMethod = QStringLiteral("setDefaultAdapterAttr");
        lastAttr = param;
        ++setDefaultAdapterAttrCount;
        return true;
    }

    void postBluetoothPower(bool power, const QDBusMessage &message)
    {
        message.setDelayedReply(true);
        lastMethod = QStringLiteral("postBluetoothPower");
        lastPostBluetoothPower = power;
        ++postBluetoothPowerCount;
        const auto bus = QDBusConnection::sessionBus();
        if (postBluetoothPowerError != QDBusError::NoError) {
            bus.send(message.createErrorReply(postBluetoothPowerError, QStringLiteral("test error")));
        } else {
            bus.send(message.createReply(true));
        }
    }

    bool showTrayWidgetUI()
    {
        lastMethod = QStringLiteral("showTrayWidgetUI");
        ++showTrayWidgetCount;
        return true;
    }
};

class BluetoothShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialState();
    void test_connectionFailedDisablesShortcut();
    void test_initWithNullInterfaceKeepsSafeState();
    void test_initWithValidInterfaceReadsPoweredConnectedDevice();
    void test_onAdapterChangedMarksDisabledWithoutAdapters();
    void test_onAdapterChangedUsesClosedStateWhenAdapterPoweredOff();
    void test_onDeviceChangedReportsNoConnectionWhenPowered();
    void test_onDeviceChangedReportsClosedWhenNotPowered();
    void test_onDeviceChangedJoinsConnectedDeviceNames();
    void test_setAddedStateFalseDisconnectsAndEmitsClosedStatus();
    void test_setAddedStateTrueRefreshesStatus();
    void test_activeClickTogglesDefaultAdapterPower();
    void test_activeMenuRequestShowsTrayWidget();
    void test_activeIgnoresUnsupportedActionsAndNullInterface();
    void test_clickDebouncesWhilePowerCallPending();
    void test_callLegacySetBluetoothPowerCallsSetDefaultAdapterAttr();
    void test_callLegacySetBluetoothPowerSkipsNullInterface();
    void test_callLegacySetBluetoothPowerSkipsInvalidInterface();
    void test_clickFallsBackToLegacyWhenPowerCallFails();
    void test_connectAndDisconnectSignalFailureBranches();

private:
    int removeGeneratedGcovFiles() const;
    QDBusInterface *newInterface(QObject *parent = nullptr) const;

private:
    FakeBluetoothService m_service;
};

} // namespace

void BluetoothShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void BluetoothShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void BluetoothShortcutTest::cleanup()
{
    m_service.reset();
    removeGeneratedGcovFiles();
}

int BluetoothShortcutTest::removeGeneratedGcovFiles() const
{
    int removed = 0;
    QDir dir(QDir::currentPath());
    const QFileInfoList files = dir.entryInfoList({QStringLiteral("*.gcov")}, QDir::Files);
    for (const QFileInfo &file : files) {
        if (QFile::remove(file.absoluteFilePath())) {
            ++removed;
        }
    }
    return removed;
}

QDBusInterface *BluetoothShortcutTest::newInterface(QObject *parent) const
{
    auto *interface = new QDBusInterface(kBluetoothTestService,
                                         kBluetoothTestPath,
                                         kBluetoothTestInterface,
                                         QDBusConnection::sessionBus(),
                                         parent);
    Q_ASSERT(interface->isValid());
    return interface;
}

void BluetoothShortcutTest::test_pluginExposesBasicMetadata()
{
    BluetoothShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("BluetoothShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("bluetooth-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("BluetoothShortcut"));
}

void BluetoothShortcutTest::test_shortcutInitialState()
{
    BluetoothShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("BluetoothShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Bluetooth"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Bluetooth"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(BLUETOOTH_ACTIVE_SYMBOLIC));
    QCOMPARE(shortcut.currentStatus().getWidgetId(), QStringLiteral("org.ukui.shortcut.bluetooth"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::Danger);
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QVERIFY(meta.contains(PluginMetaType::PC));
    QVERIFY(meta.contains(PluginMetaType::Tablet));
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::MenuButton);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::MenuButton);
}

void BluetoothShortcutTest::test_connectionFailedDisablesShortcut()
{
    BluetoothShortcut shortcut;
    shortcut.m_isEnable = true;

    shortcut.connectionFailed();

    QVERIFY(!shortcut.isEnable());
}

void BluetoothShortcutTest::test_initWithNullInterfaceKeepsSafeState()
{
    BluetoothShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);

    shortcut.init(nullptr);

    QVERIFY(shortcut.m_bluetoothIface == nullptr);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(BLUETOOTH_ACTIVE_SYMBOLIC));
    QCOMPARE(statusSpy.count(), 0);
}

void BluetoothShortcutTest::test_initWithValidInterfaceReadsPoweredConnectedDevice()
{
    BluetoothShortcut shortcut;
    auto *interface = newInterface();
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &BluetoothShortcut::enableStatusChanged);

    shortcut.init(interface);

    QCOMPARE(shortcut.m_bluetoothIface, interface);
    QCOMPARE(interface->parent(), &shortcut);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_defaultAdapterPower);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::HighLight);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Keyboard"));
}

void BluetoothShortcutTest::test_onAdapterChangedMarksDisabledWithoutAdapters()
{
    m_service.adapters.clear();
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &BluetoothShortcut::enableStatusChanged);

    shortcut.onAdapterChanged();

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);
    QCOMPARE(statusSpy.count(), 1);
}

void BluetoothShortcutTest::test_onAdapterChangedUsesClosedStateWhenAdapterPoweredOff()
{
    m_service.adapterPower[QStringLiteral("adapter0")] = false;
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &BluetoothShortcut::enableStatusChanged);

    shortcut.onAdapterChanged();

    QVERIFY(shortcut.isEnable());
    QVERIFY(!shortcut.m_defaultAdapterPower);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::BaseColor);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Closed"));
}

void BluetoothShortcutTest::test_onDeviceChangedReportsNoConnectionWhenPowered()
{
    m_service.deviceAttrs[QStringLiteral("device0")][QStringLiteral("Connected")] = false;
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = true;
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);

    shortcut.onDeviceChanged();

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("No Connection"));
}

void BluetoothShortcutTest::test_onDeviceChangedReportsClosedWhenNotPowered()
{
    m_service.deviceAttrs[QStringLiteral("device0")][QStringLiteral("Connected")] = false;
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = false;
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);

    shortcut.onDeviceChanged();

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Closed"));
}

void BluetoothShortcutTest::test_onDeviceChangedJoinsConnectedDeviceNames()
{
    m_service.pairedDevices = QStringList{QStringLiteral("device0"), QStringLiteral("device1"), QStringLiteral("device2")};
    m_service.deviceAttrs[QStringLiteral("device1")] = {{QStringLiteral("Connected"), true},
                                                        {QStringLiteral("Name"), QStringLiteral("Mouse")}};
    m_service.deviceAttrs[QStringLiteral("device2")] = {{QStringLiteral("Connected"), false},
                                                        {QStringLiteral("Name"), QStringLiteral("Speaker")}};
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = true;
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);

    shortcut.onDeviceChanged();

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Keyboard,Mouse"));
}

void BluetoothShortcutTest::test_setAddedStateFalseDisconnectsAndEmitsClosedStatus()
{
    BluetoothShortcut shortcut;
    shortcut.m_isAdded = true;
    shortcut.m_statusInfo.setColor(Color::HighLight);
    shortcut.m_statusInfo.setStatusName(QStringLiteral("Keyboard"));
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);

    shortcut.setAddedState(false);

    QVERIFY(!shortcut.m_isAdded);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::BaseColor);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Closed"));
}

void BluetoothShortcutTest::test_setAddedStateTrueRefreshesStatus()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    QSignalSpy statusSpy(&shortcut, &BluetoothShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &BluetoothShortcut::enableStatusChanged);

    shortcut.setAddedState(true);

    QVERIFY(shortcut.m_isAdded);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(shortcut.currentStatus().getStatusName(), QStringLiteral("Keyboard"));
}

void BluetoothShortcutTest::test_activeClickTogglesDefaultAdapterPower()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = false;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.postBluetoothPowerCount, 1);
    QCOMPARE(m_service.lastMethod, QStringLiteral("postBluetoothPower"));
    QCOMPARE(m_service.lastPostBluetoothPower, true);
    QTRY_VERIFY(!shortcut.m_powerCallPending);

    shortcut.m_defaultAdapterPower = true;
    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.postBluetoothPowerCount, 2);
    QCOMPARE(m_service.lastPostBluetoothPower, false);
}

void BluetoothShortcutTest::test_activeMenuRequestShowsTrayWidget()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);

    shortcut.active(PluginMetaType::MenuRequest);

    QTRY_COMPARE(m_service.showTrayWidgetCount, 1);
    QCOMPARE(m_service.lastMethod, QStringLiteral("showTrayWidgetUI"));
}

void BluetoothShortcutTest::test_activeIgnoresUnsupportedActionsAndNullInterface()
{
    BluetoothShortcut shortcut;

    shortcut.active(PluginMetaType::Click);
    shortcut.active(PluginMetaType::LongClick);
    QCOMPARE(m_service.postBluetoothPowerCount, 0);
    QCOMPARE(m_service.showTrayWidgetCount, 0);

    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.active(PluginMetaType::LongClick);
    QCOMPARE(m_service.postBluetoothPowerCount, 0);
    QCOMPARE(m_service.showTrayWidgetCount, 0);
}

void BluetoothShortcutTest::test_clickDebouncesWhilePowerCallPending()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = false;

    shortcut.active(PluginMetaType::Click);
    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.postBluetoothPowerCount, 1);
    QTRY_VERIFY(!shortcut.m_powerCallPending);

    shortcut.active(PluginMetaType::Click);
    QTRY_COMPARE(m_service.postBluetoothPowerCount, 2);
}

void BluetoothShortcutTest::test_callLegacySetBluetoothPowerCallsSetDefaultAdapterAttr()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);

    shortcut.callLegacySetBluetoothPower(true);
    QCOMPARE(m_service.setDefaultAdapterAttrCount, 1);
    QCOMPARE(m_service.lastMethod, QStringLiteral("setDefaultAdapterAttr"));
    QCOMPARE(m_service.lastAttr.value(QStringLiteral("Powered")).toBool(), true);

    shortcut.callLegacySetBluetoothPower(false);
    QCOMPARE(m_service.setDefaultAdapterAttrCount, 2);
    QCOMPARE(m_service.lastAttr.value(QStringLiteral("Powered")).toBool(), false);
}

void BluetoothShortcutTest::test_callLegacySetBluetoothPowerSkipsNullInterface()
{
    BluetoothShortcut shortcut;

    shortcut.callLegacySetBluetoothPower(true);

    QCOMPARE(m_service.setDefaultAdapterAttrCount, 0);
}

void BluetoothShortcutTest::test_callLegacySetBluetoothPowerSkipsInvalidInterface()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = new QDBusInterface(QStringLiteral("com.ukui.bluetooth.doesnotexist"),
                                                   QStringLiteral("/does/not/exist"),
                                                   QStringLiteral("com.ukui.bluetooth.doesnotexist"),
                                                   QDBusConnection::sessionBus(), &shortcut);
    QVERIFY(!shortcut.m_bluetoothIface->isValid());

    shortcut.callLegacySetBluetoothPower(true);

    QCOMPARE(m_service.setDefaultAdapterAttrCount, 0);
}

void BluetoothShortcutTest::test_clickFallsBackToLegacyWhenPowerCallFails()
{
    BluetoothShortcut shortcut;
    shortcut.m_bluetoothIface = newInterface(&shortcut);
    shortcut.m_defaultAdapterPower = false;
    m_service.postBluetoothPowerError = QDBusError::ServiceUnknown;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.postBluetoothPowerCount, 1);
    QTRY_VERIFY(!shortcut.m_powerCallPending);
    QTRY_COMPARE(m_service.setDefaultAdapterAttrCount, 1);
    QCOMPARE(m_service.lastAttr.value(QStringLiteral("Powered")).toBool(), true);
}

void BluetoothShortcutTest::test_connectAndDisconnectSignalFailureBranches()
{
    BluetoothShortcut shortcut;

    shortcut.connectSignal(QStringLiteral("missingSignal"), SLOT(onAdapterChanged()));
    shortcut.disconnectSignal(QStringLiteral("missingSignal"), SLOT(onAdapterChanged()));
}

QTEST_MAIN(BluetoothShortcutTest)

#include "bluetooth-shortcut-test.moc"
