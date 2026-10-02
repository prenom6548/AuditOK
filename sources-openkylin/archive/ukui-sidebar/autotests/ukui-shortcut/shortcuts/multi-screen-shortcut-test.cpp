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
#include <memory>

#include "QProcess"

#define QProcess TestQProcess
#define private public
#include "../../../ukui-shortcut/shortcuts/multi-screen-shortcut/multi-screen-shortcut.cpp"
#undef private
#undef QProcess

using namespace UkuiShortcut;

namespace {

class MultiScreenShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateAndMetadata();
    void test_activeIgnoresUnsupportedActions();
    void test_activeClickStartsDisplaySwitch();
    void test_activeClickHandlesStartFailure();

private:
    int removeGeneratedGcovFiles() const;
};

} // namespace

void MultiScreenShortcutTest::init()
{
    TestQProcess::resetTestData();
}

void MultiScreenShortcutTest::cleanup()
{
    TestQProcess::resetTestData();
    removeGeneratedGcovFiles();
}

int MultiScreenShortcutTest::removeGeneratedGcovFiles() const
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

void MultiScreenShortcutTest::test_pluginExposesBasicMetadata()
{
    MultiScreenShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("MultiScreenShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("multi-screen-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("MultiScreenShortcut"));
}

void MultiScreenShortcutTest::test_shortcutInitialStateAndMetadata()
{
    MultiScreenShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("MultiScreenShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Multi-Screen"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Multi-Screen"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-projection-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 4);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 4);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void MultiScreenShortcutTest::test_activeIgnoresUnsupportedActions()
{
    MultiScreenShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);

    QCOMPARE(TestQProcess::startDetachedCount, 0);
    QVERIFY(TestQProcess::lastProgram.isEmpty());
}

void MultiScreenShortcutTest::test_activeClickStartsDisplaySwitch()
{
    MultiScreenShortcut shortcut;

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("ukydisplayswitch"));
}

void MultiScreenShortcutTest::test_activeClickHandlesStartFailure()
{
    MultiScreenShortcut shortcut;
    TestQProcess::nextStartDetachedResult = false;

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(TestQProcess::startDetachedCount, 1);
    QCOMPARE(TestQProcess::lastProgram, QStringLiteral("ukydisplayswitch"));
}

QTEST_MAIN(MultiScreenShortcutTest)

#include "multi-screen-shortcut-test.moc"
