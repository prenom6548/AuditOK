/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "commonfunc.h"

#include <QCursor>
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QIcon>
#include <QImageReader>
#include <QFile>

QT_BEGIN_NAMESPACE
extern void
qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

#define BLUR_RADIUS 300
// 设置一下鼠标的居于主屏中央
void setCursorCenter()
{
    auto setCursorPos = [=](QPoint p) { QCursor::setPos(p); };

    if (!qApp->primaryScreen()) {
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [=] {
            static bool first = true;
            if (first) {
                setCursorPos(qApp->primaryScreen()->geometry().center());
                first = false;
            }
        });
    } else {
        setCursorPos(qApp->primaryScreen()->geometry().center());
    }
}

/*修改图片缩放机制，图片长宽不一致时，先取图片中央的部分*/
QPixmap scaledPixmap(QPixmap src)
{
    QPixmap rectPixmap;
    if (src.width() > src.height()) {
        QPixmap iconPixmap = src.copy((src.width() - src.height()) / 2, 0, src.height(), src.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(src.height());
    } else {
        QPixmap iconPixmap = src.copy(0, (src.height() - src.width()) / 2, src.width(), src.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(src.width());
    }
    return rectPixmap;
}

QPixmap PixmapToRound(const QPixmap &src, int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(radius * 2, radius * 2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius * 2, radius * 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius * 2, radius * 2, pixmapa);
    return pixmap;
}

QPixmap PixmapToRound(const QPixmap &src, int leftTop, int rightTop, int leftBottom, int rightBottom)
{
    if (src.isNull()) {
        return QPixmap();
    }
    QPixmap roundedPixmap(src.size());
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.moveTo(leftTop, 0);
    path.arcTo(QRect(0, 0, 2*leftTop, 2*leftTop), 90, 90);
    path.lineTo(0, src.height() - leftBottom);
    path.arcTo(QRect(0, src.height() - 2*leftBottom, 2*leftBottom, 2*leftBottom), 180, 90);
    path.lineTo(src.width() - rightBottom, src.height());
    path.arcTo(QRect(src.width() - 2*rightBottom, src.height() - 2*rightBottom, 2*rightBottom, 2*rightBottom), 270, 90);
    path.lineTo(src.width(), rightTop);
    path.arcTo(QRect(src.width() - 2*rightTop, 0, 2*rightTop, 2*rightTop), 0, 90);
    path.closeSubpath();

    painter.setClipPath(path);
    painter.drawPixmap(0, 0, src);
    painter.end();

    return roundedPixmap;
}

const QPixmap loadSvg(const QString &path, const QString color, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if (2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor) {
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

QPixmap getLoadingIcon(int size)
{
    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(size, size);
    return drawSymbolicColoredPixmap(icon, "white");
}

QPixmap scaleBlurPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if (!imgFile.exists()) {
        // qDebug()<< "pixmap file not exist!";
        return QPixmap();
    }
    QImageReader imgReader;
    imgReader.setFileName(url);
    imgReader.setAutoTransform(true);
    imgReader.setDecideFormatFromContent(true);
    QPixmap pixmap = blurPixmap(QPixmap::fromImageReader(&imgReader));
    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);

    //在设置Qt::WA_TranslucentBackground属性后，模糊图片会导致锁屏界面透明
    //因此这里修改image图形的alpha值为255.
    for (int y = 0; y < srcImg.height(); ++y) {
        QRgb *row = (QRgb *)srcImg.scanLine(y);
        for (int x = 0; x < srcImg.width(); ++x) {
            ((unsigned char *)&row[x])[3] = 255;
        }
    }
    painter.end();
    return QPixmap::fromImage(srcImg);
}
