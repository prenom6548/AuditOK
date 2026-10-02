/*
 * Copyright (C) 2026, Kylin Software Co., Ltd.
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

#include "lottie-runtime-service_p.h"

#include "lottie-renderer.h"
#include "ukui/lottie/lottie-cache-manager.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QSharedPointer>
#include <QThread>
#include <QWaitCondition>

#include <atomic>
#include <mutex>
#include <utility>

namespace UkuiQuick {

namespace {

constexpr char kRasterSessionCacheEntriesEnvironment[] =
    "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES";

int configuredRasterSessionCacheEntries()
{
    bool valid = false;
    const qlonglong configured = qgetenv(kRasterSessionCacheEntriesEnvironment).toLongLong(&valid);
    if (!valid) {
        return LottieRuntimeRasterSessionCache::kDefaultEntries;
    }
    return static_cast<int>(qBound<qlonglong>(
        LottieRuntimeRasterSessionCache::kMinimumEntries,
        configured,
        LottieRuntimeRasterSessionCache::kMaximumEntries));
}

LottieRuntimeSourceResult executeDefaultSourceWork(const LottieRuntimeSourceRequest &request,
                                                   const LottieRuntimeService::SourceCancellation &isCancelled)
{
    // 后台加载函数只接收来源，不接收主题。文件只解析一次，加载完成后再按
    // 每个订阅者自己的主题选择分别生成结果。
    LottieRuntimeSourceResult result;
    const LottieAssetRequestResult assetResult =
        LottieCacheManager::instance()->requestAsset(request.source, isCancelled);
    result.asset = assetResult.asset;
    result.sourceNotFound = assetResult.sourceNotFound;
    result.loaded = result.asset.isValid();
    return result;
}

std::mutex s_serviceMutex;
LottieRuntimeService *s_service = nullptr;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
LottieRuntimeService::SourceExecutor s_testSourceExecutor;
#endif
bool s_applicationStopping = false;
QPointer<QCoreApplication> s_shutdownHookApplication;

} // namespace

void installLottieRuntimeApplicationShutdownHook()
{
    QCoreApplication *app = QCoreApplication::instance();
    if (!app) {
        return;
    }

    bool connectHook = false;
    {
        std::lock_guard<std::mutex> lock(s_serviceMutex);
        if (s_shutdownHookApplication != app) {
            s_shutdownHookApplication = app;
            connectHook = true;
        }
    }
    if (!connectHook) {
        return;
    }

    QObject::connect(app, &QCoreApplication::aboutToQuit, app, [] {
        LottieRuntimeService *service = nullptr;
        {
            std::lock_guard<std::mutex> lock(s_serviceMutex);
            s_applicationStopping = true;
            service = s_service;
        }
        if (service) {
            service->stopForShutdown();
        }
    });
}

Q_COREAPP_STARTUP_FUNCTION(installLottieRuntimeApplicationShutdownHook)

class LottieRuntimeServicePrivate
{
public:
    // 一个订阅者正在等待哪项来源加载任务。
    struct Observation {
        quint64 subscriptionId = 0;
        QString requestedTheme;
        LottieRuntimeService::SourceCompletion completion;
    };

    // 一项可被多个订阅者等待的来源加载任务。相同来源仍在排队时可以合并；
    // 一旦开始复制文件，就不再加入后来请求，避免同一路径的新旧内容混用。
    struct SourceJob {
        quint64 jobId = 0;
        QString sourceKey;
        LottieSourceReference source;
        bool running = false;
        QSharedPointer<std::atomic<bool>> cancellation;
        QList<Observation> observations;
    };

    // 移除加载任务；如果它已经运行，就设置取消标志让后台线程自行收尾。
    void removeJobLocked(quint64 jobId)
    {
        auto it = jobs.find(jobId);
        if (it == jobs.end()) {
            return;
        }
        const QString sourceKey = it.value().sourceKey;
        if (it.value().cancellation) {
            it.value().cancellation->store(true);
        }
        jobs.erase(it);
        const auto keyIt = jobBySourceKey.find(sourceKey);
        if (keyIt != jobBySourceKey.end() && keyIt.value() == jobId) {
            jobBySourceKey.erase(keyIt);
        }
        jobOrder.removeOne(jobId);
    }

    // 让订阅者离开当前任务；最后一个等待者离开后，任务才会被移除或取消。
    void detachSubscriptionLocked(quint64 subscriptionId)
    {
        const auto obIt = observedJobBySubscription.find(subscriptionId);
        if (obIt == observedJobBySubscription.end()) {
            return;
        }
        const quint64 jobId = obIt.value();
        observedJobBySubscription.erase(obIt);
        const auto jobIt = jobs.find(jobId);
        if (jobIt == jobs.end()) {
            return;
        }
        QList<Observation> &observations = jobIt.value().observations;
        for (int index = observations.size() - 1; index >= 0; --index) {
            if (observations.at(index).subscriptionId == subscriptionId) {
                observations.removeAt(index);
            }
        }
        if (observations.isEmpty()) {
            removeJobLocked(jobId);
        }
    }

    QMutex mutex;
    QMutex shutdownMutex;
    QWaitCondition hasWork;
    QList<quint64> jobOrder;
    QHash<quint64, SourceJob> jobs;
    QHash<QString, quint64> jobBySourceKey;
    QHash<quint64, quint64> observedJobBySubscription;
    QSet<quint64> subscriptions;
    [[no_unique_address]] LottieRuntimeSourceDiagnostics diagnostics;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    std::function<void()> testStoppingTransitionHook;
#endif
    LottieRuntimeService::SourceExecutor executor;
    QThread *worker = nullptr;
    LottieRuntimeFrameScheduler *frameScheduler = nullptr;
    quint64 nextSubscriptionId = 1;
    quint64 nextJobId = 1;
    int maximumRasterSessions = LottieRuntimeRasterSessionCache::kDefaultEntries;
    bool stopping = false;
    bool workerStarted = false;
};

class LottieRuntimeAssetWorker : public QThread
{
public:
    using Deliverer = std::function<void(quint64, const LottieRuntimeSourceResult &)>;

    LottieRuntimeAssetWorker(LottieRuntimeServicePrivate *data,
                             QObject *deliveryContext, Deliverer deliverer)
        : d(data)
        , m_deliveryContext(deliveryContext)
        , m_deliverer(std::move(deliverer))
    {
    }

protected:
    void run() override
    {
        // 来源路径解析、读文件、解压和 JSON 解析都在这个线程执行。它不直接
        // 访问 Icon/LottieItem；完成后只把验证好的动画数据排队送回应用线程。
        QMutexLocker locker(&d->mutex);
        while (true) {
            while (d->jobOrder.isEmpty() && !d->stopping) {
                d->hasWork.wait(locker.mutex());
            }

            if (d->stopping) {
                d->jobOrder.clear();
                return;
            }

            const quint64 jobId = d->jobOrder.takeFirst();
            const auto jobIt = d->jobs.find(jobId);
            if (jobIt == d->jobs.end()) {
                continue;
            }
            // 从这里开始真正复制和解析文件。后来即使又请求同一路径，也会创建
            // 新任务重新读取，保证文件被替换后能够得到新的内容版本。
            jobIt.value().running = true;
            const LottieSourceReference source = jobIt.value().source;
            const QSharedPointer<std::atomic<bool>> cancellationFlag = jobIt.value().cancellation;
            locker.unlock();

            LottieRuntimeSourceResult result;
            bool executed = false;
            auto sourceWorkStarted = LottieRuntimeDiagnosticsClock::now();
            auto sourceWorkCompleted = sourceWorkStarted;
            if (d->executor) {
                const LottieRuntimeService::SourceCancellation isCancelled = [cancellationFlag] {
                    return cancellationFlag && cancellationFlag->load();
                };
                LottieRuntimeSourceRequest request;
                request.source = source;
                sourceWorkStarted = LottieRuntimeDiagnosticsClock::now();
                result = d->executor(request, isCancelled);
                sourceWorkCompleted = LottieRuntimeDiagnosticsClock::now();
                executed = true;
            }

            locker.relock();
            if (executed) {
                d->diagnostics.recordLoading(sourceWorkStarted, sourceWorkCompleted);
            }
            // 被取消或已进入停止流程的工作不发布结果；取消不构成可观察错误。
            const bool cancelled = cancellationFlag && cancellationFlag->load();
            if (cancelled || d->stopping) {
                d->removeJobLocked(jobId);
                continue;
            }
            if (m_deliverer) {
                const Deliverer deliverer = m_deliverer;
                const LottieRuntimeSourceResult deliveredResult = result;
                QMetaObject::invokeMethod(m_deliveryContext,
                                          [deliverer, jobId, deliveredResult] {
                                              deliverer(jobId, deliveredResult);
                                          },
                                          Qt::QueuedConnection);
            }
        }
    }

private:
    LottieRuntimeServicePrivate *d;
    QObject *m_deliveryContext;
    Deliverer m_deliverer;
};

LottieRuntimeSubscription::LottieRuntimeSubscription(LottieRuntimeService *service, quint64 id)
    : m_service(service)
    , m_id(id)
{
}

LottieRuntimeSubscription::LottieRuntimeSubscription(LottieRuntimeSubscription &&other) noexcept
    : m_service(other.m_service)
    , m_id(other.m_id)
{
    other.m_service = nullptr;
    other.m_id = 0;
}

LottieRuntimeSubscription &LottieRuntimeSubscription::operator=(LottieRuntimeSubscription &&other) noexcept
{
    if (this != &other) {
        if (m_service) {
            m_service->unsubscribe(m_id);
        }
        m_service = other.m_service;
        m_id = other.m_id;
        other.m_service = nullptr;
        other.m_id = 0;
    }
    return *this;
}

LottieRuntimeSubscription::~LottieRuntimeSubscription()
{
    if (m_service) {
        m_service->unsubscribe(m_id);
    }
}

bool LottieRuntimeSubscription::isValid() const
{
    return m_service && m_id != 0;
}

LottieRuntimeService::LottieRuntimeService(const SourceExecutor &executor, QObject *parent)
    : QObject(parent)
    , d(new LottieRuntimeServicePrivate)
{
    // 每个进程默认保留 16 个、最少 1 个、最多 16 个会话。这里只在服务创建时
    // 读取一次环境变量，运行中修改环境不会悄悄改变现有任务的资源上限。
    d->maximumRasterSessions = configuredRasterSessionCacheEntries();
    d->executor = executor ? executor : SourceExecutor(executeDefaultSourceWork);
    d->worker = new LottieRuntimeAssetWorker(d, this,
        [this](quint64 jobId, const LottieRuntimeSourceResult &result) {
            deliverJobResult(jobId, result);
        });

}

LottieRuntimeService::~LottieRuntimeService()
{
    stopForShutdown();
    delete d->frameScheduler;
    d->frameScheduler = nullptr;
    delete d->worker;
    delete d;
    d = nullptr;

    std::lock_guard<std::mutex> lock(s_serviceMutex);
    if (s_service == this) {
        s_service = nullptr;
    }
}

LottieRuntimeService *LottieRuntimeService::instance()
{
    QCoreApplication *app = QCoreApplication::instance();
    if (!app) {
        return nullptr;
    }
    installLottieRuntimeApplicationShutdownHook();
    if (app->closingDown()) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(s_serviceMutex);
    if (s_applicationStopping) {
        return nullptr;
    }
    if (!s_service) {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        s_service = new LottieRuntimeService(s_testSourceExecutor, app);
#else
        s_service = new LottieRuntimeService(SourceExecutor(), app);
#endif
    }
    return s_service;
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
bool LottieRuntimeService::hasInstance()
{
    std::lock_guard<std::mutex> lock(s_serviceMutex);
    return s_service != nullptr;
}

void LottieRuntimeService::destroyInstance()
{
    LottieRuntimeService *victim = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_serviceMutex);
        victim = s_service;
    }
    delete victim;
}
#endif

LottieRuntimeSubscription LottieRuntimeService::subscribe()
{
    QMutexLocker locker(&d->mutex);
    if (d->stopping) {
        return {};
    }

    const quint64 subscriptionId = d->nextSubscriptionId++;
    d->subscriptions.insert(subscriptionId);
    d->diagnostics.recordSubscription();
    return LottieRuntimeSubscription(this, subscriptionId);
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
bool LottieRuntimeService::requestSource(const LottieRuntimeSubscription &subscription,
                                         const QString &source, const QString &theme,
                                         SourceCompletion completion)
{
    return requestSource(subscription,
                         LottieSourceReference::fromText(source),
                         theme,
                         std::move(completion));
}
#endif

bool LottieRuntimeService::requestSource(const LottieRuntimeSubscription &subscription,
                                         const LottieSourceReference &source,
                                         const QString &theme,
                                         SourceCompletion completion)
{
    if (!subscription.isValid() || subscription.m_service != this) {
        return false;
    }
    if (!source.isValid()) {
        return false;
    }

    QMutexLocker locker(&d->mutex);
    d->diagnostics.recordSourceRequest();
    if (d->stopping || !d->subscriptions.contains(subscription.m_id)) {
        return false;
    }

    if (d->frameScheduler) {
        d->frameScheduler->unsubscribe(subscription.m_id);
    }

    // 先离开旧任务；如果没有其他等待者，旧任务会被移除或通知取消。
    d->detachSubscriptionLocked(subscription.m_id);

    // source 这里只说明“去哪里找”，还不能代表文件内容。界面线程不会读文件。
    // 写法不同但实际位置相同的来源会先统一成同一个任务键；只有任务尚未开始时
    // 才能合并。已经开始的任务不接收后来请求，确保路径内容被替换后，新请求会
    // 重新读取并得到新的内容版本。
    quint64 jobId = 0;
    const QString sourceKey = source.jobKey();
    const auto keyIt = d->jobBySourceKey.constFind(sourceKey);
    if (keyIt != d->jobBySourceKey.constEnd()) {
        const auto jobIt = d->jobs.find(keyIt.value());
        if (jobIt != d->jobs.end() && !jobIt.value().running) {
            jobId = jobIt.value().jobId;
        }
    }
    if (jobId == 0) {
        LottieRuntimeServicePrivate::SourceJob job;
        job.jobId = d->nextJobId++;
        job.sourceKey = sourceKey;
        job.source = source;
        job.cancellation = QSharedPointer<std::atomic<bool>>::create(false);
        jobId = job.jobId;
        d->jobs.insert(jobId, std::move(job));
        d->jobBySourceKey.insert(sourceKey, jobId);
        d->jobOrder.append(jobId);
    }

    LottieRuntimeServicePrivate::Observation observation;
    observation.subscriptionId = subscription.m_id;
    observation.requestedTheme = theme;
    observation.completion = std::move(completion);
    d->jobs[jobId].observations.append(std::move(observation));
    d->observedJobBySubscription.insert(subscription.m_id, jobId);

    if (!d->workerStarted) {
        d->workerStarted = true;
        d->worker->start();
    }
    d->hasWork.wakeOne();
    return true;
}

bool LottieRuntimeService::requestFrame(const LottieRuntimeSubscription &subscription,
                                        const LottieRuntimeFrameRequest &request,
                                        FrameCompletion completion)
{
    if (!subscription.isValid() || subscription.m_service != this || !completion) {
        return false;
    }
    LottieRuntimeFrameRequest canonicalRequest = request;
    if (!canonicalRequest.assetRevision.isValid() || !canonicalRequest.asset.isValid() ||
        canonicalRequest.asset.revision() != canonicalRequest.assetRevision ||
        canonicalRequest.asset.frameCount() <= 0 ||
        !canonicalRequest.asset.canUseTheme(canonicalRequest.effectiveTheme) ||
        !LottieRenderer::isRenderSizeAllowed(canonicalRequest.physicalPixelSize)) {
        return false;
    }

    canonicalRequest.effectiveTheme = request.asset.preferredTheme(request.effectiveTheme);
    canonicalRequest.frame = qBound(0,
                                    canonicalRequest.frame,
                                    canonicalRequest.asset.frameCount() - 1);

    QMutexLocker locker(&d->mutex);
    d->diagnostics.recordFrameRequest();
    if (d->stopping || !d->subscriptions.contains(subscription.m_id)) {
        return false;
    }

    if (!d->frameScheduler) {
        d->frameScheduler = new LottieRuntimeFrameScheduler(
            d->maximumRasterSessions, this);
    }
    return d->frameScheduler->request(subscription.m_id, canonicalRequest,
                                      std::move(completion));
}

void LottieRuntimeService::suspendFrame(const LottieRuntimeSubscription &subscription)
{
    if (!subscription.isValid() || subscription.m_service != this) {
        return;
    }

    QMutexLocker locker(&d->mutex);
    if (d->stopping || !d->subscriptions.contains(subscription.m_id) ||
        !d->frameScheduler) {
        return;
    }
    d->frameScheduler->suspend(subscription.m_id);
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
bool LottieRuntimeService::isStopping() const
{
    QMutexLocker locker(&d->mutex);
    return d->stopping;
}

LottieRuntimeFrameSchedulerStats LottieRuntimeService::testFrameSchedulerStats() const
{
    return frameSchedulerDiagnostics();
}

LottieRuntimeFrameSchedulerStats LottieRuntimeService::frameSchedulerDiagnostics() const
{
    if (!d->frameScheduler) {
        return {};
    }
    return d->frameScheduler->testStats();
}

qint64 LottieRuntimeService::sourceLoadingP95Nanoseconds() const
{
    QMutexLocker locker(&d->mutex);
    return d->diagnostics.loadingP95Nanoseconds();
}

void LottieRuntimeService::resetFrameSchedulerDiagnostics()
{
    if (d->frameScheduler) {
        d->frameScheduler->resetDiagnostics();
    }
}

bool LottieRuntimeService::testResetFrameSchedulerCache()
{
    return d->frameScheduler && d->frameScheduler->testResetCacheAndSession();
}

int LottieRuntimeService::testAssetWorkerCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->worker ? 1 : 0;
}

int LottieRuntimeService::testSourceCompletionCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->diagnostics.completionCount();
}

int LottieRuntimeService::testSubscriptionCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->diagnostics.subscriptionCount();
}

int LottieRuntimeService::testSourceRequestCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->diagnostics.sourceRequestCount();
}

int LottieRuntimeService::testFrameRequestCount() const
{
    QMutexLocker locker(&d->mutex);
    return d->diagnostics.frameRequestCount();
}

void LottieRuntimeService::testSetStoppingTransitionHook(quintptr hook)
{
    const auto *stoppingHook =
        reinterpret_cast<const std::function<void()> *>(hook);
    QMutexLocker locker(&d->mutex);
    d->testStoppingTransitionHook = stoppingHook ? *stoppingHook : std::function<void()>();
}

void LottieRuntimeService::testHoldFrameSchedulerGuiBatch(bool hold)
{
    if (hold && !d->frameScheduler) {
        d->frameScheduler = new LottieRuntimeFrameScheduler(
            d->maximumRasterSessions, this);
    }
    if (d->frameScheduler) {
        d->frameScheduler->testHoldGuiBatch(hold);
    }
}

void LottieRuntimeService::testFlushFrameSchedulerGuiBatch()
{
    if (d->frameScheduler) {
        d->frameScheduler->testFlushGuiBatch();
    }
}
#endif

void LottieRuntimeService::stopForShutdown()
{
    // 关闭顺序不能颠倒：先拒绝新请求、清掉组件回调并通知文件加载取消，
    // 再停止帧调度器（它会等待栅格线程退出），最后等待文件加载线程。
    // 因此本函数返回后，不会再有后台结果回调到已经销毁的 QML 对象。
    QMutexLocker shutdownLocker(&d->shutdownMutex);
    bool started = false;
    QThread *assetWorker = nullptr;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    std::function<void()> stoppingTransitionHook;
#endif
    {
        QMutexLocker locker(&d->mutex);
        if (!d->stopping) {
            d->stopping = true;
            // 通知所有加载任务取消，并立即释放组件的完成回调。后台线程仍可
            // 使用已经复制出来的来源和取消标志收尾，但不能再访问任何组件状态。
            for (auto it = d->jobs.begin(); it != d->jobs.end(); ++it) {
                if (it.value().cancellation) {
                    it.value().cancellation->store(true);
                }
            }
            d->jobOrder.clear();
            d->jobBySourceKey.clear();
            d->observedJobBySubscription.clear();
            d->subscriptions.clear();
            d->jobs.clear();
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
            stoppingTransitionHook = std::move(d->testStoppingTransitionHook);
#endif
            d->hasWork.wakeAll();
        }
        started = d->workerStarted;
        assetWorker = d->worker;
    }

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    if (stoppingTransitionHook) {
        stoppingTransitionHook();
    }
#endif

    if (d->frameScheduler) {
        d->frameScheduler->stop();
    }
    if (assetWorker && started) {
        assetWorker->wait();
    }
    if (assetWorker) {
        {
            QMutexLocker locker(&d->mutex);
            if (d->worker == assetWorker) {
                d->worker = nullptr;
                d->workerStarted = false;
            }
        }
        delete assetWorker;
    }
}

void LottieRuntimeService::unsubscribe(quint64 subscriptionId)
{
    QMutexLocker locker(&d->mutex);
    if (!d->subscriptions.remove(subscriptionId)) {
        return;
    }
    if (d->frameScheduler) {
        d->frameScheduler->unsubscribe(subscriptionId);
    }
    // 只取消当前订阅；最后一个等待者离开后，公共任务才会被移除或通知取消。
    d->detachSubscriptionLocked(subscriptionId);
}

void LottieRuntimeService::deliverJobResult(quint64 jobId, const LottieRuntimeSourceResult &result)
{
    QList<LottieRuntimeServicePrivate::Observation> observations;
    {
        QMutexLocker locker(&d->mutex);
        if (d->stopping) {
            d->removeJobLocked(jobId);
            return;
        }
        const auto it = d->jobs.constFind(jobId);
        if (it == d->jobs.constEnd()) {
            return;
        }
        observations = it.value().observations;
        d->removeJobLocked(jobId);
    }

    // 只把结果交给仍在等待这项任务的订阅者；每个订阅者再使用自己的主题选择。
    for (const LottieRuntimeServicePrivate::Observation &observation : std::as_const(observations)) {
        bool deliverable = false;
        {
            QMutexLocker locker(&d->mutex);
            deliverable = d->subscriptions.contains(observation.subscriptionId) &&
                d->observedJobBySubscription.value(observation.subscriptionId) == jobId;
            if (deliverable && observation.completion) {
                d->diagnostics.recordCompletion();
            }
        }
        if (!deliverable || !observation.completion) {
            continue;
        }
        LottieRuntimeSourceResult delivered = result;
        delivered.asset.setTheme(delivered.asset.preferredTheme(observation.requestedTheme));
        observation.completion(delivered);
    }
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
LottieRuntimeServiceTestScope::LottieRuntimeServiceTestScope(SourceExecutor executor)
{
    LottieRuntimeService::destroyInstance();

    std::lock_guard<std::mutex> lock(s_serviceMutex);
    Q_ASSERT(!s_testSourceExecutor);
    s_applicationStopping = false;
    s_testSourceExecutor = std::move(executor);
}

LottieRuntimeServiceTestScope::~LottieRuntimeServiceTestScope()
{
    LottieRuntimeService::destroyInstance();

    std::lock_guard<std::mutex> lock(s_serviceMutex);
    s_testSourceExecutor = {};
    s_applicationStopping = false;
}
#endif

} // namespace UkuiQuick
