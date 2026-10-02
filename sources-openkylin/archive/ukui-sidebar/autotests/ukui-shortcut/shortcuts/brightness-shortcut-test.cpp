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
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/brightness-shortcut/brightness-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

class FakeBrightnessService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.SettingsDaemon.Brightness")
public:
    bool enabled = true;
    bool enhancement = false;
    quint32 brightness = 42;
    int setBrightnessCount = 0;
    quint32 lastSetBrightness = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral(UKUI_GLOBAL_BRIGHTNESS_PATH));
        bus.unregisterService(QStringLiteral(UKUI_SETTINGS_DAEMON_NAME));
        if (!bus.registerService(QStringLiteral(UKUI_SETTINGS_DAEMON_NAME))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral(UKUI_GLOBAL_BRIGHTNESS_PATH),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral(UKUI_GLOBAL_BRIGHTNESS_PATH));
        bus.unregisterService(QStringLiteral(UKUI_SETTINGS_DAEMON_NAME));
        registered = false;
    }

    void reset()
    {
        enabled = true;
        enhancement = false;
        brightness = 42;
        setBrightnessCount = 0;
        lastSetBrightness = 0;
    }

public Q_SLOTS:
    bool isEnable()
    {
        return enabled;
    }

    bool isEnhancementBrightness()
    {
        return enhancement;
    }

    quint32 getPrimaryBrightness()
    {
        return brightness;
    }

    void setPrimaryBrightness(quint32 value)
    {
        lastSetBrightness = value;
        ++setBrightnessCount;
    }

Q_SIGNALS:
    void enableChanged(bool enabled);
    void primaryBrightnessChangedEnd(quint32 brightness);
    void enhancementBrightnessSwitchChanged(bool enhancement);
};

class BrightnessShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialState();
    void test_updateStatusInfoSelectsIconsAndInvalidState();
    void test_changeValueIgnoresSameBrightness();
    void test_setMaxValueReflectsEnhancementState();
    void test_onDbusConnectorReadyRejectsInvalidInterface();
    void test_onDbusConnectorReadyInitializesStateFromDbus();
    void test_setValueGuardsDisabledInvalidAndSameValue();
    void test_setValueSendsChangedBrightness();
    void test_onEnableChangedNoopAndDisable();
    void test_onEnableChangedEnablesAndRefreshesBrightness();
    void test_onEnhanceChangedUpdatesMaxValueOnlyWhenChanged();
    void test_dbusSignalsDriveSlotsAfterReady();
    void test_activeDoesNothing();

private:
    int removeGeneratedGcovFiles() const;
    QDBusInterface *newInterface(QObject *parent = nullptr) const;
    void waitForAsync();

private:
    FakeBrightnessService m_service;
};

} // namespace

void BrightnessShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void BrightnessShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void BrightnessShortcutTest::cleanup()
{
    m_service.reset();
    removeGeneratedGcovFiles();
}

int BrightnessShortcutTest::removeGeneratedGcovFiles() const
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

QDBusInterface *BrightnessShortcutTest::newInterface(QObject *parent) const
{
    auto *interface = new QDBusInterface(QStringLiteral(UKUI_SETTINGS_DAEMON_NAME),
                                         QStringLiteral(UKUI_GLOBAL_BRIGHTNESS_PATH),
                                         QStringLiteral(UKUI_SETTINGS_DAEMON_BRIGHTNESS_INTERFACE),
                                         QDBusConnection::sessionBus(),
                                         parent);
    Q_ASSERT(interface->isValid());
    return interface;
}

void BrightnessShortcutTest::waitForAsync()
{
    QTest::qWait(50);
}

void BrightnessShortcutTest::test_pluginExposesBasicMetadata()
{
    BrightnessShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("BrightnessShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("brightness-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("BrightnessShortcut"));
}

void BrightnessShortcutTest::test_shortcutInitialState()
{
    BrightnessShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("BrightnessShortcut"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Brightness"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-0-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getValue(), 0);
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::ProgressBar);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::ProgressBar);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 0);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 0);
}

void BrightnessShortcutTest::test_updateStatusInfoSelectsIconsAndInvalidState()
{
    BrightnessShortcut shortcut;
    shortcut.m_enhancedBrightness = 150;
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.updateStatusInfo(0);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-0-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getValue(), 0);

    shortcut.updateStatusInfo(25);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-25-24-symbolic"));

    shortcut.updateStatusInfo(50);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-50-24-symbolic"));

    shortcut.updateStatusInfo(75);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-75-24-symbolic"));

    shortcut.updateStatusInfo(120);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-100-24-symbolic"));
    QCOMPARE(shortcut.currentStatus().getValue(), 120);

    shortcut.updateStatusInfo(151);
    QVERIFY(shortcut.currentStatus().isDisabled());
    QCOMPARE(shortcut.currentStatus().getValue(), 0);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-0-24-symbolic"));
    QCOMPARE(statusSpy.count(), 6);
}

void BrightnessShortcutTest::test_changeValueIgnoresSameBrightness()
{
    BrightnessShortcut shortcut;
    shortcut.m_enhancedBrightness = 100;
    shortcut.m_currentBrightness = 30;
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.changeValue(30);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.changeValue(31);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.m_currentBrightness, 31.0);
}

void BrightnessShortcutTest::test_setMaxValueReflectsEnhancementState()
{
    BrightnessShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.m_enhance = false;
    shortcut.setMaxValue();
    QCOMPARE(shortcut.m_enhancedBrightness, 100);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 100);

    shortcut.m_enhance = true;
    shortcut.setMaxValue();
    QCOMPARE(shortcut.m_enhancedBrightness, 150);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 150);
    QCOMPARE(statusSpy.count(), 2);
}

void BrightnessShortcutTest::test_onDbusConnectorReadyRejectsInvalidInterface()
{
    BrightnessShortcut shortcut;
    QDBusInterface invalid(QStringLiteral("missing.service"),
                           QStringLiteral("/missing"),
                           QStringLiteral("missing.interface"),
                           QDBusConnection::sessionBus());

    shortcut.onDbusConnectorReady(nullptr);
    QVERIFY(shortcut.m_interface == nullptr);

    shortcut.onDbusConnectorReady(&invalid);
    QVERIFY(shortcut.m_interface == nullptr);
}

void BrightnessShortcutTest::test_onDbusConnectorReadyInitializesStateFromDbus()
{
    m_service.enabled = true;
    m_service.enhancement = true;
    m_service.brightness = 88;

    BrightnessShortcut shortcut;
    auto *interface = newInterface(&shortcut);
    QSignalSpy enableSpy(&shortcut, &BrightnessShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.onDbusConnectorReady(interface);

    QCOMPARE(shortcut.m_interface, interface);
    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_enhance);
    QCOMPARE(shortcut.m_enhancedBrightness, 150);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 150);
    QCOMPARE(shortcut.m_currentBrightness, 88.0);
    QCOMPARE(shortcut.currentStatus().getValue(), 88);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral("ukui-light-100-24-symbolic"));
    QCOMPARE(enableSpy.count(), 1);
    QVERIFY(statusSpy.count() >= 2);
}

void BrightnessShortcutTest::test_setValueGuardsDisabledInvalidAndSameValue()
{
    BrightnessShortcut shortcut;
    auto *interface = newInterface(&shortcut);
    shortcut.m_interface = interface;
    shortcut.m_currentBrightness = 60;

    shortcut.m_isEnable = false;
    shortcut.setValue(61);

    shortcut.m_isEnable = true;
    shortcut.setValue(60);
    waitForAsync();

    QCOMPARE(m_service.setBrightnessCount, 0);
}

void BrightnessShortcutTest::test_setValueSendsChangedBrightness()
{
    BrightnessShortcut shortcut;
    shortcut.m_interface = newInterface(&shortcut);
    shortcut.m_isEnable = true;
    shortcut.m_currentBrightness = 40;

    shortcut.setValue(70);

    QTRY_COMPARE(m_service.setBrightnessCount, 1);
    QCOMPARE(m_service.lastSetBrightness, quint32(70));
}

void BrightnessShortcutTest::test_onEnableChangedNoopAndDisable()
{
    BrightnessShortcut shortcut;
    shortcut.m_interface = newInterface(&shortcut);
    shortcut.m_isEnable = true;
    QSignalSpy enableSpy(&shortcut, &BrightnessShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.onEnableChanged(true);
    QCOMPARE(enableSpy.count(), 0);

    shortcut.onEnableChanged(false);
    QVERIFY(!shortcut.isEnable());
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 0);
}

void BrightnessShortcutTest::test_onEnableChangedEnablesAndRefreshesBrightness()
{
    m_service.brightness = 73;

    BrightnessShortcut shortcut;
    shortcut.m_interface = newInterface(&shortcut);
    shortcut.m_isEnable = false;
    shortcut.m_enhancedBrightness = 100;
    QSignalSpy enableSpy(&shortcut, &BrightnessShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.onEnableChanged(true);

    QVERIFY(shortcut.isEnable());
    QCOMPARE(shortcut.m_currentBrightness, 73.0);
    QCOMPARE(shortcut.currentStatus().getValue(), 73);
    QCOMPARE(enableSpy.count(), 1);
    QCOMPARE(statusSpy.count(), 1);
}

void BrightnessShortcutTest::test_onEnhanceChangedUpdatesMaxValueOnlyWhenChanged()
{
    BrightnessShortcut shortcut;
    shortcut.m_enhance = false;
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    shortcut.onEnhanceChanged(false);
    QCOMPARE(statusSpy.count(), 0);

    shortcut.onEnhanceChanged(true);
    QVERIFY(shortcut.m_enhance);
    QCOMPARE(shortcut.m_enhancedBrightness, 150);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 150);
    QCOMPARE(statusSpy.count(), 1);
}

void BrightnessShortcutTest::test_dbusSignalsDriveSlotsAfterReady()
{
    BrightnessShortcut shortcut;
    shortcut.m_enhancedBrightness = 100;
    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    QSignalSpy enableSpy(&shortcut, &BrightnessShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &BrightnessShortcut::statusChanged);

    Q_EMIT m_service.enableChanged(false);
    QTRY_VERIFY(!shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);

    Q_EMIT m_service.enhancementBrightnessSwitchChanged(true);
    QTRY_COMPARE(shortcut.m_enhancedBrightness, 150);

    shortcut.m_isEnable = true;
    Q_EMIT m_service.primaryBrightnessChangedEnd(99);
    QTRY_COMPARE(shortcut.currentStatus().getValue(), 99);
    QVERIFY(statusSpy.count() >= 2);
}

void BrightnessShortcutTest::test_activeDoesNothing()
{
    BrightnessShortcut shortcut;
    shortcut.m_interface = newInterface(&shortcut);
    shortcut.m_isEnable = true;

    shortcut.active(PluginMetaType::Click);
    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.setBrightnessCount, 0);
}

QTEST_MAIN(BrightnessShortcutTest)

#include "brightness-shortcut-test.moc"
