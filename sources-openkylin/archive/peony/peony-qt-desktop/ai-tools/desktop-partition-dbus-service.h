/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PEONY_DESKTOP_PARTITION_DBUS_SERVICE_H
#define PEONY_DESKTOP_PARTITION_DBUS_SERVICE_H

#include <QObject>

namespace PeonyDesktop {

/*!
 * DBus service: org.ukui.peony.desktop
 * Path: /org/ukui/peony/desktop/partition
 * Interface: org.ukui.peony.desktop.partition
 *
 * Tools: desktop.partition.*, desktop.widget.*, desktop.job.get
 */
class DesktopPartitionDbusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.peony.desktop.partition")

public:
    explicit DesktopPartitionDbusService(QObject *parent = nullptr);

    void registerService();

public Q_SLOTS:
    QString Invoke(const QString &tool,
                   const QString &requestId,
                   const QString &argsJson,
                   const QString &optionsJson);

    QString InvokeAsync(const QString &tool,
                        const QString &requestId,
                        const QString &argsJson,
                        const QString &optionsJson);

    QString GetJob(const QString &jobId);

    QString ListTools(const QString &prefix);

Q_SIGNALS:
    void JobFinished(const QString &jobId, const QString &responseJson);
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_PARTITION_DBUS_SERVICE_H
