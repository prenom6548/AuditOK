#include <QColor>
#include <QElapsedTimer>
#include <QFile>
#include <QGSettings/QGSettings>
#include <QGuiApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QSemaphore>
#include <QSGNode>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThread>
#include <QWaitCondition>
#include <QtTest>
#include <qqml.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#define private public
#include "lottie-attached.h"
#undef private
#include "icon.h"
#include "lottie-item.h"
#include "lottie-monotonic-clock_p.h"
#include "lottie-runtime-test-helpers_p.h"
#include "lottie-runtime-raster-worker_p.h"
#include "lottie-runtime-service_p.h"
#include "ukui/dt-theme.h"
#include "ukui/lottie/lottie-cache-manager.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

enum class FirstCommitChange {
    Seek,
    Theme,
    Size
};

Q_DECLARE_METATYPE(FirstCommitChange)

namespace {

constexpr int kShutdownBarrierTimeoutMilliseconds = 5000;
constexpr int kShutdownBarrierExecutionTimeoutMilliseconds = 30000;

class ManualLottieMonotonicClock final : public LottieMonotonicClock
{
public:
    qint64 nowNsecs() const override
    {
        return m_nowNsecs;
    }

    void advance(qint64 elapsedNsecs)
    {
        m_nowNsecs += elapsedNsecs;
    }

private:
    qint64 m_nowNsecs = 0;
};

struct ClockControlledAttached
{
    LottieAttached *attached = nullptr;
    ManualLottieMonotonicClock *clock = nullptr;
};

ClockControlledAttached createClockControlledAttached(
    LottieItem *host,
    bool sceneActive = true)
{
    auto clock = std::make_unique<ManualLottieMonotonicClock>();
    auto *clockControl = clock.get();
    auto *attached = new LottieAttached(host, std::move(clock));
    attached->setHostRenderSize(QSize(16, 16));
    attached->setSceneActive(sceneActive);
    return {attached, clockControl};
}

QByteArray animationJson(int endFrame = 2)
{
    static constexpr char animation[] = R"({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"lottie-item-test",
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
                {"ty":"rc","d":1,"s":{"a":0,"k":[16,16]},"p":{"a":0,"k":[0,0]},"r":{"a":0,"k":0},"nm":"Rectangle Path 1"},
                {"ty":"fl","c":{"a":0,"k":[1,0,0,1]},"o":{"a":0,"k":100},"r":1,"bm":0,"nm":"Fill 1"},
                {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}
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

QString writeAnimation(QTemporaryDir *directory,
                       const QString &fileName = QStringLiteral("animation.json"),
                       int endFrame = 2)
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

LottieAttached *attachedPropertiesFor(LottieItem *item)
{
    return qobject_cast<LottieAttached *>(
        qmlAttachedPropertiesObject<LottieAttached>(item, true));
}

void watchLottieAttachedSignals(LottieAttached *controller,
                                std::atomic<int> *signalCount)
{
    const auto recordSignal = [signalCount] {
        signalCount->fetch_add(1, std::memory_order_relaxed);
    };
    QObject::connect(controller, &LottieAttached::sourceChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::playingChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::loopChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::speedChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::currentFrameChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::totalFrameChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::fpsChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::themeChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::preferredThemeChanged,
                     controller, recordSignal);
    QObject::connect(controller, &LottieAttached::followHostThemeChanged,
                     controller, recordSignal);
    QObject::connect(controller, &LottieAttached::availableThemesChanged,
                     controller, recordSignal);
    QObject::connect(controller, &LottieAttached::statusChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::errorStringChanged,
                     controller, recordSignal);
    QObject::connect(controller, &LottieAttached::activeChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::completedChanged, controller, recordSignal);
    QObject::connect(controller, &LottieAttached::frameImageChanged,
                     controller, recordSignal);
    QObject::connect(controller, &LottieAttached::loaded, controller, recordSignal);
}

class TestLottieItem : public LottieItem
{
public:
    using LottieItem::LottieItem;

    QSGNode *renderWithoutSceneGraph(QSGNode *node)
    {
        return updatePaintNode(node, nullptr);
    }

    void notifyDevicePixelRatioChanged(qreal devicePixelRatio)
    {
        itemChange(ItemDevicePixelRatioHasChanged, ItemChangeData(devicePixelRatio));
    }
};

class ColorRasterSession final : public LottieRasterSession
{
public:
    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        const QColor color = request.frame != 0
            ? QColor(Qt::green)
            : (request.asset.frameCount() == 3
                   ? QColor(Qt::blue)
                   : (m_renderCount++ == 0 ? QColor(Qt::red) : QColor(Qt::blue)));
        image->fill(color);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    int m_renderCount = 0;
};

class ThemeColorRasterSession final : public LottieRasterSession
{
public:
    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(request.effectiveTheme == QStringLiteral("dark")
                        ? QColor(Qt::white)
                        : QColor(51, 51, 51));
        return LottieRuntimeFrameResult::Failure::None;
    }
};

class ThemeLoadRasterSessionController;

class ThemeLoadRasterSession final : public LottieRasterSession
{
public:
    explicit ThemeLoadRasterSession(ThemeLoadRasterSessionController *controller)
        : m_controller(controller)
    {
    }

    ~ThemeLoadRasterSession() override;

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override;

private:
    ThemeLoadRasterSessionController *m_controller = nullptr;
    bool m_loaded = false;
};

class ThemeLoadRasterSessionController
{
public:
    std::unique_ptr<LottieRasterSession> create()
    {
        return std::make_unique<ThemeLoadRasterSession>(this);
    }

    void recordLoad(const LottieRuntimeFrameRequest &request)
    {
        QMutexLocker locker(&m_mutex);
        m_loads.append({request.assetRevision, request.effectiveTheme});
    }

    int loadCount(const LottieAssetRevision &revision, const QString &theme) const
    {
        QMutexLocker locker(&m_mutex);
        return static_cast<int>(std::count_if(
            m_loads.cbegin(), m_loads.cend(), [&](const Load &load) {
                return load.revision == revision && load.theme == theme;
            }));
    }

    int destructionCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_destructionCount;
    }

    void recordDestruction()
    {
        QMutexLocker locker(&m_mutex);
        ++m_destructionCount;
    }

    void failNextRender()
    {
        QMutexLocker locker(&m_mutex);
        m_failNextRender = true;
    }

    bool takeRenderFailure()
    {
        QMutexLocker locker(&m_mutex);
        const bool shouldFail = m_failNextRender;
        m_failNextRender = false;
        return shouldFail;
    }

private:
    struct Load
    {
        LottieAssetRevision revision;
        QString theme;
    };

    mutable QMutex m_mutex;
    QList<Load> m_loads;
    int m_destructionCount = 0;
    bool m_failNextRender = false;
};

LottieRuntimeFrameResult::Failure ThemeLoadRasterSession::render(
    const LottieRuntimeFrameRequest &request, QImage *image)
{
    if (!image) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }
    if (!m_loaded) {
        m_loaded = true;
        m_controller->recordLoad(request);
    }
    if (m_controller->takeRenderFailure()) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }
    *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::green);
    return LottieRuntimeFrameResult::Failure::None;
}

ThemeLoadRasterSession::~ThemeLoadRasterSession()
{
    m_controller->recordDestruction();
}

struct BlockingRasterState
{
    std::atomic<int> requestCount {0};
    std::atomic<int> renderedCount {0};
    std::atomic<bool> firstRequestStarted {false};
    std::atomic<bool> blockNextRequest {false};
    std::atomic<bool> waitTimedOut {false};
    QSemaphore releaseFirstRequest;
    bool firstRequestFails = false;
    bool encodeFrameIdentity = false;
};

QColor frameIdentityColor(int frame)
{
    return QColor::fromHsv((frame * 53) % 360, 255, 255);
}

struct BlockingSourceState
{
    std::atomic<int> requestCount {0};
    std::atomic<bool> firstRequestStarted {false};
    QSemaphore releaseFirstRequest;
};

LottieRuntimeSourceResult loadTestSource(
    const LottieRuntimeSourceRequest &request,
    const LottieRuntimeService::SourceCancellation &isCancelled);

class HostShutdownAssetState
{
public:
    HostShutdownAssetState(QString activeSource, QString queuedSource)
        : m_blockedSources({std::move(activeSource), std::move(queuedSource)})
    {
    }

    LottieRuntimeSourceResult run(
        const LottieRuntimeSourceRequest &request,
        const LottieRuntimeService::SourceCancellation &isCancelled)
    {
        if (!m_blockedSources.contains(request.source.canonicalLocator())) {
            return loadTestSource(request, isCancelled);
        }

        QMutexLocker locker(&m_mutex);
        m_startedSources.append(request.source.canonicalLocator());
        ++m_activeCount;
        m_startedCondition.wakeAll();

        QElapsedTimer timer;
        timer.start();
        while (!m_released) {
            if (isCancelled && isCancelled()) {
                if (!m_cancellationObserved) {
                    m_cancellationObserved = true;
                    ++m_cancelledCount;
                    m_cancellationCondition.wakeAll();
                }
                m_released = true;
                m_releaseCondition.wakeAll();
                break;
            }

            const int remaining = kShutdownBarrierExecutionTimeoutMilliseconds - timer.elapsed();
            if (remaining <= 0) {
                m_timedOut = true;
                m_released = true;
                m_releaseCondition.wakeAll();
                break;
            }
            m_releaseCondition.wait(locker.mutex(), qMin(remaining, 10));
        }

        --m_activeCount;
        return {};
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

    QStringList startedSources() const
    {
        QMutexLocker locker(&m_mutex);
        return m_startedSources;
    }

    int activeCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_activeCount;
    }

    int cancelledCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_cancelledCount;
    }

    bool timedOut() const
    {
        QMutexLocker locker(&m_mutex);
        return m_timedOut;
    }

private:
    const QStringList m_blockedSources;
    mutable QMutex m_mutex;
    QWaitCondition m_startedCondition;
    QWaitCondition m_cancellationCondition;
    QWaitCondition m_releaseCondition;
    QStringList m_startedSources;
    int m_activeCount = 0;
    int m_cancelledCount = 0;
    bool m_cancellationObserved = false;
    bool m_timedOut = false;
    bool m_released = false;
};

class BlockingRasterSession final : public LottieRasterSession
{
public:
    explicit BlockingRasterSession(BlockingRasterState *state)
        : m_state(state)
    {
    }

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        const bool firstRequest = m_state->requestCount.fetch_add(1) == 0;
        const bool blockRequest = firstRequest || m_state->blockNextRequest.exchange(false);
        if (blockRequest) {
            m_state->firstRequestStarted.store(true);
            if (!m_state->releaseFirstRequest.tryAcquire(
                    1, kShutdownBarrierExecutionTimeoutMilliseconds)) {
                m_state->waitTimedOut.store(true);
                return LottieRuntimeFrameResult::Failure::Raster;
            }
            if (m_state->firstRequestFails) {
                return LottieRuntimeFrameResult::Failure::Raster;
            }
        }
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(m_state->encodeFrameIdentity
                        ? frameIdentityColor(request.frame)
                        : (request.frame == 0 ? Qt::red : Qt::green));
        m_state->renderedCount.fetch_add(1);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    BlockingRasterState *m_state;
};

LottieRuntimeSourceResult loadTestSource(
    const LottieRuntimeSourceRequest &request,
    const LottieRuntimeService::SourceCancellation &isCancelled)
{
    LottieRuntimeSourceResult result;
    result.asset = LottieCacheManager::instance()->requestAsset(
        request.source, isCancelled).asset;
    result.loaded = result.asset.isValid();
    return result;
}

LottieRuntimeSourceResult loadThemedTestSource(
    const LottieRuntimeSourceRequest &request,
    const LottieRuntimeService::SourceCancellation &isCancelled)
{
    LottieRuntimeSourceResult result = loadTestSource(request, isCancelled);
    if (result.loaded) {
        result.asset.setAvailableThemes(
            {QStringLiteral("light"), QStringLiteral("dark")});
        result.asset.setManifestInitialThemeId(QStringLiteral("light"));
    }
    return result;
}

QImage grabItemImage(QQuickItem *item)
{
    const QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    if (!result) {
        return {};
    }
    QSignalSpy readySpy(result.data(), &QQuickItemGrabResult::ready);
    if (readySpy.isEmpty() && !readySpy.wait(1000)) {
        return {};
    }
    return result->image();
}

QSize physicalSize(const QSizeF &logicalSize, qreal devicePixelRatio)
{
    return QSize(qMax(1, qRound(logicalSize.width() * devicePixelRatio)),
                 qMax(1, qRound(logicalSize.height() * devicePixelRatio)));
}

class ScopedSemaphoreRelease
{
public:
    explicit ScopedSemaphoreRelease(QSemaphore *semaphore)
        : m_semaphore(semaphore)
    {
    }

    ~ScopedSemaphoreRelease()
    {
        release();
    }

    void release()
    {
        if (m_semaphore) {
            m_semaphore->release();
            m_semaphore = nullptr;
        }
    }

    ScopedSemaphoreRelease(const ScopedSemaphoreRelease &) = delete;
    ScopedSemaphoreRelease &operator=(const ScopedSemaphoreRelease &) = delete;

private:
    QSemaphore *m_semaphore = nullptr;
};

void makeTestWindowRenderable(QQuickWindow &window)
{
    window.setGeometry(0, 0, 32, 32);
    window.show();
}

class ScopedTestEnvironmentVariable
{
public:
    ScopedTestEnvironmentVariable(const char *name, const QByteArray &value)
        : m_name(name)
        , m_previousValue(qgetenv(name))
        , m_wasSet(qEnvironmentVariableIsSet(name))
    {
        qputenv(m_name.constData(), value);
    }

    ~ScopedTestEnvironmentVariable()
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

}

class lottie_item_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void sourceEnsuresStableControllerAndPublishesLoading();
    void hostThemeAuthorityIncludesHighlightMode();
    void metadataPrecedesVisualReadyAndLoadedFiresLast();
    void firstCommitReentrancyPreservesReadyAndPlayback_data();
    void firstCommitReentrancyPreservesReadyAndPlayback();
    void rejectedReentrantSeekKeepsFirstCommitInError();
    void firstCommitSourceReplacementCancelsSupersededNotifications();
    void failureSourceReplacementCancelsSupersededNotifications();
    void preReadyFrameSelectionIsClampedAndCommittedFirst();
    void sourceReplacementRetainsReplacesAndClearsPixels();
    void initialRasterFailureClearsPixelsAndRetainsMetadata();
    void staleInitialFrameCannotPublishReadyOrPixels_data();
    void staleInitialFrameCannotPublishReadyOrPixels();
    void staleSourceFailureCannotAffectReplacement();
    void sourceEmitsOnlyOnActualChanges();
    void attachedControllerFollowsSourceSizeAndSceneChanges();
    void themeChangeReleasesOldSessionBeforeSynchronousReentry();
    void hostRenderabilityReleasesSessionDemand();
    void continueWhenHiddenCatchesUpWithoutRenderingHiddenFrames();
    void continueWhenSourceLoadsWhileAlreadyHidden();
    void continueWhenHiddenCompletesOnlyAfterFinalFrameCommit();
    void continueWhenHiddenRebasesAtControlChanges();
    void hiddenPlaybackPolicyChangesApplyAtTheChangeTime();
    void restartWhenShownResetsHiddenPlaybackToFrameZero();
    void rapidResizeDropsStaleLottieItemFrame();
    void visibleHostStressMatrix_data();
    void visibleHostStressMatrix();
    void shutdownDestroysRealHostsBeforeAboutToQuit();
    void paintWithoutWindowReleasesTheNode();
    void sceneGraphRendersFallbackAndAnimation();
};

void lottie_item_test::initTestCase()
{
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
}

void lottie_item_test::sourceEnsuresStableControllerAndPublishesLoading()
{
    LottieItem item;
    QVERIFY(!item.lottieAttached());

    item.setSource(QStringLiteral("pending.json"));

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->source(), QStringLiteral("pending.json"));
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QVERIFY(!attached->active());

    item.setSource({});
    QCOMPARE(item.lottieAttached(), attached);
    QCOMPARE(attached->status(), LottieAttached::Null);
    QVERIFY(!attached->active());
}

void lottie_item_test::hostThemeAuthorityIncludesHighlightMode()
{
    constexpr auto styleSettings = "org.ukui.style";
    constexpr auto styleColorKey = "styleName";
    QGSettings settings(styleSettings);
    const QString originalStyleColor = settings.get(styleColorKey).toString();
    auto *themeConfig = ThemeGlobalConfig::instance();
    settings.set(styleColorKey, QStringLiteral("ukui-light"));
    QTRY_VERIFY(!themeConfig->isDarkTheme());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("host-theme.json"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ThemeColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadThemedTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);
    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(51, 51, 51));

    item.setMode(LottieItem::Highlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::white));
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8), QColor(Qt::white));
    item.setMode(LottieItem::Normal);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));

    item.setMode(LottieItem::ForceHighlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));
    item.setMode(LottieItem::AutoHighlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));

    settings.set(styleColorKey, QStringLiteral("ukui-dark"));
    QTRY_VERIFY(themeConfig->isDarkTheme());
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));

    attached->setTheme(QStringLiteral("light"));
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));
    item.setMode(LottieItem::Highlight);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    settings.set(styleColorKey, QStringLiteral("ukui-light"));
    QTRY_VERIFY(!themeConfig->isDarkTheme());
    QCOMPARE(attached->theme(), QStringLiteral("light"));

    attached->resetThemePreference();
    QVERIFY(attached->followHostTheme());
    QCOMPARE(attached->preferredTheme(), QString());
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));

    item.setMode(LottieItem::Normal);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));

    settings.set(styleColorKey, originalStyleColor);
}

void lottie_item_test::metadataPrecedesVisualReadyAndLoadedFiresLast()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QVERIFY(!attached->active());

    bool metadataObservedWhileLoading = false;
    connect(attached, &LottieAttached::totalFrameChanged, attached, [&] {
        metadataObservedWhileLoading = attached->totalFrame() == 2 &&
            attached->status() == LottieAttached::Loading &&
            !attached->active();
    });

    QStringList readySignals;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (!attached->committedImage().isNull()) {
            readySignals.append(QStringLiteral("frame"));
        }
    });
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        if (attached->status() == LottieAttached::Ready) {
            readySignals.append(QStringLiteral("status"));
        }
    });
    connect(attached, &LottieAttached::activeChanged, attached, [&] {
        if (attached->active()) {
            readySignals.append(QStringLiteral("active"));
        }
    });
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    connect(attached, &LottieAttached::loaded, attached, [&] {
        readySignals.append(QStringLiteral("loaded"));
        QCOMPARE(attached->status(), LottieAttached::Ready);
        QVERIFY(attached->active());
        QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    });

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(metadataObservedWhileLoading);
    QCOMPARE(attached->totalFrame(), 2);
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(readySignals,
             QStringList({QStringLiteral("frame"), QStringLiteral("status"),
                          QStringLiteral("active"), QStringLiteral("loaded")}));
}

void lottie_item_test::firstCommitReentrancyPreservesReadyAndPlayback()
{
    QFETCH(FirstCommitChange, reentrantChange);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadThemedTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());

    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    attached->setLoop(true);
    attached->setSpeed(8.0);
    attached->setPlaying(true);

    QStringList transitionSignals;
    bool firstFrameCommitted = false;
    QSignalSpy currentFrameSpy(attached, &LottieAttached::currentFrameChanged);
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (firstFrameCommitted || attached->committedImage().isNull()) {
            return;
        }
        firstFrameCommitted = true;
        transitionSignals.append(QStringLiteral("frame"));
        switch (reentrantChange) {
        case FirstCommitChange::Seek:
            attached->setCurrentFrame(1);
            break;
        case FirstCommitChange::Theme:
            attached->setTheme(attached->theme() == QStringLiteral("light")
                                   ? QStringLiteral("dark")
                                   : QStringLiteral("light"));
            break;
        case FirstCommitChange::Size:
            item.setSize(QSizeF(24, 24));
            break;
        }
    });
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        if (attached->status() == LottieAttached::Ready) {
            transitionSignals.append(QStringLiteral("status"));
        }
    });
    connect(attached, &LottieAttached::activeChanged, attached, [&] {
        if (attached->active()) {
            transitionSignals.append(QStringLiteral("active"));
        }
    });
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    connect(attached, &LottieAttached::loaded, attached, [&] {
        transitionSignals.append(QStringLiteral("loaded"));
    });

    item.setSource(source);

    QTRY_COMPARE(loadedSpy.count(), 1);
    QCOMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(attached->active());
    QCOMPARE(transitionSignals,
             QStringList({QStringLiteral("frame"), QStringLiteral("status"),
                          QStringLiteral("active"), QStringLiteral("loaded")}));
    const int frameChangesAtReady = currentFrameSpy.count();
    QTRY_VERIFY(currentFrameSpy.count() > frameChangesAtReady);
    QCOMPARE(loadedSpy.count(), 1);
}

void lottie_item_test::firstCommitReentrancyPreservesReadyAndPlayback_data()
{
    QTest::addColumn<FirstCommitChange>("reentrantChange");
    QTest::newRow("seek") << FirstCommitChange::Seek;
    QTest::newRow("theme") << FirstCommitChange::Theme;
    QTest::newRow("size") << FirstCommitChange::Size;
}

void lottie_item_test::rejectedReentrantSeekKeepsFirstCommitInError()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    attached->setPlaying(true);

    bool rejectedSeekTriggered = false;
    int errorNotifications = 0;
    int readyNotifications = 0;
    int inactiveNotifications = 0;
    int activeNotifications = 0;
    bool loadedSnapshotWasValid = true;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (rejectedSeekTriggered || attached->committedImage().isNull()) {
            return;
        }
        rejectedSeekTriggered = true;
        LottieRuntimeService::instance()->stopForShutdown();
        attached->setCurrentFrame(1);
    });
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        if (attached->status() == LottieAttached::Ready) {
            ++readyNotifications;
        } else if (attached->status() == LottieAttached::Error) {
            ++errorNotifications;
        }
    });
    connect(attached, &LottieAttached::activeChanged, attached, [&] {
        if (attached->active()) {
            ++activeNotifications;
        } else {
            ++inactiveNotifications;
        }
    });
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    connect(attached, &LottieAttached::loaded, attached, [&] {
        loadedSnapshotWasValid = attached->status() == LottieAttached::Ready &&
            attached->active() &&
            !attached->committedImage().isNull();
    });

    item.setSource(source);

    QTRY_VERIFY(rejectedSeekTriggered);
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QVERIFY(!attached->active());
    QVERIFY(attached->committedImage().isNull());
    QVERIFY(attached->playing());
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(errorNotifications, 1);
    QCOMPARE(readyNotifications, 0);
    QCOMPARE(inactiveNotifications, 1);
    QCOMPARE(activeNotifications, 0);
    QCOMPARE(loadedSpy.count(), 0);
    QVERIFY(loadedSnapshotWasValid);
}

void lottie_item_test::firstCommitSourceReplacementCancelsSupersededNotifications()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstSource = writeAnimation(&directory, QStringLiteral("first.json"));
    const QString replacementSource = writeAnimation(
        &directory, QStringLiteral("replacement.json"), 3);
    QVERIFY(!firstSource.isEmpty());
    QVERIFY(!replacementSource.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);

    bool oldCommitExposedReadySnapshot = false;
    QStringList readySources;
    QStringList activeSources;
    QStringList loadedSources;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (attached->source() != firstSource ||
            attached->committedImage().isNull()) {
            return;
        }
        oldCommitExposedReadySnapshot = attached->status() == LottieAttached::Ready &&
            attached->active();
        item.setSource(replacementSource);
    });
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        if (attached->status() == LottieAttached::Ready) {
            readySources.append(attached->source());
        }
    });
    connect(attached, &LottieAttached::activeChanged, attached, [&] {
        if (attached->active()) {
            activeSources.append(attached->source());
        }
    });
    connect(attached, &LottieAttached::loaded, attached, [&] {
        loadedSources.append(attached->source());
    });

    item.setSource(firstSource);

    QTRY_COMPARE(attached->source(), replacementSource);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(oldCommitExposedReadySnapshot);
    QCOMPARE(readySources, QStringList({replacementSource}));
    QCOMPARE(activeSources, QStringList({replacementSource}));
    QCOMPARE(loadedSources, QStringList({replacementSource}));
}

void lottie_item_test::failureSourceReplacementCancelsSupersededNotifications()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstSource = writeAnimation(&directory, QStringLiteral("first.json"));
    const QString failingSource = writeAnimation(&directory, QStringLiteral("failing.json"));
    const QString recoverySource = writeAnimation(&directory, QStringLiteral("recovery.json"));
    QVERIFY(!firstSource.isEmpty());
    QVERIFY(!failingSource.isEmpty());
    QVERIFY(!recoverySource.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&](const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            if (request.source.canonicalLocator() == failingSource) {
                return LottieRuntimeSourceResult();
            }
            return loadTestSource(request, isCancelled);
        });

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(firstSource);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(!attached->committedImage().isNull());

    bool failureSnapshotObserved = false;
    int recoveryLoadingNotifications = 0;
    int recoveryClearedErrorNotifications = 0;
    QStringList errorStatusSources;
    QStringList loadedSources;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (attached->source() != failingSource ||
            !attached->committedImage().isNull()) {
            return;
        }
        failureSnapshotObserved = attached->status() == LottieAttached::Error &&
            !attached->active() && !attached->errorString().isEmpty();
    });
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        if (attached->source() == failingSource &&
            attached->status() == LottieAttached::Error) {
            errorStatusSources.append(attached->source());
            item.setSource(recoverySource);
            return;
        }
        if (attached->source() == recoverySource &&
            attached->status() == LottieAttached::Loading) {
            ++recoveryLoadingNotifications;
        }
    });
    connect(attached, &LottieAttached::errorStringChanged, attached, [&] {
        if (attached->source() == recoverySource && attached->errorString().isEmpty()) {
            ++recoveryClearedErrorNotifications;
        }
    });
    connect(attached, &LottieAttached::loaded, attached, [&] {
        loadedSources.append(attached->source());
    });

    item.setSource(failingSource);

    QTRY_COMPARE(attached->source(), recoverySource);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(failureSnapshotObserved);
    QCOMPARE(recoveryLoadingNotifications, 1);
    QCOMPARE(recoveryClearedErrorNotifications, 1);
    QCOMPARE(errorStatusSources, QStringList({failingSource}));
    QCOMPARE(loadedSources, QStringList({recoverySource}));
}

void lottie_item_test::preReadyFrameSelectionIsClampedAndCommittedFirst()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    attached->setCurrentFrame(4);
    QCOMPARE(attached->currentFrame(), 4);

    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    QCOMPARE(loadedSpy.count(), 1);

    attached->setCurrentFrame(-2);
    QCOMPARE(attached->currentFrame(), 0);
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(loadedSpy.count(), 1);
}

void lottie_item_test::sourceReplacementRetainsReplacesAndClearsPixels()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstSource = writeAnimation(&directory, QStringLiteral("first.json"));
    const QString replacementSource = writeAnimation(
        &directory, QStringLiteral("replacement.json"), 3);
    const QString missingSource = directory.filePath(QStringLiteral("missing.json"));
    QVERIFY(!firstSource.isEmpty());
    QVERIFY(!replacementSource.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    window.show();
    QTRY_VERIFY(window.isVisible());
    item.setSource(firstSource);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8), QColor(Qt::red));

    item.setSource(replacementSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QVERIFY(!attached->active());
    QCOMPARE(attached->totalFrame(), 0);
    QCOMPARE(attached->availableThemes(), QStringList());
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8), QColor(Qt::blue));

    item.setSource(missingSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));

    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QVERIFY(!attached->active());
    QCOMPARE(attached->errorString(),
             QStringLiteral("Lottie source validation failed."));
    QVERIFY(!attached->errorString().contains(missingSource));
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8).alpha(), 0);
}

void lottie_item_test::initialRasterFailureClearsPixelsAndRetainsMetadata()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("initial-raster-failure.json"), 3);
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    rasterState.firstRequestFails = true;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);
    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    attached->setPlaying(true);

    QTRY_VERIFY(rasterState.firstRequestStarted.load());
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QCOMPARE(attached->totalFrame(), 3);
    rasterState.releaseFirstRequest.release();
    QTRY_COMPARE(attached->status(), LottieAttached::Error);

    QCOMPARE(attached->errorString(),
             QStringLiteral("Lottie frame rasterization failed."));
    QVERIFY(attached->playing());
    QCOMPARE(attached->totalFrame(), 3);
    QCOMPARE(attached->fps(), 30.0);
    QVERIFY(!attached->active());
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8).alpha(), 0);
}

void lottie_item_test::staleInitialFrameCannotPublishReadyOrPixels()
{
    QFETCH(bool, staleRequestFails);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    rasterState.firstRequestFails = staleRequestFails;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    QTRY_VERIFY(rasterState.firstRequestStarted.load());
    QCOMPARE(attached->status(), LottieAttached::Loading);

    attached->setCurrentFrame(1);
    rasterState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    QCOMPARE(loadedSpy.count(), 1);
}

void lottie_item_test::staleInitialFrameCannotPublishReadyOrPixels_data()
{
    QTest::addColumn<bool>("staleRequestFails");
    QTest::newRow("stale-success") << false;
    QTest::newRow("stale-failure") << true;
}

void lottie_item_test::staleSourceFailureCannotAffectReplacement()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString replacementSource = writeAnimation(&directory);
    QVERIFY(!replacementSource.isEmpty());

    BlockingSourceState sourceState;
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&](const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            if (sourceState.requestCount.fetch_add(1) == 0) {
                sourceState.firstRequestStarted.store(true);
                sourceState.releaseFirstRequest.acquire();
                return LottieRuntimeSourceResult();
            }
            return loadTestSource(request, isCancelled);
        });

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(QStringLiteral("superseded.json"));

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    QSignalSpy errorSpy(attached, &LottieAttached::errorStringChanged);
    QTRY_VERIFY(sourceState.firstRequestStarted.load());

    item.setSource(replacementSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    sourceState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->source(), replacementSource);
    QCOMPARE(attached->errorString(), QString());
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(loadedSpy.count(), 1);
}

void lottie_item_test::sourceEmitsOnlyOnActualChanges()
{
    LottieItem item;
    QVERIFY(item.flags().testFlag(QQuickItem::ItemHasContents));
    QCOMPARE(item.source(), QString());
    QVERIFY(item.metaObject()->indexOfProperty("mode") >= 0);
    QVERIFY(item.metaObject()->indexOfEnumerator("HighlightMode") >= 0);
    QCOMPARE(item.mode(), LottieItem::HighlightMode(LottieItem::Normal));

    QSignalSpy modeChanges(&item, &LottieItem::modeChanged);
    item.setMode(LottieItem::Normal);
    QCOMPARE(modeChanges.count(), 0);
    item.setMode(LottieItem::Highlight);
    QCOMPARE(item.mode(), LottieItem::HighlightMode(LottieItem::Highlight));
    QCOMPARE(modeChanges.count(), 1);
    item.setMode(LottieItem::Highlight);
    QCOMPARE(modeChanges.count(), 1);

    QSignalSpy sourceChanges(&item, &LottieItem::sourceChanged);
    item.setSource({});
    QCOMPARE(sourceChanges.count(), 0);
    item.setSource(QStringLiteral("first.json"));
    QCOMPARE(item.source(), QStringLiteral("first.json"));
    QCOMPARE(sourceChanges.count(), 1);
    item.setSource(QStringLiteral("first.json"));
    QCOMPARE(sourceChanges.count(), 1);
    item.setSource(QStringLiteral("second.json"));
    QCOMPARE(item.source(), QStringLiteral("second.json"));
    QCOMPARE(sourceChanges.count(), 2);

}

void lottie_item_test::attachedControllerFollowsSourceSizeAndSceneChanges()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    QQuickWindow window;
    makeTestWindowRenderable(window);
    TestLottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setSource(source);
    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(item.lottieAttached(), attached);
    QCOMPARE(attached->source(), source);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    item.setParentItem(window.contentItem());
    QCOMPARE(item.window(), &window);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(attached->active());

    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    item.setWidth(24.0);
    item.setHeight(12.0);
    QTRY_VERIFY(frameChanges.count() >= 1);
    item.notifyDevicePixelRatioChanged(1.0);
    item.notifyLottieChanged();

    attached->setPlaying(true);
    item.setParentItem(nullptr);
    QVERIFY(!item.window());

    item.setSource({});
    QCOMPARE(attached->source(), QString());
    QVERIFY(!attached->active());
}

void lottie_item_test::themeChangeReleasesOldSessionBeforeSynchronousReentry()
{
    ScopedTestEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("3"));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstSource = writeAnimation(
        &directory, QStringLiteral("theme-reentry-first.json"), 2);
    const QString secondSource = writeAnimation(
        &directory, QStringLiteral("theme-reentry-second.json"), 3);
    const QString stableSource = writeAnimation(
        &directory, QStringLiteral("theme-reentry-third.json"), 4);
    const QString reentrantSource = writeAnimation(
        &directory, QStringLiteral("theme-reentry-fourth.json"), 5);
    QVERIFY(!firstSource.isEmpty());
    QVERIFY(!secondSource.isEmpty());
    QVERIFY(!stableSource.isEmpty());
    QVERIFY(!reentrantSource.isEmpty());

    LottieRuntimeService::destroyInstance();
    ThemeLoadRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadThemedTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);

    LottieItem firstItem;
    LottieItem secondItem;
    LottieItem stableItem;
    LottieItem reentrantItem;
    firstItem.setSize(QSizeF(16, 16));
    secondItem.setSize(QSizeF(16, 16));
    stableItem.setSize(QSizeF(16, 16));
    reentrantItem.setSize(QSizeF(16, 16));
    firstItem.setParentItem(window.contentItem());
    secondItem.setParentItem(window.contentItem());
    stableItem.setParentItem(window.contentItem());
    reentrantItem.setParentItem(window.contentItem());

    LottieAttached *firstAttached = attachedPropertiesFor(&firstItem);
    LottieAttached *secondAttached = attachedPropertiesFor(&secondItem);
    LottieAttached *stableAttached = attachedPropertiesFor(&stableItem);
    LottieAttached *reentrantAttached = attachedPropertiesFor(&reentrantItem);
    QVERIFY(firstAttached);
    QVERIFY(secondAttached);
    QVERIFY(stableAttached);
    QVERIFY(reentrantAttached);
    firstAttached->setTheme(QStringLiteral("light"));
    secondAttached->setTheme(QStringLiteral("light"));
    stableAttached->setTheme(QStringLiteral("light"));
    reentrantAttached->setTheme(QStringLiteral("light"));
    reentrantAttached->setSceneActive(false);
    firstItem.setSource(firstSource);
    secondItem.setSource(secondSource);
    stableItem.setSource(stableSource);
    reentrantItem.setSource(reentrantSource);

    QTRY_COMPARE(firstAttached->status(), LottieAttached::Ready);
    QTRY_COMPARE(secondAttached->status(), LottieAttached::Ready);
    QTRY_COMPARE(stableAttached->status(), LottieAttached::Ready);
    QTRY_COMPARE(reentrantAttached->availableThemes().size(), 2);
    reentrantAttached->setCurrentFrame(1);
    QCOMPARE(controller.destructionCount(), 0);

    bool reentered = false;
    QObject::connect(firstAttached, &LottieAttached::preferredThemeChanged,
                     firstAttached, [&] {
                         if (!reentered) {
                             reentered = true;
                             reentrantAttached->setSceneActive(true);
                         }
                     });

    firstAttached->setTheme(QStringLiteral("dark"));
    QVERIFY(reentered);
    QTRY_COMPARE(firstAttached->status(), LottieAttached::Ready);
    QTRY_COMPARE(reentrantAttached->status(), LottieAttached::Ready);
    const LottieAssetRevision stableRevision =
        LottieCacheManager::instance()->requestAsset(
            LottieSourceReference::fromText(stableSource)).asset.revision();
    QCOMPARE(controller.loadCount(stableRevision, QStringLiteral("light")), 1);

    QSignalSpy stableFrameChanges(stableAttached, &LottieAttached::frameImageChanged);
    const int stableFrameChangeCount = stableFrameChanges.count();
    stableAttached->setCurrentFrame(1);
    QTRY_VERIFY(stableFrameChanges.count() > stableFrameChangeCount);
    QTRY_COMPARE(controller.loadCount(stableRevision, QStringLiteral("light")), 1);
}

void lottie_item_test::hostRenderabilityReleasesSessionDemand()
{
    ScopedTestEnvironmentVariable sessionLimit(
        "UKUI_QUICK_LOTTIE_SESSION_CACHE_ENTRIES", QByteArrayLiteral("16"));
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("host-renderability-demand.json"), 5);
    QVERIFY(!source.isEmpty());

    LottieRuntimeService::destroyInstance();
    ThemeLoadRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadThemedTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    QQuickItem ancestor;
    ancestor.setParentItem(window.contentItem());
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(&ancestor);
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    attached->setTheme(QStringLiteral("light"));
    item.setSource(source);

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    const LottieAssetRevision revision =
        LottieCacheManager::instance()->requestAsset(
            LottieSourceReference::fromText(source)).asset.revision();
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 1);
    QCOMPARE(controller.destructionCount(), 0);

    // Paused but visible still owns the current session demand.
    attached->setPlaying(false);
    QCOMPARE(controller.destructionCount(), 0);

    // Completed but still visible also owns the current session demand.
    attached->setPlaying(true);
    attached->onClockElapsed(1000000000LL);
    QTRY_VERIFY(attached->completed());
    QCOMPARE(controller.destructionCount(), 0);
    attached->setPlaying(false);

    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    item.setVisible(false);
    QTRY_COMPARE(controller.destructionCount(), 1);

    item.setVisible(true);
    const int frameChangesBeforeVisibleResume = frameChanges.count();
    attached->setCurrentFrame(1);
    QTRY_VERIFY(frameChanges.count() > frameChangesBeforeVisibleResume);
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 2);

    ancestor.setVisible(false);
    QTRY_COMPARE(controller.destructionCount(), 2);

    ancestor.setVisible(true);
    const int frameChangesBeforeAncestorResume = frameChanges.count();
    attached->setCurrentFrame(2);
    QTRY_VERIFY(frameChanges.count() > frameChangesBeforeAncestorResume);
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 3);

    window.setVisibility(QWindow::Minimized);
    QTRY_COMPARE(window.visibility(), QWindow::Minimized);
    QTRY_COMPARE(controller.destructionCount(), 3);

    window.showNormal();
    QTRY_VERIFY(window.isVisible());
    const int frameChangesBeforeWindowResume = frameChanges.count();
    attached->setCurrentFrame(3);
    QTRY_VERIFY(frameChanges.count() > frameChangesBeforeWindowResume);
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 4);

    item.setSize(QSizeF(0, 0));
    QTRY_COMPARE(controller.destructionCount(), 4);

    item.setSize(QSizeF(20, 20));
    const int frameChangesBeforeResizeResume = frameChanges.count();
    attached->setCurrentFrame(4);
    QTRY_VERIFY(frameChanges.count() > frameChangesBeforeResizeResume);
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 5);

    item.setParentItem(nullptr);
    QTRY_COMPARE(controller.destructionCount(), 5);

    item.setParentItem(&ancestor);
    const int frameChangesBeforeDetachResume = frameChanges.count();
    attached->setCurrentFrame(1);
    QTRY_VERIFY(frameChanges.count() > frameChangesBeforeDetachResume);
    QTRY_COMPARE(controller.loadCount(revision, QStringLiteral("light")), 6);

    controller.failNextRender();
    attached->setTheme(QStringLiteral("dark"));
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QTRY_COMPARE(controller.destructionCount(), 7);
}

void lottie_item_test::continueWhenHiddenCatchesUpWithoutRenderingHiddenFrames()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("continue-when-hidden.json"), 100);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    LottieItem item;
    const ClockControlledAttached controlled =
        createClockControlledAttached(&item);
    LottieAttached *attached = controlled.attached;
    ManualLottieMonotonicClock *clockControl = controlled.clock;
    attached->setSource(source);
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    attached->setLoop(true);
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);
    attached->setSceneActive(false);

    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    const int hiddenFrame = attached->currentFrame();
    const int frameRequestsBeforeHiding =
        LottieRuntimeTest::testFrameRequestCount(runtime);
    clockControl->advance(15000000000LL);
    QCOMPARE(attached->currentFrame(), hiddenFrame);
    QCOMPARE(frameChanges.count(), 0);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             frameRequestsBeforeHiding);

    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), hiddenFrame + 4);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             frameRequestsBeforeHiding + 1);
    QVERIFY(attached->playing());
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));

    // 15 秒跳过 4 帧后还剩约半帧时间。恢复后应接着累计这段时间，
    // 不能因为隐藏/显示而重新等待一个完整帧间隔。
    attached->onClockElapsed(1600000000LL);
    QCOMPARE(attached->currentFrame(), hiddenFrame + 4);
    attached->onClockElapsed(100000000LL);
    QCOMPARE(attached->currentFrame(), hiddenFrame + 5);
}

void lottie_item_test::continueWhenSourceLoadsWhileAlreadyHidden()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("source-loads-while-hidden.json"), 100);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    LottieItem item;
    const ClockControlledAttached controlled =
        createClockControlledAttached(&item, false);
    LottieAttached *attached = controlled.attached;
    ManualLottieMonotonicClock *clockControl = controlled.clock;
    attached->setLoop(true);
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    bool metadataNotificationObserved = false;
    connect(attached, &LottieAttached::totalFrameChanged,
            attached, [&] {
        metadataNotificationObserved = true;
        clockControl->advance(5000000000LL);
    });
    attached->setSource(source);

    QTRY_COMPARE(attached->totalFrame(), 100);
    QVERIFY(metadataNotificationObserved);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime), 0);

    clockControl->advance(10000000000LL);
    QSignalSpy loaded(attached, &LottieAttached::loaded);
    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 4);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime), 1);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(loaded.count(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
}

void lottie_item_test::continueWhenHiddenCompletesOnlyAfterFinalFrameCommit()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("continue-to-final-frame.json"), 5);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    LottieItem item;
    const ClockControlledAttached controlled =
        createClockControlledAttached(&item);
    LottieAttached *attached = controlled.attached;
    ManualLottieMonotonicClock *clockControl = controlled.clock;
    attached->setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    attached->setLoop(false);
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    attached->setSceneActive(false);

    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);
    const int requestsBeforeHiding =
        LottieRuntimeTest::testFrameRequestCount(runtime);
    clockControl->advance(20000000000LL);
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(!attached->completed());
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             requestsBeforeHiding);

    runtime->testHoldFrameSchedulerGuiBatch(true);
    QSignalSpy completedChanges(attached, &LottieAttached::completedChanged);
    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 4);
    QVERIFY(!attached->completed());
    QCOMPARE(completedChanges.count(), 0);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             requestsBeforeHiding + 1);
    QTRY_COMPARE(runtime->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(runtime->testFrameSchedulerStats().pendingGuiBatches, 1);

    runtime->testFlushFrameSchedulerGuiBatch();
    QVERIFY(attached->completed());
    QCOMPARE(completedChanges.count(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    QVERIFY(attached->playing());
}

void lottie_item_test::continueWhenHiddenRebasesAtControlChanges()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("continue-speed-rebase.json"), 100);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    LottieItem item;
    const ClockControlledAttached controlled =
        createClockControlledAttached(&item);
    LottieAttached *attached = controlled.attached;
    ManualLottieMonotonicClock *clockControl = controlled.clock;
    attached->setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    attached->setLoop(true);
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    attached->setSceneActive(false);

    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);
    const int requestsBeforeHiding =
        LottieRuntimeTest::testFrameRequestCount(runtime);
    clockControl->advance(5000000000LL);
    attached->setSpeed(0.02);
    QCOMPARE(attached->currentFrame(), 1);
    clockControl->advance(5000000000LL);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             requestsBeforeHiding);

    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 4);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             requestsBeforeHiding + 1);

    QTRY_COMPARE(runtime->testFrameSchedulerStats().runningJobs, 0);
    QTRY_COMPARE(runtime->testFrameSchedulerStats().pendingGuiBatches, 0);

    // 暂停会先结算旧时间，暂停期间冻结；再次播放后从新的起点计时。
    attached->setPlaying(false);
    attached->setSpeed(0.01);
    attached->setCurrentFrame(10);
    attached->setPlaying(true);
    attached->setSceneActive(false);
    clockControl->advance(5000000000LL);
    attached->setPlaying(false);
    QCOMPARE(attached->currentFrame(), 11);
    clockControl->advance(10000000000LL);
    attached->setPlaying(true);
    clockControl->advance(5000000000LL);
    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 12);

    QTRY_COMPARE(runtime->testFrameSchedulerStats().runningJobs, 0);
    QTRY_COMPARE(runtime->testFrameSchedulerStats().pendingGuiBatches, 0);

    // loop 在修改时刻才生效：修改前的两帧按旧的循环规则从 98 回到 0，
    // 后续时间再按非循环规则前进到 1。
    attached->setPlaying(false);
    attached->setLoop(true);
    attached->setCurrentFrame(98);
    attached->setPlaying(true);
    attached->setSceneActive(false);
    clockControl->advance(7000000000LL);
    attached->setLoop(false);
    QCOMPARE(attached->currentFrame(), 0);
    clockControl->advance(4000000000LL);
    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 1);

    QTRY_COMPARE(runtime->testFrameSchedulerStats().runningJobs, 0);
    QTRY_COMPARE(runtime->testFrameSchedulerStats().pendingGuiBatches, 0);

    // 显式选帧建立新的时间起点，选帧前积累的半帧不能带到新位置。
    attached->setPlaying(false);
    attached->setLoop(true);
    attached->setCurrentFrame(20);
    attached->setPlaying(true);
    attached->setSceneActive(false);
    clockControl->advance(5000000000LL);
    attached->setCurrentFrame(30);
    clockControl->advance(5000000000LL);
    attached->setSceneActive(true);
    QCOMPARE(attached->currentFrame(), 31);
}

void lottie_item_test::hiddenPlaybackPolicyChangesApplyAtTheChangeTime()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("hidden-policy-change.json"), 100);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    LottieItem item;
    const ClockControlledAttached controlled =
        createClockControlledAttached(&item);
    LottieAttached *attached = controlled.attached;
    ManualLottieMonotonicClock *clockControl = controlled.clock;
    attached->setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    attached->setLoop(true);
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    attached->setSceneActive(false);

    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);
    const int requestsBeforeHiding =
        LottieRuntimeTest::testFrameRequestCount(runtime);
    clockControl->advance(5000000000LL);
    QSignalSpy policyChanges(attached,
                             &LottieAttached::hiddenPlaybackPolicyChanged);
    QList<int> framesSeenByPolicyNotification;
    connect(attached, &LottieAttached::hiddenPlaybackPolicyChanged,
            attached, [&] {
        framesSeenByPolicyNotification.append(attached->currentFrame());
    });
    bool shownFromFrameNotification = false;
    connect(attached, &LottieAttached::currentFrameChanged,
            attached, [&] {
        if (!shownFromFrameNotification) {
            shownFromFrameNotification = true;
            attached->setSceneActive(true);
        }
    });
    attached->setHiddenPlaybackPolicy(LottieAttached::PauseWhenHidden);
    QVERIFY(shownFromFrameNotification);
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(attached->hiddenPlaybackPolicy(), LottieAttached::PauseWhenHidden);
    QCOMPARE(policyChanges.count(), 1);
    QCOMPARE(framesSeenByPolicyNotification, QList<int>({1}));

    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime),
             requestsBeforeHiding + 1);

    attached->setSceneActive(false);
    attached->setHiddenPlaybackPolicy(LottieAttached::RestartWhenShown);
    QCOMPARE(attached->currentFrame(), 0);
    QCOMPARE(attached->hiddenPlaybackPolicy(), LottieAttached::RestartWhenShown);
    QCOMPARE(policyChanges.count(), 2);
    QCOMPARE(framesSeenByPolicyNotification, QList<int>({1, 0}));
}

void lottie_item_test::restartWhenShownResetsHiddenPlaybackToFrameZero()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("restart-when-shown.json"), 5);
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);
    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    attached->setPlaying(true);
    attached->onClockElapsed(1000000000LL);
    QTRY_VERIFY(attached->completed());
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    attached->setSpeed(0.01);
    attached->setHiddenPlaybackPolicy(LottieAttached::RestartWhenShown);

    QSignalSpy currentFrameChanges(attached, &LottieAttached::currentFrameChanged);
    QSignalSpy completedChanges(attached, &LottieAttached::completedChanged);
    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    bool restoredVisibilityFromResetSignal = false;
    connect(attached, &LottieAttached::currentFrameChanged,
            attached, [&] {
        if (!restoredVisibilityFromResetSignal) {
            restoredVisibilityFromResetSignal = true;
            item.setVisible(true);
        }
    });
    item.setVisible(false);

    QVERIFY(restoredVisibilityFromResetSignal);
    QVERIFY(item.isVisible());
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(attached->playing());
    QVERIFY(!attached->completed());
    QCOMPARE(currentFrameChanges.count(), 1);
    QCOMPARE(completedChanges.count(), 1);
    QCOMPARE(frameChanges.count(), 0);

    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(attached->playing());
}

void lottie_item_test::rapidResizeDropsStaleLottieItemFrame()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRuntimeService::destroyInstance();

    BlockingRasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);
    ScopedSemaphoreRelease firstRequestRelease(&rasterState.releaseFirstRequest);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    LottieItem item;
    item.setSize(QSizeF(16, 16));
    item.setParentItem(window.contentItem());
    item.setSource(source);

    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_VERIFY(rasterState.firstRequestStarted.load());
    QCOMPARE(attached->status(), LottieAttached::Loading);

    item.setSize(QSizeF(24, 12));
    firstRequestRelease.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QTRY_COMPARE(attached->committedImage().size(),
                 physicalSize(QSizeF(24, 12), window.devicePixelRatio()));
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
}

void lottie_item_test::visibleHostStressMatrix_data()
{
    QTest::addColumn<int>("distinctKeyCount");
    QTest::newRow("16-keys") << 16;
    QTest::newRow("32-keys") << 32;
    QTest::newRow("56-keys") << 56;
}

void lottie_item_test::visibleHostStressMatrix()
{
    QFETCH(int, distinctKeyCount);
    constexpr int hostCount = 64;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("visible-stress-%1.json").arg(distinctKeyCount),
        128 + distinctKeyCount);
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    rasterState.encodeFrameIdentity = true;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
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
                    result.asset.setManifestInitialThemeId(QStringLiteral("light"));
                }
            }
            return result;
        });
    ScopedSemaphoreRelease firstRequestRelease(&rasterState.releaseFirstRequest);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    QTRY_VERIFY(window.isVisible());

    std::vector<std::unique_ptr<LottieItem>> lottieItems;
    std::vector<std::unique_ptr<Icon>> icons;
    std::vector<QQuickItem *> hosts;
    std::vector<LottieAttached *> controllers;
    lottieItems.reserve(hostCount / 2);
    icons.reserve(hostCount / 2);
    hosts.reserve(hostCount);
    controllers.reserve(hostCount);
    QVector<int> frameImageChanges(hostCount, 0);
    QVector<int> expectedFrames(hostCount, 0);

    for (int index = 0; index < hostCount; ++index) {
        QQuickItem *host = nullptr;
        if (index % 2 == 0) {
            auto item = std::make_unique<LottieItem>();
            host = item.get();
            item->setParentItem(window.contentItem());
            item->setSource(source);
            lottieItems.push_back(std::move(item));
        } else {
            auto icon = std::make_unique<Icon>();
            host = icon.get();
            LottieRuntimeTest::setTransparentFallback(icon.get());
            icon->setParentItem(window.contentItem());
            icon->setSource(source);
            icons.push_back(std::move(icon));
        }

        host->setSize(QSizeF(16, 16));
        auto *controller = index % 2 == 0
            ? static_cast<LottieItem *>(host)->lottieAttached()
            : static_cast<Icon *>(host)->lottieAttached();
        QVERIFY(controller);
        QObject::connect(controller, &LottieAttached::frameImageChanged,
                         controller, [&, index] { ++frameImageChanges[index]; });
        hosts.push_back(host);
        controllers.push_back(controller);
    }

    QTRY_VERIFY(rasterState.firstRequestStarted.load());
    QTRY_VERIFY(([&controllers, distinctKeyCount] {
        return std::all_of(controllers.cbegin(), controllers.cend(),
                           [distinctKeyCount](LottieAttached *controller) {
                               return controller->totalFrame() == 128 + distinctKeyCount &&
                                   controller->availableThemes().size() == 2;
                           });
    }()));

    // Destroy an actual visible Host Adapter while the first raster is still
    // running. Its unique queued frame must be removed without affecting the
    // remaining visible hosts.
    const auto *service = LottieRuntimeService::instance();
    QVERIFY(service);
    auto destroyedHost = std::make_unique<LottieItem>();
    destroyedHost->setParentItem(window.contentItem());
    destroyedHost->setSize(QSizeF(15, 15));
    destroyedHost->setSource(source);
    QVERIFY(destroyedHost->lottieAttached());
    QTRY_VERIFY(service->testFrameSchedulerStats().queuedKeys > 0);
    destroyedHost.reset();
    QVERIFY(service->testFrameSchedulerStats().subscriberlessQueuedJobRemovalCount > 0);

    for (int index = 0; index < hostCount; ++index) {
        const qreal size = 24 + (index % distinctKeyCount);
        hosts.at(index)->setSize(QSizeF(size, size));
        controllers.at(index)->setFollowHostTheme(false);
        controllers.at(index)->setTheme(index % 2 == 0
                                             ? QStringLiteral("light")
                                             : QStringLiteral("dark"));
        controllers.at(index)->setPlaying(true);
        controllers.at(index)->setCurrentFrame((index + 1) % 8);
    }

    // Drive several deterministic playback ticks while the first raster is
    // still held. Every host must retain only the final request for its key.
    for (int tick = 0; tick < 3; ++tick) {
        for (int index = 0; index < hostCount; ++index) {
            expectedFrames[index] = (index + tick + 2) % 8;
            controllers.at(index)->setCurrentFrame(expectedFrames.at(index));
        }
    }
    for (LottieAttached *controller : controllers) {
        // Freeze the deterministic final state after the playback churn so
        // the latest-wins assertion is independent of timer scheduling.
        controller->setPlaying(false);
    }

    firstRequestRelease.release();
    QTRY_VERIFY([&controllers] {
        return std::all_of(controllers.cbegin(), controllers.cend(),
                           [](LottieAttached *controller) {
                               return controller->status() == LottieAttached::Ready &&
                                   controller->active();
                           });
    }());
    for (int index = 0; index < hostCount; ++index) {
        expectedFrames[index] = controllers.at(index)->currentFrame();
    }

    // After the initial checkpoint is committed, drive production playback
    // through its private clock boundary with deterministic elapsed ticks.
    // This keeps playing=true during the churn while avoiding wall-clock waits.
    for (LottieAttached *controller : controllers) {
        controller->setPlaying(true);
        controller->stopFrameTimer();
    }
    for (int tick = 0; tick < 3; ++tick) {
        for (int index = 0; index < hostCount; ++index) {
            expectedFrames[index] = (expectedFrames.at(index) + 1) %
                controllers.at(index)->totalFrame();
            controllers.at(index)->onClockElapsed(
                controllers.at(index)->frameIntervalNsecs());
        }
    }
    for (LottieAttached *controller : controllers) {
        controller->setPlaying(false);
    }
    QTRY_VERIFY([service] {
        const LottieRuntimeFrameSchedulerStats stats = service->testFrameSchedulerStats();
        return stats.runningJobs == 0 && stats.queuedKeys == 0 &&
            stats.pendingGuiBatches == 0;
    }());

    for (int index = 0; index < hostCount; ++index) {
        QCOMPARE(controllers.at(index)->committedImage().size(),
                 physicalSize(QSizeF(24 + (index % distinctKeyCount),
                                     24 + (index % distinctKeyCount)),
                              window.devicePixelRatio()));
        QCOMPARE(controllers.at(index)->theme(), index % 2 == 0
                    ? QStringLiteral("light")
                    : QStringLiteral("dark"));
        QCOMPARE(controllers.at(index)->currentFrame(), expectedFrames.at(index));
        QVERIFY(frameImageChanges.at(index) >= 1);
        const QImage image = controllers.at(index)->committedImage();
        QVERIFY(!image.isNull());
        QCOMPARE(image.pixel(0, 0), frameIdentityColor(expectedFrames.at(index)).rgba());
    }

    const LottieRuntimeFrameSchedulerStats stats = service->testFrameSchedulerStats();
    QCOMPARE(stats.rasterWorkerCount, 1);
    QVERIFY(stats.maxQueuedKeys <= 64);
    QVERIFY(stats.maxRunningJobs <= 1);
    QVERIFY(stats.maxPendingGuiBatches <= 1);
    QVERIFY(stats.coalescedRequestCount > 0);
    QVERIFY(stats.supersededRequestCount > 0);
    QVERIFY(stats.droppedResultCount > 0);
    QVERIFY(stats.immutableFrameCount > 0);
    QCOMPARE(stats.immutableFrameCount, stats.rasterJobCount);
}

void lottie_item_test::shutdownDestroysRealHostsBeforeAboutToQuit()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("real-shutdown.json"), 8);
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    const QString activeAssetSource = QStringLiteral("active-host-asset.lottie");
    const QString queuedAssetSource = QStringLiteral("queued-host-asset.lottie");
    HostShutdownAssetState assetState(activeAssetSource, queuedAssetSource);
    LottieRuntimeServiceTestScope runtimeScope(
        [&assetState](const LottieRuntimeSourceRequest &request,
                      const LottieRuntimeService::SourceCancellation &isCancelled) {
            return assetState.run(request, isCancelled);
        });

    auto window = std::make_unique<QQuickWindow>();
    makeTestWindowRenderable(*window);
    QTRY_VERIFY(window->isVisible());

    auto *service = LottieRuntimeService::instance();
    QVERIFY(service);

    // Seed the production scheduler's immutable frame cache with a real Host.
    // The next request is held independently of the active raster below.
    {
        LottieItem warmItem;
        ScopedSemaphoreRelease warmRelease(&rasterState.releaseFirstRequest);
        warmItem.setSize(QSizeF(16, 16));
        warmItem.setParentItem(window->contentItem());
        warmItem.setSource(source);
        QVERIFY(warmItem.lottieAttached());
        QTRY_VERIFY(rasterState.firstRequestStarted.load());
        warmRelease.release();
        QTRY_COMPARE(warmItem.lottieAttached()->status(), LottieAttached::Ready);
    }

    std::atomic<int> hostSignals(0);
    service->testHoldFrameSchedulerGuiBatch(true);
    rasterState.firstRequestStarted.store(false);
    rasterState.blockNextRequest.store(true);
    ScopedSemaphoreRelease activeRelease(&rasterState.releaseFirstRequest);

    auto pendingHost = std::make_unique<Icon>();
    pendingHost->setParentItem(window->contentItem());
    pendingHost->setSize(QSizeF(16, 16));
    LottieRuntimeTest::setTransparentFallback(pendingHost.get());
    LottieAttached *pendingController = pendingHost->lottieAttached();
    QVERIFY(pendingController);
    watchLottieAttachedSignals(pendingController, &hostSignals);
    pendingHost->setSource(source);
    QTRY_COMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);

    auto activeHost = std::make_unique<LottieItem>();
    activeHost->setParentItem(window->contentItem());
    activeHost->setSize(QSizeF(17, 17));
    LottieAttached *activeController = attachedPropertiesFor(activeHost.get());
    QVERIFY(activeController);
    watchLottieAttachedSignals(activeController, &hostSignals);
    activeHost->setSource(source);
    QTRY_VERIFY(rasterState.firstRequestStarted.load());

    auto queuedHost = std::make_unique<Icon>();
    queuedHost->setParentItem(window->contentItem());
    queuedHost->setSize(QSizeF(18, 18));
    LottieRuntimeTest::setTransparentFallback(queuedHost.get());
    LottieAttached *queuedController = queuedHost->lottieAttached();
    QVERIFY(queuedController);
    watchLottieAttachedSignals(queuedController, &hostSignals);
    queuedHost->setSource(source);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().queuedKeys, 1);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);

    auto activeAssetHost = std::make_unique<Icon>();
    activeAssetHost->setParentItem(window->contentItem());
    activeAssetHost->setSize(QSizeF(19, 19));
    LottieRuntimeTest::setTransparentFallback(activeAssetHost.get());
    LottieAttached *activeAssetController = activeAssetHost->lottieAttached();
    QVERIFY(activeAssetController);
    watchLottieAttachedSignals(activeAssetController, &hostSignals);
    activeAssetHost->setSource(activeAssetSource);
    QVERIFY(assetState.waitUntilStarted(activeAssetSource));

    auto queuedAssetHost = std::make_unique<LottieItem>();
    queuedAssetHost->setParentItem(window->contentItem());
    queuedAssetHost->setSize(QSizeF(20, 20));
    LottieAttached *queuedAssetController = attachedPropertiesFor(queuedAssetHost.get());
    QVERIFY(queuedAssetController);
    watchLottieAttachedSignals(queuedAssetController, &hostSignals);
    queuedAssetHost->setSource(queuedAssetSource);
    QCOMPARE(assetState.startedSources(), QStringList() << activeAssetSource);

    QPointer<Icon> pendingGuard = pendingHost.get();
    QPointer<LottieItem> activeGuard = activeHost.get();
    QPointer<Icon> queuedGuard = queuedHost.get();
    QPointer<Icon> activeAssetGuard = activeAssetHost.get();
    QPointer<LottieItem> queuedAssetGuard = queuedAssetHost.get();
    QPointer<LottieAttached> pendingControllerGuard = pendingController;
    QPointer<LottieAttached> activeControllerGuard = activeController;
    QPointer<LottieAttached> queuedControllerGuard = queuedController;
    QPointer<LottieAttached> activeAssetControllerGuard = activeAssetController;
    QPointer<LottieAttached> queuedAssetControllerGuard = queuedAssetController;
    QPointer<QQuickWindow> windowGuard = window.get();
    // All setup notifications are outside the shutdown evidence window. No
    // event-loop turn is allowed between this reset and Host destruction.
    hostSignals.store(0, std::memory_order_relaxed);
    const int callbacksBeforeDestroy = hostSignals.load(std::memory_order_relaxed);
    const int sourceCallbacksBeforeDestroy =
        LottieRuntimeTest::testSourceCompletionCount(service);
    const int frameCallbacksBeforeDestroy =
        service->testFrameSchedulerStats().completionDeliveryCount;
    QCOMPARE(callbacksBeforeDestroy, 0);

    // The production aboutToQuit hook stays on the application thread. This
    // hidden transition hook runs after Stopping becomes irreversible and
    // asset observations are released, but before scheduler/worker joins.
    // Destroying the real Hosts here exercises subscription teardown while
    // pending GUI, active/queued raster and active/queued asset work coexist.
    int stoppingHookCalls = 0;
    bool stoppingObservedByHook = false;
    std::function<void()> destroyHostsWhileStopping = [&] {
        ++stoppingHookCalls;
        stoppingObservedByHook = service->isStopping();
        // Tear down the real scene/window while all five visual Hosts still
        // exist, then destroy the Hosts/controllers in the same stopping turn.
        window.reset();
        pendingHost.reset();
        activeHost.reset();
        queuedHost.reset();
        activeAssetHost.reset();
        queuedAssetHost.reset();
        activeRelease.release();
    };
    QVERIFY(LottieRuntimeTest::setStoppingTransitionHook(
        service, &destroyHostsWhileStopping));
    const bool shutdownCallSucceeded = QMetaObject::invokeMethod(
        QCoreApplication::instance(), "aboutToQuit", Qt::DirectConnection);

    QVERIFY(shutdownCallSucceeded);
    QCOMPARE(stoppingHookCalls, 1);
    QVERIFY(stoppingObservedByHook);
    QVERIFY(pendingGuard.isNull());
    QVERIFY(activeGuard.isNull());
    QVERIFY(queuedGuard.isNull());
    QVERIFY(activeAssetGuard.isNull());
    QVERIFY(queuedAssetGuard.isNull());
    QVERIFY(windowGuard.isNull());
    QVERIFY(assetState.waitUntilCancellationObserved());
    QCOMPARE(assetState.startedSources(), QStringList() << activeAssetSource);
    QCOMPARE(assetState.cancelledCount(), 1);
    QCOMPARE(assetState.activeCount(), 0);
    QVERIFY(!assetState.timedOut());
    QVERIFY(pendingControllerGuard.isNull());
    QVERIFY(activeControllerGuard.isNull());
    QVERIFY(queuedControllerGuard.isNull());
    QVERIFY(activeAssetControllerGuard.isNull());
    QVERIFY(queuedAssetControllerGuard.isNull());

    const auto stats = service->testFrameSchedulerStats();
    QCOMPARE(stats.queuedKeys, 0);
    QCOMPARE(stats.runningJobs, 0);
    QCOMPARE(stats.pendingGuiBatches, 0);
    QCOMPARE(stats.rasterWorkerCount, 0);
    QCOMPARE(LottieRuntimeTest::testAssetWorkerCount(service), 0);
    QCOMPARE(rasterState.requestCount.load(), 2);
    QCOMPARE(rasterState.renderedCount.load(), 2);
    QVERIFY(!rasterState.waitTimedOut.load());
    QVERIFY(stats.droppedResultCount >= 1);
    QVERIFY(stats.maxQueuedKeys <= 64);
    QVERIFY(stats.maxRunningJobs <= 1);
    QVERIFY(stats.maxPendingGuiBatches <= 1);
    QCOMPARE(hostSignals.load() - callbacksBeforeDestroy, 0);
    QCOMPARE(LottieRuntimeTest::testSourceCompletionCount(service) -
                 sourceCallbacksBeforeDestroy,
             0);
    QCOMPARE(stats.completionDeliveryCount - frameCallbacksBeforeDestroy, 0);

    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QCOMPARE(hostSignals.load() - callbacksBeforeDestroy, 0);
    QCOMPARE(LottieRuntimeTest::testSourceCompletionCount(service) -
                 sourceCallbacksBeforeDestroy,
             0);
    QCOMPARE(service->testFrameSchedulerStats().completionDeliveryCount -
                 frameCallbacksBeforeDestroy,
             0);
    QVERIFY(service->isStopping());
    QVERIFY(!LottieRuntimeService::instance());
}

void lottie_item_test::paintWithoutWindowReleasesTheNode()
{
    TestLottieItem item;
    QVERIFY(!item.renderWithoutSceneGraph(new QSGNode));
}

void lottie_item_test::sceneGraphRendersFallbackAndAnimation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    TestLottieItem item;
    item.setParentItem(window.contentItem());
    item.setSize(QSizeF(16, 16));

    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(30);

    item.setSource(source);
    LottieAttached *attached = item.lottieAttached();
    QVERIFY(attached);
    QTRY_VERIFY(attached->active());
    item.update();
    window.update();
    QTest::qWait(30);

    item.setSize(QSizeF(0, 0));
    item.update();
    window.update();
    QTest::qWait(30);

    item.setSize(QSizeF(9000, 1));
    item.update();
    window.update();
    QTest::qWait(30);
    window.hide();
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
    qputenv("QT_QUICK_BACKEND", "software");
    qputenv("QSG_RENDER_LOOP", "basic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    lottie_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-item.moc"
