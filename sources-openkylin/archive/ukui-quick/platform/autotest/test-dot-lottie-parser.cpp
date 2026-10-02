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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

#include "../ukui/lottie/dot-lottie-parser.h"
#include "../ukui/lottie/dot-lottie-parser-json_p.h"

namespace {

bool writeFile(const QString &path, const QByteArray &contents)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(contents) == contents.size();
}

QByteArray animationJson()
{
    return QByteArrayLiteral(R"json({
        "v":"5.7.4",
        "fr":30,
        "ip":0,
        "op":60,
        "w":24,
        "h":24,
        "layers":[]
    })json");
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
    // >= 0 时中央目录声明该解压大小（可大于实际内容以构造超预算归档）。
    qint64 declaredSize = -1;
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
        const quint32 declaredSize = entry.declaredSize >= 0
            ? static_cast<quint32>(entry.declaredSize)
            : static_cast<quint32>(entry.contents.size());
        appendLittleEndian32(archive, 0x02014b50U);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, checksum);
        appendLittleEndian32(archive, declaredSize);
        appendLittleEndian32(archive, declaredSize);
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

QString canonicalPath(const QString &path)
{
    return QFileInfo(path).canonicalFilePath();
}

}

class dot_lottie_parser_test : public QObject
{
    Q_OBJECT

private slots:
    void createsJsonAssetOnlyForValidAnimations();
    void publishesFrameCountAndRateAsLoadingMetadata();
    void rejectsOversizedJsonInput();
    void jsonParseBoundaryContract();
    void fallbackAnimationUsesBoundedJsonBoundary();
    void cancelledManifestPostParseWinsOverParseError();
    void rejectsOversizedManifestViaParseExtractedDirectory();
    void oversizedThemeDoesNotEnterJsonParseOrSlot();
    void parsesV2ManifestThemesAndSlots();
    void parsesLegacyManifestAndFallsBackWithoutOne();
    void rejectsMalformedManifest();
    void rejectsThemeOverflowInsteadOfTruncation();
    void extractsArchiveAndRejectsUnsafeEntries();
    void enforcesArchiveBudgets();
    void cancellationAbortsArchiveWorkAndLeavesCacheReusable();
};

void dot_lottie_parser_test::createsJsonAssetOnlyForValidAnimations()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString animationPath = QDir(temporaryDir.path()).filePath(QStringLiteral("animations/scene.json"));
    const QString invalidPath = QDir(temporaryDir.path()).filePath(QStringLiteral("animations/invalid.json"));
    QVERIFY(writeFile(animationPath, animationJson()));
    QVERIFY(writeFile(invalidPath, QByteArrayLiteral("{\"fr\":30}")));

    UkuiQuick::DotLottieParser parser;
    const UkuiQuick::LottieAsset asset = parser.createJsonAsset(animationPath);
    QVERIFY(asset.isValid());
    QCOMPARE(asset.animationJsonPath(), animationPath);
    QCOMPARE(asset.resourceDir(), QFileInfo(animationPath).absolutePath());
    QCOMPARE(asset.resourceSearchDirs(),
             QStringList({QFileInfo(animationPath).absolutePath(),
                          QDir(QFileInfo(animationPath).absolutePath()).filePath(QStringLiteral("images")),
                          QDir(QFileInfo(animationPath).absolutePath()).filePath(QStringLiteral("i"))}));
    QVERIFY(!parser.createJsonAsset(invalidPath).isValid());
    QVERIFY(!parser.createJsonAsset(QDir(temporaryDir.path()).filePath(QStringLiteral("missing.json"))).isValid());
}

void dot_lottie_parser_test::publishesFrameCountAndRateAsLoadingMetadata()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString animationPath = QDir(temporaryDir.path()).filePath(QStringLiteral("scene.json"));
    QVERIFY(writeFile(animationPath, animationJson()));

    UkuiQuick::DotLottieParser parser;
    const UkuiQuick::LottieAsset jsonAsset = parser.createJsonAsset(animationPath);
    QVERIFY(jsonAsset.isValid());
    QCOMPARE(jsonAsset.frameCount(), 60);
    QCOMPARE(jsonAsset.frameRate(), 30.0);

    // dotLottie 目录中的动画在帧提交前就能发布总帧数与帧率。
    const QString extractedDir = QDir(temporaryDir.path()).filePath(QStringLiteral("extracted"));
    const QByteArray customAnimation = QByteArrayLiteral(R"json({
        "v":"5.7.4","fr":24,"ip":5,"op":29,"w":16,"h":16,"layers":[]
    })json");
    QVERIFY(writeFile(QDir(extractedDir).filePath(QStringLiteral("a/main.json")), customAnimation));

    QString error;
    const UkuiQuick::LottieAsset extractedAsset = parser.parseExtractedDirectory(
        QStringLiteral("metadata.lottie"), extractedDir, QString(), &error);
    QVERIFY2(extractedAsset.isValid(), qPrintable(error));
    QCOMPARE(extractedAsset.frameCount(), 24);
    QCOMPARE(extractedAsset.frameRate(), 24.0);
}

void dot_lottie_parser_test::rejectsOversizedJsonInput()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString oversizedPath = QDir(temporaryDir.path()).filePath(QStringLiteral("oversized.json"));
    {
        QFile file(oversizedPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, ' ');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(file.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }

    // 直接调用 Parser（不经过 CacheManager）：readFileData 在 QJsonDocument::fromJson
    // 之前按 maxJsonInputBytes 拒绝超限输入。
    UkuiQuick::DotLottieParser parser;
    const UkuiQuick::LottieAsset asset = parser.createJsonAsset(oversizedPath);
    QVERIFY(!asset.isValid());

    // 恰好不超过上限的 JSON 仍可解析。
    const QString boundedPath = QDir(temporaryDir.path()).filePath(QStringLiteral("bounded.json"));
    {
        QByteArray content = animationJson();
        content.append(QByteArray(static_cast<int>(UkuiQuick::DotLottieParser::maxJsonInputBytes()) -
                                  content.size(), ' '));
        QCOMPARE(content.size(), static_cast<int>(UkuiQuick::DotLottieParser::maxJsonInputBytes()));
        QFile file(boundedPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(content), static_cast<qint64>(content.size()));
    }
    const UkuiQuick::LottieAsset boundedAsset = parser.createJsonAsset(boundedPath);
    QVERIFY(boundedAsset.isValid());
    QCOMPARE(boundedAsset.frameCount(), 60);
}

void dot_lottie_parser_test::jsonParseBoundaryContract()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QString animationPath = QDir(temporaryDir.path()).filePath(QStringLiteral("anim.json"));
    QVERIFY(writeFile(animationPath, animationJson()));

    const UkuiQuick::LottieParserInternal::JsonParseResult parsed =
        UkuiQuick::LottieParserInternal::parseJsonFile(animationPath);
    QVERIFY(parsed.hasInput);
    QVERIFY(parsed.status == UkuiQuick::LottieParserInternal::JsonParseStatus::Parsed);
    QVERIFY(parsed.document.isObject());

    // These are the helper's boundary probes; no production call-chain total
    // is asserted.
    int preParseChecks = 0;
    const auto cancelBeforeParse = [&] { return ++preParseChecks == 3; };
    const UkuiQuick::LottieParserInternal::JsonParseResult beforeParse =
        UkuiQuick::LottieParserInternal::parseJsonFile(animationPath, cancelBeforeParse);
    QVERIFY(beforeParse.wasCancelled());
    QVERIFY(beforeParse.status == UkuiQuick::LottieParserInternal::JsonParseStatus::CancelledBeforeParse);
    QVERIFY(beforeParse.hasInput);
    QVERIFY(beforeParse.document.isNull());

    const QString malformedPath = QDir(temporaryDir.path()).filePath(QStringLiteral("malformed.json"));
    QVERIFY(writeFile(malformedPath, QByteArrayLiteral("{ not valid json")));
    int postParseChecks = 0;
    const auto cancelAfterParse = [&] { return ++postParseChecks == 4; };
    const UkuiQuick::LottieParserInternal::JsonParseResult afterParse =
        UkuiQuick::LottieParserInternal::parseJsonFile(malformedPath, cancelAfterParse);
    QVERIFY(afterParse.wasCancelled());
    QVERIFY(afterParse.status == UkuiQuick::LottieParserInternal::JsonParseStatus::CancelledAfterParse);
    QVERIFY(afterParse.hasInput);
    QVERIFY(afterParse.parseError.error != QJsonParseError::NoError);

    const QString oversizedPath = QDir(temporaryDir.path()).filePath(QStringLiteral("oversized.json"));
    QFile oversized(oversizedPath);
    QVERIFY(oversized.open(QIODevice::WriteOnly));
    const QByteArray chunk(1024 * 1024, ' ');
    for (int index = 0; index < 17; ++index) {
        QCOMPARE(oversized.write(chunk), static_cast<qint64>(chunk.size()));
    }
    oversized.close();
    const UkuiQuick::LottieParserInternal::JsonParseResult bounded =
        UkuiQuick::LottieParserInternal::parseJsonFile(oversizedPath);
    QVERIFY(!bounded.hasInput);
    QVERIFY(bounded.status == UkuiQuick::LottieParserInternal::JsonParseStatus::NoInput);
    QVERIFY(bounded.readError.contains(QStringLiteral("size budget")));
}

void dot_lottie_parser_test::fallbackAnimationUsesBoundedJsonBoundary()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    QVERIFY(QDir().mkpath(QDir(temporaryDir.path()).filePath(QStringLiteral("a"))));
    QFile animation(QDir(temporaryDir.path()).filePath(QStringLiteral("a/oversized.json")));
    QVERIFY(animation.open(QIODevice::WriteOnly));
    const QByteArray chunk(1024 * 1024, ' ');
    for (int index = 0; index < 17; ++index) {
        QCOMPARE(animation.write(chunk), static_cast<qint64>(chunk.size()));
    }
    animation.close();

    UkuiQuick::DotLottieParser parser;
    QString error;
    QVERIFY(!parser.parseExtractedDirectory(
        QStringLiteral("fallback.lottie"), temporaryDir.path(), QString(), &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("no animation json found")), qPrintable(error));
}

void dot_lottie_parser_test::cancelledManifestPostParseWinsOverParseError()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("manifest.json")),
                      QByteArrayLiteral("{ not valid json")));

    UkuiQuick::DotLottieParser parser;
    QString error;
    QVERIFY(!parser.parseExtractedDirectory(
        QStringLiteral("malformed.lottie"), temporaryDir.path(), QString(), &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("failed to parse manifest")), qPrintable(error));

    // The callback is armed for the manifest helper's post-fromJson checkpoint.
    // The helper must report cancellation before the malformed parse error is mapped.
    int checks = 0;
    const auto cancelAfterManifestParse = [&] { return ++checks == 6; };
    QVERIFY(!parser.parseExtractedDirectory(QStringLiteral("malformed.lottie"),
                                             temporaryDir.path(),
                                             QString(),
                                             &error,
                                             cancelAfterManifestParse)
                 .isValid());
    QVERIFY2(error.contains(QStringLiteral("cancelled")), qPrintable(error));
    QVERIFY2(!error.contains(QStringLiteral("failed to parse manifest")), qPrintable(error));
}

void dot_lottie_parser_test::rejectsOversizedManifestViaParseExtractedDirectory()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    // 合法动画文件 + 超大 manifest（> 16 MiB）：manifest 不得进入 JSON 解析。
    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("a/main.json")), animationJson()));
    {
        QFile manifest(QDir(temporaryDir.path()).filePath(QStringLiteral("manifest.json")));
        QVERIFY(manifest.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, ' ');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(manifest.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }

    UkuiQuick::DotLottieParser parser;
    QString error;
    const UkuiQuick::LottieAsset asset = parser.parseExtractedDirectory(
        QStringLiteral("oversized-manifest.lottie"), temporaryDir.path(), QString(), &error);
    QVERIFY(!asset.isValid());
    QVERIFY2(error.contains(QStringLiteral("size budget")), qPrintable(error));
}

void dot_lottie_parser_test::oversizedThemeDoesNotEnterJsonParseOrSlot()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    // 构造语义合法、能生成非空 slot 的 theme JSON，用 JSON 合法的尾随空白填充到 > 16 MiB。
    // 当前实现：readFileData 在 16 MiB 上限处拒绝 → theme 被跳过 → slot 为空。
    // 旧实现（5ffad56）：readFileData 无 maxBytes → 完整读入 → fromJson 解析成功（尾随空白合法）→ slot 非空。
    const QByteArray manifest = QByteArrayLiteral(R"json({
        "version":"2","initial":{"animation":"main","theme":"big"},
        "animations":[{"id":"main"}],"themes":["big"]
    })json");
    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("manifest.json")), manifest));
    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("a/main.json")), animationJson()));
    QVERIFY(QDir().mkpath(QDir(temporaryDir.path()).filePath(QStringLiteral("t"))));
    {
        QFile theme(QDir(temporaryDir.path()).filePath(QStringLiteral("t/big.json")));
        QVERIFY(theme.open(QIODevice::WriteOnly));
        // 语义合法的 theme JSON。
        const QByteArray themeJson = QByteArrayLiteral(
            R"({"rules":[{"id":"fill","type":"Color","value":[1,0,0]}]})");
        QCOMPARE(theme.write(themeJson), static_cast<qint64>(themeJson.size()));
        // JSON 合法的尾随空白填充到 > 16 MiB。
        const QByteArray chunk(1024 * 1024, ' ');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(theme.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }

    UkuiQuick::DotLottieParser parser;
    QString error;
    const UkuiQuick::LottieAsset asset = parser.parseExtractedDirectory(
        QStringLiteral("oversized-theme.lottie"), temporaryDir.path(), QString(), &error);
    QVERIFY2(asset.isValid(), qPrintable(error));
    QCOMPARE(asset.availableThemes(), QStringList({QStringLiteral("big")}));
    // 超大 theme 被预算拒绝：slot JSON 未生成。
    QVERIFY(asset.themeSlotJson(QStringLiteral("big")).isEmpty());
}

void dot_lottie_parser_test::parsesV2ManifestThemesAndSlots()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString animationPath = QDir(temporaryDir.path()).filePath(QStringLiteral("a/main.json"));
    const QString darkThemePath = QDir(temporaryDir.path()).filePath(QStringLiteral("t/dark.json"));
    const QString lightThemePath = QDir(temporaryDir.path()).filePath(QStringLiteral("t/light.json"));
    const QByteArray manifest = QByteArrayLiteral(R"json({
        "version":"2",
        "initial":{"animation":"main.json","theme":"dark"},
        "themes":[{"id":"dark.json"},"light",{"id":"dark"}],
        "animations":[{"id":"main.json","initialTheme":"light","themes":["light","dark"]}]
    })json");
    const QByteArray darkTheme = QByteArrayLiteral(R"json({
        "rules":[
            {"id":"fill","type":"Color","value":[1,0.5,0],"animations":["main.json"]},
            {"id":"unrelated","type":"Color","value":[0,0,0],"animations":["other"]},
            {"id":"animated","type":"Color","expression":"themeColor","keyframes":[
                {"frame":3,"value":[0,1,0,1],"inTangent":{"x":0.1,"y":0.2},
                 "outTangent":{"x":0.3,"y":0.4},"valueInTangent":0.25,
                 "valueOutTangent":0.75,"hold":true}
            ]}
        ]
    })json");
    QVERIFY(writeFile(animationPath, animationJson()));
    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("manifest.json")), manifest));
    QVERIFY(writeFile(darkThemePath, darkTheme));
    QVERIFY(writeFile(lightThemePath, QByteArrayLiteral("{\"rules\":[]}")));

    UkuiQuick::DotLottieParser parser;
    QString error;
    const UkuiQuick::LottieAsset asset = parser.parseExtractedDirectory(
        QStringLiteral("fixture.lottie"), temporaryDir.path(), QStringLiteral("light"), &error);
    QVERIFY2(asset.isValid(), qPrintable(error));
    QCOMPARE(asset.extractDir(), canonicalPath(temporaryDir.path()));
    QCOMPARE(asset.animationJsonPath(), canonicalPath(animationPath));
    QCOMPARE(asset.resourceDir(), QFileInfo(animationPath).absolutePath());
    QCOMPARE(asset.activeAnimationId(), QStringLiteral("main"));
    QCOMPARE(asset.availableThemes(), QStringList({QStringLiteral("dark"), QStringLiteral("light")}));
    QCOMPARE(asset.activeAnimationThemes(), QStringList({QStringLiteral("light"), QStringLiteral("dark")}));
    QVERIFY(asset.activeAnimationRestrictsThemes());
    QCOMPARE(asset.theme(), QStringLiteral("light"));

    const UkuiQuick::LottieAsset defaultThemeAsset = parser.parseExtractedDirectory(
        QStringLiteral("fixture.lottie"), temporaryDir.path(), QStringLiteral("missing"), &error);
    QVERIFY2(defaultThemeAsset.isValid(), qPrintable(error));
    QCOMPARE(defaultThemeAsset.theme(), QStringLiteral("dark"));

    QJsonParseError parseError;
    const QJsonDocument slotDocument = QJsonDocument::fromJson(asset.themeSlotJson(QStringLiteral("dark")),
                                                                &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QVERIFY(slotDocument.isObject());
    const QJsonObject themeSlots = slotDocument.object();
    QVERIFY(themeSlots.contains(QStringLiteral("fill")));
    QVERIFY(!themeSlots.contains(QStringLiteral("unrelated")));
    const QJsonObject fill = themeSlots.value(QStringLiteral("fill")).toObject()
                                 .value(QStringLiteral("p")).toObject();
    QCOMPARE(fill.value(QStringLiteral("a")).toInt(), 0);
    const QJsonArray fillColor = fill.value(QStringLiteral("k")).toArray();
    QCOMPARE(fillColor.size(), 3);
    QCOMPARE(fillColor.at(0).toDouble(), 1.0);
    QCOMPARE(fillColor.at(1).toDouble(), 0.5);
    QCOMPARE(fillColor.at(2).toDouble(), 0.0);

    const QJsonObject animated = themeSlots.value(QStringLiteral("animated")).toObject()
                                      .value(QStringLiteral("p")).toObject();
    QCOMPARE(animated.value(QStringLiteral("a")).toInt(), 1);
    QCOMPARE(animated.value(QStringLiteral("x")).toString(), QStringLiteral("themeColor"));
    const QJsonObject keyframe = animated.value(QStringLiteral("k")).toArray().first().toObject();
    QCOMPARE(keyframe.value(QStringLiteral("t")).toInt(), 3);
    QCOMPARE(keyframe.value(QStringLiteral("h")).toInt(), 1);
    QCOMPARE(keyframe.value(QStringLiteral("i")).toObject().value(QStringLiteral("x")).toDouble(), 0.1);
    QCOMPARE(keyframe.value(QStringLiteral("o")).toObject().value(QStringLiteral("y")).toDouble(), 0.4);
    QCOMPARE(keyframe.value(QStringLiteral("ti")).toDouble(), 0.25);
    QCOMPARE(keyframe.value(QStringLiteral("to")).toDouble(), 0.75);
}

void dot_lottie_parser_test::parsesLegacyManifestAndFallsBackWithoutOne()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString legacyDir = QDir(temporaryDir.path()).filePath(QStringLiteral("legacy"));
    const QString legacyAnimation = QDir(legacyDir).filePath(QStringLiteral("animations/legacy.json"));
    QVERIFY(writeFile(legacyAnimation, animationJson()));
    QVERIFY(writeFile(QDir(legacyDir).filePath(QStringLiteral("manifest.json")), QByteArrayLiteral(R"json({
        "version":"1",
        "initial":{"animation":"legacy"},
        "animations":[{"id":"legacy"}]
    })json")));

    UkuiQuick::DotLottieParser parser;
    QString error;
    const UkuiQuick::LottieAsset legacyAsset = parser.parseExtractedDirectory(
        QStringLiteral("legacy.lottie"), legacyDir, QString(), &error);
    QVERIFY2(legacyAsset.isValid(), qPrintable(error));
    QCOMPARE(legacyAsset.animationJsonPath(), canonicalPath(legacyAnimation));
    QCOMPARE(legacyAsset.activeAnimationId(), QStringLiteral("legacy"));

    const QString fallbackDir = QDir(temporaryDir.path()).filePath(QStringLiteral("fallback"));
    const QString fallbackAnimation = QDir(fallbackDir).filePath(QStringLiteral("a/first.json"));
    QVERIFY(writeFile(QDir(fallbackDir).filePath(QStringLiteral("a/not-lottie.json")), QByteArrayLiteral("{}")));
    QVERIFY(writeFile(fallbackAnimation, animationJson()));
    QVERIFY(writeFile(QDir(fallbackDir).filePath(QStringLiteral("animations/second.json")), animationJson()));
    QVERIFY(writeFile(QDir(fallbackDir).filePath(QStringLiteral("t/sun.json")), QByteArrayLiteral("{\"rules\":[]}")));

    const UkuiQuick::LottieAsset fallbackAsset = parser.parseExtractedDirectory(
        QStringLiteral("fallback.lottie"), fallbackDir, QStringLiteral("sun"), &error);
    QVERIFY2(fallbackAsset.isValid(), qPrintable(error));
    QCOMPARE(fallbackAsset.animationJsonPath(), canonicalPath(fallbackAnimation));
    QVERIFY(fallbackAsset.activeAnimationId().isEmpty());
    QCOMPARE(fallbackAsset.availableThemes(), QStringList({QStringLiteral("sun")}));
    QCOMPARE(fallbackAsset.theme(), QStringLiteral("sun"));
}

void dot_lottie_parser_test::rejectsMalformedManifest()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    QVERIFY(writeFile(QDir(temporaryDir.path()).filePath(QStringLiteral("manifest.json")),
                      QByteArrayLiteral("{ not valid json")));

    UkuiQuick::DotLottieParser parser;
    QString error;
    const UkuiQuick::LottieAsset asset = parser.parseExtractedDirectory(
        QStringLiteral("broken.lottie"), temporaryDir.path(), QString(), &error);
    QVERIFY(!asset.isValid());
    QVERIFY(error.startsWith(QStringLiteral("failed to parse manifest:")));
}

void dot_lottie_parser_test::rejectsThemeOverflowInsteadOfTruncation()
{
    const auto writeThemedDirectory = [](const QString &directory, int themeCount) {
        QJsonArray themes;
        for (int index = 0; index < themeCount; ++index) {
            themes.append(QStringLiteral("theme-%1").arg(index, 3, 10, QLatin1Char('0')));
        }
        QJsonObject manifest;
        manifest.insert(QStringLiteral("version"), QStringLiteral("2"));
        manifest.insert(QStringLiteral("themes"), themes);
        QJsonObject initial;
        initial.insert(QStringLiteral("animation"), QStringLiteral("main"));
        manifest.insert(QStringLiteral("initial"), initial);
        QJsonObject animation;
        animation.insert(QStringLiteral("id"), QStringLiteral("main"));
        manifest.insert(QStringLiteral("animations"), QJsonArray{animation});

        return writeFile(QDir(directory).filePath(QStringLiteral("manifest.json")),
                         QJsonDocument(manifest).toJson(QJsonDocument::Compact)) &&
               writeFile(QDir(directory).filePath(QStringLiteral("a/main.json")), animationJson());
    };

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    UkuiQuick::DotLottieParser parser;
    QString error;

    const QString overflowDir = QDir(temporaryDir.path()).filePath(QStringLiteral("overflow"));
    QVERIFY(writeThemedDirectory(overflowDir, UkuiQuick::DotLottieParser::maxSelectableThemes() + 1));
    const UkuiQuick::LottieAsset overflowAsset = parser.parseExtractedDirectory(
        QStringLiteral("overflow.lottie"), overflowDir, QString(), &error);
    QVERIFY(!overflowAsset.isValid());
    QVERIFY(error.contains(QStringLiteral("too many selectable themes")));

    const QString cappedDir = QDir(temporaryDir.path()).filePath(QStringLiteral("capped"));
    QVERIFY(writeThemedDirectory(cappedDir, UkuiQuick::DotLottieParser::maxSelectableThemes()));
    const UkuiQuick::LottieAsset cappedAsset = parser.parseExtractedDirectory(
        QStringLiteral("capped.lottie"), cappedDir, QString(), &error);
    QVERIFY2(cappedAsset.isValid(), qPrintable(error));
    QCOMPARE(cappedAsset.availableThemes().size(), UkuiQuick::DotLottieParser::maxSelectableThemes());
    QCOMPARE(cappedAsset.availableThemes().constFirst(), QStringLiteral("theme-000"));
    QCOMPARE(cappedAsset.availableThemes().constLast(), QStringLiteral("theme-255"));
}

void dot_lottie_parser_test::extractsArchiveAndRejectsUnsafeEntries()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString archivePath = QDir(temporaryDir.path()).filePath(QStringLiteral("animation.lottie"));
    const QByteArray manifest = QByteArrayLiteral(R"json({
        "version":"2",
        "initial":{"animation":"archived"},
        "animations":[{"id":"archived"}]
    })json");
    QVERIFY(writeFile(archivePath, storedZip({
        {QByteArrayLiteral("manifest.json"), manifest},
        {QByteArrayLiteral("a/archived.json"), animationJson()}
    })));

    UkuiQuick::DotLottieParser parser;
    QString error;
    const QString extractDir = QDir(temporaryDir.path()).filePath(QStringLiteral("cache/generation"));
    const UkuiQuick::LottieAsset asset = parser.extractArchiveAndParse(
        archivePath, archivePath, extractDir, QString(), &error);
    QVERIFY2(asset.isValid(), qPrintable(error));
    QCOMPARE(asset.animationJsonPath(),
             canonicalPath(QDir(extractDir).filePath(QStringLiteral("a/archived.json"))));
    QCOMPARE(asset.activeAnimationId(), QStringLiteral("archived"));
    QVERIFY(QFileInfo(QDir(extractDir).filePath(QStringLiteral(".complete"))).isFile());

    const QString unsafeArchive = QDir(temporaryDir.path()).filePath(QStringLiteral("unsafe.lottie"));
    QVERIFY(writeFile(unsafeArchive, storedZip({
        {QByteArrayLiteral("../outside.json"), animationJson()}
    })));
    const QString unsafeExtractDir = QDir(temporaryDir.path()).filePath(QStringLiteral("cache/unsafe"));
    const UkuiQuick::LottieAsset unsafeAsset = parser.extractArchiveAndParse(
        unsafeArchive, unsafeArchive, unsafeExtractDir, QString(), &error);
    QVERIFY(!unsafeAsset.isValid());
    QVERIFY(!error.isEmpty());
    QVERIFY(!QFileInfo(QDir(temporaryDir.path()).filePath(QStringLiteral("outside.json"))).exists());
    QVERIFY(!QFileInfo(unsafeExtractDir).exists());
}

void dot_lottie_parser_test::enforcesArchiveBudgets()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    UkuiQuick::DotLottieParser parser;
    QString error;
    int extractCounter = 0;
    const auto nextExtractDir = [&] {
        return QDir(temporaryDir.path()).filePath(
            QStringLiteral("cache/generation-%1").arg(++extractCounter));
    };

    // 压缩输入预算：超过 16 MiB 的输入不进入解压流程。
    const QString oversizedPath = QDir(temporaryDir.path()).filePath(QStringLiteral("oversized.lottie"));
    {
        QFile oversized(oversizedPath);
        QVERIFY(oversized.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, '\0');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(oversized.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }
    QVERIFY(!parser.extractArchiveAndParse(oversizedPath,
                                           oversizedPath,
                                           nextExtractDir(),
                                           QString(),
                                           &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("compressed input")), qPrintable(error));

    // 条目数预算：257 个条目拒绝，256 个条目放行。
    QList<ZipEntry> manyEntries;
    const QByteArray manifest = QByteArrayLiteral(R"json({
        "version":"2","initial":{"animation":"main"},"animations":[{"id":"main"}]
    })json");
    manyEntries.append({QByteArrayLiteral("manifest.json"), manifest, 0, -1});
    manyEntries.append({QByteArrayLiteral("a/main.json"), animationJson(), 0, -1});
    while (manyEntries.size() < UkuiQuick::DotLottieParser::maxArchiveEntries()) {
        const QByteArray name = QByteArrayLiteral("images/pad-") +
            QByteArray::number(manyEntries.size()) + QByteArrayLiteral(".png");
        manyEntries.append({name, QByteArrayLiteral("pad"), 0, -1});
    }
    QCOMPARE(manyEntries.size(), UkuiQuick::DotLottieParser::maxArchiveEntries());
    const QString maxEntriesPath = QDir(temporaryDir.path()).filePath(QStringLiteral("max-entries.lottie"));
    QVERIFY(writeFile(maxEntriesPath, storedZip(manyEntries)));
    const UkuiQuick::LottieAsset maxEntriesAsset = parser.extractArchiveAndParse(
        maxEntriesPath, maxEntriesPath, nextExtractDir(), QString(), &error);
    QVERIFY2(maxEntriesAsset.isValid(), qPrintable(error));

    manyEntries.append({QByteArrayLiteral("images/pad-extra.png"), QByteArrayLiteral("pad"), 0, -1});
    const QString overflowEntriesPath = QDir(temporaryDir.path()).filePath(QStringLiteral("overflow-entries.lottie"));
    QVERIFY(writeFile(overflowEntriesPath, storedZip(manyEntries)));
    QVERIFY(!parser.extractArchiveAndParse(overflowEntriesPath,
                                           overflowEntriesPath,
                                           nextExtractDir(),
                                           QString(),
                                           &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("entry count")), qPrintable(error));

    // 单条目解压预算：声明超过 16 MiB 的条目拒绝。
    const QString hugeEntryPath = QDir(temporaryDir.path()).filePath(QStringLiteral("huge-entry.lottie"));
    QVERIFY(writeFile(hugeEntryPath, storedZip({
        {QByteArrayLiteral("manifest.json"), manifest, 0, -1},
        {QByteArrayLiteral("a/main.json"), animationJson(), 0, -1},
        {QByteArrayLiteral("images/huge.png"), QByteArrayLiteral("small"),
         0, UkuiQuick::DotLottieParser::maxExpandedEntryBytes() + 1}
    })));
    QVERIFY(!parser.extractArchiveAndParse(hugeEntryPath,
                                           hugeEntryPath,
                                           nextExtractDir(),
                                           QString(),
                                           &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("expanded size")), qPrintable(error));

    // 总解压预算：声明总量超过 64 MiB 拒绝。
    const QString totalPath = QDir(temporaryDir.path()).filePath(QStringLiteral("total-budget.lottie"));
    {
        const qint64 chunkDeclared = UkuiQuick::DotLottieParser::maxTotalExpandedBytes() / 4;
        QList<ZipEntry> entries;
        entries.append({QByteArrayLiteral("manifest.json"), manifest, 0, -1});
        entries.append({QByteArrayLiteral("a/main.json"), animationJson(), 0, -1});
        for (int index = 0; index < 5; ++index) {
            entries.append({QByteArrayLiteral("images/big-") + QByteArray::number(index) +
                                QByteArrayLiteral(".png"),
                            QByteArrayLiteral("small"),
                            0,
                            chunkDeclared});
        }
        QVERIFY(writeFile(totalPath, storedZip(entries)));
    }
    QVERIFY(!parser.extractArchiveAndParse(totalPath,
                                           totalPath,
                                           nextExtractDir(),
                                           QString(),
                                           &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("expanded total")), qPrintable(error));

    // 压缩比预算：小归档声明出超过 100:1 的解压量拒绝。
    const QString ratioPath = QDir(temporaryDir.path()).filePath(QStringLiteral("ratio.lottie"));
    QVERIFY(writeFile(ratioPath, storedZip({
        {QByteArrayLiteral("manifest.json"), manifest, 0, -1},
        {QByteArrayLiteral("a/main.json"), animationJson(), 0, -1},
        {QByteArrayLiteral("images/ratio.png"), QByteArrayLiteral("s"), 0, 10 * 1024 * 1024}
    })));
    QVERIFY(!parser.extractArchiveAndParse(ratioPath,
                                           ratioPath,
                                           nextExtractDir(),
                                           QString(),
                                           &error).isValid());
    QVERIFY2(error.contains(QStringLiteral("compression ratio")), qPrintable(error));
}

void dot_lottie_parser_test::cancellationAbortsArchiveWorkAndLeavesCacheReusable()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString archivePath = QDir(temporaryDir.path()).filePath(QStringLiteral("cancel.lottie"));
    const QByteArray manifest = QByteArrayLiteral(R"json({
        "version":"2","initial":{"animation":"cancelled"},"animations":[{"id":"cancelled"}]
    })json");
    QVERIFY(writeFile(archivePath, storedZip({
        {QByteArrayLiteral("manifest.json"), manifest},
        {QByteArrayLiteral("a/cancelled.json"), animationJson()}
    })));

    UkuiQuick::DotLottieParser parser;
    QString error;
    const auto cancelled = [] { return true; };
    const QString extractDir = QDir(temporaryDir.path()).filePath(QStringLiteral("cache/cancelled"));

    const UkuiQuick::LottieAsset cancelledAsset = parser.extractArchiveAndParse(
        archivePath, archivePath, extractDir, QString(), &error, cancelled);
    QVERIFY(!cancelledAsset.isValid());
    QVERIFY2(error.contains(QStringLiteral("cancelled")), qPrintable(error));
    // 被取消的解压不会发布目录，也不会污染同一 source 的下一次加载。
    QVERIFY(!QFileInfo(extractDir).exists());

    const UkuiQuick::LottieAsset retryAsset = parser.extractArchiveAndParse(
        archivePath, archivePath, extractDir, QString(), &error);
    QVERIFY2(retryAsset.isValid(), qPrintable(error));
    QCOMPARE(retryAsset.activeAnimationId(), QStringLiteral("cancelled"));

    QVERIFY(!parser.parseExtractedDirectory(archivePath, extractDir, QString(), &error, cancelled).isValid());
    QVERIFY2(error.contains(QStringLiteral("cancelled")), qPrintable(error));
}

QTEST_GUILESS_MAIN(dot_lottie_parser_test)

#include "test-dot-lottie-parser.moc"
