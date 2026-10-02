/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PEONY_DESKTOP_PARTITION_JOB_MANAGER_H
#define PEONY_DESKTOP_PARTITION_JOB_MANAGER_H

#include "tool-response.h"

#include <QHash>
#include <QObject>
#include <functional>

namespace PeonyDesktop {

class PartitionJobManager : public QObject
{
    Q_OBJECT
public:
    struct JobRecord {
        QString jobId;
        QString tool;
        QString requestId;
        QString status;
        ToolResponse result;
    };

    static PartitionJobManager *instance();

    QString submit(const QString &tool, const QString &requestId,
                   const std::function<ToolResponse()> &work);

    JobRecord job(const QString &jobId) const;
    bool hasJob(const QString &jobId) const;

Q_SIGNALS:
    void jobFinished(const QString &jobId, const QString &responseJson);

private:
    explicit PartitionJobManager(QObject *parent = nullptr);

    QString nextJobId();

    QHash<QString, JobRecord> m_jobs;
    qint64 m_jobCounter = 0;
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_PARTITION_JOB_MANAGER_H
