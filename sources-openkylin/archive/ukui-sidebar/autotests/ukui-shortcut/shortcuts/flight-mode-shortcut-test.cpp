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
#include <memory>

#define private public
#include "../../../ukui-shortcut/shortcuts/flight-mode-shortcut/flight-mode-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

const QByteArray kMediaKeyStateSchema("org.ukui.SettingsDaemon.plugins.media-keys-state");
const QString kRfkillStateKey("rfkillState");

class FlightModeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_initialStateWhenFlightModeIsOff();
    void test_initialStateWhenFlightModeIsOn();
    void test_constructorDisablesWhenSchemaMissing();
    void test_constructorDisablesWhenKeyMissing();
    void test_constructorDisablesWhenUnsupported();
    void test_activeOnlyHandlesClickAndTogglesValue();
    void test_activeGuardsDisabledNullSettingsAndMissingKey();
    void test_gsettingsChangedOnlyReactsToRfkillState();
    void test_stateChangedEmitsExpectedSignals();
    void test_stateChangedIgnoresUnchangedState();

private:
    int removeGeneratedGcovFiles() const;
    void installSettings(int state, const QStringList &keys = QStringList{kRfkillStateKey}) const;
};

} // namespace

void FlightModeShortcutTest::init()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    QGSettings::resetTestData();
}

void FlightModeShortcutTest::cleanup()
{
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

int FlightModeShortcutTest::removeGeneratedGcovFiles() const
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

void FlightModeShortcutTest::installSettings(int state, const QStringList &keys) const
{
    QGSettings::installSchema(kMediaKeyStateSchema, keys);
    QGSettings::setValue(kMediaKeyStateSchema, kRfkillStateKey, state);
}

void FlightModeShortcutTest::test_pluginExposesBasicMetadata()
{
    installSettings(0);
    FlightModeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("FlightModeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("flight-mode-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("FlightModeShortcut"));
}

void FlightModeShortcutTest::test_initialStateWhenFlightModeIsOff()
{
    installSettings(0);

    FlightModeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("FlightModeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Flight Mode"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Flight Mode"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("airplane-mode-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);
    QCOMPARE(shortcut.m_state, 0);

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 2);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 2);
    QVERIFY(meta.value(PluginMetaType::PC).isVisible());
    QVERIFY(meta.value(PluginMetaType::Tablet).isVisible());
}

void FlightModeShortcutTest::test_initialStateWhenFlightModeIsOn()
{
    installSettings(1);

    FlightModeShortcut shortcut;

    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_state, 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
}

void FlightModeShortcutTest::test_constructorDisablesWhenSchemaMissing()
{
    FlightModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings == nullptr);
    QCOMPARE(shortcut.m_state, -1);
}

void FlightModeShortcutTest::test_constructorDisablesWhenKeyMissing()
{
    installSettings(0, QStringList{QStringLiteral("otherKey")});

    FlightModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);
    QCOMPARE(shortcut.m_state, -1);
}

void FlightModeShortcutTest::test_constructorDisablesWhenUnsupported()
{
    installSettings(-1);

    FlightModeShortcut shortcut;

    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);
    QCOMPARE(shortcut.m_state, -1);
}

void FlightModeShortcutTest::test_activeOnlyHandlesClickAndTogglesValue()
{
    installSettings(0);
    FlightModeShortcut shortcut;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(shortcut.m_gsettings->get(kRfkillStateKey).toInt(), 0);

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(shortcut.m_gsettings->get(kRfkillStateKey).toInt(), 1);
    QCOMPARE(shortcut.m_state, 1);

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(shortcut.m_gsettings->get(kRfkillStateKey).toInt(), 0);
    QCOMPARE(shortcut.m_state, 0);
}

void FlightModeShortcutTest::test_activeGuardsDisabledNullSettingsAndMissingKey()
{
    FlightModeShortcut noSettingsShortcut;
    noSettingsShortcut.active(PluginMetaType::Click);
    QVERIFY(noSettingsShortcut.m_gsettings == nullptr);

    installSettings(0);
    FlightModeShortcut disabledShortcut;
    disabledShortcut.m_isEnable = false;
    disabledShortcut.active(PluginMetaType::Click);
    QCOMPARE(disabledShortcut.m_gsettings->get(kRfkillStateKey).toInt(), 0);

    QGSettings::resetTestData();
    installSettings(0, QStringList{QStringLiteral("otherKey")});
    FlightModeShortcut missingKeyShortcut;
    missingKeyShortcut.m_isEnable = true;
    missingKeyShortcut.active(PluginMetaType::Click);
    QCOMPARE(missingKeyShortcut.m_gsettings->get(kRfkillStateKey).toInt(), 0);
}

void FlightModeShortcutTest::test_gsettingsChangedOnlyReactsToRfkillState()
{
    installSettings(0);
    FlightModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &FlightModeShortcut::statusChanged);

    shortcut.m_gsettings->set(QStringLiteral("otherKey"), 1);
    QCOMPARE(shortcut.m_state, 0);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.m_gsettings->set(kRfkillStateKey, 1);
    QCOMPARE(shortcut.m_state, 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(statusSpy.count(), 1);
}

void FlightModeShortcutTest::test_stateChangedEmitsExpectedSignals()
{
    installSettings(-1);
    FlightModeShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &FlightModeShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &FlightModeShortcut::statusChanged);

    shortcut.stateChanged(0);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_state, 0);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.stateChanged(1);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_state, 1);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(enableSpy.count(), 0);
    QCOMPARE(statusSpy.count(), 2);

    shortcut.stateChanged(-1);
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(shortcut.m_state, -1);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(enableSpy.takeFirst().at(0).toBool(), false);
    QCOMPARE(statusSpy.count(), 2);
}

void FlightModeShortcutTest::test_stateChangedIgnoresUnchangedState()
{
    installSettings(1);
    FlightModeShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &FlightModeShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &FlightModeShortcut::statusChanged);

    shortcut.stateChanged(1);

    QCOMPARE(shortcut.m_state, 1);
    QVERIFY(shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 0);
    QCOMPARE(statusSpy.count(), 0);
}

QTEST_MAIN(FlightModeShortcutTest)

#include "flight-mode-shortcut-test.moc"
