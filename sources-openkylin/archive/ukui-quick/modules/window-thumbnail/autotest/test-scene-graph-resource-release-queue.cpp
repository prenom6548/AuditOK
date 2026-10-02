#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QtTest>

#include "../core/scene-graph-resource-release-queue.h"

using namespace UkuiThumbnail;

namespace {

class FakeThumbnailBuffer final : public ThumbnailBuffer
{
public:
    explicit FakeThumbnailBuffer(QList<int> *releaseOrder, int id)
        : m_releaseOrder(releaseOrder)
        , m_id(id)
    {
    }

    QSGTexture *createTexture(const QQuickItem *) override
    {
        return nullptr;
    }

    bool releaseTextureResources() override
    {
        ++releaseCount;
        m_releaseOrder->append(m_id);
        return releaseResult;
    }

    bool releaseResult = true;
    int releaseCount = 0;

private:
    QList<int> *m_releaseOrder;
    int m_id;
};

class OpenGlContextGuard final
{
public:
    bool create()
    {
        m_surface.setFormat(m_context.format());
        m_surface.create();
        return m_surface.isValid() && m_context.create() && m_context.makeCurrent(&m_surface);
    }

    ~OpenGlContextGuard()
    {
        m_context.doneCurrent();
    }

private:
    QOffscreenSurface m_surface;
    QOpenGLContext m_context;
};

using Buffer = QSharedPointer<ThumbnailBuffer>;

Buffer makeBuffer(QList<int> *releaseOrder, int id)
{
    return QSharedPointer<FakeThumbnailBuffer>::create(releaseOrder, id);
}

FakeThumbnailBuffer *asFake(const Buffer &buffer)
{
    return static_cast<FakeThumbnailBuffer *>(buffer.data());
}

}

class scene_graph_resource_release_queue_test : public QObject
{
    Q_OBJECT

private slots:
    void fromWindowReturnsOneQueuePerWindow();
    void immediateEnqueueReleasesBuffers();
    void hiddenWindowReschedulesPendingBuffersWhenShown();
    void releaseWithoutContextIsRetriedWhenContextBecomesAvailable();
    void failedReleaseKeepsRemainingBuffersForOrderedRetry();
};

void scene_graph_resource_release_queue_test::fromWindowReturnsOneQueuePerWindow()
{
    QCOMPARE(SceneGraphResourceReleaseQueue::fromWindow(nullptr), nullptr);

    QQuickWindow window;
    auto *firstQueue = SceneGraphResourceReleaseQueue::fromWindow(&window);
    auto *secondQueue = SceneGraphResourceReleaseQueue::fromWindow(&window);

    QVERIFY(firstQueue != nullptr);
    QCOMPARE(firstQueue, secondQueue);
    QCOMPARE(firstQueue->parent(), &window);
}

void scene_graph_resource_release_queue_test::immediateEnqueueReleasesBuffers()
{
    OpenGlContextGuard context;
    if (!context.create()) {
        QSKIP("An OpenGL context is unavailable on this test platform");
    }

    QQuickWindow window;
    auto *queue = SceneGraphResourceReleaseQueue::fromWindow(&window);
    QList<int> releaseOrder;
    const auto first = makeBuffer(&releaseOrder, 1);
    const auto second = makeBuffer(&releaseOrder, 2);

    queue->enqueue({first, second}, false);

    QCOMPARE(releaseOrder, QList<int>({1, 2}));
    QCOMPARE(asFake(first)->releaseCount, 1);
    QCOMPARE(asFake(second)->releaseCount, 1);
}

void scene_graph_resource_release_queue_test::hiddenWindowReschedulesPendingBuffersWhenShown()
{
    if (QGuiApplication::platformName() == QStringLiteral("offscreen")) {
        QSKIP("The offscreen platform cannot initialize a QQuickWindow scene graph here");
    }

    QQuickWindow window;
    window.setGeometry(0, 0, 64, 64);
    window.show();
    QTRY_VERIFY_WITH_TIMEOUT(window.isSceneGraphInitialized(), 3000);

    auto *queue = SceneGraphResourceReleaseQueue::fromWindow(&window);
    QList<int> releaseOrder;
    const auto buffer = makeBuffer(&releaseOrder, 1);

    window.hide();
    QTRY_VERIFY_WITH_TIMEOUT(!window.isVisible(), 1000);
    queue->enqueue({buffer});
    QTest::qWait(50);
    QCOMPARE(asFake(buffer)->releaseCount, 0);

    window.show();
    QTRY_COMPARE_WITH_TIMEOUT(asFake(buffer)->releaseCount, 1, 3000);
    QCOMPARE(releaseOrder, QList<int>({1}));
}

void scene_graph_resource_release_queue_test::releaseWithoutContextIsRetriedWhenContextBecomesAvailable()
{
    QVERIFY(QOpenGLContext::currentContext() == nullptr);

    QQuickWindow window;
    auto *queue = SceneGraphResourceReleaseQueue::fromWindow(&window);
    QList<int> releaseOrder;
    const auto deferred = makeBuffer(&releaseOrder, 1);
    const auto trigger = makeBuffer(&releaseOrder, 2);

    queue->enqueue({deferred}, false);
    QCOMPARE(asFake(deferred)->releaseCount, 0);

    OpenGlContextGuard context;
    if (!context.create()) {
        QSKIP("An OpenGL context is unavailable on this test platform");
    }

    queue->enqueue({trigger}, false);

    QCOMPARE(releaseOrder, QList<int>({1, 2}));
    QCOMPARE(asFake(deferred)->releaseCount, 1);
    QCOMPARE(asFake(trigger)->releaseCount, 1);
}

void scene_graph_resource_release_queue_test::failedReleaseKeepsRemainingBuffersForOrderedRetry()
{
    OpenGlContextGuard context;
    if (!context.create()) {
        QSKIP("An OpenGL context is unavailable on this test platform");
    }

    QQuickWindow window;
    auto *queue = SceneGraphResourceReleaseQueue::fromWindow(&window);
    QList<int> releaseOrder;
    const auto first = makeBuffer(&releaseOrder, 1);
    const auto second = makeBuffer(&releaseOrder, 2);
    const auto trigger = makeBuffer(&releaseOrder, 3);
    asFake(first)->releaseResult = false;

    queue->enqueue({first, second}, false);
    QCOMPARE(releaseOrder, QList<int>({1}));
    QCOMPARE(asFake(second)->releaseCount, 0);

    asFake(first)->releaseResult = true;
    queue->enqueue({trigger}, false);

    QCOMPARE(releaseOrder, QList<int>({1, 1, 2, 3}));
    QCOMPARE(asFake(first)->releaseCount, 2);
    QCOMPARE(asFake(second)->releaseCount, 1);
    QCOMPARE(asFake(trigger)->releaseCount, 1);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    scene_graph_resource_release_queue_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-scene-graph-resource-release-queue.moc"
