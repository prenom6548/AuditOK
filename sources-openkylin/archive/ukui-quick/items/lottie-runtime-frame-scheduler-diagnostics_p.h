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
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_DIAGNOSTICS_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_DIAGNOSTICS_P_H

#include "lottie-runtime-diagnostics_p.h"
#include "lottie-runtime-frame-scheduler_p.h"

#include <QList>

namespace UkuiQuick {

template<bool Enabled>
class LottieRuntimeFrameSchedulerDiagnostics;

template<>
class LottieRuntimeFrameSchedulerDiagnostics<false>
{
public:
    using TimePoint = LottieRuntimeDiagnosticsClock::TimePoint;

    void recordCacheHit() {}
    void recordSubscriberlessQueuedJobRemoval() {}
    void recordDroppedResults(int) {}
    void recordGuiBatchScheduled(int) {}
    void recordSupersededRequest() {}
    void recordRenderKey(const LottieRuntimeRenderKey &) {}
    void recordCoalescedRequest() {}
    void recordQueuedKeys(int) {}
    void recordRunningJobs(int) {}
    void recordQueueWait(const TimePoint &, const TimePoint &) {}
    void recordRasterJob() {}
    void recordEndToEnd(const TimePoint &, const TimePoint &) {}
    void recordRasterExecution(qint64) {}
    void recordImmutableFrame() {}
    void recordCompletionDelivery() {}
    void recordGuiBatchDelivery() {}
    void reset() {}
};

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
template<>
class LottieRuntimeFrameSchedulerDiagnostics<true>
{
public:
    using Clock = LottieRuntimeDiagnosticsClock;
    using TimePoint = Clock::TimePoint;

    void recordCacheHit()
    {
        ++m_cacheHitCount;
    }

    void recordSubscriberlessQueuedJobRemoval()
    {
        ++m_subscriberlessQueuedJobRemovalCount;
    }

    void recordDroppedResults(int count)
    {
        m_droppedResultCount += count;
    }

    void recordGuiBatchScheduled(int pendingGuiBatches)
    {
        m_guiBatchScheduledAt = Clock::now();
        ++m_scheduledGuiBatchCount;
        m_maxPendingGuiBatches = qMax(m_maxPendingGuiBatches, pendingGuiBatches);
    }

    void recordSupersededRequest()
    {
        ++m_supersededRequestCount;
    }

    void recordRenderKey(const LottieRuntimeRenderKey &key)
    {
        if (m_observedRenderKeys.size() < kMaximumObservedRenderKeys &&
            !m_observedRenderKeys.contains(key)) {
            m_observedRenderKeys.append(key);
        }
    }

    void recordCoalescedRequest()
    {
        ++m_coalescedRequestCount;
    }

    void recordQueuedKeys(int queuedKeys)
    {
        m_maxQueuedKeys = qMax(m_maxQueuedKeys, queuedKeys);
    }

    void recordRunningJobs(int runningJobs)
    {
        m_maxRunningJobs = qMax(m_maxRunningJobs, runningJobs);
    }

    void recordQueueWait(const TimePoint &enqueuedAt, const TimePoint &dispatchedAt)
    {
        if (Clock::isValid(enqueuedAt)) {
            m_queueWait.record(Clock::elapsedNanoseconds(enqueuedAt, dispatchedAt));
        }
    }

    void recordRasterJob()
    {
        ++m_rasterJobCount;
    }

    void recordEndToEnd(const TimePoint &requestedAt, const TimePoint &completedAt)
    {
        m_endToEnd.record(Clock::elapsedNanoseconds(requestedAt, completedAt));
    }

    void recordRasterExecution(qint64 nanoseconds)
    {
        if (nanoseconds > 0) {
            m_rasterExecution.record(nanoseconds);
        }
    }

    void recordImmutableFrame()
    {
        ++m_immutableFrameCount;
    }

    void recordCompletionDelivery()
    {
        ++m_completionDeliveryCount;
    }

    void recordGuiBatchDelivery()
    {
        if (!Clock::isValid(m_guiBatchScheduledAt)) {
            return;
        }
        m_guiBatchDelivery.record(
            Clock::elapsedNanoseconds(m_guiBatchScheduledAt, Clock::now()));
        m_guiBatchScheduledAt = {};
    }

    void populate(LottieRuntimeFrameSchedulerStats *stats) const
    {
        stats->maxQueuedKeys = m_maxQueuedKeys;
        stats->maxRunningJobs = m_maxRunningJobs;
        stats->maxPendingGuiBatches = m_maxPendingGuiBatches;
        stats->rasterJobCount = m_rasterJobCount;
        stats->cacheHitCount = m_cacheHitCount;
        stats->scheduledGuiBatchCount = m_scheduledGuiBatchCount;
        stats->completionDeliveryCount = m_completionDeliveryCount;
        stats->coalescedRequestCount = m_coalescedRequestCount;
        stats->supersededRequestCount = m_supersededRequestCount;
        stats->droppedResultCount = m_droppedResultCount;
        stats->subscriberlessQueuedJobRemovalCount =
            m_subscriberlessQueuedJobRemovalCount;
        stats->immutableFrameCount = m_immutableFrameCount;
        stats->distinctRenderKeyCount = m_observedRenderKeys.size();
        stats->queueWaitP95Nanoseconds = m_queueWait.percentile95();
        stats->rasterExecutionP95Nanoseconds = m_rasterExecution.percentile95();
        stats->endToEndP95Nanoseconds = m_endToEnd.percentile95();
        stats->guiBatchDeliveryP95Nanoseconds = m_guiBatchDelivery.percentile95();
    }

    void reset()
    {
        m_maxQueuedKeys = 0;
        m_maxRunningJobs = 0;
        m_maxPendingGuiBatches = 0;
        m_rasterJobCount = 0;
        m_cacheHitCount = 0;
        m_scheduledGuiBatchCount = 0;
        m_completionDeliveryCount = 0;
        m_coalescedRequestCount = 0;
        m_supersededRequestCount = 0;
        m_droppedResultCount = 0;
        m_subscriberlessQueuedJobRemovalCount = 0;
        m_immutableFrameCount = 0;
        m_queueWait.clear();
        m_rasterExecution.clear();
        m_endToEnd.clear();
        m_guiBatchDelivery.clear();
        m_guiBatchScheduledAt = {};
        m_observedRenderKeys.clear();
    }

private:
    static constexpr int kMaximumObservedRenderKeys = 4096;

    int m_maxQueuedKeys = 0;
    int m_maxRunningJobs = 0;
    int m_maxPendingGuiBatches = 0;
    int m_rasterJobCount = 0;
    int m_cacheHitCount = 0;
    int m_scheduledGuiBatchCount = 0;
    int m_completionDeliveryCount = 0;
    int m_coalescedRequestCount = 0;
    int m_supersededRequestCount = 0;
    int m_droppedResultCount = 0;
    int m_subscriberlessQueuedJobRemovalCount = 0;
    int m_immutableFrameCount = 0;
    LottieRuntimeTimingSamples m_queueWait;
    LottieRuntimeTimingSamples m_rasterExecution;
    LottieRuntimeTimingSamples m_endToEnd;
    LottieRuntimeTimingSamples m_guiBatchDelivery;
    TimePoint m_guiBatchScheduledAt;
    QList<LottieRuntimeRenderKey> m_observedRenderKeys;
};

using LottieRuntimeFrameSchedulerDiagnosticsType =
    LottieRuntimeFrameSchedulerDiagnostics<true>;
#else
using LottieRuntimeFrameSchedulerDiagnosticsType =
    LottieRuntimeFrameSchedulerDiagnostics<false>;
#endif

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_DIAGNOSTICS_P_H
