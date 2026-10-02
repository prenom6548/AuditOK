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
#include <QImage>
#include <QPainter>
#include <QtTest>

#include "scenegraph/painted-rectangle-item.h"

namespace {

QImage renderItem(PaintedRectangleItem &item, const QSize &size)
{
    item.setWidth(size.width());
    item.setHeight(size.height());

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    item.paint(&painter);
    painter.end();

    return image;
}

bool colorNear(const QColor &actual, const QColor &expected, int tolerance = 3)
{
    return qAbs(actual.red() - expected.red()) <= tolerance
        && qAbs(actual.green() - expected.green()) <= tolerance
        && qAbs(actual.blue() - expected.blue()) <= tolerance
        && qAbs(actual.alpha() - expected.alpha()) <= tolerance;
}

} // namespace

class painted_rectangle_item_test : public QObject
{
    Q_OBJECT

private slots:
    void pureColorFillPaintsCenterPixel();
    void ninetyDegreeGradientStaysVertical();
    void borderWidthIsFlooredAndBorderColorReachesTheEdge();
    void oversizedRadiusLeavesCornersTransparent();
};

void painted_rectangle_item_test::pureColorFillPaintsCenterPixel()
{
    PaintedRectangleItem item;
    item.setPureColor(true);
    item.setColor(QColor(QStringLiteral("#33aa55")));

    const QImage image = renderItem(item, QSize(40, 24));
    QVERIFY(colorNear(image.pixelColor(20, 12), QColor(QStringLiteral("#33aa55"))));
}

void painted_rectangle_item_test::ninetyDegreeGradientStaysVertical()
{
    PaintedRectangleItem item;
    item.setPureColor(false);
    item.setStartColor(Qt::red);
    item.setEndColor(Qt::blue);
    item.setAngle(90.0);

    const QImage image = renderItem(item, QSize(60, 40));
    const QColor top = image.pixelColor(30, 4);
    const QColor bottom = image.pixelColor(30, 35);
    const QColor leftMid = image.pixelColor(6, 20);
    const QColor rightMid = image.pixelColor(53, 20);

    QVERIFY(top.red() > bottom.red());
    QVERIFY(bottom.blue() > top.blue());
    QVERIFY2(colorNear(leftMid, rightMid, 6), "90-degree gradient should not drift horizontally");
}

void painted_rectangle_item_test::borderWidthIsFlooredAndBorderColorReachesTheEdge()
{
    PaintedRectangleItem item;
    item.setPureColor(true);
    item.setColor(Qt::green);
    item.setBorderColor(Qt::black);
    item.setBorderWidth(2.8);

    const QImage image = renderItem(item, QSize(40, 24));
    QVERIFY(colorNear(image.pixelColor(1, 12), QColor(Qt::black)));
    QVERIFY(colorNear(image.pixelColor(20, 12), QColor(Qt::green)));
}

void painted_rectangle_item_test::oversizedRadiusLeavesCornersTransparent()
{
    PaintedRectangleItem item;
    item.setPureColor(true);
    item.setColor(Qt::yellow);
    item.setRadius(QVector4D(100.0f, 100.0f, 100.0f, 100.0f));

    const QImage image = renderItem(item, QSize(30, 18));
    QVERIFY(image.pixelColor(0, 0).alpha() < 10);
    QVERIFY(image.pixelColor(15, 9).alpha() > 200);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    painted_rectangle_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-painted-rectangle-item.moc"
