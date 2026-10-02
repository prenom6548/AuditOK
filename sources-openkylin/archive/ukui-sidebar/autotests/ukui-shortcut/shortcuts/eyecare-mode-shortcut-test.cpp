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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/eyecare-mode-shortcut/eyecare-mode-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)
Q_DECLARE_METATYPE(QDBusInterface *)

namespace {

const QByteArray kEyeCareSchema("org.ukui.SettingsDaemon.plugins.color");
const QString kEyeCareKey("mainSwitch");
const char kEyeCareService[] = "org.ukui.SettingsDaemon";
const char kEyeCarePath[] = "/GlobalSignal";
const char kEyeCareInterface[] = "org.ukui.SettingsDaemon.GlobalSignal";

class FakeEyeCareService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.SettingsDaemon.GlobalSignal")
public:
    bool supportGamma = true;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QString::fromLatin1(kEyeCarePath));
        bus.unregisterService(QString::fromLatin1(kEyeCareService));
        if (!bus.registerService(QString::fromLatin1(kEyeCareService))) {
            return false;
        }

        registered = bus.registerObject(QString::fromLatin1(kEyeCarePath),
                                        this,
                                        QDBusConnection::ExportAllSlots);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QString::fromLatin1(kEyeCarePath));
        bus.unregisterService(QString::fromLatin1(kEyeCareService));
        registered = false;
    }

    void reset()
    {
        supportGamma = true;
    }

public Q_SLOTS:
    bool isSupportGamma()
    {
        return supportGamma;
    }
};

class EyeCareModeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialStateAndMetadata();
    void test_onDbusConnectorReadyRejectsInvalidInterface();
    void test_onDbusConnectorReadyDisablesWhenGammaUnsupported();
    void test_onDbusConnectorReadyHandlesMissingSchemaOrKey();
    void test_onDbusConnectorReadyInitializesSettingsAndEnableState();
    void test_setAddedStateGuardsNullSettingsAndTogglesSignalConnection();
    void test_onQGSettingsChangedOnlyReactsToMainSwitch();
    void test_triggerGuardsAndTogglesValue();
    void test_activeOnlyHandlesClick();

private:
    int removeGeneratedGcovFiles() const;
    void waitForAsync() const;
    QDBusInterface *newInterface(QObject *parent = nullptr) const;

private:
    FakeEyeCareService m_service;
};

} // namespace

void EyeCareModeShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void EyeCareModeShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void EyeCareModeShortcutTest::init()
{
    QGSettings::resetTestData();
    m_service.reset();
}

void EyeCareModeShortcutTest::cleanup()
{
    QGSettings::resetTestData();
    removeGeneratedGcovFiles();
}

int EyeCareModeShortcutTest::removeGeneratedGcovFiles() const
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

void EyeCareModeShortcutTest::waitForAsync() const
{
    QTest::qWait(80);
}

QDBusInterface *EyeCareModeShortcutTest::newInterface(QObject *parent) const
{
    auto *interface = new QDBusInterface(QString::fromLatin1(kEyeCareService),
                                         QString::fromLatin1(kEyeCarePath),
                                         QString::fromLatin1(kEyeCareInterface),
                                         QDBusConnection::sessionBus(),
                                         parent);
    Q_ASSERT(interface->isValid());
    return interface;
}

void EyeCareModeShortcutTest::test_pluginExposesBasicMetadata()
{
    EyeCareModeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("EyeCareShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("eyecare-mode-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("EyeCareShortcut"));
}

void EyeCareModeShortcutTest::test_shortcutInitialStateAndMetadata()
{
    EyeCareModeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("EyeCareShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Eyecare Mode"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Eyecare Mode"));
    QVERIFY(!shortcut.currentStatus().getIcon().isEmpty());
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::Icon);
    QCOMPARE(meta.value(PluginMetaType::PC).index(), 9);
    QCOMPARE(meta.value(PluginMetaType::Tablet).index(), 9);
}

void EyeCareModeShortcutTest::test_onDbusConnectorReadyRejectsInvalidInterface()
{
    EyeCareModeShortcut shortcut;
    QSignalSpy enableSpy(&shortcut, &EyeCareModeShortcut::enableStatusChanged);

    shortcut.onDbusConnectorReady(nullptr);
    QCOMPARE(enableSpy.count(), 0);
    QVERIFY(!shortcut.isEnable());

    QDBusInterface invalid(QStringLiteral("invalid.service"),
                           QStringLiteral("/invalid"),
                           QStringLiteral("invalid.interface"),
                           QDBusConnection::sessionBus(),
                           &shortcut);
    shortcut.onDbusConnectorReady(&invalid);
    QCOMPARE(enableSpy.count(), 0);
    QVERIFY(!shortcut.isEnable());
}

void EyeCareModeShortcutTest::test_onDbusConnectorReadyDisablesWhenGammaUnsupported()
{
    EyeCareModeShortcut shortcut;
    m_service.supportGamma = false;
    QSignalSpy enableSpy(&shortcut, &EyeCareModeShortcut::enableStatusChanged);

    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();

    QCOMPARE(enableSpy.count(), 0);
    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings == nullptr);
}

void EyeCareModeShortcutTest::test_onDbusConnectorReadyHandlesMissingSchemaOrKey()
{
    EyeCareModeShortcut shortcut;

    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();
    QVERIFY(!shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings == nullptr);

    QGSettings::installSchema(kEyeCareSchema, QStringList{QStringLiteral("otherKey")});
    EyeCareModeShortcut missingKeyShortcut;
    missingKeyShortcut.onDbusConnectorReady(newInterface(&missingKeyShortcut));
    waitForAsync();
    QVERIFY(!missingKeyShortcut.isEnable());
    QVERIFY(missingKeyShortcut.m_gsettings != nullptr);
}

void EyeCareModeShortcutTest::test_onDbusConnectorReadyInitializesSettingsAndEnableState()
{
    QGSettings::installSchema(kEyeCareSchema, QStringList{kEyeCareKey});
    QGSettings::setValue(kEyeCareSchema, kEyeCareKey, true);

    EyeCareModeShortcut shortcut;
    shortcut.m_isAdded = true;
    QSignalSpy enableSpy(&shortcut, &EyeCareModeShortcut::enableStatusChanged);
    QSignalSpy statusSpy(&shortcut, &EyeCareModeShortcut::statusChanged);

    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();

    QVERIFY(shortcut.isEnable());
    QVERIFY(shortcut.m_gsettings != nullptr);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QCOMPARE(enableSpy.count(), 1);
    QVERIFY(statusSpy.count() >= 1);
}

void EyeCareModeShortcutTest::test_setAddedStateGuardsNullSettingsAndTogglesSignalConnection()
{
    EyeCareModeShortcut shortcut;
    QSignalSpy statusSpy(&shortcut, &EyeCareModeShortcut::statusChanged);

    shortcut.setAddedState(true);
    QVERIFY(shortcut.m_isAdded);
    QCOMPARE(statusSpy.count(), 0);

    QGSettings::installSchema(kEyeCareSchema, QStringList{kEyeCareKey});
    QGSettings::setValue(kEyeCareSchema, kEyeCareKey, false);
    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();
    statusSpy.clear();

    shortcut.setAddedState(false);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 1);

    shortcut.setAddedState(true);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
    QCOMPARE(statusSpy.count(), 2);

    shortcut.m_gsettings->set(kEyeCareKey, true);
    QTRY_COMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);

    shortcut.setAddedState(false);
    shortcut.m_gsettings->set(kEyeCareKey, false);
    waitForAsync();
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::BaseColor);
}

void EyeCareModeShortcutTest::test_onQGSettingsChangedOnlyReactsToMainSwitch()
{
    QGSettings::installSchema(kEyeCareSchema, QStringList{kEyeCareKey});
    QGSettings::setValue(kEyeCareSchema, kEyeCareKey, false);

    EyeCareModeShortcut shortcut;
    shortcut.m_isAdded = true;
    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();
    QSignalSpy statusSpy(&shortcut, &EyeCareModeShortcut::statusChanged);

    shortcut.onQGSettingsChanged(QStringLiteral("otherKey"));
    QCOMPARE(statusSpy.count(), 0);

    shortcut.m_gsettings->set(kEyeCareKey, true);
    shortcut.onQGSettingsChanged(kEyeCareKey);
    QCOMPARE(shortcut.currentStatus().getColor(), Color::ColorRole::HighLight);
    QVERIFY(statusSpy.count() >= 1);
}

void EyeCareModeShortcutTest::test_triggerGuardsAndTogglesValue()
{
    QGSettings::installSchema(kEyeCareSchema, QStringList{kEyeCareKey});
    QGSettings::setValue(kEyeCareSchema, kEyeCareKey, false);

    EyeCareModeShortcut shortcut;
    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();

    shortcut.m_isEnable = false;
    shortcut.m_isAdded = true;
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), false);

    shortcut.m_isEnable = true;
    shortcut.m_isAdded = false;
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), false);

    shortcut.m_isAdded = true;
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), true);
    shortcut.trigger();
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), false);

    QGSettings::resetTestData();
    QGSettings::installSchema(kEyeCareSchema, QStringList{QStringLiteral("otherKey")});
    EyeCareModeShortcut missingKeyShortcut;
    missingKeyShortcut.onDbusConnectorReady(newInterface(&missingKeyShortcut));
    waitForAsync();
    missingKeyShortcut.m_isEnable = true;
    missingKeyShortcut.m_isAdded = true;
    missingKeyShortcut.trigger();
    QCOMPARE(missingKeyShortcut.m_gsettings->get(kEyeCareKey).isValid(), false);
}

void EyeCareModeShortcutTest::test_activeOnlyHandlesClick()
{
    QGSettings::installSchema(kEyeCareSchema, QStringList{kEyeCareKey});
    QGSettings::setValue(kEyeCareSchema, kEyeCareKey, false);

    EyeCareModeShortcut shortcut;
    shortcut.onDbusConnectorReady(newInterface(&shortcut));
    waitForAsync();
    shortcut.m_isEnable = true;
    shortcut.m_isAdded = true;

    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), false);

    shortcut.active(PluginMetaType::Click);
    QCOMPARE(shortcut.m_gsettings->get(kEyeCareKey).toBool(), true);
}

QTEST_MAIN(EyeCareModeShortcutTest)

#include "eyecare-mode-shortcut-test.moc"
