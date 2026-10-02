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
#include <QSignalSpy>
#include <memory>

#include "QProcess"

#define QProcess TestQProcess
#define private public
#include "../../../ukui-shortcut/shortcuts/screenshot-shortcut/screenshot-shortcut.cpp"
#undef private
#undef QProcess

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kScreenshotDesktop[] = "/usr/share/applications/kylin-screenshot.desktop";
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

class ScreenShotShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateWhenInstalled();
    void test_shortcutInitialStateWhenMissing();
    void test_installStatusChangedIgnoresUnrelatedDesktop();
    void test_installStatusChangedUpdatesEnableState();
    void test_applicationInfoSignalsDriveInstallState();
    void test_kyscreenshotbeginGuardsDisabled();
    void test_activeClickAndDefaultLaunchScreenshot();
    void test_activeIgnoresLongClickAndMenuRequest();
    void test_kyscreenshotbeginFallsBackToProcessWhenDbusFails();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;
    void setScreenshotDesktopPath(const QString &path) const;

private:
    FakeAppLauncherService m_service;
};

} // namespace

void ScreenShotShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void ScreenShotShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void ScreenShotShortcutTest::cleanup()
{
    m_service.reset();
    TestQProcess::resetTestData();
    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int ScreenShotShortcutTest::removeGeneratedGcovFiles() const
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

void ScreenShotShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void ScreenShotShortcutTest::setScreenshotDesktopPath(const QString &path) const
{
    UkuiSearch::ApplicationInfo::setDesktopFilePath(QStringLiteral(KYLIN_SCREENSHOT), path);
}

void ScreenShotShortcutTest::test_pluginExposesBasicMetadata()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("ScreenShot"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("screenshot-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("ScreenShot"));
}

void ScreenShotShortcutTest::test_shortcutInitialStateWhenInstalled()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));

    ScreenShotShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("ScreenShot"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Screen Shot"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Screen Shot"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("screenshot-app-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kScreenshotDesktop));
    QVERIFY(shortcut.m_applicationInfo != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 6);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 6);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void ScreenShotShortcutTest::test_shortcutInitialStateWhenMissing()
{
    ScreenShotShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
}

void ScreenShotShortcutTest::test_installStatusChangedIgnoresUnrelatedDesktop()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ScreenShotShortcut::enableStatusChanged);

    shortcut.onAppInstallStatusChanged(QStringList{QStringLiteral("/tmp/other.desktop")});

    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
}

void ScreenShotShortcutTest::test_installStatusChangedUpdatesEnableState()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ScreenShotShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    shortcut.onAppInstallStatusChanged(QStringList{QStringLiteral("/usr/share/applications/kylin-screenshot.desktop")});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);

    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    shortcut.onAppInstallStatusChanged(QStringList{QStringLiteral("kylin-screenshot.desktop")});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kScreenshotDesktop));
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);

    shortcut.checkAppInstallState();
    QCOMPARE(enableSpy.count(), 0);
}

void ScreenShotShortcutTest::test_applicationInfoSignalsDriveInstallState()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &ScreenShotShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BDelete(QStringList{QStringLiteral("kylin-screenshot.desktop")});
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BAdd(QStringList{QStringLiteral("kylin-screenshot.desktop")});
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);
}

void ScreenShotShortcutTest::test_kyscreenshotbeginGuardsDisabled()
{
    ScreenShotShortcut shortcut;

    shortcut.kyscreenshotbegin();
    waitForAsync();

    QCOMPARE(m_service.launchAppWithArgumentsCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void ScreenShotShortcutTest::test_activeClickAndDefaultLaunchScreenshot()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.launchAppWithArgumentsCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kScreenshotDesktop));
    QCOMPARE(m_service.lastArguments, QStringList());
    QCOMPARE(TestQProcess::startDetachedCount, 0);

    m_service.reset();
    shortcut.active(static_cast<PluginMetaType::Action>(99));
    waitForAsync();
    QCOMPARE(m_service.launchAppWithArgumentsCount, 1);
}

void ScreenShotShortcutTest::test_activeIgnoresLongClickAndMenuRequest()
{
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.launchAppWithArgumentsCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void ScreenShotShortcutTest::test_kyscreenshotbeginFallsBackToProcessWhenDbusFails()
{
    QDBusConnection::sessionBus().unregisterObject(QString::fromLatin1(kAppManagerPath));
    setScreenshotDesktopPath(QString::fromLatin1(kScreenshotDesktop));
    ScreenShotShortcut shortcut;
    TestQProcess::nextStartDetachedResult = true;

    shortcut.kyscreenshotbegin();

    QTRY_COMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("kylin-screenshot"));
    QCOMPARE(TestQProcess::lastArguments, QStringList());
}

QTEST_MAIN(ScreenShotShortcutTest)

#include "screenshot-shortcut-test.moc"
