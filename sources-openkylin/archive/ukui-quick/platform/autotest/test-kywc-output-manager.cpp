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

#include <QDebug>
#include <QCoreApplication>
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

#include <KWayland/Client/event_queue.h>

#include <memory>
#include <utility>

#include "wayland-kywc-output-v1-client-protocol.h"
#include "../wayland/wayland-pointer_p.h"

// This only affects the test translation unit. The production source remains
// unchanged while its existing test friend declaration grants white-box access.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
#endif
#define private public
#include "../wayland/kywc-output-manager.cpp"
#undef private
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

Q_DECLARE_METATYPE(UkuiQuick::WaylandClient::KywcOutput::StateMasks)

namespace UkuiQuick::WaylandClient::Test
{

namespace
{

template<typename T>
T *fakeHandle(quintptr value)
{
    return reinterpret_cast<T *>(value);
}

void flushDeferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

}

class KywcOutputManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void defaultObjectsExposeDocumentedDefaults();
    void modeCallbacksAreCommittedOnlyAtTheOutputDoneBoundary();
    void outputCallbacksAggregateEveryStateMaskAndIgnoreForeignHandles();
    void donePublishesPendingOutputAndAggregatesState();
    void pendingAndPublishedModesHaveDistinctFinishedBehavior();
    void outputRemovalClearsPrimaryBeforeReportingUuid();
    void stoppingManagerCommitsEventsButInvalidatedManagerIgnoresThem();
    void finishedCallbacksRemoveModesOutputsAndManagerState();
};

void KywcOutputManagerTest::initTestCase()
{
    qRegisterMetaType<KywcOutput::StateMasks>("StateMasks");
    qRegisterMetaType<KywcOutput::StateMasks>("KywcOutput::StateMasks");
    qRegisterMetaType<KywcOutput *>("KywcOutput*");
    qRegisterMetaType<const KywcOutput *>("const KywcOutput*");
    qRegisterMetaType<KywcOutputMode *>("KywcOutputMode*");
    qRegisterMetaType<const KywcOutputMode *>("const KywcOutputMode*");
}

void KywcOutputManagerTest::defaultObjectsExposeDocumentedDefaults()
{
    KywcOutputMode mode;
    QCOMPARE(mode.id(), 0);
    QCOMPARE(mode.size(), QSize());
    QCOMPARE(mode.refreshRate(), 0);
    QVERIFY(!mode.isPreferred());

    KywcOutput output;
    QCOMPARE(output.name(), QString());
    QCOMPARE(output.make(), QString());
    QCOMPARE(output.model(), QString());
    QCOMPARE(output.serialNumber(), QString());
    QCOMPARE(output.description(), QString());
    QCOMPARE(output.uuid(), QString());
    QCOMPARE(output.physicalSize(), QSize());
    QCOMPARE(output.capabilities(), 0U);
    QVERIFY(!output.isEnabled());
    QCOMPARE(output.position(), QPoint());
    QCOMPARE(output.transform(), 0);
    QCOMPARE(output.scale(), 1.0);
    QCOMPARE(output.power(), 0U);
    QCOMPARE(output.brightness(), 0U);
    QCOMPARE(output.colorTemp(), 0U);
    QVERIFY(output.modes().isEmpty());
    QVERIFY(output.currentMode() == nullptr);

    KywcOutputManager manager;
    QVERIFY(!manager.isValid());
    QVERIFY(!manager.isInitialized());
    QVERIFY(manager.outputs().isEmpty());
    QVERIFY(manager.primaryOutput() == nullptr);
    QVERIFY(manager.eventQueue() == nullptr);
}

void KywcOutputManagerTest::modeCallbacksAreCommittedOnlyAtTheOutputDoneBoundary()
{
    KywcOutputMode mode;
    const auto *modeHandle = fakeHandle<kywc_output_mode_v1>(0x201);
    const auto *otherModeHandle = fakeHandle<kywc_output_mode_v1>(0x202);
    mode.d->m_backendMode.setup(const_cast<kywc_output_mode_v1 *>(modeHandle), true);

    KywcOutputMode::Private::sizeCallback(mode.d.data(),
                                           const_cast<kywc_output_mode_v1 *>(otherModeHandle),
                                           1024,
                                           768);
    QCOMPARE(mode.size(), QSize());

    KywcOutputMode::Private::sizeCallback(mode.d.data(),
                                           const_cast<kywc_output_mode_v1 *>(modeHandle),
                                           1920,
                                           1080);
    KywcOutputMode::Private::refreshCallback(mode.d.data(),
                                              const_cast<kywc_output_mode_v1 *>(modeHandle),
                                              60000);
    KywcOutputMode::Private::preferredCallback(mode.d.data(),
                                                const_cast<kywc_output_mode_v1 *>(modeHandle));

    QCOMPARE(mode.size(), QSize(1920, 1080));
    QCOMPARE(mode.refreshRate(), 60000);
    QVERIFY(mode.isPreferred());
    QVERIFY(mode.d->commitChanged(true));
    QVERIFY(!mode.d->commitChanged(true));
}

void KywcOutputManagerTest::outputCallbacksAggregateEveryStateMaskAndIgnoreForeignHandles()
{
    KywcOutput output;
    const auto *outputHandle = fakeHandle<kywc_output_v1>(0x501);
    const auto *otherOutputHandle = fakeHandle<kywc_output_v1>(0x502);
    output.d->m_backendOutput.setup(const_cast<kywc_output_v1 *>(outputHandle), true);

    auto *mode = new KywcOutputMode(&output);
    const auto *modeHandle = fakeHandle<kywc_output_mode_v1>(0x503);
    mode->d->m_backendMode.setup(const_cast<kywc_output_mode_v1 *>(modeHandle), true);
    mode->d->m_id = 5;
    output.d->m_modes.append(mode);

    QSignalSpy stateUpdatedSpy(&output, &KywcOutput::stateUpdated);

    KywcOutput::Private::enabledCallback(output.d.data(),
                                          const_cast<kywc_output_v1 *>(otherOutputHandle),
                                          1);
    QVERIFY(!output.isEnabled());
    QCOMPARE(output.d->m_stateMasks, KywcOutput::StateMasks {KywcOutput::StateMask::None});

    KywcOutput::Private::nameCallback(output.d.data(),
                                       const_cast<kywc_output_v1 *>(outputHandle),
                                       "Display-1");
    KywcOutput::Private::makeCallback(output.d.data(),
                                       const_cast<kywc_output_v1 *>(outputHandle),
                                       "Kylin");
    KywcOutput::Private::modelCallback(output.d.data(),
                                        const_cast<kywc_output_v1 *>(outputHandle),
                                        "KM-27");
    KywcOutput::Private::serialNumberCallback(output.d.data(),
                                               const_cast<kywc_output_v1 *>(outputHandle),
                                               "serial-1");
    KywcOutput::Private::descriptionCallback(output.d.data(),
                                              const_cast<kywc_output_v1 *>(outputHandle),
                                              "Primary display");
    KywcOutput::Private::physicalSizeCallback(output.d.data(),
                                               const_cast<kywc_output_v1 *>(outputHandle),
                                               600,
                                               340);
    KywcOutput::Private::capabilitiesCallback(output.d.data(),
                                               const_cast<kywc_output_v1 *>(outputHandle),
                                               0x5a);
    KywcOutput::Private::enabledCallback(output.d.data(),
                                          const_cast<kywc_output_v1 *>(outputHandle),
                                          1);
    KywcOutput::Private::currentModeCallback(output.d.data(),
                                              const_cast<kywc_output_v1 *>(outputHandle),
                                              const_cast<kywc_output_mode_v1 *>(modeHandle));
    KywcOutput::Private::positionCallback(output.d.data(),
                                           const_cast<kywc_output_v1 *>(outputHandle),
                                           10,
                                           20);
    KywcOutput::Private::transformCallback(output.d.data(),
                                            const_cast<kywc_output_v1 *>(outputHandle),
                                            3);
    KywcOutput::Private::scaleCallback(output.d.data(),
                                        const_cast<kywc_output_v1 *>(outputHandle),
                                        wl_fixed_from_double(1.25));
    KywcOutput::Private::powerCallback(output.d.data(),
                                        const_cast<kywc_output_v1 *>(outputHandle),
                                        2);
    KywcOutput::Private::brightnessCallback(output.d.data(),
                                             const_cast<kywc_output_v1 *>(outputHandle),
                                             80);
    KywcOutput::Private::colorTempCallback(output.d.data(),
                                            const_cast<kywc_output_v1 *>(outputHandle),
                                            6500);

    QCOMPARE(output.name(), QStringLiteral("Display-1"));
    QCOMPARE(output.make(), QStringLiteral("Kylin"));
    QCOMPARE(output.model(), QStringLiteral("KM-27"));
    QCOMPARE(output.serialNumber(), QStringLiteral("serial-1"));
    QCOMPARE(output.description(), QStringLiteral("Primary display"));
    QCOMPARE(output.physicalSize(), QSize(600, 340));
    QCOMPARE(output.capabilities(), 0x5aU);
    QVERIFY(output.isEnabled());
    QCOMPARE(output.currentMode(), mode);
    QCOMPARE(output.position(), QPoint(10, 20));
    QCOMPARE(output.transform(), 3);
    QCOMPARE(output.scale(), 1.25);
    QCOMPARE(output.power(), 2U);
    QCOMPARE(output.brightness(), 80U);
    QCOMPARE(output.colorTemp(), 6500U);

    output.d->commitChanged();

    QCOMPARE(stateUpdatedSpy.count(), 1);
    const auto masks = qvariant_cast<KywcOutput::StateMasks>(stateUpdatedSpy.first().first());
    const auto expectedMasks = KywcOutput::StateMask::Enabled
        | KywcOutput::StateMask::CurrentMode
        | KywcOutput::StateMask::Position
        | KywcOutput::StateMask::Transform
        | KywcOutput::StateMask::Scale
        | KywcOutput::StateMask::Power
        | KywcOutput::StateMask::Brightness
        | KywcOutput::StateMask::ColorTemp;
    QCOMPARE(masks, expectedMasks);
}

void KywcOutputManagerTest::donePublishesPendingOutputAndAggregatesState()
{
    KywcOutputManager manager;
    const auto *managerHandle = fakeHandle<kywc_output_manager_v1>(0x301);
    manager.d->m_manager.setup(const_cast<kywc_output_manager_v1 *>(managerHandle), true);
    manager.d->m_state = KywcOutputManager::State::Bound;

    auto *backendOutput = new KywcOutput(&manager);
    const auto *outputHandle = fakeHandle<kywc_output_v1>(0x302);
    backendOutput->d->m_backendOutput.setup(const_cast<kywc_output_v1 *>(outputHandle), true);
    backendOutput->d->m_uuid = QStringLiteral("output-1");
    backendOutput->d->m_enabled = true;
    backendOutput->d->m_position = QPoint(100, 200);
    backendOutput->d->m_stateMasks = KywcOutput::StateMask::Enabled
        | KywcOutput::StateMask::Position;

    auto *mode = new KywcOutputMode(backendOutput);
    const auto *modeHandle = fakeHandle<kywc_output_mode_v1>(0x303);
    mode->d->m_backendMode.setup(const_cast<kywc_output_mode_v1 *>(modeHandle), true);
    mode->d->m_id = 1;
    mode->d->m_size = QSize(1920, 1080);
    backendOutput->d->m_pendingModes.append(mode);
    backendOutput->d->m_currentMode = mode;
    backendOutput->d->m_stateMasks |= KywcOutput::StateMask::CurrentMode;

    manager.d->m_outputPendingList.append(backendOutput);
    QSignalSpy outputCreatedSpy(&manager, &KywcOutputManager::outputCreated);
    QSignalSpy primaryChangedSpy(&manager, &KywcOutputManager::primaryOutputChanged);
    QSignalSpy initializedSpy(&manager, &KywcOutputManager::initialized);
    QSignalSpy doneSpy(&manager, &KywcOutputManager::done);
    QSignalSpy modeAddedSpy(backendOutput, &KywcOutput::modeAdded);
    QSignalSpy stateUpdatedSpy(backendOutput, &KywcOutput::stateUpdated);

    KywcOutputManager::Private::primaryCallback(manager.d.data(),
                                                 const_cast<kywc_output_manager_v1 *>(managerHandle),
                                                 const_cast<kywc_output_v1 *>(outputHandle));
    KywcOutputManager::Private::doneCallback(manager.d.data(),
                                              const_cast<kywc_output_manager_v1 *>(managerHandle));

    QCOMPARE(outputCreatedSpy.count(), 1);
    QCOMPARE(primaryChangedSpy.count(), 1);
    QCOMPARE(initializedSpy.count(), 1);
    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(modeAddedSpy.count(), 1);
    QCOMPARE(stateUpdatedSpy.count(), 1);
    QVERIFY(manager.isInitialized());
    QCOMPARE(manager.outputs(), QList<KywcOutput *> {backendOutput});
    QCOMPARE(manager.primaryOutput(), backendOutput);
    QCOMPARE(backendOutput->modes(), QList<KywcOutputMode *> {mode});
    QCOMPARE(backendOutput->currentMode(), mode);

    const auto masks = qvariant_cast<KywcOutput::StateMasks>(stateUpdatedSpy.first().first());
    QVERIFY(masks.testFlag(KywcOutput::StateMask::Enabled));
    QVERIFY(masks.testFlag(KywcOutput::StateMask::Position));
    QVERIFY(masks.testFlag(KywcOutput::StateMask::CurrentMode));

    KywcOutputManager::Private::doneCallback(manager.d.data(),
                                              const_cast<kywc_output_manager_v1 *>(managerHandle));
    QCOMPARE(initializedSpy.count(), 1);
    QCOMPARE(doneSpy.count(), 2);
}

void KywcOutputManagerTest::pendingAndPublishedModesHaveDistinctFinishedBehavior()
{
    KywcOutput output;

    auto *pendingMode = new KywcOutputMode(&output);
    const auto *pendingHandle = fakeHandle<kywc_output_mode_v1>(0x601);
    pendingMode->d->m_backendMode.setup(const_cast<kywc_output_mode_v1 *>(pendingHandle), true);
    pendingMode->d->m_id = 6;
    output.d->m_pendingModes.append(pendingMode);
    QPointer<KywcOutputMode> pendingModeGuard(pendingMode);

    auto *publishedMode = new KywcOutputMode(&output);
    const auto *publishedHandle = fakeHandle<kywc_output_mode_v1>(0x602);
    publishedMode->d->m_backendMode.setup(const_cast<kywc_output_mode_v1 *>(publishedHandle), true);
    publishedMode->d->m_id = 7;
    output.d->m_modes.append(publishedMode);
    output.d->m_currentMode = publishedMode;
    QPointer<KywcOutputMode> publishedModeGuard(publishedMode);

    QObject::connect(pendingMode, &KywcOutputMode::removed, &output, [pendingMode, &output] {
        output.d->removeMode(pendingMode);
    });
    QObject::connect(publishedMode, &KywcOutputMode::removed, &output, [publishedMode, &output] {
        output.d->removeMode(publishedMode);
    });
    QSignalSpy modeRemovedSpy(&output, &KywcOutput::modeRemoved);

    KywcOutputMode::Private::finishedCallback(pendingMode->d.data(),
                                               const_cast<kywc_output_mode_v1 *>(pendingHandle));
    flushDeferredDeletes();

    QCOMPARE(modeRemovedSpy.count(), 0);
    QVERIFY(output.d->m_pendingModes.isEmpty());
    QVERIFY(!pendingModeGuard);

    KywcOutputMode::Private::finishedCallback(publishedMode->d.data(),
                                               const_cast<kywc_output_mode_v1 *>(publishedHandle));
    flushDeferredDeletes();

    QCOMPARE(modeRemovedSpy.count(), 1);
    QCOMPARE(modeRemovedSpy.first().first().toInt(), 7);
    QVERIFY(output.modes().isEmpty());
    QVERIFY(output.currentMode() == nullptr);
    QVERIFY(!publishedModeGuard);
}

void KywcOutputManagerTest::outputRemovalClearsPrimaryBeforeReportingUuid()
{
    KywcOutputManager manager;
    const auto *managerHandle = fakeHandle<kywc_output_manager_v1>(0x701);
    manager.d->m_manager.setup(const_cast<kywc_output_manager_v1 *>(managerHandle), true);
    manager.d->m_state = KywcOutputManager::State::Bound;

    auto *output = new KywcOutput(&manager);
    const auto *outputHandle = fakeHandle<kywc_output_v1>(0x702);
    output->d->m_backendOutput.setup(const_cast<kywc_output_v1 *>(outputHandle), true);
    output->d->m_uuid = QStringLiteral("output-7");
    QPointer<KywcOutput> outputGuard(output);
    QObject::connect(output, &KywcOutput::removed, &manager, [&manager, output] {
        manager.d->removeOutput(output);
    });
    manager.d->m_outputs.append(output);
    manager.d->m_primaryOutput = output;

    QSignalSpy primaryChangedSpy(&manager, &KywcOutputManager::primaryOutputChanged);
    QSignalSpy outputRemovedSpy(&manager, &KywcOutputManager::outputRemoved);
    QStringList signalOrder;
    QObject::connect(&manager,
                     &KywcOutputManager::primaryOutputChanged,
                     &manager,
                     [&signalOrder](const KywcOutput *primary) {
                         signalOrder.append(primary ? QStringLiteral("primary")
                                                    : QStringLiteral("primary:null"));
                     });
    QObject::connect(&manager,
                     &KywcOutputManager::outputRemoved,
                     &manager,
                     [&signalOrder](const QString &uuid) {
                         signalOrder.append(QStringLiteral("removed:") + uuid);
                     });

    KywcOutput::Private::finishedCallback(output->d.data(),
                                          const_cast<kywc_output_v1 *>(outputHandle));
    flushDeferredDeletes();

    QCOMPARE(primaryChangedSpy.count(), 1);
    QCOMPARE(outputRemovedSpy.count(), 1);
    QCOMPARE(outputRemovedSpy.first().first().toString(), QStringLiteral("output-7"));
    QCOMPARE(signalOrder, QStringList({QStringLiteral("primary:null"),
                                       QStringLiteral("removed:output-7")}));
    QVERIFY(manager.outputs().isEmpty());
    QVERIFY(manager.primaryOutput() == nullptr);
    QVERIFY(!outputGuard);
}

void KywcOutputManagerTest::stoppingManagerCommitsEventsButInvalidatedManagerIgnoresThem()
{
    KywcOutputManager manager;
    const auto *managerHandle = fakeHandle<kywc_output_manager_v1>(0x801);
    manager.d->m_manager.setup(const_cast<kywc_output_manager_v1 *>(managerHandle), true);
    manager.d->m_state = KywcOutputManager::State::Stopping;

    auto *pendingOutput = new KywcOutput(&manager);
    const auto *outputHandle = fakeHandle<kywc_output_v1>(0x802);
    pendingOutput->d->m_backendOutput.setup(const_cast<kywc_output_v1 *>(outputHandle), true);
    pendingOutput->d->m_uuid = QStringLiteral("output-8");
    QPointer<KywcOutput> pendingOutputGuard(pendingOutput);
    manager.d->m_outputPendingList.append(pendingOutput);

    QSignalSpy outputCreatedSpy(&manager, &KywcOutputManager::outputCreated);
    QSignalSpy initializedSpy(&manager, &KywcOutputManager::initialized);
    QSignalSpy doneSpy(&manager, &KywcOutputManager::done);
    QSignalSpy primaryChangedSpy(&manager, &KywcOutputManager::primaryOutputChanged);
    QSignalSpy removedSpy(&manager, &KywcOutputManager::removed);

    KywcOutputManager::Private::doneCallback(manager.d.data(),
                                              const_cast<kywc_output_manager_v1 *>(managerHandle));

    QCOMPARE(outputCreatedSpy.count(), 1);
    QCOMPARE(initializedSpy.count(), 1);
    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(manager.outputs(), QList<KywcOutput *> {pendingOutput});
    QVERIFY(manager.isInitialized());

    KywcOutputManager::Private::finishedCallback(manager.d.data(),
                                                  const_cast<kywc_output_manager_v1 *>(managerHandle));

    QCOMPARE(removedSpy.count(), 1);
    QVERIFY(!manager.isValid());
    QVERIFY(!manager.isInitialized());
    QVERIFY(manager.outputs().isEmpty());
    QVERIFY(!pendingOutputGuard);

    KywcOutputManager::Private::primaryCallback(manager.d.data(),
                                                 const_cast<kywc_output_manager_v1 *>(managerHandle),
                                                 const_cast<kywc_output_v1 *>(outputHandle));
    KywcOutputManager::Private::doneCallback(manager.d.data(),
                                              const_cast<kywc_output_manager_v1 *>(managerHandle));
    KywcOutputManager::Private::finishedCallback(manager.d.data(),
                                                  const_cast<kywc_output_manager_v1 *>(managerHandle));

    QCOMPARE(outputCreatedSpy.count(), 1);
    QCOMPARE(initializedSpy.count(), 1);
    QCOMPARE(doneSpy.count(), 1);
    QCOMPARE(primaryChangedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
}

void KywcOutputManagerTest::finishedCallbacksRemoveModesOutputsAndManagerState()
{
    KywcOutputManager manager;
    const auto *managerHandle = fakeHandle<kywc_output_manager_v1>(0x401);
    manager.d->m_manager.setup(const_cast<kywc_output_manager_v1 *>(managerHandle), true);
    manager.d->m_state = KywcOutputManager::State::Bound;
    manager.d->m_initialized = true;

    auto *output = new KywcOutput(&manager);
    const auto *outputHandle = fakeHandle<kywc_output_v1>(0x402);
    output->d->m_backendOutput.setup(const_cast<kywc_output_v1 *>(outputHandle), true);
    output->d->m_uuid = QStringLiteral("output-2");

    manager.d->m_outputs.append(output);
    manager.d->m_primaryOutput = output;
    QSignalSpy removedSpy(&manager, &KywcOutputManager::removed);

    KywcOutputManager::Private::finishedCallback(manager.d.data(),
                                                  const_cast<kywc_output_manager_v1 *>(managerHandle));
    QCOMPARE(removedSpy.count(), 1);
    QVERIFY(!manager.isValid());
    QVERIFY(!manager.isInitialized());
    QVERIFY(manager.outputs().isEmpty());
    QVERIFY(manager.primaryOutput() == nullptr);

    KywcOutputManager::Private::finishedCallback(manager.d.data(),
                                                  const_cast<kywc_output_manager_v1 *>(managerHandle));
    QCOMPARE(removedSpy.count(), 1);
}

}

QTEST_GUILESS_MAIN(UkuiQuick::WaylandClient::Test::KywcOutputManagerTest)

#include "test-kywc-output-manager.moc"
