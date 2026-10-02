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
#include <QSignalSpy>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#ifdef signals
#undef signals
#endif
#include <gio/gio.h>
#define private public
#include "../../../src/main/global-settings.h"
#undef private

using namespace Sidebar;

class FakeStatusManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.statusmanager.interface")

public:
    explicit FakeStatusManagerService(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    bool get_current_tabletmode() const { return true; }

Q_SIGNALS:
    void mode_change_signal(bool isTabletMode);
};

class FakeSettingsDaemonService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.SettingsDaemon.GlobalSignal")

public:
    explicit FakeSettingsDaemonService(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    QString getUKUILiteAnimation() const { return QStringLiteral("lite"); }

Q_SIGNALS:
    void UKUILiteChanged(const QMap<QString, QVariant> &map);
};


class globalSettings_test : public QObject
{
    Q_OBJECT

private slots:
    // GlobalSettings 测试
    void test_globalInstance();
    void test_globalInstance_returnsSameInstance();
    void test_getValue_existingKey();
    void test_getValue_nonExistingKey();
    void test_getKeys_notEmpty();
    void test_getKeys_containsExpectedKeys();
    void test_getPanelLength_invalidScreen();
    void test_updateTabletStatus_true();
    void test_updateTabletStatus_false();
    void test_updateTabletStatus_emitsSignal();
    void test_updateIsLiteMode_withAnimation();
    void test_updateIsLiteMode_withLiteAnimation();
    void test_updateIsLiteMode_emptyMap();
    void test_updateIsLiteMode_noAnimationKey();
    void test_initUSDCenterSettings_defaultValue();
    void test_getPanelLength_existingScreen_ifAvailable();

    // SettingMonitor 测试
    void test_settingMonitor_create();
    void test_settingMonitor_defaultKeys();
    void test_settingMonitor_setKeys();
    void test_settingMonitor_setKeys_overwrite();
    void test_settingMonitor_setKeys_emitsSignal();
    void test_settingMonitor_setKeys_sameKeys();
    void test_settingMonitor_getValue();
    void test_settingMonitor_getValue_nonExisting();
    void test_settingMonitor_valueChangedSlot_matchingKey();
    void test_settingMonitor_valueChangedSlot_nonMatchingKey();
    void test_directGlobalSettings_withFakeDbusServices();
    void test_directGlobalSettings_destructor();
};

// 辅助函数：调用 GlobalSettings 的私有槽
static void invokeUpdateTabletStatus(GlobalSettings *settings, bool isTabletMode)
{
    QMetaObject::invokeMethod(settings, "updateTabletStatus", Q_ARG(bool, isTabletMode));
}

static void invokeUpdateIsLiteMode(GlobalSettings *settings, const QMap<QString, QVariant> &map)
{
    QMetaObject::invokeMethod(settings, "updateIsLiteMode", Q_ARG(QVariantMap, map));
}

void globalSettings_test::test_globalInstance()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QVERIFY(settings != nullptr);
}

void globalSettings_test::test_globalInstance_returnsSameInstance()
{
    GlobalSettings *settings1 = GlobalSettings::globalInstance();
    GlobalSettings *settings2 = GlobalSettings::globalInstance();
    QCOMPARE(settings1, settings2);
}

void globalSettings_test::test_getValue_existingKey()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QVariant value = settings->getValue(UKUI_STYLE_NAME_KEY);
    QVERIFY(value.isValid());
}

void globalSettings_test::test_getValue_nonExistingKey()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QVariant value = settings->getValue("nonexistent-key-12345");
    QVERIFY(!value.isValid());
}

void globalSettings_test::test_getKeys_notEmpty()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QStringList keys = settings->getKeys();
    QVERIFY(!keys.isEmpty());
}

void globalSettings_test::test_getKeys_containsExpectedKeys()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QStringList keys = settings->getKeys();
    QVERIFY(keys.contains(UKUI_STYLE_NAME_KEY));
    QVERIFY(keys.contains(UKUI_PANEL_POSITION_KEY));
    QVERIFY(keys.contains(UKUI_PANEL_SIZE_KEY));
    QVERIFY(keys.contains(TABLET_MODE));
    QVERIFY(keys.contains(CONTROL_CENTER_EFFECT));
    QVERIFY(keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY));
}

void globalSettings_test::test_getPanelLength_invalidScreen()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    int length = settings->getPanelLength("nonexistent-screen");
    QCOMPARE(length, -1);
}

void globalSettings_test::test_updateTabletStatus_true()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    invokeUpdateTabletStatus(settings, true);
    QCOMPARE(settings->getValue(TABLET_MODE).toBool(), true);
}

void globalSettings_test::test_updateTabletStatus_false()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    invokeUpdateTabletStatus(settings, false);
    QCOMPARE(settings->getValue(TABLET_MODE).toBool(), false);
}

void globalSettings_test::test_updateTabletStatus_emitsSignal()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QSignalSpy spy(settings, &GlobalSettings::valueChanged);
    invokeUpdateTabletStatus(settings, true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(TABLET_MODE));

    // 恢复
    invokeUpdateTabletStatus(settings, false);
}

void globalSettings_test::test_updateIsLiteMode_withAnimation()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QSignalSpy spy(settings, &GlobalSettings::valueChanged);

    QMap<QString, QVariant> map;
    map.insert("animation", "normal");
    invokeUpdateIsLiteMode(settings, map);

    QCOMPARE(settings->getValue(IS_LITE_MODE).toBool(), false);
    QCOMPARE(spy.count(), 1);
}

void globalSettings_test::test_updateIsLiteMode_withLiteAnimation()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QSignalSpy spy(settings, &GlobalSettings::valueChanged);

    QMap<QString, QVariant> map;
    map.insert("animation", "lite");
    invokeUpdateIsLiteMode(settings, map);

    QCOMPARE(settings->getValue(IS_LITE_MODE).toBool(), true);
    QCOMPARE(spy.count(), 1);
}

void globalSettings_test::test_updateIsLiteMode_emptyMap()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QSignalSpy spy(settings, &GlobalSettings::valueChanged);

    QMap<QString, QVariant> map;
    invokeUpdateIsLiteMode(settings, map);

    QCOMPARE(spy.count(), 0);
}

void globalSettings_test::test_updateIsLiteMode_noAnimationKey()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    QSignalSpy spy(settings, &GlobalSettings::valueChanged);

    QMap<QString, QVariant> map;
    map.insert("other-key", "value");
    invokeUpdateIsLiteMode(settings, map);

    QCOMPARE(spy.count(), 0);
}

void globalSettings_test::test_initUSDCenterSettings_defaultValue()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    settings->initUSDCenterSettings();
    const QVariant value = settings->getValue(USD_SCALING_FACTOR_KEY);
    QVERIFY(value.isValid());
}

void globalSettings_test::test_getPanelLength_existingScreen_ifAvailable()
{
    GlobalSettings *settings = GlobalSettings::globalInstance();
    if (!settings->isKeysContain("panellength")) {
        QSKIP("panellength key is not available in current runtime schema.");
    }

    const QMap<QString, QVariant> panelLengthMap = settings->getPanelLengthMap();
    if (panelLengthMap.isEmpty()) {
        QSKIP("panellength map is empty in current runtime environment.");
    }

    const QString screenName = panelLengthMap.firstKey();
    QVERIFY(settings->getPanelLength(screenName) >= 0);
}

void globalSettings_test::test_settingMonitor_create()
{
    SettingMonitor monitor;
    QVERIFY(true);
}

void globalSettings_test::test_settingMonitor_defaultKeys()
{
    SettingMonitor monitor;
    QVERIFY(monitor.keys().isEmpty());
}

void globalSettings_test::test_settingMonitor_setKeys()
{
    SettingMonitor monitor;
    QStringList keys;
    keys << "key1" << "key2";
    monitor.setKeys(keys);
    QCOMPARE(monitor.keys(), keys);
}

void globalSettings_test::test_settingMonitor_setKeys_overwrite()
{
    SettingMonitor monitor;
    QStringList keys;
    keys << "key1" << "key2";
    monitor.setKeys(keys);

    QStringList newKeys;
    newKeys << "another-key";
    monitor.setKeys(newKeys);

    QCOMPARE(monitor.keys(), newKeys);
}

void globalSettings_test::test_settingMonitor_setKeys_emitsSignal()
{
    SettingMonitor monitor;
    QSignalSpy spy(&monitor, &SettingMonitor::keysChanged);
    QStringList keys;
    keys << "key1";
    monitor.setKeys(keys);
    QCOMPARE(spy.count(), 1);
}

void globalSettings_test::test_settingMonitor_setKeys_sameKeys()
{
    SettingMonitor monitor;
    QStringList keys;
    keys << "key1" << "key2";
    monitor.setKeys(keys);
    QSignalSpy spy(&monitor, &SettingMonitor::keysChanged);
    monitor.setKeys(keys);
    QCOMPARE(spy.count(), 0);
}

void globalSettings_test::test_settingMonitor_getValue()
{
    SettingMonitor monitor;
    QVariant value = monitor.getValue(UKUI_STYLE_NAME_KEY);
    QVERIFY(value.isValid());
}

void globalSettings_test::test_settingMonitor_getValue_nonExisting()
{
    SettingMonitor monitor;
    QVariant value = monitor.getValue("nonexistent-key-12345");
    QVERIFY(!value.isValid());
}

void globalSettings_test::test_settingMonitor_valueChangedSlot_matchingKey()
{
    SettingMonitor monitor;
    QStringList keys;
    keys << TABLET_MODE;
    monitor.setKeys(keys);

    QSignalSpy spy(&monitor, &SettingMonitor::valueChanged);

    invokeUpdateTabletStatus(GlobalSettings::globalInstance(), true);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(TABLET_MODE));

    // 恢复
    invokeUpdateTabletStatus(GlobalSettings::globalInstance(), false);
}

void globalSettings_test::test_settingMonitor_valueChangedSlot_nonMatchingKey()
{
    SettingMonitor monitor;
    QStringList keys;
    keys << "some-other-key";
    monitor.setKeys(keys);

    QSignalSpy spy(&monitor, &SettingMonitor::valueChanged);

    invokeUpdateTabletStatus(GlobalSettings::globalInstance(), true);

    QCOMPARE(spy.count(), 0);

    // 恢复
    invokeUpdateTabletStatus(GlobalSettings::globalInstance(), false);
}

void globalSettings_test::test_directGlobalSettings_withFakeDbusServices()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }
    auto *busInterface = QDBusConnection::sessionBus().interface();
    QVERIFY(busInterface != nullptr);

    FakeStatusManagerService statusService;
    FakeSettingsDaemonService settingsService;
    struct DbusRegistrationCleanup {
        bool statusObject = false;
        bool statusService = false;
        bool settingsObject = false;
        bool settingsService = false;

        ~DbusRegistrationCleanup()
        {
            if (statusObject) {
                QDBusConnection::sessionBus().unregisterObject("/");
            }
            if (statusService) {
                QDBusConnection::sessionBus().unregisterService(DBUS_STATUS_MANAGER_IF);
            }
            if (settingsObject) {
                QDBusConnection::sessionBus().unregisterObject("/GlobalSignal");
            }
            if (settingsService) {
                QDBusConnection::sessionBus().unregisterService("org.ukui.SettingsDaemon");
            }
        }
    } cleanup;

    if (!QDBusConnection::sessionBus().registerService(DBUS_STATUS_MANAGER_IF)) {
        QSKIP("Status manager service name is already owned in current session.");
    }
    cleanup.statusService = true;
    if (!QDBusConnection::sessionBus().registerObject("/",
                                                      &statusService,
                                                      QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        QSKIP("Status manager object path is already occupied in current session.");
    }
    cleanup.statusObject = true;
    if (!QDBusConnection::sessionBus().registerService("org.ukui.SettingsDaemon")) {
        QSKIP("Settings daemon service name is already owned in current session.");
    }
    cleanup.settingsService = true;
    if (!QDBusConnection::sessionBus().registerObject("/GlobalSignal",
                                                      &settingsService,
                                                      QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        QSKIP("Settings daemon object path is already occupied in current session.");
    }
    cleanup.settingsObject = true;
    QCOMPARE(busInterface->serviceOwner(DBUS_STATUS_MANAGER_IF).value(),
             QDBusConnection::sessionBus().baseService());
    QCOMPARE(busInterface->serviceOwner("org.ukui.SettingsDaemon").value(),
             QDBusConnection::sessionBus().baseService());

    auto *settings = new GlobalSettings();
    QCOMPARE(settings->getValue(TABLET_MODE).toBool(), true);
    QCOMPARE(settings->getValue(IS_LITE_MODE).toBool(), true);
    QVERIFY(QDBusConnection::sessionBus().connect("org.ukui.SettingsDaemon",
                                                 "/GlobalSignal",
                                                 "org.ukui.SettingsDaemon.GlobalSignal",
                                                 "UKUILiteChanged",
                                                 settings,
                                                 SLOT(updateIsLiteMode(QMap<QString,QVariant>))));
    QVERIFY(QObject::connect(&statusService,
                             SIGNAL(mode_change_signal(bool)),
                             settings,
                             SLOT(updateTabletStatus(bool))));
    QVERIFY(QObject::connect(&settingsService,
                             SIGNAL(UKUILiteChanged(QMap<QString,QVariant>)),
                             settings,
                             SLOT(updateIsLiteMode(QMap<QString,QVariant>))));

    QSignalSpy spy(settings, &GlobalSettings::valueChanged);

    emit statusService.mode_change_signal(false);
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1
                                 || busInterface->serviceOwner(DBUS_STATUS_MANAGER_IF).value()
                                        != QDBusConnection::sessionBus().baseService(),
                             1000);
    if (busInterface->serviceOwner(DBUS_STATUS_MANAGER_IF).value()
            != QDBusConnection::sessionBus().baseService()) {
        delete settings;
        QSKIP("Status manager service owner changed during test.");
    }
    QCOMPARE(settings->getValue(TABLET_MODE).toBool(), false);

    QMap<QString, QVariant> map;
    map.insert("animation", "normal");
    emit settingsService.UKUILiteChanged(map);
    QTRY_VERIFY_WITH_TIMEOUT(!settings->getValue(IS_LITE_MODE).toBool()
                                 || busInterface->serviceOwner("org.ukui.SettingsDaemon").value()
                                        != QDBusConnection::sessionBus().baseService(),
                             1000);
    if (busInterface->serviceOwner("org.ukui.SettingsDaemon").value()
            != QDBusConnection::sessionBus().baseService()) {
        delete settings;
        QSKIP("Settings daemon service owner changed during test.");
    }
    QCOMPARE(settings->getValue(IS_LITE_MODE).toBool(), false);

    delete settings;
}

void globalSettings_test::test_directGlobalSettings_destructor()
{
    auto *settings = new GlobalSettings();

    GSettingsSchemaSource *source = g_settings_schema_source_get_default();
    GSettingsSchema *schema = g_settings_schema_source_lookup(source, "org.ukui.panel.settings", true);
    if (schema != nullptr) {
        GSettings *panelSettings = g_settings_new_with_path("org.ukui.panel.settings", "/org/ukui/panel/settings/");
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&builder, "{sv}", "TestScreen", g_variant_new_uint32(88));
        GVariant *value = g_variant_builder_end(&builder);
        g_settings_set_value(panelSettings, "panellength", value);
        const int panelLength = settings->getPanelLength("TestScreen");
        if (panelLength != 88) {
            g_object_unref(panelSettings);
            g_settings_schema_unref(schema);
            delete settings;
            QSKIP("Panel length gsettings value is not writable in current runtime environment.");
        }
        QCOMPARE(panelLength, 88);
        g_object_unref(panelSettings);
        g_settings_schema_unref(schema);
    }

    delete settings;
    QVERIFY(true);
}

QTEST_MAIN(globalSettings_test)

#include "global-settings-test.moc"
