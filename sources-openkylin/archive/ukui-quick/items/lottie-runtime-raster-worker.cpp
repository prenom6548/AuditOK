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

#include "lottie-runtime-raster-worker_p.h"

#include "lottie-renderer.h"

#include <QList>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <algorithm>
#include <iterator>
#include <list>
#include <mutex>
#include <utility>

namespace UkuiQuick {

namespace {

using SessionFactory = std::function<std::unique_ptr<LottieRasterSession>()>;

using RasterSessionKey = LottieRuntimeSessionKey;

RasterSessionKey sessionKeyFor(const LottieRuntimeFrameRequest &request)
{
    return {request.assetRevision, request.effectiveTheme};
}

struct RasterSessionEntry
{
    RasterSessionKey key;
    std::unique_ptr<LottieRasterSession> session;
};

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
std::mutex s_sessionFactoryMutex;
SessionFactory s_testSessionFactory;
int s_liveRasterWorkerCount = 0;
#endif

class ThorVGLottieRasterSession final : public LottieRasterSession
{
public:
    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }

        const RasterSessionKey key = sessionKeyFor(request);
        if (!m_loaded || !(m_key == key)) {
            m_loaded = false;
            const LottieRenderer::Failure loadFailure =
                m_renderer.load(request.asset, request.effectiveTheme);
            if (loadFailure != LottieRenderer::Failure::None) {
                return runtimeFailure(loadFailure);
            }
            m_key = key;
            m_loaded = true;
        }

        LottieRenderer::FrameResult rendered =
            m_renderer.imageForFrame(request.frame, request.physicalPixelSize);
        if (rendered.failure != LottieRenderer::Failure::None || rendered.image.isNull()) {
            return runtimeFailure(rendered.failure);
        }

        *image = std::move(rendered.image);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    static LottieRuntimeFrameResult::Failure runtimeFailure(
        LottieRenderer::Failure failure)
    {
        switch (failure) {
        case LottieRenderer::Failure::DeterministicTheme:
            return LottieRuntimeFrameResult::Failure::DeterministicTheme;
        case LottieRenderer::Failure::TransientTheme:
            return LottieRuntimeFrameResult::Failure::TransientTheme;
        case LottieRenderer::Failure::None:
            return LottieRuntimeFrameResult::Failure::None;
        case LottieRenderer::Failure::Source:
        case LottieRenderer::Failure::Raster:
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        return LottieRuntimeFrameResult::Failure::Raster;
    }

    LottieRenderer m_renderer;
    RasterSessionKey m_key;
    bool m_loaded = false;
};

} // namespace

class LottieRuntimeRasterWorker::Private
{
public:
    struct Job
    {
        enum class Type {
            Render,
            RetainSession,
            ReleaseSession
        };

        Type type = Type::Render;
        LottieRuntimeFrameRequest request;
        RasterSessionKey sessionKey;
        Completion completion;
    };

    QMutex mutex;
    QWaitCondition hasWork;
    QList<Job> jobs;
    QObject *deliveryContext = nullptr;
    SessionFactory sessionFactory;
    int maximumSessions = LottieRuntimeRasterSessionCache::kMinimumEntries;
    bool stopping = false;
    bool started = false;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    bool resetSessions = false;
#endif
};

LottieImmutableFrame::LottieImmutableFrame(QImage image)
    : m_image(QSharedPointer<const QImage>::create(std::move(image)))
{
}

bool LottieImmutableFrame::isValid() const
{
    return m_image && !m_image->isNull();
}

QSize LottieImmutableFrame::size() const
{
    return m_image ? m_image->size() : QSize();
}

const QImage &LottieImmutableFrame::image() const
{
    static const QImage emptyImage;
    return m_image ? *m_image : emptyImage;
}

LottieRuntimeRasterWorker::LottieRuntimeRasterWorker(
    int maximumSessions, QObject *deliveryContext)
    : d(new Private)
{
    d->deliveryContext = deliveryContext;
    d->maximumSessions = qBound(
        LottieRuntimeRasterSessionCache::kMinimumEntries,
        maximumSessions,
        LottieRuntimeRasterSessionCache::kMaximumEntries);

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    std::lock_guard<std::mutex> lock(s_sessionFactoryMutex);
    d->sessionFactory = s_testSessionFactory
        ? s_testSessionFactory
        : [] {
              return std::make_unique<ThorVGLottieRasterSession>();
          };
    ++s_liveRasterWorkerCount;
#else
    d->sessionFactory = [] {
        return std::make_unique<ThorVGLottieRasterSession>();
    };
#endif
}

LottieRuntimeRasterWorker::~LottieRuntimeRasterWorker()
{
    stop();
    d.reset();

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    std::lock_guard<std::mutex> lock(s_sessionFactoryMutex);
    Q_ASSERT(s_liveRasterWorkerCount > 0);
    --s_liveRasterWorkerCount;
#endif
}

bool LottieRuntimeRasterWorker::enqueue(const LottieRuntimeFrameRequest &request,
                                        Completion completion)
{
    if (!completion) {
        return false;
    }

    QMutexLocker locker(&d->mutex);
    if (d->stopping) {
        return false;
    }

    Private::Job job;
    job.request = request;
    job.completion = std::move(completion);
    d->jobs.append(std::move(job));
    if (!d->started) {
        d->started = true;
        start();
    }
    d->hasWork.wakeOne();
    return true;
}

bool LottieRuntimeRasterWorker::retainSession(const LottieRuntimeSessionKey &key)
{
    QMutexLocker locker(&d->mutex);
    if (d->stopping) {
        return false;
    }

    Private::Job job;
    job.type = Private::Job::Type::RetainSession;
    job.sessionKey = key;
    d->jobs.append(std::move(job));
    if (!d->started) {
        d->started = true;
        start();
    }
    d->hasWork.wakeOne();
    return true;
}

bool LottieRuntimeRasterWorker::releaseSession(const LottieRuntimeSessionKey &key)
{
    QMutexLocker locker(&d->mutex);
    if (d->stopping) {
        return false;
    }

    Private::Job job;
    job.type = Private::Job::Type::ReleaseSession;
    job.sessionKey = key;
    d->jobs.append(std::move(job));
    if (!d->started) {
        d->started = true;
        start();
    }
    d->hasWork.wakeOne();
    return true;
}

void LottieRuntimeRasterWorker::stop()
{
    bool started = false;
    {
        QMutexLocker locker(&d->mutex);
        d->stopping = true;
        d->jobs.clear();
        d->hasWork.wakeAll();
        started = d->started;
    }

    if (started) {
        wait();
    }
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
void LottieRuntimeRasterWorker::testResetSessions()
{
    QMutexLocker locker(&d->mutex);
    if (!d->stopping) {
        d->resetSessions = true;
    }
}
#endif

void LottieRuntimeRasterWorker::run()
{
    // 所有会话都属于这个函数，因此只会在唯一栅格线程中创建、使用和销毁。
    // activeDemandOrder 只在需求变化命令中改变，帧访问不会重排它。容量不足时
    // 前 maximumSessions - 1 个稳定需求占 resident，其他需求使用一次性的
    // transient 会话，从而避免逐帧轮转污染 resident 集合。
    std::list<RasterSessionEntry> sessions;
    struct ActiveDemand
    {
        RasterSessionKey key;
        int count = 0;
    };
    std::list<ActiveDemand> activeDemandOrder;
    std::list<RasterSessionKey> residentDemandOrder;

    const auto containsKey = [](const std::list<RasterSessionKey> &keys,
                                const RasterSessionKey &key) {
        return std::find(keys.begin(), keys.end(), key) != keys.end();
    };

    const auto findSession = [&sessions](const RasterSessionKey &key) {
        return std::find_if(
            sessions.begin(), sessions.end(),
            [&key](const RasterSessionEntry &entry) {
                return entry.key == key;
            });
    };

    const auto rebalanceResidentDemands = [&] {
        std::list<RasterSessionKey> desiredResidents;
        const int activeCount = static_cast<int>(activeDemandOrder.size());
        const int residentLimit = activeCount <= d->maximumSessions
            ? activeCount
            : (d->maximumSessions == 1 ? 0 : d->maximumSessions - 1);
        int selected = 0;
        for (const ActiveDemand &demand : activeDemandOrder) {
            if (selected++ >= residentLimit) {
                break;
            }
            desiredResidents.push_back(demand.key);
        }

        // 需求变化是唯一允许 resident 发生变化的时机。这里销毁被 demote
        // 的会话；它们仍然是 active demand，但下次访问时必须走 transient slot。
        for (auto it = sessions.begin(); it != sessions.end();) {
            if (!containsKey(desiredResidents, it->key)) {
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
        residentDemandOrder = std::move(desiredResidents);
    };

    while (true) {
        Private::Job job;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        bool resetSessions = false;
#endif
        {
            QMutexLocker locker(&d->mutex);
            while (d->jobs.isEmpty() && !d->stopping) {
                d->hasWork.wait(locker.mutex());
            }

            if (d->stopping) {
                break;
            }

            job = std::move(d->jobs.takeFirst());
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
            resetSessions = d->resetSessions;
            d->resetSessions = false;
#endif
        }

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        if (resetSessions) {
            sessions.clear();
        }
#endif

        if (job.type == Private::Job::Type::RetainSession) {
            auto demandIt = std::find_if(
                activeDemandOrder.begin(), activeDemandOrder.end(),
                [&job](const ActiveDemand &demand) {
                    return demand.key == job.sessionKey;
                });
            if (demandIt == activeDemandOrder.end()) {
                activeDemandOrder.push_back({job.sessionKey, 1});
                rebalanceResidentDemands();
            } else {
                ++demandIt->count;
            }
            continue;
        }
        if (job.type == Private::Job::Type::ReleaseSession) {
            const auto demandIt = std::find_if(
                activeDemandOrder.begin(), activeDemandOrder.end(),
                [&job](const ActiveDemand &demand) {
                    return demand.key == job.sessionKey;
                });
            if (demandIt != activeDemandOrder.end()) {
                if (--demandIt->count == 0) {
                    activeDemandOrder.erase(demandIt);
                    rebalanceResidentDemands();
                }
            }
            continue;
        }

        LottieRuntimeFrameResult result;
        result.assetRevision = job.request.assetRevision;
        result.effectiveTheme = job.request.effectiveTheme;
        result.frameNumber = job.request.frame;
        result.physicalPixelSize = job.request.physicalPixelSize;

        const RasterSessionKey sessionKey = sessionKeyFor(job.request);
        const bool resident = containsKey(residentDemandOrder, sessionKey);
        auto sessionIt = resident ? findSession(sessionKey) : sessions.end();
        std::unique_ptr<LottieRasterSession> transientSession;
        if (resident && sessionIt == sessions.end()) {
            // resident session 是惰性创建的。rebalance 已经预留了 transient
            // slot，因此这里创建不会把 live session 数量推过配置上限。
            if (static_cast<int>(sessions.size()) < d->maximumSessions) {
                transientSession = d->sessionFactory
                    ? d->sessionFactory()
                    : nullptr;
                if (transientSession) {
                    sessions.push_back({sessionKey, std::move(transientSession)});
                    sessionIt = std::prev(sessions.end());
                }
            }
        }

        LottieRasterSession *session = nullptr;
        if (resident) {
            session = sessionIt != sessions.end() ? sessionIt->session.get() : nullptr;
        } else {
            // Overflow key 不进入 sessions；这个唯一的局部对象在本次 frame
            // 完成后销毁，因此它不会逐帧改变 resident 集合。
            transientSession = d->sessionFactory ? d->sessionFactory() : nullptr;
            session = transientSession.get();
        }

        QImage image;
        const auto rasterStartedAt = LottieRuntimeDiagnosticsClock::now();
        result.failure = session
            ? session->render(job.request, &image)
            : LottieRuntimeFrameResult::Failure::Raster;
        result.diagnostics.finish(rasterStartedAt);
        if (result.failure == LottieRuntimeFrameResult::Failure::None && !image.isNull()) {
            result.failure = LottieRuntimeFrameResult::Failure::None;
            result.frame = LottieImmutableFrame(std::move(image));
        } else {
            if (result.failure == LottieRuntimeFrameResult::Failure::None) {
                result.failure = LottieRuntimeFrameResult::Failure::Raster;
            }
            // 当前请求失败后只丢弃它使用的会话；不自动重试，也不会影响缓存中
            // 的其他动画。调用方以后再次请求这个动画时才会创建新会话。
            if (resident && sessionIt != sessions.end()) {
                sessions.erase(sessionIt);
            }
        }

        bool stopping = false;
        {
            QMutexLocker locker(&d->mutex);
            stopping = d->stopping;
        }
        if (stopping || !job.completion || !d->deliveryContext) {
            continue;
        }

        const Completion completion = job.completion;
        const LottieRuntimeFrameResult deliveredResult = result;
        QMetaObject::invokeMethod(d->deliveryContext,
                                  [completion, deliveredResult] {
                                      completion(deliveredResult);
                                  },
                                  Qt::QueuedConnection);
    }

    sessions.clear();
}

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
LottieRuntimeRasterSessionTestScope::LottieRuntimeRasterSessionTestScope(Factory factory)
{
    Q_ASSERT(factory);
    std::lock_guard<std::mutex> lock(s_sessionFactoryMutex);
    Q_ASSERT(!s_testSessionFactory);
    Q_ASSERT(s_liveRasterWorkerCount == 0);
    s_testSessionFactory = std::move(factory);
}

LottieRuntimeRasterSessionTestScope::~LottieRuntimeRasterSessionTestScope()
{
    std::lock_guard<std::mutex> lock(s_sessionFactoryMutex);
    Q_ASSERT(s_liveRasterWorkerCount == 0);
    Q_ASSERT(s_testSessionFactory);
    s_testSessionFactory = {};
}
#endif

} // namespace UkuiQuick
