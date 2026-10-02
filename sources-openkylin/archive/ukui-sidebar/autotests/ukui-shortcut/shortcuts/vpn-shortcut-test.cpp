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
#include <QDBusConnectionInterface>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <memory>

#define private public
#include "../../../ukui-shortcut/shortcuts/vpn-shortcut/vpn-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kVpnService[] = "com.kylin.kylinvpn";
const char kVpnPath[] = "/com/kylin/kylinvpn";
const char kVpnInterface[] = "com.kylin.kylinvpn";

class FakeVpnService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kylinvpn")
public:
    int showKylinVpnCount = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kVpnPath));
        bus.unregisterService(QString::fromLatin1(kVpnService));
        if (!bus.registerService(QString::fromLatin1(kVpnService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kVpnPath),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kVpnPath));
        bus.unregisterService(QString::fromLatin1(kVpnService));
        registered = false;
    }

    void unregisterObjectOnly()
    {
        QDBusConnection::sessionBus().unregisterObject(QString::fromLatin1(kVpnPath));
    }

    void reset()
    {
        showKylinVpnCount = 0;
    }

public Q_SLOTS:
    bool showKylinVpn()
    {
        ++showKylinVpnCount;
        return true;
    }
};

class VPNShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateWhenServiceAvailable();
    void test_shortcutInitialStateWhenServiceUnavailable();
    void test_setAddedStateOnlyChangesWhenNeeded();
    void test_onServiceUnregisteredDisablesShortcut();
    void test_onServiceRegisteredInitializesInterfaceAndEmits();
    void test_onServiceRegisteredDoesNothingWhenInterfaceExists();
    void test_activeGuardsDisabledNotAddedAndUnsupportedAction();
    void test_activeClickShowsVpn();
    void test_triggerHandlesAsyncCallError();
    void test_initVpnManagerDbusInterfaceFailureCanRecover();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;

private:
    FakeVpnService m_service;
};

} // namespace

void VPNShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void VPNShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void VPNShortcutTest::cleanup()
{
    m_service.reset();
    if (!m_service.registered
        || !QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1(kVpnService)).value()) {
        QVERIFY(m_service.registerService());
    } else {
        QDBusConnection::sessionBus().unregisterObject(QString::fromLatin1(kVpnPath));
        QVERIFY(QDBusConnection::sessionBus().registerObject(QString::fromLatin1(kVpnPath),
                                                             &m_service,
                                                             QDBusConnection::ExportAllSlots));
    }
    removeGeneratedGcovFiles();
}

int VPNShortcutTest::removeGeneratedGcovFiles() const
{
    int removed = 0;
    QDirIterator it(QDir::currentPath(),
                    {QStringLiteral("*.gcov")},
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (QFile::remove(it.next())) {
            ++removed;
        }
    }
    return removed;
}

void VPNShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void VPNShortcutTest::test_pluginExposesBasicMetadata()
{
    VPNShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("VPN"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("vpn-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("VPN"));
}

void VPNShortcutTest::test_shortcutInitialStateWhenServiceAvailable()
{
    VPNShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("VPN"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("VPN"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("VPN"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_vpnManagerDbusInterface != nullptr);
    QVERIFY(shortcut.m_vpnManagerDbusInterface->isValid());
    QVERIFY(shortcut.m_vpnServiceWatcher != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 12);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 12);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void VPNShortcutTest::test_shortcutInitialStateWhenServiceUnavailable()
{
    m_service.unregisterService();

    VPNShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_vpnManagerDbusInterface == nullptr);
    QVERIFY(shortcut.m_vpnServiceWatcher != nullptr);
}

void VPNShortcutTest::test_setAddedStateOnlyChangesWhenNeeded()
{
    VPNShortcut shortcut;
    QVERIFY(!shortcut.m_isAdded);

    shortcut.setAddedState(false);
    QVERIFY(!shortcut.m_isAdded);

    shortcut.setAddedState(true);
    QVERIFY(shortcut.m_isAdded);

    shortcut.setAddedState(true);
    QVERIFY(shortcut.m_isAdded);

    shortcut.setAddedState(false);
    QVERIFY(!shortcut.m_isAdded);
}

void VPNShortcutTest::test_onServiceUnregisteredDisablesShortcut()
{
    VPNShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &VPNShortcut::enableStatusChanged);

    shortcut.onServiceUnregistered();

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_vpnManagerDbusInterface == nullptr);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);
}

void VPNShortcutTest::test_onServiceRegisteredInitializesInterfaceAndEmits()
{
    m_service.unregisterService();
    VPNShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &VPNShortcut::enableStatusChanged);

    QVERIFY(m_service.registerService());
    shortcut.onServiceRegistered();

    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_vpnManagerDbusInterface != nullptr);
    QVERIFY(shortcut.m_vpnManagerDbusInterface->isValid());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
}

void VPNShortcutTest::test_onServiceRegisteredDoesNothingWhenInterfaceExists()
{
    VPNShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &VPNShortcut::enableStatusChanged);
    QDBusInterface *existingInterface = shortcut.m_vpnManagerDbusInterface;

    shortcut.onServiceRegistered();

    QCOMPARE(shortcut.m_vpnManagerDbusInterface, existingInterface);
    QCOMPARE(enableSpy.count(), 0);
}

void VPNShortcutTest::test_activeGuardsDisabledNotAddedAndUnsupportedAction()
{
    VPNShortcut shortcut;

    shortcut.m_isEnable = false;
    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.showKylinVpnCount, 0);

    shortcut.m_isEnable = true;
    shortcut.m_isAdded = false;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.showKylinVpnCount, 0);

    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();
    QCOMPARE(m_service.showKylinVpnCount, 0);
}

void VPNShortcutTest::test_activeClickShowsVpn()
{
    VPNShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    QTRY_COMPARE(m_service.showKylinVpnCount, 1);
}

void VPNShortcutTest::test_triggerHandlesAsyncCallError()
{
    VPNShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);
    m_service.unregisterObjectOnly();

    shortcut.trigger();
    waitForAsync();

    QCOMPARE(m_service.showKylinVpnCount, 0);
}

void VPNShortcutTest::test_initVpnManagerDbusInterfaceFailureCanRecover()
{
    m_service.unregisterService();
    VPNShortcut shortcut;

    QVERIFY(!shortcut.initVpnManagerDbusInterface());
    QVERIFY(shortcut.m_vpnManagerDbusInterface == nullptr);

    QVERIFY(m_service.registerService());
    QVERIFY(shortcut.initVpnManagerDbusInterface());
    QVERIFY(shortcut.m_vpnManagerDbusInterface != nullptr);
    QVERIFY(shortcut.m_vpnManagerDbusInterface->isValid());
}

QTEST_MAIN(VPNShortcutTest)

#include "vpn-shortcut-test.moc"
