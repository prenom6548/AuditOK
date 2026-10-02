/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <QtTest>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include "icon-heuristics.h"

using namespace UkuiQuick::IconHeuristics;

namespace {

constexpr qreal kCircleFitThreshold = 0.85;

QImage effectiveArea(const QImage &image)
{
    const OpaqueBounds bounds = scanOpaqueBounds(image, 4);
    return bounds.valid ? image.copy(bounds.rect) : QImage();
}

QImage createSolidCircle(int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QRectF(1, 1, size - 2, size - 2));
    painter.end();
    return effectiveArea(image);
}

QImage createRing(int size, int spanAngle = 360)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal penWidth = qMax<qreal>(2.0, size / 6.0);
    painter.setPen(QPen(Qt::white, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    const qreal inset = penWidth / 2.0 + 1.0;
    const QRectF bounds(inset, inset, size - 2 * inset, size - 2 * inset);
    if (spanAngle == 360) {
        painter.drawEllipse(bounds);
    } else {
        painter.drawArc(bounds, 30 * 16, spanAngle * 16);
    }
    painter.end();
    return effectiveArea(image);
}

QImage createDiscreteCircle(int size, bool hollow)
{
    QImage image(size, size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    const qreal radius = size / 2.0;
    for (int y = 0; y < size; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < size; ++x) {
            const qreal nx = (x + 0.5 - radius) / radius;
            const qreal ny = (y + 0.5 - radius) / radius;
            const qreal radialSquared = nx * nx + ny * ny;
            if (radialSquared <= 1.0 && (!hollow || radialSquared >= 0.45)) {
                line[x] = qRgba(255, 255, 255, 255);
            }
        }
    }
    return image;
}

QImage createBackdrop(int size, qreal radiusRatio)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 180, 120));
    painter.drawRoundedRect(QRectF(1, 1, size - 2, size - 2),
                            size * radiusRatio, size * radiusRatio);
    painter.end();
    return effectiveArea(image);
}

QImage createSquircle(int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    const qreal center = size / 2.0;
    const qreal radius = size / 2.0 - 1.0;
    QPainterPath path;
    for (int i = 0; i <= 128; ++i) {
        const qreal angle = M_PI * 2 * i / 128;
        const qreal cosine = qCos(angle);
        const qreal sine = qSin(angle);
        const QPointF point(center + radius * qSqrt(qAbs(cosine)) * (cosine < 0 ? -1 : 1),
                            center + radius * qSqrt(qAbs(sine)) * (sine < 0 ? -1 : 1));
        if (i == 0) {
            path.moveTo(point);
        } else {
            path.lineTo(point);
        }
    }
    path.closeSubpath();

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 180, 120));
    painter.drawPath(path);
    painter.end();
    return effectiveArea(image);
}

} // namespace

class TestIconHeuristics : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void coverage_counts_partial_alpha();
    void completeImagePixel_accepts_high_alpha_without_full_255();
    void completeImagePixel_repairs_corners_with_logo_near_corner();
    void completeImagePixel_uses_backdrop_color_instead_of_logo_color();
    void backdropDetection_uses_complete_span();
    void circleFit_recognizes_outer_circle_data();
    void circleFit_recognizes_outer_circle();
    void filledCircleFit_keeps_firstParty_guard_fillSensitive_data();
    void filledCircleFit_keeps_firstParty_guard_fillSensitive();
    void circleFit_rejects_backdrop_shapes_data();
    void circleFit_rejects_backdrop_shapes();
};

void TestIconHeuristics::coverage_counts_partial_alpha()
{
    QImage image(2, 2, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    image.setPixelColor(0, 0, QColor(255, 255, 255, 128));

    const qreal coverage = alphaCoverage(image, QRect(0, 0, 2, 2));

    QVERIFY2(coverage > 0.1, "Partial alpha pixels should contribute to coverage.");
    QVERIFY2(coverage < 0.2, "Coverage should remain fractional for a single half-opaque pixel.");
}

void TestIconHeuristics::completeImagePixel_accepts_high_alpha_without_full_255()
{
    QImage image(6, 6, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    const QColor backdropColor(20, 120, 220, 240);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            image.setPixelColor(x, y, backdropColor);
        }
    }
    image.setPixelColor(0, 0, QColor(0, 0, 0, 0));

    repairBackdropCorners(image);

    QVERIFY2(image.pixelColor(0, 0).alpha() > 0,
             "Corner completion should work for high-alpha backdrops even when no pixel reaches 255 alpha.");
}

void TestIconHeuristics::completeImagePixel_repairs_corners_with_logo_near_corner()
{
    QImage image(12, 12, QImage::Format_ARGB32);
    image.fill(QColor(20, 120, 220, 240));

    const QColor transparentColor(0, 0, 0, 0);
    image.setPixelColor(0, 0, transparentColor);
    image.setPixelColor(11, 0, transparentColor);
    image.setPixelColor(0, 11, transparentColor);
    image.setPixelColor(11, 11, transparentColor);

    const QColor logoColor(245, 245, 245, 240);
    const QList<QPoint> logoPoints {
        QPoint(9, 0), QPoint(10, 0),
        QPoint(9, 1), QPoint(10, 1), QPoint(11, 1),
        QPoint(9, 2), QPoint(10, 2), QPoint(11, 2)
    };
    for (const QPoint &point : logoPoints) {
        image.setPixelColor(point, logoColor);
    }

    repairBackdropCorners(image);

    QVERIFY2(image.pixelColor(11, 0).alpha() > 0,
             "A logo near the corner should not block repairing the transparent backdrop corner.");
    QCOMPARE(image.pixelColor(10, 1), logoColor);
    QCOMPARE(image.pixelColor(10, 2), logoColor);
}

void TestIconHeuristics::completeImagePixel_uses_backdrop_color_instead_of_logo_color()
{
    QImage image(12, 12, QImage::Format_ARGB32);
    const QColor backdropColor(20, 120, 220, 240);
    image.fill(backdropColor);

    const QColor transparentColor(0, 0, 0, 0);
    image.setPixelColor(11, 0, transparentColor);

    const QColor logoColor(245, 245, 245, 240);
    image.setPixelColor(11, 1, logoColor);
    image.setPixelColor(10, 1, logoColor);
    image.setPixelColor(10, 0, backdropColor);

    repairBackdropCorners(image);

    QCOMPARE(image.pixelColor(11, 0).red(), backdropColor.red());
    QCOMPARE(image.pixelColor(11, 0).green(), backdropColor.green());
    QCOMPARE(image.pixelColor(11, 0).blue(), backdropColor.blue());
}

void TestIconHeuristics::backdropDetection_uses_complete_span()
{
    const QImage circle = createSolidCircle(24);
    const QImage roundedBackdrop = createBackdrop(24, 0.2);

    QVERIFY2(!hasOpaqueRectBackdrop(cropNonShadowArea(circle)),
             "A circular silhouette must not validate each short local edge against itself.");
    QVERIFY2(hasOpaqueRectBackdrop(cropNonShadowArea(roundedBackdrop)),
             "Using the complete span must continue to accept a conventional rounded backdrop.");
}

void TestIconHeuristics::circleFit_recognizes_outer_circle_data()
{
    QTest::addColumn<int>("size");

    QTest::newRow("16px") << 16;
    QTest::newRow("24px") << 24;
    QTest::newRow("32px") << 32;
    QTest::newRow("64px") << 64;
}

void TestIconHeuristics::circleFit_recognizes_outer_circle()
{
    QFETCH(int, size);

    const qreal solidFit = circleFit(createSolidCircle(size));
    const qreal ringFit = circleFit(createRing(size));
    const qreal openRingFit = circleFit(createRing(size, 300));

    QVERIFY2(solidFit > kCircleFitThreshold,
             qPrintable(QStringLiteral("solidFit=%1").arg(solidFit)));
    QVERIFY2(ringFit > kCircleFitThreshold,
             qPrintable(QStringLiteral("ringFit=%1").arg(ringFit)));
    QVERIFY2(qAbs(solidFit - ringFit) < 0.03,
             qPrintable(QStringLiteral("solidFit=%1 ringFit=%2").arg(solidFit).arg(ringFit)));
    QVERIFY2(openRingFit > kCircleFitThreshold,
             qPrintable(QStringLiteral("openRingFit=%1").arg(openRingFit)));
}

void TestIconHeuristics::filledCircleFit_keeps_firstParty_guard_fillSensitive_data()
{
    circleFit_recognizes_outer_circle_data();
}

void TestIconHeuristics::filledCircleFit_keeps_firstParty_guard_fillSensitive()
{
    QFETCH(int, size);

    const QImage solidCircle = createDiscreteCircle(size, false);
    const QImage hollowCircle = createDiscreteCircle(size, true);

    QVERIFY2(circleFit(solidCircle) > kCircleFitThreshold,
             "The outer-contour metric must recognize a solid circle.");
    QVERIFY2(circleFit(hollowCircle) > kCircleFitThreshold,
             "The outer-contour metric must also recognize a hollow circle.");
    QVERIFY2(filledCircleFit(solidCircle) > 0.97,
             "The first-party compatibility guard must accept a filled circle.");
    QVERIFY2(filledCircleFit(hollowCircle) < 0.97,
             "The first-party compatibility guard must retain its original fill-sensitive semantics.");
}

void TestIconHeuristics::circleFit_rejects_backdrop_shapes_data()
{
    QTest::addColumn<int>("size");

    QTest::newRow("16px") << 16;
    QTest::newRow("24px") << 24;
    QTest::newRow("32px") << 32;
    QTest::newRow("64px") << 64;
}

void TestIconHeuristics::circleFit_rejects_backdrop_shapes()
{
    QFETCH(int, size);

    const qreal squareFit = circleFit(createBackdrop(size, 0.0));
    const qreal roundedFit = circleFit(createBackdrop(size, 0.2));
    const qreal largeRoundedFit = circleFit(createBackdrop(size, 0.3));
    const qreal squircleFit = circleFit(createSquircle(size));

    QVERIFY2(squareFit < kCircleFitThreshold,
             qPrintable(QStringLiteral("squareFit=%1").arg(squareFit)));
    QVERIFY2(roundedFit < kCircleFitThreshold,
             qPrintable(QStringLiteral("roundedFit=%1").arg(roundedFit)));
    QVERIFY2(largeRoundedFit < kCircleFitThreshold,
             qPrintable(QStringLiteral("largeRoundedFit=%1").arg(largeRoundedFit)));
    QVERIFY2(squircleFit < kCircleFitThreshold,
             qPrintable(QStringLiteral("squircleFit=%1").arg(squircleFit)));
}

QTEST_MAIN(TestIconHeuristics)
#include "test-icon-heuristics.moc"
