#include <QFile>
#include <QGSettings/QGSettings>
#include <QGuiApplication>
#include <QIcon>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QSemaphore>
#include <QSGTexture>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtMath>
#include <QtTest>
#include <qqml.h>

#include <atomic>
#include <memory>

#include "icon.h"
#include "icon-theme-test-utils_p.h"
#define private public
#include "lottie-attached.h"
#undef private
#include "lottie-runtime-raster-worker_p.h"
#include "lottie-runtime-service_p.h"
#include "ukui/dt-theme.h"
#include "ukui/gradient-color.h"
#include "ukui/lottie/lottie-cache-manager.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

namespace {

QByteArray animationJson(int endFrame = 2)
{
    static constexpr char animation[] = R"({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"icon-lottie-test",
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

QByteArray slotAnimationJson()
{
    QByteArray contents = animationJson();
    contents.replace(QByteArrayLiteral("\"ddd\":0,\n        \"assets\""),
                     QByteArrayLiteral("\"ddd\":0,\n        \"slots\":{\"fillColor\":{\"p\":{\"a\":0,\"k\":[1,0,0,1]}}},\n        \"assets\""));
    contents.replace(QByteArrayLiteral("\"c\":{\"a\":0,\"k\":[1,0,0,1]}"),
                     QByteArrayLiteral("\"c\":{\"a\":0,\"k\":[1,0,0,1],\"sid\":\"fillColor\"}"));
    return contents;
}

QString writeAnimation(QTemporaryDir *directory, const QString &name, int endFrame = 2)
{
    const QString source = directory->filePath(name);
    QFile file(source);
    const QByteArray animation = animationJson(endFrame);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        file.write(animation) != animation.size()) {
        return {};
    }
    return source;
}

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

struct ZipEntry
{
    QByteArray name;
    QByteArray contents;
    quint32 localHeaderOffset = 0;
};

QByteArray storedZip(QList<ZipEntry> entries)
{
    QByteArray archive;
    for (ZipEntry &entry : entries) {
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
    for (const ZipEntry &entry : entries) {
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
    appendLittleEndian16(archive, static_cast<quint16>(entries.size()));
    appendLittleEndian16(archive, static_cast<quint16>(entries.size()));
    appendLittleEndian32(archive, centralDirectorySize);
    appendLittleEndian32(archive, centralDirectoryOffset);
    appendLittleEndian16(archive, 0);
    return archive;
}

QString writeThemedAnimation(QTemporaryDir *directory,
                             const QString &name = QStringLiteral("themed-animation.lottie"))
{
    const QByteArray archive = storedZip({
        {QByteArrayLiteral("manifest.json"), QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"test\",\"theme\":\"light\"},\"animations\":[{\"id\":\"test\",\"themes\":[\"light\",\"dark\"]}],\"themes\":[{\"id\":\"light\"},{\"id\":\"dark\"}]} ")},
        {QByteArrayLiteral("a/test.json"), slotAnimationJson()},
        {QByteArrayLiteral("t/light.json"), QByteArrayLiteral("{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":[0,0,1,1]}]}")},
        {QByteArrayLiteral("t/dark.json"), QByteArrayLiteral("{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":[0,1,0,1]}]}")}
    });
    const QString source = directory->filePath(name);
    QFile file(source);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           file.write(archive) == archive.size()
        ? source
        : QString();
}

QString writeContrastingThemedAnimation(QTemporaryDir *directory)
{
    const QByteArray archive = storedZip({
        {QByteArrayLiteral("manifest.json"), QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"test\",\"theme\":\"light\"},\"animations\":[{\"id\":\"test\",\"themes\":[\"light\",\"dark\"]}],\"themes\":[{\"id\":\"light\"},{\"id\":\"dark\"}]} ")},
        {QByteArrayLiteral("a/test.json"), slotAnimationJson()},
        {QByteArrayLiteral("t/light.json"), QByteArrayLiteral("{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":[0.2,0.2,0.2,1]}]}")},
        {QByteArrayLiteral("t/dark.json"), QByteArrayLiteral("{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":[1,1,1,1]}]}")}
    });
    const QString source = directory->filePath(QStringLiteral("contrasting-animation.lottie"));
    QFile file(source);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           file.write(archive) == archive.size()
        ? source
        : QString();
}

QString writeLightOnlyThemedAnimation(QTemporaryDir *directory)
{
    const QByteArray archive = storedZip({
        {QByteArrayLiteral("manifest.json"), QByteArrayLiteral(
            "{\"version\":\"2\",\"initial\":{\"animation\":\"test\",\"theme\":\"light\"},"
            "\"animations\":[{\"id\":\"test\",\"themes\":[\"light\"]}],"
            "\"themes\":[{\"id\":\"light\"}]} ")},
        {QByteArrayLiteral("a/test.json"), slotAnimationJson()},
        {QByteArrayLiteral("t/light.json"), QByteArrayLiteral(
            "{\"rules\":[{\"id\":\"fillColor\",\"type\":\"Color\",\"value\":[0,0,1,1]}]}")}
    });
    const QString source = directory->filePath(QStringLiteral("light-only.lottie"));
    QFile file(source);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           file.write(archive) == archive.size()
        ? source
        : QString();
}

QString writeUniqueAnimation(QTemporaryDir *directory, int index)
{
    const QString source = directory->filePath(
        QStringLiteral("lru-%1.json").arg(index, 3, 10, QLatin1Char('0')));
    QFile file(source);
    QByteArray contents = animationJson();
    contents.replace(QByteArrayLiteral("icon-lottie-test"),
                     QByteArrayLiteral("lru-") + QByteArray::number(index));
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           file.write(contents) == contents.size()
        ? source
        : QString();
}

LottieAttached *attachedPropertiesFor(Icon *icon)
{
    return qobject_cast<LottieAttached *>(
        qmlAttachedPropertiesObject<LottieAttached>(icon, true));
}

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
        if (request.frame != 0) {
            image->fill(Qt::green);
        } else if (request.asset.frameCount() == 3) {
            image->fill(Qt::blue);
        } else if (request.asset.frameCount() == 4) {
            image->fill(Qt::magenta);
        } else {
            image->fill(Qt::red);
        }
        return LottieRuntimeFrameResult::Failure::None;
    }
};

class HostDemandRasterSessionController;

class HostDemandRasterSession final : public LottieRasterSession
{
public:
    explicit HostDemandRasterSession(HostDemandRasterSessionController *controller)
        : m_controller(controller)
    {
    }

    ~HostDemandRasterSession() override;

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override;

private:
    HostDemandRasterSessionController *m_controller = nullptr;
};

class HostDemandRasterSessionController
{
public:
    std::unique_ptr<LottieRasterSession> create()
    {
        QMutexLocker locker(&m_mutex);
        ++m_sessionCount;
        ++m_liveSessionCount;
        m_maximumLiveSessionCount = qMax(
            m_maximumLiveSessionCount, m_liveSessionCount);
        return std::make_unique<HostDemandRasterSession>(this);
    }

    void recordRender()
    {
        QMutexLocker locker(&m_mutex);
        ++m_renderCount;
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
    int m_destructionCount = 0;
    int m_liveSessionCount = 0;
    int m_maximumLiveSessionCount = 0;
};

LottieRuntimeFrameResult::Failure HostDemandRasterSession::render(
    const LottieRuntimeFrameRequest &request, QImage *image)
{
    if (!image) {
        return LottieRuntimeFrameResult::Failure::Raster;
    }
    m_controller->recordRender();
    *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
    image->fill(Qt::green);
    return LottieRuntimeFrameResult::Failure::None;
}

HostDemandRasterSession::~HostDemandRasterSession()
{
    m_controller->recordDestruction();
}

class TransientThemeRasterSession final : public LottieRasterSession
{
public:
    explicit TransientThemeRasterSession(std::atomic<int> *renderCount)
        : m_renderCount(renderCount)
    {
    }

    LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) override
    {
        ++(*m_renderCount);
        if (request.effectiveTheme == QStringLiteral("dark")) {
            return LottieRuntimeFrameResult::Failure::TransientTheme;
        }
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(Qt::blue);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    std::atomic<int> *m_renderCount = nullptr;
};

struct BlockingSourceState
{
    std::atomic<int> requestCount {0};
    std::atomic<bool> firstRequestStarted {false};
    QSemaphore releaseFirstRequest;
};

struct BlockingRasterState
{
    std::atomic<int> requestCount {0};
    std::atomic<bool> firstRequestStarted {false};
    QSemaphore releaseFirstRequest;
    bool firstRequestFails = false;
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
        if (firstRequest) {
            m_state->firstRequestStarted.store(true);
            m_state->releaseFirstRequest.acquire();
            if (m_state->firstRequestFails) {
                return LottieRuntimeFrameResult::Failure::Raster;
            }
        }
        if (!image) {
            return LottieRuntimeFrameResult::Failure::Raster;
        }
        *image = QImage(request.physicalPixelSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(request.frame == 0 ? Qt::red : Qt::green);
        return LottieRuntimeFrameResult::Failure::None;
    }

private:
    BlockingRasterState *m_state = nullptr;
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

void makeTestWindowRenderable(QQuickWindow &window)
{
    window.setGeometry(0, 0, 32, 32);
    window.show();
}

struct LottieWarningCapture
{
    std::atomic<int> unavailableThemeCount {0};
    std::atomic<int> missingSourceCount {0};
};

LottieWarningCapture *s_lottieWarningCapture = nullptr;

void captureLottieWarning(QtMsgType type,
                          const QMessageLogContext &context,
                          const QString &message)
{
    Q_UNUSED(context)
    if (type != QtWarningMsg || !s_lottieWarningCapture) {
        return;
    }
    if (message.contains(QStringLiteral("preferred theme is unavailable"))) {
        s_lottieWarningCapture->unavailableThemeCount.fetch_add(1);
    }
    if (message.startsWith(
            QStringLiteral("LottieCacheManager: animation source does not exist"))) {
        s_lottieWarningCapture->missingSourceCount.fetch_add(1);
    }
}

class LottieWarningHandlerScope
{
public:
    explicit LottieWarningHandlerScope(LottieWarningCapture *capture)
        : m_previousHandler(qInstallMessageHandler(captureLottieWarning))
    {
        s_lottieWarningCapture = capture;
    }

    ~LottieWarningHandlerScope()
    {
        s_lottieWarningCapture = nullptr;
        qInstallMessageHandler(m_previousHandler);
    }

private:
    QtMessageHandler m_previousHandler = nullptr;
};

}

class icon_lottie_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void untouchedStaticIconDoesNotCreateLottieController();
    void staticIconModesRemainIndependentOfLottie();
    void missingExtensionlessMotionAnimationUsesSameNamedStaticIcon_data();
    void missingExtensionlessMotionAnimationUsesSameNamedStaticIcon();
    void existingMotionAnimationWinsAndPublishesLottieSourceType();
    void invalidMotionAnimationUsesConfiguredFallbackInsteadOfSameNamedStaticIcon();
    void lottieSourceLazilyCreatesStableAsyncController();
    void asyncLoadingPublishesMetadataAndSelectedFirstFrame();
    void asyncFirstCommitReentrancyKeepsIconReady();
    void recognizesLottieSourcesAndClearsThemForRegularIcons();
    void hostThemeAuthorityIncludesIconHighlightMode();
    void themeFacadeNotificationsSeeCompleteSnapshot();
    void asyncMetadataReentryDoesNotDuplicateThemeChanged();
    void asyncMetadataReentryKeepsInitialThemeNotification();
    void themedAvailableThemesPublishWhileLoading();
    void unavailableAutomaticThemeFallsBackSilently();
    void unthemedAutomaticThemeChangesAreSilentAndDoNotRaster();
    void unavailableManualThemeFallsBackAndWarnsOncePerRevision();
    void unavailableManualThemeWarningIsAnLru();
    void rendersLottieTexturesAndPausesWhenDetached();
    void iconHostRenderabilityReleasesSessionDemand();
    void createsWindowLocalTexturesFromCommittedImage();
    void initialFailureUsesStaticFallback();
    void initialRasterFailureUsesFallbackAndRetainsMetadata();
    void transientThemeFailureRequiresDifferentSelection();
    void replacementRetainsPixelsAndFailureUsesStaticFallback();
    void rapidResizeDropsStaleIconFrame();
    void staleFrameResultsCannotPublishReadyOrPixels_data();
    void staleFrameResultsCannotPublishReadyOrPixels();
    void staleSourceSuccessCannotAffectReplacement();
    void staleSourceFailureCannotAffectReplacement();
};

void icon_lottie_test::initTestCase()
{
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
}

void icon_lottie_test::cleanup()
{
    LottieRuntimeService::destroyInstance();
}

void icon_lottie_test::untouchedStaticIconDoesNotCreateLottieController()
{
    QImage staticImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    staticImage.fill(Qt::cyan);

    Icon icon;
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QSignalSpy sourceTypeSpy(&icon, &Icon::sourceTypeChanged);
    icon.setSize(QSizeF(16, 16));
    icon.setSource(QVariant::fromValue(staticImage));
    QCOMPARE(icon.sourceType(), Icon::StaticIconSource);
    QCOMPARE(sourceTypeSpy.count(), 1);
    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));

    QQuickWindow window;
    icon.setParentItem(window.contentItem());
    window.show();
    QTRY_VERIFY(window.isVisible());
    QSGTexture *texture = icon.createTexture();
    QVERIFY(texture);
    delete texture;

    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));
}

void icon_lottie_test::staticIconModesRemainIndependentOfLottie()
{
    QQuickWindow window;
    makeTestWindowRenderable(window);

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());

    QImage staticImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    staticImage.fill(Qt::red);
    icon.setSource(QVariant::fromValue(staticImage));
    icon.setMode(Icon::Normal);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::red));

    GradientColor highlightColor(QColor(Qt::magenta));
    icon.setDtThemeHighlightColor(&highlightColor);
    icon.setMode(Icon::Highlight);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::magenta));

    icon.setMode(Icon::Disabled);
    const QImage expectedDisabled = QIcon(QPixmap::fromImage(staticImage))
        .pixmap(QSize(16, 16), QIcon::Disabled).toImage();
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), expectedDisabled.pixelColor(8, 8));
    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));
}

void icon_lottie_test::missingExtensionlessMotionAnimationUsesSameNamedStaticIcon_data()
{
    QTest::addColumn<QVariant>("assignedSource");

    const QString source = QStringLiteral("ukui-quick-autotest-ordinary-motion");
    QTest::newRow("string") << QVariant(source);
    QTest::newRow("relative-url") << QVariant(QUrl(source));
}

void icon_lottie_test::missingExtensionlessMotionAnimationUsesSameNamedStaticIcon()
{
    QFETCH(QVariant, assignedSource);
    LottieWarningCapture warningCapture;
    LottieWarningHandlerScope warningHandler(&warningCapture);
    Test::IconThemeScope themeScope;
    QTemporaryDir themeDirectory;
    QTemporaryDir qmlDirectory;
    QVERIFY(themeDirectory.isValid());
    QVERIFY(qmlDirectory.isValid());

    const QString themeName = QStringLiteral("ukui-quick-motion-fallback-theme");
    const QString source = QStringLiteral("ukui-quick-autotest-ordinary-motion");
    QVERIFY(Test::writeThemeIcon(&themeDirectory, themeName, source, QColor(Qt::yellow)));
    QIcon::setThemeSearchPaths({themeDirectory.path()});
    QIcon::setThemeName(themeName);

    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    context.setBaseUrl(QUrl::fromLocalFile(qmlDirectory.path() + QLatin1Char('/')));

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    QQmlEngine::setContextForObject(&icon, &context);
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());

    QImage configuredFallback(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    configuredFallback.fill(Qt::blue);
    icon.setFallbackSource(QVariant::fromValue(configuredFallback));
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setPlaying(true);
    QVector<LottieAttached::Status> observedStatuses;
    connect(attached, &LottieAttached::statusChanged, attached, [&] {
        observedStatuses.append(attached->status());
    });
    QVector<LottieAttached::Status> statusAtSourceTypeNotification;
    connect(&icon, &Icon::sourceTypeChanged, &icon, [&] {
        statusAtSourceTypeNotification.append(attached->status());
    });
    QSignalSpy attachedSourceSpy(attached, &LottieAttached::sourceChanged);
    QSignalSpy sourceTypeSpy(&icon, &Icon::sourceTypeChanged);
    icon.setSource(assignedSource);
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);

    QTRY_COMPARE(icon.sourceType(), Icon::StaticIconSource);
    QCOMPARE(attached->status(), LottieAttached::Null);
    QCOMPARE(attached->source(), QString());
    QCOMPARE(attached->errorString(), QString());
    QVERIFY(attached->playing());
    QCOMPARE(observedStatuses,
             QVector<LottieAttached::Status>({LottieAttached::Loading,
                                               LottieAttached::Null}));
    QCOMPARE(sourceTypeSpy.count(), 1);
    QCOMPARE(attachedSourceSpy.count(), 2);
    QCOMPARE(statusAtSourceTypeNotification,
             QVector<LottieAttached::Status>({LottieAttached::Null}));
    QCOMPARE(warningCapture.missingSourceCount.load(), 0);
    QVERIFY(!icon.fallbackSource().isNull());
    QCOMPARE(icon.source(), assignedSource);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::yellow));
}

void icon_lottie_test::existingMotionAnimationWinsAndPublishesLottieSourceType()
{
    Test::IconThemeScope themeScope;
    QTemporaryDir themeDirectory;
    QTemporaryDir qmlDirectory;
    QVERIFY(themeDirectory.isValid());
    QVERIFY(qmlDirectory.isValid());

    const QString themeName = QStringLiteral("ukui-quick-existing-motion-theme");
    const QString source = QStringLiteral("ukui-quick-autotest-existing-motion");
    QVERIFY(Test::writeThemeIcon(&themeDirectory, themeName, source, QColor(Qt::yellow)));
    QVERIFY(!writeThemedAnimation(&qmlDirectory, source + QStringLiteral(".lottie")).isEmpty());
    QIcon::setThemeSearchPaths({themeDirectory.path()});
    QIcon::setThemeName(themeName);

    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    context.setBaseUrl(QUrl::fromLocalFile(qmlDirectory.path() + QLatin1Char('/')));

    QImage configuredFallback(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    configuredFallback.fill(Qt::magenta);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    QQmlEngine::setContextForObject(&icon, &context);
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setFallbackSource(QVariant::fromValue(configuredFallback));

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setTheme(QStringLiteral("light"));
    QVector<int> sourceTypeFrameCounts;
    connect(&icon, &Icon::sourceTypeChanged, &icon, [&] {
        sourceTypeFrameCounts.append(attached->totalFrame());
    });
    QSignalSpy sourceTypeSpy(&icon, &Icon::sourceTypeChanged);

    icon.setSource(source);
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QTRY_COMPARE(icon.sourceType(), Icon::LottieSource);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(sourceTypeSpy.count(), 1);
    QCOMPARE(sourceTypeFrameCounts.size(), 1);
    QVERIFY(sourceTypeFrameCounts.constFirst() > 0);
    QCOMPARE(icon.source(), QVariant(source));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::blue));
}

void icon_lottie_test::invalidMotionAnimationUsesConfiguredFallbackInsteadOfSameNamedStaticIcon()
{
    Test::IconThemeScope themeScope;
    QTemporaryDir themeDirectory;
    QTemporaryDir qmlDirectory;
    QVERIFY(themeDirectory.isValid());
    QVERIFY(qmlDirectory.isValid());

    const QString themeName = QStringLiteral("ukui-quick-invalid-motion-theme");
    const QString source = QStringLiteral("ukui-quick-autotest-invalid-motion");
    QVERIFY(Test::writeThemeIcon(&themeDirectory, themeName, source, QColor(Qt::yellow)));
    QIcon::setThemeSearchPaths({themeDirectory.path()});
    QIcon::setThemeName(themeName);

    QFile invalidAnimation(qmlDirectory.filePath(source + QStringLiteral(".lottie")));
    const QByteArray invalidContents = QByteArrayLiteral("not a dotLottie archive");
    QVERIFY(invalidAnimation.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(invalidAnimation.write(invalidContents), qint64(invalidContents.size()));
    invalidAnimation.close();

    QQmlEngine engine;
    QQmlContext context(engine.rootContext());
    context.setBaseUrl(QUrl::fromLocalFile(qmlDirectory.path() + QLatin1Char('/')));

    QImage configuredFallback(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    configuredFallback.fill(Qt::blue);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    QQmlEngine::setContextForObject(&icon, &context);
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setFallbackSource(QVariant::fromValue(configuredFallback));
    QSignalSpy sourceTypeSpy(&icon, &Icon::sourceTypeChanged);
    icon.setSource(source);
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QCOMPARE(icon.sourceType(), Icon::LottieSource);
    QCOMPARE(sourceTypeSpy.count(), 1);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::blue));
}

void icon_lottie_test::lottieSourceLazilyCreatesStableAsyncController()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    QVERIFY(!source.isEmpty());

    Icon explicitlyAccessedIcon;
    LottieAttached *explicitlyAccessed = explicitlyAccessedIcon.lottieAttached();
    QVERIFY(explicitlyAccessed);
    QCOMPARE(explicitlyAccessed, explicitlyAccessedIcon.lottieAttached());
    QCOMPARE(explicitlyAccessed->status(), LottieAttached::Null);

    Icon icon;
    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));

    icon.setSource(source);
    QVERIFY(qmlAttachedPropertiesObject<LottieAttached>(&icon, false));

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached, icon.lottieAttached());
    QCOMPARE(attached->source(), source);
    QCOMPARE(attached->status(), LottieAttached::Loading);
}

void icon_lottie_test::asyncLoadingPublishesMetadataAndSelectedFirstFrame()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("selected.json"));
    QVERIFY(!source.isEmpty());

    BlockingSourceState sourceState;
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&](const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            sourceState.firstRequestStarted.store(true);
            sourceState.releaseFirstRequest.acquire();
            if (isCancelled()) {
                return LottieRuntimeSourceResult();
            }
            return loadTestSource(request, isCancelled);
        });

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QTRY_VERIFY(sourceState.firstRequestStarted.load());

    attached->setCurrentFrame(99);
    QCOMPARE(attached->currentFrame(), 99);

    bool metadataObservedWhileLoading = false;
    connect(attached, &LottieAttached::totalFrameChanged, attached, [&] {
        metadataObservedWhileLoading = attached->totalFrame() == 2 &&
            attached->status() == LottieAttached::Loading && !attached->active();
    });

    QStringList transitionSignals;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (!attached->committedImage().isNull()) {
            transitionSignals.append(QStringLiteral("frame"));
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

    sourceState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(metadataObservedWhileLoading);
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(transitionSignals,
             QStringList({QStringLiteral("frame"), QStringLiteral("status"),
                          QStringLiteral("active"), QStringLiteral("loaded")}));
}

void icon_lottie_test::asyncFirstCommitReentrancyKeepsIconReady()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("reentrant.json"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    bool firstFrameCommitted = false;
    connect(attached, &LottieAttached::frameImageChanged, attached, [&] {
        if (firstFrameCommitted || attached->committedImage().isNull()) {
            return;
        }
        firstFrameCommitted = true;
        attached->setCurrentFrame(1);
    });
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);

    icon.setSource(source);

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(firstFrameCommitted);
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));
    QCOMPARE(loadedSpy.count(), 1);
}

void icon_lottie_test::recognizesLottieSourcesAndClearsThemForRegularIcons()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString jsonSource = writeAnimation(&directory, QStringLiteral("animation.JSON"));
    const QString nextSource = writeAnimation(&directory, QStringLiteral("next-animation.json"));
    QVERIFY(!jsonSource.isEmpty());
    QVERIFY(!nextSource.isEmpty());

    Icon icon;
    QSignalSpy sourceChanges(&icon, &Icon::sourceChanged);
    QSignalSpy sourceTypeChanges(&icon, &Icon::sourceTypeChanged);
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));

    icon.setSource(jsonSource);
    QCOMPARE(icon.sourceType(), Icon::LottieSource);
    QCOMPARE(sourceTypeChanges.count(), 1);
    QCOMPARE(icon.source().userType(), QMetaType::QString);
    QCOMPARE(icon.source().toString(), jsonSource);
    QCOMPARE(sourceChanges.count(), 1);
    LottieAttached *attached = attachedPropertiesFor(&icon);
    QVERIFY(attached);
    QCOMPARE(icon.lottieAttached(), attached);
    QCOMPARE(attached->source(), jsonSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    const QUrl nextUrl = QUrl::fromLocalFile(nextSource);
    icon.setSource(nextUrl);
    QCOMPARE(icon.sourceType(), Icon::LottieSource);
    QCOMPARE(sourceTypeChanges.count(), 1);
    QCOMPARE(icon.source().userType(), QMetaType::QUrl);
    QCOMPARE(icon.source().toUrl(), nextUrl);
    QCOMPARE(attached->source(), nextSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QCOMPARE(sourceChanges.count(), 2);

    icon.setSource(QIcon());
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QCOMPARE(sourceTypeChanges.count(), 2);
    QCOMPARE(icon.source().userType(), QMetaType::QIcon);
    QVERIFY(icon.source().value<QIcon>().isNull());
    QCOMPARE(attached->source(), QString());
    QCOMPARE(attached->status(), LottieAttached::Null);
    QCOMPARE(sourceChanges.count(), 3);

    icon.setSource(QString());
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QCOMPARE(sourceTypeChanges.count(), 2);
    QCOMPARE(icon.source().userType(), QMetaType::QString);
    QCOMPARE(icon.source().toString(), QString());
    QCOMPARE(attached->source(), QString());
    QCOMPARE(sourceChanges.count(), 4);
}

void icon_lottie_test::hostThemeAuthorityIncludesIconHighlightMode()
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
    const QString source = writeContrastingThemedAnimation(&directory);
    QVERIFY(!source.isEmpty());

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(QUrl::fromLocalFile(source));
    LottieAttached *attached = attachedPropertiesFor(&icon);
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->availableThemes(), QStringList({QStringLiteral("light"), QStringLiteral("dark")}));
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(51, 51, 51));

    icon.setMode(Icon::Highlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::white));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::white));
    icon.setMode(Icon::Normal);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));
    QTRY_COMPARE(attached->committedImage().pixelColor(0, 0), QColor(51, 51, 51));

    icon.setMode(Icon::ForceHighlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));
    icon.setMode(Icon::AutoHighlight);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));

    settings.set(styleColorKey, QStringLiteral("ukui-dark"));
    QTRY_VERIFY(themeConfig->isDarkTheme());
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));

    attached->setTheme(QStringLiteral("light"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->theme(), QStringLiteral("light"));

    // A manual theme remains authoritative while Icon.mode changes underneath it.
    icon.setMode(Icon::Highlight);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    icon.setMode(Icon::Normal);
    QCOMPARE(attached->theme(), QStringLiteral("light"));

    attached->setFollowHostTheme(true);
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));

    attached->setFollowHostTheme(false);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));

    attached->resetThemePreference();
    QVERIFY(attached->followHostTheme());
    QCOMPARE(attached->preferredTheme(), QString());
    QCOMPARE(attached->theme(), QStringLiteral("dark"));

    // Disabling following without a preference creates a dormant preference from
    // the current effective host theme and restores it when following is paused.
    attached->setFollowHostTheme(false);
    QCOMPARE(attached->preferredTheme(), QStringLiteral("dark"));
    QCOMPARE(attached->theme(), QStringLiteral("dark"));
    settings.set(styleColorKey, QStringLiteral("ukui-light"));
    QTRY_VERIFY(!themeConfig->isDarkTheme());
    QCOMPARE(attached->theme(), QStringLiteral("dark"));
    attached->setFollowHostTheme(true);
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("dark"));
    attached->setFollowHostTheme(false);
    QCOMPARE(attached->theme(), QStringLiteral("dark"));

    // An explicitly empty preference is source-default, not reset.
    attached->setTheme(QString());
    QCOMPARE(attached->preferredTheme(), QString());
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    settings.set(styleColorKey, QStringLiteral("ukui-dark"));
    QTRY_VERIFY(themeConfig->isDarkTheme());
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    attached->resetThemePreference();
    QVERIFY(attached->followHostTheme());
    QCOMPARE(attached->preferredTheme(), QString());
    QTRY_COMPARE(attached->theme(), QStringLiteral("dark"));

    settings.set(styleColorKey, QStringLiteral("ukui-light"));
    QTRY_VERIFY(!themeConfig->isDarkTheme());
    QTRY_COMPARE(attached->theme(), QStringLiteral("light"));

    settings.set(styleColorKey, originalStyleColor);
}

void icon_lottie_test::themeFacadeNotificationsSeeCompleteSnapshot()
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
    const QString source = writeThemedAnimation(&directory, QStringLiteral("snapshot.lottie"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);

    QString observedPreferred;
    QString observedEffective;
    bool observedFollowing = true;
    connect(attached, &LottieAttached::preferredThemeChanged, attached, [&] {
        observedPreferred = attached->preferredTheme();
        observedEffective = attached->theme();
        observedFollowing = attached->followHostTheme();
    });
    attached->setTheme(QStringLiteral("dark"));
    QCOMPARE(observedPreferred, QStringLiteral("dark"));
    QCOMPARE(observedEffective, QStringLiteral("dark"));
    QVERIFY(!observedFollowing);

    observedPreferred.clear();
    observedEffective.clear();
    observedFollowing = false;
    connect(attached, &LottieAttached::followHostThemeChanged, attached, [&] {
        observedPreferred = attached->preferredTheme();
        observedEffective = attached->theme();
        observedFollowing = attached->followHostTheme();
    });
    attached->setFollowHostTheme(true);
    QCOMPARE(observedPreferred, QStringLiteral("dark"));
    QCOMPARE(observedEffective, QStringLiteral("light"));
    QVERIFY(observedFollowing);

    settings.set(styleColorKey, originalStyleColor);
}

void icon_lottie_test::asyncMetadataReentryDoesNotDuplicateThemeChanged()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedAnimation(&directory, QStringLiteral("metadata-reentry.lottie"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setTheme(QStringLiteral("light"));

    QSignalSpy themeChanges(attached, &LottieAttached::themeChanged);
    bool reentered = false;
    connect(attached, &LottieAttached::totalFrameChanged, attached, [&] {
        if (!reentered) {
            reentered = true;
            attached->setTheme(QStringLiteral("dark"));
        }
    });

    icon.setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(reentered);
    QCOMPARE(attached->theme(), QStringLiteral("dark"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("dark"));
    QCOMPARE(themeChanges.count(), 1);
}

void icon_lottie_test::asyncMetadataReentryKeepsInitialThemeNotification()
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
    const QString source = writeThemedAnimation(&directory, QStringLiteral("theme-reentry.lottie"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);

    QSignalSpy themeChanges(attached, &LottieAttached::themeChanged);
    bool reentered = false;
    QString observedTheme;
    QString observedPreferredTheme;
    bool observedFollowing = true;
    int observedFrame = -1;
    int observedTotalFrame = -1;
    connect(attached, &LottieAttached::themeChanged, attached, [&] {
        observedTheme = attached->theme();
        observedPreferredTheme = attached->preferredTheme();
        observedFollowing = attached->followHostTheme();
        observedFrame = attached->currentFrame();
        observedTotalFrame = attached->totalFrame();
    });
    connect(attached, &LottieAttached::totalFrameChanged, attached, [&] {
        if (reentered) {
            return;
        }
        reentered = true;
        attached->setTheme(attached->theme());
        attached->setCurrentFrame(1);
    });

    icon.setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(reentered);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("light"));
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->currentFrame(), 1);
    QCOMPARE(themeChanges.count(), 1);
    QCOMPARE(observedTheme, QStringLiteral("light"));
    QCOMPARE(observedPreferredTheme, QStringLiteral("light"));
    QVERIFY(!observedFollowing);
    QCOMPARE(observedFrame, 1);
    QCOMPARE(observedTotalFrame, 2);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::green));

    settings.set(styleColorKey, originalStyleColor);
}

void icon_lottie_test::themedAvailableThemesPublishWhileLoading()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedAnimation(&directory, QStringLiteral("loading.lottie"));
    QVERIFY(!source.isEmpty());

    BlockingSourceState sourceState;
    BlockingRasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&](const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            sourceState.firstRequestStarted.store(true);
            sourceState.releaseFirstRequest.acquire();
            if (isCancelled()) {
                return LottieRuntimeSourceResult();
            }
            return loadTestSource(request, isCancelled);
        });
    struct ReleaseOnExit
    {
        QSemaphore &sourceSemaphore;
        QSemaphore &rasterSemaphore;
        ~ReleaseOnExit()
        {
            sourceSemaphore.release();
            rasterSemaphore.release();
            rasterSemaphore.release();
        }
    } releaseOnExit {sourceState.releaseFirstRequest, rasterState.releaseFirstRequest};

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);

    QTRY_VERIFY(sourceState.firstRequestStarted.load());
    QCOMPARE(attached->status(), LottieAttached::Loading);
    sourceState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->availableThemes(),
                 QStringList({QStringLiteral("light"), QStringLiteral("dark")}));
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QVERIFY(rasterState.firstRequestStarted.load());

    rasterState.releaseFirstRequest.release();
    rasterState.releaseFirstRequest.release();
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
}

void icon_lottie_test::unavailableAutomaticThemeFallsBackSilently()
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
    const QString source = writeLightOnlyThemedAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieWarningCapture warningCapture;
    LottieWarningHandlerScope warningHandler(&warningCapture);
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->availableThemes(), QStringList({QStringLiteral("light")}));
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    const QImage initialImage = attached->committedImage();
    QVERIFY(!initialImage.isNull());
    frameChanges.clear();

    settings.set(styleColorKey, QStringLiteral("ukui-dark"));
    QTRY_VERIFY(themeConfig->isDarkTheme());
    QTest::qWait(20);
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(frameChanges.count(), 0);
    QCOMPARE(attached->committedImage(), initialImage);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 0);

    settings.set(styleColorKey, originalStyleColor);
}

void icon_lottie_test::unthemedAutomaticThemeChangesAreSilentAndDoNotRaster()
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
    const QString source = writeAnimation(&directory, QStringLiteral("unthemed.json"));
    QVERIFY(!source.isEmpty());

    LottieWarningCapture warningCapture;
    LottieWarningHandlerScope warningHandler(&warningCapture);
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(attached->availableThemes().isEmpty());
    QCOMPARE(attached->theme(), QString());
    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    const QImage initialImage = attached->committedImage();
    QVERIFY(!initialImage.isNull());
    frameChanges.clear();

    settings.set(styleColorKey, QStringLiteral("ukui-dark"));
    QTRY_VERIFY(themeConfig->isDarkTheme());
    QTest::qWait(20);
    QCOMPARE(attached->theme(), QString());
    QCOMPARE(frameChanges.count(), 0);
    QCOMPARE(attached->committedImage(), initialImage);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 0);

    settings.set(styleColorKey, QStringLiteral("ukui-light"));
    QTRY_VERIFY(!themeConfig->isDarkTheme());
    QTest::qWait(20);
    QCOMPARE(attached->theme(), QString());
    QCOMPARE(frameChanges.count(), 0);
    QCOMPARE(attached->committedImage(), initialImage);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 0);

    settings.set(styleColorKey, originalStyleColor);
}

void icon_lottie_test::unavailableManualThemeFallsBackAndWarnsOncePerRevision()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedAnimation(&directory, QStringLiteral("manual.lottie"));
    const QString replacement = writeThemedAnimation(
        &directory, QStringLiteral("manual-copy.lottie"));
    QVERIFY(!source.isEmpty());
    QVERIFY(!replacement.isEmpty());

    LottieWarningCapture warningCapture;
    LottieWarningHandlerScope warningHandler(&warningCapture);
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setTheme(QStringLiteral("missing"));
    QCOMPARE(attached->preferredTheme(), QStringLiteral("missing"));
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->theme(), QString());

    icon.setSource(source);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QTRY_COMPARE(warningCapture.unavailableThemeCount.load(), 1);
    QCOMPARE(attached->preferredTheme(), QStringLiteral("missing"));
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(attached->availableThemes(), QStringList({QStringLiteral("light"),
                                                       QStringLiteral("dark")}));

    attached->setTheme(QStringLiteral("missing"));
    QTest::qWait(20);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 1);

    icon.setSource(replacement);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->preferredTheme(), QStringLiteral("missing"));
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    // 本地 dotLottie 的 revision 标识“规范 URI 上的这个文件代际”。即使
    // 两个路径的字节相同，它们也是两个 revision，因此各警告一次。
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 2);

    attached->setTheme(QString());
    QCOMPARE(attached->preferredTheme(), QString());
    QVERIFY(!attached->followHostTheme());
    QCOMPARE(attached->theme(), QStringLiteral("light"));
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 2);
}

void icon_lottie_test::unavailableManualThemeWarningIsAnLru()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    LottieWarningCapture warningCapture;
    LottieWarningHandlerScope warningHandler(&warningCapture);
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setTheme(QStringLiteral("lru-preference"));

    QStringList sources;
    for (int index = 0; index < 256; ++index) {
        const QString source = writeUniqueAnimation(&directory, index);
        QVERIFY(!source.isEmpty());
        sources.append(source);
        icon.setSource(source);
        QTRY_COMPARE_WITH_TIMEOUT(attached->status(), LottieAttached::Ready, 3000);
    }
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 256);

    icon.setSource(sources.constFirst());
    QTRY_COMPARE_WITH_TIMEOUT(attached->status(), LottieAttached::Ready, 3000);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 256);

    const QString newSource = writeUniqueAnimation(&directory, 256);
    QVERIFY(!newSource.isEmpty());
    icon.setSource(newSource);
    QTRY_COMPARE_WITH_TIMEOUT(attached->status(), LottieAttached::Ready, 3000);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 257);

    icon.setSource(sources.at(1));
    QTRY_COMPARE_WITH_TIMEOUT(attached->status(), LottieAttached::Ready, 3000);
    QCOMPARE(warningCapture.unavailableThemeCount.load(), 258);
}

void icon_lottie_test::rendersLottieTexturesAndPausesWhenDetached()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("long-animation.json"), 120);
    QVERIFY(!source.isEmpty());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setSource(QUrl::fromLocalFile(source));
    LottieAttached *attached = attachedPropertiesFor(&icon);
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QCOMPARE(icon.createTexture(), nullptr);

    QSignalSpy frameChanges(attached, &LottieAttached::frameImageChanged);
    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    icon.setParentItem(window.contentItem());
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    const qreal devicePixelRatio = window.devicePixelRatio();

    QSGTexture *texture = icon.createTexture();
    QVERIFY(texture);
    QCOMPARE(texture->textureSize(), physicalSize(QSizeF(16, 16), devicePixelRatio));
    delete texture;

    icon.setWidth(20.0);
    icon.setHeight(12.0);
    QTRY_COMPARE(attached->committedImage().size(),
                 physicalSize(QSizeF(20, 12), devicePixelRatio));
    QVERIFY(frameChanges.count() >= 1);
    texture = icon.createTexture();
    QVERIFY(texture);
    QCOMPARE(texture->textureSize(), physicalSize(QSizeF(20, 12), devicePixelRatio));
    delete texture;

    icon.setSize(QSizeF(9000, 1));
    texture = icon.createTexture();
    QVERIFY(texture);
    QCOMPARE(texture->textureSize(), QSize(1, 1));
    delete texture;

    QImage oversizedFallback(QSize(9000, 1), QImage::Format_ARGB32_Premultiplied);
    oversizedFallback.fill(Qt::cyan);
    icon.setFallbackSource(QVariant::fromValue(oversizedFallback));
    texture = icon.createTexture();
    QVERIFY(texture);
    QCOMPARE(texture->textureSize(), oversizedFallback.size());
    delete texture;

    icon.setSize(QSizeF(16, 16));
    attached->setCurrentFrame(0);
    attached->setLoop(true);
    attached->setPlaying(true);
    QTRY_VERIFY_WITH_TIMEOUT(attached->currentFrame() > 0, 1000);

    icon.setParentItem(nullptr);
    const int stoppedFrame = attached->currentFrame();
    QTest::qWait(100);
    QCOMPARE(attached->currentFrame(), stoppedFrame);
    window.hide();
}

void icon_lottie_test::iconHostRenderabilityReleasesSessionDemand()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(
        &directory, QStringLiteral("icon-host-renderability-demand.json"), 5);
    QVERIFY(!source.isEmpty());

    LottieRuntimeService::destroyInstance();
    HostDemandRasterSessionController controller;
    LottieRuntimeRasterSessionTestScope rasterScope([&controller] {
        return controller.create();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    QQuickItem ancestor;
    ancestor.setParentItem(window.contentItem());
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(&ancestor);
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QTRY_COMPARE(controller.sessionCount(), 1);
    QCOMPARE(controller.destructionCount(), 0);

    // Paused and completed while still displayed retain the session demand.
    attached->setPlaying(false);
    QCOMPARE(controller.destructionCount(), 0);
    attached->setPlaying(true);
    attached->onClockElapsed(1000000000LL);
    QTRY_VERIFY(attached->completed());
    QCOMPARE(controller.destructionCount(), 0);
    attached->setPlaying(false);

    ancestor.setVisible(false);
    QTRY_COMPARE(controller.destructionCount(), 1);

    ancestor.setVisible(true);
    attached->setCurrentFrame(1);
    QTRY_COMPARE(controller.sessionCount(), 2);

    window.setVisibility(QWindow::Minimized);
    QTRY_COMPARE(window.visibility(), QWindow::Minimized);
    QTRY_COMPARE(controller.destructionCount(), 2);

    window.showNormal();
    QTRY_VERIFY(window.isVisible());
    attached->setCurrentFrame(2);
    QTRY_COMPARE(controller.sessionCount(), 3);

    icon.setParentItem(nullptr);
    QTRY_COMPARE(controller.destructionCount(), 3);

    icon.setParentItem(&ancestor);
    attached->setCurrentFrame(3);
    QTRY_COMPARE(controller.sessionCount(), 4);
    QCOMPARE(controller.maximumLiveSessionCount(), 1);

    icon.setParentItem(nullptr);
    QTRY_COMPARE(controller.destructionCount(), 4);
}

void icon_lottie_test::replacementRetainsPixelsAndFailureUsesStaticFallback()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString firstSource = writeAnimation(&directory, QStringLiteral("first.json"), 2);
    const QString replacementSource = writeAnimation(&directory, QStringLiteral("replacement.json"), 3);
    const QString failingSource = writeAnimation(&directory, QStringLiteral("fallback.json"), 4);
    QVERIFY(!firstSource.isEmpty());
    QVERIFY(!replacementSource.isEmpty());
    QVERIFY(!failingSource.isEmpty());

    BlockingSourceState replacementState;
    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(
        [&](const LottieRuntimeSourceRequest &request,
            const LottieRuntimeService::SourceCancellation &isCancelled) {
            if (request.source.canonicalLocator() == replacementSource) {
                replacementState.firstRequestStarted.store(true);
                replacementState.releaseFirstRequest.acquire();
                if (isCancelled()) {
                    return LottieRuntimeSourceResult();
                }
            }
            if (request.source.canonicalLocator() == failingSource) {
                return LottieRuntimeSourceResult();
            }
            return loadTestSource(request, isCancelled);
        });
    struct ReleaseOnExit
    {
        QSemaphore &semaphore;
        ~ReleaseOnExit()
        {
            semaphore.release();
        }
    } releaseOnExit {replacementState.releaseFirstRequest};

    QImage fallbackImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    fallbackImage.fill(Qt::yellow);

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setFallbackSource(QVariant::fromValue(fallbackImage));
    icon.setSource(firstSource);

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::red));

    icon.setMode(Icon::Highlight);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::red));
    icon.setMode(Icon::Normal);

    icon.setSource(replacementSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QVERIFY(!attached->active());
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::red));
    QTRY_VERIFY(replacementState.firstRequestStarted.load());

    replacementState.releaseFirstRequest.release();
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::blue));

    icon.setSource(failingSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QVERIFY(!attached->active());
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::yellow));

    QImage staticImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    staticImage.fill(Qt::cyan);
    icon.setSource(QVariant::fromValue(staticImage));
    QCOMPARE(icon.lottieAttached(), attached);
    QCOMPARE(attached->status(), LottieAttached::Null);
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::cyan));
}

void icon_lottie_test::rapidResizeDropsStaleIconFrame()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("rapid-resize.json"));
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QTRY_VERIFY(rasterState.firstRequestStarted.load());
    QCOMPARE(attached->status(), LottieAttached::Loading);

    icon.setSize(QSizeF(24, 12));
    rasterState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QTRY_COMPARE(attached->committedImage().size(),
                 physicalSize(QSizeF(24, 12), window.devicePixelRatio()));
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::red));
}

void icon_lottie_test::initialFailureUsesStaticFallback()
{
    LottieRuntimeServiceTestScope runtimeScope(
        [](const LottieRuntimeSourceRequest &,
           const LottieRuntimeService::SourceCancellation &) {
            return LottieRuntimeSourceResult();
        });

    QImage fallbackImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    fallbackImage.fill(QColor(26, 26, 26));

    QQuickWindow window;
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isVisible());

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setFallbackSource(QVariant::fromValue(fallbackImage));
    icon.setSource(QStringLiteral("initial-failure.json"));

    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QCOMPARE(attached->errorString(),
             QStringLiteral("Lottie source validation failed."));
    QVERIFY(!attached->errorString().contains(QStringLiteral("initial-failure.json")));
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(26, 26, 26));

    GradientColor highlightColor(QColor(Qt::magenta));
    icon.setDtThemeHighlightColor(&highlightColor);
    icon.setMode(Icon::ForceHighlight);
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::magenta));
}

void icon_lottie_test::initialRasterFailureUsesFallbackAndRetainsMetadata()
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

    QImage fallback(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    fallback.fill(Qt::yellow);
    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setFallbackSource(QVariant::fromValue(fallback));
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
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
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::yellow));
}

void icon_lottie_test::transientThemeFailureRequiresDifferentSelection()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedAnimation(
        &directory, QStringLiteral("transient-theme-failure.lottie"));
    QVERIFY(!source.isEmpty());

    std::atomic<int> renderCount {0};
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<TransientThemeRasterSession>(&renderCount);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    attached->setFollowHostTheme(false);
    attached->setTheme(QStringLiteral("light"));
    attached->setPlaying(true);
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(renderCount.load(), 1);
    QCOMPARE(attached->totalFrame(), 2);
    QVERIFY(!attached->committedImage().isNull());

    attached->setTheme(QStringLiteral("dark"));
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QCOMPARE(attached->errorString(),
             QStringLiteral("Lottie theme application failed."));
    QCOMPARE(renderCount.load(), 2);
    QVERIFY(attached->playing());
    QCOMPARE(attached->totalFrame(), 2);
    QVERIFY(attached->committedImage().isNull());

    const int rendersAfterFailure = renderCount.load();
    icon.setSize(QSizeF(20, 12));
    icon.setVisible(false);
    icon.setVisible(true);
    attached->setTheme(QStringLiteral("dark"));
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QCOMPARE(renderCount.load(), rendersAfterFailure);
    QCOMPARE(attached->status(), LottieAttached::Error);

    attached->setTheme(QStringLiteral("light"));
    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QVERIFY(!attached->committedImage().isNull());
    QVERIFY(renderCount.load() > rendersAfterFailure);

    const int rendersBeforeSecondDarkSelection = renderCount.load();
    attached->setTheme(QStringLiteral("dark"));
    QTRY_COMPARE(attached->status(), LottieAttached::Error);
    QCOMPARE(renderCount.load(), rendersBeforeSecondDarkSelection + 1);
}

void icon_lottie_test::createsWindowLocalTexturesFromCommittedImage()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("window-local.json"));
    QVERIFY(!source.isEmpty());

    LottieRuntimeRasterSessionTestScope rasterScope([] {
        return std::make_unique<ColorRasterSession>();
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow firstWindow;
    QQuickWindow secondWindow;
    firstWindow.setGeometry(0, 0, 32, 32);
    secondWindow.setGeometry(0, 0, 32, 32);

    Icon firstIcon;
    firstIcon.setSize(QSizeF(16, 16));
    firstIcon.setParentItem(firstWindow.contentItem());
    firstIcon.setSource(source);

    Icon secondIcon;
    secondIcon.setSize(QSizeF(16, 16));
    secondIcon.setParentItem(secondWindow.contentItem());
    secondIcon.setSource(source);

    firstWindow.show();
    secondWindow.show();
    QTRY_VERIFY(firstWindow.isVisible());
    QTRY_VERIFY(secondWindow.isVisible());
    QTRY_COMPARE(firstIcon.lottieAttached()->status(), LottieAttached::Ready);
    QTRY_COMPARE(secondIcon.lottieAttached()->status(), LottieAttached::Ready);

    QSGTexture *firstTexture = firstIcon.createTexture();
    QSGTexture *secondTexture = secondIcon.createTexture();
    QVERIFY(firstTexture);
    QVERIFY(secondTexture);
    QCOMPARE(firstTexture->textureSize(),
             physicalSize(QSizeF(16, 16), firstWindow.devicePixelRatio()));
    QCOMPARE(secondTexture->textureSize(),
             physicalSize(QSizeF(16, 16), secondWindow.devicePixelRatio()));
    QVERIFY(firstTexture != secondTexture);
    delete firstTexture;
    delete secondTexture;
}

void icon_lottie_test::staleFrameResultsCannotPublishReadyOrPixels()
{
    QFETCH(bool, firstRequestFails);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("stale.json"));
    QVERIFY(!source.isEmpty());

    BlockingRasterState rasterState;
    rasterState.firstRequestFails = firstRequestFails;
    LottieRuntimeRasterSessionTestScope rasterScope([&] {
        return std::make_unique<BlockingRasterSession>(&rasterState);
    });
    LottieRuntimeServiceTestScope runtimeScope(loadTestSource);

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(source);
    LottieAttached *attached = icon.lottieAttached();
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

void icon_lottie_test::staleFrameResultsCannotPublishReadyOrPixels_data()
{
    QTest::addColumn<bool>("firstRequestFails");
    QTest::newRow("stale-success") << false;
    QTest::newRow("stale-failure") << true;
}

void icon_lottie_test::staleSourceSuccessCannotAffectReplacement()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString supersededSource = writeAnimation(
        &directory, QStringLiteral("superseded.json"), 2);
    const QString replacementSource = writeAnimation(
        &directory, QStringLiteral("replacement.json"), 3);
    QVERIFY(!supersededSource.isEmpty());
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
                return loadTestSource(request, [] {
                    return false;
                });
            }
            return loadTestSource(request, isCancelled);
        });
    struct ReleaseOnExit
    {
        QSemaphore &semaphore;
        ~ReleaseOnExit()
        {
            semaphore.release();
        }
    } releaseOnExit {sourceState.releaseFirstRequest};

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(supersededSource);
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QSignalSpy errorSpy(attached, &LottieAttached::errorStringChanged);
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    QTRY_VERIFY(sourceState.firstRequestStarted.load());

    icon.setSource(replacementSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    sourceState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->source(), replacementSource);
    QCOMPARE(attached->totalFrame(), 3);
    QCOMPARE(attached->errorString(), QString());
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));
}

void icon_lottie_test::staleSourceFailureCannotAffectReplacement()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString replacementSource = writeAnimation(
        &directory, QStringLiteral("replacement.json"), 3);
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
    struct ReleaseOnExit
    {
        QSemaphore &semaphore;
        ~ReleaseOnExit()
        {
            semaphore.release();
        }
    } releaseOnExit {sourceState.releaseFirstRequest};

    QQuickWindow window;
    makeTestWindowRenderable(window);
    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setParentItem(window.contentItem());
    icon.setSource(QStringLiteral("superseded.json"));
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QSignalSpy errorSpy(attached, &LottieAttached::errorStringChanged);
    QSignalSpy loadedSpy(attached, &LottieAttached::loaded);
    QTRY_VERIFY(sourceState.firstRequestStarted.load());

    icon.setSource(replacementSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
    sourceState.releaseFirstRequest.release();

    QTRY_COMPARE(attached->status(), LottieAttached::Ready);
    QCOMPARE(attached->source(), replacementSource);
    QCOMPARE(attached->errorString(), QString());
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(attached->committedImage().pixelColor(0, 0), QColor(Qt::blue));
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
    icon_lottie_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-icon-lottie.moc"
