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
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <memory>

#define private public
#include "../../../ukui-shortcut/shortcuts/power-mode-shortcut/power-mode-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kPowerService[] = "org.ukui.upower";
const char kPowerPath[] = "/upower/PowerManager";
const char kPowerInterface[] = "org.ukui.powerManager";

class FakePowerManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.powerManager")
public:
    int batterySaver = 0;
    int lastSetBatterySaver = -1;
    int getBatterySaverCount = 0;
    int setBatterySaverCount = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kPowerPath));
        bus.unregisterService(QString::fromLatin1(kPowerService));
        if (!bus.registerService(QString::fromLatin1(kPowerService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kPowerPath),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kPowerPath));
        bus.unregisterService(QString::fromLatin1(kPowerService));
        registered = false;
    }

    void reset()
    {
        batterySaver = 0;
        lastSetBatterySaver = -1;
        getBatterySaverCount = 0;
        setBatterySaverCount = 0;
    }

public Q_SLOTS:
    int GetBatterySaver()
    {
        ++getBatterySaverCount;
        return batterySaver;
    }

    bool SetBatterySaver(int state)
    {
        lastSetBatterySaver = state;
        batterySaver = state;
        ++setBatterySaverCount;
        return true;
    }

Q_SIGNALS:
    void PowerConfigChanged(QStringList keys);
};

class PowerModeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateWhenPowerServiceAvailable();
    void test_constructorDisablesWhenPowerServiceUnavailable();
    void test_setStateUpdatesStatusAndEnableSignals();
    void test_activeClickTogglesBatterySaver();
    void test_activeGuardsNullInterfaceInvalidStateAndUnsupportedActions();
    void test_stateChangedSlotIgnoresUnrelatedKeys();
    void test_stateChangedSlotRefreshesStateForBatterySaverKey();
    void test_dbusSignalDrivesStateChangedSlot();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;

private:
    FakePowerManagerService m_service;
};

} // namespace

void PowerModeShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void PowerModeShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void PowerModeShortcutTest::cleanup()
{
    m_service.reset();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int PowerModeShortcutTest::removeGeneratedGcovFiles() const
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

void PowerModeShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void PowerModeShortcutTest::test_pluginExposesBasicMetadata()
{
    PowerModeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("PowerModeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("power-mode-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("PowerModeShortcut"));
}

void PowerModeShortcutTest::test_shortcutInitialStateWhenPowerServiceAvailable()
{
    m_service.batterySaver = 1;

    PowerModeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("PowerModeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Power-Saving Mode"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Power-Saving Mode"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-eco-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_powerState, 1);
    QVERIFY(shortcut.m_powerManagerIface != nullptr);
    QVERIFY(shortcut.m_powerManagerIface->isValid());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 5);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 5);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void PowerModeShortcutTest::test_constructorDisablesWhenPowerServiceUnavailable()
{
    m_service.unregisterService();

    PowerModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_powerManagerIface != nullptr);
    QVERIFY(!shortcut.m_powerManagerIface->isValid());
}

void PowerModeShortcutTest::test_setStateUpdatesStatusAndEnableSignals()
{
    PowerModeShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &PowerModeShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &PowerModeShortcut::statusChanged);

    shortcut.setState(0);
    QCOMPARE(shortcut.m_powerState, 0);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(!shortcut.currentStatus().isDisabled());
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.setState(1);
    QCOMPARE(shortcut.m_powerState, 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(!shortcut.currentStatus().isDisabled());
    QVERIFY(shortcut.isEnable());
    QCOMPARE(statusSpy.count(), 2);

    shortcut.setState(2);
    QCOMPARE(shortcut.m_powerState, 2);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::Disable);
    QVERIFY(shortcut.currentStatus().isDisabled());
    QVERIFY(shortcut.isEnable());
    QCOMPARE(statusSpy.count(), 3);

    shortcut.setState(3);
    QCOMPARE(shortcut.m_powerState, 3);
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);
    QCOMPARE(statusSpy.count(), 4);

    shortcut.setState(0);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(statusSpy.count(), 5);
}

void PowerModeShortcutTest::test_activeClickTogglesBatterySaver()
{
    m_service.batterySaver = 0;
    PowerModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &PowerModeShortcut::statusChanged);
    m_service.reset();

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(m_service.setBatterySaverCount, 1);
    QCOMPARE(m_service.lastSetBatterySaver, 1);
    QCOMPARE(shortcut.m_powerState, 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(m_service.setBatterySaverCount, 2);
    QCOMPARE(m_service.lastSetBatterySaver, 0);
    QCOMPARE(shortcut.m_powerState, 0);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);
}

void PowerModeShortcutTest::test_activeGuardsNullInterfaceInvalidStateAndUnsupportedActions()
{
    PowerModeShortcut shortcut;
    m_service.reset();

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(m_service.setBatterySaverCount, 0);

    shortcut.m_powerState = 2;
    shortcut.active(PluginMetaType::Click);
    QCOMPARE(m_service.setBatterySaverCount, 0);

    shortcut.m_powerManagerIface = nullptr;
    shortcut.m_powerState = 0;
    shortcut.active(PluginMetaType::Click);
    QCOMPARE(m_service.setBatterySaverCount, 0);
}

void PowerModeShortcutTest::test_stateChangedSlotIgnoresUnrelatedKeys()
{
    m_service.batterySaver = 0;
    PowerModeShortcut shortcut;
    m_service.reset();

    shortcut.stateChangedSlot(QStringList{QStringLiteral("otherKey")});

    QCOMPARE(m_service.getBatterySaverCount, 0);
    QCOMPARE(shortcut.m_powerState, 0);
}

void PowerModeShortcutTest::test_stateChangedSlotRefreshesStateForBatterySaverKey()
{
    m_service.batterySaver = 0;
    PowerModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &PowerModeShortcut::statusChanged);
    m_service.reset();
    m_service.batterySaver = 2;

    shortcut.stateChangedSlot(QStringList{QStringLiteral("batterySaver")});

    QCOMPARE(m_service.getBatterySaverCount, 1);
    QCOMPARE(shortcut.m_powerState, 2);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::Disable);
    QVERIFY(shortcut.currentStatus().isDisabled());
    QCOMPARE(statusSpy.count(), 1);
}

void PowerModeShortcutTest::test_dbusSignalDrivesStateChangedSlot()
{
    m_service.batterySaver = 0;
    PowerModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &PowerModeShortcut::statusChanged);
    m_service.reset();
    m_service.batterySaver = 1;

    Q_EMIT m_service.PowerConfigChanged(QStringList{QStringLiteral("batterySaver")});
    QTRY_COMPARE(shortcut.m_powerState, 1);

    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(m_service.getBatterySaverCount >= 1);
    QVERIFY(statusSpy.count() >= 1);
}

QTEST_MAIN(PowerModeShortcutTest)

#include "power-mode-shortcut-test.moc"
