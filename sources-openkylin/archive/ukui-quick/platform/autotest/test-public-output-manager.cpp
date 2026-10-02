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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTest>

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
#endif
#define private public
#include "../wayland/kywc-output-manager.cpp"
#include "../output-manager.cpp"
#undef private
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

Q_DECLARE_METATYPE(UkuiQuick::Output *)
Q_DECLARE_METATYPE(UkuiQuick::OutputMode *)
Q_DECLARE_METATYPE(UkuiQuick::Output::Transform)

static_assert(std::is_same_v<decltype(std::declval<const UkuiQuick::OutputManager &>().outputs()),
                             QList<UkuiQuick::Output *>>);
static_assert(std::is_same_v<decltype(&UkuiQuick::OutputManager::outputAdded),
                             void (UkuiQuick::OutputManager::*)(UkuiQuick::Output *)>);
static_assert(std::is_same_v<decltype(&UkuiQuick::OutputManager::primaryOutputChanged),
                             void (UkuiQuick::OutputManager::*)(UkuiQuick::Output *)>);
static_assert(std::is_same_v<decltype(std::declval<const UkuiQuick::Output &>().currentMode()),
                             UkuiQuick::OutputMode *>);
static_assert(std::is_same_v<decltype(std::declval<const UkuiQuick::Output &>().modes()),
                             QList<UkuiQuick::OutputMode *>>);
static_assert(std::is_same_v<decltype(std::declval<const UkuiQuick::Output &>().transform()),
                             UkuiQuick::Output::Transform>);
static_assert(std::is_same_v<decltype(&UkuiQuick::Output::transformChanged),
                             void (UkuiQuick::Output::*)(UkuiQuick::Output::Transform)>);

namespace UkuiQuick::Test
{

namespace
{

void flushDeferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

OutputMode *findPublicMode(const Output &output, int id)
{
    const auto modes = output.modes();
    const auto it = std::find_if(modes.cbegin(), modes.cend(), [id](const OutputMode *mode) {
        return mode->id() == id;
    });
    return it == modes.cend() ? nullptr : *it;
}

struct BackendOutputFixture
{
    WaylandClient::KywcOutput backend;
    WaylandClient::KywcOutputMode *initialMode {nullptr};

    BackendOutputFixture()
    {
        backend.d->m_uuid = QStringLiteral("output-uuid");
        backend.d->m_name = QStringLiteral("Display-1");
        backend.d->m_make = QStringLiteral("Kylin");
        backend.d->m_model = QStringLiteral("KM-27");
        backend.d->m_serialNumber = QStringLiteral("serial-1");
        backend.d->m_description = QStringLiteral("Primary display");
        backend.d->m_physicalSize = QSize(600, 340);
        backend.d->m_capabilities = static_cast<uint32_t>(Output::Capability::Power)
            | static_cast<uint32_t>(Output::Capability::Brightness)
            | static_cast<uint32_t>(Output::Capability::ColorTemperature)
            | (1U << 12);
        backend.d->m_enabled = true;
        backend.d->m_position = QPoint(10, 20);
        backend.d->m_transform = 3;
        backend.d->m_scale = 1.25;
        backend.d->m_power = 2;
        backend.d->m_brightness = 80;
        backend.d->m_colorTemp = 6500;

        initialMode = addMode(42, QSize(1920, 1080), 60000, true);
        backend.d->m_currentMode = initialMode;
    }

    WaylandClient::KywcOutputMode *addMode(int id, const QSize &size, int refreshRate, bool preferred)
    {
        auto *mode = new WaylandClient::KywcOutputMode(&backend);
        mode->d->m_id = id;
        mode->d->m_size = size;
        mode->d->m_refreshRate = refreshRate;
        mode->d->m_preferred = preferred;
        backend.d->m_modes.append(mode);
        return mode;
    }
};

}

class OutputManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void initialSnapshotCopiesBackendStateAndFiltersCapabilities();
    void stateUpdatesForwardOncePerChange();
    void modeLifecycleUsesStablePublicModes();
    void managerHelpersDeduplicateOutputsAndOrderLifecycleSignals();
    void managerInitializationEmitsOnlyOnFalseToTrueTransitions();
    void transformUsesWaylandTransformEnum();
    void outputsRetainAdditionOrder();
    void invalidatingTheBackendClearsTheSessionBeforeNotifyingClients();
    void invalidatingAnUnavailableManagerDoesNotRepeatLifecycleSignals();
};

void OutputManagerTest::initTestCase()
{
    qRegisterMetaType<Output *>("Output*");
    qRegisterMetaType<OutputMode *>("OutputMode*");
    qRegisterMetaType<Output::Transform>("Transform");
    qRegisterMetaType<uint32_t>("uint32_t");
}

void OutputManagerTest::initialSnapshotCopiesBackendStateAndFiltersCapabilities()
{
    BackendOutputFixture fixture;
    Output output;
    output.d->setup(&fixture.backend);

    QCOMPARE(output.uuid(), QStringLiteral("output-uuid"));
    QCOMPARE(output.name(), QStringLiteral("Display-1"));
    QCOMPARE(output.make(), QStringLiteral("Kylin"));
    QCOMPARE(output.model(), QStringLiteral("KM-27"));
    QCOMPARE(output.serialNumber(), QStringLiteral("serial-1"));
    QCOMPARE(output.description(), QStringLiteral("Primary display"));
    QCOMPARE(output.physicalSize(), QSize(600, 340));
    QCOMPARE(output.position(), QPoint(10, 20));
    QVERIFY(output.capabilities().testFlag(Output::Capability::Power));
    QVERIFY(output.capabilities().testFlag(Output::Capability::Brightness));
    QVERIFY(output.capabilities().testFlag(Output::Capability::ColorTemperature));
    QVERIFY(!output.capabilities().testFlag(static_cast<Output::Capability>(1U << 12)));
    QVERIFY(output.isEnabled());
    QCOMPARE(output.transform(), Output::Transform::Rotate270);
    QCOMPARE(output.scale(), 1.25);
    QCOMPARE(output.power(), 2U);
    QCOMPARE(output.brightness(), 80U);
    QCOMPARE(output.colorTemp(), 6500U);

    QCOMPARE(output.modes().size(), 1);
    auto *mode = output.currentMode();
    QVERIFY(mode != nullptr);
    QCOMPARE(mode, findPublicMode(output, 42));
    QCOMPARE(mode->id(), 42);
    QCOMPARE(mode->size(), QSize(1920, 1080));
    QCOMPARE(mode->refreshRate(), 60000);
    QVERIFY(mode->preferred());
}

void OutputManagerTest::stateUpdatesForwardOncePerChange()
{
    BackendOutputFixture fixture;
    Output output;
    output.d->setup(&fixture.backend);

    QSignalSpy enabledSpy(&output, &Output::enabledChanged);
    QSignalSpy currentModeSpy(&output, &Output::currentModeChanged);
    QSignalSpy positionSpy(&output, &Output::positionChanged);
    QSignalSpy transformSpy(&output, &Output::transformChanged);
    QSignalSpy scaleSpy(&output, &Output::scaleChanged);
    QSignalSpy powerSpy(&output, &Output::powerChanged);
    QSignalSpy brightnessSpy(&output, &Output::brightnessChanged);
    QSignalSpy colorTempSpy(&output, &Output::colorTempChanged);

    fixture.backend.d->m_enabled = false;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled);
    QVERIFY(!output.isEnabled());
    QCOMPARE(enabledSpy.count(), 1);
    QCOMPARE(enabledSpy.first().first().toBool(), false);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled);
    QCOMPARE(enabledSpy.count(), 1);

    fixture.backend.d->m_enabled = true;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled);
    QVERIFY(output.isEnabled());
    QCOMPARE(enabledSpy.count(), 2);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled);
    QCOMPARE(enabledSpy.count(), 2);

    auto *secondBackendMode = fixture.addMode(77, QSize(2560, 1440), 144000, false);
    Q_EMIT fixture.backend.modeAdded(secondBackendMode);
    auto *secondMode = findPublicMode(output, secondBackendMode->id());
    QVERIFY(secondMode != nullptr);
    fixture.backend.d->m_currentMode = secondBackendMode;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::CurrentMode);
    QCOMPARE(output.currentMode(), secondMode);
    QCOMPARE(currentModeSpy.count(), 1);
    QCOMPARE(currentModeSpy.first().first().value<OutputMode *>(), secondMode);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::CurrentMode);
    QCOMPARE(currentModeSpy.count(), 1);

    fixture.backend.d->m_position = QPoint(-40, 35);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Position);
    QCOMPARE(output.position(), QPoint(-40, 35));
    QCOMPARE(positionSpy.count(), 1);
    QCOMPARE(positionSpy.first().first().toPoint(), QPoint(-40, 35));
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Position);
    QCOMPARE(positionSpy.count(), 1);

    fixture.backend.d->m_transform = static_cast<int>(Output::Transform::Flipped180);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Transform);
    QCOMPARE(output.transform(), Output::Transform::Flipped180);
    QCOMPARE(transformSpy.count(), 1);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Transform);
    QCOMPARE(transformSpy.count(), 1);

    fixture.backend.d->m_scale = 2.0;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Scale);
    QCOMPARE(output.scale(), 2.0);
    QCOMPARE(scaleSpy.count(), 1);
    QCOMPARE(scaleSpy.first().first().toReal(), 2.0);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Scale);
    QCOMPARE(scaleSpy.count(), 1);

    fixture.backend.d->m_power = 1;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Power);
    QCOMPARE(output.power(), 1U);
    QCOMPARE(powerSpy.count(), 1);
    QCOMPARE(powerSpy.first().first().toUInt(), 1U);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Power);
    QCOMPARE(powerSpy.count(), 1);

    fixture.backend.d->m_brightness = 90;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Brightness);
    QCOMPARE(output.brightness(), 90U);
    QCOMPARE(brightnessSpy.count(), 1);
    QCOMPARE(brightnessSpy.first().first().toUInt(), 90U);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Brightness);
    QCOMPARE(brightnessSpy.count(), 1);

    fixture.backend.d->m_colorTemp = 5000;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::ColorTemp);
    QCOMPARE(output.colorTemp(), 5000U);
    QCOMPARE(colorTempSpy.count(), 1);
    QCOMPARE(colorTempSpy.first().first().toUInt(), 5000U);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::ColorTemp);
    QCOMPARE(colorTempSpy.count(), 1);

    fixture.backend.d->m_enabled = false;
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled
                                        | WaylandClient::KywcOutput::StateMask::CurrentMode);
    QVERIFY(!output.isEnabled());
    QVERIFY(output.currentMode() == nullptr);
    QCOMPARE(enabledSpy.count(), 3);
    QCOMPARE(currentModeSpy.count(), 2);
    QCOMPARE(currentModeSpy.last().first().value<OutputMode *>(), nullptr);
    Q_EMIT fixture.backend.stateUpdated(WaylandClient::KywcOutput::StateMask::Enabled
                                        | WaylandClient::KywcOutput::StateMask::CurrentMode);
    QCOMPARE(enabledSpy.count(), 3);
    QCOMPARE(currentModeSpy.count(), 2);
}

void OutputManagerTest::modeLifecycleUsesStablePublicModes()
{
    BackendOutputFixture fixture;
    Output output;
    output.d->setup(&fixture.backend);
    auto *firstMode = output.currentMode();
    QVERIFY(firstMode != nullptr);

    QSignalSpy modesChangedSpy(&output, &Output::modesChanged);
    auto *backendMode = fixture.addMode(88, QSize(1280, 720), 60000, false);
    Q_EMIT fixture.backend.modeAdded(backendMode);
    QCOMPARE(modesChangedSpy.count(), 1);

    auto *publicMode = findPublicMode(output, backendMode->id());
    QVERIFY(publicMode != nullptr);
    QCOMPARE(publicMode->size(), QSize(1280, 720));
    QCOMPARE(publicMode->refreshRate(), 60000);
    QVERIFY(!publicMode->preferred());

    backendMode->d->m_size = QSize(1920, 1200);
    backendMode->d->m_refreshRate = 75000;
    backendMode->d->m_preferred = true;
    Q_EMIT fixture.backend.modeChanged(backendMode);
    QCOMPARE(modesChangedSpy.count(), 2);
    QCOMPARE(findPublicMode(output, backendMode->id()), publicMode);
    QCOMPARE(publicMode->size(), QSize(1920, 1200));
    QCOMPARE(publicMode->refreshRate(), 75000);
    QVERIFY(publicMode->preferred());

    QPointer<OutputMode> publicModeGuard(publicMode);
    QVERIFY(fixture.backend.d->m_modes.removeOne(backendMode));
    Q_EMIT fixture.backend.modeRemoved(backendMode->id());
    QCOMPARE(modesChangedSpy.count(), 3);
    QVERIFY(findPublicMode(output, backendMode->id()) == nullptr);
    QCOMPARE(output.currentMode(), firstMode);

    flushDeferredDeletes();
    QVERIFY(publicModeGuard.isNull());
    QCOMPARE(output.currentMode(), firstMode);
}

void OutputManagerTest::managerHelpersDeduplicateOutputsAndOrderLifecycleSignals()
{
    BackendOutputFixture fixture;
    OutputManager manager;
    QSignalSpy outputAddedSpy(&manager, &OutputManager::outputAdded);
    QSignalSpy outputRemovedSpy(&manager, &OutputManager::outputRemoved);
    QSignalSpy primaryChangedSpy(&manager, &OutputManager::primaryOutputChanged);

    manager.d->addOutput(&fixture.backend);
    manager.d->addOutput(&fixture.backend);
    QCOMPARE(outputAddedSpy.count(), 1);
    QCOMPARE(manager.outputs().size(), 1);
    auto *publicOutput = manager.outputs().first();
    QCOMPARE(publicOutput->uuid(), fixture.backend.uuid());

    QStringList lifecycleOrder;
    connect(&manager, &OutputManager::primaryOutputChanged, &manager,
            [&lifecycleOrder](Output *output) {
                lifecycleOrder.append(output ? QStringLiteral("primary") : QStringLiteral("primary-null"));
            });
    connect(&manager, &OutputManager::outputRemoved, &manager,
            [&lifecycleOrder](const QString &uuid) {
                lifecycleOrder.append(QStringLiteral("removed:") + uuid);
            });

    manager.d->setPrimaryOutput(&fixture.backend);
    QCOMPARE(manager.primaryOutput(), publicOutput);
    QCOMPARE(primaryChangedSpy.count(), 1);
    QCOMPARE(primaryChangedSpy.first().first().value<Output *>(), publicOutput);

    lifecycleOrder.clear();
    QPointer<Output> publicOutputGuard(publicOutput);
    manager.d->removeOutput(fixture.backend.uuid());
    QCOMPARE(manager.outputs().size(), 0);
    QCOMPARE(manager.primaryOutput(), nullptr);
    QCOMPARE(outputRemovedSpy.count(), 1);
    QCOMPARE(outputRemovedSpy.first().first().toString(), fixture.backend.uuid());
    QCOMPARE(primaryChangedSpy.count(), 2);
    QCOMPARE(primaryChangedSpy.last().first().value<Output *>(), nullptr);
    const QStringList expectedLifecycleOrder {
        QStringLiteral("primary-null"),
        QStringLiteral("removed:output-uuid"),
    };
    QCOMPARE(lifecycleOrder, expectedLifecycleOrder);

    flushDeferredDeletes();
    QVERIFY(publicOutputGuard.isNull());
}

void OutputManagerTest::managerInitializationEmitsOnlyOnFalseToTrueTransitions()
{
    OutputManager manager;
    QSignalSpy initializedSpy(&manager, &OutputManager::initialized);

    manager.d->setInitialized(true);
    manager.d->setInitialized(true);
    manager.d->setInitialized(false);
    manager.d->setInitialized(true);

    QVERIFY(manager.isInitialized());
    QCOMPARE(initializedSpy.count(), 2);
}

void OutputManagerTest::transformUsesWaylandTransformEnum()
{
    QCOMPARE(static_cast<int>(Output::Transform::Normal), 0);
    QCOMPARE(static_cast<int>(Output::Transform::Rotate90), 1);
    QCOMPARE(static_cast<int>(Output::Transform::Rotate180), 2);
    QCOMPARE(static_cast<int>(Output::Transform::Rotate270), 3);
    QCOMPARE(static_cast<int>(Output::Transform::Flipped), 4);
    QCOMPARE(static_cast<int>(Output::Transform::Flipped90), 5);
    QCOMPARE(static_cast<int>(Output::Transform::Flipped180), 6);
    QCOMPARE(static_cast<int>(Output::Transform::Flipped270), 7);

    Output output;
    output.d->m_transform = Output::Transform::Rotate270;
    QCOMPARE(output.transform(), Output::Transform::Rotate270);
}

void OutputManagerTest::outputsRetainAdditionOrder()
{
    OutputManager manager;
    auto *firstOutput = new Output(&manager);
    auto *secondOutput = new Output(&manager);
    firstOutput->d->m_uuid = QStringLiteral("first");
    secondOutput->d->m_uuid = QStringLiteral("second");
    manager.d->m_outputs.append(firstOutput);
    manager.d->m_outputs.append(secondOutput);

    const QList<Output *> expectedOutputs {firstOutput, secondOutput};
    QCOMPARE(manager.outputs(), expectedOutputs);
}

void OutputManagerTest::invalidatingTheBackendClearsTheSessionBeforeNotifyingClients()
{
    qRegisterMetaType<Output *>("Output*");

    OutputManager manager;
    auto *primaryOutput = new Output(&manager);
    auto *secondaryOutput = new Output(&manager);
    primaryOutput->d->m_uuid = QStringLiteral("primary");
    secondaryOutput->d->m_uuid = QStringLiteral("secondary");
    manager.d->m_outputs.append(primaryOutput);
    manager.d->m_outputs.append(secondaryOutput);
    manager.d->m_primaryOutput = primaryOutput;
    manager.d->m_initialized = true;
    manager.d->m_backend = new WaylandClient::KywcOutputManager(&manager);

    QPointer<Output> primaryGuard(primaryOutput);
    QPointer<Output> secondaryGuard(secondaryOutput);
    QSignalSpy availableSpy(&manager, &OutputManager::availableChanged);
    QSignalSpy primaryChangedSpy(&manager, &OutputManager::primaryOutputChanged);
    QSignalSpy outputRemovedSpy(&manager, &OutputManager::outputRemoved);

    QObject::connect(&manager, &OutputManager::availableChanged, &manager, [&manager, &primaryGuard, &secondaryGuard](bool available) {
        QVERIFY(!available);
        QVERIFY(!manager.isAvailable());
        QVERIFY(!manager.isInitialized());
        QVERIFY(manager.outputs().isEmpty());
        QVERIFY(manager.primaryOutput() == nullptr);
        QVERIFY(primaryGuard);
        QVERIFY(secondaryGuard);
    });

    manager.d->invalidate();

    QCOMPARE(availableSpy.count(), 1);
    QCOMPARE(availableSpy.first().first().toBool(), false);
    QCOMPARE(primaryChangedSpy.count(), 0);
    QCOMPARE(outputRemovedSpy.count(), 0);
    QVERIFY(primaryGuard);
    QVERIFY(secondaryGuard);

    flushDeferredDeletes();
    QVERIFY(primaryGuard.isNull());
    QVERIFY(secondaryGuard.isNull());
}

void OutputManagerTest::invalidatingAnUnavailableManagerDoesNotRepeatLifecycleSignals()
{
    OutputManager manager;
    QSignalSpy availableSpy(&manager, &OutputManager::availableChanged);
    QSignalSpy primaryChangedSpy(&manager, &OutputManager::primaryOutputChanged);

    manager.d->invalidate();

    QCOMPARE(availableSpy.count(), 0);
    QCOMPARE(primaryChangedSpy.count(), 0);
}

}

QTEST_GUILESS_MAIN(UkuiQuick::Test::OutputManagerTest)

#include "test-public-output-manager.moc"
