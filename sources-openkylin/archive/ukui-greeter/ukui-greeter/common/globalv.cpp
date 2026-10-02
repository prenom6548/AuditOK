/* globalv.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "globalv.h"

#include <QDebug>
#include <QPixmap>
#include <QProcess>
#include <QPainter>
#include <QImageReader>
#include <QPainterPath>
#include <QMimeDatabase>
#include <QtSvg/QSvgRenderer>

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

#define BLUR_RADIUS 300

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage srcImg = pixmap.toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);

    //在设置Qt::WA_TranslucentBackground属性后，模糊图片会导致锁屏界面透明
    //因此这里修改image图形的alpha值为255.
    for (int y = 0;y < srcImg.height();++y) {
      QRgb *row = (QRgb*)srcImg.scanLine(y);
      for (int x = 0; x < srcImg.width(); ++x) {
        ((unsigned char*)&row[x])[3] = 255;
      }
    }
    painter.end();
    return QPixmap::fromImage(srcImg);
}

QPixmap scaleBlurPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if(!imgFile.exists()){
        qDebug()<< "pixmap file not exist!";
        return QPixmap();
    }
    QImageReader imgReader;
    imgReader.setFileName(url);
    imgReader.setAutoTransform(true);
    imgReader.setDecideFormatFromContent(true);
    QPixmap pixmap = blurPixmap(QPixmap::fromImageReader(&imgReader));
    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

/**
 * @brief scaledPixmap
 * @param width
 * @param height
 * @param url
 * @return
 * 图片缩放
 */
QPixmap scaledPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if(!imgFile.exists()){
        qDebug()<< "not find the pixmap file";
        return QPixmap();
    }
    QImage image(url);
    QPixmap pixmap = QPixmap::fromImage(image);
    if(pixmap.isNull()) {
        qDebug() << "pixmap is null";
        QProcess exec;
        QString program("file " + url);
        exec.start(program);
        exec.waitForFinished(1000);
        QString output = exec.readAllStandardOutput();
        qDebug() << output;
        if(output.contains("SVG")){
            qDebug() << "image format is SVG";
            QSvgRenderer render(url);
            QImage image(width, height, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            render.render(&painter, image.rect());
            pixmap.convertFromImage(image);
        } else if(output.contains("TIFF")) {
            qDebug() << "image format is TIFF";

        }
    }

    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

bool isPicture(QString filepath)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filepath);
    qDebug() << "mime: " << filepath << mime.name();
    return mime.name().startsWith("image/");
}

QString ElideText(QFont font,int width,QString strInfo)
{
    QFontMetrics fontMetrics(font);
    //如果当前字体下，字符串长度大于指定宽度
    if(fontMetrics.width(strInfo) > width)
    {
        strInfo= QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

/*修改图片缩放机制，图片长宽不一致时，先取图片中央的部分*/
QPixmap cutPixmap(QPixmap src)
{
    QPixmap rectPixmap;
    if (src.width() > src.height()) {
        QPixmap iconPixmap = src.copy((src.width() - src.height())/2, 0, src.height(), src.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(src.height());
    } else {
        QPixmap iconPixmap = src.copy(0, (src.height() - src.width())/2, src.width(), src.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(src.width());
    }
    return QPixmap(rectPixmap);
}

QPixmap PixmapToRound(const QPixmap &src, int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(radius*2,radius*2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius*2, radius*2, pixmapa);
    return pixmap;
}
