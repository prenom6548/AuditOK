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
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/soft-freeze-shortcut/soft-freeze-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const QByteArray kSoftFreezeSchema("org.ukui.process-manager.soft-freeze-mode");
const QString kFreezeStateKey("enabled");

class SoftFreezeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void cleanupTestCase();

    void test_pluginExposesBasicMetadata();
    void test_initialStateWhenFreezeDisabled();
    void test_initialStateWhenFreezeEnabled();
    void test_constructorDisablesWhenSchemaMissing();
    void test_constructorDisablesWhenKeyMissing();
    void test_isEnableAlwaysReturnsFalseForCurrentProductPolicy();
    void test_activeClickTogglesFreezeStateWhenInternallyEnabled();
    void test_activeGuardsNonClickDisabledAndMissingSettings();
    void test_freezeStateChangedUpdatesOnlyOnStateChange();
    void test_gsettingsChangedOnlyReactsToFreezeStateKey();

private:
    int removeGeneratedGcovFiles() const;
    void installSettings(bool enabled) const;
};

} // namespace

void SoftFreezeShortcutTest::init()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

void SoftFreezeShortcutTest::cleanup()
{
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

void SoftFreezeShortcutTest::cleanupTestCase()
{
    removeGeneratedGcovFiles();
}

int SoftFreezeShortcutTest::removeGeneratedGcovFiles() const
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

void SoftFreezeShortcutTest::installSettings(bool enabled) const
{
    QGSettings::installSchema(kSoftFreezeSchema, QStringList{kFreezeStateKey});
    QGSettings::setValue(kSoftFreezeSchema, kFreezeStateKey, enabled);
}

void SoftFreezeShortcutTest::test_pluginExposesBasicMetadata()
{
    installSettings(false);
    SoftFreezeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("SoftFreeze"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("soft-freeze-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("SoftFreeze"));
}

void SoftFreezeShortcutTest::test_initialStateWhenFreezeDisabled()
{
    installSettings(false);

    SoftFreezeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("SoftFreeze"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Soft Freeze"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Soft Freeze"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.m_gsettings != nullptr);
    QVERIFY(shortcut.m_isEnable);
    QVERIFY(!shortcut.m_state);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 7);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 7);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(!meta.value(PluginMetaType::Tablet).isVisible());
}

void SoftFreezeShortcutTest::test_initialStateWhenFreezeEnabled()
{
    installSettings(true);

    SoftFreezeShortcut shortcut;

    QVERIFY(shortcut.m_isEnable);
    QVERIFY(shortcut.m_state);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void SoftFreezeShortcutTest::test_constructorDisablesWhenSchemaMissing()
{
    SoftFreezeShortcut shortcut;

    QVERIFY(!shortcut.m_isEnable);
    QVERIFY(shortcut.m_gsettings == nullptr);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
}

void SoftFreezeShortcutTest::test_constructorDisablesWhenKeyMissing()
{
    QGSettings::installSchema(kSoftFreezeSchema, QStringList{QStringLiteral("otherKey")});

    SoftFreezeShortcut shortcut;

    QVERIFY(!shortcut.m_isEnable);
    QVERIFY(shortcut.m_gsettings != nullptr);
}

void SoftFreezeShortcutTest::test_isEnableAlwaysReturnsFalseForCurrentProductPolicy()
{
    installSettings(true);
    SoftFreezeShortcut shortcut;

    QVERIFY(shortcut.m_isEnable);
    QVERIFY(!shortcut.isEnable());
}

void SoftFreezeShortcutTest::test_activeClickTogglesFreezeStateWhenInternallyEnabled()
{
    installSettings(false);
    SoftFreezeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &SoftFreezeShortcut::statusChanged);

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(shortcut.m_gsettings->get(kFreezeStateKey).toBool(), true);
    QVERIFY(shortcut.m_state);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(shortcut.m_gsettings->get(kFreezeStateKey).toBool(), false);
    QVERIFY(!shortcut.m_state);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);
}

void SoftFreezeShortcutTest::test_activeGuardsNonClickDisabledAndMissingSettings()
{
    installSettings(false);
    SoftFreezeShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(shortcut.m_gsettings->get(kFreezeStateKey).toBool(), false);

    shortcut.m_isEnable = false;
    shortcut.active(PluginMetaType::Click);
    QCOMPARE(shortcut.m_gsettings->get(kFreezeStateKey).toBool(), false);

    QGSettings::resetTestData();
    SoftFreezeShortcut noSettingsShortcut;
    noSettingsShortcut.active(PluginMetaType::Click);
    QVERIFY(noSettingsShortcut.m_gsettings == nullptr);
}

void SoftFreezeShortcutTest::test_freezeStateChangedUpdatesOnlyOnStateChange()
{
    installSettings(false);
    SoftFreezeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &SoftFreezeShortcut::statusChanged);

    shortcut.freezeStateChanged(false);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.freezeStateChanged(true);
    QVERIFY(shortcut.m_state);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.freezeStateChanged(true);
    QCOMPARE(statusSpy.count(), 1);
}

void SoftFreezeShortcutTest::test_gsettingsChangedOnlyReactsToFreezeStateKey()
{
    installSettings(false);
    SoftFreezeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &SoftFreezeShortcut::statusChanged);

    shortcut.m_gsettings->set(QStringLiteral("otherKey"), true);
    QCOMPARE(statusSpy.count(), 0);
    QVERIFY(!shortcut.m_state);

    shortcut.m_gsettings->set(kFreezeStateKey, true);
    QCOMPARE(statusSpy.count(), 1);
    QVERIFY(shortcut.m_state);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

QTEST_MAIN(SoftFreezeShortcutTest)

#include "soft-freeze-shortcut-test.moc"
