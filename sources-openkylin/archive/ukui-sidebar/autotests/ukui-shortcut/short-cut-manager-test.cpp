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
#include <QFileInfo>
#include <QMetaType>
#include <QTemporaryDir>

#define private public
#include "short-cut-manager.h"
#include "../../ukui-shortcut/short-cut-manager.cpp"
#undef private

using namespace UkuiShortcut;

class shortCutManager_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_emptyDirectory_returnsEmptyList();
    void test_findShortcuts_filtersInvalidPluginsAndLoadsValidShortcut();
    void test_findShortcuts_appendsWhenCalledMultipleTimes();
    void test_getWidgetInfo_returnsMatchAndMiss();
    void test_getInstance_returnsSingleton();

private:
    QDir pluginDir() const;
    void clearSingleton();
    int removeGeneratedGcovFiles() const;
};

void shortCutManager_test::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    QVERIFY(QDir(QFINDTESTDATA("fake-plugins")).exists());
}

void shortCutManager_test::cleanupTestCase()
{
    clearSingleton();
    removeGeneratedGcovFiles();
}

void shortCutManager_test::cleanup()
{
    clearSingleton();
    removeGeneratedGcovFiles();
}

QDir shortCutManager_test::pluginDir() const
{
    QDir dir(QCoreApplication::applicationDirPath() + QStringLiteral("/short-cut-manager-test-plugins"));
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    return dir;
}

void shortCutManager_test::clearSingleton()
{
    delete instance;
    instance = nullptr;
}

int shortCutManager_test::removeGeneratedGcovFiles() const
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

void shortCutManager_test::test_emptyDirectory_returnsEmptyList()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ShortcutManagerPrivate managerPrivate;
    managerPrivate.findShortcuts(QDir(dir.path()), QStringLiteral("/not-exists"));

    QVERIFY(managerPrivate.getShortcuts().isEmpty());
}

void shortCutManager_test::test_findShortcuts_filtersInvalidPluginsAndLoadsValidShortcut()
{
    ShortcutManagerPrivate managerPrivate;
    managerPrivate.findShortcuts(pluginDir(), QStringLiteral("/not-exists"));

    const QList<Shortcut *> shortcuts = managerPrivate.getShortcuts();
    QCOMPARE(shortcuts.count(), 1);

    Shortcut *shortcut = shortcuts.first();
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("fake.shortcut"));

    const StatusInfo info = shortcut->currentStatus();
    QCOMPARE(info.getWidgetId(), QStringLiteral("fake.widget"));
    QCOMPARE(info.getName(), QStringLiteral("Fake Shortcut"));

    qDeleteAll(shortcuts);
}

void shortCutManager_test::test_findShortcuts_appendsWhenCalledMultipleTimes()
{
    ShortcutManagerPrivate managerPrivate;
    managerPrivate.findShortcuts(pluginDir(), QStringLiteral("/not-exists"));
    managerPrivate.findShortcuts(pluginDir(), QStringLiteral("/not-exists"));

    const QList<Shortcut *> shortcuts = managerPrivate.getShortcuts();
    QCOMPARE(shortcuts.count(), 2);

    qDeleteAll(shortcuts);
}

void shortCutManager_test::test_getWidgetInfo_returnsMatchAndMiss()
{
    ShortcutManager manager;
    qDeleteAll(manager.d->m_allShortcuts);
    manager.d->m_allShortcuts.clear();
    manager.d->findShortcuts(pluginDir(), QStringLiteral("/not-exists"));
    QCOMPARE(manager.getShortcuts().count(), 1);

    const QPair<bool, QString> match = manager.getWidgetInfo(QStringLiteral("fake.widget"));
    QVERIFY(match.first);
    QCOMPARE(match.second, QStringLiteral("Fake Shortcut"));

    const QPair<bool, QString> miss = manager.getWidgetInfo(QStringLiteral("missing.widget"));
    QVERIFY(!miss.first);
    QVERIFY(miss.second.isEmpty());
}

void shortCutManager_test::test_getInstance_returnsSingleton()
{
    ShortcutManager *first = ShortcutManager::getInstance();
    ShortcutManager *second = ShortcutManager::getInstance();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
    QCOMPARE(first->getShortcuts().count(), 1);
}

QTEST_MAIN(shortCutManager_test)
#include "short-cut-manager-test.moc"
