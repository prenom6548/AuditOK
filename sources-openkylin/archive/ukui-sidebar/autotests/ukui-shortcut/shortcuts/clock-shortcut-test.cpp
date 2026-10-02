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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/clock-shortcut/clock-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kClockDesktop[] = "/usr/share/applications/ukui-clock.desktop";

class FakeAppLauncherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")
public:
    QString lastDesktopFile;
    int launchAppCount = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"));
        bus.unregisterService(QStringLiteral("com.kylin.ProcessManager"));
        if (!bus.registerService(QStringLiteral("com.kylin.ProcessManager"))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"));
        bus.unregisterService(QStringLiteral("com.kylin.ProcessManager"));
        registered = false;
    }

    void reset()
    {
        lastDesktopFile.clear();
        launchAppCount = 0;
    }

public Q_SLOTS:
    bool LaunchApp(const QString &desktopFile)
    {
        lastDesktopFile = desktopFile;
        ++launchAppCount;
        return true;
    }
};

class FakeBrokenAppLauncherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")
public:
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"));
        bus.unregisterService(QStringLiteral("com.kylin.ProcessManager"));
        if (!bus.registerService(QStringLiteral("com.kylin.ProcessManager"))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral("/com/kylin/ProcessManager/AppLauncher"));
        bus.unregisterService(QStringLiteral("com.kylin.ProcessManager"));
        registered = false;
    }
};

class ClockShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateWhenInstalled();
    void test_shortcutInitialStateWhenMissing();
    void test_setAddedStateOnlyChangesWhenNeeded();
    void test_getDesktopUsesApplicationInfo();
    void test_installAndUninstallSlotsIgnoreUnrelatedDesktop();
    void test_installAndUninstallSlotsUpdateEnableState();
    void test_applicationInfoSignalsDriveInstallState();
    void test_activeGuardsDisabledNotAddedAndUnsupportedAction();
    void test_activeClickLaunchesClock();
    void test_activeClickFallsBackWhenDbusUnavailable();
    void test_destructorHandlesNullApplicationInfo();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;
    void setClockDesktopPath(const QString &path) const;

private:
    FakeAppLauncherService m_service;
};

} // namespace

void ClockShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void ClockShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void ClockShortcutTest::cleanup()
{
    m_service.reset();
    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int ClockShortcutTest::removeGeneratedGcovFiles() const
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

void ClockShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void ClockShortcutTest::setClockDesktopPath(const QString &path) const
{
    UkuiSearch::ApplicationInfo::setDesktopFilePath(QStringLiteral("ukui-clock"), path);
}

void ClockShortcutTest::test_pluginExposesBasicMetadata()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("Clock"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("clock-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("Clock"));
}

void ClockShortcutTest::test_shortcutInitialStateWhenInstalled()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));

    ClockShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("Clock"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Clock"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Clock"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kClockDesktop));
    QVERIFY(shortcut.m_applicationInfo != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 10);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 10);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void ClockShortcutTest::test_shortcutInitialStateWhenMissing()
{
    ClockShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
}

void ClockShortcutTest::test_setAddedStateOnlyChangesWhenNeeded()
{
    ClockShortcut shortcut;
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

void ClockShortcutTest::test_getDesktopUsesApplicationInfo()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;

    QCOMPARE(shortcut.getDesktop(QStringLiteral("ukui-clock")), QString::fromLatin1(kClockDesktop));
    QCOMPARE(shortcut.getDesktop(QStringLiteral("missing-app")), QString());
}

void ClockShortcutTest::test_installAndUninstallSlotsIgnoreUnrelatedDesktop()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ClockShortcut::enableStatusChanged);

    shortcut.onAppUninstalled(QStringList{QStringLiteral("/tmp/other.desktop")});
    shortcut.onAppInstalled(QStringList{QStringLiteral("/tmp/other.desktop")});

    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
}

void ClockShortcutTest::test_installAndUninstallSlotsUpdateEnableState()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ClockShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    shortcut.onAppUninstalled(QStringList{QStringLiteral("/usr/share/applications/ukui-clock.desktop")});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);

    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    shortcut.onAppInstalled(QStringList{QStringLiteral("ukui-clock.desktop")});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kClockDesktop));
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);

    shortcut.checkAppInstallState();
    QCOMPARE(enableSpy.count(), 0);
}

void ClockShortcutTest::test_applicationInfoSignalsDriveInstallState()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ClockShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BDelete(QStringList{QStringLiteral("ukui-clock.desktop")});
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BAdd(QStringList{QStringLiteral("ukui-clock.desktop")});
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);
}

void ClockShortcutTest::test_activeGuardsDisabledNotAddedAndUnsupportedAction()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;

    shortcut.m_isEnable = false;
    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);

    shortcut.m_isEnable = true;
    shortcut.m_isAdded = false;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);

    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);
}

void ClockShortcutTest::test_activeClickLaunchesClock()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    ClockShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.launchAppCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kClockDesktop));
}

void ClockShortcutTest::test_activeClickFallsBackWhenDbusUnavailable()
{
    setClockDesktopPath(QString::fromLatin1(kClockDesktop));
    m_service.unregisterService();
    FakeBrokenAppLauncherService brokenService;
    QVERIFY(brokenService.registerService());
    const QByteArray originalPath = qgetenv("PATH");
    qputenv("PATH", QByteArray("/nonexistent"));

    ClockShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    qputenv("PATH", originalPath);
    QCOMPARE(m_service.launchAppCount, 0);
    brokenService.unregisterService();
    QVERIFY(m_service.registerService());
}

void ClockShortcutTest::test_destructorHandlesNullApplicationInfo()
{
    auto *shortcut = new ClockShortcut;
    delete shortcut->m_applicationInfo;
    shortcut->m_applicationInfo = nullptr;

    delete shortcut;

    QVERIFY(true);
}

QTEST_MAIN(ClockShortcutTest)

#include "clock-shortcut-test.moc"
