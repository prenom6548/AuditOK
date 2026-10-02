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
#include "../../../ukui-shortcut/shortcuts/dark-mode-shortcut/dark-mode-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const QByteArray kUkuiStyleSchema("org.ukui.style");
const QByteArray kGtkStyleSchema("org.mate.interface");
const QString kUkuiStyleKey("styleName");
const QString kGtkStyleKey("gtkTheme");

class DarkModeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_initialStateWhenSchemasAndKeysExist();
    void test_constructorDisablesWhenUkuiSchemaMissing();
    void test_constructorDisablesWhenUkuiKeyMissing();
    void test_constructorToleratesGtkSchemaOrKeyMissing();
    void test_setIconButtonColorForDarkLightAndUnknown();
    void test_setAddedStateGuardsNullSettingsAndEmitsOnChanges();
    void test_onQGSettingsChangedOnlyReactsToStyleName();
    void test_triggerTogglesUkuiAndGtkThemes();
    void test_triggerGuardsDisabledNotAddedAndMissingKeys();
    void test_activeOnlyHandlesClick();

private:
    int removeGeneratedGcovFiles() const;
    void installCompleteSettings(const QString &ukuiStyle = QStringLiteral("ukui-light"),
                                 const QString &gtkStyle = QStringLiteral("ukui-white")) const;
};

} // namespace

void DarkModeShortcutTest::init()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    QGSettings::resetTestData();
}

void DarkModeShortcutTest::cleanup()
{
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

int DarkModeShortcutTest::removeGeneratedGcovFiles() const
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

void DarkModeShortcutTest::installCompleteSettings(const QString &ukuiStyle, const QString &gtkStyle) const
{
    QGSettings::installSchema(kUkuiStyleSchema, QStringList{kUkuiStyleKey});
    QGSettings::installSchema(kGtkStyleSchema, QStringList{kGtkStyleKey});
    QGSettings::setValue(kUkuiStyleSchema, kUkuiStyleKey, ukuiStyle);
    QGSettings::setValue(kGtkStyleSchema, kGtkStyleKey, gtkStyle);
}

void DarkModeShortcutTest::test_pluginExposesBasicMetadata()
{
    installCompleteSettings();
    DarkModeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("DarkModeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("dark-mode-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("DarkModeShortcut"));
}

void DarkModeShortcutTest::test_initialStateWhenSchemasAndKeysExist()
{
    installCompleteSettings(QStringLiteral("ukui-dark"), QStringLiteral("ukui-black"));

    DarkModeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("DarkModeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Dark Mode"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Dark Mode"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_gsettingsUkui != nullptr);
    QVERIFY(shortcut.m_gsettingsGtk != nullptr);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 8);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 8);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void DarkModeShortcutTest::test_constructorDisablesWhenUkuiSchemaMissing()
{
    QGSettings::installSchema(kGtkStyleSchema, QStringList{kGtkStyleKey});

    DarkModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettingsUkui == nullptr);
    QVERIFY(shortcut.m_gsettingsGtk != nullptr);
}

void DarkModeShortcutTest::test_constructorDisablesWhenUkuiKeyMissing()
{
    QGSettings::installSchema(kUkuiStyleSchema, QStringList{QStringLiteral("otherKey")});
    QGSettings::installSchema(kGtkStyleSchema, QStringList{kGtkStyleKey});

    DarkModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettingsUkui != nullptr);
}

void DarkModeShortcutTest::test_constructorToleratesGtkSchemaOrKeyMissing()
{
    QGSettings::installSchema(kUkuiStyleSchema, QStringList{kUkuiStyleKey});
    QGSettings::setValue(kUkuiStyleSchema, kUkuiStyleKey, QStringLiteral("ukui-light"));

    {
        DarkModeShortcut shortcut;
        QVERIFY(shortcut.isEnable());
        QVERIFY(shortcut.m_gsettingsGtk == nullptr);
    }

    QGSettings::installSchema(kGtkStyleSchema, QStringList{QStringLiteral("otherKey")});
    DarkModeShortcut shortcut;
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_gsettingsGtk != nullptr);
}

void DarkModeShortcutTest::test_setIconButtonColorForDarkLightAndUnknown()
{
    installCompleteSettings(QStringLiteral("ukui-dark"));
    DarkModeShortcut shortcut;

    shortcut.setIconButtonColor();
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);

    shortcut.m_gsettingsUkui->set(kUkuiStyleKey, QStringLiteral("ukui-light"));
    shortcut.setIconButtonColor();
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);

    shortcut.m_currentStatus.setColor(Color::ColorRole::HighLight);
    shortcut.m_gsettingsUkui->set(kUkuiStyleKey, QStringLiteral("custom"));
    shortcut.setIconButtonColor();
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void DarkModeShortcutTest::test_setAddedStateGuardsNullSettingsAndEmitsOnChanges()
{
    DarkModeShortcut noSettingsShortcut;
    QSignalSpy noSettingsSpy(&noSettingsShortcut, &DarkModeShortcut::statusChanged);
    noSettingsShortcut.setAddedState(true);
    QVERIFY(noSettingsShortcut.m_isAdded);
    QCOMPARE(noSettingsSpy.count(), 0);

    installCompleteSettings(QStringLiteral("ukui-dark"));
    DarkModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &DarkModeShortcut::statusChanged);

    shortcut.setAddedState(false);
    QVERIFY(!shortcut.m_isAdded);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.setAddedState(true);
    QVERIFY(shortcut.m_isAdded);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.setAddedState(true);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.setAddedState(false);
    QVERIFY(!shortcut.m_isAdded);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);
}

void DarkModeShortcutTest::test_onQGSettingsChangedOnlyReactsToStyleName()
{
    installCompleteSettings(QStringLiteral("ukui-light"));
    DarkModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &DarkModeShortcut::statusChanged);

    shortcut.onQGSettingsChanged(QStringLiteral("otherKey"));
    QCOMPARE(statusSpy.count(), 0);

    shortcut.m_gsettingsUkui->set(kUkuiStyleKey, QStringLiteral("ukui-dark"));
    shortcut.onQGSettingsChanged(kUkuiStyleKey);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);
}

void DarkModeShortcutTest::test_triggerTogglesUkuiAndGtkThemes()
{
    installCompleteSettings(QStringLiteral("ukui-dark"), QStringLiteral("ukui-black"));
    DarkModeShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_isAdded = true;

    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-light"));
    QCOMPARE(shortcut.m_gsettingsGtk->get(kGtkStyleKey).toString(), QStringLiteral("ukui-white"));

    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-dark"));
    QCOMPARE(shortcut.m_gsettingsGtk->get(kGtkStyleKey).toString(), QStringLiteral("ukui-black"));
}

void DarkModeShortcutTest::test_triggerGuardsDisabledNotAddedAndMissingKeys()
{
    installCompleteSettings(QStringLiteral("ukui-light"), QStringLiteral("ukui-white"));
    DarkModeShortcut shortcut;

    shortcut.m_isEnable = false;
    shortcut.m_isAdded = true;
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-light"));

    shortcut.m_isEnable = true;
    shortcut.m_isAdded = false;
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-light"));

    QGSettings::resetTestData();
    QGSettings::installSchema(kUkuiStyleSchema, QStringList{QStringLiteral("otherKey")});
    QGSettings::installSchema(kGtkStyleSchema, QStringList{QStringLiteral("otherKey")});
    DarkModeShortcut missingKeysShortcut;
    missingKeysShortcut.m_isEnable = true;
    missingKeysShortcut.m_isAdded = true;
    missingKeysShortcut.trigger();
    QCOMPARE(missingKeysShortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QString());
    QCOMPARE(missingKeysShortcut.m_gsettingsGtk->get(kGtkStyleKey).toString(), QString());
}

void DarkModeShortcutTest::test_activeOnlyHandlesClick()
{
    installCompleteSettings(QStringLiteral("ukui-light"), QStringLiteral("ukui-white"));
    DarkModeShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_isAdded = true;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-light"));

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(shortcut.m_gsettingsUkui->get(kUkuiStyleKey).toString(), QStringLiteral("ukui-dark"));
}

QTEST_MAIN(DarkModeShortcutTest)

#include "dark-mode-shortcut-test.moc"
