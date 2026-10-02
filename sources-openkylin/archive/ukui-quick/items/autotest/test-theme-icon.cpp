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

#include <QApplication>
#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QtTest>

#include "theme-icon.h"
#include "ukui/ukui-theme-proxy.h"

using namespace UkuiQuick;

namespace {

QString writeIcon(QTemporaryDir &dir, const QString &name, const QColor &color)
{
    const QString path = dir.filePath(name + QStringLiteral(".png"));
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(color);
    image.save(path);
    return path;
}

QColor renderCenterColor(ThemeIcon &icon)
{
    QImage canvas(16, 16, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    QPainter painter(&canvas);
    icon.paint(&painter);
    painter.end();

    return canvas.pixelColor(canvas.width() / 2, canvas.height() / 2);
}

bool colorNear(const QColor &actual, const QColor &expected, int tolerance = 1)
{
    return qAbs(actual.red() - expected.red()) <= tolerance
        && qAbs(actual.green() - expected.green()) <= tolerance
        && qAbs(actual.blue() - expected.blue()) <= tolerance
        && qAbs(actual.alpha() - expected.alpha()) <= tolerance;
}

void initIconItem(ThemeIcon &icon)
{
    icon.setWidth(16);
    icon.setHeight(16);
}

}

class theme_icon_test : public QObject
{
    Q_OBJECT

private slots:
    void nullPixmapIsNotPureColor();
    void symbolicPixmapWithTransparentPixelsIsPureColor();
    void mixedPixmapIsNotPureColor();
    void radiusClampsNegativeValues();
    void keepAspectRatioDefaultsToTrueAndCanBeUpdated();
    void emptyFallbackDoesNotChangeSource();
    void fallbackPathIsStored();
    void fallbackLoadsWhenIconIsNull();
    void fallbackIsIgnoredWhenSourceAlreadyLoaded();
    void sourceBitmapCreatesRenderableIcon();
    void disabledModePaintsUsingDisabledThemeColor();
    void forceHighlightOverridesMixedPixmapCheck();
    void autoHighlightModeCanBeSetWithoutCrashing();
    void unsupportedSourceUsesBundledDefaultIcon();
    void sourceVariantsRenderExpectedColor_data();
    void sourceVariantsRenderExpectedColor();
};

void theme_icon_test::nullPixmapIsNotPureColor()
{
    QTest::ignoreMessage(QtWarningMsg, "pixmap is null!");
    QVERIFY(!ThemeIcon::isPixmapPureColor(QPixmap()));
}

void theme_icon_test::symbolicPixmapWithTransparentPixelsIsPureColor()
{
    QImage image(4, 4, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    image.setPixelColor(1, 1, ThemeIcon::getSymbolicColor());
    image.setPixelColor(2, 2, ThemeIcon::getSymbolicColor());

    QVERIFY(ThemeIcon::isPixmapPureColor(QPixmap::fromImage(image)));
}

void theme_icon_test::mixedPixmapIsNotPureColor()
{
    QImage image(4, 4, QImage::Format_ARGB32);
    image.fill(ThemeIcon::getSymbolicColor());
    image.setPixelColor(0, 0, QColor(Qt::red));

    QVERIFY(!ThemeIcon::isPixmapPureColor(QPixmap::fromImage(image)));
}

void theme_icon_test::radiusClampsNegativeValues()
{
    ThemeIcon icon;
    initIconItem(icon);

    QCOMPARE(icon.radius(), 0);

    icon.setRadius(-5);
    QCOMPARE(icon.radius(), 0);

    icon.setRadius(7);
    QCOMPARE(icon.radius(), 7);
}

void theme_icon_test::keepAspectRatioDefaultsToTrueAndCanBeUpdated()
{
    ThemeIcon icon;
    initIconItem(icon);

    QVERIFY(icon.getKeepAspectRatio());

    icon.setKeepAspectRatio(false);
    QVERIFY(!icon.getKeepAspectRatio());
}

void theme_icon_test::emptyFallbackDoesNotChangeSource()
{
    ThemeIcon icon;
    initIconItem(icon);

    QTest::ignoreMessage(QtWarningMsg, "ThemeIcon: fallback is empty!");
    icon.setFallBack(QString());

    QVERIFY(!icon.getSource().isValid());
}

void theme_icon_test::fallbackPathIsStored()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ThemeIcon icon;
    initIconItem(icon);
    const QString fallbackPath = writeIcon(dir, QStringLiteral("fallback"), QColor(Qt::blue));
    icon.setFallBack(fallbackPath);

    QCOMPARE(icon.getFallBack(), fallbackPath);
}

void theme_icon_test::fallbackLoadsWhenIconIsNull()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ThemeIcon icon;
    initIconItem(icon);
    const QString fallbackPath = writeIcon(dir, QStringLiteral("fallback"), QColor(Qt::blue));

    icon.setFallBack(fallbackPath);

    QCOMPARE(icon.getSource().toString(), fallbackPath);
    QCOMPARE(renderCenterColor(icon), QColor(Qt::blue));
}

void theme_icon_test::fallbackIsIgnoredWhenSourceAlreadyLoaded()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ThemeIcon icon;
    initIconItem(icon);
    const QString primaryPath = writeIcon(dir, QStringLiteral("primary"), QColor(Qt::red));
    const QString fallbackPath = writeIcon(dir, QStringLiteral("fallback"), QColor(Qt::blue));

    icon.setSource(primaryPath);
    icon.setFallBack(fallbackPath);

    QCOMPARE(icon.getSource().toString(), primaryPath);
    QCOMPARE(renderCenterColor(icon), QColor(Qt::red));
}

void theme_icon_test::sourceBitmapCreatesRenderableIcon()
{
    QBitmap bitmap(8, 8);
    bitmap.fill(Qt::color1);

    ThemeIcon icon;
    initIconItem(icon);
    icon.setSource(QVariant::fromValue(bitmap));
    icon.setMode(Icon::Highlight | Icon::ForceHighlight);

    QCOMPARE(icon.getSource().userType(), QMetaType::QBitmap);
    QVERIFY(colorNear(renderCenterColor(icon), Theme::instance()->color(Theme::HighlightedText)));
}

void theme_icon_test::disabledModePaintsUsingDisabledThemeColor()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ThemeIcon icon;
    initIconItem(icon);
    icon.setSource(writeIcon(dir, QStringLiteral("primary"), QColor(Qt::red)));
    icon.setMode(Icon::Disabled);

    const QColor painted = renderCenterColor(icon);
    QVERIFY(colorNear(painted, Theme::instance()->color(Theme::ButtonText, Theme::Disabled)));
}

void theme_icon_test::forceHighlightOverridesMixedPixmapCheck()
{
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(Qt::red);
    image.setPixelColor(0, 0, Qt::green);

    ThemeIcon icon;
    initIconItem(icon);
    icon.setSource(QVariant::fromValue(QPixmap::fromImage(image)));
    icon.setMode(Icon::Highlight | Icon::ForceHighlight);

    const QColor painted = renderCenterColor(icon);
    QVERIFY(colorNear(painted, Theme::instance()->color(Theme::HighlightedText)));
}

void theme_icon_test::autoHighlightModeCanBeSetWithoutCrashing()
{
    ThemeIcon icon;
    initIconItem(icon);

    icon.setMode(Icon::AutoHighlight);
    QVERIFY(icon.mode().testFlag(Icon::AutoHighlight));
    QCOMPARE(icon.mode().testFlag(Icon::Highlight), Theme::instance()->isDarkTheme());
}

void theme_icon_test::unsupportedSourceUsesBundledDefaultIcon()
{
    ThemeIcon icon;
    initIconItem(icon);
    icon.setSource(QVariant::fromValue(42));

    const QColor painted = renderCenterColor(icon);
    QVERIFY(colorNear(painted, QColor(QStringLiteral("#ff00ff"))));
}

void theme_icon_test::sourceVariantsRenderExpectedColor_data()
{
    QTest::addColumn<QVariant>("source");
    QTest::addColumn<QColor>("expectedColor");

    QImage greenImage(8, 8, QImage::Format_ARGB32);
    greenImage.fill(Qt::green);

    QPixmap redPixmap(8, 8);
    redPixmap.fill(Qt::red);

    QPixmap yellowPixmap(8, 8);
    yellowPixmap.fill(Qt::yellow);

    QTest::addRow("image") << QVariant::fromValue(greenImage) << QColor(Qt::green);
    QTest::addRow("pixmap") << QVariant::fromValue(redPixmap) << QColor(Qt::red);
    QTest::addRow("icon") << QVariant::fromValue(QIcon(yellowPixmap)) << QColor(Qt::yellow);
}

void theme_icon_test::sourceVariantsRenderExpectedColor()
{
    QFETCH(QVariant, source);
    QFETCH(QColor, expectedColor);

    ThemeIcon icon;
    initIconItem(icon);
    icon.setSource(source);

    QCOMPARE(renderCenterColor(icon), expectedColor);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QApplication app(argc, argv);
    theme_icon_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-theme-icon.moc"
