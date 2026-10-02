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

#include <QtTest>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QtQml>
#include <QScopedPointer>

#include "gradient-color.h"

using namespace UkuiQuick;

namespace {

bool colorNear(const QColor &actual, const QColor &expected)
{
    return qAbs(actual.red() - expected.red()) <= 1
        && qAbs(actual.green() - expected.green()) <= 1
        && qAbs(actual.blue() - expected.blue()) <= 1
        && qAbs(actual.alpha() - expected.alpha()) <= 1;
}

}

class gradient_color_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructorUsesTransparentPureColor();
    void pureConstructorStoresPureColor();
    void gradientConstructorStoresGradientValues();
    void settersUpdateValuesAndEmitSignals();
    void settersIgnoreUnchangedValues();
    void qmlCanSetColorTypeExplicitly();
    void inequalityComparesColorTypeAndValues();
    void setAlphaMultipliesAndClampsAlpha();
    void mixBackgroundColorHandlesTransparentFastPaths();
    void mixBackgroundColorBlendsForegroundOverBackground();
    void gradientPointsFollowAngle();
};

void gradient_color_test::defaultConstructorUsesTransparentPureColor()
{
    GradientColor color;

    QCOMPARE(color.gradientDeg(), 0);
    QCOMPARE(color.gradientStart(), QColor(0, 0, 0, 0));
    QCOMPARE(color.gradientEnd(), QColor(0, 0, 0, 0));
    QCOMPARE(color.gradientBackground(), QColor(0, 0, 0, 0));
    QCOMPARE(color.pureColor(), QColor(0, 0, 0, 0));
    QCOMPARE(color.colorType(), DtColorType::Pure);
}

void gradient_color_test::pureConstructorStoresPureColor()
{
    GradientColor color(QColor(10, 20, 30, 40));

    QCOMPARE(color.pureColor(), QColor(10, 20, 30, 40));
    QCOMPARE(color.colorType(), DtColorType::Pure);
}

void gradient_color_test::gradientConstructorStoresGradientValues()
{
    GradientColor color(90, QColor(10, 20, 30, 40), QColor(50, 60, 70, 80), QColor(90, 100, 110, 120));

    QCOMPARE(color.gradientDeg(), 90);
    QCOMPARE(color.gradientStart(), QColor(10, 20, 30, 40));
    QCOMPARE(color.gradientEnd(), QColor(50, 60, 70, 80));
    QCOMPARE(color.gradientBackground(), QColor(90, 100, 110, 120));
    QCOMPARE(color.colorType(), DtColorType::Gradient);
}

void gradient_color_test::settersUpdateValuesAndEmitSignals()
{
    GradientColor color;
    QSignalSpy startSpy(&color, &GradientColor::gradientStartChanged);
    QSignalSpy endSpy(&color, &GradientColor::gradientEndChanged);
    QSignalSpy backgroundSpy(&color, &GradientColor::gradientBackgroundChanged);
    QSignalSpy degSpy(&color, &GradientColor::gradientDegChanged);
    QSignalSpy pureSpy(&color, &GradientColor::pureColorChanged);
    QSignalSpy typeSpy(&color, &GradientColor::colorTypeChanged);

    color.setGradientStart(QColor(1, 2, 3, 4));
    color.setGradientEnd(QColor(5, 6, 7, 8));
    color.setGradientBackground(QColor(9, 10, 11, 12));
    color.setGradientDeg(45);
    color.setPureColor(QColor(13, 14, 15, 16));
    color.setColorType(DtColorType::Gradient);

    QCOMPARE(color.gradientStart(), QColor(1, 2, 3, 4));
    QCOMPARE(color.gradientEnd(), QColor(5, 6, 7, 8));
    QCOMPARE(color.gradientBackground(), QColor(9, 10, 11, 12));
    QCOMPARE(color.gradientDeg(), 45);
    QCOMPARE(color.pureColor(), QColor(13, 14, 15, 16));
    QCOMPARE(color.colorType(), DtColorType::Gradient);
    QCOMPARE(startSpy.count(), 1);
    QCOMPARE(endSpy.count(), 1);
    QCOMPARE(backgroundSpy.count(), 1);
    QCOMPARE(degSpy.count(), 1);
    QCOMPARE(pureSpy.count(), 1);
    QCOMPARE(typeSpy.count(), 1);
}

void gradient_color_test::settersIgnoreUnchangedValues()
{
    GradientColor color;
    color.setGradientStart(QColor(1, 2, 3, 4));
    color.setGradientEnd(QColor(5, 6, 7, 8));
    color.setGradientBackground(QColor(9, 10, 11, 12));
    color.setGradientDeg(45);
    color.setPureColor(QColor(13, 14, 15, 16));
    color.setColorType(DtColorType::Gradient);

    QSignalSpy startSpy(&color, &GradientColor::gradientStartChanged);
    QSignalSpy endSpy(&color, &GradientColor::gradientEndChanged);
    QSignalSpy backgroundSpy(&color, &GradientColor::gradientBackgroundChanged);
    QSignalSpy degSpy(&color, &GradientColor::gradientDegChanged);
    QSignalSpy pureSpy(&color, &GradientColor::pureColorChanged);
    QSignalSpy typeSpy(&color, &GradientColor::colorTypeChanged);

    color.setGradientStart(QColor(1, 2, 3, 4));
    color.setGradientEnd(QColor(5, 6, 7, 8));
    color.setGradientBackground(QColor(9, 10, 11, 12));
    color.setGradientDeg(45);
    color.setPureColor(QColor(13, 14, 15, 16));
    color.setColorType(DtColorType::Gradient);

    QCOMPARE(startSpy.count(), 0);
    QCOMPARE(endSpy.count(), 0);
    QCOMPARE(backgroundSpy.count(), 0);
    QCOMPARE(degSpy.count(), 0);
    QCOMPARE(pureSpy.count(), 0);
    QCOMPARE(typeSpy.count(), 0);
}

void gradient_color_test::qmlCanSetColorTypeExplicitly()
{
    qmlRegisterType<GradientColor>(
        "org.ukui.quick.platform.test", 1, 0, "GradientColor");
    qmlRegisterUncreatableMetaObject(
        DtColorType::staticMetaObject,
        "org.ukui.quick.platform.test", 1, 0, "DtColorType",
        "DtColorType is an enumeration container");

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQml 2.15
        import org.ukui.quick.platform.test 1.0

        GradientColor {
            colorType: DtColorType.Gradient
            gradientDeg: 45
        }
    )", QUrl());

    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    QCOMPARE(object->property("colorType").toInt(), int(DtColorType::Gradient));
    QCOMPARE(object->property("gradientDeg").toInt(), 45);
}

void gradient_color_test::inequalityComparesColorTypeAndValues()
{
    GradientColor pureColor(QColor(1, 2, 3, 4));
    GradientColor samePureColor(QColor(1, 2, 3, 4));
    GradientColor differentPureColor(QColor(5, 6, 7, 8));
    GradientColor gradientColor(45, QColor(1, 2, 3, 4), QColor(5, 6, 7, 8), QColor(9, 10, 11, 12));
    GradientColor sameGradientColor(45, QColor(1, 2, 3, 4), QColor(5, 6, 7, 8), QColor(9, 10, 11, 12));

    QVERIFY(!(pureColor != &samePureColor));
    QVERIFY(pureColor != &differentPureColor);
    QVERIFY(pureColor != &gradientColor);
    QVERIFY(!(gradientColor != &sameGradientColor));
    QVERIFY(gradientColor != static_cast<GradientColor *>(nullptr));
    QVERIFY(!(pureColor != QColor(1, 2, 3, 4)));
    QVERIFY(pureColor != QColor(5, 6, 7, 8));
    QVERIFY(gradientColor != QColor(1, 2, 3, 4));
}

void gradient_color_test::setAlphaMultipliesAndClampsAlpha()
{
    GradientColor color;

    QCOMPARE(color.setAlphaF(QColor(10, 20, 30, 128), 0.5).alpha(), 64);
    QCOMPARE(color.setAlphaF(QColor(10, 20, 30, 200), 2.0).alpha(), 255);
}

void gradient_color_test::mixBackgroundColorHandlesTransparentFastPaths()
{
    GradientColor color;
    const QColor transparentForeground(10, 20, 30, 0);
    const QColor transparentBackground(40, 50, 60, 0);
    const QColor foreground(70, 80, 90, 100);
    const QColor background(110, 120, 130, 140);

    QCOMPARE(color.mixBackGroundColor(transparentForeground, background, 0.5), background);
    QCOMPARE(color.mixBackGroundColor(foreground, transparentBackground, 0.5), foreground);
}

void gradient_color_test::mixBackgroundColorBlendsForegroundOverBackground()
{
    GradientColor color;
    const QColor foreground(255, 0, 0, 128);
    const QColor background(0, 0, 255, 255);
    const QColor mixed = color.mixBackGroundColor(foreground, background, 1.0);

    QVERIFY(colorNear(mixed, QColor(128, 0, 127, 255)));
}

void gradient_color_test::gradientPointsFollowAngle()
{
    GradientColor color;

    QCOMPARE(color.getGradientStartPoint(0, 100, 80), QPoint(0, 40));
    QCOMPARE(color.getGradientEndPoint(0, 100, 80), QPoint(100, 40));
    QCOMPARE(color.getGradientStartPoint(90, 100, 80), QPoint(50, 0));
    QCOMPARE(color.getGradientEndPoint(90, 100, 80), QPoint(50, 80));
}

QTEST_GUILESS_MAIN(gradient_color_test)

#include "test-gradient-color.moc"
