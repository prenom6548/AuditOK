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
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#define private public
#include "../../../ukui-shortcut/shortcuts/volume-shortcut/volume-shortcut.cpp"
#undef private

using namespace UkuiShortcut;

Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo)

namespace {

class FakeVolumeService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.volume.control")
public:
    bool validDevice = true;
    qint32 volume = 45;
    bool mute = false;
    bool boostStatus = false;
    qint32 boostValue = 150;
    QString lastMethod;
    int setMuteCount = 0;
    int setVolumeCount = 0;
    bool lastMute = false;
    int lastVolume = 0;
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH));
        bus.unregisterService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME));
        if (!bus.registerService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH));
        bus.unregisterService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME));
        registered = false;
    }

    void reset()
    {
        validDevice = true;
        volume = 45;
        mute = false;
        boostStatus = false;
        boostValue = 150;
        lastMethod.clear();
        setMuteCount = 0;
        setVolumeCount = 0;
        lastMute = false;
        lastVolume = 0;
    }

public Q_SLOTS:
    bool isValidDevice(int type)
    {
        Q_UNUSED(type)
        return validDevice;
    }

    qint32 getVolume(int type, int index)
    {
        Q_UNUSED(type)
        Q_UNUSED(index)
        return volume;
    }

    bool getMute(int type, int index)
    {
        Q_UNUSED(type)
        Q_UNUSED(index)
        return mute;
    }

    bool getVolumeBoostStatus()
    {
        return boostStatus;
    }

    qint32 getVolumeBoostValue()
    {
        return boostValue;
    }

    void setMute(int type, int index, bool value)
    {
        Q_UNUSED(type)
        Q_UNUSED(index)
        lastMethod = QStringLiteral("setMute");
        lastMute = value;
        ++setMuteCount;
    }

    void setVolume(int type, int index, int value)
    {
        Q_UNUSED(type)
        Q_UNUSED(index)
        lastMethod = QStringLiteral("setVolume");
        lastVolume = value;
        ++setVolumeCount;
    }

Q_SIGNALS:
    void volumeChanged(qint32 type, qint32 index, QDBusVariant value);
    void muteChanged(qint32 type, qint32 index, bool mute);
    void deviceAdjust(qint32 type);
    void deviceChanged(qint32 type, QString portName, QString cardName);
    void changed(QString key, QDBusVariant value);
};

class FakeBrokenVolumeService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.volume.control")
public:
    bool registered = false;

    bool registerService()
    {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            return false;
        }

        bus.unregisterObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH));
        bus.unregisterService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME));
        if (!bus.registerService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME))) {
            return false;
        }

        registered = bus.registerObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH),
                                        this,
                                        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        return registered;
    }

    void unregisterService()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.unregisterObject(QStringLiteral(UKUI_VOLUME_CONTROL_PATH));
        bus.unregisterService(QStringLiteral(UKUI_VOLUME_CONTROL_NAME));
        registered = false;
    }
};

class VolumeShortcutTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_pluginExposesBasicMetadata();
    void test_shortcutInitialState();
    void test_changeValueSelectsIconsAndClampsRange();
    void test_muteForcesMutedIcon();
    void test_onVolumeChangedIgnoresNonOutputType();
    void test_onVolumeChangedUpdatesEnabledAvailableStatus();
    void test_onMuteChangedUpdatesEnabledAvailableStatus();
    void test_onChangedUpdatesMaxValueAndGuardsUnavailableState();
    void test_setValueGuardsUnavailableAndSameValue();
    void test_setValueUnmutesBeforeChangingVolume();
    void test_activeClickTogglesMuteWhenAvailable();
    void test_activeIgnoresUnavailableAndUnsupportedActions();
    void test_getDeviceVolumeHandlesAvailableAndUnavailableDevice();
    void test_initVolumeArgsReadsBoostSettings();
    void test_invalidDbusRepliesCoverErrorPaths();
    void test_onDbusConnectorReadyInitializesInterfaces();
    void test_deviceSignalsRefreshVolume();
    void test_initDBusConnectFailureBranches();

private:
    int removeGeneratedGcovFiles() const;
    QDBusInterface *newDeviceInterface(QObject *parent = nullptr) const;
    void waitForAsync();

private:
    FakeVolumeService m_service;
};

} // namespace

void VolumeShortcutTest::initTestCase()
{
    qRegisterMetaType<StatusInfo>("StatusInfo");
    qRegisterMetaType<QDBusVariant>("QDBusVariant");
    qRegisterMetaType<QDBusInterface *>("QDBusInterface*");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available");
    }
    QVERIFY(m_service.registerService());
}

void VolumeShortcutTest::cleanupTestCase()
{
    m_service.unregisterService();
    removeGeneratedGcovFiles();
}

void VolumeShortcutTest::cleanup()
{
    m_service.reset();
    removeGeneratedGcovFiles();
}

int VolumeShortcutTest::removeGeneratedGcovFiles() const
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

QDBusInterface *VolumeShortcutTest::newDeviceInterface(QObject *parent) const
{
    auto *interface = new QDBusInterface(QStringLiteral(UKUI_VOLUME_CONTROL_NAME),
                                         QStringLiteral(UKUI_VOLUME_CONTROL_PATH),
                                         QStringLiteral(UKUI_VOLUME_CONTROL_INTERFACE),
                                         QDBusConnection::sessionBus(),
                                         parent);
    Q_ASSERT(interface->isValid());
    return interface;
}

void VolumeShortcutTest::waitForAsync()
{
    QTest::qWait(50);
}

void VolumeShortcutTest::test_pluginExposesBasicMetadata()
{
    VolumeShortcutPlugin plugin;

    QCOMPARE(plugin.pluginId(), QStringLiteral("VolumeShortcut"));
    QCOMPARE(plugin.translations(), QStringList{QStringLiteral("volume-shortcut")});

    std::unique_ptr<Shortcut> shortcut(plugin.createShortcut());
    QVERIFY(shortcut != nullptr);
    QCOMPARE(shortcut->pluginId(), QStringLiteral("VolumeShortcut"));
}

void VolumeShortcutTest::test_shortcutInitialState()
{
    VolumeShortcut shortcut;

    QCOMPARE(shortcut.pluginId(), QStringLiteral("VolumeShortcut"));
    QCOMPARE(shortcut.currentStatus().getName(), QStringLiteral("Audio"));
    QCOMPARE(shortcut.currentStatus().getToolTip(), QStringLiteral("Volume"));
    QCOMPARE(shortcut.currentStatus().getWidgetId(), QStringLiteral("org.ukui.shortcut.audio"));
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MUTED_SYMBOLIC));
    QCOMPARE(shortcut.currentStatus().getValue(), 0);
    QVERIFY(!shortcut.isEnable());

    const auto meta = shortcut.pluginMetaData();
    QCOMPARE(meta.size(), 2);
    QCOMPARE(meta.value(PluginMetaType::PC).pluginType(), PluginMetaType::ProgressBar);
    QCOMPARE(meta.value(PluginMetaType::Tablet).pluginType(), PluginMetaType::ProgressBar);
}

void VolumeShortcutTest::test_changeValueSelectsIconsAndClampsRange()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeMaxValue = 120;
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.changeValue(-1);
    QCOMPARE(shortcut.currentStatus().getValue(), 0);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MUTED_SYMBOLIC));

    shortcut.changeValue(12);
    QCOMPARE(shortcut.currentStatus().getValue(), 12);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_LOW_SYMBOLIC));

    shortcut.changeValue(50);
    QCOMPARE(shortcut.currentStatus().getValue(), 50);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MEDIUM_SYMBOLIC));

    shortcut.changeValue(100);
    QCOMPARE(shortcut.currentStatus().getValue(), 100);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_HIGH_SYMBOLIC));

    shortcut.changeValue(200);
    QCOMPARE(shortcut.currentStatus().getValue(), 120);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_HIGH_SYMBOLIC));
    QCOMPARE(statusSpy.count(), 5);
}

void VolumeShortcutTest::test_muteForcesMutedIcon()
{
    VolumeShortcut shortcut;
    shortcut.changeValue(80);

    shortcut.mute(true);

    QVERIFY(shortcut.m_mute);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MUTED_SYMBOLIC));
}

void VolumeShortcutTest::test_onVolumeChangedIgnoresNonOutputType()
{
    VolumeShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.onVolumeChanged(0, -1, QDBusVariant(QVariant(80.0)));

    QCOMPARE(statusSpy.count(), 0);
    QCOMPARE(shortcut.m_currentVolume, 0.0);
}

void VolumeShortcutTest::test_onVolumeChangedUpdatesEnabledAvailableStatus()
{
    VolumeShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.onVolumeChanged(1, -1, QDBusVariant(QVariant(66.0)));

    QCOMPARE(shortcut.m_currentVolume, 66.0);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MEDIUM_SYMBOLIC));
}

void VolumeShortcutTest::test_onMuteChangedUpdatesEnabledAvailableStatus()
{
    VolumeShortcut shortcut;
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;
    shortcut.m_currentStatusInfo.setValue(70);
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.onMuteChanged(1, -1, true);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MUTED_SYMBOLIC));

    shortcut.onMuteChanged(0, -1, false);
    QCOMPARE(statusSpy.count(), 1);
    QVERIFY(shortcut.m_mute);
}

void VolumeShortcutTest::test_onChangedUpdatesMaxValueAndGuardsUnavailableState()
{
    VolumeShortcut shortcut;
    shortcut.m_currentVolume = 40;
    shortcut.m_isEnable = false;
    shortcut.m_volumeAvailable = true;
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.onChanged(QStringLiteral("volume-increase"), QDBusVariant(QVariant(true)));
    QCOMPARE(shortcut.m_isIncrease, true);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 100);

    shortcut.onChanged(QStringLiteral("volume-increase-value"), QDBusVariant(QVariant(180)));
    QCOMPARE(shortcut.m_volumeMaxValue, 180);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 180);
    QCOMPARE(statusSpy.count(), 2);

    shortcut.m_isEnable = true;
    shortcut.onChanged(QStringLiteral("unknown"), QDBusVariant(QVariant(0)));
    QCOMPARE(shortcut.currentStatus().getValue(), 40);
    QCOMPARE(statusSpy.count(), 4);
}

void VolumeShortcutTest::test_setValueGuardsUnavailableAndSameValue()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_currentVolume = 20;

    shortcut.setValue(20);
    shortcut.setValue(30);
    waitForAsync();

    QCOMPARE(m_service.setVolumeCount, 0);
    QCOMPARE(m_service.setMuteCount, 0);
}

void VolumeShortcutTest::test_setValueUnmutesBeforeChangingVolume()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_currentVolume = 20;
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;
    shortcut.m_mute = true;

    shortcut.setValue(60);
    QTRY_COMPARE(m_service.setMuteCount, 1);
    QTRY_COMPARE(m_service.setVolumeCount, 1);
    QCOMPARE(m_service.lastMute, false);
    QCOMPARE(m_service.lastVolume, 60);
}

void VolumeShortcutTest::test_activeClickTogglesMuteWhenAvailable()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;
    shortcut.m_mute = false;

    shortcut.active(PluginMetaType::Click);

    QTRY_COMPARE(m_service.setMuteCount, 1);
    QCOMPARE(m_service.lastMute, true);
}

void VolumeShortcutTest::test_activeIgnoresUnavailableAndUnsupportedActions()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_isEnable = false;
    shortcut.m_volumeAvailable = true;

    shortcut.active(PluginMetaType::Click);
    shortcut.active(PluginMetaType::LongClick);
    shortcut.active(PluginMetaType::MenuRequest);
    waitForAsync();

    QCOMPARE(m_service.setMuteCount, 0);
    QCOMPARE(m_service.setVolumeCount, 0);
}

void VolumeShortcutTest::test_getDeviceVolumeHandlesAvailableAndUnavailableDevice()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    m_service.validDevice = false;
    shortcut.getDeviceVolume();
    QTRY_COMPARE(statusSpy.count(), 1);
    QVERIFY(!shortcut.m_volumeAvailable);
    QVERIFY(shortcut.currentStatus().isDisabled());
    QCOMPARE(shortcut.currentStatus().getIcon(), QStringLiteral(AUDIO_VOLUME_MUTED_SYMBOLIC));

    m_service.validDevice = true;
    shortcut.m_volumeBoostInterface = new QDBusInterface(QStringLiteral(UKUI_VOLUME_CONTROL_NAME),
                                                         QStringLiteral(UKUI_VOLUME_CONTROL_PATH),
                                                         QStringLiteral(UKUI_VOLUME_CONTROL_INTERFACE),
                                                         QDBusConnection::sessionBus(),
                                                         &shortcut);
    shortcut.getDeviceVolume();
    QTRY_VERIFY(shortcut.m_volumeAvailable);
}

void VolumeShortcutTest::test_initVolumeArgsReadsBoostSettings()
{
    m_service.boostStatus = true;
    m_service.boostValue = 175;

    VolumeShortcut shortcut;
    shortcut.m_volumeBoostInterface = newDeviceInterface(&shortcut);
    QSignalSpy statusSpy(&shortcut, &VolumeShortcut::statusChanged);

    shortcut.initVolumeArgs<bool>(QStringLiteral("getVolumeBoostStatus"), VolumeShortcut::IsIncrease);
    QTRY_VERIFY(shortcut.m_isIncrease);

    shortcut.initVolumeArgs<qint32>(QStringLiteral("getVolumeBoostValue"), VolumeShortcut::VolumeMaxValue);
    QTRY_COMPARE(shortcut.m_volumeMaxValue, 175);
    QCOMPARE(shortcut.currentStatus().getMaxValue(), 175);
    QVERIFY(statusSpy.count() >= 2);
}

void VolumeShortcutTest::test_invalidDbusRepliesCoverErrorPaths()
{
    m_service.unregisterService();
    FakeBrokenVolumeService brokenService;
    QVERIFY(brokenService.registerService());

    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_volumeBoostInterface = newDeviceInterface(&shortcut);
    shortcut.m_isEnable = true;
    shortcut.m_volumeAvailable = true;

    shortcut.getDeviceVolume();
    shortcut.initVolumeArgs<qint32>(QStringLiteral("getVolume"), VolumeShortcut::CurrentVolume);
    shortcut.initVolumeArgs<bool>(QStringLiteral("getVolumeBoostStatus"), VolumeShortcut::IsIncrease);
    shortcut.active(PluginMetaType::Click);
    shortcut.setValue(25);
    waitForAsync();

    brokenService.unregisterService();
    QVERIFY(m_service.registerService());
}

void VolumeShortcutTest::test_onDbusConnectorReadyInitializesInterfaces()
{
    VolumeShortcut shortcut;
    auto *interface = newDeviceInterface();
    QSignalSpy enableSpy(&shortcut, &VolumeShortcut::enableStatusChanged);

    shortcut.onDbusConnectorReady(nullptr);
    QVERIFY(shortcut.m_volumeDeviceInterface == nullptr);

    shortcut.onDbusConnectorReady(interface);
    QTRY_VERIFY(shortcut.m_volumeDeviceInterface == interface);
    QVERIFY(shortcut.m_volumeBoostInterface != nullptr);
    QTRY_VERIFY(shortcut.m_volumeAvailable);
    QTRY_VERIFY(shortcut.isEnable());
    QVERIFY(enableSpy.count() >= 1);
}

void VolumeShortcutTest::test_deviceSignalsRefreshVolume()
{
    VolumeShortcut shortcut;
    shortcut.m_volumeDeviceInterface = newDeviceInterface(&shortcut);
    shortcut.m_volumeBoostInterface = new QDBusInterface(QStringLiteral(UKUI_VOLUME_CONTROL_NAME),
                                                         QStringLiteral(UKUI_VOLUME_CONTROL_PATH),
                                                         QStringLiteral(UKUI_VOLUME_CONTROL_INTERFACE),
                                                         QDBusConnection::sessionBus(),
                                                         &shortcut);

    shortcut.onDeviceAdjust(1);
    shortcut.onDeviceChanged(1, QStringLiteral("port"), QStringLiteral("card"));
    QTRY_VERIFY(shortcut.m_volumeAvailable);
}

void VolumeShortcutTest::test_initDBusConnectFailureBranches()
{
    m_service.unregisterService();

    VolumeShortcut shortcut;
    shortcut.initDBusConnect();

    QVERIFY(m_service.registerService());
}

QTEST_MAIN(VolumeShortcutTest)

#include "volume-shortcut-test.moc"
