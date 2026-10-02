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
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSignalSpy>
#include <memory>

#include "QProcess"

class TestQFile
{
public:
    static bool exists(const QString &path)
    {
        return s_existingFiles.contains(path);
    }

    static void setExists(const QString &path, bool exists)
    {
        if (exists) {
            s_existingFiles.insert(path);
        } else {
            s_existingFiles.remove(path);
        }
    }

    static void resetTestData()
    {
        s_existingFiles.clear();
    }

private:
    inline static QSet<QString> s_existingFiles;
};

#define QProcess TestQProcess
#define QFile TestQFile
#define private public
#include "../../../ukui-shortcut/shortcuts/services-and-supports-shortcut/services-and-supports-shortcut.cpp"
#undef private
#undef QFile
#undef QProcess

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kServiceSupportDesktop[] = "/usr/share/applications/kylin-os-manager.desktop";
const char kAppManagerService[] = "com.kylin.ProcessManager";
const char kAppManagerPath[] = "/com/kylin/ProcessManager/AppLauncher";

class FakeAppLauncherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")
public:
    QString lastDesktopFile;
    QStringList lastArguments;
    int launchAppWithArgumentsCount = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kAppManagerPath));
        bus.unregisterService(QString::fromLatin1(kAppManagerService));
        if (!bus.registerService(QString::fromLatin1(kAppManagerService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kAppManagerPath),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kAppManagerPath));
        bus.unregisterService(QString::fromLatin1(kAppManagerService));
        registered = false;
    }

    void reset()
    {
        lastDesktopFile.clear();
        lastArguments.clear();
        launchAppWithArgumentsCount = 0;
    }

public Q_SLOTS:
    bool LaunchAppWithArguments(const QString &desktopFile, const QStringList &arguments)
    {
        lastDesktopFile = desktopFile;
        lastArguments = arguments;
        ++launchAppWithArgumentsCount;
        return true;
    }
};

class ServicesAndSupportsShortcutTest : public QObject
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
    void test_installAndUninstallSlotsIgnoreUnrelatedDesktop();
    void test_installAndUninstallSlotsUpdateEnableState();
    void test_applicationInfoSignalsDriveInstallState();
    void test_openServicesAndSupportsGuardsDisabledAndNotAdded();
    void test_activeClickAndDefaultLaunchServiceSupport();
    void test_activeIgnoresLongClickAndMenuRequest();
    void test_openServicesAndSupportsFallsBackToProcessWhenDbusFails();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;
    void setServiceSupportInstalled(bool installed) const;

private:
    FakeAppLauncherService m_service;
};

} // namespace

void ServicesAndSupportsShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void ServicesAndSupportsShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void ServicesAndSupportsShortcutTest::cleanup()
{
    m_service.reset();
    TestQFile::resetTestData();
    TestQProcess::resetTestData();
    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int ServicesAndSupportsShortcutTest::removeGeneratedGcovFiles() const
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

void ServicesAndSupportsShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void ServicesAndSupportsShortcutTest::setServiceSupportInstalled(bool installed) const
{
    TestQFile::setExists(QString::fromLatin1(kServiceSupportDesktop), installed);
}

void ServicesAndSupportsShortcutTest::test_pluginExposesBasicMetadata()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("ServicesAndSupportsShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("services-and-supports-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("ServicesAndSupportsShortcut"));
}

void ServicesAndSupportsShortcutTest::test_shortcutInitialStateWhenInstalled()
{
    setServiceSupportInstalled(true);

    ServicesAndSupportsShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("ServicesAndSupportsShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Services and Supports"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Services and Supports"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_applicationInfo != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 13);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 13);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void ServicesAndSupportsShortcutTest::test_shortcutInitialStateWhenMissing()
{
    ServicesAndSupportsShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
}

void ServicesAndSupportsShortcutTest::test_setAddedStateOnlyChangesWhenNeeded()
{
    ServicesAndSupportsShortcut shortcut;
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

void ServicesAndSupportsShortcutTest::test_installAndUninstallSlotsIgnoreUnrelatedDesktop()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ServicesAndSupportsShortcut::enableStatusChanged);

    shortcut.onAppUninstalled(QStringList{QStringLiteral("/tmp/other.desktop")});
    shortcut.onAppInstalled(QStringList{QStringLiteral("/tmp/other.desktop")});

    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
}

void ServicesAndSupportsShortcutTest::test_installAndUninstallSlotsUpdateEnableState()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ServicesAndSupportsShortcut::enableStatusChanged);

    shortcut.onAppUninstalled(QStringList{QString::fromLatin1(kServiceSupportDesktop)});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);

    shortcut.onAppInstalled(QStringList{QString::fromLatin1(kServiceSupportDesktop)});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
}

void ServicesAndSupportsShortcutTest::test_applicationInfoSignalsDriveInstallState()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ServicesAndSupportsShortcut::enableStatusChanged);

    Q_EMIT shortcut.m_applicationInfo->appDBItems2BDelete(QStringList{QString::fromLatin1(kServiceSupportDesktop)});
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    Q_EMIT shortcut.m_applicationInfo->appDBItems2BAdd(QStringList{QString::fromLatin1(kServiceSupportDesktop)});
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);
}

void ServicesAndSupportsShortcutTest::test_openServicesAndSupportsGuardsDisabledAndNotAdded()
{
    ServicesAndSupportsShortcut disabledShortcut;
    disabledShortcut.setAddedState(true);
    disabledShortcut.openServicesAndSupports();
    waitForAsync();
    QCOMPARE(m_service.launchAppWithArgumentsCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);

    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut notAddedShortcut;
    notAddedShortcut.openServicesAndSupports();
    waitForAsync();
    QCOMPARE(m_service.launchAppWithArgumentsCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void ServicesAndSupportsShortcutTest::test_activeClickAndDefaultLaunchServiceSupport()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.launchAppWithArgumentsCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kServiceSupportDesktop));
    QCOMPARE(m_service.lastArguments, QStringList{QStringLiteral("-jumpTab=ServiceSupport")});
    QCOMPARE(TestQProcess::startDetachedCount, 0);

    m_service.reset();
    shortcut.active(static_cast<PluginMetaType::Action>(99));
    waitForAsync();
    QCOMPARE(m_service.launchAppWithArgumentsCount, 1);
}

void ServicesAndSupportsShortcutTest::test_activeIgnoresLongClickAndMenuRequest()
{
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.launchAppWithArgumentsCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void ServicesAndSupportsShortcutTest::test_openServicesAndSupportsFallsBackToProcessWhenDbusFails()
{
    QDBusConnection::sessionBus().unregisterObject(QString::fromLatin1(kAppManagerPath));
    setServiceSupportInstalled(true);
    ServicesAndSupportsShortcut shortcut;
    shortcut.setAddedState(true);

    shortcut.openServicesAndSupports();

    QTRY_COMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("/usr/bin/kylin-os-manager -jumpTab=ServiceSupport"));
}

QTEST_MAIN(ServicesAndSupportsShortcutTest)

#include "services-and-supports-shortcut-test.moc"
