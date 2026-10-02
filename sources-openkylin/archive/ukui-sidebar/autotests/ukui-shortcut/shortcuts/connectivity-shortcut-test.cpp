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
#include "../../../ukui-shortcut/shortcuts/connectivity-shortcut/connectivity-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kConnectivityDesktop[] = "/usr/share/applications/kylin-connectivity.desktop";
const char kAppLauncherService[] = "com.kylin.ProcessManager";
const char kAppLauncherPath[] = "/com/kylin/ProcessManager/AppLauncher";

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
        if (registered) {
            return true;
        }

        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kAppLauncherPath));
        bus.unregisterService(QString::fromLatin1(kAppLauncherService));
        if (!bus.registerService(QString::fromLatin1(kAppLauncherService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kAppLauncherPath),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        if (!registered) {
            bus.unregisterService(QString::fromLatin1(kAppLauncherService));
        }
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kAppLauncherPath));
        bus.unregisterService(QString::fromLatin1(kAppLauncherService));
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

class ConnectivityShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateAndMetadata();
    void test_setAddedStateOnlyChangesWhenNeeded();
    void test_installAndUninstallSlotsOnlyReactToConnectivityDesktop();
    void test_applicationInfoSignalsDriveInstallState();
    void test_activeGuardsDisabledAndNotAdded();
    void test_activeClickLaunchesConnectivity();
    void test_activeDefaultAlsoLaunchesConnectivity();
    void test_activeIgnoresLongClickAndMenuRequest();

private:
    int removeGeneratedGcovFiles() const;
    bool ensureFakeServiceAvailable();
    void waitForAsync() const;

private:
    FakeAppLauncherService m_service;
    bool m_fakeServiceAvailable = false;
};

} // namespace

void ConnectivityShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    m_fakeServiceAvailable = m_service.registerService();
}

void ConnectivityShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void ConnectivityShortcutTest::cleanup()
{
    m_service.reset();
    if (!m_service.registered) {
        m_fakeServiceAvailable = m_service.registerService();
    }
    removeGeneratedGcovFiles();
}

int ConnectivityShortcutTest::removeGeneratedGcovFiles() const
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

bool ConnectivityShortcutTest::ensureFakeServiceAvailable()
{
    if (!m_service.registered) {
        m_fakeServiceAvailable = m_service.registerService();
    }

    return m_fakeServiceAvailable;
}

void ConnectivityShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void ConnectivityShortcutTest::test_pluginExposesBasicMetadata()
{
    ConnectivityShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("ConnectivityShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("connectivity-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("ConnectivityShortcut"));
}

void ConnectivityShortcutTest::test_shortcutInitialStateAndMetadata()
{
    ConnectivityShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("ConnectivityShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Connectivity"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Connectivity"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(shortcut.isEnable(), QFile::exists(QString::fromLatin1(kConnectivityDesktop)));
    QVERIFY(shortcut.m_applicationInfo != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 14);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 14);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void ConnectivityShortcutTest::test_setAddedStateOnlyChangesWhenNeeded()
{
    ConnectivityShortcut shortcut;
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

void ConnectivityShortcutTest::test_installAndUninstallSlotsOnlyReactToConnectivityDesktop()
{
    ConnectivityShortcut shortcut;
    shortcut.m_isEnable = true;
    QSignalSpy enableSpy(&shortcut, &ConnectivityShortcut::enableStatusChanged);

    shortcut.onAppUninstalled(QStringList{QStringLiteral("/tmp/other.desktop")});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);

    shortcut.onAppUninstalled(QStringList{QString::fromLatin1(kConnectivityDesktop)});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);

    shortcut.onAppInstalled(QStringList{QStringLiteral("/tmp/other.desktop")});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);

    shortcut.onAppInstalled(QStringList{QString::fromLatin1(kConnectivityDesktop)});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
}

void ConnectivityShortcutTest::test_applicationInfoSignalsDriveInstallState()
{
    ConnectivityShortcut shortcut;
    shortcut.m_isEnable = true;
    QSignalSpy enableSpy(&shortcut, &ConnectivityShortcut::enableStatusChanged);

    Q_EMIT shortcut.m_applicationInfo->appDBItems2BDelete(QStringList{QString::fromLatin1(kConnectivityDesktop)});
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    Q_EMIT shortcut.m_applicationInfo->appDBItems2BAdd(QStringList{QString::fromLatin1(kConnectivityDesktop)});
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);
}

void ConnectivityShortcutTest::test_activeGuardsDisabledAndNotAdded()
{
    ConnectivityShortcut shortcut;

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
}

void ConnectivityShortcutTest::test_activeClickLaunchesConnectivity()
{
    if (!ensureFakeServiceAvailable()) {
        QSKIP("com.kylin.ProcessManager is already owned in current session");
    }

    ConnectivityShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.launchAppCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kConnectivityDesktop));
}

void ConnectivityShortcutTest::test_activeDefaultAlsoLaunchesConnectivity()
{
    if (!ensureFakeServiceAvailable()) {
        QSKIP("com.kylin.ProcessManager is already owned in current session");
    }

    ConnectivityShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_isAdded = true;

    shortcut.active(static_cast<PluginMetaType::Action>(999));
    waitForAsync();

    QCOMPARE(m_service.launchAppCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kConnectivityDesktop));
}

void ConnectivityShortcutTest::test_activeIgnoresLongClickAndMenuRequest()
{
    ConnectivityShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_isAdded = true;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.launchAppCount, 0);
}

QTEST_MAIN(ConnectivityShortcutTest)

#include "connectivity-shortcut-test.moc"
