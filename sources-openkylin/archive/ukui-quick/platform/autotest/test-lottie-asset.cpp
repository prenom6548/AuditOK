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
#include <QTemporaryDir>
#include <QtTest>

#define private public
#include "../ukui/lottie/lottie-asset.h"
#undef private

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

QString canonicalPath(const QString &path)
{
    return QFileInfo(path).canonicalFilePath();
}

}

class lottie_asset_test : public QObject
{
    Q_OBJECT

private slots:
    void storesMetadataAndDetachesCopies();
    void carriesFrameMetadataAndDefaultRevision();
    void selectsOnlyAllowedThemes();
    void returnsOnlyTrustedImageCandidates();
    void sharesAndReleasesBoundedThemeFailureState();
};

void lottie_asset_test::storesMetadataAndDetachesCopies()
{
    UkuiQuick::LottieAsset asset;
    QVERIFY(!asset.isValid());

    asset.setAnimationJsonPath(QStringLiteral("animation.json"));
    asset.setResourceDir(QStringLiteral("resources"));
    asset.setExtractDir(QStringLiteral("extract"));
    asset.setTheme(QStringLiteral("dark"));
    asset.setManifestInitialThemeId(QStringLiteral("dark"));
    asset.setAnimationInitialThemeId(QStringLiteral("light"));
    asset.setAvailableThemes({QStringLiteral("light"), QStringLiteral("dark"),
                              QStringLiteral("light"), QString()});
    asset.setActiveAnimationId(QStringLiteral("spinner"));
    asset.setActiveAnimationThemes({QStringLiteral("light")});
    asset.setActiveAnimationRestrictsThemes(true);
    asset.setImageResourcePaths({QStringLiteral("images/one.png"),
                                 QStringLiteral("images/one.png")});
    asset.addImageResourcePath(QStringLiteral("images/two.png"));
    asset.setResourceSearchDirs({QStringLiteral("resources"),
                                 QStringLiteral("fallback"),
                                 QStringLiteral("fallback")});
    asset.addResourceSearchDir(QStringLiteral("extra"));
    asset.setThemeSlotJson(QStringLiteral("dark"), QByteArrayLiteral("{\"slot\":1}"));
    asset.setThemeSlotJson(QString(), QByteArrayLiteral("ignored"));

    QVERIFY(asset.isValid());
    QCOMPARE(asset.animationJsonPath(), QStringLiteral("animation.json"));
    QCOMPARE(asset.resourceDir(), QStringLiteral("resources"));
    QCOMPARE(asset.extractDir(), QStringLiteral("extract"));
    QCOMPARE(asset.theme(), QStringLiteral("dark"));
    QCOMPARE(asset.manifestInitialThemeId(), QStringLiteral("dark"));
    QCOMPARE(asset.animationInitialThemeId(), QStringLiteral("light"));
    QCOMPARE(asset.availableThemes(),
             QStringList({QStringLiteral("light")}));
    QCOMPARE(asset.activeAnimationId(), QStringLiteral("spinner"));
    QCOMPARE(asset.activeAnimationThemes(), QStringList({QStringLiteral("light")}));
    QVERIFY(asset.activeAnimationRestrictsThemes());
    QCOMPARE(asset.imageResourcePaths(),
             QStringList({QStringLiteral("images/one.png"), QStringLiteral("images/two.png")}));
    QCOMPARE(asset.imageResourceDeclarationCount(), 3);
    QCOMPARE(asset.imageResourceReferenceCount(QStringLiteral("images/one.png")), 2);
    QCOMPARE(asset.imageResourceReferenceCount(QStringLiteral("images/two.png")), 1);
    QCOMPARE(asset.resourceSearchDirs(),
             QStringList({QStringLiteral("resources"),
                          QStringLiteral("fallback"),
                          QStringLiteral("extra")}));
    QCOMPARE(asset.themeSlotJson(QStringLiteral("dark")), QByteArrayLiteral("{\"slot\":1}"));
    QVERIFY(asset.themeSlotJson(QString()).isEmpty());

    UkuiQuick::LottieAsset copy = asset;
    copy.setTheme(QStringLiteral("light"));
    copy.addImageResourcePath(QStringLiteral("images/copy.png"));
    copy.setThemeSlotJson(QStringLiteral("light"), QByteArrayLiteral("{\"slot\":2}"));

    QCOMPARE(asset.theme(), QStringLiteral("dark"));
    QCOMPARE(copy.theme(), QStringLiteral("light"));
    QCOMPARE(asset.imageResourcePaths(),
             QStringList({QStringLiteral("images/one.png"), QStringLiteral("images/two.png")}));
    QCOMPARE(copy.imageResourcePaths(),
             QStringList({QStringLiteral("images/one.png"),
                          QStringLiteral("images/two.png"),
                          QStringLiteral("images/copy.png")}));
    QCOMPARE(asset.imageResourceDeclarationCount(), 3);
    QCOMPARE(copy.imageResourceDeclarationCount(), 4);
    QVERIFY(asset.themeSlotJson(QStringLiteral("light")).isEmpty());
    QCOMPARE(copy.themeSlotJson(QStringLiteral("light")), QByteArrayLiteral("{\"slot\":2}"));
}

void lottie_asset_test::carriesFrameMetadataAndDefaultRevision()
{
    UkuiQuick::LottieAsset asset;
    QVERIFY(!asset.revision().isValid());
    QCOMPARE(asset.revision(), UkuiQuick::LottieAssetRevision());
    QCOMPARE(asset.frameCount(), 0);
    QCOMPARE(asset.frameRate(), 0.0);

    asset.setFrameCount(60);
    asset.setFrameRate(24.5);
    QCOMPARE(asset.frameCount(), 60);
    QCOMPARE(asset.frameRate(), 24.5);

    UkuiQuick::LottieAsset copy = asset;
    copy.setFrameCount(1);
    QCOMPARE(asset.frameCount(), 60);
    QCOMPARE(copy.frameCount(), 1);
    QCOMPARE(copy.revision(), asset.revision());
}

void lottie_asset_test::selectsOnlyAllowedThemes()
{
    UkuiQuick::LottieAsset asset;
    asset.setAvailableThemes({QStringLiteral("light"),
                              QStringLiteral("dark"),
                              QStringLiteral("contrast")});
    asset.setManifestInitialThemeId(QStringLiteral("dark"));
    asset.setAnimationInitialThemeId(QStringLiteral("light"));
    asset.setActiveAnimationThemes({QStringLiteral("light"), QStringLiteral("contrast")});
    asset.setActiveAnimationRestrictsThemes(true);

    QCOMPARE(asset.availableThemes(),
             QStringList({QStringLiteral("light"), QStringLiteral("contrast")}));
    QVERIFY(asset.canUseTheme(QString()));
    QVERIFY(asset.canUseTheme(QStringLiteral("light")));
    QVERIFY(!asset.canUseTheme(QStringLiteral("dark")));
    QVERIFY(!asset.canUseTheme(QStringLiteral("missing")));
    QCOMPARE(asset.preferredTheme(QStringLiteral("contrast")), QStringLiteral("contrast"));
    QCOMPARE(asset.preferredTheme(QStringLiteral("dark")), QStringLiteral("light"));
    QCOMPARE(asset.preferredTheme(QStringLiteral("missing")), QStringLiteral("light"));

    asset.setActiveAnimationRestrictsThemes(false);
    QCOMPARE(asset.availableThemes(),
             QStringList({QStringLiteral("light"), QStringLiteral("dark"),
                          QStringLiteral("contrast")}));
    QVERIFY(asset.canUseTheme(QStringLiteral("dark")));
    QCOMPARE(asset.preferredTheme(QStringLiteral("missing")), QStringLiteral("dark"));

    asset.setManifestInitialThemeId(QStringLiteral("missing"));
    QCOMPARE(asset.preferredTheme(), QStringLiteral("light"));
}

void lottie_asset_test::returnsOnlyTrustedImageCandidates()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString primaryDir = QDir(temporaryDir.path()).filePath(QStringLiteral("primary"));
    const QString fallbackDir = QDir(temporaryDir.path()).filePath(QStringLiteral("fallback"));
    const QString outsidePath = QDir(temporaryDir.path()).filePath(QStringLiteral("outside.png"));
    const QString nestedPath = QDir(primaryDir).filePath(QStringLiteral("nested/icon.png"));
    const QString imagesPath = QDir(primaryDir).filePath(QStringLiteral("images/icon.png"));
    const QString shortPath = QDir(primaryDir).filePath(QStringLiteral("i/icon.png"));
    const QString fallbackPath = QDir(fallbackDir).filePath(QStringLiteral("icon.png"));
    const QString explicitPath = QDir(primaryDir).filePath(QStringLiteral("explicit/only.png"));
    QVERIFY(writeFile(nestedPath, QByteArrayLiteral("nested")));
    QVERIFY(writeFile(imagesPath, QByteArrayLiteral("images")));
    QVERIFY(writeFile(shortPath, QByteArrayLiteral("short")));
    QVERIFY(writeFile(fallbackPath, QByteArrayLiteral("fallback")));
    QVERIFY(writeFile(explicitPath, QByteArrayLiteral("explicit")));
    QVERIFY(writeFile(outsidePath, QByteArrayLiteral("outside")));

    UkuiQuick::LottieAsset asset;
    asset.setResourceDir(primaryDir);
    asset.setResourceSearchDirs({fallbackDir, primaryDir});
    asset.setImageResourcePaths({explicitPath, explicitPath, outsidePath});

    QCOMPARE(asset.imagePathCandidates(QStringLiteral("nested/icon.png")),
             QStringList({canonicalPath(nestedPath),
                          canonicalPath(imagesPath),
                          canonicalPath(shortPath),
                          canonicalPath(fallbackPath)}));
    QCOMPARE(asset.imagePathCandidates(QStringLiteral("explicit/only.png")),
             QStringList({canonicalPath(explicitPath)}));
    QVERIFY(asset.imagePathCandidates(QStringLiteral("outside.png")).isEmpty());

    const QStringList unsafePaths{
        QStringLiteral("../outside.png"),
        QStringLiteral("/tmp/outside.png"),
        QStringLiteral("C:/outside.png"),
        QStringLiteral("nested\\icon.png"),
        QStringLiteral(":/outside.png")
    };
    for (const QString &path : unsafePaths) {
        QVERIFY2(asset.imagePathCandidates(path).isEmpty(), qPrintable(path));
    }
}

void lottie_asset_test::sharesAndReleasesBoundedThemeFailureState()
{
    using namespace UkuiQuick;

    QStringList themes;
    for (int index = 0; index < 300; ++index) {
        themes.append(QStringLiteral("theme-%1").arg(index));
    }

    {
        LottieAsset asset;
        asset.setAvailableThemes(themes);
        asset.setRevision(LottieAssetRevision::fromIdentityDigest(
            QByteArrayLiteral("revision-one")));
        QCOMPARE(asset.themeFailureStateCapacity(), 256);
        QCOMPARE(asset.selectableThemeOrdinal(QStringLiteral("theme-255")), 255);
        QCOMPARE(asset.selectableThemeOrdinal(QStringLiteral("theme-299")), 299);
        QVERIFY(!asset.hasDeterministicThemeFailure(255));

        LottieAsset otherHost = asset;
        otherHost.setTheme(QStringLiteral("theme-255"));
        otherHost.rememberDeterministicThemeFailure(255);
        QVERIFY(asset.hasDeterministicThemeFailure(255));
        QVERIFY(otherHost.hasDeterministicThemeFailure(255));

        LottieAsset independentlyReloaded;
        independentlyReloaded.setAvailableThemes(themes);
        independentlyReloaded.setRevision(LottieAssetRevision::fromIdentityDigest(
            QByteArrayLiteral("revision-one")));
        QVERIFY(independentlyReloaded.hasDeterministicThemeFailure(255));

        {
            LottieAsset restrictedAnimation;
            restrictedAnimation.setAvailableThemes(themes);
            restrictedAnimation.setActiveAnimationThemes({
                QStringLiteral("theme-299"), QStringLiteral("theme-298")});
            restrictedAnimation.setActiveAnimationRestrictsThemes(true);
            restrictedAnimation.setRevision(LottieAssetRevision::fromIdentityDigest(
                QByteArrayLiteral("restricted-revision")));
            QCOMPARE(restrictedAnimation.availableThemes(),
                     QStringList({QStringLiteral("theme-298"),
                                  QStringLiteral("theme-299")}));
            QCOMPARE(restrictedAnimation.themeFailureStateCapacity(), 2);
            QCOMPARE(restrictedAnimation.selectableThemeOrdinal(
                         QStringLiteral("theme-298")), 0);
            QCOMPARE(restrictedAnimation.selectableThemeOrdinal(
                         QStringLiteral("theme-299")), 1);
            QCOMPARE(restrictedAnimation.selectableThemeOrdinal(
                         QStringLiteral("theme-0")), -1);
            restrictedAnimation.rememberDeterministicThemeFailure(1);
            QVERIFY(restrictedAnimation.hasDeterministicThemeFailure(1));
        }

        LottieAsset newRevision = asset;
        newRevision.setRevision(LottieAssetRevision::fromIdentityDigest(
            QByteArrayLiteral("revision-two")));
        QVERIFY(!newRevision.hasDeterministicThemeFailure(255));
    }

    // The registry holds only weak references. Recreating the same revisions
    // after every owner is gone must start with fresh failure state.
    LottieAsset reloadedAfterRelease;
    reloadedAfterRelease.setAvailableThemes(themes);
    reloadedAfterRelease.setRevision(LottieAssetRevision::fromIdentityDigest(
        QByteArrayLiteral("revision-one")));
    QVERIFY(!reloadedAfterRelease.hasDeterministicThemeFailure(255));

    LottieAsset restrictedAfterRelease;
    restrictedAfterRelease.setAvailableThemes(themes);
    restrictedAfterRelease.setActiveAnimationThemes({
        QStringLiteral("theme-299"), QStringLiteral("theme-298")});
    restrictedAfterRelease.setActiveAnimationRestrictsThemes(true);
    restrictedAfterRelease.setRevision(LottieAssetRevision::fromIdentityDigest(
        QByteArrayLiteral("restricted-revision")));
    QVERIFY(!restrictedAfterRelease.hasDeterministicThemeFailure(1));
}

QTEST_GUILESS_MAIN(lottie_asset_test)

#include "test-lottie-asset.moc"
