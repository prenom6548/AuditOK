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
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "output-manager-monitor.h"

#include "../output-manager.h"

#include <QDebug>
#include <QTimer>

#include <algorithm>
#include <utility>

namespace
{

QString formatMode(const UkuiQuick::OutputMode *mode)
{
    if (!mode) {
        return QStringLiteral("none");
    }

    return QStringLiteral("id=%1, %2x%3 @ %4 mHz%5")
        .arg(mode->id())
        .arg(mode->size().width())
        .arg(mode->size().height())
        .arg(mode->refreshRate())
        .arg(mode->preferred() ? QStringLiteral(", preferred") : QString());
}

}

OutputManagerMonitor::OutputManagerMonitor(UkuiQuick::OutputManager *manager, QObject *parent)
    : QAbstractListModel(parent)
    , m_manager(manager)
{
    Q_ASSERT(m_manager);

    connect(m_manager, &UkuiQuick::OutputManager::outputAdded, this,
            [this](UkuiQuick::Output *output) {
                addOutput(output);
                scheduleLog(QStringLiteral("outputAdded(%1)").arg(output->uuid()));
            });
    connect(m_manager, &UkuiQuick::OutputManager::outputRemoved, this,
            [this](const QString &uuid) {
                removeOutput(uuid);
                scheduleLog(QStringLiteral("outputRemoved(%1)").arg(uuid));
            });
    connect(m_manager, &UkuiQuick::OutputManager::primaryOutputChanged, this,
            [this](UkuiQuick::Output *output) {
                Q_EMIT statusChanged();
                scheduleLog(QStringLiteral("primaryOutputChanged(%1)")
                                .arg(output ? output->uuid() : QStringLiteral("none")));
            });
    connect(m_manager, &UkuiQuick::OutputManager::initialized, this, [this] {
        synchronizeOutputs();
        Q_EMIT statusChanged();
        scheduleLog(QStringLiteral("initialized"));
    });
    connect(m_manager, &UkuiQuick::OutputManager::availableChanged, this, [this](bool available) {
        if (!available) {
            clearOutputs();
        } else {
            synchronizeOutputs();
        }
        Q_EMIT statusChanged();
        scheduleLog(QStringLiteral("availableChanged(%1)").arg(available));
    });

    synchronizeOutputs();
    scheduleLog(QStringLiteral("monitorStarted"));
}

OutputManagerMonitor::~OutputManagerMonitor()
{
    clearOutputs();
}

int OutputManagerMonitor::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_outputs.size();
}

QVariant OutputManagerMonitor::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_outputs.size()) {
        return {};
    }

    const auto *output = m_outputs.at(index.row());
    if (!output) {
        return {};
    }

    switch (role) {
    case UuidRole:
        return output->uuid();
    case NameRole:
        return output->name();
    case MakeRole:
        return output->make();
    case ModelRole:
        return output->model();
    case SerialNumberRole:
        return output->serialNumber();
    case DescriptionRole:
        return output->description();
    case PhysicalSizeRole:
        return QStringLiteral("%1 x %2 mm")
            .arg(output->physicalSize().width())
            .arg(output->physicalSize().height());
    case PositionRole:
        return QStringLiteral("(%1, %2)")
            .arg(output->position().x())
            .arg(output->position().y());
    case CapabilitiesRole:
        return capabilitiesText(output);
    case EnabledRole:
        return output->isEnabled();
    case TransformRole:
        return static_cast<int>(output->transform());
    case TransformTextRole:
        return transformText(output->transform());
    case ScaleRole:
        return output->scale();
    case PowerRole:
        return output->power();
    case BrightnessRole:
        return output->brightness();
    case ColorTempRole:
        return output->colorTemp();
    case CurrentModeRole:
        return currentModeText(output);
    case ModeSummaryRole:
        return modeSummary(output);
    default:
        return {};
    }
}

QHash<int, QByteArray> OutputManagerMonitor::roleNames() const
{
    return {
        {UuidRole, "uuid"},
        {NameRole, "name"},
        {MakeRole, "make"},
        {ModelRole, "model"},
        {SerialNumberRole, "serialNumber"},
        {DescriptionRole, "description"},
        {PhysicalSizeRole, "physicalSize"},
        {PositionRole, "position"},
        {CapabilitiesRole, "capabilities"},
        {EnabledRole, "enabled"},
        {TransformRole, "transform"},
        {TransformTextRole, "transformText"},
        {ScaleRole, "scale"},
        {PowerRole, "power"},
        {BrightnessRole, "brightness"},
        {ColorTempRole, "colorTemp"},
        {CurrentModeRole, "currentMode"},
        {ModeSummaryRole, "modeSummary"},
    };
}

bool OutputManagerMonitor::available() const
{
    return m_manager && m_manager->isAvailable();
}

bool OutputManagerMonitor::initialized() const
{
    return m_manager && m_manager->isInitialized();
}

QString OutputManagerMonitor::primaryOutputUuid() const
{
    if (!m_manager || !m_manager->primaryOutput()) {
        return QStringLiteral("none");
    }
    return m_manager->primaryOutput()->uuid();
}

int OutputManagerMonitor::outputCount() const
{
    return m_outputs.size();
}

QVariantList OutputManagerMonitor::outputs() const
{
    QVariantList result;
    result.reserve(m_outputs.size());
    for (const auto *output : m_outputs) {
        result.append(outputProperties(output));
    }
    return result;
}

int OutputManagerMonitor::selectedIndex() const
{
    return m_selectedIndex;
}

void OutputManagerMonitor::setSelectedIndex(int index)
{
    if (index < 0 || index >= m_outputs.size()) {
        index = -1;
    }
    if (m_selectedIndex == index) {
        return;
    }

    m_selectedIndex = index;
    Q_EMIT selectedIndexChanged();
    Q_EMIT selectedOutputChanged();
}

QVariantMap OutputManagerMonitor::selectedOutput() const
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_outputs.size()) {
        return {};
    }
    return outputProperties(m_outputs.at(m_selectedIndex));
}

QString OutputManagerMonitor::logText() const
{
    return m_logLines.join(QLatin1Char('\n'));
}

QVariantList OutputManagerMonitor::logEntries() const
{
    return m_logEntries;
}

void OutputManagerMonitor::synchronizeOutputs()
{
    clearOutputs();
    if (!m_manager) {
        return;
    }

    const auto outputs = m_manager->outputs();
    if (outputs.isEmpty()) {
        return;
    }

    beginInsertRows(QModelIndex(), 0, outputs.size() - 1);
    m_outputs = outputs;
    endInsertRows();
    for (auto *output : m_outputs) {
        connectOutput(output);
    }
    normalizeSelectedIndex();
    Q_EMIT outputCountChanged();
    Q_EMIT outputsChanged();
}

void OutputManagerMonitor::addOutput(UkuiQuick::Output *output)
{
    if (!output || m_outputs.contains(output)) {
        return;
    }

    const int row = m_outputs.size();
    beginInsertRows(QModelIndex(), row, row);
    m_outputs.append(output);
    endInsertRows();
    connectOutput(output);
    normalizeSelectedIndex();
    Q_EMIT outputCountChanged();
    Q_EMIT outputsChanged();
}

void OutputManagerMonitor::removeOutput(UkuiQuick::Output *output)
{
    const int row = m_outputs.indexOf(output);
    if (row < 0) {
        return;
    }

    const int previousSelectedIndex = m_selectedIndex;
    beginRemoveRows(QModelIndex(), row, row);
    m_outputs.removeAt(row);
    endRemoveRows();
    disconnectOutput(output);
    m_outputSnapshots.remove(output);
    int nextSelectedIndex = previousSelectedIndex;
    if (previousSelectedIndex > row) {
        nextSelectedIndex = previousSelectedIndex - 1;
    } else if (previousSelectedIndex == row) {
        nextSelectedIndex = qMin(row, m_outputs.size() - 1);
    }
    setSelectedIndex(nextSelectedIndex);
    normalizeSelectedIndex();
    Q_EMIT outputCountChanged();
    Q_EMIT outputsChanged();
}

void OutputManagerMonitor::removeOutput(const QString &uuid)
{
    for (auto *output : std::as_const(m_outputs)) {
        if (output && output->uuid() == uuid) {
            removeOutput(output);
            return;
        }
    }
}

void OutputManagerMonitor::clearOutputs()
{
    if (m_outputs.isEmpty()) {
        return;
    }

    beginResetModel();
    for (auto *output : std::as_const(m_outputs)) {
        disconnectOutput(output);
    }
    m_outputs.clear();
    m_outputSnapshots.clear();
    endResetModel();
    setSelectedIndex(-1);
    Q_EMIT outputCountChanged();
    Q_EMIT outputsChanged();
}

void OutputManagerMonitor::connectOutput(UkuiQuick::Output *output)
{
    m_outputSnapshots.insert(output, outputProperties(output));
    auto &connections = m_outputConnections[output];
    const auto update = [this, output](const QString &reason) {
        updateOutput(output, reason);
    };

    connections << connect(output, &UkuiQuick::Output::modesChanged, this,
                           [update] { update(QStringLiteral("modesChanged")); });
    connections << connect(output, &UkuiQuick::Output::positionChanged, this,
                           [update](const QPoint &) { update(QStringLiteral("positionChanged")); });
    connections << connect(output, &UkuiQuick::Output::enabledChanged, this,
                           [update](bool) { update(QStringLiteral("enabledChanged")); });
    connections << connect(output, &UkuiQuick::Output::transformChanged, this,
                           [update](UkuiQuick::Output::Transform) { update(QStringLiteral("transformChanged")); });
    connections << connect(output, &UkuiQuick::Output::scaleChanged, this,
                           [update](qreal) { update(QStringLiteral("scaleChanged")); });
    connections << connect(output, &UkuiQuick::Output::powerChanged, this,
                           [update](uint32_t) { update(QStringLiteral("powerChanged")); });
    connections << connect(output, &UkuiQuick::Output::brightnessChanged, this,
                           [update](uint32_t) { update(QStringLiteral("brightnessChanged")); });
    connections << connect(output, &UkuiQuick::Output::colorTempChanged, this,
                           [update](uint32_t) { update(QStringLiteral("colorTempChanged")); });
    connections << connect(output, &UkuiQuick::Output::currentModeChanged, this,
                           [update](UkuiQuick::OutputMode *) { update(QStringLiteral("currentModeChanged")); });
    connections << connect(output, &QObject::destroyed, this, [this, output] {
        removeOutput(output);
        scheduleLog(QStringLiteral("outputDestroyed"));
    });
}

void OutputManagerMonitor::disconnectOutput(UkuiQuick::Output *output)
{
    const auto connections = m_outputConnections.take(output);
    for (const auto &connection : connections) {
        disconnect(connection);
    }
}

void OutputManagerMonitor::updateOutput(UkuiQuick::Output *output, const QString &reason)
{
    const int row = m_outputs.indexOf(output);
    if (row < 0) {
        return;
    }

    const QVariantMap before = m_outputSnapshots.value(output);
    const QVariantMap after = outputProperties(output);
    const auto changes = changedProperties(before, after, reason);
    QVariantMap updatedSnapshot = before;
    for (const auto &change : changes) {
        updatedSnapshot.insert(change.key, change.after);
    }
    m_outputSnapshots.insert(output, updatedSnapshot);

    const QModelIndex changed = index(row, 0);
    Q_EMIT dataChanged(changed, changed);
    if (row == m_selectedIndex) {
        Q_EMIT selectedOutputChanged();
    }
    Q_EMIT outputsChanged();
    scheduleOutputUpdate(output->uuid(), changes);
}

QList<OutputManagerMonitor::PropertyChange> OutputManagerMonitor::changedProperties(
    const QVariantMap &before, const QVariantMap &after, const QString &reason) const
{
    if (before.isEmpty()) {
        return {};
    }

    QList<PropertyChange> changes;
    const auto append = [&before, &after, &changes](const QString &key, const QString &label) {
        const QVariant beforeValue = before.value(key);
        const QVariant afterValue = after.value(key);
        if (beforeValue != afterValue) {
            changes.append({key, label, beforeValue, afterValue});
        }
    };

    if (reason == QStringLiteral("modesChanged")) {
        append(QStringLiteral("currentMode"), QStringLiteral("当前模式"));
        append(QStringLiteral("modeSummary"), QStringLiteral("支持模式"));
    } else if (reason == QStringLiteral("positionChanged")) {
        append(QStringLiteral("position"), QStringLiteral("位置"));
    } else if (reason == QStringLiteral("enabledChanged")) {
        append(QStringLiteral("enabled"), QStringLiteral("启用状态"));
        append(QStringLiteral("currentMode"), QStringLiteral("当前模式"));
    } else if (reason == QStringLiteral("transformChanged")) {
        append(QStringLiteral("transformText"), QStringLiteral("旋转"));
    } else if (reason == QStringLiteral("scaleChanged")) {
        append(QStringLiteral("scale"), QStringLiteral("缩放"));
    } else if (reason == QStringLiteral("powerChanged")) {
        append(QStringLiteral("power"), QStringLiteral("电源"));
    } else if (reason == QStringLiteral("brightnessChanged")) {
        append(QStringLiteral("brightness"), QStringLiteral("亮度"));
    } else if (reason == QStringLiteral("colorTempChanged")) {
        append(QStringLiteral("colorTemp"), QStringLiteral("色温"));
    } else if (reason == QStringLiteral("currentModeChanged")) {
        append(QStringLiteral("currentMode"), QStringLiteral("当前模式"));
    }

    return changes;
}

void OutputManagerMonitor::scheduleLog(const QString &reason)
{
    const auto exists = std::any_of(m_pendingLogEntries.cbegin(), m_pendingLogEntries.cend(),
                                    [&reason](const PendingLogEntry &entry) {
                                        return entry.reason == reason && entry.outputUuid.isEmpty();
                                    });
    if (!exists) {
        m_pendingLogEntries.append({reason, {}, {}});
    }
    scheduleLogWrite();
}

void OutputManagerMonitor::scheduleOutputUpdate(const QString &uuid, const QList<PropertyChange> &changes)
{
    if (changes.isEmpty()) {
        return;
    }

    auto entry = std::find_if(m_pendingLogEntries.begin(), m_pendingLogEntries.end(),
                              [&uuid](const PendingLogEntry &candidate) {
                                  return candidate.outputUuid == uuid;
                              });
    if (entry == m_pendingLogEntries.end()) {
        m_pendingLogEntries.append({{}, uuid, changes});
    } else {
        for (const auto &change : changes) {
            const auto existingChange = std::find_if(entry->changes.begin(), entry->changes.end(),
                                                     [&change](const PropertyChange &candidate) {
                                                         return candidate.key == change.key;
                                                     });
            if (existingChange == entry->changes.end()) {
                entry->changes.append(change);
            } else {
                existingChange->after = change.after;
            }
        }
    }
    scheduleLogWrite();
}

void OutputManagerMonitor::scheduleLogWrite()
{
    if (m_logScheduled) {
        return;
    }

    m_logScheduled = true;
    QTimer::singleShot(0, this, &OutputManagerMonitor::writeLog);
}

void OutputManagerMonitor::writeLog()
{
    m_logScheduled = false;
    const auto pendingEntries = std::exchange(m_pendingLogEntries, {});

    QStringList lines;
    QVariantList entries;
    const QString header = QStringLiteral("──────── 第 %1 轮信号更新 ────────")
                               .arg(++m_updateSequence);
    lines << header;
    entries.append(QVariantMap {
        {QStringLiteral("text"), header},
        {QStringLiteral("color"), QStringLiteral("#111827")},
        {QStringLiteral("header"), true},
    });

    for (const auto &pendingEntry : pendingEntries) {
        const QVariantMap entry = logEntry(pendingEntry);
        entries.append(entry);
        lines.append(entry.value(QStringLiteral("text")).toString());
    }

    for (const auto &line : std::as_const(lines)) {
        qInfo().noquote() << line;
    }

    constexpr int maximumLogLines = 500;
    m_logLines += lines;
    if (m_logLines.size() > maximumLogLines) {
        m_logLines = m_logLines.mid(m_logLines.size() - maximumLogLines);
    }
    m_logEntries += entries;
    if (m_logEntries.size() > maximumLogLines) {
        m_logEntries = m_logEntries.mid(m_logEntries.size() - maximumLogLines);
    }
    Q_EMIT logTextChanged();
    Q_EMIT logEntriesChanged();
}

void OutputManagerMonitor::normalizeSelectedIndex()
{
    const int expectedIndex = m_outputs.isEmpty() ? -1 : qBound(0, m_selectedIndex, m_outputs.size() - 1);
    if (m_selectedIndex == expectedIndex) {
        Q_EMIT selectedOutputChanged();
        return;
    }

    m_selectedIndex = expectedIndex;
    Q_EMIT selectedIndexChanged();
    Q_EMIT selectedOutputChanged();
}

QVariantMap OutputManagerMonitor::outputProperties(const UkuiQuick::Output *output) const
{
    if (!output) {
        return {};
    }

    return {
        {QStringLiteral("uuid"), output->uuid()},
        {QStringLiteral("name"), output->name()},
        {QStringLiteral("make"), output->make()},
        {QStringLiteral("model"), output->model()},
        {QStringLiteral("serialNumber"), output->serialNumber()},
        {QStringLiteral("description"), output->description()},
        {QStringLiteral("physicalSize"), QStringLiteral("%1 x %2 mm")
                                             .arg(output->physicalSize().width())
                                             .arg(output->physicalSize().height())},
        {QStringLiteral("position"), QStringLiteral("(%1, %2)")
                                          .arg(output->position().x())
                                          .arg(output->position().y())},
        {QStringLiteral("capabilities"), capabilitiesText(output)},
        {QStringLiteral("enabled"), output->isEnabled()},
        {QStringLiteral("transform"), static_cast<int>(output->transform())},
        {QStringLiteral("transformText"), transformText(output->transform())},
        {QStringLiteral("scale"), output->scale()},
        {QStringLiteral("power"), output->power()},
        {QStringLiteral("brightness"), output->brightness()},
        {QStringLiteral("colorTemp"), output->colorTemp()},
        {QStringLiteral("currentMode"), currentModeText(output)},
        {QStringLiteral("modeSummary"), modeSummary(output)},
    };
}

QVariantMap OutputManagerMonitor::logEntry(const PendingLogEntry &entry) const
{
    if (!entry.outputUuid.isEmpty()) {
        QStringList lines;
        for (const auto &change : entry.changes) {
            lines.append(QStringLiteral("%1：%2 -> %3")
                             .arg(change.label, formatLogValue(change.before), formatLogValue(change.after)));
        }
        return {
            {QStringLiteral("text"), QStringLiteral("更新 %1：\n%2")
                                         .arg(entry.outputUuid, lines.join(QLatin1Char('\n')))},
            {QStringLiteral("color"), QStringLiteral("#1d4ed8")},
            {QStringLiteral("header"), false},
        };
    }

    return eventLogEntry(entry.reason);
}

QVariantMap OutputManagerMonitor::eventLogEntry(const QString &reason) const
{
    const auto argument = [&reason](const QString &prefix) {
        return reason.mid(prefix.size(), reason.size() - prefix.size() - 1);
    };
    if (reason.startsWith(QStringLiteral("outputAdded("))) {
        return {
            {QStringLiteral("text"), QStringLiteral("新增 Output：%1")
                                         .arg(argument(QStringLiteral("outputAdded(")))},
            {QStringLiteral("color"), QStringLiteral("#16794c")},
            {QStringLiteral("header"), false},
        };
    }
    if (reason.startsWith(QStringLiteral("outputRemoved("))) {
        return {
            {QStringLiteral("text"), QStringLiteral("删除 Output：%1")
                                         .arg(argument(QStringLiteral("outputRemoved(")))},
            {QStringLiteral("color"), QStringLiteral("#b42318")},
            {QStringLiteral("header"), false},
        };
    }
    if (reason.startsWith(QStringLiteral("availableChanged("))
        || reason == QStringLiteral("initialized")
        || reason.startsWith(QStringLiteral("primaryOutputChanged("))) {
        return {
            {QStringLiteral("text"), QStringLiteral("Manager 状态：%1").arg(reason)},
            {QStringLiteral("color"), QStringLiteral("#9a6700")},
            {QStringLiteral("header"), false},
        };
    }
    return {
        {QStringLiteral("text"), QStringLiteral("事件：%1").arg(reason)},
        {QStringLiteral("color"), QStringLiteral("#4b5563")},
        {QStringLiteral("header"), false},
    };
}

QString OutputManagerMonitor::formatLogValue(const QVariant &value) const
{
    if (!value.isValid()) {
        return QStringLiteral("none");
    }
    if (value.userType() == QMetaType::Bool) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    if (value.userType() == QMetaType::QStringList) {
        const QStringList values = value.toStringList();
        return values.isEmpty() ? QStringLiteral("none") : values.join(QStringLiteral(", "));
    }
    return value.toString();
}

QString OutputManagerMonitor::transformText(UkuiQuick::Output::Transform transform) const
{
    switch (transform) {
    case UkuiQuick::Output::Transform::Normal:
        return QStringLiteral("旋转 0°");
    case UkuiQuick::Output::Transform::Rotate90:
        return QStringLiteral("逆时针 90°");
    case UkuiQuick::Output::Transform::Rotate180:
        return QStringLiteral("翻转（180°）");
    case UkuiQuick::Output::Transform::Rotate270:
        return QStringLiteral("顺时针 90°");
    case UkuiQuick::Output::Transform::Flipped:
        return QStringLiteral("水平翻转");
    case UkuiQuick::Output::Transform::Flipped90:
        return QStringLiteral("水平翻转后逆时针 90°");
    case UkuiQuick::Output::Transform::Flipped180:
        return QStringLiteral("水平翻转后旋转 180°");
    case UkuiQuick::Output::Transform::Flipped270:
        return QStringLiteral("水平翻转后顺时针 90°");
    default:
        return QStringLiteral("未知（%1）").arg(static_cast<int>(transform));
    }
}

QString OutputManagerMonitor::capabilitiesText(const UkuiQuick::Output *output) const
{
    QStringList capabilities;
    if (output->capabilities().testFlag(UkuiQuick::Output::Capability::Power)) {
        capabilities.append(QStringLiteral("Power"));
    }
    if (output->capabilities().testFlag(UkuiQuick::Output::Capability::Brightness)) {
        capabilities.append(QStringLiteral("Brightness"));
    }
    if (output->capabilities().testFlag(UkuiQuick::Output::Capability::ColorTemperature)) {
        capabilities.append(QStringLiteral("ColorTemperature"));
    }
    return capabilities.isEmpty() ? QStringLiteral("none") : capabilities.join(QStringLiteral(", "));
}

QString OutputManagerMonitor::currentModeText(const UkuiQuick::Output *output) const
{
    return formatMode(output->currentMode());
}

QStringList OutputManagerMonitor::modeSummary(const UkuiQuick::Output *output) const
{
    QStringList modes;
    for (const auto *mode : output->modes()) {
        modes.append(formatMode(mode));
    }
    return modes;
}
