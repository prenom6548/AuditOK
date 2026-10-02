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
#include "../../../ukui-shortcut/shortcuts/notebook-shortcut/notebook-shortcut.cpp"
#undef private
#undef QProcess

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kNotebookDesktop[] = "/usr/share/applications/ukui-notebook.desktop";
const char kAppManagerService[] = "com.kylin.ProcessManager";
const char kAppManagerPath[] = "/com/kylin/ProcessManager/AppLauncher";

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

class NotebookShortcutTest : public QObject
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
    void test_getDesktopPathUsesApplicationInfo();
    void test_installAndUninstallSlotsIgnoreUnrelatedDesktop();
    void test_installAndUninstallSlotsUpdateEnableState();
    void test_applicationInfoSignalsDriveInstallState();
    void test_activeGuardsDisabledNotAddedAndUnsupportedAction();
    void test_activeClickLaunchesNotebook();
    void test_activeClickFallsBackToProcessWhenDbusFails();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;
    void setNotebookDesktopPath(const QString &path) const;

private:
    FakeAppLauncherService m_service;
};

} // namespace

void NotebookShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void NotebookShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void NotebookShortcutTest::cleanup()
{
    m_service.reset();
    TestQProcess::resetTestData();
    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    if (!m_service.registered) {
        QVERIFY(m_service.registerService());
    }
    removeGeneratedGcovFiles();
}

int NotebookShortcutTest::removeGeneratedGcovFiles() const
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

void NotebookShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

void NotebookShortcutTest::setNotebookDesktopPath(const QString &path) const
{
    UkuiSearch::ApplicationInfo::setDesktopFilePath(QStringLiteral("ukui-notebook"), path);
}

void NotebookShortcutTest::test_pluginExposesBasicMetadata()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("Notebook"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("notebook-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("Notebook"));
}

void NotebookShortcutTest::test_shortcutInitialStateWhenInstalled()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));

    NotebookShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("Notebook"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Notebook"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Notebook"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kNotebookDesktop));
    QVERIFY(shortcut.m_applicationInfo != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 11);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 11);
    QCOMPARE(meta.value(PluginMetaType::PC).preAction(), PluginMetaType::Hide);
    QCOMPARE(meta.value(PluginMetaType::Tablet).preAction(), PluginMetaType::Hide);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void NotebookShortcutTest::test_shortcutInitialStateWhenMissing()
{
    NotebookShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
}

void NotebookShortcutTest::test_setAddedStateOnlyChangesWhenNeeded()
{
    NotebookShortcut shortcut;
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

void NotebookShortcutTest::test_getDesktopPathUsesApplicationInfo()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;

    QCOMPARE(shortcut.getDesktopPath(QStringLiteral("ukui-notebook")), QString::fromLatin1(kNotebookDesktop));
    QCOMPARE(shortcut.getDesktopPath(QStringLiteral("missing-app")), QString());
}

void NotebookShortcutTest::test_installAndUninstallSlotsIgnoreUnrelatedDesktop()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &NotebookShortcut::enableStatusChanged);

    shortcut.onAppUninstalled(QStringList{QStringLiteral("/tmp/other.desktop")});
    shortcut.onAppInstalled(QStringList{QStringLiteral("/tmp/other.desktop")});

    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
}

void NotebookShortcutTest::test_installAndUninstallSlotsUpdateEnableState()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &NotebookShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    shortcut.onAppUninstalled(QStringList{QStringLiteral("/usr/share/applications/ukui-notebook.desktop")});
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);

    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    shortcut.onAppInstalled(QStringList{QStringLiteral("ukui-notebook.desktop")});
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_appPath, QString::fromLatin1(kNotebookDesktop));
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);

    shortcut.checkAppInstallState();
    QCOMPARE(enableSpy.count(), 0);
}

void NotebookShortcutTest::test_applicationInfoSignalsDriveInstallState()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &NotebookShortcut::enableStatusChanged);

    UkuiSearch::ApplicationInfo::clearDesktopFilePaths();
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BDelete(QStringList{QStringLiteral("ukui-notebook.desktop")});
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    Q_EMIT shortcut.m_applicationInfo->appDBItems2BAdd(QStringList{QStringLiteral("ukui-notebook.desktop")});
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 2);
}

void NotebookShortcutTest::test_activeGuardsDisabledNotAddedAndUnsupportedAction()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;

    shortcut.m_isEnable = false;
    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);

    shortcut.m_isEnable = true;
    shortcut.m_isAdded = false;
    shortcut.active(PluginMetaType::Click);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);

    shortcut.m_isAdded = true;
    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();
    QCOMPARE(m_service.launchAppCount, 0);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void NotebookShortcutTest::test_activeClickLaunchesNotebook()
{
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);

    shortcut.active(PluginMetaType::Click);
    waitForAsync();

    QCOMPARE(m_service.launchAppCount, 1);
    QCOMPARE(m_service.lastDesktopFile, QString::fromLatin1(kNotebookDesktop));
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void NotebookShortcutTest::test_activeClickFallsBackToProcessWhenDbusFails()
{
    QDBusConnection::sessionBus().unregisterObject(QString::fromLatin1(kAppManagerPath));
    setNotebookDesktopPath(QString::fromLatin1(kNotebookDesktop));
    NotebookShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.setAddedState(true);
    TestQProcess::nextStartDetachedResult = false;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("ukui-notebook"));
}

QTEST_MAIN(NotebookShortcutTest)

#include "notebook-shortcut-test.moc"
