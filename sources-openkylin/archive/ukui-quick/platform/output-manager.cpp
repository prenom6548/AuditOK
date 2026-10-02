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

#include "output-manager.h"

#include <algorithm>
#include <QDebug>
#include <QHash>
#include <QPointer>
#include "kywc-output-manager.h"
#include "registry.h"
#include "wayland-integration_p.h"

namespace UkuiQuick
{
class OutputMode::Private
{
public:
    int m_id = 0;
    bool m_preferred = false;
    int m_refreshRate = 0;
    QSize m_size;
};

/*!
    \class UkuiQuick::OutputMode
    \brief 输出模式类，表示显示器支持的一种分辨率与刷新率组合。

    OutputMode 由 Output 管理，不直接构造。

    \sa Output::modes(), Output::currentMode()
*/

OutputMode::OutputMode(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

OutputMode::~OutputMode()
{
    delete d;
}

/*!
  \brief 获取输出模式的ID
*/
int OutputMode::id() const
{
    return d->m_id;
}

/*!
  \brief 获取输出模式的大小
*/
QSize OutputMode::size() const
{
    return d->m_size;
}

/*!
   \brief 获取输出模式的刷新率
 */
int OutputMode::refreshRate() const
{
    return d->m_refreshRate;
}

/*!
   \brief 获取输出模式是否为最佳模式
 */
bool OutputMode::preferred() const
{
    return d->m_preferred;
}

/************** Output **************/
class Output::Private
{
public:
    explicit Private(Output *output);
    ~Private() = default;
    void setup(const WaylandClient::KywcOutput *backendOutput);
    void initializeModes();
    bool addMode(const WaylandClient::KywcOutputMode *backendMode);
    void removeMode(int id);
    void updateMode(OutputMode *mode, const WaylandClient::KywcOutputMode *backendMode);
    void syncCurrentMode(bool notifyChanges);
    static Capabilities convertCapabilities(uint32_t waylandFlags);

    Output *q {nullptr};
    QPointer<const WaylandClient::KywcOutput> m_backendOutput;
    QString m_uuid;
    QString m_name;
    QString m_make;
    QString m_model;
    QString m_serialNumber;
    QString m_description;
    QSize m_physicalSize;
    QPoint m_position;
    Capabilities m_capabilities;
    bool m_enabled = false;
    Output::Transform m_transform = Output::Transform::Normal;
    qreal m_scale = 1.0;
    uint32_t m_power = 0;
    uint32_t m_brightness = 0;
    uint32_t m_colorTemp = 0;
    QHash<int, OutputMode *> m_modes;
    QPointer<OutputMode> m_currentMode;
};

Output::Private::Private(Output *output)
    : q(output)
{
}

void Output::Private::setup(const WaylandClient::KywcOutput *backendOutput)
{
    m_backendOutput = backendOutput;
    m_uuid = backendOutput->uuid();
    m_name = backendOutput->name();
    m_make = backendOutput->make();
    m_model = backendOutput->model();
    m_serialNumber = backendOutput->serialNumber();
    m_description = backendOutput->description();
    m_physicalSize = backendOutput->physicalSize();
    m_position = backendOutput->position();
    m_capabilities = convertCapabilities(backendOutput->capabilities());
    m_enabled = backendOutput->isEnabled();
    m_transform = static_cast<Output::Transform>(backendOutput->transform());
    m_scale = backendOutput->scale();
    m_power = backendOutput->power();
    m_brightness = backendOutput->brightness();
    m_colorTemp = backendOutput->colorTemp();
    initializeModes();

    connect(m_backendOutput, &WaylandClient::KywcOutput::stateUpdated, q,
            [this](WaylandClient::KywcOutput::StateMasks masks) {

        if (!m_backendOutput) {
            return;
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Enabled)) {
            const bool enabled = m_backendOutput->isEnabled();
            if (enabled != m_enabled) {
                m_enabled = enabled;
                Q_EMIT q->enabledChanged(m_enabled);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::CurrentMode)) {
            syncCurrentMode(true);
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Position)) {
            const QPoint position = m_backendOutput->position();
            if (position != m_position) {
                m_position = position;
                Q_EMIT q->positionChanged(m_position);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Transform)) {
            const auto transform = static_cast<Output::Transform>(m_backendOutput->transform());
            if (transform != m_transform) {
                m_transform = transform;
                Q_EMIT q->transformChanged(m_transform);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Scale)) {
            const qreal scale = m_backendOutput->scale();
            if (scale != m_scale) {
                m_scale = scale;
                Q_EMIT q->scaleChanged(m_scale);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Power)) {
            const uint32_t power = m_backendOutput->power();
            if (power != m_power) {
                m_power = power;
                Q_EMIT q->powerChanged(m_power);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::Brightness)) {
            const uint32_t brightness = m_backendOutput->brightness();
            if (brightness != m_brightness) {
                m_brightness = brightness;
                Q_EMIT q->brightnessChanged(m_brightness);
            }
        }

        if (masks.testFlag(WaylandClient::KywcOutput::StateMask::ColorTemp)) {
            const uint32_t colorTemp = m_backendOutput->colorTemp();
            if (colorTemp != m_colorTemp) {
                m_colorTemp = colorTemp;
                Q_EMIT q->colorTempChanged(m_colorTemp);
            }
        }
    });

    connect(m_backendOutput, &WaylandClient::KywcOutput::modeAdded, q,
            [this](const WaylandClient::KywcOutputMode *backendMode) {
        if (addMode(backendMode)) {
            Q_EMIT q->modesChanged();
        }
    });
    connect(m_backendOutput, &WaylandClient::KywcOutput::modeChanged, q,
            [this](const WaylandClient::KywcOutputMode *backendMode) {
            if (auto* mode = m_modes.value(backendMode->id())) {
                updateMode(mode, backendMode);
                Q_EMIT q->modesChanged();
            }
    });
    connect(m_backendOutput, &WaylandClient::KywcOutput::modeRemoved, q,
            [this](int id) { removeMode(id); });
}

void Output::Private::initializeModes()
{
    if (!m_backendOutput) {
        return;
    }
    const auto backendModes = m_backendOutput->modes();
    m_modes.reserve(backendModes.size());
    for (const auto *backendMode : backendModes) {
        addMode(backendMode);
    }
    syncCurrentMode(false);
}

bool Output::Private::addMode(const WaylandClient::KywcOutputMode *backendMode)
{
    if (!backendMode) {
        return false;
    }

    const int id = backendMode->id();
    auto *mode = m_modes.value(id);

    const bool added = !mode;
    if (added) {
        mode = new OutputMode(q);
        mode->d->m_id = id;
        m_modes.insert(id, mode);
        updateMode(mode, backendMode);
    }
    return added;
}

void Output::Private::removeMode(int id)
{
    auto *mode = m_modes.take(id);
    if (!mode) {
        return;
    }

    if (mode == m_currentMode) {
        m_currentMode = nullptr;
    }
    mode->deleteLater();
    Q_EMIT q->modesChanged();
}

void Output::Private::updateMode(OutputMode *mode, const WaylandClient::KywcOutputMode *backendMode)
{
    const int id = backendMode->id();
    if (mode->d->m_id != id) {
        return;
    }

    mode->d->m_size = backendMode->size();
    mode->d->m_refreshRate = backendMode->refreshRate();
    mode->d->m_preferred = backendMode->isPreferred();
}

void Output::Private::syncCurrentMode(bool notifyChanges)
{
    if (!m_backendOutput) {
        return;
    }
    OutputMode *currentMode = nullptr;
    if (m_enabled) {
        if (const auto *backendCurrentMode = m_backendOutput->currentMode()) {
            currentMode = m_modes.value(backendCurrentMode->id());
            if (!currentMode) {
                qWarning() << "Output received a current mode before its mode-added signal"
                           << backendCurrentMode->id();
            }
        }
    }

    if (currentMode == m_currentMode) {
        return;
    }

    m_currentMode = currentMode;
    if (notifyChanges) {
        Q_EMIT q->currentModeChanged(m_currentMode);
    }
}

Output::Capabilities Output::Private::convertCapabilities(uint32_t flags)
{
    constexpr uint32_t supported =
        static_cast<uint32_t>(Capability::Power)
        | static_cast<uint32_t>(Capability::Brightness)
        | static_cast<uint32_t>(Capability::ColorTemperature);

    return Capabilities{static_cast<Capability>(flags & supported)};
}

/*!
    \class UkuiQuick::Output
    \brief 输出设备类，表示一个物理显示输出（如显示器、内置屏幕等）。

    Output 由 OutputManager 管理，不直接构造。

    \sa OutputManager
*/

/*!
    \enum Output::Capability
    \brief 输出设备支持的能力标志位。

    \value Power 电源管理（DPMS）。
    \value Brightness 亮度调节。
    \value ColorTemperature 色温调节。
*/

/*!
    \enum Output::Transform
    \brief 输出设备的旋转变换类型。

    \value Normal
           正常方向。
    \value Rotate90
           逆时针旋转 90°。
    \value Rotate180
           旋转 180°。
    \value Rotate270
           逆时针旋转 270°。
    \value Flipped
           水平翻转。
    \value Flipped90
           水平翻转后逆时针旋转 90°。
    \value Flipped180
           水平翻转后旋转 180°。
    \value Flipped270
           水平翻转后逆时针旋转 270°。
*/
Output::Output(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Output::~Output()
{
    delete d;
}

/*!
    \brief 输出设备的唯一标识符。
*/
QString Output::uuid() const
{
    return d->m_uuid;
}

/*!
    \brief 输出设备的名称。
*/
QString Output::name() const
{
    return d->m_name;
}

/*!
    \brief 输出设备的制造商。
*/
QString Output::make() const
{
    return d->m_make;
}

/*!
    \brief 输出设备的型号。
*/
QString Output::model() const
{
    return d->m_model;
}

/*!
    \brief 输出设备的序列号。
*/
QString Output::serialNumber() const
{
    return d->m_serialNumber;
}

/*!
    \brief 输出设备的描述。
*/
QString Output::description() const
{
    return d->m_description;
}

/*!
    \brief 输出设备的物理尺寸。
*/
QSize Output::physicalSize() const
{
    return d->m_physicalSize;
}

/*!
    \brief 输出设备的位置。
*/
QPoint Output::position() const
{
    return d->m_position;
}

/*!
    \brief 输出设备的 capabilities。
*/
Output::Capabilities Output::capabilities() const
{
    return d->m_capabilities;
}

/*!
    \brief 输出设备是否已启用。
*/
bool Output::isEnabled() const
{
    return d->m_enabled;
}

/*!
    \brief 输出设备的旋转角度。
*/
Output::Transform Output::transform() const
{
    return d->m_transform;
}

/*!
    \brief 输出设备的缩放比例。
*/
qreal Output::scale() const
{
    return d->m_scale;
}

/*!
    \brief 输出设备 power 状态。
*/
uint32_t Output::power() const
{
    return d->m_power;
}

/*!
    \brief 输出设备的亮度。
*/
uint32_t Output::brightness() const
{
    return d->m_brightness;
}

/*!
    \brief 输出设备的色温。
*/
uint32_t Output::colorTemp() const
{
    return d->m_colorTemp;
}

/*!
    \brief 输出设备的当前分辨率模式。
*/
OutputMode *Output::currentMode() const
{
    return d->m_currentMode;
}

/*!
    \brief 输出设备的所有分辨率模式。
*/
QList<OutputMode *> Output::modes() const
{
    return d->m_modes.values();
}

/************** OutputManager **************/
class OutputManager::Private
{
public:
    explicit Private(OutputManager *outputManager);
    ~Private();
    void bindBackend(quint32 name, quint32 version);
    void invalidate();
    void setInitialized(bool initialized);

    void initializeOutputs();
    void addOutput(const WaylandClient::KywcOutput *backendOutput);
    void removeOutput(const QString &uuid);
    void setPrimaryOutput(const WaylandClient::KywcOutput *backendOutput);
    Output *createOutput(const WaylandClient::KywcOutput *backendOutput);
    Output *findOutput(const QString &uuid) const;

    OutputManager *q {nullptr};
    QPointer<WaylandClient::Registry> m_registry;
    WaylandClient::KywcOutputManager *m_backend {nullptr};
    QList<Output *> m_outputs;
    QPointer<Output> m_primaryOutput;
    bool m_initialized = false;
};

OutputManager::Private::Private(OutputManager *outputManager)
    : q(outputManager)
{
    const auto *integration = WaylandIntegration::self();
    if (!integration) {
        return;
    }

    m_registry = integration->registry();
    if (!m_registry) {
        return;
    }

    connect(m_registry, &WaylandClient::Registry::kywcOutputManagerAnnounced, q, [this](quint32 name, quint32 version) {
        bindBackend(name, version);
    });

    connect(m_registry, &WaylandClient::Registry::kywcOutputManagerRemoved, q, [this](quint32) {
        invalidate();
    });

    if (!m_registry->hasInterface(WaylandClient::Registry::Interface::KywcOutputManager)) {
        return;
    }

    const auto interfaceInfo = m_registry->interface(WaylandClient::Registry::Interface::KywcOutputManager);
    if (interfaceInfo.name != 0) {
        bindBackend(interfaceInfo.name, interfaceInfo.version);
    }
}

OutputManager::Private::~Private()
{
    invalidate();
}

void OutputManager::Private::bindBackend(quint32 name, quint32 version)
{
    if (m_backend || !m_registry) {
        return;
    }

    auto *backend = m_registry->createKywcOutputManager(name, version, q);
    if (!backend) {
        qWarning() << "Failed to create KywcOutputManager";
        return;
    }

    if (!backend->isValid()) {
        backend->deleteLater();
        qWarning() << "Failed to create KywcOutputManager";
        return;
    }

    m_backend = backend;

    connect(backend, &WaylandClient::KywcOutputManager::outputCreated, q, [this](const WaylandClient::KywcOutput *backendOutput) {
        addOutput(backendOutput);
    });
    connect(backend, &WaylandClient::KywcOutputManager::outputRemoved, q, [this](const QString &uuid) {
        removeOutput(uuid);
    });
    connect(backend, &WaylandClient::KywcOutputManager::initialized, q, [this] {
        initializeOutputs();
        setInitialized(true);
    });
    connect(backend, &WaylandClient::KywcOutputManager::done, q, &OutputManager::configurationUpdated);

    connect(backend, &WaylandClient::KywcOutputManager::primaryOutputChanged, q,
            [this, backend](const WaylandClient::KywcOutput *backendOutput) {
        if (backend != m_backend) {
            return;
        }
        setPrimaryOutput(backendOutput);
    });
    connect(backend, &WaylandClient::KywcOutputManager::removed, q, [this] {
        invalidate();
    });

    Q_EMIT q->availableChanged(true);
}

void OutputManager::Private::initializeOutputs()
{
    if (!m_backend) {
        qWarning("OutputManager::initializeOutputs: backend is not initialized");
        return;
    }

    for (const auto *backendOutput : m_backend->outputs()) {
        addOutput(backendOutput);
    }
    setPrimaryOutput(m_backend->primaryOutput());
}

void OutputManager::Private::addOutput(const WaylandClient::KywcOutput *backendOutput)
{
    if (!backendOutput) {
        return;
    }

    const QString uuid = backendOutput->uuid();
    if (findOutput(uuid)) {
        return;
    }

    auto *output = createOutput(backendOutput);
    m_outputs.append(output);
    Q_EMIT q->outputAdded(output);
}

void OutputManager::Private::removeOutput(const QString &uuid)
{
    auto *output = findOutput(uuid);
    if (!output) {
        return;
    }
    m_outputs.removeOne(output);

    if (output == m_primaryOutput) {
        setPrimaryOutput(nullptr);
    }

    Q_EMIT q->outputRemoved(uuid);
    output->deleteLater();
}

void OutputManager::Private::setPrimaryOutput(const WaylandClient::KywcOutput *backendOutput)
{
    Output *primaryOutput = nullptr;
    if (backendOutput) {
        primaryOutput = findOutput(backendOutput->uuid());
    }

    if (primaryOutput != m_primaryOutput) {
        m_primaryOutput = primaryOutput;
        Q_EMIT q->primaryOutputChanged(primaryOutput);
    }
}

Output *OutputManager::Private::createOutput(const WaylandClient::KywcOutput *backendOutput)
{
    auto *output = new Output(q);
    output->d->setup(backendOutput);
    return output;
}

Output *OutputManager::Private::findOutput(const QString &uuid) const
{
    const auto it = std::find_if(m_outputs.cbegin(), m_outputs.cend(), [&uuid](const Output *output) {
        return output->uuid() == uuid;
    });
    return it == m_outputs.cend() ? nullptr : *it;
}

void OutputManager::Private::invalidate()
{
    m_primaryOutput = nullptr;
    const auto outputs = std::exchange(m_outputs, {});
    for ( auto *output : outputs) {
        output->deleteLater();
    }
    setInitialized(false);

    if (m_backend) {
        QObject::disconnect(m_backend, nullptr, q, nullptr);
        m_backend->deleteLater();
        m_backend = nullptr;
        Q_EMIT q->availableChanged(false);
    }
}

void OutputManager::Private::setInitialized(bool initialized)
{
    if (m_initialized == initialized) {
        return;
    }
    m_initialized = initialized;
    if (m_initialized) {
        Q_EMIT q->initialized();
    }
}

/*!
    \class UkuiQuick::OutputManager
    \brief 输出设备管理器，负责管理所有显示输出设备。

    在收到 availableChanged(false) 信号后停止使用 OutputManager及其管理的 Output 对象。

    \code
        auto *manager = new OutputManager(this);
        connect(manager, &OutputManager::initialized, this, [manager]() {
            for (auto *output : manager->outputs()) {
                qDebug() << output->name() << output->currentMode()->size();
            }
        });
    \endcode
    \sa Output, OutputMode
*/
OutputManager::OutputManager(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputManager::~OutputManager()
{
    delete d;
}

/*!
    \brief 请求不再使用输出设备管理器，调用后会收到 availableChanged(false) 信号。
*/
void OutputManager::stop() const
{
    if (d->m_backend && d->m_backend->isValid()) {
        d->m_backend->stop();
    }
}

/*!
    \brief 输出设备管理器是否可用，代表是否可以发送请求，如：使用stop()方法。
*/
bool OutputManager::isAvailable() const
{
    return d->m_backend;
}

/*!
    \brief 输出设备管理器是否已初始化, 返回 true 时, 调用 outputs() 方法才会返回已就绪的输出设备。
*/
bool OutputManager::isInitialized() const
{
    return d->m_initialized;
}

/*!
    \brief 返回所有已就绪的输出设备。
*/
QList<Output *> OutputManager::outputs() const
{
    return d->m_outputs;
}

/*!
    \brief 返回主输出设备。
*/
Output *OutputManager::primaryOutput() const
{
    return d->m_primaryOutput;
}

/*!
    \fn void OutputManager::outputAdded(Output *output)
    \brief 新的输出设备添加时发送的信号。
*/

/*!
    \fn void OutputManager::outputRemoved(const QString &uuid)
    \brief 输出设备移除时发送的信号。
*/

/*!
    \fn void OutputManager::primaryOutputChanged(Output *primaryOutput)
    \brief 主输出设备改变时发送的信号。
*/

/*!
    \fn void OutputManager::initialized()
    \brief 输出设备管理完成初始化时发送的信号。
*/

/*!
    \fn void OutputManager::availableChanged(bool available)
    \brief 输出设备管理可用状态改变时发送的信号。
*/

/*!
    \fn void OutputManager::configurationUpdated()
    \brief 输出配置一轮更新完成时发送的信号。
*/

/*!
    \fn void Output::modesChanged()
    \brief 输出设备的显示模式列表发生变化时发送的信号。
*/

/*!
    \fn void Output::positionChanged(const QPoint &position)
    \brief 输出设备在桌面坐标系中的位置发生变化时发送的信号。
*/

/*!
    \fn void Output::enabledChanged(bool enabled)
    \brief 输出设备的启用状态发生变化时发送的信号。
*/

/*!
    \fn void Output::transformChanged(Transform transform)
    \brief 输出设备的旋转变换发生变化时发送的信号。
*/

/*!
    \fn void Output::scaleChanged(qreal scale)
    \brief 输出设备的缩放比例发生变化时发送的信号。
*/

/*!
    \fn void Output::powerChanged(uint32_t power)
    \brief 输出设备的电源状态发生变化时发送的信号。
*/

/*!
    \fn void Output::brightnessChanged(uint32_t brightness)
    \brief 输出设备的亮度发生变化时发送的信号。
*/

/*!
    \fn void Output::colorTempChanged(uint32_t colorTemp)
    \brief 输出设备的色温发生变化时发送的信号。
*/

/*!
    \fn void Output::currentModeChanged(OutputMode *currentMode)
    \brief 输出设备的当前显示模式发生变化时发送的信号。
*/

}
