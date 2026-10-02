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
#include "../../../ukui-shortcut/shortcuts/night-mode-shortcut/night-mode-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const QByteArray kColorSchema("org.ukui.SettingsDaemon.plugins.color");
const QString kDarkModeKey("darkMode");

class NightModeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void cleanupTestCase();

    void test_pluginExposesBasicMetadata();
    void test_initialStateWhenDarkModeDisabled();
    void test_initialStateWhenDarkModeEnabled();
    void test_constructorDisablesWhenSchemaMissing();
    void test_constructorDisablesWhenKeyMissing();
    void test_activeClickTogglesDarkMode();
    void test_activeIgnoresNonClickAndMissingSettings();
    void test_gsettingsChangedUpdatesColorAndEmitsStatus();
    void test_gsettingsChangedIgnoresOtherKeys();

private:
    int removeGeneratedGcovFiles() const;
    void installSettings(bool darkMode) const;
};

} // namespace

void NightModeShortcutTest::init()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

void NightModeShortcutTest::cleanup()
{
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

void NightModeShortcutTest::cleanupTestCase()
{
    removeGeneratedGcovFiles();
}

int NightModeShortcutTest::removeGeneratedGcovFiles() const
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

void NightModeShortcutTest::installSettings(bool darkMode) const
{
    QGSettings::installSchema(kColorSchema, QStringList{kDarkModeKey});
    QGSettings::setValue(kColorSchema, kDarkModeKey, darkMode);
}

void NightModeShortcutTest::test_pluginExposesBasicMetadata()
{
    installSettings(false);
    NightModeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("NightModeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("night-mode-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("NightModeShortcut"));
}

void NightModeShortcutTest::test_initialStateWhenDarkModeDisabled()
{
    installSettings(false);

    NightModeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("NightModeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Night mode"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Night Mode"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-nightmode-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 3);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 3);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void NightModeShortcutTest::test_initialStateWhenDarkModeEnabled()
{
    installSettings(true);

    NightModeShortcut shortcut;

    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void NightModeShortcutTest::test_constructorDisablesWhenSchemaMissing()
{
    NightModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings == nullptr);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
}

void NightModeShortcutTest::test_constructorDisablesWhenKeyMissing()
{
    QGSettings::installSchema(kColorSchema, QStringList{QStringLiteral("otherKey")});

    NightModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);
}

void NightModeShortcutTest::test_activeClickTogglesDarkMode()
{
    installSettings(false);
    NightModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &NightModeShortcut::statusChanged);

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(shortcut.m_gsettings->get(kDarkModeKey).toBool(), true);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.active(PluginMetaType::Click);

    QCOMPARE(shortcut.m_gsettings->get(kDarkModeKey).toBool(), false);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);
}

void NightModeShortcutTest::test_activeIgnoresNonClickAndMissingSettings()
{
    installSettings(false);
    NightModeShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(shortcut.m_gsettings->get(kDarkModeKey).toBool(), false);

    QGSettings::resetTestData();
    NightModeShortcut noSettingsShortcut;
    noSettingsShortcut.active(PluginMetaType::Click);
    QVERIFY(noSettingsShortcut.m_gsettings == nullptr);
}

void NightModeShortcutTest::test_gsettingsChangedUpdatesColorAndEmitsStatus()
{
    installSettings(false);
    NightModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &NightModeShortcut::statusChanged);

    shortcut.m_gsettings->set(kDarkModeKey, true);

    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.m_gsettings->set(kDarkModeKey, false);

    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);
}

void NightModeShortcutTest::test_gsettingsChangedIgnoresOtherKeys()
{
    installSettings(false);
    NightModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &NightModeShortcut::statusChanged);

    shortcut.m_gsettings->set(QStringLiteral("otherKey"), true);

    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 0);
}

QTEST_MAIN(NightModeShortcutTest)

#include "night-mode-shortcut-test.moc"
