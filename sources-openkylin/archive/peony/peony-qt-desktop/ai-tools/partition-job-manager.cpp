/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "partition-job-manager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>

namespace PeonyDesktop {

static PartitionJobManager *s_instance = nullptr;

PartitionJobManager *PartitionJobManager::instance()
{
    if (!s_instance) {
        s_instance = new PartitionJobManager(qApp);
    }
    return s_instance;
}

PartitionJobManager::PartitionJobManager(QObject *parent)
    : QObject(parent)
{
}

QString PartitionJobManager::nextJobId()
{
  return QStringLiteral("job-%1-%2")
      .arg(QDateTime::currentMSecsSinceEpoch())
      .arg(++m_jobCounter);
}

QString PartitionJobManager::submit(const QString &tool, const QString &requestId,
                                    const std::function<ToolResponse()> &work)
{
    const QString jobId = nextJobId();

    JobRecord record;
    record.jobId = jobId;
    record.tool = tool;
    record.requestId = requestId;
    record.status = QStringLiteral("pending");
    m_jobs.insert(jobId, record);

    QTimer::singleShot(0, this, [this, jobId, work]() {
        JobRecord running = m_jobs.value(jobId);
        running.status = QStringLiteral("running");
        m_jobs.insert(jobId, running);

        ToolResponse result = work();
        result.jobId = jobId;
        if (result.success && result.status == QStringLiteral("completed")) {
            result.status = QStringLiteral("completed");
        } else if (!result.success) {
            result.status = QStringLiteral("failed");
        }

        JobRecord finished;
        finished.jobId = jobId;
        finished.tool = running.tool;
        finished.requestId = running.requestId;
        finished.status = result.status;
        finished.result = result;
        m_jobs.insert(jobId, finished);

        Q_EMIT jobFinished(jobId, result.toJsonString());
    });

    return jobId;
}

PartitionJobManager::JobRecord PartitionJobManager::job(const QString &jobId) const
{
    return m_jobs.value(jobId);
}

bool PartitionJobManager::hasJob(const QString &jobId) const
{
    return m_jobs.contains(jobId);
}

} // namespace PeonyDesktop
