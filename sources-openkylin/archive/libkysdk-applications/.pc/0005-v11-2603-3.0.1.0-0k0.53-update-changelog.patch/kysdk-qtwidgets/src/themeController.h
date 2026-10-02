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

#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include <QGSettings/QGSettings>
#include <QPushButton>

static const QByteArray FITTHEMEWINDOW = "org.ukui.style";

/**
 * @brief 监听主题控制
 * @since 1.0.0
 */

enum ThemeFlag {
    LightTheme,
    DarkTheme
};

enum IconFlag {
    DefaultStyle,
    ClassicStyle
};

enum WidgetThemeFlag {
    DefaultTheme, // 寻光
    ClassicTheme, // 启典
    FashionTheme // 和印
};

class ThemeController
{

public:
    static QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    static QPixmap drawColoredPixmap(const QPixmap &source, const QColor &sampleColor);
    static QColor getCurrentIconColor();
    static QColor mixColor(const QColor &c1, const QColor &c2, qreal bias = 0.5);
    static WidgetThemeFlag widgetTheme();
    static ThemeFlag themeMode();
    static IconFlag iconTheme();
    static int systemFontSize();
    static QPixmap drawFashionBackground(const QRect &rect, int sub_width, int sub_height, int radius, int flag); // flag ? rightBottom : leftBottom;
    static bool isPixmapPureColor(const QPixmap &pixmap);
    static QIcon overlayIcons(const QIcon &icon1, const QIcon &icon2, int iconWidth, int iconHeight, int offsetX = 0, int offsetY = 0);
    static QPixmap overlayPixmaps(const QPixmap &pixmap1, const QPixmap &pixmap2, int offsetX = 0, int offsetY = 0);
    static QColor lanhuHSLToQtHsl(int h, int s, int l, int a = 255);
    static QColor adjustH(const QColor c, int range);
    static QColor adjustS(const QColor c, int range);
    static QColor adjustL(const QColor c, int range);
    static QColor adjustA(const QColor c, int range);
    static QColor highlightClick(bool isDark, QPalette p);
    static QColor highlightHover(bool isDark, QPalette p);
    static QColor buttonDarkClick(bool isDark, QPalette p);
    //    static void normalButtonColor(bool isDark, QPalette p, QPen &defaultPen, QBrush &defaultBrush,  QPen &hoverPen, QBrush &hoverBrush,
    //                            QPen &clickPen, QBrush &clickBrush,  QPen &disablePen, QBrush &disableBrush);
    static bool systemLang();

    static QColor getPaletteColorFromDT(QString dtString, QPalette::ColorGroup group = QPalette::Normal);
    static QColor getCustomColorFromDT(QString dtString);
    static QString getFontStylesFromDT(QString dtString);
    static int getRadiusFromDT(QString dtString);
    static int getLineOrMarginFromDT(QString dtString);

    static void getGradientFromDT(QString key, QColor &startColor, QColor &endColor);

    ThemeController();
    ~ThemeController();

    virtual void changeTheme()
    {
    }
    virtual void changeIconStyle()
    {
    }
    void initThemeStyle();

    QGSettings *m_gsetting;
    // unused
    ThemeFlag m_themeFlag;
    IconFlag m_iconFlag;
};

#endif // THEMECONTROLLER_H
