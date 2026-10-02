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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QBitmap>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSignalSpy>
#include <QtTest>

#include "icon.h"

using namespace UkuiQuick;

namespace {

QImage makeSolidImage(const QSize &size, const QColor &color)
{
    QImage image(size, QImage::Format_ARGB32);
    image.fill(color);
    return image;
}

}

class icon_test : public QObject
{
    Q_OBJECT

private slots:
    void radiusClampsNegativeValues();
    void radiusClipClearsCornersAndPreservesCenter();
    void radiusLargerThanHalfSizeClampsToHalfSize();
    void radiusEnablesRadiusPath();
    void forceRoundWinsOverRadius();
    void addUnifiedIconBoxWinsOverForceRoundAndRadius();
    void staticTextureOutsideLottieDimensionLimitRemainsRenderable();
    void sourceDefaultsToNullIcon();
    void sourceGetterPreservesAssignedImage();
    void sourceGetterPreservesExplicitClear();
    void sourceGetterPreservesAssignedTypes_data();
    void sourceGetterPreservesAssignedTypes();
    void sourceChangedTracksAssignedValue();
    void sourceTypeNotificationSkipsReenteredAssignment();
};

void icon_test::radiusClampsNegativeValues()
{
    Icon icon;
    QSignalSpy radiusSpy(&icon, &Icon::radiusChanged);

    QCOMPARE(icon.radius(), 0);

    icon.setRadius(-5);
    QCOMPARE(icon.radius(), 0);
    QCOMPARE(radiusSpy.count(), 0);

    icon.setRadius(6);
    QCOMPARE(icon.radius(), 6);
    QCOMPARE(radiusSpy.count(), 1);
}

void icon_test::radiusClipClearsCornersAndPreservesCenter()
{
    Icon icon;
    const QImage source = makeSolidImage(QSize(16, 16), QColor(Qt::red));

    const QImage clipped = icon.clipRoundedRect(source, 4);

    QCOMPARE(clipped.pixelColor(8, 8), QColor(Qt::red));
    QCOMPARE(clipped.pixelColor(0, 0).alpha(), 0);
    QCOMPARE(clipped.pixelColor(15, 0).alpha(), 0);
    QCOMPARE(clipped.pixelColor(0, 15).alpha(), 0);
    QCOMPARE(clipped.pixelColor(15, 15).alpha(), 0);
}

void icon_test::radiusLargerThanHalfSizeClampsToHalfSize()
{
    Icon icon;
    const QImage source = makeSolidImage(QSize(16, 16), QColor(Qt::red));

    const QImage clipped = icon.clipRoundedRect(source, 100);
    const QImage expected = icon.clipRoundedRect(source, 8);

    QCOMPARE(clipped, expected);
}

void icon_test::radiusEnablesRadiusPath()
{
    Icon icon;

    QCOMPARE(static_cast<int>(icon.renderPath()), static_cast<int>(Icon::RenderPath::Direct));

    icon.setRadius(6);

    QCOMPARE(static_cast<int>(icon.renderPath()), static_cast<int>(Icon::RenderPath::Radius));
}

void icon_test::forceRoundWinsOverRadius()
{
    Icon icon;

    icon.setRadius(6);
    icon.setForceRound(true);

    QCOMPARE(static_cast<int>(icon.renderPath()), static_cast<int>(Icon::RenderPath::ForceRound));
}

void icon_test::addUnifiedIconBoxWinsOverForceRoundAndRadius()
{
    Icon icon;

    icon.setRadius(6);
    icon.setForceRound(true);
    icon.setAddUnifiedIconBox(true);

    QCOMPARE(static_cast<int>(icon.renderPath()), static_cast<int>(Icon::RenderPath::UnifiedIconBox));
}

void icon_test::staticTextureOutsideLottieDimensionLimitRemainsRenderable()
{
    constexpr int widthOutsideLottieLimit = 8193;
    const QImage source = makeSolidImage(QSize(widthOutsideLottieLimit, 1), QColor(Qt::red));

    QQuickWindow window;
    Icon icon;
    icon.setSize(source.size());
    icon.setSource(QVariant::fromValue(source));
    icon.setParentItem(window.contentItem());
    window.setGeometry(0, 0, 32, 32);
    window.show();
    QTRY_VERIFY(window.isExposed());

    QSGTexture *texture = icon.createTexture();
    QVERIFY(texture);
    QCOMPARE(texture->textureSize(), source.size());
    delete texture;
}

void icon_test::sourceGetterPreservesAssignedImage()
{
    const QImage source = makeSolidImage(QSize(16, 16), QColor(Qt::red));
    Icon icon;

    icon.setSource(QVariant::fromValue(source));

    QCOMPARE(icon.source().userType(), QMetaType::QImage);
    QCOMPARE(icon.source().value<QImage>(), source);
    QCOMPARE(icon.sourceType(), Icon::StaticIconSource);
}

void icon_test::sourceDefaultsToNullIcon()
{
    Icon icon;

    QCOMPARE(icon.source().userType(), QMetaType::QIcon);
    QVERIFY(icon.source().value<QIcon>().isNull());
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
}

void icon_test::sourceGetterPreservesExplicitClear()
{
    const QImage image = makeSolidImage(QSize(16, 16), QColor(Qt::red));
    Icon icon;
    icon.setSource(QVariant::fromValue(image));
    QSignalSpy sourceSpy(&icon, &Icon::sourceChanged);
    QSignalSpy sourceTypeSpy(&icon, &Icon::sourceTypeChanged);

    icon.setSource(QStringLiteral(""));

    QCOMPARE(icon.source().userType(), QMetaType::QString);
    QCOMPARE(icon.source().toString(), QString());
    QCOMPARE(icon.sourceType(), Icon::UnknownSource);
    QCOMPARE(sourceSpy.count(), 1);
    QCOMPARE(sourceTypeSpy.count(), 1);
}

void icon_test::sourceGetterPreservesAssignedTypes_data()
{
    QTest::addColumn<QVariant>("source");

    const QImage image = makeSolidImage(QSize(4, 4), QColor(Qt::red));
    const QPixmap pixmap = QPixmap::fromImage(image);
    QBitmap bitmap(QSize(4, 4));
    bitmap.clear();
    const QIcon icon(pixmap);

    QTest::newRow("theme-name") << QVariant(QStringLiteral("application-system-symbolic"));
    QTest::newRow("image") << QVariant::fromValue(image);
    QTest::newRow("pixmap") << QVariant::fromValue(pixmap);
    QTest::newRow("bitmap") << QVariant::fromValue(bitmap);
    QTest::newRow("icon") << QVariant::fromValue(icon);
    QTest::newRow("unsupported") << QVariant(42);
}

void icon_test::sourceGetterPreservesAssignedTypes()
{
    QFETCH(QVariant, source);
    Icon icon;
    QSignalSpy sourceSpy(&icon, &Icon::sourceChanged);

    icon.setSource(source);
    icon.setSource(source);

    const QVariant actual = icon.source();
    QCOMPARE(sourceSpy.count(), 1);
    QCOMPARE(actual.userType(), source.userType());
    switch (source.userType()) {
    case QMetaType::QImage:
        QCOMPARE(actual.value<QImage>(), source.value<QImage>());
        break;
    case QMetaType::QPixmap:
        QCOMPARE(actual.value<QPixmap>().cacheKey(), source.value<QPixmap>().cacheKey());
        break;
    case QMetaType::QBitmap:
        QCOMPARE(actual.value<QBitmap>().cacheKey(), source.value<QBitmap>().cacheKey());
        break;
    case QMetaType::QIcon:
        QCOMPARE(actual.value<QIcon>().cacheKey(), source.value<QIcon>().cacheKey());
        break;
    default:
        QCOMPARE(actual, source);
        break;
    }
}

void icon_test::sourceChangedTracksAssignedValue()
{
    const QImage image = makeSolidImage(QSize(4, 4), QColor(Qt::red));
    Icon icon;
    QSignalSpy sourceSpy(&icon, &Icon::sourceChanged);

    icon.setSource(QVariant::fromValue(image));
    icon.setSource(QVariant::fromValue(image));
    icon.setSource(QString());
    icon.setSource(QString());
    icon.setSource(42);

    QCOMPARE(sourceSpy.count(), 3);
    QCOMPARE(icon.source(), QVariant(42));
}

void icon_test::sourceTypeNotificationSkipsReenteredAssignment()
{
    const QVariant image = QVariant::fromValue(
        makeSolidImage(QSize(4, 4), QColor(Qt::red)));
    Icon icon;
    bool reentered = false;
    connect(&icon, &Icon::sourceChanged, &icon, [&] {
        if (reentered) {
            return;
        }
        reentered = true;
        icon.setSource(QStringLiteral(""));
        icon.setSource(image);
    });

    QVector<Icon::SourceType> observedTypes;
    connect(&icon, &Icon::sourceTypeChanged, &icon, [&] {
        observedTypes.append(icon.sourceType());
    });

    icon.setSource(image);

    QCOMPARE(icon.source(), image);
    QCOMPARE(icon.sourceType(), Icon::StaticIconSource);
    QCOMPARE(observedTypes,
             QVector<Icon::SourceType>({Icon::UnknownSource,
                                        Icon::StaticIconSource}));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_QUICK_BACKEND", "software");
    qputenv("QSG_RENDER_LOOP", "basic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    icon_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-icon.moc"
