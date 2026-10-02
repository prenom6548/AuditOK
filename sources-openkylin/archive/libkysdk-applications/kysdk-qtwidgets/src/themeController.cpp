/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "themeController.h"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStringList>
#include <QVariant>
#include <QtMath>

#define ColorDifference 10

static WidgetThemeFlag g_widgetThemeFlag = DefaultTheme;
static ThemeFlag g_themeFlag = LightTheme;
static IconFlag g_iconFlag = ClassicStyle;
static int g_fontSize = 11;
static QGSettings *g_gsetting = nullptr;

static const QStringList getAppList()
{
    QStringList list{
        //    "kylin-nm",
        "ukui-menu",
        "ukui-panel",
        "ukui-sidebar",
        "ukui-volume-control-applet-qt",
        "panelukui-panel",
        "ukui-power-manager-tray",
        //    "ukui-bluetooth",
        "sogouimebs",
        "kylin-device-daemoon",
        "ukui-notifications",
        "ukui-flash-disk",
        "mktip",
        "kylin-video"};
    return list;
}

static inline QString getUppercaseLetters(QString str)
{
    if (str.isEmpty()) {
        return str;
    }

    QString firstStr = str.left(1).toUpper();
    QString remainingStr = str.mid(1);

    return firstStr + remainingStr;
}

static inline QString getDTColorGroup(QPalette::ColorGroup colorGroup)
{
    //    QPalette::ColorGroup colorGroup;
    QString str;
    if (colorGroup == QPalette::Active)
        str = "active";
    else if (colorGroup == QPalette::Inactive)
        str = "inactive";
    else if (colorGroup == QPalette::Disabled)
        str = "disable";
    else if (colorGroup == QPalette::NColorGroups)
        str = "ncolorgroups";
    else if (colorGroup == QPalette::Current)
        str = "current";
    else if (colorGroup == QPalette::All)
        str = "all";
    else if (colorGroup == QPalette::Normal)
        str = "normal";
    return str;
}

static inline QPalette::ColorRole getColorRole(QString str)
{
    QPalette::ColorRole colorRole;
    if (str.contains("windowtext"))
        colorRole = QPalette::WindowText;
    else if (str.contains("button"))
        colorRole = QPalette::Button;
    else if (str.contains("light"))
        colorRole = QPalette::Light;
    else if (str.contains("midlight"))
        colorRole = QPalette::Midlight;
    else if (str.contains("dark"))
        colorRole = QPalette::Dark;
    else if (str.contains("mid"))
        colorRole = QPalette::Mid;
    else if (str.contains("text"))
        colorRole = QPalette::Text;
    else if (str.contains("brighttext"))
        colorRole = QPalette::BrightText;
    else if (str.contains("buttontext"))
        colorRole = QPalette::ButtonText;
    else if (str.contains("base"))
        colorRole = QPalette::Base;
    else if (str.contains("window"))
        colorRole = QPalette::Window;
    else if (str.contains("shadow"))
        colorRole = QPalette::Shadow;
    else if (str.contains("highlight"))
        colorRole = QPalette::Highlight;
    else if (str.contains("highlightedtext"))
        colorRole = QPalette::HighlightedText;
    else if (str.contains("link"))
        colorRole = QPalette::Link;
    else if (str.contains("linkvisited"))
        colorRole = QPalette::LinkVisited;
    else if (str.contains("alternatebase"))
        colorRole = QPalette::AlternateBase;
    else if (str.contains("norole"))
        colorRole = QPalette::NoRole;
    else if (str.contains("tooltipbase"))
        colorRole = QPalette::ToolTipBase;
    else if (str.contains("tooltiptext"))
        colorRole = QPalette::ToolTipText;
    else if (str.contains("placeholdertext"))
        colorRole = QPalette::PlaceholderText;
    else if (str.contains("ncolorroles"))
        colorRole = QPalette::NColorRoles;
    else if (str.contains("foreground"))
        colorRole = QPalette::Foreground;
    else if (str.contains("background"))
        colorRole = QPalette::Background;

    return colorRole;
}

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}
static inline QColor adjustColor(const QColor c, int hRange = 0, int sRange = 0, int lRange = 0, int aRange = 0)
{
    QColor cc;
    int s, l, a;

    if ((c.hslSaturationF() + sRange * 1.0 / 100.0) > 1) {
        //        qWarning() << "adjustColor s out of range1";
        s = 255;
    } else if ((c.hslSaturationF() + sRange * 1.0 / 100.0) < 0) {
        //        qWarning() << "adjustColor s out of range0";
        s = 0;
    } else
        s = (c.hslSaturationF() + sRange * 1.0 / 100.0) * 255.0;

    if ((c.lightnessF() + lRange * 1.0 / 100.0) > 0.97) { // 根据设计稿寻光色彩识别   l应<=0.97
        qWarning() << "adjustColor l out of range1" << c.lightnessF() << lRange << (c.lightnessF() + lRange * 1.0 / 100.0);
        l = 255.0 * 0.97;
    } else if ((c.lightnessF() + lRange * 1.0 / 100.0) < 0.15) { // 根据设计稿上l最小取的均值
        qWarning() << "adjustColor l out of range0";
        l = 0.15 * 255.0;
    } else
        l = (c.lightnessF() + lRange * 1.0 / 100.0) * 255.0;

    if ((c.alphaF() + aRange * 1.0 / 100.0) > 1) {
        qWarning() << "adjustColor a out of range1";
        a = 255;
    } else if ((c.alphaF() + aRange * 1.0 / 100.0) < 0) {
        qWarning() << "adjustColor a out of range0";
        a = 0;
    } else
        a = (c.alphaF() + aRange * 1.0 / 100.0) * 255.0;
    cc.setHsl(c.hslHue() + hRange, s, l, a);
    return cc;
}

QPixmap ThemeController::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QColor gray(128, 128, 128);
    QColor standard(31, 32, 34);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red() - gray.red()) < 20 && qAbs(color.green() - gray.green()) < 20 && qAbs(color.blue() - gray.blue()) < 20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if (qAbs(color.red() - standard.red()) < 20 && qAbs(color.green() - standard.green()) < 20 && qAbs(color.blue() - standard.blue()) < 20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else {
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap ThemeController::drawColoredPixmap(const QPixmap &source, const QColor &sampleColor)
{
    QImage img = source.toImage();
    for(int x = 0 ; x < img.width() ;x++)
    {
        for( int y = 0; y < img.height();y++)
        {
            auto color = img.pixelColor(x,y);
            if(color.alpha() > 0)
            {
                color.setRed(sampleColor.red());
                color.setGreen(sampleColor.green());
                color.setBlue(sampleColor.blue());
                img.setPixelColor(x,y,color);
            }
        }
    }
    return QPixmap::fromImage(img);
}

QColor ThemeController::getCurrentIconColor()
{
    QPixmap pixmap = QIcon::fromTheme("open-menu-symbolic").pixmap(16, 16);
    QImage img = pixmap.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                return color;
            }
        }
    }
}

QColor ThemeController::mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(), c2.redF(), bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(), c2.blueF(), bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

WidgetThemeFlag ThemeController::widgetTheme()
{
    return g_widgetThemeFlag;
}

ThemeFlag ThemeController::themeMode()
{
    return g_themeFlag;
}

IconFlag ThemeController::iconTheme()
{
    return g_iconFlag;
}

int ThemeController::systemFontSize()
{
    if (!g_gsetting)
        return 11;
    if (g_gsetting->keys().contains("systemFontSize")) {
        g_fontSize = (int)g_gsetting->get("systemFontSize").toDouble();
    }
    return g_fontSize;
}

QPixmap ThemeController::drawFashionBackground(const QRect &rect, int sub_width, int sub_height, int radius, int flag)
{

    //    int radius = 6;
    //    int sub_width = 32;
    //    int sub_height = 36;
    QPixmap framePixmap(rect.size());

    framePixmap.fill(Qt::transparent);
    QRect drawRect;
    drawRect = rect.adjusted(0, 0, 1, 1);

    auto baseColor = qApp->palette().color(QPalette::Active, QPalette::Button);

    QColor startColor = mixColor(baseColor, getCustomColorFromDT("kwhite"), 0.5);
    QColor endColor = mixColor(baseColor, getCustomColorFromDT("kblack"), 0.1);

    QLinearGradient linearGradient;
    QPainterPath path;
    if (flag) // right
    {
        path.moveTo(drawRect.bottomRight() - QPoint(0, radius + sub_height));
        path.lineTo(drawRect.bottomRight() - QPoint(0, radius));
        path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(radius, 0));
        path.lineTo(drawRect.bottomRight() - QPoint(radius + sub_width, 0));
        path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(0, radius + sub_height));

        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        linearGradient.setStart(drawRect.right(), drawRect.bottom() - (radius + sub_height));
        linearGradient.setFinalStop(drawRect.right(), drawRect.bottom());
    } else // left
    {
        path.moveTo(drawRect.bottomLeft() - QPoint(0, radius + sub_height));
        path.lineTo(drawRect.bottomLeft() - QPoint(0, radius));
        path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() + QPoint(radius, 0));
        path.lineTo(drawRect.bottomLeft() + QPoint(radius + sub_width, 0));
        path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() - QPoint(0, radius + sub_height));

        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        linearGradient.setStart(drawRect.left(), drawRect.bottom() - (radius + sub_height));
        linearGradient.setFinalStop(drawRect.left(), drawRect.bottom());
    }
    QPainter painter(&framePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(linearGradient);
    painter.drawPath(path);
    return framePixmap;
}

bool ThemeController::isPixmapPureColor(const QPixmap &pixmap)
{
    QColor symbolic_color = getCustomColorFromDT("kgray-17");

    if (pixmap.isNull()) {
        qWarning("pixmap is null!");
        return false;
    }
    QImage image = pixmap.toImage();

    QVector<QColor> vector;
    int total_red = 0;
    int total_green = 0;
    int total_blue = 0;
    bool pure = true;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alphaF() > 0.3) {
                QColor color = image.pixelColor(x, y);
                vector << color;
                total_red += color.red();
                total_green += color.green();
                total_blue += color.blue();
                int dr = qAbs(color.red() - symbolic_color.red());
                int dg = qAbs(color.green() - symbolic_color.green());
                int db = qAbs(color.blue() - symbolic_color.blue());
                if (dr > ColorDifference || dg > ColorDifference || db > ColorDifference)
                    pure = false;
            }
        }
    }

    if (pure)
        return true;

    qreal squareRoot_red = 0;
    qreal squareRoot_green = 0;
    qreal squareRoot_blue = 0;
    qreal average_red = total_red / vector.count();
    qreal average_green = total_green / vector.count();
    qreal average_blue = total_blue / vector.count();
    for (QColor color : vector) {
        squareRoot_red += (color.red() - average_red) * (color.red() - average_red);
        squareRoot_green += (color.green() - average_green) * (color.green() - average_green);
        squareRoot_blue += (color.blue() - average_blue) * (color.blue() - average_blue);
    }

    qreal arithmeticSquareRoot_red = qSqrt(squareRoot_red / vector.count());
    qreal arithmeticSquareRoot_green = qSqrt(squareRoot_green / vector.count());
    qreal arithmeticSquareRoot_blue = qSqrt(squareRoot_blue / vector.count());
    return arithmeticSquareRoot_red < 2.0 && arithmeticSquareRoot_green < 2.0 && arithmeticSquareRoot_blue < 2.0;
}

QIcon ThemeController::overlayIcons(const QIcon &icon1, const QIcon &icon2, int iconWidth, int iconHeight, int offsetX, int offsetY)
{
    if (!icon1.isNull() && !icon2.isNull()) {
        QSize size = icon1.actualSize(QSize(iconWidth, iconHeight));

        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        icon1.paint(&painter, QRect(0, 0, size.width(), size.height()), Qt::AlignCenter);
        if (iconWidth > offsetX && iconHeight > offsetY)
            icon2.paint(&painter, QRect(offsetX, offsetY, pixmap.width() - offsetX, size.height() - offsetY), Qt::AlignCenter);
        else
            icon2.paint(&painter, QRect(pixmap.width() - 16, pixmap.height() - 16, 16, 16), Qt::AlignCenter);

        QIcon resultIcon(pixmap);

        return resultIcon;
    } else {
        return QIcon();
    }
}

QPixmap ThemeController::overlayPixmaps(const QPixmap &pixmap1, const QPixmap &pixmap2, int offsetX, int offsetY)
{
    if (!pixmap1.isNull() && !pixmap2.isNull()) {
        QPixmap resultPixmap(pixmap1.size());
        resultPixmap.fill(Qt::transparent);

        QPainter painter(&resultPixmap);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        painter.drawPixmap(QRect(0, 0, pixmap1.width(), pixmap1.height()), pixmap1);

        if (pixmap1.width() > offsetX && pixmap1.height() > offsetY)
            painter.drawPixmap(QRect(offsetX, offsetY, pixmap1.width() - offsetX, pixmap1.height() - offsetY), pixmap2);
        else
            painter.drawPixmap(QRect(pixmap1.width() - 16, pixmap1.height() - 16, 16, 16), pixmap2);

        painter.end();

        return resultPixmap;
    } else {
        return QPixmap();
    }
}

QColor ThemeController::lanhuHSLToQtHsl(int h, int s, int l, int a)
{
    QColor c;
    c.setHsl(h, s * 255.0 / 100.0, l * 255.0 / 100.0, a);
    return c;
}

QColor ThemeController::adjustH(const QColor c, int range)
{
    QColor cc;
    if ((c.hslHue() + range) >= 0)
        cc.setHsl(c.hslHue() + range, c.hslSaturation(), c.lightness(), c.alpha());
    else
        cc.setHsl(c.hslHue(), c.hslSaturation(), c.lightness(), c.alpha());

    return cc;
}

QColor ThemeController::adjustS(const QColor c, int range)
{
    QColor cc;
    int s;

    if ((c.hslSaturationF() + range * 1.0 / 100.0) > 1) {
        //        qWarning() << "adjustS s out of range1";
        s = 255;
    } else if ((c.hslSaturationF() + range * 1.0 / 100.0) < 0) {
        //        qWarning() << "adjustS s out of range0";
        s = 0;
    } else
        s = (c.hslSaturationF() + range * 1.0 / 100.0) * 255.0;
    cc.setHsl(c.hslHue(), s, c.lightness(), c.alpha());
    return cc;
}

QColor ThemeController::adjustL(const QColor c, int range)
{
    QColor cc;
    int l;

    if ((c.lightnessF() + range * 1.0 / 100.0) > 0.97) {
        //        qWarning() << "adjustL l out of range1";
        l = 255.0 * 0.97;
    } else if ((c.lightnessF() + range * 1.0 / 100.0) < 0.15) {
        //        qWarning() << "adjustL l out of range0" << c.lightnessF() << range << (c.lightnessF() + range * 1.0 / 100.0);
        l = 255.0 * 0.15;
    } else
        l = (c.lightnessF() + range * 1.0 / 100.0) * 255.0;

    cc.setHsl(c.hslHue(), c.hslSaturation(), l, c.alpha());
    return cc;
}

QColor ThemeController::adjustA(const QColor c, int range)
{
    QColor cc;
    int a;
    if ((c.alphaF() + range * 1.0 / 100.0) > 1) {
        //        qWarning() << "adjustA a out of range1";
        a = 255;
    } else if ((c.alphaF() + range * 1.0 / 100.0) < 0) {
        //        qWarning() << "adjustA a out of range0";
        a = 0;
    } else
        a = (c.alphaF() + range * 1.0 / 100.0) * 255.0;

    cc.setHsl(c.hslHue(), c.hslSaturation(), c.lightness(), a);
    return cc;
}

QColor ThemeController::highlightClick(bool isDark, QPalette p)
{
    if (isDark) {
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -25, -34); // hightlight->210,72%,71% 210,47%,37%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 3, 10, 31); // hightlight->209,87%,57% 212,97%,88%
}

QColor ThemeController::highlightHover(bool isDark, QPalette p)
{
    if (isDark) {
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -26, -40); // 210,46%,31%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 4, 2, 36); // 213,89%,93%
}

QColor ThemeController::buttonDarkClick(bool isDark, QPalette p)
{
    if (isDark) {
        return adjustColor(p.color(QPalette::Disabled, QPalette::ButtonText), 0, 1, 5);
    }
    return QColor();
}

bool ThemeController::systemLang()
{
    QString systemLang = QLocale::system().name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ")
        return true;
    else
        return false;
}

QColor ThemeController::getPaletteColorFromDT(QString dtString, QPalette::ColorGroup group)
{
    QColor color;
    char *m_char = dtString.toLocal8Bit().data();
    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QBrush>()) {
        QBrush brush = qApp->property(m_char).value<QBrush>();
        color = brush.color();
    }

    if (!color.isValid())
    {
        QPalette::ColorGroup colorGroup;
        colorGroup = qApp->palette().currentColorGroup();
        QPalette::ColorRole colorRole = getColorRole(dtString);
        color = qApp->palette().color(colorGroup, colorRole);
    }
    return color;
}

QColor ThemeController::getCustomColorFromDT(QString dtString)
{
    QColor color;
    char *m_char = dtString.toLocal8Bit().data();
    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QBrush>()) {
        QBrush brush = qApp->property(m_char).value<QBrush>();
        color = brush.color();
    }
    return color;
}

QString ThemeController::getFontStylesFromDT(QString dtString)
{
    char *m_char = dtString.toLocal8Bit().data();
    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QString>()) {
        return qApp->property(m_char).value<QString>();
    }
    return QString();
}

int ThemeController::getRadiusFromDT(QString dtString)
{
    char *m_char = dtString.toLocal8Bit().data();
    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<int>()) {
        return qApp->property(m_char).value<int>();
    }
    return -1;
}

int ThemeController::getLineOrMarginFromDT(QString dtString)
{
    char *m_char = dtString.toLocal8Bit().data();
    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QColor>()) {
        return qApp->property(m_char).value<int>();
    }
    return -1;
}

void ThemeController::getGradientFromDT(QString key, QColor &startColor, QColor &endColor)
{
    // int type;
    // QVariantMap option;

    // QDBusConnection bus = QDBusConnection::sessionBus();
    // QDBusMessage message = QDBusMessage::createMethodCall(
    //     "com.kylin.themeConfig",
    //     "/com/kylin/ThemeConfig",
    //     "com.kylin.ThemeConfig.DTParameters",
    //     "GetDTThemeConfig");

    // QVariantList arguments;
    // arguments.append(key);
    // message.setArguments(arguments);

    // QDBusReply<QVariantMap> reply = bus.call(message);

    // if (!reply.isValid()) {
    //     qDebug() << "complex type failed!";
    // } else {
    //     option = reply.value();

    //     if (option.contains("type")) {
    //         type = option.value("type").toUInt();
    //     }

    //     if (type == 3 && option.contains(QStringLiteral("start_color")) && option.contains(QStringLiteral("end_color"))) {

    //         QByteArray startvalue = option.value(QStringLiteral("start_color")).toByteArray();
    //         uint8_t c[4];
    //         c[0] = startvalue[0];
    //         c[1] = startvalue[1];
    //         c[2] = startvalue[2];
    //         c[3] = startvalue[3];
    //         startColor.setRed(c[0]);
    //         startColor.setGreen(c[1]);
    //         startColor.setBlue(c[2]);
    //         startColor.setAlpha(c[3]);

    //         QByteArray endvalue = option.value(QStringLiteral("end_color")).toByteArray();
    //         c[0] = endvalue[0];
    //         c[1] = endvalue[1];
    //         c[2] = endvalue[2];
    //         c[3] = endvalue[3];
    //         endColor.setRed(c[0]);
    //         endColor.setGreen(c[1]);
    //         endColor.setBlue(c[2]);
    //         endColor.setAlpha(c[3]);
    //     }
    // }
    char *m_char = key.toLocal8Bit().data();

    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QBrush>()) {
        QBrush brush = qApp->property(m_char).value<QBrush>();
        if(brush.gradient() && brush.gradient()->stops().length() > 1)
        {
            startColor = brush.gradient()->stops().at(0).second;
            endColor = brush.gradient()->stops().at(1).second;
        }
        else
        {
            startColor = brush.color();
            endColor = brush.color();
        }
    }
}

ThemeController::ThemeController()
    : m_gsetting(nullptr)
{
    if (QGSettings::isSchemaInstalled(FITTHEMEWINDOW)) {
        m_gsetting = new QGSettings(FITTHEMEWINDOW);
        g_gsetting = m_gsetting;
        initThemeStyle();
    }
}

ThemeController::~ThemeController()
{
}

void ThemeController::initThemeStyle()
{
    if (!m_gsetting)
        return;
    if (m_gsetting->keys().contains("styleName")) {
        QString styleName = m_gsetting->get("styleName").toString();
        if(qApp && qApp->property("useLightTheme").isValid() && qApp->property("useLightTheme").toBool())
            g_themeFlag = LightTheme;
        else if(qApp && qApp->property("useDarkTheme").isValid() && qApp->property("useDarkTheme").toBool())
            g_themeFlag = DarkTheme;
        else if (styleName == "ukui-dark" || styleName == "ukui-black" || (styleName == "ukui-default" && getAppList().contains(QApplication::applicationName())))
            g_themeFlag = DarkTheme;
        else
            g_themeFlag = LightTheme;
    }

    // 初始化图标主题
    if (m_gsetting->keys().contains("iconThemeName")) {
        QString iconThemeName = m_gsetting->get("iconThemeName").toString();
        if (iconThemeName == "ukui-icon-theme-default")
            g_iconFlag = DefaultStyle;
        else {
            g_iconFlag = ClassicStyle;
        }
    }
    if (m_gsetting->keys().contains("widgetThemeName")) {
        // 初始化样式主题
        QString widgetThemeName = m_gsetting->get("widgetThemeName").toString();
        if (widgetThemeName == "default")
            g_widgetThemeFlag = DefaultTheme;
        else if (widgetThemeName == "classical")
            g_widgetThemeFlag = ClassicTheme;
        else if(widgetThemeName == "fashion")
            g_widgetThemeFlag = FashionTheme;
        else if(widgetThemeName == "dawnlight")
        g_widgetThemeFlag = DawnLightTheme;
    }
}
