#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QGuiApplication>
#include <QList>
#include <QTemporaryDir>
#include <QtTest>

#include <memory>
#include <thorvg.h>

#include "lottie-renderer.h"
#include "ukui/lottie/lottie-cache-manager.h"

using namespace UkuiQuick;

namespace {

LottieAsset assetForSource(const QString &source, const QString &theme = {})
{
    LottieAsset asset = LottieCacheManager::instance()->requestAsset(
        LottieSourceReference::fromText(source)).asset;
    if (asset.isValid()) {
        asset.setTheme(asset.preferredTheme(theme));
    }
    return asset;
}

QByteArray animationJson()
{
    return QByteArray(R"json({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"renderer-test",
        "ddd":0,
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
                {"ty":"fl","c":{"a":0,"k":[1,0,0,1]},"o":{"a":0,"k":100},"r":1,"bm":0,"nm":"Fill 1"},
                {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100},"sk":{"a":0,"k":0},"sa":{"a":0,"k":0},"nm":"Transform"}
            ],
            "ip":0,
            "op":2,
            "st":0,
            "bm":0
        }]
    })json");
}

QByteArray imageAnimationJson()
{
    return QByteArray(R"json({
        "v":"5.7.4","fr":30,"ip":0,"op":2,"w":16,"h":16,
        "nm":"image-renderer-test","ddd":0,
        "assets":[{"id":"image_0","w":16,"h":16,"u":"images/","p":"icon.png","e":0}],
        "layers":[{
            "ddd":0,"ind":1,"ty":2,"nm":"Image Layer","refId":"image_0","sr":1,
            "ks":{"o":{"a":0,"k":100},"r":{"a":0,"k":0},"p":{"a":0,"k":[8,8,0]},
                  "a":{"a":0,"k":[8,8,0]},"s":{"a":0,"k":[100,100,100]}},
            "ao":0,"ip":0,"op":2,"st":0,"bm":0
        }]
    })json");
}

QByteArray pngImage(const QColor &color)
{
    QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
        return {};
    }
    return bytes;
}

QByteArray pngWithCorruptImageData(const QColor &color)
{
    QByteArray bytes = pngImage(color);
    const int imageDataChunk = bytes.indexOf(QByteArrayLiteral("IDAT"));
    if (imageDataChunk < 0 || imageDataChunk + 6 >= bytes.size()) {
        return {};
    }

    // 保留 PNG 签名和 IHDR 尺寸，让入口预算检查可以读取元数据；只破坏
    // IDAT 内容，确保真正的图片解码器必须报告失败。
    bytes[imageDataChunk + 6] = static_cast<char>(
        bytes.at(imageDataChunk + 6) ^ 0x7f);
    return bytes;
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

QByteArray slotAnimationJson()
{
    return QByteArray(R"json({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":2,
        "w":16,
        "h":16,
        "nm":"renderer-theme-test",
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

QByteArray colorThemeJson(const QByteArray &color)
{
    return QByteArray(R"json({"rules":[{"id":"fillColor","type":"Color","value":)json") + color +
           QByteArrayLiteral("}]}");
}

QString writeThemedAnimation(QTemporaryDir *directory)
{
    const QByteArray archive = storedZip({
        {QByteArrayLiteral("manifest.json"), QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"test\",\"theme\":\"light\"},\"animations\":[{\"id\":\"test\",\"themes\":[\"light\",\"dark\"]}],\"themes\":[{\"id\":\"light\"},{\"id\":\"dark\"}]} ")},
        {QByteArrayLiteral("a/test.json"), slotAnimationJson()},
        {QByteArrayLiteral("t/light.json"), colorThemeJson(QByteArrayLiteral("[0,0,1,1]"))},
        {QByteArrayLiteral("t/dark.json"), colorThemeJson(QByteArrayLiteral("[0,1,0,1]"))}
    });
    const QString source = directory->filePath(QStringLiteral("animation.lottie"));
    QFile file(source);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
           file.write(archive) == archive.size()
        ? source
        : QString();
}

bool isDominantColor(const QImage &image, const QColor &color)
{
    const QColor center = image.pixelColor(image.width() / 2, image.height() / 2);
    return center.alpha() > 240 &&
           qAbs(center.red() - color.red()) < 10 &&
           qAbs(center.green() - color.green()) < 10 &&
           qAbs(center.blue() - color.blue()) < 10;
}

QString writeAnimation(QTemporaryDir *directory, const QByteArray &contents = animationJson())
{
    const QString source = directory->filePath(QStringLiteral("animation.json"));
    QFile file(source);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
        file.write(contents) != contents.size()) {
        return {};
    }
    return source;
}

}

class lottie_renderer_test : public QObject
{
    Q_OBJECT

private slots:
    void releasesThorVGEngineWhenLastRendererIsDestroyed();
    void loadsAndRendersAnimation();
    void publishedFramesRemainImmutableAcrossRenders();
    void normalizesFrameNumbers();
    void rejectsUnsafeRenderSizes();
    void rejectsInvalidAnimationMetadata();
    void rendersFromImmutableBoundImageSnapshot();
    void fallsBackToLaterBoundImageCandidate();
    void resolverDecodeFailureDoesNotProduceFrame();
};

void lottie_renderer_test::releasesThorVGEngineWhenLastRendererIsDestroyed()
{
    // This is intentionally the first test: no earlier renderer may leave a
    // ThorVG initialization reference in this process.
    {
        LottieRenderer renderer;
    }
    QCOMPARE(tvg::Initializer::term(), tvg::Result::InsufficientCondition);

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());
    const QByteArray originalAnimation = animationJson();
    const LottieAsset asset = assetForSource(source);
    QVERIFY(asset.isValid());
    QCOMPARE(asset.animationJsonData(), originalAnimation);

    // Hold a test-owned reference so the final term() below performs ThorVG's
    // real renderer-leak check after both LottieRenderer owners are gone.
    QCOMPARE(tvg::Initializer::init(0), tvg::Result::Success);
    auto firstRenderer = std::make_unique<LottieRenderer>();
    auto lastRenderer = std::make_unique<LottieRenderer>();
    firstRenderer.reset();

    QCOMPARE(lastRenderer->load(asset, {}), LottieRenderer::Failure::None);
    const auto frame = lastRenderer->imageForFrame(0, QSize(32, 32));
    QCOMPARE(frame.failure, LottieRenderer::Failure::None);
    QVERIFY(!frame.image.isNull());
    lastRenderer.reset();
    // ThorVG 的内存加载不得改写由缓存和其他请求共享的资产字节。
    QCOMPARE(asset.animationJsonData(), originalAnimation);
    QCOMPARE(assetForSource(source).animationJsonData(), originalAnimation);

    QCOMPARE(tvg::Initializer::term(), tvg::Result::Success);
    QCOMPARE(tvg::Initializer::term(), tvg::Result::InsufficientCondition);
}

void lottie_renderer_test::loadsAndRendersAnimation()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    const LottieAsset asset = assetForSource(source);
    QVERIFY(asset.isValid());
    LottieRenderer renderer;
    QCOMPARE(renderer.load(asset, {}), LottieRenderer::Failure::None);

    const LottieRenderer::FrameResult frameResult =
        renderer.imageForFrame(0, QSize(32, 32));
    QCOMPARE(frameResult.failure, LottieRenderer::Failure::None);
    const QImage image = frameResult.image;
    QVERIFY(!image.isNull());
    QCOMPARE(image.size(), QSize(32, 32));
    QVERIFY(image.pixelColor(16, 16).alpha() > 0);
}

void lottie_renderer_test::publishedFramesRemainImmutableAcrossRenders()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeThemedAnimation(&directory);
    QVERIFY(!source.isEmpty());

    const LottieAsset asset = assetForSource(source, QStringLiteral("light"));
    QVERIFY(asset.isValid());
    LottieRenderer renderer;
    QCOMPARE(renderer.load(asset,
                           QStringLiteral("light")),
             LottieRenderer::Failure::None);

    const QImage lightFrame = renderer.imageForFrame(0, QSize(32, 32)).image;
    QVERIFY(isDominantColor(lightFrame, Qt::blue));
    const QImage lightSnapshot = lightFrame.copy();

    QCOMPARE(renderer.load(asset, QStringLiteral("dark")), LottieRenderer::Failure::None);
    const QImage darkFrame = renderer.imageForFrame(0, QSize(32, 32)).image;
    QVERIFY(isDominantColor(darkFrame, Qt::green));
    QVERIFY(lightFrame.constBits() != darkFrame.constBits());
    QCOMPARE(lightFrame, lightSnapshot);

    const QImage resizedFrame = renderer.imageForFrame(0, QSize(17, 19)).image;
    QCOMPARE(resizedFrame.size(), QSize(17, 19));
    QVERIFY(darkFrame.constBits() != resizedFrame.constBits());
    QCOMPARE(lightFrame, lightSnapshot);

    QVERIFY(isDominantColor(darkFrame, Qt::green));
}

void lottie_renderer_test::normalizesFrameNumbers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory);
    QVERIFY(!source.isEmpty());

    LottieRenderer renderer;
    QCOMPARE(renderer.load(assetForSource(source), {}), LottieRenderer::Failure::None);
    const QImage lastFrame = renderer.imageForFrame(-1, QSize(16, 16)).image;
    const QImage wrappedFrame = renderer.imageForFrame(3, QSize(16, 16)).image;
    QVERIFY(!lastFrame.isNull());
    QVERIFY(!wrappedFrame.isNull());
    QCOMPARE(lastFrame.size(), QSize(16, 16));
    QCOMPARE(wrappedFrame.size(), QSize(16, 16));
}

void lottie_renderer_test::rejectsUnsafeRenderSizes()
{
    QVERIFY(!LottieRenderer::isRenderSizeAllowed(QSize()));
    QVERIFY(!LottieRenderer::isRenderSizeAllowed(QSize(8193, 1)));
    QVERIFY(!LottieRenderer::isRenderSizeAllowed(QSize(8192, 8193)));
    QVERIFY(LottieRenderer::isRenderSizeAllowed(QSize(8192, 8192)));
}

void lottie_renderer_test::rejectsInvalidAnimationMetadata()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QByteArray invalidAnimation = animationJson();
    invalidAnimation.replace(QByteArrayLiteral("\"fr\":30"), QByteArrayLiteral("\"fr\":0"));
    const QString source = writeAnimation(&directory, invalidAnimation);
    QVERIFY(!source.isEmpty());

    LottieRenderer renderer;
    QCOMPARE(renderer.load(assetForSource(source), {}), LottieRenderer::Failure::Source);
    QCOMPARE(renderer.imageForFrame(0, QSize(16, 16)).failure,
             LottieRenderer::Failure::Raster);
}

void lottie_renderer_test::rendersFromImmutableBoundImageSnapshot()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, imageAnimationJson());
    QVERIFY(!source.isEmpty());
    QVERIFY(QDir().mkpath(directory.filePath(QStringLiteral("images"))));
    const QString imagePath = directory.filePath(QStringLiteral("images/icon.png"));
    QFile imageFile(imagePath);
    const QByteArray redImage = pngImage(Qt::red);
    QVERIFY(imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(imageFile.write(redImage), static_cast<qint64>(redImage.size()));
    imageFile.close();

    const LottieAsset asset = assetForSource(source);
    QVERIFY(asset.isValid());
    QVERIFY(asset.boundImageResource(QStringLiteral("images/icon.png")).isValid());

    // 资产发布后原文件可以被替换；这份资产仍必须渲染它已经验证的红色快照。
    const QByteArray blueImage = pngImage(Qt::blue);
    QVERIFY(imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(imageFile.write(blueImage), static_cast<qint64>(blueImage.size()));
    imageFile.close();

    LottieRenderer renderer;
    QCOMPARE(renderer.load(asset, {}), LottieRenderer::Failure::None);
    const QImage frame = renderer.imageForFrame(0, QSize(16, 16)).image;
    const QColor center = frame.pixelColor(8, 8);
    QVERIFY2(isDominantColor(frame, Qt::red),
             qPrintable(QStringLiteral("center=%1,%2,%3,%4")
                            .arg(center.red()).arg(center.green())
                            .arg(center.blue()).arg(center.alpha())));

    const LottieAsset replacement = assetForSource(source);
    QVERIFY(replacement.isValid());
    QVERIFY(replacement.revision() != asset.revision());
    QCOMPARE(renderer.load(replacement, {}), LottieRenderer::Failure::None);
    QVERIFY(isDominantColor(renderer.imageForFrame(0, QSize(16, 16)).image,
                            Qt::blue));
}

void lottie_renderer_test::fallsBackToLaterBoundImageCandidate()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, imageAnimationJson());
    QVERIFY(!source.isEmpty());
    QVERIFY(QDir().mkpath(directory.filePath(QStringLiteral("images"))));

    QImage unsupportedImage(16, 16, QImage::Format_ARGB32_Premultiplied);
    unsupportedImage.fill(Qt::red);
    QVERIFY(unsupportedImage.save(
        directory.filePath(QStringLiteral("images/icon.png")), "BMP"));

    QFile fallbackFile(directory.filePath(QStringLiteral("icon.png")));
    const QByteArray fallbackImage = pngImage(Qt::green);
    QVERIFY(fallbackFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(fallbackFile.write(fallbackImage),
             static_cast<qint64>(fallbackImage.size()));
    fallbackFile.close();

    const LottieAsset asset = assetForSource(source);
    QVERIFY(asset.isValid());

    LottieRenderer renderer;
    QCOMPARE(renderer.load(asset, {}), LottieRenderer::Failure::None);
    const QImage frame = renderer.imageForFrame(0, QSize(16, 16)).image;
    QVERIFY(isDominantColor(frame, Qt::green));

    const QByteArray replacementImage = pngImage(Qt::blue);
    QVERIFY(fallbackFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(fallbackFile.write(replacementImage),
             static_cast<qint64>(replacementImage.size()));
    fallbackFile.close();

    const LottieAsset replacement = assetForSource(source);
    QVERIFY(replacement.isValid());
    QVERIFY(replacement.revision() != asset.revision());
    QCOMPARE(renderer.load(replacement, {}), LottieRenderer::Failure::None);
    QVERIFY(isDominantColor(renderer.imageForFrame(0, QSize(16, 16)).image,
                            Qt::blue));
}

void lottie_renderer_test::resolverDecodeFailureDoesNotProduceFrame()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = writeAnimation(&directory, imageAnimationJson());
    QVERIFY(!source.isEmpty());
    QVERIFY(QDir().mkpath(directory.filePath(QStringLiteral("images"))));

    const QByteArray corruptImage = pngWithCorruptImageData(Qt::red);
    QVERIFY(!corruptImage.isEmpty());
    QFile imageFile(directory.filePath(QStringLiteral("images/icon.png")));
    QVERIFY(imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(imageFile.write(corruptImage),
             static_cast<qint64>(corruptImage.size()));
    imageFile.close();

    // 入口检查能够识别格式和安全尺寸，但 ThorVG 仍必须实际解码图片。
    // resolver 解码失败不能被伪装成一次成功的动画加载。
    const LottieAsset asset = assetForSource(source);
    QVERIFY(asset.isValid());
    QVERIFY(asset.boundImageResource(QStringLiteral("images/icon.png")).isValid());

    LottieRenderer renderer;
    const LottieRenderer::Failure loadFailure = renderer.load(asset, {});
    QVERIFY(loadFailure == LottieRenderer::Failure::None ||
            loadFailure == LottieRenderer::Failure::Source);
    if (loadFailure == LottieRenderer::Failure::None) {
        const LottieRenderer::FrameResult frame =
            renderer.imageForFrame(0, QSize(16, 16));
        QCOMPARE(frame.failure, LottieRenderer::Failure::Raster);
        QVERIFY(frame.image.isNull());
    }
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
    lottie_renderer_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-lottie-renderer.moc"
