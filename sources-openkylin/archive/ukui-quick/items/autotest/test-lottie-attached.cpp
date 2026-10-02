#include <QFile>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>
#include <qqml.h>

#include <memory>

#include "icon.h"
#define private public
#include "lottie-attached.h"
#undef private
#include "lottie-item.h"
#include "lottie-runtime-service_p.h"
#include "lottie-runtime-test-helpers_p.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

namespace {

LottieAttached *attachedPropertiesFor(QObject *object)
{
    return qobject_cast<LottieAttached *>(
        qmlAttachedPropertiesObject<LottieAttached>(object, true));
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
        "nm":"attached-test",
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

void assertUnsupportedHostIsInert(QObject *object, LottieRuntimeService *runtime)
{
    LottieAttached *attached = attachedPropertiesFor(object);
    QVERIFY(attached);

    QSignalSpy sourceChanges(attached, &LottieAttached::sourceChanged);
    QSignalSpy playingChanges(attached, &LottieAttached::playingChanged);
    QSignalSpy loopChanges(attached, &LottieAttached::loopChanged);
    QSignalSpy speedChanges(attached, &LottieAttached::speedChanged);
    QSignalSpy frameChanges(attached, &LottieAttached::currentFrameChanged);
    QSignalSpy themeChanges(attached, &LottieAttached::themeChanged);
    QSignalSpy preferredThemeChanges(attached, &LottieAttached::preferredThemeChanged);
    QSignalSpy followHostThemeChanges(attached, &LottieAttached::followHostThemeChanged);
    QSignalSpy availableThemesChanges(attached, &LottieAttached::availableThemesChanged);
    QSignalSpy totalFrameChanges(attached, &LottieAttached::totalFrameChanged);
    QSignalSpy fpsChanges(attached, &LottieAttached::fpsChanged);
    QSignalSpy statusChanges(attached, &LottieAttached::statusChanged);
    QSignalSpy errorChanges(attached, &LottieAttached::errorStringChanged);
    QSignalSpy activeChanges(attached, &LottieAttached::activeChanged);
    QSignalSpy completedChanges(attached, &LottieAttached::completedChanged);
    QSignalSpy hiddenPlaybackPolicyChanges(
        attached, &LottieAttached::hiddenPlaybackPolicyChanged);
    QSignalSpy frameImageChanges(attached, &LottieAttached::frameImageChanged);
    QSignalSpy loaded(attached, &LottieAttached::loaded);

    attached->setSource(QStringLiteral("unsupported-host.json"));
    attached->setPlaying(true);
    attached->setLoop(true);
    attached->setSpeed(2.0);
    attached->setCurrentFrame(3);
    attached->setHiddenPlaybackPolicy(LottieAttached::RestartWhenShown);
    attached->setTheme(QStringLiteral("dark"));
    attached->setFollowHostTheme(false);
    attached->resetThemePreference();
    attached->setHostTheme(QStringLiteral("dark"));
    attached->setHostRenderSize(QSize(32, 32));
    attached->setSceneActive(true);

    QCOMPARE(attached->source(), QString());
    QCOMPARE(attached->playing(), false);
    QCOMPARE(attached->loop(), false);
    QCOMPARE(attached->speed(), 1.0);
    QCOMPARE(attached->currentFrame(), 0);
    QCOMPARE(attached->totalFrame(), 0);
    QCOMPARE(attached->fps(), 30.0);
    QCOMPARE(attached->theme(), QString());
    QCOMPARE(attached->preferredTheme(), QString());
    QCOMPARE(attached->followHostTheme(), true);
    QVERIFY(attached->availableThemes().isEmpty());
    QCOMPARE(attached->status(), LottieAttached::Null);
    QCOMPARE(attached->errorString(), QString());
    QVERIFY(!attached->active());
    QVERIFY(!attached->completed());
    QCOMPARE(attached->hiddenPlaybackPolicy(), LottieAttached::PauseWhenHidden);
    QVERIFY(attached->committedImage().isNull());
    QCOMPARE(LottieRuntimeTest::testSubscriptionCount(runtime), 0);
    QCOMPARE(LottieRuntimeTest::testSourceRequestCount(runtime), 0);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime), 0);

    const QList<QSignalSpy *> signalSpies {
        &sourceChanges,
        &playingChanges,
        &loopChanges,
        &speedChanges,
        &frameChanges,
        &themeChanges,
        &preferredThemeChanges,
        &followHostThemeChanges,
        &availableThemesChanges,
        &totalFrameChanges,
        &fpsChanges,
        &statusChanges,
        &errorChanges,
        &activeChanges,
        &completedChanges,
        &hiddenPlaybackPolicyChanges,
        &frameImageChanges,
        &loaded
    };
    for (QSignalSpy *spy : signalSpies) {
        QCOMPARE(spy->count(), 0);
    }
}

}

class lottie_attached_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void factoryRegistersSupportedHosts();
    void unsupportedObjectDoesNotCreateRuntimeWork();
    void iconCreatesAttachmentOnlyThroughFactory();
    void lottieItemUsesTheAttachedController();
    void iconClearsAttachedAnimationForNonLottieSource();
    void qmlAttachedPropertiesRegisterTheirHosts();
    void themeFacadeExposesHostFollowingAndPreference();
    void themePreferenceNotificationsAreReentrantSafe();
    void highlightModeBelongsToLottieItemHost();
    void hiddenPlaybackPolicyDefaultsToPause();
};

void lottie_attached_test::initTestCase()
{
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
}

void lottie_attached_test::factoryRegistersSupportedHosts()
{
    QQuickItem plainItem;
    LottieAttached *plainAttachment = attachedPropertiesFor(&plainItem);
    QVERIFY(plainAttachment);
    QCOMPARE(plainAttachment->parent(), &plainItem);

    Icon icon;
    LottieAttached *iconAttachment = attachedPropertiesFor(&icon);
    QCOMPARE(icon.lottieAttached(), iconAttachment);

    LottieItem item;
    LottieAttached *itemAttachment = attachedPropertiesFor(&item);
    QCOMPARE(item.lottieAttached(), itemAttachment);
}

void lottie_attached_test::unsupportedObjectDoesNotCreateRuntimeWork()
{
    LottieRuntimeServiceTestScope serviceScope(
        [](const LottieRuntimeSourceRequest &, const LottieRuntimeService::SourceCancellation &) {
            return LottieRuntimeSourceResult();
        });
    LottieRuntimeService *runtime = LottieRuntimeService::instance();
    QVERIFY(runtime);

    QObject plainObject;
    QQuickItem plainItem;
    assertUnsupportedHostIsInert(&plainObject, runtime);
    assertUnsupportedHostIsInert(&plainItem, runtime);
    QCOMPARE(LottieRuntimeTest::testSubscriptionCount(runtime), 0);
    QCOMPARE(LottieRuntimeTest::testSourceRequestCount(runtime), 0);
    QCOMPARE(LottieRuntimeTest::testFrameRequestCount(runtime), 0);
}

void lottie_attached_test::iconCreatesAttachmentOnlyThroughFactory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    const QString nextSource = writeAnimation(&directory, QStringLiteral("next-animation.json"));
    QVERIFY(!source.isEmpty());
    QVERIFY(!nextSource.isEmpty());

    Icon icon;
    QVERIFY(!qmlAttachedPropertiesObject<LottieAttached>(&icon, false));

    icon.setSource(QUrl::fromLocalFile(source));
    LottieAttached *attached = icon.lottieAttached();
    QVERIFY(attached);
    QCOMPARE(attached->source(), source);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    attached = attachedPropertiesFor(&icon);
    QCOMPARE(icon.lottieAttached(), attached);
    QCOMPARE(attached->source(), source);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    icon.setMode(Icon::Highlight);

    icon.setSource(QUrl::fromLocalFile(nextSource));
    QCOMPARE(attached->source(), nextSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
}

void lottie_attached_test::lottieItemUsesTheAttachedController()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    const QString nextSource = writeAnimation(&directory, QStringLiteral("next-animation.json"));
    QVERIFY(!source.isEmpty());
    QVERIFY(!nextSource.isEmpty());

    LottieItem item;
    QVERIFY(!item.lottieAttached());
    item.setSource(source);
    QVERIFY(item.lottieAttached());

    LottieAttached *attached = item.lottieAttached();
    QCOMPARE(item.lottieAttached(), attached);
    QCOMPARE(attached->source(), source);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    item.setSource(nextSource);
    QCOMPARE(attached->source(), nextSource);
    QCOMPARE(attached->status(), LottieAttached::Loading);
}

void lottie_attached_test::iconClearsAttachedAnimationForNonLottieSource()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    QVERIFY(!source.isEmpty());

    Icon icon;
    icon.setSource(QUrl::fromLocalFile(source));
    LottieAttached *attached = attachedPropertiesFor(&icon);
    QVERIFY(attached);
    QCOMPARE(attached->status(), LottieAttached::Loading);

    icon.setSource(QVariant::fromValue(QIcon()));
    QCOMPARE(attached->source(), QString());
    QVERIFY(!attached->active());
}

void lottie_attached_test::qmlAttachedPropertiesRegisterTheirHosts()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, QStringLiteral("animation.json"));
    QVERIFY(!source.isEmpty());

    QQmlEngine engine;
    engine.setImportPathList({});
    engine.rootContext()->setContextProperty(QStringLiteral("animationSource"), source);

    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQuick 2.15
        import org.ukui.quick.items 1.0

        Item {
            LottieItem {
                objectName: "lottieItem"
                source: animationSource
                mode: LottieItem.Highlight
                Lottie.playing: true
                Lottie.hiddenPlaybackPolicy: Lottie.ContinueWhenHidden
                property bool loadingState: Lottie.status === Lottie.Loading
                property bool emptyError: Lottie.errorString.length === 0
            }
        }
    )", QUrl());

    QObject *root = component.create();
    QVERIFY2(root, qPrintable(component.errorString()));

    auto *lottieItem = root->findChild<LottieItem *>(QStringLiteral("lottieItem"));
    QVERIFY(lottieItem);
    QCOMPARE(lottieItem->mode(),
             LottieItem::HighlightMode(LottieItem::Highlight));
    QVERIFY(lottieItem->lottieAttached());
    QVERIFY(lottieItem->lottieAttached()->playing());
    QCOMPARE(lottieItem->lottieAttached()->hiddenPlaybackPolicy(),
             LottieAttached::ContinueWhenHidden);
    QCOMPARE(lottieItem->lottieAttached()->source(), source);
    QCOMPARE(lottieItem->lottieAttached()->status(), LottieAttached::Loading);
    QVERIFY(lottieItem->property("loadingState").toBool());
    QVERIFY(lottieItem->property("emptyError").toBool());

    delete root;
}

void lottie_attached_test::themeFacadeExposesHostFollowingAndPreference()
{
    LottieItem item;
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    QVERIFY(attached->property("followHostTheme").isValid());
    QCOMPARE(attached->property("followHostTheme").toBool(), true);
    QVERIFY(attached->property("preferredTheme").isValid());
    QCOMPARE(attached->property("preferredTheme").toString(), QString());

    QVERIFY(attached->setProperty("theme", QStringLiteral("dark")));
    QCOMPARE(attached->property("followHostTheme").toBool(), false);
    QCOMPARE(attached->property("preferredTheme").toString(), QStringLiteral("dark"));

    QVERIFY(QMetaObject::invokeMethod(attached, "resetThemePreference", Qt::DirectConnection));
    QCOMPARE(attached->property("followHostTheme").toBool(), true);
    QCOMPARE(attached->property("preferredTheme").toString(), QString());
}

void lottie_attached_test::themePreferenceNotificationsAreReentrantSafe()
{
    LottieItem item;
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    QSignalSpy preferredThemeChanges(attached, &LottieAttached::preferredThemeChanged);
    QSignalSpy followHostThemeChanges(attached, &LottieAttached::followHostThemeChanged);
    bool resetDuringNotification = false;
    connect(attached, &LottieAttached::preferredThemeChanged, attached, [&] {
        if (!resetDuringNotification) {
            resetDuringNotification = true;
            attached->resetThemePreference();
        }
    });

    attached->setTheme(QStringLiteral("dark"));

    QCOMPARE(attached->preferredTheme(), QString());
    QVERIFY(attached->followHostTheme());
    QCOMPARE(preferredThemeChanges.count(), 2);
    QCOMPARE(followHostThemeChanges.count(), 1);
}

void lottie_attached_test::highlightModeBelongsToLottieItemHost()
{
    LottieItem item;
    QVERIFY(item.metaObject()->indexOfProperty("mode") >= 0);
    QVERIFY(item.metaObject()->indexOfEnumerator("HighlightMode") >= 0);

    LottieAttached attached;
    QVERIFY(attached.metaObject()->indexOfEnumerator("HighlightMode") < 0);
}

void lottie_attached_test::hiddenPlaybackPolicyDefaultsToPause()
{
    LottieItem item;
    LottieAttached *attached = attachedPropertiesFor(&item);
    QVERIFY(attached);
    QCOMPARE(attached->hiddenPlaybackPolicy(),
             LottieAttached::PauseWhenHidden);

    QSignalSpy changes(attached, &LottieAttached::hiddenPlaybackPolicyChanged);
    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    QCOMPARE(attached->hiddenPlaybackPolicy(),
             LottieAttached::ContinueWhenHidden);
    QCOMPARE(changes.count(), 1);

    attached->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    QCOMPARE(changes.count(), 1);
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
    lottie_attached_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-attached.moc"
