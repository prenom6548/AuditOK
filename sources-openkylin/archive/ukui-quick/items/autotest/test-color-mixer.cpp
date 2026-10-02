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
#include <QtTest>

#include "color-mixer.h"

using namespace UkuiQuick;

namespace {

QColor mixNormalColor(const QColor &foreground, const QColor &background)
{
    const qreal foregroundAlpha = foreground.alphaF();
    const qreal backgroundAlpha = background.alphaF();
    const qreal backgroundRatio = 1 - foregroundAlpha;
    const qreal mixedAlpha = foregroundAlpha + backgroundAlpha * backgroundRatio;

    const qreal redComponent = (foreground.redF() * foregroundAlpha
                                + background.redF() * backgroundAlpha * backgroundRatio) / mixedAlpha;
    const qreal greenComponent = (foreground.greenF() * foregroundAlpha
                                  + background.greenF() * backgroundAlpha * backgroundRatio) / mixedAlpha;
    const qreal blueComponent = (foreground.blueF() * foregroundAlpha
                                 + background.blueF() * backgroundAlpha * backgroundRatio) / mixedAlpha;

    return QColor::fromRgbF(redComponent, greenComponent, blueComponent, mixedAlpha);
}

bool colorNear(const QColor &actual, const QColor &expected)
{
    return qAbs(actual.red() - expected.red()) <= 1
        && qAbs(actual.green() - expected.green()) <= 1
        && qAbs(actual.blue() - expected.blue()) <= 1
        && qAbs(actual.alpha() - expected.alpha()) <= 1;
}

}

class color_mixer_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultStateUsesNoMixerAndWhiteColor();
    void settingSameMixModeDoesNotEmitSignal();
    void noMixerUsesBackgroundColor();
    void normalModeMixesForegroundOverBackground();
};

void color_mixer_test::defaultStateUsesNoMixerAndWhiteColor()
{
    ColorMix mixer;

    QCOMPARE(mixer.mixMode(), ColorMix::NoMixer);
    QCOMPARE(mixer.color(), QColor(Qt::white));
    QCOMPARE(mixer.backColorAlpha(), 0.0);
    QCOMPARE(mixer.foreColorAlpha(), 0.0);
    QCOMPARE(mixer.backColorRole(), Theme::Base);
    QCOMPARE(mixer.foreColorRole(), Theme::Base);
    QCOMPARE(mixer.backColorGroup(), Theme::Active);
    QCOMPARE(mixer.foreColorGroup(), Theme::Active);
}

void color_mixer_test::settingSameMixModeDoesNotEmitSignal()
{
    ColorMix mixer;
    QSignalSpy mixModeSpy(&mixer, &ColorMix::mixModeChanged);

    mixer.setMixMode(ColorMix::NoMixer);

    QCOMPARE(mixModeSpy.count(), 0);
}

void color_mixer_test::noMixerUsesBackgroundColor()
{
    ColorMix mixer;
    QSignalSpy colorSpy(&mixer, &ColorMix::colorChanged);

    mixer.setBackColorAlpha(0.75);

    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(mixer.color(), Theme::instance()->color(Theme::Base, Theme::Active, 0.75));
}

void color_mixer_test::normalModeMixesForegroundOverBackground()
{
    ColorMix mixer;
    mixer.setForeColorRole(Theme::Highlight);
    mixer.setBackColorAlpha(0.8);
    mixer.setForeColorAlpha(0.25);

    QSignalSpy colorSpy(&mixer, &ColorMix::colorChanged);
    QSignalSpy mixModeSpy(&mixer, &ColorMix::mixModeChanged);

    mixer.setMixMode(ColorMix::Normal);

    const QColor foreground = Theme::instance()->color(Theme::Highlight, Theme::Active, 0.25);
    const QColor background = Theme::instance()->color(Theme::Base, Theme::Active, 0.8);
    const QColor expected = mixNormalColor(foreground, background);

    QCOMPARE(mixModeSpy.count(), 1);
    QCOMPARE(colorSpy.count(), 1);
    QVERIFY(colorNear(mixer.color(), expected));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false;kf.windowsystem=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    color_mixer_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-color-mixer.moc"
