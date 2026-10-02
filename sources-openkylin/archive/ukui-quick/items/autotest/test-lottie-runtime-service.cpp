#include <QColor>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QStringList>
#include <QTemporaryDir>
#include <QThread>
#include <QUrl>
#include <QVector>
#include <QWaitCondition>
#include <QtTest>

#include <atomic>
#include <functional>
#include <thread>
#include <utility>
#include <vector>

#include "lottie-runtime-test-helpers_p.h"
#include "lottie-runtime-service_p.h"
#include "ukui/lottie/dot-lottie-parser.h"
#include "ukui/lottie/lottie-cache-manager.h"

using namespace UkuiQuick;

namespace {

constexpr int kShutdownBarrierTimeoutMilliseconds = 5000;

class ScopedEnvironmentVariable
{
public:
    ScopedEnvironmentVariable(const char *name, const QByteArray &value)
        : m_name(name)
        , m_previousValue(qgetenv(name))
        , m_wasSet(qEnvironmentVariableIsSet(name))
    {
        qputenv(m_name.constData(), value);
    }

    ~ScopedEnvironmentVariable()
    {
        if (m_wasSet) {
            qputenv(m_name.constData(), m_previousValue);
        } else {
            qunsetenv(m_name.constData());
        }
    }

private:
    QByteArray m_name;
    QByteArray m_previousValue;
    bool m_wasSet = false;
};

class GatedSourceExecutor
{
public:
    LottieRuntimeSourceResult run(const LottieRuntimeSourceRequest &request,
                                  const LottieRuntimeService::SourceCancellation &isCancelled)
    {
        {
            QMutexLocker locker(&m_dataMutex);
            m_startedSources.append(request.source.canonicalLocator());
        }

        LottieRuntimeSourceResult outcome;
        bool released = false;
        {
            QMutexLocker locker(&m_gateMutex);
            while (!m_released && !(isCancelled && isCancelled())) {
                m_gate.wait(&m_gateMutex, 20);
            }
            if (m_released) {
                m_released = false;
                outcome = m_pendingOutcome;
                released = true;
            }
        }

        {
            QMutexLocker locker(&m_dataMutex);
            if (released) {
                ++m_completedCount;
            } else {
                ++m_cancelledCount;
            }
        }
        return outcome;
    }

    void releaseOnce(const LottieRuntimeSourceResult &outcome)
    {
        QMutexLocker locker(&m_gateMutex);
        m_pendingOutcome = outcome;
        m_released = true;
        m_gate.wakeAll();
    }

    QStringList startedSources() const
    {
        QMutexLocker locker(&m_dataMutex);
        return m_startedSources;
    }

    int completedCount() const
    {
        QMutexLocker locker(&m_dataMutex);
        return m_completedCount;
    }

    int cancelledCount() const
    {
        QMutexLocker locker(&m_dataMutex);
        return m_cancelledCount;
    }

private:
    mutable QMutex m_dataMutex;
    QStringList m_startedSources;
    int m_completedCount = 0;
    int m_cancelledCount = 0;

    QMutex m_gateMutex;
    QWaitCondition m_gate;
    LottieRuntimeSourceResult m_pendingOutcome;
    bool m_released = false;
};

class ShutdownBarrierSourceExecutor
{
public:
    explicit ShutdownBarrierSourceExecutor(bool releaseOnCancellation = false)
        : m_releaseOnCancellation(releaseOnCancellation)
    {
    }

    LottieRuntimeSourceResult run(
        const LottieRuntimeSourceRequest &request,
        const LottieRuntimeService::SourceCancellation &isCancelled)
    {
        QMutexLocker locker(&m_mutex);
        m_startedSources.append(request.source.canonicalLocator());
        ++m_activeCount;
        m_startedCondition.wakeAll();

        QElapsedTimer timer;
        timer.start();
        while (!m_released) {
            if (!m_cancellationObserved && isCancelled && isCancelled()) {
                m_cancellationObserved = true;
                ++m_cancelledCount;
                m_cancellationCondition.wakeAll();
                if (m_releaseOnCancellation) {
                    m_released = true;
                }
            }
            if (!m_released) {
                const int remaining = kShutdownBarrierTimeoutMilliseconds - timer.elapsed();
                if (remaining <= 0) {
                    m_timedOut = true;
                    m_released = true;
                    m_releaseCondition.wakeAll();
                    break;
                }
                m_releaseCondition.wait(locker.mutex(), qMin(remaining, 10));
            }
        }

        LottieRuntimeSourceResult result;
        if (!m_cancellationObserved) {
            result.loaded = true;
            result.asset.setAnimationJsonPath(request.source.canonicalLocator());
        }
        --m_activeCount;
        ++m_completedCount;
        return result;
    }

    bool waitUntilStarted(const QString &source)
    {
        QMutexLocker locker(&m_mutex);
        QElapsedTimer timer;
        timer.start();
        while (!m_startedSources.contains(source)) {
            const int remaining = kShutdownBarrierTimeoutMilliseconds - timer.elapsed();
            if (remaining <= 0 || !m_startedCondition.wait(locker.mutex(), remaining)) {
                return m_startedSources.contains(source);
            }
        }
        return true;
    }

    bool waitUntilCancellationObserved()
    {
        QMutexLocker locker(&m_mutex);
        QElapsedTimer timer;
        timer.start();
        while (!m_cancellationObserved) {
            const int remaining = kShutdownBarrierTimeoutMilliseconds - timer.elapsed();
            if (remaining <= 0 || !m_cancellationCondition.wait(locker.mutex(), remaining)) {
                return m_cancellationObserved;
            }
        }
        return true;
    }

    void release()
    {
        QMutexLocker locker(&m_mutex);
        m_released = true;
        m_releaseCondition.wakeAll();
    }

    QStringList startedSources() const
    {
        QMutexLocker locker(&m_mutex);
        return m_startedSources;
    }

    int cancelledCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_cancelledCount;
    }

    int activeCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_activeCount;
    }

    int completedCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_completedCount;
    }

    bool timedOut() const
    {
        QMutexLocker locker(&m_mutex);
        return m_timedOut;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_startedCondition;
    QWaitCondition m_cancellationCondition;
    QWaitCondition m_releaseCondition;
    QStringList m_startedSources;
    int m_cancelledCount = 0;
    int m_activeCount = 0;
    int m_completedCount = 0;
    bool m_cancellationObserved = false;
    bool m_released = false;
    bool m_releaseOnCancellation = false;
    bool m_timedOut = false;
};

class RecordingSourceExecutor
{
public:
    LottieRuntimeSourceResult run(const LottieRuntimeSourceRequest &request,
                                  const LottieRuntimeService::SourceCancellation &)
    {
        QMutexLocker locker(&m_mutex);
        m_startedSources.append(request.source.canonicalLocator());
        m_executionThreads.append(QThread::currentThread());

        LottieRuntimeSourceResult result;
        result.loaded = !request.source.canonicalLocator().endsWith(QStringLiteral(".missing"));
        if (result.loaded) {
            result.asset.setAnimationJsonPath(request.source.canonicalLocator());
        }
        return result;
    }

    QStringList startedSources() const
    {
        QMutexLocker locker(&m_mutex);
        return m_startedSources;
    }

    QList<QThread *> executionThreads() const
    {
        QMutexLocker locker(&m_mutex);
        return m_executionThreads;
    }

private:
    mutable QMutex m_mutex;
    QStringList m_startedSources;
    QList<QThread *> m_executionThreads;
};

/**
 * 模拟 worker 侧先物化 snapshot 的受控执行器：开始执行时读取 source 当前内容
 * （内容摘要作为结果身份标记），随后挂起等待释放或协作取消。
 */
class SnapshotAtEntryExecutor
{
public:
    LottieRuntimeSourceResult run(const LottieRuntimeSourceRequest &request,
                                  const LottieRuntimeService::SourceCancellation &isCancelled)
    {
        QByteArray content;
        {
            QFile file(request.source.canonicalLocator());
            if (file.open(QIODevice::ReadOnly)) {
                content = file.readAll();
            }
        }

        bool released = false;
        {
            QMutexLocker locker(&m_mutex);
            m_startedSources.append(request.source.canonicalLocator());
            while (!m_released && !(isCancelled && isCancelled())) {
                m_gate.wait(&m_mutex, 20);
            }
            released = m_released;
            m_released = false;
        }

        LottieRuntimeSourceResult outcome;
        if (!released) {
            QMutexLocker locker(&m_mutex);
            ++m_cancelledCount;
            return outcome;
        }
        outcome.loaded = true;
        outcome.asset.setAnimationJsonPath(
            QString::fromLatin1(QCryptographicHash::hash(content, QCryptographicHash::Sha256).toHex()));
        return outcome;
    }

    void releaseOnce()
    {
        QMutexLocker locker(&m_mutex);
        m_released = true;
        m_gate.wakeAll();
    }

    QStringList startedSources() const
    {
        QMutexLocker locker(&m_mutex);
        return m_startedSources;
    }

    int cancelledCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_cancelledCount;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_gate;
    QStringList m_startedSources;
    int m_cancelledCount = 0;
    bool m_released = false;
};

class FaultInjectingRasterSession final : public LottieRasterSession
{
public:
    FaultInjectingRasterSession(bool failFirstRender,
                                QMutex *mutex,
                                int *renderCount,
                                QList<QThread *> *renderThreads,
                                QList<QThread *> *destructionThreads)
        : m_failFirstRender(failFirstRender)
        , m_mutex(mutex)
        , m_renderCount(renderCount)
        , m_renderThreads(renderThreads)
        , m_destructionThreads(destructionThreads)
    {
    }

    ~FaultInjectingRasterSession() override
    {
        QMutexLocker locker(m_mutex);
        m_destructionThreads->append(QThread::currentThread());
    }

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        {
            QMutexLocker locker(m_mutex);
            ++*m_renderCount;
            m_renderThreads->append(QThread::currentThread());
        }

        if (m_failFirstRender) {
            m_failFirstRender = false;
            return LottieRuntimeFrameResult::Failure::Raster;
        }

        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::green);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    bool m_failFirstRender = false;
    QMutex *m_mutex = nullptr;
    int *m_renderCount = nullptr;
    QList<QThread *> *m_renderThreads = nullptr;
    QList<QThread *> *m_destructionThreads = nullptr;
};

class CountingRasterSession final : public LottieRasterSession
{
public:
    explicit CountingRasterSession(std::atomic<int> *renderCount)
        : m_renderCount(renderCount)
    {
    }

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        ++*m_renderCount;
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::green);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    std::atomic<int> *m_renderCount = nullptr;
};

class IdentityBoundRasterSessionController;

class IdentityBoundRasterSession final : public LottieRasterSession
{
public:
    explicit IdentityBoundRasterSession(IdentityBoundRasterSessionController *controller)
        : m_controller(controller)
    {
    }

    ~IdentityBoundRasterSession() override;

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override;

private:
    bool matches(const LottieRuntimeFrameRequest &request) const
    {
        return m_revision == request.assetRevision &&
            m_animationPath == request.asset.animationJsonPath() &&
            m_resourceDir == request.asset.resourceDir() &&
            m_extractDir == request.asset.extractDir() &&
            m_resourceSearchDirs == request.asset.resourceSearchDirs() &&
            m_theme == request.effectiveTheme;
    }

    IdentityBoundRasterSessionController *m_controller = nullptr;
    LottieAssetRevision m_revision;
    QString m_animationPath;
    QString m_resourceDir;
    QString m_extractDir;
    QStringList m_resourceSearchDirs;
    QString m_theme;
    bool m_bound = false;
};

class IdentityBoundRasterSessionController
{
public:
    std::unique_ptr<LottieRasterSession> create()
    {
        QMutexLocker locker(&m_mutex);
        ++m_sessionCount;
        ++m_liveSessionCount;
        m_maximumLiveSessionCount = qMax(
            m_maximumLiveSessionCount, m_liveSessionCount);
        return std::make_unique<IdentityBoundRasterSession>(this);
    }

    void recordRender()
    {
        QMutexLocker locker(&m_mutex);
        ++m_renderCount;
    }

    void recordLoad(const LottieAssetRevision &revision)
    {
        QMutexLocker locker(&m_mutex);
        ++m_loadCount;
        m_loadRevisions.append(revision);
    }

    void recordMismatch()
    {
        QMutexLocker locker(&m_mutex);
        ++m_mismatchCount;
    }

    void failOnceForSize(const QSize &size)
    {
        QMutexLocker locker(&m_mutex);
        m_failureSize = size;
        m_failureConsumed = false;
    }

    bool shouldFail(const QSize &size)
    {
        QMutexLocker locker(&m_mutex);
        if (m_failureConsumed || size != m_failureSize) {
            return false;
        }
        m_failureConsumed = true;
        return true;
    }

    void recordDestruction()
    {
        QMutexLocker locker(&m_mutex);
        Q_ASSERT(m_liveSessionCount > 0);
        --m_liveSessionCount;
        ++m_destructionCount;
    }

    int sessionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sessionCount;
    }

    int renderCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderCount;
    }

    int loadCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_loadCount;
    }

    QList<LottieAssetRevision> loadRevisions() const
    {
        QMutexLocker locker(&m_mutex);
        return m_loadRevisions;
    }

    int mismatchCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_mismatchCount;
    }

    int destructionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_destructionCount;
    }

    int maximumLiveSessionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_maximumLiveSessionCount;
    }

private:
    mutable QMutex m_mutex;
    int m_sessionCount = 0;
    int m_renderCount = 0;
    int m_loadCount = 0;
    QList<LottieAssetRevision> m_loadRevisions;
    int m_mismatchCount = 0;
    int m_destructionCount = 0;
    int m_liveSessionCount = 0;
    int m_maximumLiveSessionCount = 0;
    QSize m_failureSize;
    bool m_failureConsumed = false;
};

IdentityBoundRasterSession::~IdentityBoundRasterSession()
{
    m_controller->recordDestruction();
}

LottieRuntimeFrameResult::Failure IdentityBoundRasterSession::render(
    const LottieRuntimeFrameRequest &request, QImage *image)
{
    if (!image) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }

    if (!m_bound) {
        m_revision = request.assetRevision;
        m_animationPath = request.asset.animationJsonPath();
        m_resourceDir = request.asset.resourceDir();
        m_extractDir = request.asset.extractDir();
        m_resourceSearchDirs = request.asset.resourceSearchDirs();
        m_theme = request.effectiveTheme;
        m_bound = true;
        m_controller->recordLoad(request.assetRevision);
    } else if (!matches(request)) {
        m_controller->recordMismatch();
        return LottieRuntimeFrameResult::Failure::Raster;
    }

    m_controller->recordRender();
    if (m_controller->shouldFail(request.physicalPixelSize)) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }
    *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::green);
    return LottieRuntimeFrameResult::Failure::None;
}

class BlockingRasterSessionController;

class BlockingRasterSession final : public LottieRasterSession
{
public:
    explicit BlockingRasterSession(BlockingRasterSessionController *controller)
        : m_controller(controller)
    {
    }

    ~BlockingRasterSession() override;

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override;

private:
    BlockingRasterSessionController *m_controller = nullptr;
};

class BlockingRasterSessionController
{
public:
    std::unique_ptr<LottieRasterSession> create()
    {
        return std::make_unique<BlockingRasterSession>(this);
    }

    bool waitUntilStarted()
    {
        QMutexLocker locker(&m_mutex);
        QElapsedTimer timer;
        timer.start();
        while (!m_started) {
            const int remaining = kShutdownBarrierTimeoutMilliseconds -
                static_cast<int>(timer.elapsed());
            if (remaining <= 0 || !m_startedCondition.wait(&m_mutex, remaining)) {
                break;
            }
        }
        return m_started;
    }

    void enableStepMode()
    {
        QMutexLocker locker(&m_mutex);
        m_stepMode = true;
        m_releasePermits = 0;
        m_released = false;
    }

    void releaseOne()
    {
        QMutexLocker locker(&m_mutex);
        ++m_releasePermits;
        m_releasedCondition.wakeAll();
    }

    void release()
    {
        QMutexLocker locker(&m_mutex);
        m_released = true;
        m_releasedCondition.wakeAll();
    }

    void setReleasePredicate(std::function<bool()> predicate)
    {
        QMutexLocker locker(&m_mutex);
        m_releasePredicate = std::move(predicate);
    }

    void blockNextRender()
    {
        QMutexLocker locker(&m_mutex);
        m_started = false;
        m_released = false;
    }

    void render(const QSize &size, QImage *image)
    {
        QMutexLocker locker(&m_mutex);
        m_started = true;
        m_startedSizes.append(size);
        m_startedCondition.wakeAll();
        QElapsedTimer releaseTimer;
        releaseTimer.start();
        while (!m_released && (!m_stepMode || m_releasePermits == 0)) {
            if (m_releasePredicate && m_releasePredicate()) {
                m_released = true;
                break;
            }
            const int remaining = kShutdownBarrierTimeoutMilliseconds -
                static_cast<int>(releaseTimer.elapsed());
            if (remaining <= 0 || !m_releasedCondition.wait(&m_mutex, qMin(1, remaining))) {
                m_released = true;
                break;
            }
        }
        if (m_stepMode && m_releasePermits > 0) {
            --m_releasePermits;
        }
        m_renderedSizes.append(size);
        m_renderThreads.append(QThread::currentThread());
        if (m_renderedSizes.size() == 1 && m_firstRenderFails) {
            return;
        }
        *image = QImage(size, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::green);
    }

    void setFirstRenderFails(bool fails)
    {
        QMutexLocker locker(&m_mutex);
        m_firstRenderFails = fails;
    }

    QList<QSize> renderedSizes() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderedSizes;
    }

    QList<QSize> startedSizes() const
    {
        QMutexLocker locker(&m_mutex);
        return m_startedSizes;
    }

    void destroyed()
    {
        QMutexLocker locker(&m_mutex);
        m_destructionThreads.append(QThread::currentThread());
    }

    QList<QThread *> renderThreads() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderThreads;
    }

    QList<QThread *> destructionThreads() const
    {
        QMutexLocker locker(&m_mutex);
        return m_destructionThreads;
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_startedCondition;
    QWaitCondition m_releasedCondition;
    QList<QSize> m_renderedSizes;
    QList<QSize> m_startedSizes;
    QList<QThread *> m_renderThreads;
    QList<QThread *> m_destructionThreads;
    std::function<bool()> m_releasePredicate;
    bool m_started = false;
    bool m_released = false;
    bool m_stepMode = false;
    int m_releasePermits = 0;
    bool m_firstRenderFails = false;
};

class ScopedRasterControllerRelease
{
public:
    explicit ScopedRasterControllerRelease(BlockingRasterSessionController *controller)
        : m_controller(controller)
    {
    }

    ~ScopedRasterControllerRelease()
    {
        release();
    }

    void arm()
    {
        m_armed = true;
    }

    void release()
    {
        if (m_armed && m_controller) {
            m_controller->release();
            m_armed = false;
        }
    }

    ScopedRasterControllerRelease(const ScopedRasterControllerRelease &) = delete;
    ScopedRasterControllerRelease &operator=(const ScopedRasterControllerRelease &) = delete;

private:
    BlockingRasterSessionController *m_controller = nullptr;
    bool m_armed = true;
};

BlockingRasterSession::~BlockingRasterSession()
{
    m_controller->destroyed();
}

LottieRuntimeFrameResult::Failure BlockingRasterSession::render(
    const LottieRuntimeFrameRequest &request, QImage *image)
{
    if (!image) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }
    m_controller->render(request.physicalPixelSize, image);
    return image->isNull() ? LottieRuntimeFrameResult::Failure::Raster
                           : LottieRuntimeFrameResult::Failure::None;
}

class RasterSessionRecorder
{
public:
    std::unique_ptr<LottieRasterSession> create()
    {
        QMutexLocker locker(&m_mutex);
        const bool failFirstRender = m_sessionCount++ == 0;
        return std::make_unique<FaultInjectingRasterSession>(
            failFirstRender, &m_mutex, &m_renderCount, &m_renderThreads,
            &m_destructionThreads);
    }

    int sessionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sessionCount;
    }

    int renderCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderCount;
    }

    QList<QThread *> renderThreads() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderThreads;
    }

    QList<QThread *> destructionThreads() const
    {
        QMutexLocker locker(&m_mutex);
        return m_destructionThreads;
    }

private:
    mutable QMutex m_mutex;
    int m_sessionCount = 0;
    int m_renderCount = 0;
    QList<QThread *> m_renderThreads;
    QList<QThread *> m_destructionThreads;
};

class ClassifiedFailureSessionController;

class ClassifiedFailureRasterSession final : public LottieRasterSession
{
public:
    explicit ClassifiedFailureRasterSession(
        ClassifiedFailureSessionController *controller)
        : m_controller(controller)
    {
    }

    ~ClassifiedFailureRasterSession() override;

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override;

private:
    ClassifiedFailureSessionController *m_controller = nullptr;
};

class ClassifiedFailureSessionController
{
public:
    explicit ClassifiedFailureSessionController(
        QList<LottieRuntimeFrameResult::Failure> failures)
        : m_failures(std::move(failures))
    {
    }

    std::unique_ptr<LottieRasterSession> create()
    {
        QMutexLocker locker(&m_mutex);
        ++m_sessionCount;
        return std::make_unique<ClassifiedFailureRasterSession>(this);
    }

    LottieRuntimeFrameResult::Failure render(const QSize &size, QImage *image)
    {
        QMutexLocker locker(&m_mutex);
        ++m_renderCount;
        const auto failure = m_failures.isEmpty()
            ? LottieRuntimeFrameResult::Failure::None
            : m_failures.takeFirst();
        if (failure == LottieRuntimeFrameResult::Failure::None && image) {
            *image = QImage(size, QImage::Format_ARGB32_Premultiplied);
            image->fill(Qt::green);
        }
        return failure;
    }

    void destroyed()
    {
        QMutexLocker locker(&m_mutex);
        ++m_destructionCount;
    }

    int sessionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_sessionCount;
    }

    int renderCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_renderCount;
    }

    int destructionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_destructionCount;
    }

private:
    mutable QMutex m_mutex;
    QList<LottieRuntimeFrameResult::Failure> m_failures;
    int m_sessionCount = 0;
    int m_renderCount = 0;
    int m_destructionCount = 0;
};

ClassifiedFailureRasterSession::~ClassifiedFailureRasterSession()
{
    m_controller->destroyed();
}

LottieRuntimeFrameResult::Failure ClassifiedFailureRasterSession::render(
    const LottieRuntimeFrameRequest &request, QImage *image)
{
    return m_controller->render(request.physicalPixelSize, image);
}

LottieRuntimeSourceResult loadedOutcome(const QString &source)
{
    LottieRuntimeSourceResult outcome;
    outcome.loaded = true;
    outcome.asset.setAnimationJsonPath(source);
    return outcome;
}

LottieRuntimeSourceResult failedOutcome()
{
    return {};
}

LottieRuntimeFrameRequest frameRequestFor(const LottieRuntimeSourceResult &result,
                                          int frame, const QSize &physicalPixelSize,
                                          const QString &effectiveTheme)
{
    LottieRuntimeFrameRequest request;
    request.assetRevision = result.asset.revision();
    request.asset = result.asset;
    request.effectiveTheme = effectiveTheme;
    request.frame = frame;
    request.physicalPixelSize = physicalPixelSize;
    return request;
}

bool writeBytes(const QString &path, const QByteArray &bytes);

QByteArray animationJson(int endFrame = 2)
{
    static constexpr char animation[] = R"({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"runtime-service-test",
        "ddd":0,
        "assets":[],
        "layers":[{
            "ddd":0,
            "ind":1,
            "ty":4,
            "nm":"Shape Layer 1",
            "sr":1,
            "ks":{
                "o":{"a":0,"k":100},
                "r":{"a":0,"k":0},
                "p":{"a":0,"k":[8,8,0]},
                "a":{"a":0,"k":[0,0]},
                "s":{"a":0,"k":[100,100,100]}
            },
            "ao":0,
            "shapes":[
                {"ty":"rc","d":1,"s":{"a":0,"k":[16,16]},"p":{"a":0,"k":[0,0]},"r":0,"nm":"Rectangle Path 1"},
                {"ty":"fl","c":{"a":0,"k":[1,0,0,1]},"o":{"a":0,"k":100},"r":1,"bm":0,"nm":"Fill 1"},
                {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":0,"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}
            ],
            "ip":0,
            "op":2,
            "st":0,
            "bm":0
        }]
    })";
    QByteArray contents(animation, sizeof(animation) - 1);
    return contents.replace(QByteArrayLiteral("\"op\":2"),
                            QByteArrayLiteral("\"op\":") + QByteArray::number(endFrame));
}

QString writeAnimation(QTemporaryDir *directory, const QString &fileName, int endFrame = 2)
{
    const QString source = directory->filePath(fileName);
    QFile file(source);
    const QByteArray animation = animationJson(endFrame);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        file.write(animation) != animation.size()) {
        return {};
    }
    return source;
}

QVector<LottieRuntimeSourceResult> loadAnimationSources(
    QTemporaryDir *directory, const QString &filePrefix, int count)
{
    QVector<LottieRuntimeSourceResult> results;
    results.reserve(count);
    for (int index = 0; index < count; ++index) {
        const QString source = writeAnimation(
            directory,
            QStringLiteral("%1-%2.json").arg(filePrefix).arg(index),
            index + 3);
        if (source.isEmpty()) {
            return {};
        }
        LottieRuntimeSourceResult result;
        result.asset = LottieCacheManager::instance()->requestAsset(
            LottieSourceReference::fromText(source)).asset;
        result.loaded = result.asset.isValid();
        if (!result.loaded) {
            return {};
        }
        results.append(result);
    }
    return results;
}

bool writeBytes(const QString &path, const QByteArray &bytes)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(bytes) == bytes.size();
}

// 与平台测试一致的最小 stored zip 构造器，用于生成真实 .lottie 归档。
struct ZipEntry
{
    QByteArray name;
    QByteArray contents;
    quint32 localHeaderOffset = 0;
};

void appendLittleEndian16(QByteArray &data, quint16 value)
{
    data.append(static_cast<char>(value & 0xff));
    data.append(static_cast<char>((value >> 8) & 0xff));
}

void appendLittleEndian32(QByteArray &data, quint32 value)
{
    data.append(static_cast<char>(value & 0xff));
    data.append(static_cast<char>((value >> 8) & 0xff));
    data.append(static_cast<char>((value >> 16) & 0xff));
    data.append(static_cast<char>((value >> 24) & 0xff));
}

quint32 crc32(const QByteArray &data)
{
    quint32 checksum = 0xffffffffU;
    for (const unsigned char byte : data) {
        checksum ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            checksum = (checksum >> 1) ^ (0xedb88320U & (-(checksum & 1U)));
        }
    }
    return ~checksum;
}

QByteArray storedZip(const QList<ZipEntry> &entries)
{
    QByteArray archive;
    QList<ZipEntry> mutableEntries = entries;
    for (ZipEntry &entry : mutableEntries) {
        entry.localHeaderOffset = static_cast<quint32>(archive.size());
        const quint32 checksum = crc32(entry.contents);
        appendLittleEndian32(archive, 0x04034b50U);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, checksum);
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian16(archive, static_cast<quint16>(entry.name.size()));
        appendLittleEndian16(archive, 0);
        archive.append(entry.name);
        archive.append(entry.contents);
    }

    const quint32 centralDirectoryOffset = static_cast<quint32>(archive.size());
    for (const ZipEntry &entry : std::as_const(mutableEntries)) {
        const quint32 checksum = crc32(entry.contents);
        appendLittleEndian32(archive, 0x02014b50U);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, checksum);
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian16(archive, static_cast<quint16>(entry.name.size()));
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, 0);
        appendLittleEndian32(archive, entry.localHeaderOffset);
        archive.append(entry.name);
    }

    const quint32 centralDirectorySize = static_cast<quint32>(archive.size()) - centralDirectoryOffset;
    appendLittleEndian32(archive, 0x06054b50U);
    appendLittleEndian16(archive, 0);
    appendLittleEndian16(archive, 0);
    appendLittleEndian16(archive, static_cast<quint16>(mutableEntries.size()));
    appendLittleEndian16(archive, static_cast<quint16>(mutableEntries.size()));
    appendLittleEndian32(archive, centralDirectorySize);
    appendLittleEndian32(archive, centralDirectoryOffset);
    appendLittleEndian16(archive, 0);
    return archive;
}

QByteArray themedArchiveManifest(const QByteArray &animationId, int themeCount)
{
    QByteArray themes;
    for (int index = 0; index < themeCount; ++index) {
        if (!themes.isEmpty()) {
            themes.append(',');
        }
        themes.append("\"theme-" + QByteArray::number(index) + "\"");
    }
    return QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"") + animationId +
        QByteArrayLiteral("\"},\"animations\":[{\"id\":\"") + animationId +
        QByteArrayLiteral("\"}],\"themes\":[") + themes + QByteArrayLiteral("]}");
}

QByteArray dotLottieArchive(const QByteArray &animationId,
                            const QByteArray &animation,
                            const QByteArray &manifest = QByteArray())
{
    const QByteArray manifestBytes = manifest.isEmpty()
        ? QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"") + animationId +
              QByteArrayLiteral("\"},\"animations\":[{\"id\":\"") + animationId +
              QByteArrayLiteral("\"}]}")
        : manifest;
    return storedZip({
        {QByteArrayLiteral("manifest.json"), manifestBytes},
        {QByteArrayLiteral("a/") + animationId + QByteArrayLiteral(".json"), animation}
    });
}

QString writeArchive(QTemporaryDir *directory,
                     const QString &fileName,
                     const QByteArray &animationId,
                     const QByteArray &animation,
                     const QByteArray &manifest = QByteArray())
{
    const QString path = directory->filePath(fileName);
    return writeBytes(path, dotLottieArchive(animationId, animation, manifest)) ? path : QString();
}

// 测试动画 JSON：fr=30, ip=0, op=2 → 2 帧。
QByteArray metadataAnimationJson(int framesPerSecond, int inPoint, int outPoint)
{
    return QByteArrayLiteral("{\"v\":\"5.7.4\",\"fr\":") + QByteArray::number(framesPerSecond) +
        QByteArrayLiteral(",\"ip\":") + QByteArray::number(inPoint) +
        QByteArrayLiteral(",\"op\":") + QByteArray::number(outPoint) +
        QByteArrayLiteral(",\"w\":16,\"h\":16,\"nm\":\"runtime\",\"ddd\":0,\"assets\":[],") +
        QByteArrayLiteral("\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":4,\"nm\":\"l\",\"sr\":1,") +
        QByteArrayLiteral("\"ks\":{\"o\":{\"a\":0,\"k\":100},\"r\":{\"a\":0,\"k\":0},") +
        QByteArrayLiteral("\"p\":{\"a\":0,\"k\":[8,8,0]},\"a\":{\"a\":0,\"k\":[0,0]},") +
        QByteArrayLiteral("\"s\":{\"a\":0,\"k\":[100,100,100]}},\"ao\":0,\"shapes\":") +
        QByteArrayLiteral("[{\"ty\":\"fl\",\"c\":{\"a\":0,\"k\":[1,0,0,1]},\"o\":{\"a\":0,\"k\":100},") +
        QByteArrayLiteral("\"r\":1,\"bm\":0,\"nm\":\"f\"}],\"ip\":") + QByteArray::number(inPoint) +
        QByteArrayLiteral(",\"op\":") + QByteArray::number(outPoint) +
        QByteArrayLiteral(",\"st\":0,\"bm\":0}]}");
}

QByteArray themedAnimationJson()
{
    return QByteArray(R"json({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"runtime-theme-test",
        "ddd":0,
        "slots":{"fillColor":{"p":{"a":0,"k":[1,0,0,1]}}},
        "assets":[],
        "layers":[{
            "ddd":0,
            "ind":1,
            "ty":4,
            "nm":"Shape Layer 1",
            "sr":1,
            "ks":{"o":{"a":0,"k":100},"r":{"a":0,"k":0},"p":{"a":0,"k":[8,8,0]},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,100,100]}},
            "ao":0,
            "shapes":[
                {"ty":"rc","d":1,"s":{"a":0,"k":[16,16]},"p":{"a":0,"k":[0,0]},"r":{"a":0,"k":0},"nm":"Rectangle Path 1"},
                {"ty":"fl","c":{"a":0,"k":[1,0,0,1],"sid":"fillColor"},"o":{"a":0,"k":100},"r":1,"bm":0,"nm":"Fill 1"},
                {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}
            ],
            "ip":0,
            "op":2,
            "st":0,
            "bm":0
        }]
    })json");
}

QByteArray themeSlotJson(const QByteArray &color)
{
    return QByteArrayLiteral("{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":") +
        color + QByteArrayLiteral("}]}");
}

QString writeThemedArchive(
    QTemporaryDir *directory,
    const QByteArray &animationName = QByteArrayLiteral("runtime-theme-test"))
{
    const QByteArray manifest = QByteArrayLiteral(
        "{\"version\":\"2\",\"initial\":{\"animation\":\"main\",\"theme\":\"light\"},") +
        QByteArrayLiteral("\"animations\":[{\"id\":\"main\",\"themes\":[\"light\",\"dark\"]}],") +
        QByteArrayLiteral("\"themes\":[{\"id\":\"light\"},{\"id\":\"dark\"}]}");
    QByteArray animation = themedAnimationJson();
    animation.replace(QByteArrayLiteral("runtime-theme-test"), animationName);
    const QByteArray archive = storedZip({
        {QByteArrayLiteral("manifest.json"), manifest},
        {QByteArrayLiteral("a/main.json"), animation},
        {QByteArrayLiteral("t/light.json"), themeSlotJson(QByteArrayLiteral("[0,0,1,1]"))},
        {QByteArrayLiteral("t/dark.json"), themeSlotJson(QByteArrayLiteral("[0,1,0,1]"))}
    });
    const QString source = directory->filePath(QStringLiteral("runtime-theme.lottie"));
    return writeBytes(source, archive) ? source : QString();
}

} // namespace

class lottie_runtime_service_test : public QObject
{
    Q_OBJECT

private slots:
    void serviceIsCreatedLazilyOnceAndOwnedByTheApplication();
    void assetWorkRunsSeriallyAndDeliversOnTheApplicationThread();
    void newerRequestReplacesQueuedWorkOfItsSubscription();
    void supersededInFlightResultIsNotDelivered();
    void supersededCountsOnlyOutstandingReplacements();
    void destroyingSubscriptionRemovesQueuedWorkAndDropsInFlightResults();
    void stoppingRejectsNewWorkAndDropsUndeliveredResults();
    void shutdownReleasesAssetObservationsBeforeJoiningActiveWork();
    void subscriptionDestructionOverlapsStoppingWindow();
    void aboutToQuitCannotResurrectRuntime();
    void shutdownDropsCombinedAssetRasterGuiAndHostWork();
    void boundedStressMatrixCovers16To56DistinctKeys();
    void sameSourceIsLoadedOnceForAllSubscribers();
    void queuedCanonicalSourceAliasesShareOneJob_data();
    void queuedCanonicalSourceAliasesShareOneJob();
    void relativeSourceBasesRemainDistinctQueuedJobs();
    void runningCanonicalSourceJobDoesNotAcceptAliases();
    void oneSubscriberLeavingDoesNotCancelSharedLoad();
    void lastObserverLeavingCancelsSharedLoad();
    void switchingSourceDetachesWithoutCancellingRemainingObservers();
    void queuedSharedJobSurvivesUntilItsLastObserverLeaves();
    void runningJobIsNotJoinedAfterSourceContentReplaced();
    void shuttingDownHonoursContractOnActiveJsonPath();
    void assetFailureIsContainedToTheRequest();
    void completionCanReleaseItsSubscription();
    void testScopeRestoresACleanRuntime();
    void defaultExecutorLoadsRealSourcesSerially();
    void defaultExecutorReportsSourceWorkLatencySeparately();
    void parserExecutorUnzipCancellationIsReaped();
    void defaultExecutorCarriesContentDerivedRevisions();
    void defaultExecutorPublishesLoadingMetadataBeforeFrames();
    void defaultExecutorRejectsBudgetAndThemeOverflow();
    void rasterRequestUsesRevisionClampedFrameAndPhysicalPixels();
    void equalRenderKeysCoalesceAcrossSubscriptions();
    void boundedFrameQueueDoesNotLetDeferredChurnStarveOlderJobs();
    void unsubscribingQueuedFrameDropsJob();
    void retainsConcurrentTinyFrameWorkingSet();
    void preservesByteBudget();
    void frameCacheDoesNotRetainSourceAssetPayload();
    void boundsTinyFrameMetadataAt512();
    void mixedCacheAndWorkerDeliveryRemainsBatched();
    void latestFrameRequestWinsAndStaleFailureIsIgnored();
    void keepsDemandedSessionsResidentWithinCapacity();
    void overflowDoesNotPolluteResidentSessions();
    void capacityOneUsesOnlyTransientSessions();
    void sharesSessionAcrossHosts();
    void sessionDemandTracksRenderability_data();
    void sessionDemandTracksRenderability();
    void failedSessionDoesNotEvictHealthyResidents();
    void rasterSessionLimitUsesDemandBound_data();
    void rasterSessionLimitUsesDemandBound();
    void rasterSessionLimitIsReadWhenServiceIsCreated();
    void rasterFailureEvictsOnlyMatchingSession();
    void rasterFailureDiscardsSessionUntilLaterRequest();
    void deterministicThemeFailureIsRememberedByRevision();
    void staleDeterministicThemeFailureDoesNotPoisonRevision();
    void structuredFailuresDiscardSessionsAndCacheOnlyDeterministicThemes();
    void destroyingSubscriptionDropsQueuedRasterResult();
    void rasterRequestRendersDotLottieThemeSlot();
};

void lottie_runtime_service_test::serviceIsCreatedLazilyOnceAndOwnedByTheApplication()
{
    LottieRuntimeServiceTestScope scope(
        [](const LottieRuntimeSourceRequest &, const LottieRuntimeService::SourceCancellation &) {
            return LottieRuntimeSourceResult();
        });

    QVERIFY(!LottieRuntimeService::hasInstance());
    auto *first = LottieRuntimeService::instance();
    QVERIFY(first);
    QCOMPARE(LottieRuntimeService::instance(), first);
    QCOMPARE(LottieRuntimeService::hasInstance(), true);
    QCOMPARE(first->parent(), QCoreApplication::instance());
    QVERIFY(!first->isStopping());
}

void lottie_runtime_service_test::assetWorkRunsSeriallyAndDeliversOnTheApplicationThread()
{
    RecordingSourceExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto subscriptionA = service->subscribe();
    auto subscriptionB = service->subscribe();
    QVERIFY(subscriptionA.isValid());
    QVERIFY(subscriptionB.isValid());

    QStringList deliveredSources;
    QThread *deliveryThread = nullptr;
    std::atomic<int> completions(0);
    const auto recordCompletion = [&](const LottieRuntimeSourceResult &result) {
        deliveryThread = QThread::currentThread();
        deliveredSources.append(result.asset.animationJsonPath());
        ++completions;
    };

    QVERIFY(service->requestSource(subscriptionA, QStringLiteral("a.lottie"),
                                   QStringLiteral("light"), recordCompletion));
    QVERIFY(service->requestSource(subscriptionB, QStringLiteral("b.lottie"), {},
                                   recordCompletion));

    QTRY_COMPARE(completions.load(), 2);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("a.lottie") << QStringLiteral("b.lottie");
    QCOMPARE(executor.startedSources(), expectedExecutions);
    QCOMPARE(deliveredSources, expectedExecutions);
    QCOMPARE(deliveryThread, QCoreApplication::instance()->thread());

    const QList<QThread *> executionThreads = executor.executionThreads();
    QCOMPARE(executionThreads.size(), 2);
    QCOMPARE(executionThreads.first(), executionThreads.last());
    QVERIFY(executionThreads.first() != QCoreApplication::instance()->thread());
}

void lottie_runtime_service_test::newerRequestReplacesQueuedWorkOfItsSubscription()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto keeper = service->subscribe();
    auto subscriber = service->subscribe();

    std::atomic<int> keeperCompletions(0);
    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);

    QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QVERIFY(service->requestSource(subscriber, QStringLiteral("first.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++firstCompletions;
                                   }));
    QVERIFY(service->requestSource(subscriber, QStringLiteral("second.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++secondCompletions;
                                   }));

    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("busy.lottie")));
    gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
    QTRY_COMPARE(keeperCompletions.load(), 1);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("busy.lottie") << QStringLiteral("second.lottie");
    QTRY_COMPARE(gate.startedSources(), expectedExecutions);
    gate.releaseOnce(loadedOutcome(QStringLiteral("second.lottie")));
    QTRY_COMPARE(secondCompletions.load(), 1);

    QCOMPARE(firstCompletions.load(), 0);
}

void lottie_runtime_service_test::supersededInFlightResultIsNotDelivered()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto subscriber = service->subscribe();

    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);

    QVERIFY(service->requestSource(subscriber, QStringLiteral("first.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++firstCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("first.lottie")));

    // 取代运行中的旧请求：旧工作被协作取消而非等到自然完成。
    QVERIFY(service->requestSource(subscriber, QStringLiteral("second.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++secondCompletions;
                                   }));
    QTRY_COMPARE(gate.cancelledCount(), 1);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("first.lottie") << QStringLiteral("second.lottie");
    QTRY_COMPARE(gate.startedSources(), expectedExecutions);
    gate.releaseOnce(loadedOutcome(QStringLiteral("second.lottie")));
    QTRY_COMPARE(secondCompletions.load(), 1);
    QCOMPARE(gate.completedCount(), 1);

    QCoreApplication::processEvents();
    QCOMPARE(firstCompletions.load(), 0);
}

void lottie_runtime_service_test::supersededCountsOnlyOutstandingReplacements()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("superseded.json"), 8);
    QVERIFY(!source.isEmpty());

    const LottieAsset asset = LottieCacheManager::instance()->requestAsset(
        LottieSourceReference::fromText(source)).asset;
    QVERIFY(asset.isValid());
    LottieRuntimeSourceResult sourceResult;
    sourceResult.loaded = true;
    sourceResult.asset = asset;

    BlockingRasterSessionController rasterController;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterController] {
        return rasterController.create();
    });
    LottieRuntimeServiceTestScope serviceScope(
        [](const LottieRuntimeSourceRequest &, const LottieRuntimeService::SourceCancellation &) {
            return LottieRuntimeSourceResult();
        });

    auto *service = LottieRuntimeService::instance();
    auto requestForKey = [&](int frame, int size) {
        return frameRequestFor(sourceResult, frame, QSize(size, size), {});
    };
    auto waitForCallback = [](const std::atomic<int> &callbacks, int expected) {
        QTRY_COMPARE(callbacks.load(), expected);
    };

    auto completedSubscription = service->subscribe();
    QVERIFY(completedSubscription.isValid());
    std::atomic<int> completedCallbacks(0);
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(completedSubscription, requestForKey(0, 24),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++completedCallbacks;
                                  }));
    QVERIFY(rasterController.waitUntilStarted());
    rasterController.release();
    waitForCallback(completedCallbacks, 1);
    const int completedSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    QVERIFY(service->requestFrame(completedSubscription, requestForKey(1, 24),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++completedCallbacks;
                                  }));
    waitForCallback(completedCallbacks, 2);
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             completedSuperseded);

    auto sameKeyKeeper = service->subscribe();
    auto sameKeySubscription = service->subscribe();
    QVERIFY(sameKeyKeeper.isValid());
    QVERIFY(sameKeySubscription.isValid());
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(sameKeyKeeper, requestForKey(6, 25),
                                  [&](const LottieRuntimeFrameResult &) {}));
    QVERIFY(rasterController.waitUntilStarted());
    QVERIFY(service->requestFrame(sameKeySubscription, requestForKey(7, 26),
                                  [&](const LottieRuntimeFrameResult &) {}));
    const int sameKeySuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    QVERIFY(service->requestFrame(sameKeySubscription, requestForKey(7, 26),
                                  [&](const LottieRuntimeFrameResult &) {}));
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             sameKeySuperseded);
    rasterController.release();
    QTRY_VERIFY(service->testFrameSchedulerStats().runningJobs == 0);

    auto keeper = service->subscribe();
    auto queuedSubscription = service->subscribe();
    QVERIFY(keeper.isValid());
    QVERIFY(queuedSubscription.isValid());
    std::atomic<int> queuedCallbacks(0);
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(keeper, requestForKey(2, 25),
                                  [&](const LottieRuntimeFrameResult &) {}));
    QVERIFY(rasterController.waitUntilStarted());
    const int queuedSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    QVERIFY(service->requestFrame(queuedSubscription, requestForKey(3, 26),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++queuedCallbacks;
                                  }));
    QVERIFY(service->requestFrame(queuedSubscription, requestForKey(4, 27),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++queuedCallbacks;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             queuedSuperseded + 1);

    auto coalescedSubscription = service->subscribe();
    QVERIFY(coalescedSubscription.isValid());
    const int coalescedSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    const int coalescedRequests =
        service->testFrameSchedulerStats().coalescedRequestCount;
    QVERIFY(service->requestFrame(coalescedSubscription, requestForKey(5, 28),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++queuedCallbacks;
                                  }));
    auto secondCoalescedSubscription = service->subscribe();
    QVERIFY(secondCoalescedSubscription.isValid());
    QVERIFY(service->requestFrame(secondCoalescedSubscription, requestForKey(5, 28),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++queuedCallbacks;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             coalescedSuperseded);
    QCOMPARE(service->testFrameSchedulerStats().coalescedRequestCount,
             coalescedRequests + 1);

    rasterController.release();
    QTRY_VERIFY(service->testFrameSchedulerStats().runningJobs == 0);

    auto runningSubscription = service->subscribe();
    QVERIFY(runningSubscription.isValid());
    std::atomic<int> runningCallbacks(0);
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(runningSubscription, requestForKey(6, 29),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++runningCallbacks;
                                  }));
    QVERIFY(rasterController.waitUntilStarted());
    const int runningSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    QVERIFY(service->requestFrame(runningSubscription, requestForKey(7, 30),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++runningCallbacks;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             runningSuperseded + 1);
    rasterController.release();
    waitForCallback(runningCallbacks, 1);

    auto pendingSubscription = service->subscribe();
    QVERIFY(pendingSubscription.isValid());
    std::atomic<int> pendingCallbacks(0);
    service->testHoldFrameSchedulerGuiBatch(true);
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(pendingSubscription, requestForKey(0, 31),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++pendingCallbacks;
                                  }));
    QVERIFY(rasterController.waitUntilStarted());
    rasterController.release();
    QTRY_VERIFY(service->testFrameSchedulerStats().pendingGuiBatches == 1);
    const int pendingSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    QVERIFY(service->requestFrame(pendingSubscription, requestForKey(1, 32),
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++pendingCallbacks;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             pendingSuperseded + 1);
    QTRY_VERIFY(service->testFrameSchedulerStats().runningJobs == 0);
    service->testFlushFrameSchedulerGuiBatch();
    waitForCallback(pendingCallbacks, 1);

    auto dropKeeper = service->subscribe();
    auto droppedSubscription = service->subscribe();
    QVERIFY(dropKeeper.isValid());
    QVERIFY(droppedSubscription.isValid());
    rasterController.blockNextRender();
    QVERIFY(service->requestFrame(dropKeeper, requestForKey(2, 33),
                                  [&](const LottieRuntimeFrameResult &) {}));
    QVERIFY(rasterController.waitUntilStarted());
    QVERIFY(service->requestFrame(droppedSubscription, requestForKey(3, 34),
                                  [&](const LottieRuntimeFrameResult &) {}));
    const int droppedSuperseded =
        service->testFrameSchedulerStats().supersededRequestCount;
    const int subscriberlessRemovals =
        service->testFrameSchedulerStats().subscriberlessQueuedJobRemovalCount;
    droppedSubscription = LottieRuntimeSubscription();
    QCOMPARE(service->testFrameSchedulerStats().supersededRequestCount,
             droppedSuperseded);
    QCOMPARE(service->testFrameSchedulerStats().subscriberlessQueuedJobRemovalCount,
             subscriberlessRemovals + 1);
    rasterController.release();
}

void lottie_runtime_service_test::destroyingSubscriptionRemovesQueuedWorkAndDropsInFlightResults()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    std::atomic<int> keeperCompletions(0);
    std::atomic<int> queuedCompletions(0);
    std::atomic<int> inFlightCompletions(0);
    std::atomic<int> lateCompletions(0);

    {
        auto keeper = service->subscribe();
        QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++keeperCompletions;
                                       }));
        QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("busy.lottie")));

        {
            auto observer = service->subscribe();
            QVERIFY(service->requestSource(observer, QStringLiteral("observed.lottie"), {},
                                           [&](const LottieRuntimeSourceResult &) {
                                               ++queuedCompletions;
                                           }));
        }

        gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
        QTRY_COMPARE(keeperCompletions.load(), 1);
    }

    QCoreApplication::processEvents();
    QCOMPARE(queuedCompletions.load(), 0);
    QCOMPARE(gate.completedCount(), 1);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("busy.lottie");
    QCOMPARE(gate.startedSources(), expectedExecutions);

    {
        auto inFlight = service->subscribe();
        QVERIFY(service->requestSource(inFlight, QStringLiteral("inflight.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++inFlightCompletions;
                                       }));
        QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("inflight.lottie")));
    }

    // 最后订阅者销毁使运行中的解压/加载被协作取消，而不是等待其自然完成。
    QTRY_COMPARE(gate.cancelledCount(), 1);
    QCOMPARE(gate.completedCount(), 1);

    QCoreApplication::processEvents();
    QCOMPARE(inFlightCompletions.load(), 0);

    auto late = service->subscribe();
    QVERIFY(late.isValid());
    QVERIFY(service->requestSource(late, QStringLiteral("late.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++lateCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("late.lottie")));
    gate.releaseOnce(loadedOutcome(QStringLiteral("late.lottie")));
    QTRY_COMPARE(lateCompletions.load(), 1);
}

void lottie_runtime_service_test::stoppingRejectsNewWorkAndDropsUndeliveredResults()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto inFlightSubscription = service->subscribe();
    auto queuedSubscription = service->subscribe();

    std::atomic<int> inFlightCompletions(0);
    std::atomic<int> queuedCompletions(0);
    std::atomic<int> lateCompletions(0);

    QVERIFY(service->requestSource(inFlightSubscription, QStringLiteral("inflight.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++inFlightCompletions;
                                   }));
    QVERIFY(service->requestSource(queuedSubscription, QStringLiteral("queued.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++queuedCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("inflight.lottie")));

    // 停止会协作取消运行中的工作，不必等待长解压/加载自然结束。
    QElapsedTimer stopDuration;
    stopDuration.start();
    service->stopForShutdown();
    QVERIFY(stopDuration.elapsed() < 5000);
    QCOMPARE(gate.cancelledCount(), 1);

    QVERIFY(service->isStopping());
    QCoreApplication::processEvents();
    QCOMPARE(inFlightCompletions.load(), 0);
    QCOMPARE(queuedCompletions.load(), 0);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("inflight.lottie");
    QCOMPARE(gate.startedSources(), expectedExecutions);

    QVERIFY(!service->requestSource(inFlightSubscription, QStringLiteral("late.lottie"), {},
                                    [&](const LottieRuntimeSourceResult &) {
                                        ++lateCompletions;
                                    }));
    QCOMPARE(lateCompletions.load(), 0);
    QVERIFY(!service->subscribe().isValid());
}

void lottie_runtime_service_test::shutdownReleasesAssetObservationsBeforeJoiningActiveWork()
{
    ShutdownBarrierSourceExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto activeSubscription = service->subscribe();
    auto queuedSubscription = service->subscribe();
    QVERIFY(activeSubscription.isValid());
    QVERIFY(queuedSubscription.isValid());

    auto hostState = QSharedPointer<int>::create(1);
    QWeakPointer<int> weakHostState = hostState.toWeakRef();
    std::atomic<int> callbackCount(0);

    QVERIFY(service->requestSource(activeSubscription, QStringLiteral("active.lottie"), {},
                                   [hostState, &callbackCount](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(hostState);
                                       ++callbackCount;
                                   }));
    QVERIFY(executor.waitUntilStarted(QStringLiteral("active.lottie")));

    QVERIFY(service->requestSource(queuedSubscription, QStringLiteral("queued.lottie"), {},
                                   [hostState, &callbackCount](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(hostState);
                                       ++callbackCount;
                                   }));
    QCOMPARE(executor.startedSources(), QStringList() << QStringLiteral("active.lottie"));

    std::thread shutdownThread([service] {
        service->stopForShutdown();
    });
    LottieRuntimeTest::ScopedThreadJoin shutdownThreadGuard(
        &shutdownThread, [&executor] { executor.release(); });
    QVERIFY(executor.waitUntilCancellationObserved());
    QVERIFY(!executor.timedOut());

    hostState.reset();
    const bool observationsReleased = weakHostState.isNull();

    executor.release();
    shutdownThreadGuard.join();
    QCOMPARE(executor.cancelledCount(), 1);
    QCOMPARE(executor.activeCount(), 0);
    QCOMPARE(executor.completedCount(), 1);
    QCOMPARE(callbackCount.load(), 0);
    QVERIFY(observationsReleased);

    // A second shutdown call must be a no-op after the workers have joined.
    service->stopForShutdown();
    QVERIFY(service->isStopping());
}

void lottie_runtime_service_test::subscriptionDestructionOverlapsStoppingWindow()
{
    ShutdownBarrierSourceExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    auto hostState = QSharedPointer<int>::create(1);
    QWeakPointer<int> weakHostState = hostState.toWeakRef();
    std::atomic<int> callbackCount(0);
    QVERIFY(service->requestSource(subscription, QStringLiteral("active.lottie"), {},
                                   [hostState, &callbackCount](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(hostState);
                                       ++callbackCount;
                                   }));
    QVERIFY(executor.waitUntilStarted(QStringLiteral("active.lottie")));

    std::atomic<bool> stoppingObserved(false);
    std::atomic<bool> subscriptionDestroyed(false);
    std::atomic<bool> cancellationObservedBeforeRelease(false);
    std::atomic<bool> timedOut(false);
    std::thread subscriptionThread(
        [subscription = std::move(subscription), service, &executor,
         &stoppingObserved, &subscriptionDestroyed,
         &cancellationObservedBeforeRelease, &timedOut]() mutable {
            QElapsedTimer timer;
            timer.start();
            while (!service->isStopping() &&
                   timer.elapsed() < kShutdownBarrierTimeoutMilliseconds) {
                QThread::yieldCurrentThread();
            }
            if (!service->isStopping()) {
                timedOut.store(true);
                subscription = LottieRuntimeSubscription();
                executor.release();
                return;
            }

            stoppingObserved.store(true);
            subscription = LottieRuntimeSubscription();
            subscriptionDestroyed.store(true);
            const bool cancellationObserved = executor.waitUntilCancellationObserved();
            cancellationObservedBeforeRelease.store(cancellationObserved);
            executor.release();
        });
    LottieRuntimeTest::ScopedThreadJoin subscriptionThreadGuard(
        &subscriptionThread, [&executor] { executor.release(); });

    QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "aboutToQuit", Qt::DirectConnection));
    subscriptionThreadGuard.join();

    QVERIFY(!timedOut.load());
    QVERIFY(stoppingObserved.load());
    QVERIFY(subscriptionDestroyed.load());
    QVERIFY(cancellationObservedBeforeRelease.load());
    QCOMPARE(executor.cancelledCount(), 1);
    QCOMPARE(executor.activeCount(), 0);
    QCOMPARE(executor.completedCount(), 1);
    QCOMPARE(executor.startedSources(), QStringList() << QStringLiteral("active.lottie"));
    QVERIFY(!executor.timedOut());
    QCOMPARE(callbackCount.load(), 0);
    hostState.reset();
    QVERIFY(weakHostState.isNull());
    QCOMPARE(LottieRuntimeTest::testAssetWorkerCount(service), 0);
    QVERIFY(service->isStopping());
    QVERIFY(!service->subscribe().isValid());
}

void lottie_runtime_service_test::aboutToQuitCannotResurrectRuntime()
{
    // The production hook must close the gate even when no runtime has been
    // created yet; otherwise a later lazy instance() call would resurrect
    // worker threads during application shutdown.
    {
        LottieRuntimeServiceTestScope scope(
            [](const LottieRuntimeSourceRequest &,
               const LottieRuntimeService::SourceCancellation &) {
                return LottieRuntimeSourceResult();
            });

        QVERIFY(!LottieRuntimeService::hasInstance());
        QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                          "aboutToQuit", Qt::DirectConnection));
        QVERIFY(!LottieRuntimeService::instance());
    }

    // The same no-resurrection contract applies after an existing runtime has
    // been stopped, and the test seam must remain idempotent.
    {
        LottieRuntimeServiceTestScope scope(
            [](const LottieRuntimeSourceRequest &,
               const LottieRuntimeService::SourceCancellation &) {
                return LottieRuntimeSourceResult();
            });

        auto *service = LottieRuntimeService::instance();
        QVERIFY(service);
        auto subscription = service->subscribe();
        QVERIFY(subscription.isValid());

        QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                          "aboutToQuit", Qt::DirectConnection));
        QVERIFY(service->isStopping());
        QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                          "aboutToQuit", Qt::DirectConnection));

        LottieRuntimeService::destroyInstance();
        QVERIFY(!LottieRuntimeService::hasInstance());
        QVERIFY(!LottieRuntimeService::instance());
    }
}

void lottie_runtime_service_test::shutdownDropsCombinedAssetRasterGuiAndHostWork()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("shutdown-seed.json"));
    QVERIFY(!source.isEmpty());

    const LottieAsset seedAsset = LottieCacheManager::instance()->requestAsset(
        LottieSourceReference::fromText(source)).asset;
    QVERIFY(seedAsset.isValid());
    QVERIFY(seedAsset.revision().isValid());

    ShutdownBarrierSourceExecutor assetExecutor(true);
    BlockingRasterSessionController rasterController;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterController] {
        return rasterController.create();
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&assetExecutor, source, seedAsset](
            const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            if (request.source.canonicalLocator() == source) {
                LottieRuntimeSourceResult result;
                result.loaded = true;
                result.asset = seedAsset;
                return result;
            }
            return assetExecutor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto seedSubscription = service->subscribe();
    auto pendingSubscription = service->subscribe();
    auto activeRasterSubscription = service->subscribe();
    auto queuedRasterSubscription = service->subscribe();
    auto activeAssetSubscription = service->subscribe();
    auto queuedAssetSubscription = service->subscribe();
    auto destroyedAssetSubscription = service->subscribe();
    QVERIFY(seedSubscription.isValid());
    QVERIFY(pendingSubscription.isValid());
    QVERIFY(activeRasterSubscription.isValid());
    QVERIFY(queuedRasterSubscription.isValid());
    QVERIFY(activeAssetSubscription.isValid());
    QVERIFY(queuedAssetSubscription.isValid());
    QVERIFY(destroyedAssetSubscription.isValid());

    std::atomic<int> assetCallbacks(0);
    LottieRuntimeSourceResult seedResult;
    QVERIFY(service->requestSource(seedSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       seedResult = result;
                                       ++assetCallbacks;
                                   }));
    QTRY_COMPARE(assetCallbacks.load(), 1);
    QVERIFY(seedResult.loaded);

    const LottieRuntimeFrameRequest warmRequest = frameRequestFor(
        seedResult, 0, QSize(12, 12), seedResult.asset.theme());
    std::atomic<int> warmCallbacks(0);
    QVERIFY(service->requestFrame(seedSubscription, warmRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++warmCallbacks;
                                  }));
    QVERIFY(rasterController.waitUntilStarted());
    rasterController.release();
    QTRY_COMPARE(warmCallbacks.load(), 1);

    auto hostState = QSharedPointer<int>::create(1);
    QWeakPointer<int> weakHostState = hostState.toWeakRef();
    auto destroyedHostState = QSharedPointer<int>::create(1);
    QWeakPointer<int> weakDestroyedHostState = destroyedHostState.toWeakRef();
    std::atomic<int> hostCallbacks(0);

    service->testHoldFrameSchedulerGuiBatch(true);
    QVERIFY(service->requestFrame(pendingSubscription, warmRequest,
                                  [hostState, &hostCallbacks](
                                      const LottieRuntimeFrameResult &) {
                                      Q_UNUSED(hostState);
                                      ++hostCallbacks;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);

    rasterController.blockNextRender();
    rasterController.setReleasePredicate([service] {
        return service->isStopping();
    });
    LottieRuntimeFrameRequest activeRasterRequest = warmRequest;
    activeRasterRequest.physicalPixelSize = QSize(13, 13);
    QVERIFY(service->requestFrame(activeRasterSubscription, activeRasterRequest,
                                  [hostState, &hostCallbacks](
                                      const LottieRuntimeFrameResult &) {
                                      Q_UNUSED(hostState);
                                      ++hostCallbacks;
                                  }));
    QVERIFY(rasterController.waitUntilStarted());

    LottieRuntimeFrameRequest queuedRasterRequest = warmRequest;
    queuedRasterRequest.physicalPixelSize = QSize(14, 14);
    QVERIFY(service->requestFrame(queuedRasterSubscription, queuedRasterRequest,
                                  [hostState, &hostCallbacks](
                                      const LottieRuntimeFrameResult &) {
                                      Q_UNUSED(hostState);
                                      ++hostCallbacks;
                                  }));

    QVERIFY(service->requestSource(activeAssetSubscription,
                                   QStringLiteral("active-asset.lottie"), {},
                                   [hostState, &hostCallbacks](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(hostState);
                                       ++hostCallbacks;
                                   }));
    QVERIFY(assetExecutor.waitUntilStarted(QStringLiteral("active-asset.lottie")));
    QVERIFY(service->requestSource(queuedAssetSubscription,
                                   QStringLiteral("queued-asset.lottie"), {},
                                   [hostState, &hostCallbacks](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(hostState);
                                       ++hostCallbacks;
                                   }));
    QCOMPARE(assetExecutor.startedSources(),
             QStringList() << QStringLiteral("active-asset.lottie"));

    QVERIFY(service->requestSource(destroyedAssetSubscription,
                                   QStringLiteral("destroyed-asset.lottie"), {},
                                   [destroyedHostState, &hostCallbacks](
                                       const LottieRuntimeSourceResult &) {
                                       Q_UNUSED(destroyedHostState);
                                       ++hostCallbacks;
                                   }));
    // Destroy one host/subscription while active and queued work from other
    // hosts remains held. Its queued observation must disappear immediately;
    // the still-live pending GUI batch and source/raster callbacks are then
    // released by stop() itself.
    destroyedAssetSubscription = LottieRuntimeSubscription();
    destroyedHostState.reset();
    QVERIFY(weakDestroyedHostState.isNull());
    QCOMPARE(assetExecutor.startedSources(),
             QStringList() << QStringLiteral("active-asset.lottie"));

    QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "aboutToQuit", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "aboutToQuit", Qt::DirectConnection));

    const LottieRuntimeFrameSchedulerStats stats = service->testFrameSchedulerStats();
    QCOMPARE(stats.rasterWorkerCount, 0);
    QCOMPARE(stats.queuedKeys, 0);
    QCOMPARE(stats.deferredSubscriptions, 0);
    QCOMPARE(stats.runningJobs, 0);
    QCOMPARE(stats.pendingGuiBatches, 0);
    QVERIFY(stats.maxQueuedKeys <= 64);
    QVERIFY(stats.maxRunningJobs <= 1);
    QVERIFY(stats.maxPendingGuiBatches <= 1);
    QCOMPARE(stats.rasterJobCount, 2);
    QVERIFY(stats.droppedResultCount >= 1);

    QCOMPARE(assetExecutor.startedSources(),
             QStringList() << QStringLiteral("active-asset.lottie"));
    QCOMPARE(assetExecutor.cancelledCount(), 1);
    QCOMPARE(assetExecutor.activeCount(), 0);
    QCOMPARE(assetExecutor.completedCount(), 1);
    QVERIFY(!assetExecutor.timedOut());
    QCOMPARE(rasterController.renderedSizes(),
             QList<QSize>({QSize(12, 12), QSize(13, 13)}));
    QCOMPARE(rasterController.renderThreads().size(), 2);
    QCOMPARE(rasterController.destructionThreads().size(), 1);
    QCOMPARE(rasterController.destructionThreads().constFirst(),
             rasterController.renderThreads().constFirst());
    QVERIFY(rasterController.renderThreads().constFirst() !=
            QCoreApplication::instance()->thread());

    QCoreApplication::processEvents();
    QCOMPARE(hostCallbacks.load(), 0);
    hostState.reset();
    QVERIFY(weakHostState.isNull());
    QVERIFY(!service->subscribe().isValid());
    QVERIFY(!service->requestSource(activeAssetSubscription,
                                    QStringLiteral("late-asset.lottie"), {},
                                    [&](const LottieRuntimeSourceResult &) {
                                        ++hostCallbacks;
                                    }));
    QVERIFY(!service->requestFrame(activeRasterSubscription, warmRequest,
                                   [&](const LottieRuntimeFrameResult &) {
                                       ++hostCallbacks;
                                   }));
}

void lottie_runtime_service_test::boundedStressMatrixCovers16To56DistinctKeys()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QList<int> distinctKeyCounts = {16, 32, 56};
    for (const int distinctKeyCount : distinctKeyCounts) {
        const QString source = writeAnimation(
            &directory, QStringLiteral("stress-%1.json").arg(distinctKeyCount),
            128 + distinctKeyCount);
        QVERIFY(!source.isEmpty());

        const int hostCount = distinctKeyCount + 8;
        BlockingRasterSessionController rasterController;
        LottieRuntimeRasterSessionTestScope rasterScope([&rasterController] {
            return rasterController.create();
        });
        LottieRuntimeServiceTestScope runtimeScope(
            [source](const LottieRuntimeSourceRequest &request,
                     const LottieRuntimeService::SourceCancellation &isCancelled) {
                LottieRuntimeSourceResult result;
                if (request.source.canonicalLocator() == source) {
                    result.asset = LottieCacheManager::instance()->requestAsset(
                        request.source, isCancelled).asset;
                    result.loaded = result.asset.isValid();
                    if (result.loaded) {
                        result.asset.setAvailableThemes(
                            {QStringLiteral("light"), QStringLiteral("dark")});
                        result.asset.setTheme(QStringLiteral("light"));
                    }
                }
                return result;
            });
        ScopedRasterControllerRelease rasterRelease(&rasterController);

        auto *service = LottieRuntimeService::instance();
        auto sourceSubscription = service->subscribe();
        QVERIFY(sourceSubscription.isValid());
        std::atomic<int> sourceCallbacks(0);
        LottieRuntimeSourceResult sourceResult;
        QVERIFY(service->requestSource(sourceSubscription, source, {},
                                       [&](const LottieRuntimeSourceResult &result) {
                                           sourceResult = result;
                                           ++sourceCallbacks;
                                       }));
        QTRY_COMPARE(sourceCallbacks.load(), 1);
        QVERIFY(sourceResult.loaded);

        std::vector<LottieRuntimeSubscription> hosts;
        hosts.reserve(static_cast<size_t>(hostCount));
        for (int index = 0; index < hostCount; ++index) {
            hosts.push_back(service->subscribe());
            QVERIFY(hosts.back().isValid());
        }

        const LottieRuntimeFrameRequest baseRequest = frameRequestFor(
            sourceResult, 0, QSize(20, 20), QStringLiteral("light"));
        const auto requestForKey = [&](int keyIndex) {
            LottieRuntimeFrameRequest request = baseRequest;
            request.frame = keyIndex;
            request.effectiveTheme = keyIndex % 2 == 0
                ? QStringLiteral("light")
                : QStringLiteral("dark");
            request.physicalPixelSize = QSize(20 + keyIndex, 20 + keyIndex);
            return request;
        };

        QVector<int> deliveredCounts(hostCount, 0);
        QVector<QSize> deliveredSizes(hostCount);
        std::atomic<int> deliveredTotal(0);
        const auto completionFor = [&](int hostIndex) {
            return [&, hostIndex](const LottieRuntimeFrameResult &result) {
                QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                ++deliveredCounts[hostIndex];
                deliveredSizes[hostIndex] = result.physicalPixelSize;
                ++deliveredTotal;
            };
        };

        rasterController.blockNextRender();
        QVERIFY(service->requestFrame(hosts.front(), requestForKey(0), completionFor(0)));
        QVERIFY(rasterController.waitUntilStarted());

        for (int hostIndex = 0; hostIndex < hostCount; ++hostIndex) {
            const int latestKey = (hostIndex + 1) % distinctKeyCount;
            if (hostIndex != 0) {
                QVERIFY(service->requestFrame(hosts.at(hostIndex),
                                               requestForKey(hostIndex % distinctKeyCount),
                                               completionFor(hostIndex)));
            }
            // Resize/theme/playback churn replaces the first request with the
            // current key before the worker is released.
            QVERIFY(service->requestFrame(hosts.at(hostIndex),
                                           requestForKey(latestKey),
                                           completionFor(hostIndex)));
        }

        const LottieRuntimeFrameSchedulerStats heldStats = service->testFrameSchedulerStats();
        QCOMPARE(heldStats.rasterWorkerCount, 1);
        QCOMPARE(heldStats.maxRunningJobs, 1);
        QVERIFY(heldStats.queuedKeys <= 64);
        QCOMPARE(heldStats.deferredSubscriptions, 0);
        QVERIFY(heldStats.maxPendingGuiBatches <= 1);

        rasterRelease.release();
        QTRY_COMPARE(deliveredTotal.load(), hostCount);
        for (int hostIndex = 0; hostIndex < hostCount; ++hostIndex) {
            QCOMPARE(deliveredCounts.at(hostIndex), 1);
            QCOMPARE(deliveredSizes.at(hostIndex),
                     QSize(20 + ((hostIndex + 1) % distinctKeyCount),
                           20 + ((hostIndex + 1) % distinctKeyCount)));
        }

        auto cacheSubscription = service->subscribe();
        QVERIFY(cacheSubscription.isValid());
        std::atomic<int> cacheCallbacks(0);
        service->testHoldFrameSchedulerGuiBatch(true);
        QVERIFY(service->requestFrame(cacheSubscription, requestForKey(1),
                                      [&](const LottieRuntimeFrameResult &result) {
                                          QCOMPARE(result.failure,
                                                   LottieRuntimeFrameResult::Failure::None);
                                          ++cacheCallbacks;
                                      }));
        const LottieRuntimeFrameSchedulerStats pendingStats = service->testFrameSchedulerStats();
        QCOMPARE(pendingStats.pendingGuiBatches, 1);
        QVERIFY(pendingStats.maxPendingGuiBatches <= 1);
        QVERIFY(pendingStats.cacheHitCount >= 1);
        service->testFlushFrameSchedulerGuiBatch();
        QTRY_COMPARE(cacheCallbacks.load(), 1);

        // Prove queued work is removed when its only subscription disappears:
        // keep one raster job active so the deliberately unique second key
        // cannot start before its subscription is destroyed.
        rasterController.blockNextRender();
        rasterRelease.arm();
        std::atomic<int> heldRasterCallbacks(0);
        LottieRuntimeFrameRequest heldRequest = requestForKey(0);
        heldRequest.physicalPixelSize = QSize(200, 200);
        QVERIFY(service->requestFrame(hosts.front(), heldRequest,
                                       [&](const LottieRuntimeFrameResult &) {
                                           ++heldRasterCallbacks;
                                       }));
        QVERIFY(rasterController.waitUntilStarted());

        auto discardedSubscription = service->subscribe();
        QVERIFY(discardedSubscription.isValid());
        LottieRuntimeFrameRequest discardedRequest = heldRequest;
        discardedRequest.physicalPixelSize = QSize(201, 201);
        QVERIFY(service->requestFrame(discardedSubscription, discardedRequest,
                                      [](const LottieRuntimeFrameResult &) {}));
        QCOMPARE(service->testFrameSchedulerStats().queuedKeys, 1);
        discardedSubscription = LottieRuntimeSubscription();
        QCOMPARE(service->testFrameSchedulerStats().queuedKeys, 0);
        QVERIFY(service->testFrameSchedulerStats().subscriberlessQueuedJobRemovalCount > 0);
        rasterRelease.release();
        QTRY_COMPARE(heldRasterCallbacks.load(), 1);

        const LottieRuntimeFrameSchedulerStats finalStats = service->testFrameSchedulerStats();
        QVERIFY(finalStats.rasterWorkerCount == 1);
        QVERIFY(finalStats.maxQueuedKeys <= 64);
        QVERIFY(finalStats.maxRunningJobs <= 1);
        QVERIFY(finalStats.maxPendingGuiBatches <= 1);
        QVERIFY(finalStats.coalescedRequestCount > 0);
        QVERIFY(finalStats.supersededRequestCount >= hostCount);
        QVERIFY(finalStats.subscriberlessQueuedJobRemovalCount > 0);
        QCOMPARE(finalStats.immutableFrameCount, finalStats.rasterJobCount);
        QVERIFY(finalStats.droppedResultCount > 0);
    }
}

void lottie_runtime_service_test::sameSourceIsLoadedOnceForAllSubscribers()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    // busy keeper 保证 worker 占用，使后续共享 job 停留在排队状态
    // ——不依赖线程调度即可断言合并。
    auto keeper = service->subscribe();
    auto subscriberA = service->subscribe();
    auto subscriberB = service->subscribe();

    std::atomic<int> keeperCompletions(0);
    std::atomic<int> completionsA(0);
    std::atomic<int> completionsB(0);
    LottieRuntimeSourceResult resultA;
    LottieRuntimeSourceResult resultB;

    QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("busy.lottie")));

    // busy 运行期间两个订阅者请求同一 source：job 保持排队 → 合并为一个。
    QVERIFY(service->requestSource(subscriberA, QStringLiteral("shared.lottie"), QStringLiteral("light"),
                                   [&](const LottieRuntimeSourceResult &result) {
                                       resultA = result;
                                       ++completionsA;
                                   }));
    QVERIFY(service->requestSource(subscriberB, QStringLiteral("shared.lottie"), QStringLiteral("dark"),
                                   [&](const LottieRuntimeSourceResult &result) {
                                       resultB = result;
                                       ++completionsB;
                                   }));
    // 两个共享请求都在排队：只有 busy 已启动。
    QCOMPARE(gate.startedSources().size(), 1);

    gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
    QTRY_COMPARE(keeperCompletions.load(), 1);
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("shared.lottie")));
    // 同一 source 的多个订阅者只执行一次资产加载。
    QCOMPARE(gate.startedSources().size(), 2);

    LottieRuntimeSourceResult outcome;
    outcome.loaded = true;
    outcome.asset.setAnimationJsonPath(QStringLiteral("shared.lottie"));
    outcome.asset.setAvailableThemes({QStringLiteral("light"), QStringLiteral("dark")});
    gate.releaseOnce(outcome);
    QTRY_COMPARE(completionsA.load(), 1);
    QTRY_COMPARE(completionsB.load(), 1);
    QCOMPARE(gate.completedCount(), 2);

    // theme 等订阅者特有派生状态按各自请求应用，互不串用。
    QCOMPARE(resultA.asset.theme(), QStringLiteral("light"));
    QCOMPARE(resultB.asset.theme(), QStringLiteral("dark"));
    QCOMPARE(resultA.asset.animationJsonPath(), QStringLiteral("shared.lottie"));
}

void lottie_runtime_service_test::queuedCanonicalSourceAliasesShareOneJob_data()
{
    QTest::addColumn<QString>("firstSource");
    QTest::addColumn<QString>("secondSource");
    QTest::addColumn<QUrl>("baseUrl");

    const QUrl qmlBase(QStringLiteral("file:///tmp/lottie-canonical-source/"));
    QTest::newRow("motion-name-and-file-name")
        << QStringLiteral("alarm-24-symbolic-motion")
        << QStringLiteral("alarm-24-symbolic-motion.lottie")
        << qmlBase;

    const QString localPath = QStringLiteral("/tmp/lottie-canonical-source/local.json");
    QTest::newRow("local-path-and-file-url")
        << localPath << QUrl::fromLocalFile(localPath).toString() << QUrl();
    QTest::newRow("absolute-and-qml-relative-file")
        << localPath << QStringLiteral("local.json") << qmlBase;

    const QUrl qrcBase(QStringLiteral("qrc:/lottie-canonical-source/"));
    QTest::newRow("qrc-url-and-qml-relative-resource")
        << QStringLiteral("qrc:/lottie-canonical-source/local.json")
        << QStringLiteral("local.json") << qrcBase;
}

void lottie_runtime_service_test::queuedCanonicalSourceAliasesShareOneJob()
{
    QFETCH(QString, firstSource);
    QFETCH(QString, secondSource);
    QFETCH(QUrl, baseUrl);

    const LottieSourceReference firstReference =
        LottieSourceReference::fromText(firstSource, baseUrl);
    const LottieSourceReference secondReference =
        LottieSourceReference::fromText(secondSource, baseUrl);
    QVERIFY(firstReference.isValid());
    QVERIFY(secondReference.isValid());
    QCOMPARE(firstReference.jobKey(), secondReference.jobKey());
    const QString canonicalLocator = firstReference.canonicalLocator();

    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto keeper = service->subscribe();
    auto firstSubscriber = service->subscribe();
    auto secondSubscriber = service->subscribe();
    std::atomic<int> keeperCompletions(0);
    std::atomic<int> aliasCompletions(0);

    QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QTRY_COMPARE(gate.startedSources(), QStringList{QStringLiteral("busy.lottie")});

    QVERIFY(service->requestSource(firstSubscriber, firstReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++aliasCompletions;
                                   }));
    QVERIFY(service->requestSource(secondSubscriber, secondReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++aliasCompletions;
                                   }));
    QCOMPARE(gate.startedSources().size(), 1);

    gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
    QTRY_COMPARE(keeperCompletions.load(), 1);
    QTRY_COMPARE(gate.startedSources().size(), 2);
    QCOMPARE(gate.startedSources().constLast(), canonicalLocator);

    gate.releaseOnce(loadedOutcome(canonicalLocator));
    QTRY_COMPARE(aliasCompletions.load(), 2);
    QCOMPARE(gate.startedSources().size(), 2);
}

void lottie_runtime_service_test::relativeSourceBasesRemainDistinctQueuedJobs()
{
    const QString source = QStringLiteral("alarm-24-symbolic-motion");
    const LottieSourceReference firstReference = LottieSourceReference::fromText(
        source, QUrl(QStringLiteral("file:///tmp/lottie-source-base-a/")));
    const LottieSourceReference secondReference = LottieSourceReference::fromText(
        source, QUrl(QStringLiteral("file:///tmp/lottie-source-base-b/")));
    QVERIFY(firstReference.jobKey() != secondReference.jobKey());

    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });
    auto *service = LottieRuntimeService::instance();
    auto keeper = service->subscribe();
    auto firstSubscriber = service->subscribe();
    auto secondSubscriber = service->subscribe();
    std::atomic<int> keeperCompletions(0);
    std::atomic<int> sourceCompletions(0);

    QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QTRY_COMPARE(gate.startedSources().size(), 1);
    QVERIFY(service->requestSource(firstSubscriber, firstReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++sourceCompletions;
                                   }));
    QVERIFY(service->requestSource(secondSubscriber, secondReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++sourceCompletions;
                                   }));

    gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
    QTRY_COMPARE(keeperCompletions.load(), 1);
    QTRY_COMPARE(gate.startedSources().size(), 2);
    gate.releaseOnce(loadedOutcome(source));
    QTRY_COMPARE(sourceCompletions.load(), 1);
    QTRY_COMPARE(gate.startedSources().size(), 3);
    gate.releaseOnce(loadedOutcome(source));
    QTRY_COMPARE(sourceCompletions.load(), 2);
}

void lottie_runtime_service_test::runningCanonicalSourceJobDoesNotAcceptAliases()
{
    const QUrl baseUrl(QStringLiteral("file:///tmp/lottie-active-source/"));
    const QString firstSource = QStringLiteral("alarm-24-symbolic-motion");
    const QString secondSource = QStringLiteral("alarm-24-symbolic-motion.lottie");
    const LottieSourceReference firstReference =
        LottieSourceReference::fromText(firstSource, baseUrl);
    const LottieSourceReference secondReference =
        LottieSourceReference::fromText(secondSource, baseUrl);
    QCOMPARE(firstReference.jobKey(), secondReference.jobKey());
    const QString firstLocator = firstReference.canonicalLocator();
    const QString secondLocator = secondReference.canonicalLocator();

    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });
    auto *service = LottieRuntimeService::instance();
    auto firstSubscriber = service->subscribe();
    auto secondSubscriber = service->subscribe();
    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);

    QVERIFY(service->requestSource(firstSubscriber, firstReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++firstCompletions;
                                   }));
    QTRY_COMPARE(gate.startedSources(), QStringList{firstLocator});
    QVERIFY(service->requestSource(secondSubscriber, secondReference, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++secondCompletions;
                                   }));

    gate.releaseOnce(loadedOutcome(firstLocator));
    QTRY_COMPARE(firstCompletions.load(), 1);
    QTRY_COMPARE(gate.startedSources().size(), 2);
    QCOMPARE(gate.startedSources().constLast(), secondLocator);
    gate.releaseOnce(loadedOutcome(secondLocator));
    QTRY_COMPARE(secondCompletions.load(), 1);
}

void lottie_runtime_service_test::oneSubscriberLeavingDoesNotCancelSharedLoad()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    std::atomic<int> leaverCompletions(0);
    std::atomic<int> remainingCompletions(0);

    {
        auto leaver = service->subscribe();
        auto remaining = service->subscribe();
        QVERIFY(service->requestSource(leaver, QStringLiteral("shared.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++leaverCompletions;
                                       }));
        QVERIFY(service->requestSource(remaining, QStringLiteral("shared.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++remainingCompletions;
                                       }));
        QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("shared.lottie")));
        QCOMPARE(gate.startedSources().size(), 1);

        // 一个观察者离开不是取消理由：仍有订阅者观察同一 source generation。
        leaver = LottieRuntimeSubscription();
        QTest::qWait(100);
        QCOMPARE(gate.cancelledCount(), 0);

        gate.releaseOnce(loadedOutcome(QStringLiteral("shared.lottie")));
        QTRY_COMPARE(remainingCompletions.load(), 1);
        QCOMPARE(leaverCompletions.load(), 0);
        QCOMPARE(gate.completedCount(), 1);
    }
}

void lottie_runtime_service_test::lastObserverLeavingCancelsSharedLoad()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);

    {
        auto first = service->subscribe();
        {
            auto second = service->subscribe();
            QVERIFY(service->requestSource(first, QStringLiteral("shared.lottie"), {},
                                           [&](const LottieRuntimeSourceResult &) {
                                               ++firstCompletions;
                                           }));
            QVERIFY(service->requestSource(second, QStringLiteral("shared.lottie"), {},
                                           [&](const LottieRuntimeSourceResult &) {
                                               ++secondCompletions;
                                           }));
            QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("shared.lottie")));
            QCOMPARE(gate.startedSources().size(), 1);

            // 还剩一个观察者：不取消。
            second = LottieRuntimeSubscription();
            QTest::qWait(100);
            QCOMPARE(gate.cancelledCount(), 0);
        }
    }

    // 最后观察者离开：运行中的加载被协作取消，且不成为可观察错误。
    QTRY_COMPARE(gate.cancelledCount(), 1);
    QCOMPARE(gate.completedCount(), 0);
    QCoreApplication::processEvents();
    QCOMPARE(firstCompletions.load(), 0);
    QCOMPARE(secondCompletions.load(), 0);

    // 取消后 runtime 仍可接受新工作。
    auto next = service->subscribe();
    std::atomic<int> nextCompletions(0);
    QVERIFY(service->requestSource(next, QStringLiteral("next.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++nextCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("next.lottie")));
    gate.releaseOnce(loadedOutcome(QStringLiteral("next.lottie")));
    QTRY_COMPARE(nextCompletions.load(), 1);
}

void lottie_runtime_service_test::switchingSourceDetachesWithoutCancellingRemainingObservers()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto switcher = service->subscribe();
    auto stayer = service->subscribe();

    std::atomic<int> switcherCompletions(0);
    std::atomic<int> stayerCompletions(0);
    QStringList switcherSources;
    QStringList stayerSources;

    QVERIFY(service->requestSource(switcher, QStringLiteral("first.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       switcherSources.append(result.asset.animationJsonPath());
                                       ++switcherCompletions;
                                   }));
    QVERIFY(service->requestSource(stayer, QStringLiteral("first.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       stayerSources.append(result.asset.animationJsonPath());
                                       ++stayerCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("first.lottie")));
    QCOMPARE(gate.startedSources().size(), 1);

    // 一个订阅者切换 source：仅解除自身观察，不取消仍在被观察的共享工作。
    QVERIFY(service->requestSource(switcher, QStringLiteral("second.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       switcherSources.append(result.asset.animationJsonPath());
                                       ++switcherCompletions;
                                   }));
    QTest::qWait(100);
    QCOMPARE(gate.cancelledCount(), 0);

    gate.releaseOnce(loadedOutcome(QStringLiteral("first.lottie")));
    QTRY_COMPARE(stayerCompletions.load(), 1);
    QCOMPARE(stayerSources, QStringList({QStringLiteral("first.lottie")}));
    QCOMPARE(switcherCompletions.load(), 0);

    QStringList expectedExecutions;
    expectedExecutions << QStringLiteral("first.lottie") << QStringLiteral("second.lottie");
    QTRY_COMPARE(gate.startedSources(), expectedExecutions);
    gate.releaseOnce(loadedOutcome(QStringLiteral("second.lottie")));
    QTRY_COMPARE(switcherCompletions.load(), 1);
    QCOMPARE(switcherSources, QStringList({QStringLiteral("second.lottie")}));
}

void lottie_runtime_service_test::queuedSharedJobSurvivesUntilItsLastObserverLeaves()
{
    GatedSourceExecutor gate;
    LottieRuntimeServiceTestScope scope(
        [&gate](const LottieRuntimeSourceRequest &request,
                const LottieRuntimeService::SourceCancellation &isCancelled) {
            return gate.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    std::atomic<int> keeperCompletions(0);
    std::atomic<int> survivorCompletions(0);
    std::atomic<int> removedCompletions(0);
    std::atomic<int> lateCompletions(0);

    auto keeper = service->subscribe();
    QVERIFY(service->requestSource(keeper, QStringLiteral("busy.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("busy.lottie")));

    // 两个订阅者共享排队中的 source job。
    {
        auto leaver = service->subscribe();
        auto survivor = service->subscribe();
        QVERIFY(service->requestSource(leaver, QStringLiteral("shared.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++removedCompletions;
                                       }));
        QVERIFY(service->requestSource(survivor, QStringLiteral("shared.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++survivorCompletions;
                                       }));

        // 一个观察者离开后仍有订阅者观察：排队工作保留。
        leaver = LottieRuntimeSubscription();
        QTest::qWait(100);
        QCOMPARE(gate.startedSources().contains(QStringLiteral("shared.lottie")), false);

        gate.releaseOnce(loadedOutcome(QStringLiteral("busy.lottie")));
        QTRY_COMPARE(keeperCompletions.load(), 1);

        QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("shared.lottie")));
        gate.releaseOnce(loadedOutcome(QStringLiteral("shared.lottie")));
        QTRY_COMPARE(survivorCompletions.load(), 1);
        QCOMPARE(removedCompletions.load(), 0);
    }

    // 排队中的共享 job 在全部观察者离开后被移除，不会执行。
    auto secondKeeper = service->subscribe();
    QVERIFY(service->requestSource(secondKeeper, QStringLiteral("busy2.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++keeperCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("busy2.lottie")));
    {
        auto removedA = service->subscribe();
        auto removedB = service->subscribe();
        QVERIFY(service->requestSource(removedA, QStringLiteral("removed.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++removedCompletions;
                                       }));
        QVERIFY(service->requestSource(removedB, QStringLiteral("removed.lottie"), {},
                                       [&](const LottieRuntimeSourceResult &) {
                                           ++removedCompletions;
                                       }));
    }
    gate.releaseOnce(loadedOutcome(QStringLiteral("busy2.lottie")));
    QTRY_COMPARE(keeperCompletions.load(), 2);
    QCoreApplication::processEvents();
    QCOMPARE(gate.startedSources().contains(QStringLiteral("removed.lottie")), false);
    QCOMPARE(removedCompletions.load(), 0);

    // 被移除的 source 之后仍可重新请求。
    auto late = service->subscribe();
    QVERIFY(service->requestSource(late, QStringLiteral("removed.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++lateCompletions;
                                   }));
    QTRY_VERIFY(gate.startedSources().contains(QStringLiteral("removed.lottie")));
    gate.releaseOnce(loadedOutcome(QStringLiteral("removed.lottie")));
    QTRY_COMPARE(lateCompletions.load(), 1);
}

void lottie_runtime_service_test::runningJobIsNotJoinedAfterSourceContentReplaced()
{
    SnapshotAtEntryExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = directory.filePath(QStringLiteral("replaced.json"));
    const QByteArray contentA = animationJson();
    QVERIFY(writeBytes(source, contentA));

    auto *service = LottieRuntimeService::instance();
    auto firstSubscriber = service->subscribe();
    auto secondSubscriber = service->subscribe();

    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);
    LottieRuntimeSourceResult firstResult;
    LottieRuntimeSourceResult secondResult;

    // 第一个订阅者的 job 开始运行并在 worker 中物化自己的 snapshot（内容 A）。
    QVERIFY(service->requestSource(firstSubscriber, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       firstResult = result;
                                       ++firstCompletions;
                                   }));
    QTRY_VERIFY(executor.startedSources().contains(source));
    QCOMPARE(executor.startedSources().size(), 1);

    // 运行期间原路径被替换为 B：随后的请求不得加入运行中的 job，
    // 必须获得自己的 job 与 snapshot。
    const QByteArray contentB = metadataAnimationJson(30, 0, 4);
    QVERIFY(writeBytes(source, contentB));
    QVERIFY(service->requestSource(secondSubscriber, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       secondResult = result;
                                       ++secondCompletions;
                                   }));
    QTest::qWait(100);
    QCOMPARE(executor.startedSources().size(), 1);

    executor.releaseOnce();
    QTRY_COMPARE(firstCompletions.load(), 1);
    QTRY_VERIFY(executor.startedSources().size() == 2);
    executor.releaseOnce();
    QTRY_COMPARE(secondCompletions.load(), 1);

    // 第一个得到 A 的身份，第二个得到 B 的身份，两者不同。
    const QString identityA = QString::fromLatin1(
        QCryptographicHash::hash(contentA, QCryptographicHash::Sha256).toHex());
    const QString identityB = QString::fromLatin1(
        QCryptographicHash::hash(contentB, QCryptographicHash::Sha256).toHex());
    QCOMPARE(firstResult.asset.animationJsonPath(), identityA);
    QCOMPARE(secondResult.asset.animationJsonPath(), identityB);
    QVERIFY(identityA != identityB);
}

void lottie_runtime_service_test::shuttingDownHonoursContractOnActiveJsonPath()
{
    SnapshotAtEntryExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = directory.filePath(QStringLiteral("active.json"));
    QVERIFY(writeBytes(source, animationJson()));

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    std::atomic<int> completions(0);
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++completions;
                                   }));
    QTRY_VERIFY(executor.startedSources().contains(source));

    // 活跃 JSON 路径上的停止必须有界返回：worker 观察取消后收尾，
    // 且取消不作为可观察结果交付。
    QElapsedTimer stopDuration;
    stopDuration.start();
    QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "aboutToQuit", Qt::DirectConnection));
    QVERIFY(QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "aboutToQuit", Qt::DirectConnection));
    QVERIFY(stopDuration.elapsed() < 5000);
    QCOMPARE(executor.cancelledCount(), 1);

    QVERIFY(service->isStopping());
    QCoreApplication::processEvents();
    QCOMPARE(completions.load(), 0);
    QVERIFY(!service->subscribe().isValid());
}

void lottie_runtime_service_test::assetFailureIsContainedToTheRequest()
{
    RecordingSourceExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto failingSubscription = service->subscribe();
    auto healthySubscription = service->subscribe();

    std::atomic<int> completions(0);
    bool failingLoaded = true;
    bool healthyLoaded = false;
    const auto recordCompletion = [&](bool *loadedFlag) {
        return [loadedFlag, &completions](const LottieRuntimeSourceResult &result) {
            *loadedFlag = result.loaded;
            ++completions;
        };
    };

    QVERIFY(service->requestSource(failingSubscription, QStringLiteral("broken.missing"), {},
                                   recordCompletion(&failingLoaded)));
    QVERIFY(service->requestSource(healthySubscription, QStringLiteral("healthy.lottie"), {},
                                   recordCompletion(&healthyLoaded)));

    QTRY_COMPARE(completions.load(), 2);
    QCOMPARE(failingLoaded, false);
    QCOMPARE(healthyLoaded, true);
    QVERIFY(!service->isStopping());
}

void lottie_runtime_service_test::completionCanReleaseItsSubscription()
{
    RecordingSourceExecutor executor;
    LottieRuntimeServiceTestScope scope(
        [&executor](const LottieRuntimeSourceRequest &request,
                    const LottieRuntimeService::SourceCancellation &isCancelled) {
            return executor.run(request, isCancelled);
        });

    auto *service = LottieRuntimeService::instance();
    auto releasable = service->subscribe();
    QVERIFY(releasable.isValid());

    std::atomic<int> firstCompletions(0);
    std::atomic<int> secondCompletions(0);

    QVERIFY(service->requestSource(releasable, QStringLiteral("first.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       releasable = LottieRuntimeSubscription();
                                       ++firstCompletions;
                                   }));
    QTRY_COMPARE(firstCompletions.load(), 1);
    QVERIFY(!releasable.isValid());

    auto next = service->subscribe();
    QVERIFY(service->requestSource(next, QStringLiteral("second.lottie"), {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++secondCompletions;
                                   }));
    QTRY_COMPARE(secondCompletions.load(), 1);
}

void lottie_runtime_service_test::testScopeRestoresACleanRuntime()
{
    QVERIFY(!LottieRuntimeService::hasInstance());
    {
        auto *real = LottieRuntimeService::instance();
        QVERIFY(real);
        auto realSubscription = real->subscribe();
        QVERIFY(realSubscription.isValid());
    }

    RecordingSourceExecutor executor;
    {
        LottieRuntimeServiceTestScope scope(
            [&executor](const LottieRuntimeSourceRequest &request,
                        const LottieRuntimeService::SourceCancellation &isCancelled) {
                return executor.run(request, isCancelled);
            });
        QCOMPARE(LottieRuntimeService::hasInstance(), false);

        auto *scoped = LottieRuntimeService::instance();
        QVERIFY(scoped);
        auto subscription = scoped->subscribe();

        std::atomic<int> completions(0);
        QVERIFY(scoped->requestSource(subscription, QStringLiteral("controlled.lottie"), {},
                                      [&](const LottieRuntimeSourceResult &) {
                                          ++completions;
                                      }));
        QTRY_COMPARE(completions.load(), 1);
        QCOMPARE(executor.startedSources(),
                 QStringList() << QStringLiteral("controlled.lottie"));
    }

    QCOMPARE(LottieRuntimeService::hasInstance(), false);
    LottieRuntimeService::destroyInstance();
}

void lottie_runtime_service_test::defaultExecutorLoadsRealSourcesSerially()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> completions(0);
    LottieRuntimeSourceResult firstResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       firstResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 1);
    QCOMPARE(firstResult.loaded, true);
    QVERIFY(firstResult.asset.isValid());

    LottieRuntimeSourceResult missingResult;
    missingResult.loaded = true;
    QVERIFY(service->requestSource(subscription,
                                   directory.filePath(QStringLiteral("missing.json")), {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       missingResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 2);
    QCOMPARE(missingResult.loaded, false);
    QVERIFY(!missingResult.asset.isValid());
    QVERIFY(!service->isStopping());

    LottieRuntimeSourceResult repeatResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       repeatResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 3);
    QCOMPARE(repeatResult.loaded, true);
    QCOMPARE(repeatResult.asset.animationJsonPath(), firstResult.asset.animationJsonPath());

    LottieRuntimeFrameRequest frameRequest = frameRequestFor(
        repeatResult, 0, QSize(16, 16), repeatResult.asset.theme());
    LottieRuntimeFrameResult frameResult;
    QVERIFY(service->requestFrame(subscription, frameRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      frameResult = result;
                                      ++completions;
                                  }));
    QTRY_COMPARE(completions.load(), 4);
    QCOMPARE(frameResult.failure, LottieRuntimeFrameResult::Failure::None);
    QVERIFY(frameResult.frame.isValid());
    QVERIFY(LottieRuntimeTest::frameSchedulerDiagnostics(service).rasterExecutionP95Nanoseconds > 0);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::defaultExecutorReportsSourceWorkLatencySeparately()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("source-latency.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> completions(0);
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &) {
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 1);

    // This metric ends when the production source executor returns. It must
    // therefore be available without waiting for a Host frame commit.
    QVERIFY(LottieRuntimeTest::sourceLoadingP95Nanoseconds(service) > 0);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::parserExecutorUnzipCancellationIsReaped()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QByteArray activeAnimation = metadataAnimationJson(25, 0, 2).replace(
        QByteArrayLiteral("\"nm\":\"runtime\""),
        QByteArrayLiteral("\"nm\":\"ticket11-unzip-cancel-active\""));
    const QByteArray queuedAnimation = metadataAnimationJson(25, 0, 2).replace(
        QByteArrayLiteral("\"nm\":\"runtime\""),
        QByteArrayLiteral("\"nm\":\"ticket11-unzip-cancel-queued\""));
    const QString activeSource = writeArchive(
        &directory, QStringLiteral("active-cancel.lottie"), QByteArrayLiteral("active"),
        activeAnimation);
    const QString queuedSource = writeArchive(
        &directory, QStringLiteral("queued-cancel.lottie"), QByteArrayLiteral("queued"),
        queuedAnimation);
    QVERIFY(!activeSource.isEmpty());
    QVERIFY(!queuedSource.isEmpty());

    const QString blockingUnzip = QString::fromLocal8Bit(
        LOTTIE_BLOCKING_UNZIP_HELPER_PATH);
    QVERIFY(QFileInfo(blockingUnzip).isExecutable());
    const QString markerPath = directory.filePath(QStringLiteral("unzip-started.pid"));
    ScopedEnvironmentVariable markerEnvironment(
        "UKUI_QUICK_LOTTIE_BLOCKING_UNZIP_MARKER", QFile::encodeName(markerPath));

    std::atomic<int> executorStarted(0);
    LottieRuntimeServiceTestScope runtimeScope(
        [blockingUnzip, extractRoot = directory.path(), &executorStarted](
            const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            ++executorStarted;
            DotLottieParser parser(blockingUnzip);
            QString error;
            LottieRuntimeSourceResult result;
            const QString source = request.source.canonicalLocator();
            result.asset = parser.extractArchiveAndParse(
                source,
                source,
                QDir(extractRoot).filePath(
                    QFileInfo(source).fileName() + QStringLiteral("_FILES")),
                QString(),
                &error,
                isCancelled);
            result.loaded = result.asset.isValid();
            return result;
        });

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    auto activeSubscription = service->subscribe();
    auto queuedSubscription = service->subscribe();
    QVERIFY(activeSubscription.isValid());
    QVERIFY(queuedSubscription.isValid());

    std::atomic<int> callbackCount(0);
    QVERIFY(service->requestSource(activeSubscription, activeSource, {},
                                   [&callbackCount](const LottieRuntimeSourceResult &) {
                                       ++callbackCount;
                                   }));
    QTRY_VERIFY_WITH_TIMEOUT(QFileInfo(markerPath).size() > 0,
                             kShutdownBarrierTimeoutMilliseconds);
    QFile marker(markerPath);
    QVERIFY(marker.open(QIODevice::ReadOnly));
    bool processIdOk = false;
    const qint64 processId = marker.readAll().trimmed().toLongLong(&processIdOk);
    QVERIFY(processIdOk);
    QVERIFY(processId > 0);
    QVERIFY(QFileInfo(QStringLiteral("/proc/%1").arg(processId)).exists());
    QCOMPARE(LottieRuntimeTest::testAssetWorkerCount(service), 1);

    QVERIFY(service->requestSource(queuedSubscription, queuedSource, {},
                                   [&callbackCount](const LottieRuntimeSourceResult &) {
                                       ++callbackCount;
                                   }));
    QCOMPARE(executorStarted.load(), 1);

    const bool hookInvoked = QMetaObject::invokeMethod(
        QCoreApplication::instance(), "aboutToQuit", Qt::DirectConnection);
    QVERIFY(hookInvoked);
    QCOMPARE(executorStarted.load(), 1);
    QVERIFY(!QFileInfo(QStringLiteral("/proc/%1").arg(processId)).exists());
    QCOMPARE(callbackCount.load(), 0);
    QCOMPARE(LottieRuntimeTest::testAssetWorkerCount(service), 0);
    QVERIFY(service->isStopping());
    QVERIFY(!LottieRuntimeService::instance());
}

void lottie_runtime_service_test::defaultExecutorCarriesContentDerivedRevisions()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray originalBytes = metadataAnimationJson(30, 0, 2);
    const QString source = directory.filePath(QStringLiteral("revision.json"));
    QVERIFY(writeBytes(source, originalBytes));

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> completions(0);
    LottieRuntimeSourceResult firstResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       firstResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 1);
    QCOMPARE(firstResult.loaded, true);
    const LottieAssetRevision originalRevision = firstResult.asset.revision();
    QVERIFY(originalRevision.isValid());

    // 同一路径内容变化得到新 revision。
    QVERIFY(writeBytes(source, metadataAnimationJson(30, 0, 62)));
    LottieRuntimeSourceResult changedResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       changedResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 2);
    QCOMPARE(changedResult.loaded, true);
    QVERIFY(changedResult.asset.revision().isValid());
    QVERIFY(changedResult.asset.revision() != originalRevision);

    // 恢复原始内容恢复原始 revision。
    QVERIFY(writeBytes(source, originalBytes));
    LottieRuntimeSourceResult restoredResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       restoredResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 3);
    QCOMPARE(restoredResult.asset.revision(), originalRevision);

    // 不同路径的相同内容共享同一 revision：source 字符串不是结果身份。
    const QString copyPath = directory.filePath(QStringLiteral("copy/other-name.json"));
    QVERIFY(writeBytes(copyPath, originalBytes));
    LottieRuntimeSourceResult copyResult;
    QVERIFY(service->requestSource(subscription, copyPath, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       copyResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 4);
    QCOMPARE(copyResult.asset.revision(), originalRevision);

    // file:// URL 形式的同一 source 也得到同一 revision。
    LottieRuntimeSourceResult urlResult;
    QVERIFY(service->requestSource(subscription, QUrl::fromLocalFile(source).toString(), {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       urlResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 5);
    QCOMPARE(urlResult.asset.revision(), originalRevision);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::defaultExecutorPublishesLoadingMetadataBeforeFrames()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QByteArray manifest = QByteArrayLiteral(
        "{\"version\":\"2\",\"initial\":{\"animation\":\"main\",\"theme\":\"dark\"},")
        + QByteArrayLiteral("\"animations\":[{\"id\":\"main\",\"themes\":[\"light\",\"dark\"]}],")
        + QByteArrayLiteral("\"themes\":[\"dark\",\"light\"]}");
    const QString source = writeArchive(&directory,
                                        QStringLiteral("metadata.lottie"),
                                        QByteArrayLiteral("main"),
                                        metadataAnimationJson(24, 2, 26),
                                        manifest);
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> completions(0);
    LottieRuntimeSourceResult result;
    QVERIFY(service->requestSource(subscription, source, QStringLiteral("light"),
                                   [&](const LottieRuntimeSourceResult &outcome) {
                                       result = outcome;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 1);
    QCOMPARE(result.loaded, true);
    QVERIFY(result.asset.isValid());
    QVERIFY(result.asset.revision().isValid());

    // 帧元数据在首帧提交前即可发布。
    QCOMPARE(result.asset.frameCount(), 24);
    QCOMPARE(result.asset.frameRate(), 24.0);
    QCOMPARE(result.asset.availableThemes(),
             QStringList({QStringLiteral("dark"), QStringLiteral("light")}));
    QCOMPARE(result.asset.theme(), QStringLiteral("light"));
    QCOMPARE(result.asset.activeAnimationId(), QStringLiteral("main"));

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::defaultExecutorRejectsBudgetAndThemeOverflow()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> completions(0);

    // 超过 256 个结构性可选主题：验证失败而不是截断。
    const QString overflowSource = writeArchive(&directory,
                                                QStringLiteral("overflow.lottie"),
                                                QByteArrayLiteral("main"),
                                                metadataAnimationJson(30, 0, 2),
                                                themedArchiveManifest(QByteArrayLiteral("main"), 257));
    QVERIFY(!overflowSource.isEmpty());
    LottieRuntimeSourceResult overflowResult;
    QVERIFY(service->requestSource(subscription, overflowSource, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       overflowResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 1);
    QCOMPARE(overflowResult.loaded, false);
    QVERIFY(!overflowResult.asset.isValid());

    // 恰好 256 个主题按声明顺序完整发布。
    const QString cappedSource = writeArchive(&directory,
                                              QStringLiteral("capped.lottie"),
                                              QByteArrayLiteral("main"),
                                              metadataAnimationJson(30, 0, 2),
                                              themedArchiveManifest(QByteArrayLiteral("main"), 256));
    QVERIFY(!cappedSource.isEmpty());
    LottieRuntimeSourceResult cappedResult;
    QVERIFY(service->requestSource(subscription, cappedSource, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       cappedResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 2);
    QCOMPARE(cappedResult.loaded, true);
    QCOMPARE(cappedResult.asset.availableThemes().size(), 256);
    QCOMPARE(cappedResult.asset.availableThemes().constFirst(), QStringLiteral("theme-0"));
    QCOMPARE(cappedResult.asset.availableThemes().constLast(), QStringLiteral("theme-255"));
    QVERIFY(cappedResult.asset.revision().isValid());

    // 超过 16 MiB 的压缩输入被预算拒绝。
    const QString oversizedSource = directory.filePath(QStringLiteral("oversized.lottie"));
    {
        QFile oversized(oversizedSource);
        QVERIFY(oversized.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, '\0');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(oversized.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }
    LottieRuntimeSourceResult oversizedResult;
    QVERIFY(service->requestSource(subscription, oversizedSource, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       oversizedResult = result;
                                       ++completions;
                                   }));
    QTRY_COMPARE(completions.load(), 3);
    QCOMPARE(oversizedResult.loaded, false);
    QVERIFY(!oversizedResult.asset.isValid());

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::rasterRequestUsesRevisionClampedFrameAndPhysicalPixels()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("raster.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameResult frameResult;
    LottieAssetRevision expectedRevision;
    bool frameRequestAccepted = false;

    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       ++assetCompletions;
                                       QVERIFY(result.loaded);
                                       expectedRevision = result.asset.revision();

                                       LottieRuntimeFrameRequest request = frameRequestFor(
                                           result, 999, QSize(37, 23), result.asset.theme());
                                       frameRequestAccepted = service->requestFrame(
                                           subscription, request,
                                           [&](const LottieRuntimeFrameResult &outcome) {
                                               frameResult = outcome;
                                               ++frameCompletions;
                                           });
                                   }));

    QTRY_COMPARE(assetCompletions.load(), 1);
    QVERIFY(frameRequestAccepted);
    QTRY_COMPARE(frameCompletions.load(), 1);
    QCOMPARE(frameResult.failure, LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(frameResult.assetRevision, expectedRevision);
    QCOMPARE(frameResult.frameNumber, 1);
    QCOMPARE(frameResult.physicalPixelSize, QSize(37, 23));
    QVERIFY(frameResult.frame.isValid());
    QCOMPARE(frameResult.frame.size(), QSize(37, 23));
    QVERIFY(frameResult.frame.image().pixelColor(18, 11).alpha() > 0);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::equalRenderKeysCoalesceAcrossSubscriptions()
{
    std::atomic<int> renderCount(0);
    LottieRuntimeRasterSessionTestScope rasterScope([&renderCount] {
        return std::make_unique<CountingRasterSession>(&renderCount);
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("coalesced.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto firstSubscription = service->subscribe();
    auto secondSubscription = service->subscribe();
    QVERIFY(firstSubscription.isValid());
    QVERIFY(secondSubscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeFrameRequest request;
    QVERIFY(service->requestSource(firstSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(23, 17), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    std::atomic<int> firstFrameCompletions(0);
    std::atomic<int> secondFrameCompletions(0);
    QVERIFY(service->requestFrame(firstSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                                      ++firstFrameCompletions;
                                  }));
    QVERIFY(service->requestFrame(secondSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                                      ++secondFrameCompletions;
                                  }));

    QTRY_COMPARE(firstFrameCompletions.load(), 1);
    QTRY_COMPARE(secondFrameCompletions.load(), 1);
    QCOMPARE(renderCount.load(), 1);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::boundedFrameQueueDoesNotLetDeferredChurnStarveOlderJobs()
{
    BlockingRasterSessionController controller;
    controller.enableStepMode();
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;
    ScopedRasterControllerRelease controllerRelease(&controller);

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("bounded.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto firstSubscription = service->subscribe();
    QVERIFY(firstSubscription.isValid());
    LottieRuntimeFrameRequest request;
    std::atomic<int> assetCompletions(0);
    QVERIFY(service->requestSource(firstSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(8, 8), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    std::atomic<int> frameCompletions(0);
    QVERIFY(service->requestFrame(firstSubscription, request,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++frameCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());

    std::vector<LottieRuntimeSubscription> subscriptions;
    subscriptions.reserve(65);
    for (int index = 0; index < 64; ++index) {
        subscriptions.push_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
        LottieRuntimeFrameRequest queuedRequest = request;
        queuedRequest.physicalPixelSize = QSize(16 + index, 16 + index);
        QVERIFY(service->requestFrame(subscriptions.back(), queuedRequest,
                                      [&](const LottieRuntimeFrameResult &) {
                                          ++frameCompletions;
                                      }));
    }

    auto deferredSubscription = service->subscribe();
    QVERIFY(deferredSubscription.isValid());
    LottieRuntimeFrameRequest deferredRequest = request;
    deferredRequest.physicalPixelSize = QSize(512, 512);
    QVERIFY(service->requestFrame(deferredSubscription, deferredRequest,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++frameCompletions;
                                  }));

    auto laterDeferredSubscription = service->subscribe();
    QVERIFY(laterDeferredSubscription.isValid());
    LottieRuntimeFrameRequest laterDeferredRequest = request;
    laterDeferredRequest.physicalPixelSize = QSize(600, 600);
    QVERIFY(service->requestFrame(laterDeferredSubscription,
                                  laterDeferredRequest,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++frameCompletions;
                                  }));

    const LottieRuntimeFrameSchedulerStats fullStats = service->testFrameSchedulerStats();
    QCOMPARE(fullStats.queuedKeys, 64);
    QCOMPARE(fullStats.deferredSubscriptions, 2);
    QCOMPARE(fullStats.runningJobs, 1);
    QVERIFY(fullStats.maxQueuedKeys <= 64);

    // 每完成一个任务就把 deferred 宿主改成更“新”的帧。它仍不能越过已经
    // 等待更久的普通队列，否则持续播放的一个宿主会独占 raster worker。
    for (int step = 0; step < 3; ++step) {
        controller.releaseOne();
        QTRY_VERIFY(controller.startedSizes().size() >= step + 2);
        QCOMPARE(controller.startedSizes().at(step + 1),
                 QSize(16 + step, 16 + step));

        deferredRequest.physicalPixelSize = QSize(513 + step, 513 + step);
        QVERIFY(service->requestFrame(deferredSubscription,
                                      deferredRequest,
                                      [&](const LottieRuntimeFrameResult &) {
                                          ++frameCompletions;
                                      }));
    }

    controller.release();
    controllerRelease.release();
    QTRY_COMPARE(frameCompletions.load(), 67);

    // deferred 只是队列满时保存请求的位置，不是“越新越优先”的第二条队列。
    // 先等待的宿主 A 使用自己的最终尺寸 515×515，随后才轮到宿主 B。
    const QList<QSize> startedSizes = controller.startedSizes();
    QCOMPARE(startedSizes.size(), 67);
    QCOMPARE(startedSizes.at(65), QSize(515, 515));
    QCOMPARE(startedSizes.at(66), QSize(600, 600));

    const LottieRuntimeFrameSchedulerStats finalStats = service->testFrameSchedulerStats();
    QVERIFY(finalStats.maxQueuedKeys <= 64);
    QVERIFY(finalStats.maxPendingGuiBatches <= 1);
    QCOMPARE(finalStats.rasterJobCount, 67);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::unsubscribingQueuedFrameDropsJob()
{
    BlockingRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("queued-removal.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto runningSubscription = service->subscribe();
    auto queuedSubscription = service->subscribe();
    QVERIFY(runningSubscription.isValid());
    QVERIFY(queuedSubscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeFrameRequest request;
    QVERIFY(service->requestSource(runningSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(8, 8), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    std::atomic<int> runningCompletions(0);
    QVERIFY(service->requestFrame(runningSubscription, request,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++runningCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());

    LottieRuntimeFrameRequest queuedRequest = request;
    queuedRequest.physicalPixelSize = QSize(18, 18);
    std::atomic<int> queuedCompletions(0);
    QVERIFY(service->requestFrame(queuedSubscription, queuedRequest,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++queuedCompletions;
                                  }));
    QCOMPARE(service->testFrameSchedulerStats().queuedKeys, 1);

    queuedSubscription = LottieRuntimeSubscription();
    QCOMPARE(service->testFrameSchedulerStats().queuedKeys, 0);

    controller.release();
    QTRY_COMPARE(runningCompletions.load(), 1);

    auto barrierSubscription = service->subscribe();
    QVERIFY(barrierSubscription.isValid());
    LottieRuntimeFrameRequest barrierRequest = request;
    barrierRequest.physicalPixelSize = QSize(19, 19);
    std::atomic<int> barrierCompletions(0);
    QVERIFY(service->requestFrame(barrierSubscription, barrierRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++barrierCompletions;
                                  }));
    QTRY_COMPARE(barrierCompletions.load(), 1);
    QCOMPARE(queuedCompletions.load(), 0);
    QCOMPARE(controller.renderedSizes(), QList<QSize>({QSize(8, 8), QSize(19, 19)}));

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::retainsConcurrentTinyFrameWorkingSet()
{
    std::atomic<int> renderCount(0);
    LottieRuntimeRasterSessionTestScope rasterScope([&renderCount] {
        return std::make_unique<CountingRasterSession>(&renderCount);
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("concurrent-tiny-working-set.json"), 363);
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    LottieRuntimeSubscription sourceSubscription = service->subscribe();
    QVERIFY(sourceSubscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeSourceResult sourceResult;
    QVERIFY(service->requestSource(sourceSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       sourceResult = result;
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);
    QVERIFY(sourceResult.loaded);
    QCOMPARE(sourceResult.asset.frameCount(), 363);

    std::vector<LottieRuntimeSubscription> subscriptions;
    subscriptions.reserve(363);
    for (int index = 0; index < 363; ++index) {
        subscriptions.emplace_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
    }

    std::atomic<int> completions(0);
    const auto requestRound = [&](int expectedRasterJobs, int expectedCompletions) {
        service->testHoldFrameSchedulerGuiBatch(true);
        for (int frame = 0; frame < 363; ++frame) {
            const LottieRuntimeFrameRequest request = frameRequestFor(
                sourceResult, frame, QSize(50, 50), sourceResult.asset.theme());
            QVERIFY(service->requestFrame(
                subscriptions.at(frame), request,
                [&, frame](const LottieRuntimeFrameResult &result) {
                    QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                    QCOMPARE(result.frameNumber, frame);
                    QCOMPARE(result.physicalPixelSize, QSize(50, 50));
                    QVERIFY(result.frame.isValid());
                    QCOMPARE(result.frame.size(), QSize(50, 50));
                    ++completions;
                }));
        }

        QTRY_COMPARE(service->testFrameSchedulerStats().rasterJobCount,
                     expectedRasterJobs);
        QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
        QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
        service->testFlushFrameSchedulerGuiBatch();
        QCOMPARE(completions.load(), expectedCompletions);
    };

    requestRound(363, 363);
    const LottieRuntimeFrameSchedulerStats firstRoundStats =
        service->testFrameSchedulerStats();
    QCOMPARE(renderCount.load(), 363);
    QCOMPARE(firstRoundStats.rasterJobCount, 363);
    QCOMPARE(firstRoundStats.cacheEntryCount, 363);
    QCOMPARE(firstRoundStats.cacheBytes, 363LL * 50LL * 50LL * 4LL);
    QVERIFY(firstRoundStats.cacheBytes < 16LL * 1024LL * 1024LL);

    requestRound(363, 726);
    const LottieRuntimeFrameSchedulerStats secondRoundStats =
        service->testFrameSchedulerStats();
    QCOMPARE(renderCount.load(), 363);
    QCOMPARE(secondRoundStats.rasterJobCount, 363);
    QCOMPARE(secondRoundStats.cacheHitCount, 363);
    QCOMPARE(secondRoundStats.cacheEntryCount, 363);
    QCOMPARE(secondRoundStats.cacheBytes, 363LL * 50LL * 50LL * 4LL);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::boundsTinyFrameMetadataAt512()
{
    std::atomic<int> renderCount(0);
    LottieRuntimeRasterSessionTestScope rasterScope([&renderCount] {
        return std::make_unique<CountingRasterSession>(&renderCount);
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("tiny-cache.json"), 514);
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeFrameRequest request;
    LottieRuntimeSourceResult sourceResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       sourceResult = result;
                                       request = frameRequestFor(
                                           result, 0, QSize(1, 1), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);
    QVERIFY(sourceResult.loaded);

    int frameCompletions = 0;
    const auto requestFrameAndWait = [&](int frame) {
        LottieRuntimeFrameRequest frameRequest = request;
        frameRequest.frame = frame;
        frameRequest.physicalPixelSize = QSize(1, 1);
        const int expectedCompletionCount = frameCompletions + 1;
        QVERIFY(service->requestFrame(subscription, frameRequest,
                                      [&](const LottieRuntimeFrameResult &result) {
                                          QCOMPARE(result.failure,
                                                   LottieRuntimeFrameResult::Failure::None);
                                          ++frameCompletions;
                                      }));
        QTRY_COMPARE(frameCompletions, expectedCompletionCount);
    };

    constexpr int expectedEntryLimit = 512;
    for (int frame = 0; frame < expectedEntryLimit; ++frame) {
        requestFrameAndWait(frame);
    }
    const LottieRuntimeFrameSchedulerStats fullStats = service->testFrameSchedulerStats();
    QCOMPARE(fullStats.cacheEntryCount, expectedEntryLimit);
    QVERIFY(fullStats.cacheBytes <= 16LL * 1024LL * 1024LL);

    const int rendersBeforeHit = renderCount.load();
    requestFrameAndWait(0);
    QCOMPARE(renderCount.load(), rendersBeforeHit);

    requestFrameAndWait(expectedEntryLimit);
    QCOMPARE(renderCount.load(), rendersBeforeHit + 1);
    QCOMPARE(service->testFrameSchedulerStats().cacheEntryCount, expectedEntryLimit);

    // Frame 1 was the oldest entry after frame 0 was touched, so it must have
    // been evicted by frame 512 and render again.
    requestFrameAndWait(1);
    QCOMPARE(renderCount.load(), rendersBeforeHit + 2);

    const LottieRuntimeFrameSchedulerStats finalStats = service->testFrameSchedulerStats();
    QVERIFY(finalStats.cacheEntryCount <= expectedEntryLimit);
    QVERIFY(finalStats.cacheBytes <= 16LL * 1024LL * 1024LL);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::preservesByteBudget()
{
    std::atomic<int> renderCount(0);
    LottieRuntimeRasterSessionTestScope rasterScope([&renderCount] {
        return std::make_unique<CountingRasterSession>(&renderCount);
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("cache.json"), 5);
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeFrameRequest request;
    LottieRuntimeSourceResult sourceResult;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       sourceResult = result;
                                       request = frameRequestFor(
                                           result, 0, QSize(1024, 1024), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);
    QVERIFY(sourceResult.loaded);

    int frameCompletions = 0;
    const auto requestFrameAndWait = [&](int frame, const QSize &size) {
        LottieRuntimeFrameRequest frameRequest = request;
        frameRequest.frame = frame;
        frameRequest.physicalPixelSize = size;
        const int expectedCompletionCount = frameCompletions + 1;
        QVERIFY(service->requestFrame(subscription, frameRequest,
                                      [&](const LottieRuntimeFrameResult &result) {
                                          QCOMPARE(result.failure,
                                                   LottieRuntimeFrameResult::Failure::None);
                                          ++frameCompletions;
                                      }));
        QTRY_COMPARE(frameCompletions, expectedCompletionCount);
    };

    constexpr qint64 fourMiB = 4LL * 1024LL * 1024LL;
    constexpr qint64 sixteenMiB = 16LL * 1024LL * 1024LL;

    requestFrameAndWait(0, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 1);
    QCOMPARE(service->testFrameSchedulerStats().cacheBytes, fourMiB);

    requestFrameAndWait(0, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 1);
    QCOMPARE(service->testFrameSchedulerStats().cacheHitCount, 1);

    requestFrameAndWait(1, QSize(1024, 1024));
    requestFrameAndWait(2, QSize(1024, 1024));
    requestFrameAndWait(3, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 4);
    QCOMPARE(service->testFrameSchedulerStats().cacheEntryCount, 4);
    QCOMPARE(service->testFrameSchedulerStats().cacheBytes, sixteenMiB);

    // Touch frame 0 so frame 1 becomes the least-recently-used entry.
    requestFrameAndWait(0, QSize(1024, 1024));
    requestFrameAndWait(4, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 5);
    QCOMPARE(service->testFrameSchedulerStats().cacheEntryCount, 4);
    QCOMPARE(service->testFrameSchedulerStats().cacheBytes, sixteenMiB);
    requestFrameAndWait(1, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 6);

    // A single frame larger than the budget is delivered but bypasses insertion.
    requestFrameAndWait(0, QSize(2049, 2048));
    QCOMPARE(renderCount.load(), 7);
    QCOMPARE(service->testFrameSchedulerStats().cacheBytes, sixteenMiB);
    QCOMPARE(service->testFrameSchedulerStats().cacheEntryCount, 4);

    // Two cache hits submitted before the event loop runs share one GUI batch.
    auto secondSubscription = service->subscribe();
    QVERIFY(secondSubscription.isValid());
    std::atomic<int> firstHitCompletions(0);
    std::atomic<int> secondHitCompletions(0);
    const int scheduledBatchesBeforeHits =
        service->testFrameSchedulerStats().scheduledGuiBatchCount;
    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++firstHitCompletions;
                                  }));
    QVERIFY(service->requestFrame(secondSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++secondHitCompletions;
                                  }));
    const LottieRuntimeFrameSchedulerStats pendingStats = service->testFrameSchedulerStats();
    QCOMPARE(pendingStats.pendingGuiBatches, 1);
    QCOMPARE(pendingStats.scheduledGuiBatchCount, scheduledBatchesBeforeHits + 1);
    QVERIFY(pendingStats.maxPendingGuiBatches <= 1);
    QTRY_COMPARE(firstHitCompletions.load(), 1);
    QTRY_COMPARE(secondHitCompletions.load(), 1);

    // A new revision at the same source path cannot satisfy the old cache key.
    QVERIFY(writeBytes(source, animationJson(5) + QByteArrayLiteral("\n")));
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(1024, 1024), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 2);
    requestFrameAndWait(0, QSize(1024, 1024));
    QCOMPARE(renderCount.load(), 8);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::frameCacheDoesNotRetainSourceAssetPayload()
{
    std::atomic<int> renderCount(0);
    LottieRuntimeRasterSessionTestScope rasterScope([&renderCount] {
        return std::make_unique<CountingRasterSession>(&renderCount);
    });

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("frame-cache-owner.json"));
    QVERIFY(!source.isEmpty());

    const LottieAsset baseAsset = LottieCacheManager::instance()
                                      ->requestAsset(LottieSourceReference::fromText(source))
                                      .asset;
    QVERIFY(baseAsset.isValid());
    QVERIFY(baseAsset.revision().isValid());

    QByteArray assetPayload(1024 * 1024, 'x');
    LottieAsset payloadAsset = baseAsset;
    payloadAsset.setThemeSlotJson(QStringLiteral("unused-test-payload"), assetPayload);
    QVERIFY(!assetPayload.isDetached());

    LottieRuntimeFrameScheduler scheduler(1);
    LottieRuntimeFrameRequest request;
    request.assetRevision = payloadAsset.revision();
    request.asset = payloadAsset;
    request.frame = 0;
    request.physicalPixelSize = QSize(10, 10);

    std::atomic<int> frameCompletions(0);
    QVERIFY(scheduler.request(1, request, [&](const LottieRuntimeFrameResult &result) {
        QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
        ++frameCompletions;
    }));
    QTRY_COMPARE(frameCompletions.load(), 1);
    QCOMPARE(renderCount.load(), 1);

    // 帧缓存只应持有 QImage。取消订阅并放下请求后，大块资产负载
    // 必须只剩测试变量这一份引用，即使缓存帧仍然存在。
    scheduler.unsubscribe(1);
    request.asset = LottieAsset();
    payloadAsset = LottieAsset();
    QVERIFY(assetPayload.isDetached());

    // 帧缓存不需要原资产也能按 revision 命中并交付同一张不可变图片。
    request.assetRevision = baseAsset.revision();
    request.asset = baseAsset;
    std::atomic<int> cacheCompletions(0);
    QVERIFY(scheduler.request(2, request, [&](const LottieRuntimeFrameResult &result) {
        QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
        ++cacheCompletions;
    }));
    QTRY_COMPARE(cacheCompletions.load(), 1);
    QCOMPARE(renderCount.load(), 1);
}

void lottie_runtime_service_test::mixedCacheAndWorkerDeliveryRemainsBatched()
{
    BlockingRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("mixed-batch.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto workerSubscription = service->subscribe();
    auto cacheSubscription = service->subscribe();
    auto staleSubscription = service->subscribe();
    QVERIFY(workerSubscription.isValid());
    QVERIFY(cacheSubscription.isValid());
    QVERIFY(staleSubscription.isValid());

    std::atomic<int> assetCompletions(0);
    LottieRuntimeFrameRequest request;
    QVERIFY(service->requestSource(workerSubscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(11, 11), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    std::atomic<int> cacheWarmCompletions(0);
    QVERIFY(service->requestFrame(workerSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++cacheWarmCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());
    controller.release();
    QTRY_COMPARE(cacheWarmCompletions.load(), 1);

    controller.blockNextRender();
    LottieRuntimeFrameRequest workerRequest = request;
    workerRequest.physicalPixelSize = QSize(22, 22);
    std::atomic<int> workerCompletions(0);
    std::atomic<int> cacheHitCompletions(0);
    std::atomic<int> staleCacheCompletions(0);
    QVERIFY(service->requestFrame(workerSubscription, workerRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++workerCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());

    service->testHoldFrameSchedulerGuiBatch(true);
    const int scheduledBatchesBeforeWorkerCompletion =
        service->testFrameSchedulerStats().scheduledGuiBatchCount;
    controller.release();
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);

    QVERIFY(service->requestFrame(cacheSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++cacheHitCompletions;
                                  }));

    QVERIFY(service->requestFrame(staleSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure,
                                               LottieRuntimeFrameResult::Failure::None);
                                      ++staleCacheCompletions;
                                  }));

    const LottieRuntimeFrameSchedulerStats pendingStats = service->testFrameSchedulerStats();
    QCOMPARE(pendingStats.pendingGuiBatches, 1);
    QCOMPARE(pendingStats.scheduledGuiBatchCount,
             scheduledBatchesBeforeWorkerCompletion + 1);
    QVERIFY(pendingStats.maxPendingGuiBatches <= 1);
    QCOMPARE(workerCompletions.load(), 0);
    QCOMPARE(cacheHitCompletions.load(), 0);
    QCOMPARE(staleCacheCompletions.load(), 0);

    std::atomic<int> replacementCompletions(0);
    QVERIFY(service->requestFrame(staleSubscription, workerRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.physicalPixelSize, QSize(22, 22));
                                      ++replacementCompletions;
                                  }));

    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(workerCompletions.load(), 1);
    QCOMPARE(cacheHitCompletions.load(), 1);
    QCOMPARE(staleCacheCompletions.load(), 0);
    QCOMPARE(replacementCompletions.load(), 1);

    const LottieRuntimeFrameSchedulerStats finalStats = service->testFrameSchedulerStats();
    QCOMPARE(finalStats.pendingGuiBatches, 0);
    QVERIFY(finalStats.maxPendingGuiBatches <= 1);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::latestFrameRequestWinsAndStaleFailureIsIgnored()
{
    BlockingRasterSessionController controller;
    controller.setFirstRenderFails(true);
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("latest.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());
    LottieRuntimeFrameRequest request;
    std::atomic<int> assetCompletions(0);
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(20, 20), result.asset.theme());
                                       ++assetCompletions;
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    std::atomic<int> staleCompletions(0);
    std::atomic<int> latestCompletions(0);
    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++staleCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());

    LottieRuntimeFrameRequest latestRequest = request;
    latestRequest.physicalPixelSize = QSize(21, 21);
    QVERIFY(service->requestFrame(subscription, latestRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                                      QCOMPARE(result.physicalPixelSize, QSize(21, 21));
                                      ++latestCompletions;
                                  }));

    controller.release();
    QTRY_COMPARE(latestCompletions.load(), 1);
    QTest::qWait(50);
    QCOMPARE(staleCompletions.load(), 0);
    QCOMPARE(controller.renderedSizes(), QList<QSize>({QSize(20, 20), QSize(21, 21)}));

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::keepsDemandedSessionsResidentWithinCapacity()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("16"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("demanded-resident"), 15);
    QCOMPARE(sources.size(), 15);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    std::vector<LottieRuntimeSubscription> subscriptions;
    subscriptions.reserve(sources.size());
    for (int index = 0; index < sources.size(); ++index) {
        subscriptions.emplace_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
    }

    std::atomic<int> completions(0);
    const auto requestRound = [&](int round) {
        service->testHoldFrameSchedulerGuiBatch(true);
        for (int index = 0; index < sources.size(); ++index) {
            const LottieRuntimeFrameRequest request = frameRequestFor(
                sources.at(index), round, QSize(50, 50), {});
            const LottieAssetRevision expectedRevision = sources.at(index).asset.revision();
            QVERIFY(service->requestFrame(
                subscriptions.at(index), request,
                [&, expectedRevision, round](const LottieRuntimeFrameResult &result) {
                    QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                    QCOMPARE(result.assetRevision, expectedRevision);
                    QCOMPARE(result.frameNumber, round);
                    QCOMPARE(result.physicalPixelSize, QSize(50, 50));
                    QVERIFY(result.frame.isValid());
                    ++completions;
                }));
        }

        QTRY_COMPARE(service->testFrameSchedulerStats().rasterJobCount,
                     (round + 1) * sources.size());
        QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
        QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
        service->testFlushFrameSchedulerGuiBatch();
        QCOMPARE(completions.load(), (round + 1) * sources.size());
    };

    requestRound(0);
    QCOMPARE(controller.sessionCount(), 15);
    QCOMPARE(controller.loadCount(), 15);
    QCOMPARE(controller.maximumLiveSessionCount(), 15);

    requestRound(1);
    QCOMPARE(controller.sessionCount(), 15);
    QCOMPARE(controller.loadCount(), 15);
    const QList<LottieAssetRevision> loadRevisions = controller.loadRevisions();
    QCOMPARE(loadRevisions.size(), 15);
    for (const LottieRuntimeSourceResult &source : sources) {
        QCOMPARE(loadRevisions.count(source.asset.revision()), 1);
    }
    QCOMPARE(controller.renderCount(), 30);
    QCOMPARE(controller.mismatchCount(), 0);
    QCOMPARE(controller.maximumLiveSessionCount(), 15);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 15);
}

void lottie_runtime_service_test::overflowDoesNotPolluteResidentSessions()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("14"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("overflow-resident"), 15);
    QCOMPARE(sources.size(), 15);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    std::vector<LottieRuntimeSubscription> subscriptions;
    subscriptions.reserve(sources.size());
    for (int index = 0; index < sources.size(); ++index) {
        subscriptions.emplace_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
    }

    std::atomic<int> completions(0);
    const auto requestRound = [&](int round) {
        service->testHoldFrameSchedulerGuiBatch(true);
        for (int index = 0; index < sources.size(); ++index) {
            const LottieRuntimeFrameRequest request = frameRequestFor(
                sources.at(index), round, QSize(50, 50), {});
            QVERIFY(service->requestFrame(
                subscriptions.at(index), request,
                [&](const LottieRuntimeFrameResult &result) {
                    QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                    QCOMPARE(result.frameNumber, round);
                    QCOMPARE(result.physicalPixelSize, QSize(50, 50));
                    QVERIFY(result.frame.isValid());
                    ++completions;
                }));
        }

        QTRY_COMPARE(service->testFrameSchedulerStats().rasterJobCount,
                     (round + 1) * sources.size());
        QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
        QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
        service->testFlushFrameSchedulerGuiBatch();
        QCOMPARE(completions.load(), (round + 1) * sources.size());
    };

    requestRound(0);
    QCOMPARE(controller.sessionCount(), 15);
    QCOMPARE(controller.loadCount(), 15);
    QVERIFY(controller.maximumLiveSessionCount() <= 14);

    requestRound(1);
    // Thirteen stable residents remain warm; only the two overflow keys use
    // the transient slot again on the second frame round.
    QCOMPARE(controller.sessionCount(), 17);
    QCOMPARE(controller.loadCount(), 17);
    const QList<LottieAssetRevision> loadRevisions = controller.loadRevisions();
    QCOMPARE(loadRevisions.size(), 17);
    for (int index = 0; index < sources.size(); ++index) {
        QCOMPARE(loadRevisions.count(sources.at(index).asset.revision()),
                 index < 13 ? 1 : 2);
    }
    QCOMPARE(controller.renderCount(), 30);
    QCOMPARE(controller.mismatchCount(), 0);
    QVERIFY(controller.maximumLiveSessionCount() <= 14);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 17);
}

void lottie_runtime_service_test::capacityOneUsesOnlyTransientSessions()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("1"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("capacity-one-transient"), 3);
    QCOMPARE(sources.size(), 3);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    std::vector<LottieRuntimeSubscription> subscriptions;
    subscriptions.reserve(sources.size());
    for (int index = 0; index < sources.size(); ++index) {
        subscriptions.emplace_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
    }

    int completions = 0;
    const auto requestRound = [&](int round) {
        for (int index = 0; index < sources.size(); ++index) {
            QVERIFY(service->requestFrame(
                subscriptions.at(index),
                frameRequestFor(sources.at(index), round, QSize(50, 50), {}),
                [&](const LottieRuntimeFrameResult &result) {
                    QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                    QCOMPARE(result.frameNumber, round);
                    QVERIFY(result.frame.isValid());
                    ++completions;
                }));
            QTRY_COMPARE(completions, round * sources.size() + index + 1);
        }
    };

    requestRound(0);
    QCOMPARE(controller.sessionCount(), 3);
    QCOMPARE(controller.loadCount(), 3);
    QCOMPARE(controller.maximumLiveSessionCount(), 1);
    QTRY_COMPARE(controller.destructionCount(), 3);

    requestRound(1);
    QCOMPARE(controller.sessionCount(), 6);
    QCOMPARE(controller.loadCount(), 6);
    QCOMPARE(controller.maximumLiveSessionCount(), 1);
    QTRY_COMPARE(controller.destructionCount(), 6);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 6);
}

void lottie_runtime_service_test::sharesSessionAcrossHosts()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("16"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("shared-host-session"), 1);
    QCOMPARE(sources.size(), 1);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    LottieRuntimeSubscription firstHost = service->subscribe();
    LottieRuntimeSubscription secondHost = service->subscribe();
    QVERIFY(firstHost.isValid());
    QVERIFY(secondHost.isValid());

    int completions = 0;
    const auto requestFrameAndWait = [&](LottieRuntimeSubscription &subscription,
                                         int frame) {
        const int expectedCompletions = completions + 1;
        QVERIFY(service->requestFrame(
            subscription,
            frameRequestFor(sources.constFirst(), frame, QSize(50, 50), {}),
            [&](const LottieRuntimeFrameResult &result) {
                QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                QVERIFY(result.frame.isValid());
                ++completions;
            }));
        QTRY_COMPARE(completions, expectedCompletions);
    };

    requestFrameAndWait(firstHost, 0);
    requestFrameAndWait(secondHost, 1);
    QCOMPARE(controller.sessionCount(), 1);
    QCOMPARE(controller.loadCount(), 1);
    QCOMPARE(controller.destructionCount(), 0);

    service->suspendFrame(firstHost);
    requestFrameAndWait(secondHost, 2);
    QCOMPARE(controller.sessionCount(), 1);
    QCOMPARE(controller.loadCount(), 1);
    QCOMPARE(controller.destructionCount(), 0);

    service->suspendFrame(secondHost);
    QTRY_COMPARE(controller.destructionCount(), 1);
    QCOMPARE(controller.sessionCount(), 1);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 1);
}

void lottie_runtime_service_test::sessionDemandTracksRenderability_data()
{
    QTest::addColumn<bool>("releasesDemand");
    QTest::addColumn<bool>("destroysSubscription");

    QTest::newRow("visible") << false << false;
    QTest::newRow("paused") << false << false;
    QTest::newRow("completed") << false << false;
    QTest::newRow("hidden") << true << false;
    QTest::newRow("minimized") << true << false;
    QTest::newRow("detached") << true << false;
    QTest::newRow("invalid-size") << true << false;
    QTest::newRow("error") << true << false;
    QTest::newRow("destroyed") << true << true;
}

void lottie_runtime_service_test::sessionDemandTracksRenderability()
{
    QFETCH(bool, releasesDemand);
    QFETCH(bool, destroysSubscription);

    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("16"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("renderability-demand"), 1);
    QCOMPARE(sources.size(), 1);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    LottieRuntimeSubscription subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    int completions = 0;
    const auto requestFrameAndWait = [&](int frame) {
        const int expectedCompletions = completions + 1;
        QVERIFY(service->requestFrame(
            subscription,
            frameRequestFor(sources.constFirst(), frame, QSize(50, 50), {}),
            [&](const LottieRuntimeFrameResult &result) {
                QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
                QVERIFY(result.frame.isValid());
                ++completions;
            }));
        QTRY_COMPARE(completions, expectedCompletions);
    };

    requestFrameAndWait(0);
    QCOMPARE(controller.sessionCount(), 1);
    QCOMPARE(controller.loadCount(), 1);

    if (destroysSubscription) {
        subscription = LottieRuntimeSubscription();
    } else if (releasesDemand) {
        service->suspendFrame(subscription);
    }

    if (!releasesDemand) {
        requestFrameAndWait(1);
        QCOMPARE(controller.sessionCount(), 1);
        QCOMPARE(controller.loadCount(), 1);
        QCOMPARE(controller.destructionCount(), 0);
    } else {
        QTRY_COMPARE(controller.destructionCount(), 1);
        QCOMPARE(controller.sessionCount(), 1);

        // Restoring renderability only makes the next request eligible to
        // lazily create a session; suspension itself never recreates one.
        if (!destroysSubscription) {
            requestFrameAndWait(1);
            QCOMPARE(controller.sessionCount(), 2);
            QCOMPARE(controller.loadCount(), 2);
        }
    }

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(),
             releasesDemand && !destroysSubscription ? 2 : 1);
}

void lottie_runtime_service_test::failedSessionDoesNotEvictHealthyResidents()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("3"));
    IdentityBoundRasterSessionController controller;
    controller.failOnceForSize(QSize(51, 51));
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("failed-resident"), 3);
    QCOMPARE(sources.size(), 3);

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    std::vector<LottieRuntimeSubscription> subscriptions;
    for (int index = 0; index < sources.size(); ++index) {
        subscriptions.emplace_back(service->subscribe());
        QVERIFY(subscriptions.back().isValid());
    }

    int completions = 0;
    const auto requestFrameAndWait = [&](int index, int frame, const QSize &size,
                                         LottieRuntimeFrameResult::Failure failure) {
        const int expectedCompletions = completions + 1;
        QVERIFY(service->requestFrame(
            subscriptions.at(index),
            frameRequestFor(sources.at(index), frame, size, {}),
            [&, failure](const LottieRuntimeFrameResult &result) {
                QCOMPARE(result.failure, failure);
                if (failure == LottieRuntimeFrameResult::Failure::None) {
                    QVERIFY(result.frame.isValid());
                }
                ++completions;
            }));
        QTRY_COMPARE(completions, expectedCompletions);
    };

    requestFrameAndWait(0, 0, QSize(50, 50),
                        LottieRuntimeFrameResult::Failure::None);
    requestFrameAndWait(1, 0, QSize(51, 51),
                        LottieRuntimeFrameResult::Failure::Raster);
    requestFrameAndWait(2, 0, QSize(52, 52),
                        LottieRuntimeFrameResult::Failure::None);
    QTRY_COMPARE(controller.destructionCount(), 1);
    QCOMPARE(controller.sessionCount(), 3);
    QCOMPARE(controller.loadCount(), 3);
    QVERIFY(controller.maximumLiveSessionCount() <= 3);

    // The healthy resident sessions stay warm while the failed key is idle.
    requestFrameAndWait(0, 1, QSize(50, 50),
                        LottieRuntimeFrameResult::Failure::None);
    requestFrameAndWait(2, 1, QSize(52, 52),
                        LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(controller.sessionCount(), 3);
    QCOMPARE(controller.loadCount(), 3);

    // A later caller-triggered request may create the failed key again, but
    // there is no worker-side automatic retry and no healthy eviction.
    requestFrameAndWait(1, 1, QSize(51, 51),
                        LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(controller.sessionCount(), 4);
    QCOMPARE(controller.loadCount(), 4);
    QCOMPARE(controller.maximumLiveSessionCount(), 3);
    QCOMPARE(controller.mismatchCount(), 0);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 4);
}

void lottie_runtime_service_test::rasterSessionLimitUsesDemandBound_data()
{
    QTest::addColumn<QByteArray>("configuredLimit");
    QTest::addColumn<int>("distinctAnimations");
    QTest::addColumn<QString>("requestOrder");
    QTest::addColumn<int>("expectedSessionCreations");
    QTest::addColumn<int>("expectedCapacity");

    QTest::newRow("one-entry")
        << QByteArrayLiteral("1") << 2 << QStringLiteral("0,1,0") << 3 << 1;
    QTest::newRow("minimum-is-one")
        << QByteArrayLiteral("0") << 2 << QStringLiteral("0,1,0") << 3 << 1;
    QTest::newRow("two-entry-demand-bound")
        << QByteArrayLiteral("2") << 3 << QStringLiteral("0,1,0,2,1") << 5 << 1;
    QTest::newRow("invalid-uses-default-sixteen")
        << QByteArrayLiteral("invalid") << 5 << QStringLiteral("0,1,2,3,4,0") << 6 << 1;
    QTest::newRow("hard-maximum-is-sixteen")
        << QByteArrayLiteral("99") << 17
        << QStringLiteral("0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,16,1") << 19 << 1;
}

void lottie_runtime_service_test::rasterSessionLimitUsesDemandBound()
{
    QFETCH(QByteArray, configuredLimit);
    QFETCH(int, distinctAnimations);
    QFETCH(QString, requestOrder);
    QFETCH(int, expectedSessionCreations);
    QFETCH(int, expectedCapacity);

    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", configuredLimit);
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("bounded-session"), distinctAnimations);
    QCOMPARE(sources.size(), distinctAnimations);

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());
    const QStringList order = requestOrder.split(QLatin1Char(','));
    int completions = 0;
    QList<LottieRuntimeFrameResult> results;
    for (int requestIndex = 0; requestIndex < order.size(); ++requestIndex) {
        bool validIndex = false;
        const int sourceIndex = order.at(requestIndex).toInt(&validIndex);
        QVERIFY(validIndex);
        QVERIFY(sourceIndex >= 0 && sourceIndex < sources.size());
        const LottieRuntimeFrameRequest request = frameRequestFor(
            sources.at(sourceIndex), 0,
            QSize(17 + requestIndex, 17 + requestIndex), {});
        QVERIFY(service->requestFrame(subscription, request,
                                      [&](const LottieRuntimeFrameResult &result) {
            results.append(result);
            ++completions;
        }));
        QTRY_COMPARE(completions, requestIndex + 1);
    }

    QCOMPARE(results.size(), order.size());
    for (const LottieRuntimeFrameResult &result : std::as_const(results)) {
        QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
        QVERIFY(result.frame.isValid());
    }
    QCOMPARE(controller.sessionCount(), expectedSessionCreations);
    QCOMPARE(controller.loadCount(), expectedSessionCreations);
    QCOMPARE(controller.renderCount(), order.size());
    QCOMPARE(controller.mismatchCount(), 0);
    QCOMPARE(controller.maximumLiveSessionCount(), expectedCapacity);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), expectedSessionCreations);
}

void lottie_runtime_service_test::rasterSessionLimitIsReadWhenServiceIsCreated()
{
    ScopedEnvironmentVariable initialSessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("1"));
    IdentityBoundRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("fixed-session-limit"), 2);
    QCOMPARE(sources.size(), 2);

    // 此时调度器和栅格线程还没创建，但配置必须已经属于这个服务实例。
    auto *service = LottieRuntimeService::instance();
    ScopedEnvironmentVariable changedSessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("16"));
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    const QList<int> order({0, 1, 0});
    int completions = 0;
    for (int requestIndex = 0; requestIndex < order.size(); ++requestIndex) {
        const LottieRuntimeFrameRequest request = frameRequestFor(
            sources.at(order.at(requestIndex)), 0,
            QSize(21 + requestIndex, 21 + requestIndex), {});
        QVERIFY(service->requestFrame(subscription, request,
                                      [&](const LottieRuntimeFrameResult &result) {
            QCOMPARE(result.failure, LottieRuntimeFrameResult::Failure::None);
            ++completions;
        }));
        QTRY_COMPARE(completions, requestIndex + 1);
    }

    QCOMPARE(controller.sessionCount(), 3);
    QCOMPARE(controller.mismatchCount(), 0);
    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 3);
}

void lottie_runtime_service_test::rasterFailureEvictsOnlyMatchingSession()
{
    ScopedEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("2"));
    IdentityBoundRasterSessionController controller;
    controller.failOnceForSize(QSize(19, 19));
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope serviceScope;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QVector<LottieRuntimeSourceResult> sources = loadAnimationSources(
        &directory, QStringLiteral("isolated-session-failure"), 2);
    QCOMPARE(sources.size(), 2);

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());
    const QList<LottieRuntimeFrameRequest> requests({
        frameRequestFor(sources.at(0), 0, QSize(17, 17), {}),
        frameRequestFor(sources.at(1), 0, QSize(18, 18), {}),
        frameRequestFor(sources.at(1), 0, QSize(19, 19), {}),
        frameRequestFor(sources.at(0), 0, QSize(20, 20), {})
    });
    QList<LottieRuntimeFrameResult::Failure> failures;
    for (int index = 0; index < requests.size(); ++index) {
        QVERIFY(service->requestFrame(
            subscription, requests.at(index),
            [&](const LottieRuntimeFrameResult &result) {
                failures.append(result.failure);
            }));
        QTRY_COMPARE(failures.size(), index + 1);
    }

    QCOMPARE(failures, QList<LottieRuntimeFrameResult::Failure>({
        LottieRuntimeFrameResult::Failure::None,
        LottieRuntimeFrameResult::Failure::None,
        LottieRuntimeFrameResult::Failure::Raster,
        LottieRuntimeFrameResult::Failure::None
    }));
    QCOMPARE(controller.sessionCount(), 3);
    QCOMPARE(controller.loadCount(), 3);
    QCOMPARE(controller.renderCount(), 4);
    QCOMPARE(controller.destructionCount(), 2);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 3);
}

void lottie_runtime_service_test::rasterFailureDiscardsSessionUntilLaterRequest()
{
    RasterSessionRecorder recorder;
    LottieRuntimeRasterSessionTestScope rasterScope([&recorder] {
        return recorder.create();
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("raster-failure.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameRequest request;
    LottieRuntimeFrameResult firstResult;
    LottieRuntimeFrameResult secondResult;

    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       ++assetCompletions;
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(19, 13), result.asset.theme());
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      firstResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 1);
    QCOMPARE(firstResult.failure, LottieRuntimeFrameResult::Failure::Raster);
    QVERIFY(!firstResult.frame.isValid());
    QCOMPARE(recorder.sessionCount(), 1);
    QCOMPARE(recorder.renderCount(), 1);

    // A failed request is not retried by the worker and does not stop the runtime.
    QTest::qWait(100);
    QCOMPARE(frameCompletions.load(), 1);
    QCOMPARE(recorder.sessionCount(), 1);
    QVERIFY(!service->isStopping());

    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      secondResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 2);
    QCOMPARE(secondResult.failure, LottieRuntimeFrameResult::Failure::None);
    QVERIFY(secondResult.frame.isValid());
    QCOMPARE(secondResult.frame.size(), QSize(19, 13));
    QCOMPARE(recorder.sessionCount(), 2);
    QCOMPARE(recorder.renderCount(), 2);

    const QList<QThread *> renderThreads = recorder.renderThreads();
    QCOMPARE(renderThreads.size(), 2);
    QCOMPARE(renderThreads.first(), renderThreads.last());
    QVERIFY(renderThreads.first() != QCoreApplication::instance()->thread());

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
    const QList<QThread *> destructionThreads = recorder.destructionThreads();
    QCOMPARE(destructionThreads.size(), 2);
    QCOMPARE(destructionThreads.first(), destructionThreads.last());
    QCOMPARE(destructionThreads.first(), renderThreads.first());
}

void lottie_runtime_service_test::deterministicThemeFailureIsRememberedByRevision()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedArchive(
        &directory, QByteArrayLiteral("real-deterministic-theme-test"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameRequest request;
    LottieRuntimeFrameResult firstResult;
    LottieRuntimeFrameResult secondResult;

    QVERIFY(service->requestSource(subscription, source, QStringLiteral("dark"),
                                   [&](const LottieRuntimeSourceResult &result) {
                                       ++assetCompletions;
                                       QVERIFY(result.loaded);
                                       LottieRuntimeSourceResult corrupted = result;
                                       // Keep the validated theme metadata but make the copied
                                       // slot deterministically rejectable by ThorVG.
                                       corrupted.asset.setThemeSlotJson(
                                           QStringLiteral("dark"),
                                           QByteArrayLiteral("{\"broken\":true}"));
                                       request = frameRequestFor(
                                           corrupted, 0, QSize(32, 24), QStringLiteral("dark"));
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    QTest::ignoreMessage(
        QtWarningMsg,
        "LottieRenderer: ThorVG rejected dotLottie theme slots for: \"dark\"");
    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      firstResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 1);
    QCOMPARE(firstResult.failure,
             LottieRuntimeFrameResult::Failure::DeterministicTheme);
    QCOMPARE(firstResult.effectiveTheme, QStringLiteral("dark"));
    QCOMPARE(firstResult.physicalPixelSize, QSize(32, 24));
    QVERIFY(!firstResult.frame.isValid());

    const LottieRuntimeFrameSchedulerStats failedThemeStats =
        service->testFrameSchedulerStats();
    QCOMPARE(failedThemeStats.rasterJobCount, 1);
    QCOMPARE(failedThemeStats.cacheEntryCount, 0);
    QVERIFY(!service->isStopping());

    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      secondResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 2);
    QCOMPARE(secondResult.failure,
             LottieRuntimeFrameResult::Failure::DeterministicTheme);
    QVERIFY(!secondResult.frame.isValid());
    QCOMPARE(service->testFrameSchedulerStats().rasterJobCount,
             failedThemeStats.rasterJobCount);

    auto secondSubscription = service->subscribe();
    QVERIFY(secondSubscription.isValid());
    LottieRuntimeFrameResult sharedResult;
    QVERIFY(service->requestFrame(secondSubscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      sharedResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 3);
    QCOMPARE(sharedResult.failure,
             LottieRuntimeFrameResult::Failure::DeterministicTheme);
    QCOMPARE(service->testFrameSchedulerStats().rasterJobCount,
             failedThemeStats.rasterJobCount);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::staleDeterministicThemeFailureDoesNotPoisonRevision()
{
    ClassifiedFailureSessionController controller({
        LottieRuntimeFrameResult::Failure::DeterministicTheme,
        LottieRuntimeFrameResult::Failure::None,
        LottieRuntimeFrameResult::Failure::None
    });
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedArchive(
        &directory, QByteArrayLiteral("stale-deterministic-theme-test"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    std::atomic<int> sourceCompletions(0);
    LottieRuntimeSourceResult sourceResult;
    QVERIFY(service->requestSource(subscription, source, QStringLiteral("dark"),
                                   [&](const LottieRuntimeSourceResult &result) {
                                       sourceResult = result;
                                       ++sourceCompletions;
                                   }));
    QTRY_COMPARE(sourceCompletions.load(), 1);

    const LottieRuntimeFrameRequest darkRequest = frameRequestFor(
        sourceResult, 0, QSize(23, 17), QStringLiteral("dark"));
    const LottieRuntimeFrameRequest lightRequest = frameRequestFor(
        sourceResult, 0, QSize(23, 17), QStringLiteral("light"));
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameResult latestResult;

    service->testHoldFrameSchedulerGuiBatch(true);
    QVERIFY(service->requestFrame(subscription, darkRequest,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(controller.renderCount(), 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);

    QVERIFY(service->requestFrame(subscription, lightRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      latestResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(controller.renderCount(), 2);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(frameCompletions.load(), 1);
    QCOMPARE(latestResult.failure, LottieRuntimeFrameResult::Failure::None);

    const int jobsBeforeDarkRetry =
        service->testFrameSchedulerStats().rasterJobCount;
    QVERIFY(service->requestFrame(subscription, darkRequest,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      latestResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 2);
    QCOMPARE(latestResult.failure, LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(controller.renderCount(), 3);
    QCOMPARE(service->testFrameSchedulerStats().rasterJobCount,
             jobsBeforeDarkRetry + 1);

    LottieRuntimeService::destroyInstance();
}

void lottie_runtime_service_test::structuredFailuresDiscardSessionsAndCacheOnlyDeterministicThemes()
{
    ClassifiedFailureSessionController controller({
        LottieRuntimeFrameResult::Failure::DeterministicTheme,
        LottieRuntimeFrameResult::Failure::TransientTheme,
        LottieRuntimeFrameResult::Failure::TransientTheme,
        LottieRuntimeFrameResult::Failure::Raster,
        LottieRuntimeFrameResult::Failure::None
    });
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedArchive(
        &directory, QByteArrayLiteral("structured-failure-test"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    LottieRuntimeFrameRequest request;
    int sourceCompletions = 0;
    QVERIFY(service->requestSource(subscription, source, QStringLiteral("dark"),
                                   [&](const LottieRuntimeSourceResult &result) {
        QVERIFY(result.loaded);
        request = frameRequestFor(result, 0, QSize(23, 17), QStringLiteral("dark"));
        ++sourceCompletions;
    }));
    QTRY_COMPARE(sourceCompletions, 1);

    const auto requestAndWait = [&](LottieRuntimeFrameResult::Failure expected) {
        int completions = 0;
        LottieRuntimeFrameResult delivered;
        QVERIFY(service->requestFrame(subscription, request,
                                      [&](const LottieRuntimeFrameResult &result) {
            delivered = result;
            ++completions;
        }));
        QTRY_COMPARE(completions, 1);
        QCOMPARE(delivered.failure, expected);
    };

    requestAndWait(LottieRuntimeFrameResult::Failure::DeterministicTheme);
    QCOMPARE(controller.renderCount(), 1);
    QCOMPARE(controller.destructionCount(), 1);

    requestAndWait(LottieRuntimeFrameResult::Failure::DeterministicTheme);
    QCOMPARE(controller.renderCount(), 1);
    QCOMPARE(controller.sessionCount(), 1);

    request.effectiveTheme = QStringLiteral("light");
    requestAndWait(LottieRuntimeFrameResult::Failure::TransientTheme);
    requestAndWait(LottieRuntimeFrameResult::Failure::TransientTheme);
    requestAndWait(LottieRuntimeFrameResult::Failure::Raster);
    QCOMPARE(controller.renderCount(), 4);
    QCOMPARE(controller.sessionCount(), 4);
    QCOMPARE(controller.destructionCount(), 4);

    requestAndWait(LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(controller.renderCount(), 5);
    QCOMPARE(controller.sessionCount(), 5);
    QCOMPARE(controller.destructionCount(), 4);
    QCOMPARE(service->testFrameSchedulerStats().cacheEntryCount, 1);
    QVERIFY(!service->isStopping());

    LottieRuntimeService::destroyInstance();
    QCOMPARE(controller.destructionCount(), 5);
}

void lottie_runtime_service_test::destroyingSubscriptionDropsQueuedRasterResult()
{
    BlockingRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });

    QVERIFY(!LottieRuntimeService::hasInstance());
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("raster-subscription.json"));
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());
    auto barrierSubscription = service->subscribe();
    QVERIFY(barrierSubscription.isValid());

    std::atomic<int> assetCompletions(0);
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameRequest request;
    QVERIFY(service->requestSource(subscription, source, {},
                                   [&](const LottieRuntimeSourceResult &result) {
                                       ++assetCompletions;
                                       QVERIFY(result.loaded);
                                       request = frameRequestFor(
                                           result, 0, QSize(17, 11), result.asset.theme());
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++frameCompletions;
                                  }));
    QVERIFY(controller.waitUntilStarted());

    subscription = LottieRuntimeSubscription();
    controller.release();

    // This later request is a FIFO barrier: its GUI callback cannot arrive
    // before the worker has posted the callback for the destroyed subscription.
    std::atomic<int> barrierCompletions(0);
    QVERIFY(service->requestFrame(barrierSubscription, request,
                                  [&](const LottieRuntimeFrameResult &) {
                                      ++barrierCompletions;
                                  }));
    QTRY_COMPARE(barrierCompletions.load(), 1);
    QCOMPARE(frameCompletions.load(), 0);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

void lottie_runtime_service_test::rasterRequestRendersDotLottieThemeSlot()
{
    QVERIFY(!LottieRuntimeService::hasInstance());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedArchive(&directory);
    QVERIFY(!source.isEmpty());

    auto *service = LottieRuntimeService::instance();
    auto subscription = service->subscribe();
    QVERIFY(subscription.isValid());

    std::atomic<int> assetCompletions(0);
    std::atomic<int> frameCompletions(0);
    LottieRuntimeFrameResult frameResult;
    LottieRuntimeFrameRequest request;

    QVERIFY(service->requestSource(subscription, source, QStringLiteral("dark"),
                                   [&](const LottieRuntimeSourceResult &result) {
                                       ++assetCompletions;
                                       QVERIFY(result.loaded);
                                       QCOMPARE(result.asset.theme(), QStringLiteral("dark"));
                                       QVERIFY(result.asset.availableThemes().contains(
                                           QStringLiteral("light")));
                                       QVERIFY(result.asset.availableThemes().contains(
                                           QStringLiteral("dark")));

                                       request = frameRequestFor(
                                           result, 0, QSize(32, 24), result.asset.theme());
                                   }));
    QTRY_COMPARE(assetCompletions.load(), 1);

    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      frameResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 1);
    QCOMPARE(frameResult.failure, LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(frameResult.effectiveTheme, QStringLiteral("dark"));
    QVERIFY(frameResult.frame.isValid());
    QCOMPARE(frameResult.frame.size(), QSize(32, 24));

    const QColor center = frameResult.frame.image().pixelColor(16, 12);
    QVERIFY(center.alpha() > 240);
    QVERIFY(center.green() > 240);
    QVERIFY(center.red() < 10);
    QVERIFY(center.blue() < 10);

    LottieRuntimeFrameResult defaultThemeResult;
    request.effectiveTheme.clear();
    QVERIFY(service->requestFrame(subscription, request,
                                  [&](const LottieRuntimeFrameResult &result) {
                                      defaultThemeResult = result;
                                      ++frameCompletions;
                                  }));
    QTRY_COMPARE(frameCompletions.load(), 2);
    QCOMPARE(defaultThemeResult.failure, LottieRuntimeFrameResult::Failure::None);
    QCOMPARE(defaultThemeResult.effectiveTheme, QStringLiteral("light"));
    const QColor defaultCenter = defaultThemeResult.frame.image().pixelColor(16, 12);
    QVERIFY(defaultCenter.alpha() > 240);
    QVERIFY(defaultCenter.blue() > 240);
    QVERIFY(defaultCenter.red() < 10);
    QVERIFY(defaultCenter.green() < 10);

    LottieRuntimeService::destroyInstance();
    QCOMPARE(LottieRuntimeService::hasInstance(), false);
}

int main(int argc, char *argv[])
{
    QTemporaryDir temporaryHome;
    if (!temporaryHome.isValid()) {
        return 1;
    }

    qputenv("HOME", temporaryHome.path().toUtf8());
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    lottie_runtime_service_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-runtime-service.moc"
