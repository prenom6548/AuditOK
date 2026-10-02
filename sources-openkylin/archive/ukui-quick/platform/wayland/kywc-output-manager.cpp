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
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "kywc-output-manager.h"
#include "wayland-pointer_p.h"
#include "wayland-kywc-output-v1-client-protocol.h"
#include <QDebug>
#include <QSize>
#include <QPoint>
#include <memory>
#include <utility>

namespace UkuiQuick::WaylandClient {

class KywcOutputMode::Private
{
public:
    explicit Private(KywcOutputMode *q);
    ~Private();
    void init_listener(kywc_output_mode_v1 *mode);
    void release();
    bool commitChanged(bool notify);

    static void sizeCallback(void *data, kywc_output_mode_v1 *mode, int32_t width, int32_t height);
    static void refreshCallback(void *data, kywc_output_mode_v1 *mode, int32_t refresh);
    static void preferredCallback(void *data, kywc_output_mode_v1 *mode);
    static void finishedCallback(void *data, kywc_output_mode_v1 *mode);

    static struct kywc_output_mode_v1_listener s_modeListener;
    WaylandPointer<kywc_output_mode_v1, kywc_output_mode_v1_destroy> m_backendMode;
    KywcOutputMode *q {nullptr};

    // local id, 用于区分不同的mode
    int m_id = 0;
    QSize m_size;
    int m_refreshRate = 0;
    bool m_preferred = false;
    bool m_isChanged = false;
};


KywcOutputMode::KywcOutputMode(QObject *parent) : QObject(parent), d(new Private(this))
{
}

int KywcOutputMode::id() const
{
    return d->m_id;
}

QSize KywcOutputMode::size() const
{
    return d->m_size;
}

int KywcOutputMode::refreshRate() const
{
    return d->m_refreshRate;
}

bool KywcOutputMode::isPreferred() const
{
    return d->m_preferred;
}

KywcOutputMode::~KywcOutputMode() = default;

void KywcOutputMode::setup(kywc_output_mode_v1 *mode)
{
    Q_ASSERT(!d->m_backendMode);
    Q_ASSERT(mode);
    d->m_backendMode.setup(mode);
    d->init_listener(mode);
}

KywcOutputMode::Private::Private(KywcOutputMode *q) : q(q)
{
}

KywcOutputMode::Private::~Private() = default;

void KywcOutputMode::Private::release()
{
    if (m_backendMode) {
        m_backendMode.release();
    }
}

bool KywcOutputMode::Private::commitChanged(bool notify)
{
    //返回当前值，并重置为false，用于后续判断是否有变化
    return std::exchange(m_isChanged, false) && notify;
}

//finished: 此事件表示该输出设备已不再可用。输出对象将变为无效状态。客户端应发送销毁请求并释放与之关联的所有资源。
kywc_output_mode_v1_listener KywcOutputMode::Private::s_modeListener = {
    .size = sizeCallback,
    .refresh = refreshCallback,
    .preferred = preferredCallback,
    .finished = finishedCallback
};


void KywcOutputMode::Private::init_listener(kywc_output_mode_v1 *mode)
{
     kywc_output_mode_v1_add_listener(mode, &s_modeListener, this);
}

void KywcOutputMode::Private::sizeCallback(void *data, kywc_output_mode_v1 *mode, int32_t width, int32_t height)
{
    auto* p = reinterpret_cast<KywcOutputMode::Private *>(data);
    if (p->m_backendMode != mode) {
        return;
    }
    p->m_size = QSize(width, height);
    p->m_isChanged = true;
}

void KywcOutputMode::Private::refreshCallback(void *data, kywc_output_mode_v1 *mode, int32_t refresh)
{
    auto* p = reinterpret_cast<KywcOutputMode::Private *>(data);
    if (p->m_backendMode != mode) {
        return;
    }
    p->m_refreshRate = refresh;
    p->m_isChanged = true;
}

void KywcOutputMode::Private::preferredCallback(void *data, kywc_output_mode_v1 *mode)
{
    auto* p = reinterpret_cast<KywcOutputMode::Private *>(data);
    if (p->m_backendMode != mode) {
        return;
    }
    p->m_preferred = true;
    p->m_isChanged = true;
}

void KywcOutputMode::Private::finishedCallback(void *data, kywc_output_mode_v1 *mode)
{
    auto* p = reinterpret_cast<KywcOutputMode::Private *>(data);
    if (p->m_backendMode != mode) {
        return;
    }
    p->release();
    Q_EMIT p->q->removed();
}

/**********KywcOutput*********/

class KywcOutput::Private
{
public:
    explicit Private(KywcOutput *q);
    ~Private();

    void init_listener(kywc_output_v1 *output);
    void release();
    void removeMode(KywcOutputMode *mode);
    void tearDown();
    void commitChanged();

    static struct kywc_output_v1_listener s_outputListener;
    static void nameCallback(void *data, kywc_output_v1 *output, const char *name);
    static void makeCallback(void *data, kywc_output_v1 *output, const char *make);
    static void modelCallback(void *data, kywc_output_v1 *output, const char *model);
    static void serialNumberCallback(void *data, kywc_output_v1 *output, const char *serial);
    static void descriptionCallback(void *data, kywc_output_v1 *output, const char *description);
    static void physicalSizeCallback(void *data, kywc_output_v1 *output, int32_t width, int32_t height);
    static void capabilitiesCallback(void *data, kywc_output_v1 *output, uint32_t flags);
    static void enabledCallback(void *data, kywc_output_v1 *output, int32_t enabled);
    static void modeCallback(void *data, kywc_output_v1 *output, kywc_output_mode_v1 *mode);
    static void currentModeCallback(void *data, kywc_output_v1 *output, kywc_output_mode_v1 *mode);
    static void positionCallback(void *data, kywc_output_v1 *output, int32_t x, int32_t y);
    static void transformCallback(void *data, kywc_output_v1 *output, int32_t transform);
    static void scaleCallback(void *data, kywc_output_v1 *output, wl_fixed_t scale);
    static void powerCallback(void *data, kywc_output_v1 *output, uint32_t power);
    static void brightnessCallback(void *data, kywc_output_v1 *output, uint32_t brightness);
    static void colorTempCallback(void *data, kywc_output_v1 *output, uint32_t colorTemp);
    static void finishedCallback(void *data, kywc_output_v1 *output);

    WaylandPointer<kywc_output_v1, kywc_output_v1_destroy> m_backendOutput;
    KywcOutput *q {nullptr};
    EventQueue *m_queue {nullptr};
    QList<KywcOutputMode *> m_modes;
    QList<KywcOutputMode *> m_pendingModes;

    KywcOutputMode *m_currentMode {nullptr};

    QString m_name;
    QString m_make;
    QString m_model;
    QString m_serialNumber;
    QString m_description;
    QSize m_physicalSize;
    uint32_t m_capabilities = 0;
    bool m_enabled {false};
    QPoint m_position;
    int m_transform = 0;
    qreal m_scale = 1.0;
    uint32_t m_power = 0;
    uint32_t m_brightness = 0;
    uint32_t m_colorTemp = 0;
    QString m_uuid;
    StateMasks m_stateMasks {StateMask::None};
    //local id, 作为mode的唯一标识
    int m_nextModeId = 1;
};

KywcOutput::KywcOutput(QObject *parent) : QObject(parent), d(new Private(this))
{
}

KywcOutput::~KywcOutput() = default;

void KywcOutput::setup(kywc_output_v1 *output)
{
    Q_ASSERT(!d->m_backendOutput);
    Q_ASSERT(output);
    d->m_backendOutput.setup(output);
    d->init_listener(output);
}

void KywcOutput::setEventQueue(EventQueue* queue)
{
    d->m_queue = queue;
}

QString KywcOutput::name() const
{
    return d->m_name;
}

QString KywcOutput::make() const
{
    return d->m_make;
}

QString KywcOutput::model() const
{
    return d->m_model;
}

QString KywcOutput::serialNumber() const
{
    return d->m_serialNumber;
}

QString KywcOutput::description() const
{
    return d->m_description;
}

QSize KywcOutput::physicalSize() const
{
    return d->m_physicalSize;
}

uint32_t KywcOutput::capabilities() const
{
    return d->m_capabilities;
}

bool KywcOutput::isEnabled() const
{
    return d->m_enabled;
}

QPoint KywcOutput::position() const
{
    return d->m_position;
}

int KywcOutput::transform() const
{
    return d->m_transform;
}

qreal KywcOutput::scale() const
{
    return d->m_scale;
}

uint32_t KywcOutput::power() const
{
    return d->m_power;
}

uint32_t KywcOutput::brightness() const
{
    return d->m_brightness;
}

uint32_t KywcOutput::colorTemp() const
{
    return d->m_colorTemp;
}

QString KywcOutput::uuid() const
{
    return d->m_uuid;
}

QList<KywcOutputMode*> KywcOutput::modes() const
{
    return d->m_modes;
}

KywcOutputMode* KywcOutput::currentMode() const
{
    return d->m_currentMode;
}

KywcOutput::Private::Private(KywcOutput *q) : q(q)
{
}

KywcOutput::Private::~Private()
{
    tearDown();
}

void KywcOutput::Private::removeMode(KywcOutputMode *mode)
{
    if (!mode) {
        return;
    }
    if (m_currentMode == mode) {
        m_currentMode = nullptr;
    }
    if (m_pendingModes.removeOne(mode)) {
        mode->deleteLater();
        return;
    }
    if (m_modes.removeOne(mode)) {
        Q_EMIT q->modeRemoved(mode->id());
        mode->deleteLater();
    }
}

void KywcOutput::Private::tearDown()
{
    auto modes = std::exchange(m_modes, {});
    auto pendingModes = std::exchange(m_pendingModes, {});
    m_stateMasks = StateMask::None;
    m_currentMode = nullptr;
    m_nextModeId = 1;
    qDeleteAll(modes);
    qDeleteAll(pendingModes);
    release();
}

void KywcOutput::Private::commitChanged()
{
    const auto existingModes = m_modes;
    const auto pendingModes = std::exchange(m_pendingModes, {});
    //mode add
    if (!pendingModes.isEmpty()) {
        m_modes.append(pendingModes);
        for (const auto *mode : pendingModes) {
            mode->d->commitChanged(false);
            Q_EMIT q->modeAdded(mode);
        }
    }

    //mode changed
    for (const auto *mode : existingModes) {
        if (mode->d->commitChanged(true)) {
            Q_EMIT q->modeChanged(mode);
        }
    }

    //other changed
    if (m_stateMasks != StateMasks{StateMask::None}) {
        Q_EMIT q->stateUpdated(m_stateMasks);
        m_stateMasks = StateMask::None;
    }
}

//finished: 此事件表示该输出设备已不再可用。输出对象将变为无效状态。客户端应发送销毁请求并释放与之关联的所有资源。
kywc_output_v1_listener KywcOutput::Private::s_outputListener = {
    .name = nameCallback,
    .make = makeCallback,
    .model = modelCallback,
    .serial_number = serialNumberCallback,
    .description = descriptionCallback,
    .physical_size = physicalSizeCallback,
    .mode = modeCallback,
    .capabilities = capabilitiesCallback,
    .enabled = enabledCallback,
    .current_mode = currentModeCallback,
    .position = positionCallback,
    .transform = transformCallback,
    .scale = scaleCallback,
    .power = powerCallback,
    .brightness = brightnessCallback,
    .color_temp = colorTempCallback,
    .finished = finishedCallback
};

void KywcOutput::Private::init_listener(kywc_output_v1 *output)
{
    kywc_output_v1_add_listener(output, &s_outputListener, this);
}

void KywcOutput::Private::release()
{
    if (m_backendOutput) {
        m_backendOutput.release();
    }
}

void KywcOutput::Private::nameCallback(void *data, kywc_output_v1 *output, const char *name)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_name = QString::fromUtf8(name);
}

void KywcOutput::Private::makeCallback(void *data, kywc_output_v1 *output, const char *make)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_make = QString::fromUtf8(make);
}

void KywcOutput::Private::modelCallback(void *data, kywc_output_v1 *output, const char *model)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_model = QString::fromUtf8(model);
}

void KywcOutput::Private::serialNumberCallback(void *data, kywc_output_v1 *output, const char *serial)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_serialNumber = QString::fromUtf8(serial);
}

void KywcOutput::Private::descriptionCallback(void *data, kywc_output_v1 *output, const char *description)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_description = QString::fromUtf8(description);
}

void KywcOutput::Private::physicalSizeCallback(void *data, kywc_output_v1 *output, int32_t width, int32_t height)
{
    auto p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_physicalSize = QSize(width, height);
}

void KywcOutput::Private::capabilitiesCallback(void *data, kywc_output_v1 *output, uint32_t flags)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_capabilities = flags;
}

void KywcOutput::Private::enabledCallback(void *data, kywc_output_v1 *output, int32_t enabled)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_enabled = (enabled != 0);
    p->m_stateMasks |= StateMask::Enabled;
}

void KywcOutput::Private::modeCallback(void *data, kywc_output_v1 *output, kywc_output_mode_v1 *mode)
{
    std::unique_ptr<kywc_output_mode_v1, decltype(&kywc_output_mode_v1_destroy)> modeGuard(mode, kywc_output_mode_v1_destroy);
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }

    auto* kywcMode = new KywcOutputMode(p->q);
    kywcMode->d->m_id = p->m_nextModeId++;
    p->m_pendingModes.append(kywcMode);
    if (p->m_queue) {
        p->m_queue->addProxy(mode);
    }
    kywcMode->setup(modeGuard.release());
    connect(kywcMode, &KywcOutputMode::removed, p->q, [p, kywcMode]() {
        p->removeMode(kywcMode);
    });
}

void KywcOutput::Private::currentModeCallback(void *data, kywc_output_v1 *output, kywc_output_mode_v1 *mode)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }

    KywcOutputMode *current = nullptr;
    for (auto* m : p->m_modes) {
        if (m->d->m_backendMode == mode) {
            current = m;
            break;
        }
    }
    if (!current) {
        for (auto* m : p->m_pendingModes) {
            if (m->d->m_backendMode == mode) {
                current = m;
                break;
            }
        }
    }

    if (current) {
        p->m_currentMode = current;
        p->m_stateMasks |= StateMask::CurrentMode;
    } else {
        qWarning() << "KywcOutput::Private::currentModeCallback: No mode found";
    }
}

void KywcOutput::Private::positionCallback(void *data, kywc_output_v1 *output, int32_t x, int32_t y)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_position = QPoint(x, y);
    p->m_stateMasks |= StateMask::Position;
}

void KywcOutput::Private::transformCallback(void *data, kywc_output_v1 *output, int32_t transform)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_transform = transform;
    p->m_stateMasks |= StateMask::Transform;
}

void KywcOutput::Private::scaleCallback(void *data, kywc_output_v1 *output, wl_fixed_t scale)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_scale = wl_fixed_to_double(scale);
    p->m_stateMasks |= StateMask::Scale;
}

void KywcOutput::Private::powerCallback(void *data, kywc_output_v1 *output, uint32_t power)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_power = power;
    p->m_stateMasks |= StateMask::Power;
}

void KywcOutput::Private::brightnessCallback(void *data, kywc_output_v1 *output, uint32_t brightness)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_brightness = brightness;
    p->m_stateMasks |= StateMask::Brightness;
}

void KywcOutput::Private::colorTempCallback(void *data, kywc_output_v1 *output, uint32_t colorTemp)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->m_colorTemp = colorTemp;
    p->m_stateMasks |= StateMask::ColorTemp;
}

void KywcOutput::Private::finishedCallback(void *data, kywc_output_v1 *output)
{
    auto* p = reinterpret_cast<KywcOutput::Private *>(data);
    if (p->m_backendOutput != output) {
        return;
    }
    p->tearDown();
    Q_EMIT p->q->removed();
}

/**********KywcOutputManager*********/

class KywcOutputManager::Private
{
public:
    explicit Private(KywcOutputManager *q);
    ~Private();

    bool canSendRequests() const;
    bool canReceiveEvents() const;

    void init_listener();
    void tearDown();
    void removeOutput(KywcOutput *output);
    void commitPending();
    void transitionTo(KywcOutputManager::State newState);

    static void outputCallback(void *data, kywc_output_manager_v1 *manager, kywc_output_v1 *output, const char *uuid);
    static void primaryCallback(void *data, kywc_output_manager_v1 *manager, kywc_output_v1 *output);
    static void doneCallback(void *data, kywc_output_manager_v1 *manager);
    static void finishedCallback(void *data, kywc_output_manager_v1 *manager);

    static struct kywc_output_manager_v1_listener s_managerListener;
    WaylandPointer<kywc_output_manager_v1, kywc_output_manager_v1_destroy> m_manager;
    EventQueue *m_queue {nullptr};
    KywcOutputManager *q {nullptr};
    QList<KywcOutput *> m_outputs;
    QList<KywcOutput *> m_outputPendingList;
    KywcOutput *m_primaryOutput {nullptr};
    kywc_output_v1 *m_pendingPrimaryOutput {nullptr};
    bool m_initialized {false};
    KywcOutputManager::State m_state = KywcOutputManager::State::Unbound;
};

KywcOutputManager::KywcOutputManager(QObject *parent) : QObject(parent), d(new Private(this))
{
}

KywcOutputManager::~KywcOutputManager() = default;

bool KywcOutputManager::isValid() const
{
    return d->m_manager && d->m_state != State::Unbound && d->m_state != State::Invalidated;
}

void KywcOutputManager::destroy()
{
    if (d->m_state == State::Invalidated) {
        return;
    }

    d->tearDown();
    Q_EMIT removed();
}

void KywcOutputManager::setup(kywc_output_manager_v1 *manager)
{
    Q_ASSERT(!d->m_manager);
    Q_ASSERT(manager);
    d->m_manager.setup(manager);
    d->transitionTo(State::Bound);
    d->init_listener();
}

void KywcOutputManager::setEventQueue(EventQueue *queue)
{
    d->m_queue = queue;
}

EventQueue *KywcOutputManager::eventQueue() const
{
    return d->m_queue;
}

void KywcOutputManager::stop()
{
    if (!d->canSendRequests()) {
        qWarning() << "KywcOutputManager::stop: can not send requests";
        return;
    }
    d->transitionTo(State::Stopping);
    kywc_output_manager_v1_stop(d->m_manager);
}

QList<KywcOutput*> KywcOutputManager::outputs() const
{
    return d->m_outputs;
}

KywcOutput* KywcOutputManager::primaryOutput() const
{
    return d->m_primaryOutput;
}

bool KywcOutputManager::isInitialized() const
{
    return d->m_initialized;
}

KywcOutputManager::Private::Private(KywcOutputManager *q) : q(q)
{
}

KywcOutputManager::Private::~Private()
{
    tearDown();
}

bool KywcOutputManager::Private::canSendRequests() const
{
    return m_manager && m_state == State::Bound;
}

bool KywcOutputManager::Private::canReceiveEvents() const
{
    return m_manager && (m_state == State::Bound || m_state == State::Stopping);
}

void KywcOutputManager::Private::removeOutput(KywcOutput* output)
{
    if (!output) {
        return;
    }
    if (m_primaryOutput == output) {
        m_primaryOutput = nullptr;
        Q_EMIT q->primaryOutputChanged(nullptr);
    }
    m_outputPendingList.removeAll(output);
    m_outputs.removeAll(output);
    Q_EMIT q->outputRemoved(output->uuid());
    output->deleteLater();
}

void KywcOutputManager::Private::tearDown()
{
    transitionTo(State::Invalidated);
    const auto allOutputs = m_outputs + m_outputPendingList;
    m_outputs.clear();
    m_outputPendingList.clear();
    m_primaryOutput = nullptr;
    m_pendingPrimaryOutput = nullptr;
    qDeleteAll(allOutputs);
    m_manager.release();
    m_initialized = false;
}

void KywcOutputManager::Private::transitionTo(KywcOutputManager::State newState)
{
    if (m_state == newState) {
        return;
    }
    m_state = newState;
}

void KywcOutputManager::Private::commitPending()
{
    // done事件发送说明，说明所有output都已就绪，增加到outputs列表中并发送新增信号
    auto existingOutputs = m_outputs;
    while (!m_outputPendingList.isEmpty()) {
        auto *out = m_outputPendingList.takeFirst();
        if (!m_outputs.contains(out)) {
            m_outputs.append(out);
            out->d->commitChanged();
            Q_EMIT q->outputCreated(out);
        }
    }

    for (auto *out : existingOutputs) {
        out->d->commitChanged();
    }

    //标记主屏幕
    if (m_pendingPrimaryOutput) {
        KywcOutput *newPrimary = nullptr;
        for (auto *out : m_outputs) {
            if (out->d->m_backendOutput == m_pendingPrimaryOutput) {
                newPrimary = out;
                break;
            }
        }
        if (newPrimary && m_primaryOutput != newPrimary) {
            m_primaryOutput = newPrimary;
            Q_EMIT q->primaryOutputChanged(newPrimary);
        }
        m_pendingPrimaryOutput = nullptr;
    }
}

void KywcOutputManager::Private::init_listener()
{
    kywc_output_manager_v1_add_listener(m_manager, &s_managerListener, this);
}

//finished: 表示合成器不再发送任何相关事件。对象无效，客户端释放所有资源。
kywc_output_manager_v1_listener KywcOutputManager::Private::s_managerListener = {
    .output = outputCallback,
    .primary = primaryCallback,
    .done = doneCallback,
    .finished = finishedCallback
};

void KywcOutputManager::Private::outputCallback(void *data, kywc_output_manager_v1 *manager, kywc_output_v1 *output, const char *uuid)
{
    std::unique_ptr<kywc_output_v1, decltype(&kywc_output_v1_destroy)>
        outputGuard(output, kywc_output_v1_destroy);

    auto* p = reinterpret_cast<KywcOutputManager::Private *>(data);
    if (p->m_manager != manager || !p->canReceiveEvents()) {
        return;
    }

    auto* out = new KywcOutput(p->q);
    out->d->m_uuid = QString::fromUtf8(uuid);
    if (p->m_queue) {
        p->m_queue->addProxy(output);
    }
    out->setEventQueue(p->m_queue);
    out->setup(outputGuard.release());
    connect(out, &KywcOutput::removed, p->q, [p, out]() {
        p->removeOutput(out);
    });
    p->m_outputPendingList.append(out);
}

void KywcOutputManager::Private::primaryCallback(void *data, kywc_output_manager_v1 *manager, kywc_output_v1 *output)
{
    auto* p = reinterpret_cast<KywcOutputManager::Private *>(data);
    if (p->m_manager != manager || !p->canReceiveEvents()) {
        return;
    }
    p->m_pendingPrimaryOutput = output;
}

void KywcOutputManager::Private::doneCallback(void *data, kywc_output_manager_v1 *manager)
{
    auto* p = reinterpret_cast<KywcOutputManager::Private *>(data);
    if (p->m_manager != manager || !p->canReceiveEvents()) {
        return;
    }

    p->commitPending();
    if (!p->m_initialized) {
        //初始化完成
        p->m_initialized = true;
        Q_EMIT p->q->initialized();
    }
    //一轮事件处理完成后，发送done信号
    Q_EMIT p->q->done();
}

void KywcOutputManager::Private::finishedCallback(void *data, kywc_output_manager_v1 *manager)
{
    auto* p = reinterpret_cast<KywcOutputManager::Private *>(data);
    if (p->m_manager != manager) {
        return;
    }
    if (p->m_state == State::Invalidated) {
        return;
    }
    p->tearDown();
    Q_EMIT p->q->removed();
}

}
