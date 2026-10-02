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
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/status-change-shortcut/status-change-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kStatusService[] = "com.kylin.statusmanager.interface";
const char kStatusPath[] = "/";

class FakeStatusManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.statusmanager.interface")
public:
    bool tabletModeEnabled = true;
    bool tabletMode = false;
    bool setTabletModeReturn = true;
    bool registered = false;
    bool lastSetTabletMode = false;
    QString lastCaller;
    QString lastReason;
    int setTabletModeCount = 0;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kStatusPath));
        bus.unregisterService(QString::fromLatin1(kStatusService));
        if (!bus.registerService(QString::fromLatin1(kStatusService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kStatusPath),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kStatusPath));
        bus.unregisterService(QString::fromLatin1(kStatusService));
        registered = false;
    }

    void reset()
    {
        tabletModeEnabled = true;
        tabletMode = false;
        setTabletModeReturn = true;
        lastSetTabletMode = false;
        lastCaller.clear();
        lastReason.clear();
        setTabletModeCount = 0;
    }

public Q_SLOTS:
    bool isTabletModeEnabled()
    {
        return tabletModeEnabled;
    }

    bool get_current_tabletmode()
    {
        return tabletMode;
    }

    bool set_tabletmode(bool enabled, const QString &caller, const QString &reason)
    {
        lastSetTabletMode = enabled;
        lastCaller = caller;
        lastReason = reason;
        ++setTabletModeCount;
        if (setTabletModeReturn) {
            tabletMode = enabled;
        }
        return setTabletModeReturn;
    }

Q_SIGNALS:
    void mode_change_signal(bool tabletMode);
    void isTabletModeEnabledChanged(bool enabled);
};

class FakeBrokenStatusManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.statusmanager.interface")
public:
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kStatusPath));
        bus.unregisterService(QString::fromLatin1(kStatusService));
        if (!bus.registerService(QString::fromLatin1(kStatusService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kStatusPath),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kStatusPath));
        bus.unregisterService(QString::fromLatin1(kStatusService));
        registered = false;
    }
};

class StatusChangeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateFromDbus();
    void test_initInfoUsesMaxTabletName();
    void test_constructorHandlesInvalidInterface();
    void test_enableChangedDisablesWhenRefreshReplyFails();
    void test_statusChangeSlotEmitsOnlyOnChange();
    void test_enableChangedEmitsOnlyOnChangeAndRefreshesStatus();
    void test_activeClickTogglesTabletMode();
    void test_activeIgnoresUnsupportedActions();
    void test_dbusSignalsDriveSlots();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;

private:
    FakeStatusManagerService m_service;
};

} // namespace

void StatusChangeShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void StatusChangeShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void StatusChangeShortcutTest::cleanup()
{
    m_service.reset();
    kdk_test_set_system_category("PC");
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int StatusChangeShortcutTest::removeGeneratedGcovFiles() const
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

void StatusChangeShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void StatusChangeShortcutTest::test_pluginExposesBasicMetadata()
{
    StatusChangeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("StatusChangeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("status-change-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("StatusChangeShortcut"));
}

void StatusChangeShortcutTest::test_shortcutInitialStateFromDbus()
{
    m_service.tabletModeEnabled = true;
    m_service.tabletMode = true;

    StatusChangeShortcut shortcut;
    waitForAsync();

    QCOMPARE(shortcut.pluginId(), QStringLiteral("StatusChangeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Tablet Mode"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("tablet-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_isTabletMode);
    QVERIFY(shortcut.m_statusManagerIface != nullptr);
    QVERIFY(shortcut.m_statusManagerIface->isValid());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 0);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 0);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
}

void StatusChangeShortcutTest::test_initInfoUsesMaxTabletName()
{
    kdk_test_set_system_category("MaxTablet");

    StatusChangeShortcut shortcut;
    waitForAsync();

    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Max Tablet"));
}

void StatusChangeShortcutTest::test_constructorHandlesInvalidInterface()
{
    m_service.unregisterService();

    StatusChangeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_statusManagerIface != nullptr);
    QVERIFY(!shortcut.m_statusManagerIface->isValid());
    QVERIFY(shortcut.pluginMetaData().isEmpty());
}

void StatusChangeShortcutTest::test_enableChangedDisablesWhenRefreshReplyFails()
{
    m_service.unregisterService();
    FakeBrokenStatusManagerService brokenService;
    QVERIFY(brokenService.registerService());

    StatusChangeShortcut shortcut;
    waitForAsync();
    QSignalSpy enableSpy(&shortcut, &StatusChangeShortcut::enableStatusChanged);

    shortcut.m_isEnable = false;
    shortcut.enableChanged(true);
    waitForAsync();

    QVERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);

    brokenService.unregisterService();
    QVERIFY(m_service.registerService());
}

void StatusChangeShortcutTest::test_statusChangeSlotEmitsOnlyOnChange()
{
    StatusChangeShortcut shortcut;
    waitForAsync();
    QSignalSpy statusSpy(&shortcut, &StatusChangeShortcut::statusChanged);

    shortcut.statusChangeSlot(shortcut.m_isTabletMode);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.statusChangeSlot(!shortcut.m_isTabletMode);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getColor(),
             shortcut.m_isTabletMode ? Color::ColorRole::HighLight : Color::ColorRole::BaseColor);
}

void StatusChangeShortcutTest::test_enableChangedEmitsOnlyOnChangeAndRefreshesStatus()
{
    m_service.tabletModeEnabled = false;
    m_service.tabletMode = true;
    StatusChangeShortcut shortcut;
    waitForAsync();
    QSignalSpy enableSpy(&shortcut, &StatusChangeShortcut::enableStatusChanged);

    shortcut.enableChanged(false);
    QCOMPARE(enableSpy.count(), 0);

    shortcut.enableChanged(true);
    waitForAsync();
    QCOMPARE(enableSpy.count(), 1);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_isTabletMode);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void StatusChangeShortcutTest::test_activeClickTogglesTabletMode()
{
    m_service.tabletModeEnabled = true;
    m_service.tabletMode = false;
    StatusChangeShortcut shortcut;
    waitForAsync();

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QTRY_COMPARE(m_service.setTabletModeCount, 1);
    QCOMPARE(m_service.lastSetTabletMode, true);
    QCOMPARE(m_service.lastCaller, QStringLiteral("ukui-sidebar"));
    QCOMPARE(m_service.lastReason, QStringLiteral("changemode"));
}

void StatusChangeShortcutTest::test_activeIgnoresUnsupportedActions()
{
    StatusChangeShortcut shortcut;
    waitForAsync();

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);

    QCOMPARE(m_service.setTabletModeCount, 0);
}

void StatusChangeShortcutTest::test_dbusSignalsDriveSlots()
{
    m_service.tabletModeEnabled = false;
    StatusChangeShortcut shortcut;
    waitForAsync();
    QSignalSpy enableSpy(&shortcut, &StatusChangeShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &StatusChangeShortcut::statusChanged);

    Q_EMIT m_service.isTabletModeEnabledChanged(true);
    waitForAsync();
    QVERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    Q_EMIT m_service.mode_change_signal(true);
    waitForAsync();
    QVERIFY(shortcut.m_isTabletMode);
    QVERIFY(statusSpy.count() >= 1);
}

QTEST_MAIN(StatusChangeShortcutTest)

#include "status-change-shortcut-test.moc"
