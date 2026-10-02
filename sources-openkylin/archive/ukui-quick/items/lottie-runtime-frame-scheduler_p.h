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
 *
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_P_H

#include <QObject>
#include <QtGlobal>

#include "lottie-runtime-raster-worker_p.h"

namespace UkuiQuick {

/**
 * @brief 一帧渲染结果的唯一标识。
 *
 * 只有资产版本、实际主题、有效帧号和输出像素尺寸会影响渲染结果。资产版本
 * 已包含动画和它绑定的图片快照，不需要把路径等细节再展开到这个 key 中。
 * Icon/LottieItem 和窗口不在标识中，因此不同组件可以共用完全相同的 QImage。
 */
struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeRenderKey
{
    LottieAssetRevision assetRevision;
    QString effectiveTheme;
    int frame = 0;
    QSize physicalPixelSize;

    bool operator==(const LottieRuntimeRenderKey &other) const
    {
        return assetRevision == other.assetRevision &&
            effectiveTheme == other.effectiveTheme &&
            frame == other.frame &&
            physicalPixelSize == other.physicalPixelSize;
    }
};

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeFrameSchedulerStats
{
    int queuedKeys = 0;
    int deferredSubscriptions = 0;
    int runningJobs = 0;
    int rasterWorkerCount = 0;
    int maxQueuedKeys = 0;
    int maxRunningJobs = 0;
    int maxPendingGuiBatches = 0;
    int pendingGuiBatches = 0;
    int rasterJobCount = 0;
    int cacheHitCount = 0;
    int cacheEntryCount = 0;
    qint64 cacheBytes = 0;
    int scheduledGuiBatchCount = 0;
    int completionDeliveryCount = 0;
    int coalescedRequestCount = 0;
    int supersededRequestCount = 0;
    int droppedResultCount = 0;
    int subscriberlessQueuedJobRemovalCount = 0;
    int immutableFrameCount = 0;
    int distinctRenderKeyCount = 0;
    qint64 queueWaitP95Nanoseconds = 0;
    qint64 rasterExecutionP95Nanoseconds = 0;
    qint64 endToEndP95Nanoseconds = 0;
    qint64 guiBatchDeliveryP95Nanoseconds = 0;
};
#endif

/**
 * @brief 为 Icon 和 LottieItem 共用的“只保留最新请求”调度器和 QImage 缓存。
 *
 * 调度器运行在应用线程，是唯一可以向栅格线程提交任务的对象。等待任务按首次
 * 准入顺序公平派发；同一组件随后只替换请求内容，不会借“最新帧”反复插队。
 * 后台渲染结果和缓存命中也都从这里统一交给组件，避免形成多条交付路径。
 */
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeFrameScheduler final : public QObject
{
public:
    using Completion = LottieRuntimeRasterWorker::Completion;

    explicit LottieRuntimeFrameScheduler(int maximumRasterSessions,
                                         QObject *parent = nullptr);
    ~LottieRuntimeFrameScheduler() override;

    bool request(quint64 subscriptionId,
                 const LottieRuntimeFrameRequest &request,
                 Completion completion);
    void suspend(quint64 subscriptionId);
    void unsubscribe(quint64 subscriptionId);
    void stop();

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    // 以下入口只供内部测试使用，不是 QML 或安装后的诊断接口。
    LottieRuntimeFrameSchedulerStats testStats() const;
    void resetDiagnostics();
    // 仅在没有运行、排队或待交付任务时清空帧缓存和渲染会话。
    bool testResetCacheAndSession();
    void testHoldGuiBatch(bool hold);
    void testFlushGuiBatch();
#endif

private:
    class Private;
    std::unique_ptr<Private> d;

    void dispatchNext();
    void completeWorkerJob(quint64 jobId, const LottieRuntimeFrameResult &result);
    void flushGuiBatch();
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_FRAME_SCHEDULER_P_H
