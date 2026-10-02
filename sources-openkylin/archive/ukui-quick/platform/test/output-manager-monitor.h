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

#ifndef UKUI_QUICK_OUTPUT_MANAGER_MONITOR_H
#define UKUI_QUICK_OUTPUT_MANAGER_MONITOR_H

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include "../output-manager.h"

class OutputManagerMonitor : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY statusChanged)
    Q_PROPERTY(bool initialized READ initialized NOTIFY statusChanged)
    Q_PROPERTY(QString primaryOutputUuid READ primaryOutputUuid NOTIFY statusChanged)
    Q_PROPERTY(int outputCount READ outputCount NOTIFY outputCountChanged)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY outputsChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap selectedOutput READ selectedOutput NOTIFY selectedOutputChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logTextChanged)
    Q_PROPERTY(QVariantList logEntries READ logEntries NOTIFY logEntriesChanged)

public:
    enum Role {
        UuidRole = Qt::UserRole + 1,
        NameRole,
        MakeRole,
        ModelRole,
        SerialNumberRole,
        DescriptionRole,
        PhysicalSizeRole,
        PositionRole,
        CapabilitiesRole,
        EnabledRole,
        TransformRole,
        TransformTextRole,
        ScaleRole,
        PowerRole,
        BrightnessRole,
        ColorTempRole,
        CurrentModeRole,
        ModeSummaryRole,
    };
    Q_ENUM(Role)

    explicit OutputManagerMonitor(UkuiQuick::OutputManager *manager, QObject *parent = nullptr);
    ~OutputManagerMonitor() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool available() const;
    bool initialized() const;
    QString primaryOutputUuid() const;
    int outputCount() const;
    QVariantList outputs() const;
    int selectedIndex() const;
    void setSelectedIndex(int index);
    QVariantMap selectedOutput() const;
    QString logText() const;
    QVariantList logEntries() const;

Q_SIGNALS:
    void statusChanged();
    void outputCountChanged();
    void outputsChanged();
    void selectedIndexChanged();
    void selectedOutputChanged();
    void logTextChanged();
    void logEntriesChanged();

private:
    struct PropertyChange {
        QString key;
        QString label;
        QVariant before;
        QVariant after;
    };

    struct PendingLogEntry {
        QString reason;
        QString outputUuid;
        QList<PropertyChange> changes;
    };

    void synchronizeOutputs();
    void addOutput(UkuiQuick::Output *output);
    void removeOutput(UkuiQuick::Output *output);
    void removeOutput(const QString &uuid);
    void clearOutputs();
    void connectOutput(UkuiQuick::Output *output);
    void disconnectOutput(UkuiQuick::Output *output);
    void updateOutput(UkuiQuick::Output *output, const QString &reason);
    QList<PropertyChange> changedProperties(const QVariantMap &before, const QVariantMap &after,
                                            const QString &reason) const;
    void scheduleLog(const QString &reason);
    void scheduleOutputUpdate(const QString &uuid, const QList<PropertyChange> &changes);
    void scheduleLogWrite();
    void writeLog();
    void normalizeSelectedIndex();

    QVariantMap outputProperties(const UkuiQuick::Output *output) const;
    QVariantMap logEntry(const PendingLogEntry &entry) const;
    QVariantMap eventLogEntry(const QString &reason) const;
    QString formatLogValue(const QVariant &value) const;
    QString transformText(UkuiQuick::Output::Transform transform) const;
    QString capabilitiesText(const UkuiQuick::Output *output) const;
    QString currentModeText(const UkuiQuick::Output *output) const;
    QStringList modeSummary(const UkuiQuick::Output *output) const;

    UkuiQuick::OutputManager *m_manager = nullptr;
    QList<UkuiQuick::Output *> m_outputs;
    QHash<UkuiQuick::Output *, QList<QMetaObject::Connection>> m_outputConnections;
    QHash<UkuiQuick::Output *, QVariantMap> m_outputSnapshots;
    QList<PendingLogEntry> m_pendingLogEntries;
    QStringList m_logLines;
    QVariantList m_logEntries;
    int m_selectedIndex = -1;
    quint64 m_updateSequence = 0;
    bool m_logScheduled = false;
};

#endif // UKUI_QUICK_OUTPUT_MANAGER_MONITOR_H
