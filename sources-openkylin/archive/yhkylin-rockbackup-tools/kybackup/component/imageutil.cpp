/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "imageutil.h"

#include <QPainter>
#include <QApplication>
#include <QIcon>

//const QPixmap ImageUtil::loadSvg(const QString &path, const QString& color, int size)
//{
//    int origSize = size;
//    const auto ratio = qApp->devicePixelRatio();
//    if ( 2 == ratio) {
//        size += origSize;
//    } else if (3 == ratio) {
//        size += origSize;
//    }
//    QPixmap pixmap(size, size);
//    QSvgRenderer renderer(path);
//    pixmap.fill(Qt::transparent);

//    QPainter painter;
//    painter.begin(&pixmap);
//    renderer.render(&painter);
//    painter.end();

//    pixmap.setDevicePixelRatio(ratio);
//    return drawSymbolicColoredPixmap(pixmap, color);
//}

const QPixmap ImageUtil::loadTheme(const QString &theme, const QString &defaultTheme, const QString& color, int size)
{
//    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
//    size = origSize * ratio;

    QIcon icon = QIcon::fromTheme(theme, QIcon(defaultTheme));
    QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(size, size)));
    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

const QPixmap ImageUtil::loadTheme(const QString &theme, const QString &defaultTheme, const QString& color, QSize size)
{
//    int origWidth = size.width();
//    int origHeight = size.height();
    const auto ratio = qApp->devicePixelRatio();
//    size.setWidth(origWidth * ratio);
//    size.setHeight(origHeight * ratio);

    QIcon icon = QIcon::fromTheme(theme, QIcon(defaultTheme));
    QPixmap pixmap = icon.pixmap(icon.actualSize(size));
    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

const QPixmap ImageUtil::loadPixmap(QIcon &icon, const QString &color, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    size = origSize * ratio;

    QPixmap pixmap = icon.pixmap(QSize(size, size));

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap ImageUtil::drawSymbolicColoredPixmap(const QPixmap &source, const QString& cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ( "white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if( "black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray"== cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}
