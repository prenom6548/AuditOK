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

#include "lottie-runtime-frame-scheduler_p.h"

#include "lottie-renderer.h"
#include "lottie-runtime-frame-scheduler-diagnostics_p.h"

#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QtMath>

#include <limits>
#include <list>
#include <optional>
#include <utility>

namespace UkuiQuick {

namespace {

constexpr int kMaximumQueuedKeys = 64;
// 16 MiB 只计算缓存 QImage 的像素内存；512 限制 tiny-frame 产生的缓存
// 索引数量。帧缓存不会保留完整资产或图片资源快照。对于常见的 20–25 帧
// 小动画，512 条记录可以覆盖最多约 16 个并发动画，同时仍能防止 1×1 图片
// 制造无限记录。
constexpr int kMaximumFrameCacheEntries = 512;
constexpr qint64 kFrameCacheBudgetBytes = 16LL * 1024LL * 1024LL;

using RenderKey = LottieRuntimeRenderKey;
using SchedulerClock = LottieRuntimeDiagnosticsClock;
using SchedulerTimePoint = SchedulerClock::TimePoint;

struct CacheLookupKey
{
    quint64 revisionId = 0;
    QString effectiveTheme;
    int frame = 0;
    QSize physicalPixelSize;

    bool operator==(const CacheLookupKey &other) const
    {
        return revisionId == other.revisionId &&
            effectiveTheme == other.effectiveTheme &&
            frame == other.frame &&
            physicalPixelSize == other.physicalPixelSize;
    }
};

uint qHash(const CacheLookupKey &key, uint seed = 0)
{
    seed = ::qHash(key.revisionId, seed);
    seed = ::qHash(key.effectiveTheme, seed);
    seed = ::qHash(key.frame, seed);
    seed = ::qHash(key.physicalPixelSize.width(), seed);
    return ::qHash(key.physicalPixelSize.height(), seed);
}

RenderKey renderKeyFor(const LottieRuntimeFrameRequest &request)
{
    RenderKey key;
    key.assetRevision = request.assetRevision;
    key.effectiveTheme = request.effectiveTheme;
    key.frame = request.frame;
    key.physicalPixelSize = request.physicalPixelSize;
    return key;
}

qint64 imageByteCount(const QImage &image)
{
    if (image.isNull() || image.height() <= 0 || image.bytesPerLine() <= 0) {
        return 0;
    }

    const qint64 bytesPerLine = image.bytesPerLine();
    const qint64 height = image.height();
    if (bytesPerLine > std::numeric_limits<qint64>::max() / height) {
        return 0;
    }
    return bytesPerLine * height;
}

LottieRuntimeFrameResult resultFor(const RenderKey &key, const LottieImmutableFrame &frame)
{
    LottieRuntimeFrameResult result;
    result.failure = LottieRuntimeFrameResult::Failure::None;
    result.assetRevision = key.assetRevision;
    result.effectiveTheme = key.effectiveTheme;
    result.frameNumber = key.frame;
    result.physicalPixelSize = key.physicalPixelSize;
    result.frame = frame;
    return result;
}

LottieRuntimeFrameResult failureResultFor(
    const RenderKey &key, LottieRuntimeFrameResult::Failure failure)
{
    LottieRuntimeFrameResult result;
    result.failure = failure;
    result.assetRevision = key.assetRevision;
    result.effectiveTheme = key.effectiveTheme;
    result.frameNumber = key.frame;
    result.physicalPixelSize = key.physicalPixelSize;
    return result;
}

}

class LottieRuntimeFrameScheduler::Private
{
public:
    // Interest 表示某个组件现在想要哪一帧；Job 表示一次真正的栅格任务。
    // 组件改请求时只保留最新 Interest；动画、主题、帧和尺寸都相同的
    // 多个组件，可以共用同一个 Job。
    enum class InterestState {
        Idle,
        Queued,
        Running,
        Deferred,
        PendingDelivery
    };

    struct PendingDelivery
    {
        quint64 token = 0;
        RenderKey key;
        LottieRuntimeFrameResult result;
    };

    struct Interest
    {
        bool current = false;
        quint64 token = 0;
        quint64 admissionSerial = 0;
        quint64 jobId = 0;
        RenderKey key;
        LottieRuntimeSessionKey sessionKey;
        bool sessionDemanded = false;
        LottieRuntimeFrameRequest request;
        LottieRuntimeFrameScheduler::Completion completion;
        InterestState state = InterestState::Idle;
        std::optional<PendingDelivery> pending;
        SchedulerTimePoint requestedAt;
    };

    struct Job
    {
        quint64 jobId = 0;
        RenderKey key;
        LottieRuntimeFrameRequest request;
        QList<quint64> subscribers;
        quint64 admissionSerial = 0;
        SchedulerTimePoint requestedAt;
        SchedulerTimePoint enqueuedAt;
    };

    struct CacheEntry
    {
        RenderKey key;
        CacheLookupKey lookupKey;
        LottieImmutableFrame frame;
        qint64 bytes = 0;
        CacheEntry *older = nullptr;
        CacheEntry *newer = nullptr;
    };

    struct CacheRevision
    {
        LottieAssetRevision revision;
        quint64 id = 0;
        int entryCount = 0;
    };

    explicit Private(LottieRuntimeFrameScheduler *owner)
        : owner(owner)
    {
    }

    int queuedJobIndex(quint64 jobId) const
    {
        for (int index = 0; index < queuedJobs.size(); ++index) {
            if (queuedJobs.at(index).jobId == jobId) {
                return index;
            }
        }
        return -1;
    }

    int queuedKeyIndex(const RenderKey &key) const
    {
        for (int index = 0; index < queuedJobs.size(); ++index) {
            if (queuedJobs.at(index).key == key) {
                return index;
            }
        }
        return -1;
    }

    quint64 findRevisionId(const LottieAssetRevision &revision) const
    {
        for (const CacheRevision &cacheRevision : cacheRevisions) {
            if (cacheRevision.revision == revision) {
                return cacheRevision.id;
            }
        }
        return 0;
    }

    quint64 ensureRevisionId(const LottieAssetRevision &revision)
    {
        const quint64 existingId = findRevisionId(revision);
        if (existingId != 0) {
            return existingId;
        }

        const quint64 id = nextCacheRevisionId++;
        // 帧缓存只需要一个紧凑 revision 标识来构造查找键。完整资产由 Host、
        // 待处理任务和栅格 session 按实际需要持有，缓存不能延长其生命周期。
        cacheRevisions.append({revision, id, 0});
        return id;
    }

    CacheLookupKey lookupKeyFor(const RenderKey &key, quint64 revisionId) const
    {
        CacheLookupKey lookupKey;
        lookupKey.revisionId = revisionId;
        lookupKey.effectiveTheme = key.effectiveTheme;
        lookupKey.frame = key.frame;
        lookupKey.physicalPixelSize = key.physicalPixelSize;
        return lookupKey;
    }

    void linkAtLruTail(CacheEntry *entry)
    {
        entry->older = cacheLruTail;
        entry->newer = nullptr;
        if (cacheLruTail) {
            cacheLruTail->newer = entry;
        } else {
            cacheLruHead = entry;
        }
        cacheLruTail = entry;
    }

    void unlinkFromLru(CacheEntry *entry)
    {
        if (entry->older) {
            entry->older->newer = entry->newer;
        } else {
            cacheLruHead = entry->newer;
        }
        if (entry->newer) {
            entry->newer->older = entry->older;
        } else {
            cacheLruTail = entry->older;
        }
        entry->older = nullptr;
        entry->newer = nullptr;
    }

    void touchCacheEntry(CacheEntry *entry)
    {
        if (entry != cacheLruTail) {
            unlinkFromLru(entry);
            linkAtLruTail(entry);
        }
    }

    void releaseRevision(quint64 revisionId)
    {
        for (int index = 0; index < cacheRevisions.size(); ++index) {
            if (cacheRevisions.at(index).id == revisionId) {
                if (cacheRevisions.at(index).entryCount == 0) {
                    cacheRevisions.removeAt(index);
                }
                return;
            }
        }
    }

    void removeCacheEntry(CacheEntry *entry)
    {
        if (!entry) {
            return;
        }
        const qint64 bytes = entry->bytes;
        const quint64 revisionId = entry->lookupKey.revisionId;
        cacheIndex.remove(entry->lookupKey);
        unlinkFromLru(entry);
        cacheEntries.remove_if([entry](const CacheEntry &candidate) {
            return &candidate == entry;
        });
        cacheBytes -= bytes;
        for (CacheRevision &cacheRevision : cacheRevisions) {
            if (cacheRevision.id == revisionId) {
                Q_ASSERT(cacheRevision.entryCount > 0);
                --cacheRevision.entryCount;
                break;
            }
        }
        releaseRevision(revisionId);
    }

    CacheEntry *cacheEntryFor(const RenderKey &key) const
    {
        const quint64 revisionId = findRevisionId(key.assetRevision);
        if (revisionId == 0) {
            return nullptr;
        }
        const CacheLookupKey lookupKey = lookupKeyFor(key, revisionId);
        const auto it = cacheIndex.constFind(lookupKey);
        return it == cacheIndex.constEnd() ? nullptr : it.value();
    }

    bool cacheResult(const RenderKey &key, LottieRuntimeFrameResult *result)
    {
        CacheEntry *entry = cacheEntryFor(key);
        if (!entry) {
            return false;
        }

        touchCacheEntry(entry);
        diagnostics.recordCacheHit();
        *result = resultFor(key, entry->frame);
        return true;
    }

    void insertCache(const RenderKey &key, const LottieImmutableFrame &frame)
    {
        const qint64 bytes = imageByteCount(frame.image());
        if (bytes <= 0 || bytes > kFrameCacheBudgetBytes) {
            return;
        }

        if (CacheEntry *existing = cacheEntryFor(key)) {
            removeCacheEntry(existing);
        }
        while (cacheLruHead) {
            if (cacheEntries.size() < kMaximumFrameCacheEntries &&
                cacheBytes <= kFrameCacheBudgetBytes - bytes) {
                break;
            }
            removeCacheEntry(cacheLruHead);
        }

        if (cacheEntries.size() >= kMaximumFrameCacheEntries ||
            cacheBytes > kFrameCacheBudgetBytes - bytes) {
            return;
        }

        const quint64 revisionId = ensureRevisionId(key.assetRevision);
        const CacheLookupKey lookupKey = lookupKeyFor(key, revisionId);
        cacheEntries.push_back({key, lookupKey, frame, bytes});
        auto *entry = &cacheEntries.back();
        cacheIndex.insert(lookupKey, entry);
        linkAtLruTail(entry);
        for (CacheRevision &cacheRevision : cacheRevisions) {
            if (cacheRevision.id == revisionId) {
                ++cacheRevision.entryCount;
                break;
            }
        }
        cacheBytes += bytes;
    }

    quint64 oldestDeferredSubscription() const
    {
        quint64 bestSerial = std::numeric_limits<quint64>::max();
        quint64 bestId = 0;
        for (auto it = interests.constBegin(); it != interests.constEnd(); ++it) {
            if (it.value().current && it.value().state == InterestState::Deferred &&
                it.value().admissionSerial < bestSerial) {
                bestSerial = it.value().admissionSerial;
                bestId = it.key();
            }
        }
        return bestId;
    }

    int oldestQueuedJobIndex() const
    {
        int bestIndex = -1;
        quint64 bestSerial = std::numeric_limits<quint64>::max();
        for (int index = 0; index < queuedJobs.size(); ++index) {
            if (queuedJobs.at(index).admissionSerial < bestSerial) {
                bestSerial = queuedJobs.at(index).admissionSerial;
                bestIndex = index;
            }
        }
        return bestIndex;
    }

    std::optional<Job> takeDeferredJob(quint64 subscriptionId)
    {
        const auto deferredIt = interests.constFind(subscriptionId);
        if (deferredIt == interests.constEnd() || !deferredIt.value().current ||
            deferredIt.value().state != InterestState::Deferred) {
            return std::nullopt;
        }

        Job job;
        const int queuedIndex = queuedKeyIndex(deferredIt.value().key);
        if (queuedIndex >= 0) {
            job = queuedJobs.takeAt(queuedIndex);
        } else {
            job.jobId = nextJobId++;
            job.key = deferredIt.value().key;
            job.request = deferredIt.value().request;
            job.admissionSerial = deferredIt.value().admissionSerial;
            job.requestedAt = deferredIt.value().requestedAt;
            job.enqueuedAt = deferredIt.value().requestedAt;
        }

        for (auto it = interests.begin(); it != interests.end(); ++it) {
            if (it.value().current && it.value().state == InterestState::Deferred &&
                it.value().key == job.key) {
                job.subscribers.append(it.key());
                job.admissionSerial = qMin(job.admissionSerial,
                                            it.value().admissionSerial);
            }
        }
        return job;
    }

    int deferredCount() const
    {
        int count = 0;
        for (auto it = interests.constBegin(); it != interests.constEnd(); ++it) {
            if (it.value().current && it.value().state == InterestState::Deferred) {
                ++count;
            }
        }
        return count;
    }

    void removeSubscriptionFromJobs(quint64 subscriptionId)
    {
        auto interestIt = interests.find(subscriptionId);
        if (interestIt == interests.end()) {
            return;
        }

        Interest &interest = interestIt.value();
        if (interest.state == InterestState::Queued) {
            const int index = queuedJobIndex(interest.jobId);
            if (index >= 0) {
                Job &job = queuedJobs[index];
                job.subscribers.removeAll(subscriptionId);
                if (job.subscribers.isEmpty()) {
                    queuedJobs.removeAt(index);
                    diagnostics.recordSubscriberlessQueuedJobRemoval();
                } else {
                    quint64 oldestAdmission = std::numeric_limits<quint64>::max();
                    for (const quint64 subscriber : std::as_const(job.subscribers)) {
                        const auto subscriberInterest = interests.constFind(subscriber);
                        if (subscriberInterest != interests.constEnd()) {
                            oldestAdmission = qMin(oldestAdmission,
                                                   subscriberInterest.value().admissionSerial);
                        }
                    }
                    job.admissionSerial = oldestAdmission;
                }
            }
        } else if (interest.state == InterestState::Running && runningJob) {
            const int removed = runningJob->subscribers.removeAll(subscriptionId);
            diagnostics.recordDroppedResults(removed);
        }

        interest.jobId = 0;
        interest.state = InterestState::Idle;
        interest.pending.reset();
    }

    void releaseSessionDemand(quint64 subscriptionId)
    {
        auto interestIt = interests.find(subscriptionId);
        if (interestIt == interests.end() || !interestIt.value().sessionDemanded) {
            return;
        }

        if (worker) {
            worker->releaseSession(interestIt.value().sessionKey);
        }
        interestIt.value().sessionDemanded = false;
        interestIt.value().sessionKey = {};
    }

    void scheduleDelivery(quint64 subscriptionId,
                           quint64 token,
                           const RenderKey &key,
                           const LottieRuntimeFrameResult &result)
    {
        auto it = interests.find(subscriptionId);
        if (it == interests.end() || !it.value().current ||
            it.value().token != token) {
            return;
        }

        it.value().state = InterestState::PendingDelivery;
        it.value().pending = PendingDelivery{token, key, result};
        if (batchScheduled) {
            return;
        }

        batchScheduled = true;
        ++pendingGuiBatches;
        diagnostics.recordGuiBatchScheduled(pendingGuiBatches);
        // 缓存命中和后台渲染结果都在下一次事件循环统一交付。无论同一时刻
        // 完成多少帧，GUI 队列里最多只有一个待处理的批次。
        QMetaObject::invokeMethod(owner, [owner = owner] {
            owner->flushGuiBatch();
        }, Qt::QueuedConnection);
    }

    LottieRuntimeFrameScheduler *owner = nullptr;
    std::unique_ptr<LottieRuntimeRasterWorker> worker;
    QHash<quint64, Interest> interests;
    QList<Job> queuedJobs;
    std::optional<Job> runningJob;
    std::list<CacheEntry> cacheEntries;
    QHash<CacheLookupKey, CacheEntry *> cacheIndex;
    QList<CacheRevision> cacheRevisions;
    CacheEntry *cacheLruHead = nullptr;
    CacheEntry *cacheLruTail = nullptr;
    qint64 cacheBytes = 0;
    quint64 nextCacheRevisionId = 1;
    quint64 nextJobId = 1;
    quint64 nextAdmissionSerial = 1;
    bool stopping = false;
    bool batchScheduled = false;
    int pendingGuiBatches = 0;
    [[no_unique_address]] LottieRuntimeFrameSchedulerDiagnosticsType diagnostics;
    bool guiBatchFlushHeld = false;
};

LottieRuntimeFrameScheduler::LottieRuntimeFrameScheduler(
    int maximumRasterSessions, QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->worker.reset(new LottieRuntimeRasterWorker(maximumRasterSessions, this));
}

LottieRuntimeFrameScheduler::~LottieRuntimeFrameScheduler()
{
    stop();
    d->worker.reset();
    d.reset();
}

bool LottieRuntimeFrameScheduler::request(quint64 subscriptionId,
                                          const LottieRuntimeFrameRequest &request,
                                          Completion completion)
{
    if (subscriptionId == 0 || !completion || d->stopping ||
        !request.assetRevision.isValid() || !request.asset.isValid() ||
        request.asset.revision() != request.assetRevision ||
        request.asset.frameCount() <= 0 ||
        !request.asset.canUseTheme(request.effectiveTheme) ||
        !LottieRenderer::isRenderSizeAllowed(request.physicalPixelSize)) {
        return false;
    }

    const SchedulerTimePoint requestedAt = SchedulerClock::now();
    LottieRuntimeSessionKey requestedSessionKey;
    requestedSessionKey.assetRevision = request.assetRevision;
    requestedSessionKey.effectiveTheme = request.effectiveTheme;
    const auto existingInterest = d->interests.constFind(subscriptionId);
    const bool preservesAdmission = existingInterest != d->interests.constEnd() &&
        existingInterest.value().current &&
        (existingInterest.value().state == Private::InterestState::Queued ||
         existingInterest.value().state == Private::InterestState::Deferred) &&
        existingInterest.value().admissionSerial != 0;
    const quint64 admissionSerial = preservesAdmission
        ? existingInterest.value().admissionSerial
        : d->nextAdmissionSerial++;
    const bool replacesOutstanding = existingInterest != d->interests.constEnd() &&
        existingInterest.value().current &&
        existingInterest.value().state != Private::InterestState::Idle &&
        existingInterest.value().key != renderKeyFor(request);
    const bool keepsSessionDemand = existingInterest != d->interests.constEnd() &&
        existingInterest.value().sessionDemanded &&
        existingInterest.value().sessionKey == requestedSessionKey;
    const bool releasesPreviousSessionDemand = existingInterest != d->interests.constEnd() &&
        existingInterest.value().sessionDemanded && !keepsSessionDemand;
    const LottieRuntimeSessionKey previousSessionKey =
        releasesPreviousSessionDemand ? existingInterest.value().sessionKey
                                      : LottieRuntimeSessionKey();
    if (replacesOutstanding) {
        d->diagnostics.recordSupersededRequest();
    }
    d->removeSubscriptionFromJobs(subscriptionId);
    if (releasesPreviousSessionDemand) {
        d->worker->releaseSession(previousSessionKey);
    }

    Private::Interest &interest = d->interests[subscriptionId];
    interest.current = true;
    interest.token++;
    interest.admissionSerial = admissionSerial;
    interest.key = renderKeyFor(request);
    interest.sessionKey = requestedSessionKey;
    interest.sessionDemanded = keepsSessionDemand;
    interest.request = request;
    interest.completion = std::move(completion);
    interest.state = Private::InterestState::Idle;
    interest.jobId = 0;
    interest.requestedAt = requestedAt;

    if (!keepsSessionDemand) {
        d->worker->retainSession(requestedSessionKey);
        interest.sessionDemanded = true;
    }

    d->diagnostics.recordRenderKey(interest.key);

    const int themeOrdinal = request.asset.selectableThemeOrdinal(
        request.effectiveTheme);
    if (request.asset.hasDeterministicThemeFailure(themeOrdinal)) {
        const LottieRuntimeFrameResult knownFailure = failureResultFor(
            interest.key, LottieRuntimeFrameResult::Failure::DeterministicTheme);
        d->scheduleDelivery(subscriptionId, interest.token, interest.key, knownFailure);
        return true;
    }

    LottieRuntimeFrameResult cached;
    if (d->cacheResult(interest.key, &cached)) {
        d->scheduleDelivery(subscriptionId, interest.token, interest.key, cached);
        return true;
    }

    if (d->runningJob && d->runningJob->key == interest.key) {
        d->runningJob->subscribers.append(subscriptionId);
        interest.state = Private::InterestState::Running;
        interest.jobId = d->runningJob->jobId;
        d->diagnostics.recordCoalescedRequest();
        return true;
    }

    const int queuedIndex = d->queuedKeyIndex(interest.key);
    if (queuedIndex >= 0) {
        Private::Job &queuedJob = d->queuedJobs[queuedIndex];
        queuedJob.subscribers.append(subscriptionId);
        queuedJob.admissionSerial = qMin(queuedJob.admissionSerial,
                                         interest.admissionSerial);
        interest.state = Private::InterestState::Queued;
        interest.jobId = queuedJob.jobId;
        d->diagnostics.recordCoalescedRequest();
    } else if (d->queuedJobs.size() < kMaximumQueuedKeys) {
        Private::Job job;
        job.jobId = d->nextJobId++;
        job.key = interest.key;
        job.request = interest.request;
        job.subscribers.append(subscriptionId);
        job.admissionSerial = interest.admissionSerial;
        job.requestedAt = requestedAt;
        job.enqueuedAt = requestedAt;
        d->queuedJobs.append(std::move(job));
        d->diagnostics.recordQueuedKeys(d->queuedJobs.size());
        interest.state = Private::InterestState::Queued;
        interest.jobId = d->queuedJobs.constLast().jobId;
    } else {
        interest.state = Private::InterestState::Deferred;
    }

    dispatchNext();
    return true;
}

void LottieRuntimeFrameScheduler::unsubscribe(quint64 subscriptionId)
{
    auto it = d->interests.find(subscriptionId);
    if (it == d->interests.end()) {
        return;
    }

    d->removeSubscriptionFromJobs(subscriptionId);
    d->releaseSessionDemand(subscriptionId);
    d->interests.erase(it);
    dispatchNext();
}

void LottieRuntimeFrameScheduler::suspend(quint64 subscriptionId)
{
    auto it = d->interests.find(subscriptionId);
    if (it == d->interests.end()) {
        return;
    }

    // 暂停只撤销当前帧兴趣和 session demand；source subscription 仍由
    // LottieRuntimeService 持有，因此 metadata 与 Host facade 状态不会丢失。
    d->removeSubscriptionFromJobs(subscriptionId);
    d->releaseSessionDemand(subscriptionId);
    d->interests.erase(it);
    dispatchNext();
}

void LottieRuntimeFrameScheduler::dispatchNext()
{
    if (d->stopping || d->runningJob) {
        return;
    }

    Private::Job nextJob;
    const quint64 oldestDeferredId = d->oldestDeferredSubscription();
    const int oldestQueuedIndex = d->oldestQueuedJobIndex();
    const quint64 deferredAdmission = oldestDeferredId == 0
        ? std::numeric_limits<quint64>::max()
        : d->interests.value(oldestDeferredId).admissionSerial;
    const quint64 queuedAdmission = oldestQueuedIndex < 0
        ? std::numeric_limits<quint64>::max()
        : d->queuedJobs.at(oldestQueuedIndex).admissionSerial;
    if (oldestDeferredId != 0 && deferredAdmission < queuedAdmission) {
        // Deferred 仍然只保存该组件的最新帧，但排队次序沿用它第一次开始
        // 等待的时刻。持续更新的播放组件不能反复插到旧任务前面。
        std::optional<Private::Job> deferredJob =
            d->takeDeferredJob(oldestDeferredId);
        if (!deferredJob) {
            return;
        }
        nextJob = std::move(*deferredJob);
    } else if (oldestQueuedIndex >= 0) {
        // Session residency is selected by demand changes in the raster worker;
        // queue dispatch must not scan frame order and turn it into an LRU.
        nextJob = d->queuedJobs.takeAt(oldestQueuedIndex);
    } else if (oldestDeferredId != 0) {
        // 没有普通队列时，接纳当前等待最久的 Deferred 请求。
        std::optional<Private::Job> deferredJob =
            d->takeDeferredJob(oldestDeferredId);
        if (!deferredJob) {
            return;
        }
        nextJob = std::move(*deferredJob);
    } else {
        return;
    }

    d->runningJob = std::move(nextJob);
    for (const quint64 subscriptionId : std::as_const(d->runningJob->subscribers)) {
        auto it = d->interests.find(subscriptionId);
        if (it == d->interests.end() || !it.value().current) {
            continue;
        }
        it.value().state = Private::InterestState::Running;
        it.value().jobId = d->runningJob->jobId;
    }

    d->diagnostics.recordRunningJobs(1);
    const SchedulerTimePoint dispatchStartedAt = SchedulerClock::now();
    d->diagnostics.recordQueueWait(d->runningJob->enqueuedAt, dispatchStartedAt);
    const quint64 jobId = d->runningJob->jobId;
    const bool accepted = d->worker->enqueue(
        d->runningJob->request,
        [this, jobId](const LottieRuntimeFrameResult &result) {
            completeWorkerJob(jobId, result);
        });
    if (!accepted) {
        d->runningJob.reset();
        dispatchNext();
    } else {
        d->diagnostics.recordRasterJob();
    }
}

void LottieRuntimeFrameScheduler::completeWorkerJob(
    quint64 jobId, const LottieRuntimeFrameResult &result)
{
    if (d->stopping || !d->runningJob || d->runningJob->jobId != jobId) {
        if (d->stopping && d->runningJob && d->runningJob->jobId == jobId) {
            d->diagnostics.recordDroppedResults(d->runningJob->subscribers.size());
        }
        return;
    }

    Private::Job job = std::move(*d->runningJob);
    d->runningJob.reset();
    d->diagnostics.recordEndToEnd(job.requestedAt, SchedulerClock::now());
    d->diagnostics.recordRasterExecution(result.diagnostics.executionNanoseconds());

    LottieRuntimeFrameResult canonicalResult = result;
    canonicalResult.assetRevision = job.key.assetRevision;
    canonicalResult.effectiveTheme = job.key.effectiveTheme;
    canonicalResult.frameNumber = job.key.frame;
    canonicalResult.physicalPixelSize = job.key.physicalPixelSize;
    const bool success = canonicalResult.failure == LottieRuntimeFrameResult::Failure::None &&
        canonicalResult.frame.isValid();
    if (success) {
        d->diagnostics.recordImmutableFrame();
        d->insertCache(job.key, canonicalResult.frame);
    }

    for (const quint64 subscriptionId : std::as_const(job.subscribers)) {
        auto it = d->interests.find(subscriptionId);
        if (it == d->interests.end() || !it.value().current ||
            it.value().state != Private::InterestState::Running ||
            it.value().jobId != jobId || it.value().key != job.key) {
            d->diagnostics.recordDroppedResults(1);
            continue;
        }
        d->scheduleDelivery(subscriptionId, it.value().token, job.key, canonicalResult);
    }

    dispatchNext();
}

void LottieRuntimeFrameScheduler::flushGuiBatch()
{
    if (d->guiBatchFlushHeld) {
        return;
    }

    if (d->pendingGuiBatches > 0) {
        d->diagnostics.recordGuiBatchDelivery();
        --d->pendingGuiBatches;
    }
    d->batchScheduled = false;
    if (d->stopping) {
        for (auto it = d->interests.begin(); it != d->interests.end(); ++it) {
            it.value().pending.reset();
        }
        return;
    }

    QList<quint64> pendingSubscriptions;
    for (auto it = d->interests.constBegin(); it != d->interests.constEnd(); ++it) {
        if (it.value().pending) {
            pendingSubscriptions.append(it.key());
        }
    }

    for (const quint64 subscriptionId : std::as_const(pendingSubscriptions)) {
        auto it = d->interests.find(subscriptionId);
        if (it == d->interests.end() || !it.value().current || !it.value().pending) {
            continue;
        }

        const Private::PendingDelivery pending = *it.value().pending;
        if (it.value().token != pending.token || it.value().key != pending.key) {
            // 本批次前面的回调可能已经让这个组件请求了新帧。此时保留新请求，
            // 等它自己的批次处理，不能在这里把状态改回 Idle。
            continue;
        }
        it.value().pending.reset();
        it.value().state = Private::InterestState::Idle;
        it.value().jobId = 0;

        // 调用组件回调前再检查一次，因为本批次前面的回调可能已经换了请求，
        // 也可能销毁了这个订阅。
        it = d->interests.find(subscriptionId);
        if (it == d->interests.end() || !it.value().current ||
            it.value().token != pending.token || it.value().key != pending.key ||
            d->stopping || !it.value().completion) {
            continue;
        }
        if (pending.result.failure ==
            LottieRuntimeFrameResult::Failure::DeterministicTheme) {
            const int themeOrdinal = it.value().request.asset.selectableThemeOrdinal(
                pending.key.effectiveTheme);
            it.value().request.asset.rememberDeterministicThemeFailure(themeOrdinal);
        }
        const Completion completion = it.value().completion;
        d->diagnostics.recordCompletionDelivery();
        completion(pending.result);
    }
}

void LottieRuntimeFrameScheduler::stop()
{
    if (d->stopping) {
        return;
    }

    d->stopping = true;
    d->queuedJobs.clear();
    for (auto it = d->interests.begin(); it != d->interests.end(); ++it) {
        it.value().current = false;
        it.value().pending.reset();
        it.value().completion = {};
        it.value().request = {};
        it.value().state = Private::InterestState::Idle;
        it.value().jobId = 0;
    }
    if (d->runningJob) {
        d->diagnostics.recordDroppedResults(d->runningJob->subscribers.size());
        d->runningJob->subscribers.clear();
    }
    d->batchScheduled = false;
    d->pendingGuiBatches = 0;
    d->guiBatchFlushHeld = false;
    if (d->worker) {
        d->worker->stop();
        d->worker.reset();
    }
    d->runningJob.reset();
    d->interests.clear();
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
LottieRuntimeFrameSchedulerStats LottieRuntimeFrameScheduler::testStats() const
{
    LottieRuntimeFrameSchedulerStats stats;
    stats.queuedKeys = d->queuedJobs.size();
    stats.deferredSubscriptions = d->deferredCount();
    stats.runningJobs = d->runningJob ? 1 : 0;
    stats.rasterWorkerCount = d->worker ? 1 : 0;
    stats.pendingGuiBatches = d->pendingGuiBatches;
    stats.cacheEntryCount = d->cacheEntries.size();
    stats.cacheBytes = d->cacheBytes;
    d->diagnostics.populate(&stats);
    return stats;
}

void LottieRuntimeFrameScheduler::resetDiagnostics()
{
    d->diagnostics.reset();
}

bool LottieRuntimeFrameScheduler::testResetCacheAndSession()
{
    if (d->stopping || d->runningJob || !d->queuedJobs.isEmpty() ||
        d->pendingGuiBatches != 0 || d->batchScheduled) {
        return false;
    }

    d->cacheEntries.clear();
    d->cacheIndex.clear();
    d->cacheRevisions.clear();
    d->cacheLruHead = nullptr;
    d->cacheLruTail = nullptr;
    d->cacheBytes = 0;
    if (d->worker) {
        d->worker->testResetSessions();
    }
    return true;
}

void LottieRuntimeFrameScheduler::testHoldGuiBatch(bool hold)
{
    d->guiBatchFlushHeld = hold;
}

void LottieRuntimeFrameScheduler::testFlushGuiBatch()
{
    d->guiBatchFlushHeld = false;
    flushGuiBatch();
}
#endif

} // namespace UkuiQuick
