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
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/auto-rotation-shortcut/auto-rotation-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

class FakeStatusManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.statusmanager.interface")
public:
    bool supportedAutoRotation = false;
    bool autoRotation = false;
    bool tabletMode = true;
    bool setAutoRotationReturn = true;
    bool registered = false;
    bool lastSetAutoRotation = false;
    QString lastCaller;
    QString lastReason;
    int setAutoRotationCount = 0;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral("/"));
        bus.unregisterService(QStringLiteral("com.kylin.statusmanager.interface"));
        if (!bus.registerService(QStringLiteral("com.kylin.statusmanager.interface"))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral("/"),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral("/"));
        bus.unregisterService(QStringLiteral("com.kylin.statusmanager.interface"));
        registered = false;
    }

    void reset()
    {
        supportedAutoRotation = false;
        autoRotation = false;
        tabletMode = true;
        setAutoRotationReturn = true;
        lastSetAutoRotation = false;
        lastCaller.clear();
        lastReason.clear();
        setAutoRotationCount = 0;
    }

public Q_SLOTS:
    bool is_supported_autorotation()
    {
        return supportedAutoRotation;
    }

    bool get_auto_rotation()
    {
        return autoRotation;
    }

    bool get_current_tabletmode()
    {
        return tabletMode;
    }

    bool set_auto_rotation(bool enabled, const QString &caller, const QString &reason)
    {
        lastSetAutoRotation = enabled;
        lastCaller = caller;
        lastReason = reason;
        ++setAutoRotationCount;
        if (setAutoRotationReturn) {
            autoRotation = enabled;
        }
        return setAutoRotationReturn;
    }

Q_SIGNALS:
    void mode_change_signal(bool tabletMode);
    void auto_rotation_change_signal(bool autoRotation);
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

        bus.unregisterObject(QStringLiteral("/"));
        bus.unregisterService(QStringLiteral("com.kylin.statusmanager.interface"));
        if (!bus.registerService(QStringLiteral("com.kylin.statusmanager.interface"))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral("/"),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral("/"));
        bus.unregisterService(QStringLiteral("com.kylin.statusmanager.interface"));
        registered = false;
    }
};

class AutoRotationShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateFromDbus();
    void test_constructorDisablesWhenSupportedFlagReturnsTrue();
    void test_constructorDisablesWhenNotTabletMode();
    void test_constructorHandlesInvalidInterface();
    void test_constructorHandlesInvalidMethodRepliesAndConnectFailures();
    void test_statusChangeSlotEmitsOnlyOnChange();
    void test_autoRotationChangeSlotUpdatesColorAndEmission();
    void test_activeClickTogglesAutoRotation();
    void test_activeIgnoresUnsupportedActions();
    void test_dbusSignalsDriveSlots();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;

private:
    FakeStatusManagerService m_service;
};

} // namespace

void AutoRotationShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void AutoRotationShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void AutoRotationShortcutTest::cleanup()
{
    m_service.reset();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int AutoRotationShortcutTest::removeGeneratedGcovFiles() const
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

void AutoRotationShortcutTest::waitForAsync() const
{
    QTest::qWait(50);
}

void AutoRotationShortcutTest::test_pluginExposesBasicMetadata()
{
    AutoRotationShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("AutoRotationShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("auto-rotation-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("AutoRotationShortcut"));
}

void AutoRotationShortcutTest::test_shortcutInitialStateFromDbus()
{
    m_service.autoRotation = true;
    m_service.tabletMode = true;

    AutoRotationShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("AutoRotationShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Auto Rotation"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Auto Rotation"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("screen-rotation-landscape-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_autoRotationEnable);
    QVERIFY(shortcut.m_statusManagerIface != nullptr);
    QVERIFY(shortcut.m_statusManagerIface->isValid());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 1);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 1);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void AutoRotationShortcutTest::test_constructorDisablesWhenSupportedFlagReturnsTrue()
{
    m_service.supportedAutoRotation = true;
    m_service.autoRotation = true;
    m_service.tabletMode = true;

    AutoRotationShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(!shortcut.m_autoRotationEnable);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
}

void AutoRotationShortcutTest::test_constructorDisablesWhenNotTabletMode()
{
    m_service.autoRotation = true;
    m_service.tabletMode = false;

    AutoRotationShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_autoRotationEnable);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void AutoRotationShortcutTest::test_constructorHandlesInvalidInterface()
{
    m_service.unregisterService();

    AutoRotationShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Auto Rotation"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
}

void AutoRotationShortcutTest::test_constructorHandlesInvalidMethodRepliesAndConnectFailures()
{
    m_service.unregisterService();
    FakeBrokenStatusManagerService brokenService;
    QVERIFY(brokenService.registerService());

    AutoRotationShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    brokenService.unregisterService();
    QVERIFY(m_service.registerService());
}

void AutoRotationShortcutTest::test_statusChangeSlotEmitsOnlyOnChange()
{
    AutoRotationShortcut shortcut;
    shortcut.m_isEnable = false;
    QSignalSpy enableSpy(&shortcut, &AutoRotationShortcut::enableStatusChanged);

    shortcut.statusChangeSlot(false);
    QCOMPARE(enableSpy.count(), 0);

    shortcut.statusChangeSlot(true);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);

    shortcut.statusChangeSlot(true);
    QCOMPARE(enableSpy.count(), 1);
}

void AutoRotationShortcutTest::test_autoRotationChangeSlotUpdatesColorAndEmission()
{
    AutoRotationShortcut shortcut;
    shortcut.m_isEnable = false;
    QSignalSpy statusSpy(&shortcut, &AutoRotationShortcut::statusChanged);

    shortcut.autoRotationChangeSlot(true);
    QVERIFY(shortcut.m_autoRotationEnable);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.m_isEnable = true;
    shortcut.autoRotationChangeSlot(false);
    QVERIFY(!shortcut.m_autoRotationEnable);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 1);
}

void AutoRotationShortcutTest::test_activeClickTogglesAutoRotation()
{
    m_service.autoRotation = false;

    AutoRotationShortcut shortcut;
    QVERIFY(shortcut.isEnable());

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.setAutoRotationCount, 1);
    QVERIFY(m_service.lastSetAutoRotation);
    QCOMPARE(m_service.lastCaller, QStringLiteral("ukui-sidebar"));
    QCOMPARE(m_service.lastReason, QStringLiteral("setAutoRotation"));
}

void AutoRotationShortcutTest::test_activeIgnoresUnsupportedActions()
{
    AutoRotationShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.setAutoRotationCount, 0);
}

void AutoRotationShortcutTest::test_dbusSignalsDriveSlots()
{
    AutoRotationShortcut shortcut;
    shortcut.m_isEnable = false;
    QSignalSpy enableSpy(&shortcut, &AutoRotationShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &AutoRotationShortcut::statusChanged);

    Q_EMIT m_service.mode_change_signal(true);
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    Q_EMIT m_service.auto_rotation_change_signal(true);
    QTRY_COMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(statusSpy.count() >= 1);
}

QTEST_MAIN(AutoRotationShortcutTest)

#include "auto-rotation-shortcut-test.moc"
