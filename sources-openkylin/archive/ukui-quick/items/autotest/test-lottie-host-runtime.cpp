#include <QColor>
#include <QFile>
#include <QGuiApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QQuickWindow>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSemaphore>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>

#include "icon.h"
#define private public
#include "lottie-attached.h"
#undef private
#include "lottie-item.h"
#include "lottie-runtime-raster-worker_p.h"
#include "lottie-runtime-service_p.h"
#include "ukui/lottie/lottie-cache-manager.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

namespace {

const QUrl kLottieSourceResolutionBaseUrl(
    QStringLiteral("file:///tmp/lottie-source-resolution/"));

struct RasterRequestRecord
{
    LottieAssetRevision revision;
    QString theme;
    int frame = 0;
    QSize size;
};

class RasterState
{
public:
    void setFailure(int frame, LottieRuntimeFrameResult::Failure failure)
    {
        QMutexLocker locker(&m_mutex);
        m_failureFrame = frame;
        m_failure = failure;
    }

    LottieRuntimeFrameResult::Failure render(const LottieRuntimeFrameRequest &request,
                                              QImage *image)
    {
        QMutexLocker locker(&m_mutex);
        m_requests.append({request.assetRevision, request.effectiveTheme, request.frame,
                           request.physicalPixelSize});
        if (request.frame == m_failureFrame) {
            return m_failure;
        }

        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        if (request.effectiveTheme == QStringLiteral("dark")) {
            image->fill(request.frame == 0 ? Qt::yellow
                                          : (request.frame == 1 ? Qt::cyan : Qt::magenta));
        } else {
            image->fill(request.frame == 0 ? Qt::red
                                          : (request.frame == 1 ? Qt::green : Qt::blue));
        }
        return LottieRuntimeFrameResult::Failure::None;
    }

    int count() const
    {
        QMutexLocker locker(&m_mutex);
        return m_requests.size();
    }

    RasterRequestRecord at(int index) const
    {
        QMutexLocker locker(&m_mutex);
        Q_ASSERT(index >= 0 && index < m_requests.size());
        return m_requests.at(index);
    }

private:
    mutable QMutex m_mutex;
    QList<RasterRequestRecord> m_requests;
    int m_failureFrame = -1;
    LottieRuntimeFrameResult::Failure m_failure =
        LottieRuntimeFrameResult::Failure::None;
};

class ControlledRasterSession final : public LottieRasterSession
{
public:
    explicit ControlledRasterSession(RasterState *state)
        : m_state(state)
    {
    }

    LottieRuntimeFrameResult::Failure render(const LottieRuntimeFrameRequest &request,
                                              QImage *image) override
    {
        return m_state ? m_state->render(request, image)
                       : LottieRuntimeFrameResult::Failure::Raster;
    }

private:
    RasterState *m_state = nullptr;
};

LottieRuntimeSourceResult loadThemedSource(
    const LottieRuntimeSourceRequest &request,
    const LottieRuntimeService::SourceCancellation &isCancelled)
{
    LottieRuntimeSourceResult result;
    result.asset = LottieCacheManager::instance()->requestAsset(
        request.source, isCancelled).asset;
    result.loaded = result.asset.isValid();
    if (result.loaded) {
        result.asset.setAvailableThemes({QStringLiteral("light"), QStringLiteral("dark")});
        result.asset.setManifestInitialThemeId(QStringLiteral("light"));
    }
    return result;
}

QString writeAnimation(QTemporaryDir *directory, const QString &name)
{
    const QString source = directory->filePath(name + QStringLiteral(".json"));
    QByteArray contents = QByteArrayLiteral(R"({
        "v":"5.7.4","fr":10,"ip":0,"op":3,"w":16,"h":16,
        "nm":"host-runtime-test","ddd":0,"assets":[],"layers":[]
    })");
    contents.replace(QByteArrayLiteral("host-runtime-test"), name.toUtf8());

    QFile file(source);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        file.write(contents) != contents.size()) {
        return {};
    }
    return source;
}

bool hasColorAtCenter(const QImage &image, const QColor &color)
{
    return !image.isNull() &&
        image.pixelColor(image.width() / 2, image.height() / 2) == color;
}

template<typename Host>
void prepareHost(Host *host, QQuickWindow *window, const QString &source,
                 LottieAttached **attached)
{
    window->setGeometry(0, 0, 40, 24);
    window->show();
    QTRY_VERIFY(window->isVisible());
    host->setSize(QSizeF(16, 16));
    host->setParentItem(window->contentItem());
    host->setSource(source);
    *attached = host->lottieAttached();
    QVERIFY(*attached);
}

template<typename Host>
void exerciseSameEffectiveSelection(const QString &name)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    Host host;
    LottieAttached *attached = nullptr;
    prepareHost(&host, &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));

    LottieRuntimeService *service = LottieRuntimeService::instance();
    QVERIFY(service);
    QSignalSpy frameChanged(attached, &LottieAttached::frameImageChanged);
    QSignalSpy themeChanged(attached, &LottieAttached::themeChanged);
    QSignalSpy currentFrameChanged(attached, &LottieAttached::currentFrameChanged);
    bool reentered = false;
    QObject::connect(attached, &LottieAttached::currentFrameChanged, attached, [&] {
        if (!reentered) {
            reentered = true;
            attached->setTheme(attached->theme());
        }
    });

    service->testHoldFrameSchedulerGuiBatch(true);
    const int oldRasterCount = rasterState.count();
    attached->setCurrentFrame(1);
    QVERIFY(reentered);
    QCOMPARE(currentFrameChanged.count(), 1);
    QCOMPARE(themeChanged.count(), 0);
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));
    QVERIFY(!attached->followHostTheme());
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
    QCOMPARE(frameChanged.count(), 0);

    QTRY_COMPARE(rasterState.count(), oldRasterCount + 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
    const RasterRequestRecord request = rasterState.at(oldRasterCount);
    QCOMPARE(request.theme, QStringLiteral("light"));
    QCOMPARE(request.frame, 1);

    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(frameChanged.count(), 1);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::green));
    QVERIFY(!attached->completed());
}

template<typename Host>
void exerciseChangedThemeSupersedesPendingFrame(const QString &name)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    Host host;
    LottieAttached *attached = nullptr;
    prepareHost(&host, &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));

    LottieRuntimeService *service = LottieRuntimeService::instance();
    QVERIFY(service);
    QSignalSpy frameChanged(attached, &LottieAttached::frameImageChanged);
    const int oldRasterCount = rasterState.count();
    service->testHoldFrameSchedulerGuiBatch(true);
    host.setSize(QSizeF(17, 16));
    QTRY_COMPARE(rasterState.count(), oldRasterCount + 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
    QCOMPARE(rasterState.at(oldRasterCount).theme, QStringLiteral("light"));

    attached->setTheme(QStringLiteral("dark"));
    QTRY_COMPARE(rasterState.count(), oldRasterCount + 2);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
    QCOMPARE(frameChanged.count(), 0);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
    const RasterRequestRecord latest = rasterState.at(oldRasterCount + 1);
    QCOMPARE(latest.theme, QStringLiteral("dark"));
    QCOMPARE(latest.frame, 0);

    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(frameChanged.count(), 1);
    QCOMPARE(attached->theme(), QStringLiteral("dark"));
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::yellow));
}

template<typename Host>
void exercisePlaybackCheckpoint(const QString &name)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    Host host;
    LottieAttached *attached = nullptr;
    prepareHost(&host, &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));

    LottieRuntimeService *service = LottieRuntimeService::instance();
    QVERIFY(service);
    service->testHoldFrameSchedulerGuiBatch(true);
    QSignalSpy completedChanged(attached, &LottieAttached::completedChanged);
    QSignalSpy frameChanged(attached, &LottieAttached::frameImageChanged);
    const int oldRasterCount = rasterState.count();
    attached->setPlaying(true);
    attached->onClockElapsed(1000000000LL);
    QCOMPARE(attached->currentFrame(), 2);
    QVERIFY(!attached->completed());
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
    QTRY_COMPARE(rasterState.count(), oldRasterCount + 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);

    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(completedChanged.count(), 1);
    QVERIFY(attached->completed());
    QVERIFY(attached->playing());
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));

    service->testHoldFrameSchedulerGuiBatch(true);
    const int frameNotifications = frameChanged.count();
    attached->setPlaying(true);
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(!attached->completed());
    attached->onClockElapsed(1000000000LL);
    QCOMPARE(attached->currentFrame(), 0);
    QCOMPARE(frameChanged.count(), frameNotifications);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(rasterState.count(), oldRasterCount + 1);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
    service->testFlushFrameSchedulerGuiBatch();
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
    attached->setPlaying(false);
}

template<typename Host>
void exerciseRasterFailurePolicy(const QString &name, bool checkpoint)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    rasterState.setFailure(1, LottieRuntimeFrameResult::Failure::Raster);
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    Host host;
    LottieAttached *attached = nullptr;
    prepareHost(&host, &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));

    if (checkpoint) {
        attached->setCurrentFrame(1);
        QTRY_COMPARE(attached->status(), LottieAttached::Error);
        QCOMPARE(attached->errorString(),
                 QStringLiteral("Lottie frame rasterization failed."));
        QVERIFY(attached->committedImage().isNull());
        QVERIFY(!attached->playing());
    } else {
        attached->setLoop(true);
        attached->setPlaying(true);
        attached->onClockElapsed(100000000LL);
        QTRY_COMPARE(rasterState.count(), 2);
        QCOMPARE(attached->status(), LottieAttached::Ready);
        QVERIFY(attached->active());
        QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
        QVERIFY(attached->playing());
        attached->setPlaying(false);
    }
}

template<typename Host>
void exerciseDestroyedHostDropsPendingDelivery(const QString &name)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    auto host = std::make_unique<Host>();
    LottieAttached *attached = nullptr;
    prepareHost(host.get(), &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    LottieRuntimeService *service = LottieRuntimeService::instance();
    QVERIFY(service);
    service->testHoldFrameSchedulerGuiBatch(true);
    int deliveredFrames = 0;
    QObject::connect(attached, &LottieAttached::frameImageChanged, &window,
                     [&deliveredFrames] { ++deliveredFrames; });
    const int oldRasterCount = rasterState.count();
    attached->setCurrentFrame(1);
    QTRY_COMPARE(rasterState.count(), oldRasterCount + 1);
    QTRY_COMPARE(service->testFrameSchedulerStats().runningJobs, 0);
    QCOMPARE(service->testFrameSchedulerStats().pendingGuiBatches, 1);
    host.reset();
    service->testFlushFrameSchedulerGuiBatch();
    QCOMPARE(deliveredFrames, 0);
}

template<typename Host>
void exerciseHiddenPlaybackPolicyAtHostBoundary(const QString &name)
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, name);
    QVERIFY(!source.isEmpty());

    RasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&rasterState] {
        return std::make_unique<ControlledRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope serviceScope(loadThemedSource);

    QQuickWindow window;
    Host host;
    LottieAttached *attached = nullptr;
    prepareHost(&host, &window, source, &attached);
    attached->setPlaying(false);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    attached->setCurrentFrame(2);
    QTRY_VERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));

    LottieRuntimeService *service = LottieRuntimeService::instance();
    QVERIFY(service);
    attached->setSpeed(0.01);
    attached->setPlaying(true);
    QCOMPARE(attached->hiddenPlaybackPolicy(), LottieAttached::PauseWhenHidden);
    const int pauseRequests = service->testFrameRequestCount();
    host.setVisible(false);
    attached->onClockElapsed(1000000000LL);
    QCOMPARE(attached->currentFrame(), 2);
    QCOMPARE(service->testFrameRequestCount(), pauseRequests);
    QSignalSpy pauseResumeFrames(attached, &LottieAttached::frameImageChanged);
    host.setVisible(true);
    QCOMPARE(attached->currentFrame(), 2);
    QTRY_COMPARE(pauseResumeFrames.count(), 1);

    // Detach 会同时丢失 window 和有效渲染尺寸。ContinueWhenHidden 必须先
    // 保存当前帧已走过的时间，不能因为随后清空尺寸而从零重新计时。
    attached->setLoop(true);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->onClockElapsed(8000000000LL);
    QCOMPARE(attached->currentFrame(), 2);
    QSignalSpy detachResumeFrames(attached, &LottieAttached::frameImageChanged);
    host.setParentItem(nullptr);
    host.setParentItem(window.contentItem());
    QTRY_COMPARE(detachResumeFrames.count(), 1);
    QVERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));
    attached->onClockElapsed(3000000000LL);
    QCOMPARE(attached->currentFrame(), 0);

    attached->setPlaying(false);
    attached->setCurrentFrame(2);
    QTRY_VERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));

    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    attached->setPlaying(true);
    host.setSize(QSizeF(9000, 1));
    attached->onClockElapsed(20000000000LL);
    QCOMPARE(attached->currentFrame(), 2);
    host.setSize(QSizeF(16, 16));
    QTRY_VERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));

    attached->setPlaying(false);
    attached->setHiddenPlaybackPolicy(LottieAttached::RestartWhenShown);
    host.setSize(QSizeF(9000, 1));
    QCOMPARE(attached->currentFrame(), 2);
    host.setSize(QSizeF(16, 16));
    QTRY_VERIFY(hasColorAtCenter(attached->committedImage(), Qt::blue));

    attached->setPlaying(true);
    const int restartRequests = service->testFrameRequestCount();
    host.setVisible(false);
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(attached->playing());
    QVERIFY(!attached->completed());
    QCOMPARE(service->testFrameRequestCount(), restartRequests);

    host.setVisible(true);
    QTRY_VERIFY(hasColorAtCenter(attached->committedImage(), Qt::red));
    QCOMPARE(attached->currentFrame(), 0);
    QVERIFY(attached->playing());
    attached->setPlaying(false);
}

}

class lottie_host_runtime_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void sameEffectiveSelectionKeepsCheckpoint();
    void changedThemeSupersedesPendingFrame();
    void playbackWaitsForCheckpointCommit();
    void rasterFailureUsesPresentationPolicy();
    void destroyedHostDropsPendingDelivery();
    void hiddenPlaybackPolicyAppliesToBothHosts();
    void motionIconNamesReachRuntimeAsThemeLocators_data();
    void motionIconNamesReachRuntimeAsThemeLocators();
};

void lottie_host_runtime_test::initTestCase()
{
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
}

void lottie_host_runtime_test::sameEffectiveSelectionKeepsCheckpoint()
{
    exerciseSameEffectiveSelection<Icon>(QStringLiteral("same-effective-icon"));
    exerciseSameEffectiveSelection<LottieItem>(QStringLiteral("same-effective-item"));
}

void lottie_host_runtime_test::changedThemeSupersedesPendingFrame()
{
    exerciseChangedThemeSupersedesPendingFrame<Icon>(QStringLiteral("changed-theme-icon"));
    exerciseChangedThemeSupersedesPendingFrame<LottieItem>(QStringLiteral("changed-theme-item"));
}

void lottie_host_runtime_test::playbackWaitsForCheckpointCommit()
{
    exercisePlaybackCheckpoint<Icon>(QStringLiteral("playback-icon"));
    exercisePlaybackCheckpoint<LottieItem>(QStringLiteral("playback-item"));
}

void lottie_host_runtime_test::rasterFailureUsesPresentationPolicy()
{
    exerciseRasterFailurePolicy<Icon>(QStringLiteral("failure-icon-checkpoint"), true);
    exerciseRasterFailurePolicy<LottieItem>(QStringLiteral("failure-item-checkpoint"), true);
    exerciseRasterFailurePolicy<Icon>(QStringLiteral("failure-icon-playback"), false);
    exerciseRasterFailurePolicy<LottieItem>(QStringLiteral("failure-item-playback"), false);
}

void lottie_host_runtime_test::destroyedHostDropsPendingDelivery()
{
    exerciseDestroyedHostDropsPendingDelivery<Icon>(QStringLiteral("destroy-icon"));
    exerciseDestroyedHostDropsPendingDelivery<LottieItem>(QStringLiteral("destroy-item"));
}

void lottie_host_runtime_test::hiddenPlaybackPolicyAppliesToBothHosts()
{
    exerciseHiddenPlaybackPolicyAtHostBoundary<Icon>(
        QStringLiteral("hidden-policy-icon"));
    exerciseHiddenPlaybackPolicyAtHostBoundary<LottieItem>(
        QStringLiteral("hidden-policy-item"));
}

void lottie_host_runtime_test::motionIconNamesReachRuntimeAsThemeLocators_data()
{
    QTest::addColumn<QString>("hostType");
    QTest::addColumn<QString>("source");
    QTest::addColumn<int>("expectedKind");
    QTest::addColumn<QString>("expectedLocator");

    const QString alarm = QStringLiteral("alarm-24-symbolic-motion");
    const QString service =
        QStringLiteral("ukui-service-support-24-symbolic-motion.lottie");
    const int motionIconKind = static_cast<int>(LottieSourceKind::MotionIcon);
    QTest::newRow("icon-name")
        << QStringLiteral("Icon") << alarm << motionIconKind
        << alarm + QStringLiteral(".lottie");
    QTest::newRow("icon-file-name")
        << QStringLiteral("Icon") << service << motionIconKind << service;
    QTest::newRow("item-name")
        << QStringLiteral("LottieItem") << alarm << motionIconKind
        << alarm + QStringLiteral(".lottie");
    QTest::newRow("item-file-name")
        << QStringLiteral("LottieItem") << service << motionIconKind << service;

    const QString relativeFile = QStringLiteral("local-animation.lottie");
    const int relativeFileKind = static_cast<int>(LottieSourceKind::RelativeFile);
    QTest::newRow("icon-qml-relative-file")
        << QStringLiteral("Icon") << relativeFile << relativeFileKind << relativeFile;
    QTest::newRow("item-qml-relative-file")
        << QStringLiteral("LottieItem") << relativeFile << relativeFileKind << relativeFile;

    const QString explicitJson = QStringLiteral("alarm-24-symbolic-motion.json");
    QTest::newRow("icon-explicit-json")
        << QStringLiteral("Icon") << explicitJson << relativeFileKind << explicitJson;
    QTest::newRow("item-explicit-json")
        << QStringLiteral("LottieItem") << explicitJson << relativeFileKind << explicitJson;
}

void lottie_host_runtime_test::motionIconNamesReachRuntimeAsThemeLocators()
{
    QFETCH(QString, hostType);
    QFETCH(QString, source);
    QFETCH(int, expectedKind);
    QFETCH(QString, expectedLocator);

    QMutex requestMutex;
    LottieSourceReference requestedSource;
    QSemaphore requestObserved;
    LottieRuntimeServiceTestScope serviceScope(
        [&requestMutex, &requestedSource, &requestObserved](
            const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &) {
            {
                QMutexLocker locker(&requestMutex);
                requestedSource = request.source;
            }
            requestObserved.release();
            return LottieRuntimeSourceResult();
        });

    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    context.setBaseUrl(kLottieSourceResolutionBaseUrl);
    std::unique_ptr<QQuickItem> host;
    if (hostType == QStringLiteral("Icon")) {
        auto icon = std::make_unique<Icon>();
        QQmlEngine::setContextForObject(icon.get(), &context);
        icon->setSource(source);
        host = std::move(icon);
    } else {
        auto item = std::make_unique<LottieItem>();
        QQmlEngine::setContextForObject(item.get(), &context);
        item->setSource(source);
        host = std::move(item);
    }
    QVERIFY2(requestObserved.tryAcquire(1, 1000),
             "the Host did not submit the motion-icon name to the Lottie runtime");

    QMutexLocker locker(&requestMutex);
    QCOMPARE(static_cast<int>(requestedSource.kind()), expectedKind);
    QCOMPARE(requestedSource.canonicalLocator(), expectedLocator);
    QCOMPARE(requestedSource.baseUrl(), kLottieSourceResolutionBaseUrl);
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
    lottie_host_runtime_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-host-runtime.moc"
