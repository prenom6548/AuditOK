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
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSignalBlocker>
#include <QSignalSpy>

#define private public
#include "../../ukui-shortcut/shortcuts/wifi-shortcut/wifi-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kWifiShortcutService[] = "com.kylin.network";
const char kWifiShortcutPath[] = "/com/kylin/network";
const char kWifiShortcutInterface[] = "com.kylin.network";

class FakeShortcutNetworkService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.network")
public:
    QVariantList wiredReply {1, 1, QStringLiteral("eth0")};
    QVariantList wirelessReply {1, 1, QStringLiteral("wifi0")};
    bool registered = false;
    QString lastMethod;
    bool lastEnabled = false;
    int wirelessSetCount = 0;
    int wiredSetCount = 0;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(kWifiShortcutPath);
        bus.unregisterService(kWifiShortcutService);
        if (!bus.registerService(kWifiShortcutService)) {
            return false;
        }

        registered = bus.registerObject(kWifiShortcutPath, this, QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(kWifiShortcutPath);
        bus.unregisterService(kWifiShortcutService);
        registered = false;
    }

public Q_SLOTS:
    QVariantList getNetworkDeviceData(int type)
    {
        return type == 0 ? wiredReply : wirelessReply;
    }

    void setWirelessSwitchEnable(bool enabled)
    {
        lastMethod = QStringLiteral("setWirelessSwitchEnable");
        lastEnabled = enabled;
        ++wirelessSetCount;
    }

    void setWiredSwitchEnable(bool enabled)
    {
        lastMethod = QStringLiteral("setWiredSwitchEnable");
        lastEnabled = enabled;
        ++wiredSetCount;
    }
};

class WiFiShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialState();
    void test_onNetworkDeviceDataChanged_ignoresShortPayload();
    void test_updateStatus_prefersWirelessWhenOpen();
    void test_updateStatus_usesWirelessClosedState();
    void test_updateStatus_fallsBackToWiredStatus();
    void test_updateStatus_openDevicesWithoutNames();
    void test_updateStatus_wiredOpenWithName();
    void test_updateStatus_marksDisabledWhenNoDevice();
    void test_setAddedState_updatesClosedStatusWhenRemoved();
    void test_setAddedState_trueEmitsQuerySignal();
    void test_active_clickTogglesWireless();
    void test_active_clickTogglesWired();
    void test_active_nonClickDoesNothing();
    void test_onDbusConnectorReady_invalidInterfaceIgnored();
    void test_onDbusConnectorReady_validInterfaceStartsWorker();
    void test_networkDeviceDataChanged_reEmitsCurrentInterface();
    void test_endHelpers_releaseOwnedObjects();

private:
    int removeGeneratedGcovFiles() const;

private:
    FakeShortcutNetworkService m_service;
};

} // namespace

void WiFiShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void WiFiShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void WiFiShortcutTest::cleanup()
{
    m_service.lastMethod.clear();
    m_service.lastEnabled = false;
    m_service.wiredSetCount = 0;
    m_service.wirelessSetCount = 0;
    m_service.wiredReply = QVariantList{1, 1, QStringLiteral("eth0")};
    m_service.wirelessReply = QVariantList{1, 1, QStringLiteral("wifi0")};
    removeGeneratedGcovFiles();
}

int WiFiShortcutTest::removeGeneratedGcovFiles() const
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

void WiFiShortcutTest::test_pluginExposesBasicMetadata()
{
    WiFiShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("WiFi"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("wifi-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("WiFi"));
}

void WiFiShortcutTest::test_shortcutInitialState()
{
    WiFiShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("WiFi"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Network"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Network"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(WIRELESS_ICON));
    QCOMPARE(shortcut.currentStatus().getWidgetId(), QStringLiteral("org.ukui.shortcut.network"));
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QVERIFY(meta.contains(PluginMetaType::PC));
    QVERIFY(meta.contains(PluginMetaType::Tablet));
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::MenuButton);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::MenuButton);
}

void WiFiShortcutTest::test_onNetworkDeviceDataChanged_ignoresShortPayload()
{
    WiFiShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &WiFiShortcut::enableStatusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{1, 1, QStringLiteral("only-three")});

    QCOMPARE(statusSpy.count(), 0);
    QCOMPARE(enableSpy.count(), 0);
}

void WiFiShortcutTest::test_updateStatus_prefersWirelessWhenOpen()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &WiFiShortcut::enableStatusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        1, 1, QStringLiteral("wired-name"),
        1, 1, QStringLiteral("wireless-name")
    });

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(enableSpy.count(), 1);
    const StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getIcon(), QStringLiteral(WIRELESS_ICON));
    QCOMPARE(info.getStatusName(), QStringLiteral("wireless-name"));
    QCOMPARE(info.getColor(), Color::HighLight);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
    QVERIFY(shortcut.isEnable());
}

void WiFiShortcutTest::test_updateStatus_usesWirelessClosedState()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        0, 0, QStringLiteral(""),
        1, 0, QStringLiteral("")
    });

    QCOMPARE(statusSpy.count(), 1);
    const StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getIcon(), QStringLiteral(WIRELESS_ICON));
    QCOMPARE(info.getStatusName(), QStringLiteral("Closed"));
    QCOMPARE(info.getColor(), Color::BaseColor);
}

void WiFiShortcutTest::test_updateStatus_fallsBackToWiredStatus()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        1, 1, QStringLiteral(""),
        0, 0, QStringLiteral("")
    });

    QCOMPARE(statusSpy.count(), 1);
    StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getIcon(), QStringLiteral(WIRED_ICON));
    QCOMPARE(info.getStatusName(), QStringLiteral("Not connected"));
    QCOMPARE(info.getColor(), Color::HighLight);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        1, 0, QStringLiteral("wired-name"),
        0, 0, QStringLiteral("")
    });
    info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getStatusName(), QStringLiteral("Closed"));
    QCOMPARE(info.getColor(), Color::BaseColor);
}

void WiFiShortcutTest::test_updateStatus_openDevicesWithoutNames()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        0, 0, QStringLiteral(""),
        1, 1, QStringLiteral("")
    });

    QCOMPARE(statusSpy.count(), 1);
    const StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getIcon(), QStringLiteral(WIRELESS_ICON));
    QCOMPARE(info.getStatusName(), QStringLiteral("Not connected"));
    QCOMPARE(info.getColor(), Color::HighLight);
}

void WiFiShortcutTest::test_updateStatus_wiredOpenWithName()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        1, 1, QStringLiteral("wired-name"),
        0, 0, QStringLiteral("")
    });

    QCOMPARE(statusSpy.count(), 1);
    const StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getIcon(), QStringLiteral(WIRED_ICON));
    QCOMPARE(info.getStatusName(), QStringLiteral("wired-name"));
    QCOMPARE(info.getColor(), Color::HighLight);
}

void WiFiShortcutTest::test_updateStatus_marksDisabledWhenNoDevice()
{
    WiFiShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);
    QSignalSpy enableSpy(&shortcut, &WiFiShortcut::enableStatusChanged);

    shortcut.onNetworkDeviceDataChanged(QVariantList{
        0, 0, QStringLiteral(""),
        0, 0, QStringLiteral("")
    });

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(enableSpy.count(), 1);
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);
}

void WiFiShortcutTest::test_setAddedState_updatesClosedStatusWhenRemoved()
{
    WiFiShortcut shortcut;
    shortcut.m_currentStatus.setStatusName(QStringLiteral("before"));
    shortcut.m_currentStatus.setColor(Color::HighLight);
    QSignalSpy statusSpy(&shortcut, &WiFiShortcut::statusChanged);

    shortcut.setAddedState(false);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.m_isAdded = true;
    shortcut.setAddedState(false);

    QCOMPARE(statusSpy.count(), 1);
    const StatusInfo info = qvariant_cast<StatusInfo>(statusSpy.takeFirst().at(0));
    QCOMPARE(info.getStatusName(), QStringLiteral("Closed"));
    QCOMPARE(info.getColor(), Color::BaseColor);
    QVERIFY(!shortcut.m_isAdded);
}

void WiFiShortcutTest::test_setAddedState_trueEmitsQuerySignal()
{
    WiFiShortcut shortcut;
    QSignalSpy querySpy(&shortcut, &WiFiShortcut::toGetNetworkDeviceData);

    shortcut.setAddedState(true);

    QCOMPARE(querySpy.count(), 1);
    QCOMPARE(qvariant_cast<QDBusInterface *>(querySpy.takeFirst().at(0)), shortcut.m_networkInterface);
    QVERIFY(shortcut.m_isAdded);
}

void WiFiShortcutTest::test_active_clickTogglesWireless()
{
    WiFiShortcut shortcut;
    shortcut.m_isWirelessEnable = true;
    shortcut.m_isWirelessOpen = false;
    shortcut.m_isWireEnable = true;
    shortcut.m_isWireOpen = true;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.wirelessSetCount, 1);
    QCOMPARE(m_service.lastMethod, QStringLiteral("setWirelessSwitchEnable"));
    QCOMPARE(m_service.lastEnabled, true);
    QCOMPARE(m_service.wiredSetCount, 0);
}

void WiFiShortcutTest::test_active_clickTogglesWired()
{
    WiFiShortcut shortcut;
    shortcut.m_isWirelessEnable = false;
    shortcut.m_isWireEnable = true;
    shortcut.m_isWireOpen = true;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.wiredSetCount, 1);
    QCOMPARE(m_service.lastMethod, QStringLiteral("setWiredSwitchEnable"));
    QCOMPARE(m_service.lastEnabled, false);
    QCOMPARE(m_service.wirelessSetCount, 0);
}

void WiFiShortcutTest::test_active_nonClickDoesNothing()
{
    WiFiShortcut shortcut;
    shortcut.m_isWirelessEnable = true;
    shortcut.m_isWirelessOpen = true;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);

    QCOMPARE(m_service.wirelessSetCount, 0);
    QCOMPARE(m_service.wiredSetCount, 0);
}

void WiFiShortcutTest::test_onDbusConnectorReady_invalidInterfaceIgnored()
{
    WiFiShortcut shortcut;
    QDBusInterface invalidInterface(QStringLiteral("missing.service"),
                                    QStringLiteral("/missing"),
                                    QStringLiteral("missing.interface"),
                                    QDBusConnection::sessionBus());

    shortcut.onDbusConnectorReady(nullptr);
    QVERIFY(shortcut.m_networkInterface == nullptr);
    QVERIFY(shortcut.m_wifiDbusThread == nullptr);
    QVERIFY(shortcut.m_wifiDbusQuery == nullptr);

    shortcut.onDbusConnectorReady(&invalidInterface);
    QVERIFY(shortcut.m_networkInterface == nullptr);
    QVERIFY(shortcut.m_wifiDbusThread == nullptr);
    QVERIFY(shortcut.m_wifiDbusQuery == nullptr);
}

void WiFiShortcutTest::test_onDbusConnectorReady_validInterfaceStartsWorker()
{
    WiFiShortcut shortcut;
    QDBusInterface interface(kWifiShortcutService,
                             kWifiShortcutPath,
                             kWifiShortcutInterface,
                             QDBusConnection::sessionBus());
    QVERIFY(interface.isValid());

    {
        QSignalBlocker blocker(&shortcut);
        shortcut.onDbusConnectorReady(&interface);
    }

    QVERIFY(shortcut.m_networkInterface == &interface);
    QVERIFY(shortcut.m_wifiDbusThread != nullptr);
    QVERIFY(shortcut.m_wifiDbusQuery != nullptr);
    QVERIFY(shortcut.m_wifiDbusThread->isRunning());

    QThread *thread = shortcut.m_wifiDbusThread;
    WifiDbusQuery *query = shortcut.m_wifiDbusQuery;
    shortcut.m_wifiDbusQuery = nullptr;
    shortcut.m_wifiDbusThread = nullptr;
    shortcut.m_networkInterface = nullptr;
    QObject::connect(thread, &QThread::finished, query, &QObject::deleteLater);
    thread->quit();
    thread->wait();
    delete thread;
}

void WiFiShortcutTest::test_networkDeviceDataChanged_reEmitsCurrentInterface()
{
    WiFiShortcut shortcut;
    shortcut.m_networkInterface = nullptr;

    shortcut.networkDeviceDataChanged();
}

void WiFiShortcutTest::test_endHelpers_releaseOwnedObjects()
{
    WiFiShortcut shortcut;
    auto *thread = new QThread();
    auto *query = new WifiDbusQuery();
    auto *interface = new QDBusInterface(kWifiShortcutService,
                                         kWifiShortcutPath,
                                         kWifiShortcutInterface,
                                         QDBusConnection::sessionBus());
    QVERIFY(interface->isValid());

    shortcut.m_wifiDbusThread = thread;
    shortcut.m_wifiDbusQuery = query;
    shortcut.m_networkInterface = interface;
    thread->start();
    query->moveToThread(thread);
    QObject::connect(&shortcut, &WiFiShortcut::toGetNetworkDeviceData, query, &WifiDbusQuery::getNetworkDeviceData);
    QObject::connect(query, &WifiDbusQuery::networkDeviceData, &shortcut, &WiFiShortcut::onNetworkDeviceDataChanged);

    shortcut.endQueryThread();
    QVERIFY(shortcut.m_wifiDbusThread == nullptr);
    QVERIFY(shortcut.m_wifiDbusQuery == nullptr);

    shortcut.endNetworkDbus();
    QVERIFY(shortcut.m_networkInterface == nullptr);
}

QTEST_MAIN(WiFiShortcutTest)

#include "wifi-shortcut-test.moc"
