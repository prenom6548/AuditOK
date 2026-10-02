#include <QFile>
#include <QGuiApplication>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "icon.h"
#include "lottie-attached.h"
#include "lottie-item.h"
#include "ukui-quick-items-plugin.h"

using namespace UkuiQuick;

namespace {

QByteArray animationJson()
{
    return QByteArrayLiteral(R"({
        "v":"5.7.4","fr":30,"ip":0,"op":2,"w":16,"h":16,
        "nm":"production-host-smoke","ddd":0,"assets":[],
        "layers":[{"ddd":0,"ind":1,"ty":4,"nm":"Shape","sr":1,
        "ks":{"o":{"a":0,"k":100},"r":{"a":0,"k":0},
        "p":{"a":0,"k":[8,8,0]},"a":{"a":0,"k":[0,0]},
        "s":{"a":0,"k":[100,100,100]}},"ao":0,
        "shapes":[{"ty":"rc","d":1,"s":{"a":0,"k":[16,16]},
        "p":{"a":0,"k":[0,0]},"r":{"a":0,"k":0}},
        {"ty":"fl","c":{"a":0,"k":[1,0,0,1]},"o":{"a":0,"k":100},"r":1},
        {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},
        "s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100}}],
        "ip":0,"op":2,"st":0,"bm":0}]
    })");
}

QString writeAnimation(QTemporaryDir *directory)
{
    const QString source = directory->filePath(QStringLiteral("production.json"));
    QFile file(source);
    const QByteArray contents = animationJson();
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
            file.write(contents) == contents.size()
        ? source
        : QString();
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

}

class lottie_production_hosts_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void productionHostsCommitRealAsyncPixels();
};

void lottie_production_hosts_test::initTestCase()
{
    UkuiQuickItemsPlugin plugin;
    plugin.registerTypes("org.ukui.quick.items");
}

void lottie_production_hosts_test::productionHostsCommitRealAsyncPixels()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    QQuickWindow window;
    window.setGeometry(0, 0, 48, 24);

    Icon icon;
    icon.setSize(QSizeF(16, 16));
    icon.setSource(QUrl::fromLocalFile(source));
    icon.setParentItem(window.contentItem());

    LottieItem item;
    item.setX(24);
    item.setSize(QSizeF(16, 16));
    item.setSource(source);
    item.setParentItem(window.contentItem());

    window.show();
    QTRY_VERIFY(window.isExposed());

    LottieAttached *iconFacade = icon.lottieAttached();
    LottieAttached *itemFacade = item.lottieAttached();
    QVERIFY(iconFacade);
    QVERIFY(itemFacade);
    QCOMPARE(iconFacade->hiddenPlaybackPolicy(), LottieAttached::PauseWhenHidden);
    QCOMPARE(itemFacade->hiddenPlaybackPolicy(), LottieAttached::PauseWhenHidden);
    QSignalSpy policyChanges(itemFacade,
                             &LottieAttached::hiddenPlaybackPolicyChanged);
    itemFacade->setHiddenPlaybackPolicy(LottieAttached::ContinueWhenHidden);
    QCOMPARE(itemFacade->hiddenPlaybackPolicy(),
             LottieAttached::ContinueWhenHidden);
    QCOMPARE(policyChanges.count(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(iconFacade->status(), LottieAttached::Ready, 3000);
    QTRY_COMPARE_WITH_TIMEOUT(itemFacade->status(), LottieAttached::Ready, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(!iconFacade->committedImage().isNull(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(!itemFacade->committedImage().isNull(), 3000);

    QCOMPARE(iconFacade->committedImage().pixelColor(8, 8), QColor(Qt::red));
    QCOMPARE(itemFacade->committedImage().pixelColor(8, 8), QColor(Qt::red));
    QCOMPARE(grabItemImage(&icon).pixelColor(8, 8), QColor(Qt::red));
    QCOMPARE(grabItemImage(&item).pixelColor(8, 8), QColor(Qt::red));
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
    lottie_production_hosts_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-production-hosts.moc"
