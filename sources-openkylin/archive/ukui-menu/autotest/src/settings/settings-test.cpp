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

#define private public
#include "../../../src/settings/settings.h"
#undef private

#include <QtTest>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "fakes/QGSettings"
#include "fakes/libkypackages.h"

using namespace UkuiMenu;

namespace {

constexpr auto UKUI_STYLE_SCHEMA = "org.ukui.style";
constexpr auto CONTROL_CENTER_SETTING = "org.ukui.control-center.personalise";
constexpr auto UKUI_MENU_SCHEMA = "org.ukui.menu.settings";

QStringList expectedDefaultFavoriteAppsFromSystemConfig()
{
    QStringList expected;

    if (QFile::exists(UKUI_MENU_GLOBAL_CONFIG_FILE)) {
        QSettings settings(UKUI_MENU_GLOBAL_CONFIG_FILE, QSettings::IniFormat);
        settings.beginGroup("Default Favorite Apps");
        QMap<int, QString> apps;
        for (const auto &key : settings.allKeys()) {
            apps.insert(key.toInt(), settings.value(key).toString());
        }
        settings.endGroup();

        for (const auto &app : apps) {
            expected.append(app);
        }
    }

    return expected;
}

}

class settings_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_settingModule_registersType();
    void test_globalSetting_defaultsWithoutSchemas();
    void test_globalSetting_readsSchemasAndConfig();
    void test_globalSetting_styleSchemaChangedSignal();
    void test_globalSetting_controlCenterChangedSignal();
    void test_globalSetting_sysModeChangedSlot();
    void test_globalSetting_systemAndFavoriteQueries();
    void test_menuSetting_defaultsWithoutSchema();
    void test_menuSetting_repairsMissingVersion();
    void test_menuSetting_repairsLegacyMajorVersion();
    void test_menuSetting_setAndUpdateValue();
};

void settings_test::init()
{
    FakeQGSettingsStore::reset();
    FakeKdkPackageStore::reset();
}

void settings_test::test_settingModule_registersType()
{
    SettingModule::defineModule("Ukui.Menu.Test", 1, 0);
    QVERIFY(QMetaType::type("GlobalSetting::Key") != QMetaType::UnknownType);
}

void settings_test::test_globalSetting_defaultsWithoutSchemas()
{
    GlobalSetting setting(nullptr);

    QCOMPARE(setting.get(GlobalSetting::StyleName).toString(), QStringLiteral(UKUI_STYLE_VALUE_LIGHT));
    QCOMPARE(setting.get(GlobalSetting::ThemeColor).toString(), QStringLiteral("themeColor"));
    QCOMPARE(setting.get(GlobalSetting::SystemFontSize).toString(), QStringLiteral("systemFontSize"));
    QCOMPARE(setting.get(GlobalSetting::Transparency).toInt(), 1);
    QCOMPARE(setting.get(GlobalSetting::EffectEnabled).toBool(), false);
    QCOMPARE(setting.get(GlobalSetting::IsLiteMode).toBool(), false);
}

void settings_test::test_globalSetting_readsSchemasAndConfig()
{
    FakeQGSettingsStore::setSchemaInstalled(UKUI_STYLE_SCHEMA, true);
    FakeQGSettingsStore::setSchemaInstalled(CONTROL_CENTER_SETTING, true);
    FakeQGSettingsStore::setSchemaData(UKUI_STYLE_SCHEMA, {
        { QStringLiteral("styleName"), QStringLiteral("ukui-dark") },
        { QStringLiteral("themeColor"), QStringLiteral("#112233") },
        { QStringLiteral("systemFontSize"), 14 },
        { QStringLiteral("iconThemeName"), QStringLiteral("my-icons") }
    });
    FakeQGSettingsStore::setSchemaData(CONTROL_CENTER_SETTING, {
        { QStringLiteral("transparency"), 0.56 },
        { QStringLiteral("effect"), true }
    });

    GlobalSetting setting(nullptr);

    QCOMPARE(setting.get(GlobalSetting::StyleName).toString(), QStringLiteral("ukui-dark"));
    QCOMPARE(setting.get(GlobalSetting::ThemeColor).toString(), QStringLiteral("#112233"));
    QCOMPARE(setting.get(GlobalSetting::SystemFontSize).toInt(), 14);
    QCOMPARE(setting.get(GlobalSetting::Transparency).toReal(), 0.56);
    QCOMPARE(setting.get(GlobalSetting::EffectEnabled).toBool(), true);
    QCOMPARE(setting.defaultFavoriteApps(), expectedDefaultFavoriteAppsFromSystemConfig());
}

void settings_test::test_globalSetting_styleSchemaChangedSignal()
{
    FakeQGSettingsStore::setSchemaInstalled(UKUI_STYLE_SCHEMA, true);
    FakeQGSettingsStore::setSchemaData(UKUI_STYLE_SCHEMA, {
        { QStringLiteral("styleName"), QStringLiteral("ukui-light") },
        { QStringLiteral("themeColor"), QStringLiteral("#abc") },
        { QStringLiteral("systemFontSize"), 12 },
        { QStringLiteral("iconThemeName"), QStringLiteral("icons-a") }
    });

    GlobalSetting setting(nullptr);
    QSignalSpy spy(&setting, &GlobalSetting::styleChanged);

    const auto settingsChildren = setting.findChildren<QGSettings *>();
    QVERIFY(!settingsChildren.isEmpty());
    auto *styleSettings = settingsChildren.first();

    FakeQGSettingsStore::setSchemaData(UKUI_STYLE_SCHEMA, {
        { QStringLiteral("styleName"), QStringLiteral("ukui-dark") },
        { QStringLiteral("themeColor"), QStringLiteral("#def") },
        { QStringLiteral("systemFontSize"), 16 },
        { QStringLiteral("iconThemeName"), QStringLiteral("icons-b") }
    });

    emit styleSettings->changed(QStringLiteral("styleName"));
    emit styleSettings->changed(QStringLiteral("themeColor"));
    emit styleSettings->changed(QStringLiteral("systemFontSize"));
    emit styleSettings->changed(QStringLiteral("iconThemeName"));

    QCOMPARE(setting.get(GlobalSetting::StyleName).toString(), QStringLiteral("ukui-dark"));
    QCOMPARE(setting.get(GlobalSetting::ThemeColor).toString(), QStringLiteral("#def"));
    QCOMPARE(setting.get(GlobalSetting::SystemFontSize).toInt(), 16);
    QCOMPARE(setting.get(GlobalSetting::IconThemeName).toString(), QStringLiteral("icons-b"));
    QCOMPARE(spy.count(), 4);
    QCOMPARE(spy.at(0).at(0).value<GlobalSetting::Key>(), GlobalSetting::StyleName);
    QCOMPARE(spy.at(1).at(0).value<GlobalSetting::Key>(), GlobalSetting::ThemeColor);
    QCOMPARE(spy.at(2).at(0).value<GlobalSetting::Key>(), GlobalSetting::SystemFontSize);
    QCOMPARE(spy.at(3).at(0).value<GlobalSetting::Key>(), GlobalSetting::IconThemeName);
}

void settings_test::test_globalSetting_controlCenterChangedSignal()
{
    FakeQGSettingsStore::setSchemaInstalled(CONTROL_CENTER_SETTING, true);
    FakeQGSettingsStore::setSchemaData(CONTROL_CENTER_SETTING, {
        { QStringLiteral("transparency"), 0.5 },
        { QStringLiteral("effect"), false }
    });

    GlobalSetting setting(nullptr);
    QSignalSpy spy(&setting, &GlobalSetting::styleChanged);

    const auto settingsChildren = setting.findChildren<QGSettings *>();
    QVERIFY(!settingsChildren.isEmpty());
    auto *controlSettings = settingsChildren.first();

    FakeQGSettingsStore::setSchemaData(CONTROL_CENTER_SETTING, {
        { QStringLiteral("transparency"), 0.75 },
        { QStringLiteral("effect"), true }
    });

    emit controlSettings->changed(QStringLiteral("transparency"));
    emit controlSettings->changed(QStringLiteral("effect"));

    QCOMPARE(setting.get(GlobalSetting::Transparency).toReal(), 0.75);
    QCOMPARE(setting.get(GlobalSetting::EffectEnabled).toBool(), true);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).value<GlobalSetting::Key>(), GlobalSetting::Transparency);
    QCOMPARE(spy.at(1).at(0).value<GlobalSetting::Key>(), GlobalSetting::EffectEnabled);
}

void settings_test::test_globalSetting_sysModeChangedSlot()
{
    GlobalSetting setting(nullptr);
    QSignalSpy spy(&setting, &GlobalSetting::styleChanged);

    setting.onSysModeChanged({{QStringLiteral("animation"), QStringLiteral("lite")}});
    QCOMPARE(setting.get(GlobalSetting::IsLiteMode).toBool(), true);

    setting.onSysModeChanged({{QStringLiteral("animation"), QStringLiteral("normal")}});
    QCOMPARE(setting.get(GlobalSetting::IsLiteMode).toBool(), false);

    setting.onSysModeChanged({});
    QCOMPARE(spy.count(), 2);
}

void settings_test::test_globalSetting_systemAndFavoriteQueries()
{
    FakeKdkPackageStore::setRemovable("/tmp/system.desktop", false);
    FakeKdkPackageStore::setRemovable("/tmp/user.desktop", true);

    GlobalSetting setting(nullptr);

    QCOMPARE(setting.isSystemApp(QStringLiteral("/tmp/system.desktop")), true);
    QCOMPARE(setting.isSystemApp(QStringLiteral("/tmp/user.desktop")), false);

    const QStringList expectedFavorites = expectedDefaultFavoriteAppsFromSystemConfig();
    if (!expectedFavorites.isEmpty()) {
        QCOMPARE(setting.defaultFavoriteApps(), expectedFavorites);
        QCOMPARE(setting.isDefaultFavoriteApp(expectedFavorites.first()), true);
    }
    QCOMPARE(setting.isDefaultFavoriteApp(QStringLiteral("missing.desktop")), false);
}

void settings_test::test_menuSetting_defaultsWithoutSchema()
{
    MenuSetting setting(nullptr);

    QCOMPARE(setting.get(QStringLiteral(UKUI_MENU_ANIMATION_DURATION)).toInt(), 300);
    QCOMPARE(setting.get(QStringLiteral(FOLLOW_UKUI_PANEL)).toBool(), true);
    QCOMPARE(setting.get(QStringLiteral(MENU_WIDTH)).toInt(), 766);
    QCOMPARE(setting.get(QStringLiteral(MENU_HEIGHT)).toInt(), 688);
    QCOMPARE(setting.get(QStringLiteral(MENU_MARGIN)).toInt(), 8);
    QCOMPARE(setting.set(QStringLiteral("width"), 900), false);
}

void settings_test::test_menuSetting_repairsMissingVersion()
{
    FakeQGSettingsStore::setSchemaInstalled(UKUI_MENU_SCHEMA, true);
    FakeQGSettingsStore::setSchemaData(UKUI_MENU_SCHEMA, {
        { QStringLiteral("width"), 500 },
        { QStringLiteral("height"), 400 }
    });

    MenuSetting setting(nullptr);

    QCOMPARE(setting.get(QStringLiteral(MENU_WIDTH)).toInt(), 766);
    QCOMPARE(setting.get(QStringLiteral(MENU_HEIGHT)).toInt(), 688);

    const auto calls = FakeQGSettingsStore::setCalls(UKUI_MENU_SCHEMA);
    QVERIFY(calls.contains(qMakePair(QStringLiteral("width"), QVariant(766))));
    QVERIFY(calls.contains(qMakePair(QStringLiteral("height"), QVariant(688))));
}

void settings_test::test_menuSetting_repairsLegacyMajorVersion()
{
    FakeQGSettingsStore::setSchemaInstalled(UKUI_MENU_SCHEMA, true);
    FakeQGSettingsStore::setSchemaData(UKUI_MENU_SCHEMA, {
        { QStringLiteral("width"), 520 },
        { QStringLiteral("height"), 420 },
        { QStringLiteral("margin"), 16 },
        { QStringLiteral("followUkuiPanel"), false },
        { QStringLiteral("animationDuration"), 180 },
        { QStringLiteral("version"), QStringLiteral("0.9") }
    });

    MenuSetting setting(nullptr);

    QCOMPARE(setting.get(QStringLiteral(MENU_WIDTH)).toInt(), 766);
    QCOMPARE(setting.get(QStringLiteral(MENU_HEIGHT)).toInt(), 688);
    QCOMPARE(setting.get(QStringLiteral(MENU_MARGIN)).toInt(), 16);
    QCOMPARE(setting.get(QStringLiteral(FOLLOW_UKUI_PANEL)).toBool(), false);

    const auto calls = FakeQGSettingsStore::setCalls(UKUI_MENU_SCHEMA);
    QVERIFY(calls.contains(qMakePair(QStringLiteral("width"), QVariant(766))));
    QVERIFY(calls.contains(qMakePair(QStringLiteral("height"), QVariant(688))));
    QVERIFY(calls.contains(qMakePair(QStringLiteral("version"), QVariant(QStringLiteral("1.0")))));
}

void settings_test::test_menuSetting_setAndUpdateValue()
{
    FakeQGSettingsStore::setSchemaInstalled(UKUI_MENU_SCHEMA, true);
    FakeQGSettingsStore::setSchemaData(UKUI_MENU_SCHEMA, {
        { QStringLiteral("width"), 600 },
        { QStringLiteral("height"), 500 },
        { QStringLiteral("version"), QStringLiteral("1.0") }
    });

    MenuSetting setting(nullptr);
    QSignalSpy spy(&setting, &MenuSetting::changed);

    QCOMPARE(setting.set(QStringLiteral("width"), 820), true);
    QCOMPARE(FakeQGSettingsStore::schemaData(UKUI_MENU_SCHEMA).value(QStringLiteral("width")).toInt(), 820);

    const auto settingsChildren = setting.findChildren<QGSettings *>();
    QVERIFY(!settingsChildren.isEmpty());
    auto *gsettings = settingsChildren.first();

    emit gsettings->changed(QStringLiteral("width"));

    QCOMPARE(setting.get(QStringLiteral("width")).toInt(), 820);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("width"));
}

QTEST_MAIN(settings_test)

#include "settings-test.moc"
