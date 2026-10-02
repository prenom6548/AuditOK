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

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#include "../stubs/QProcess"

#define QProcess TestQProcess
#define private public
#include "../../../ukui-shortcut/shortcuts/system-setting-shortcut/system-setting-shortcut.cpp"
#undef private
#undef QProcess

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const char kControlCenterDesktop[] = "/usr/share/applications/ukui-control-center.desktop";

class SystemSettingShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateAndMetadata();
    void test_launchAppReturnsFalseWhenServiceMissing();
    void test_launchAppCallsDbusWhenServiceAvailable();
    void test_triggerUsesDbusWhenLaunchSucceeds();
    void test_triggerFallsBackToProcessWhenDbusUnavailable();
    void test_triggerHandlesProcessFailure();
    void test_activeOnlyHandlesClick();

private:
    int removeGeneratedGcovFiles() const;
};

} // namespace

void SystemSettingShortcutTest::cleanupTestCase()
{
    removeGeneratedGcovFiles();
}

void SystemSettingShortcutTest::cleanup()
{
    QDBusInterface::resetTestData();
    TestQProcess::resetTestData();
    removeGeneratedGcovFiles();
}

int SystemSettingShortcutTest::removeGeneratedGcovFiles() const
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

void SystemSettingShortcutTest::test_pluginExposesBasicMetadata()
{
    SystemSettingShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("SystemSetting"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("system-setting-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("SystemSetting"));
}

void SystemSettingShortcutTest::test_shortcutInitialStateAndMetadata()
{
    SystemSettingShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("SystemSetting"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Settings"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Settings"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("system-settings-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), -1);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), -1);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void SystemSettingShortcutTest::test_launchAppReturnsFalseWhenServiceMissing()
{
    QDBusInterface::setNextValid(false);
    SystemSettingShortcut shortcut;

    QVERIFY(!shortcut.LaunchApp(QString::fromLatin1(kControlCenterDesktop)));
    QCOMPARE(QDBusInterface::createdCount(), 1);
    QCOMPARE(QDBusInterface::callCount(), 0);
}

void SystemSettingShortcutTest::test_launchAppCallsDbusWhenServiceAvailable()
{
    SystemSettingShortcut shortcut;

    QVERIFY(shortcut.LaunchApp(QString::fromLatin1(kControlCenterDesktop)));

    QCOMPARE(QDBusInterface::callCount(), 1);
    QCOMPARE(QDBusInterface::lastMethod(), QStringLiteral("LaunchApp"));
    QCOMPARE(QDBusInterface::lastDesktopFile(), QString::fromLatin1(kControlCenterDesktop));
}

void SystemSettingShortcutTest::test_triggerUsesDbusWhenLaunchSucceeds()
{
    SystemSettingShortcut shortcut;

    shortcut.trigger();

    QCOMPARE(QDBusInterface::callCount(), 1);
    QCOMPARE(TestQProcess::startDetachedCount, 0);
}

void SystemSettingShortcutTest::test_triggerFallsBackToProcessWhenDbusUnavailable()
{
    QDBusInterface::setNextValid(false);
    SystemSettingShortcut shortcut;

    shortcut.trigger();

    QCOMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("ukui-control-center"));
}

void SystemSettingShortcutTest::test_triggerHandlesProcessFailure()
{
    QDBusInterface::setNextValid(false);
    TestQProcess::nextStartDetachedResult = false;
    SystemSettingShortcut shortcut;

    shortcut.trigger();

    QCOMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("ukui-control-center"));
}

void SystemSettingShortcutTest::test_activeOnlyHandlesClick()
{
    SystemSettingShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(QDBusInterface::callCount(), 0);

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(QDBusInterface::callCount(), 1);
}

QTEST_MAIN(SystemSettingShortcutTest)

#include "system-setting-shortcut-test.moc"
