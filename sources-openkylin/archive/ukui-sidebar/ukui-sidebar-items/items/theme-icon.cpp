/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-6-7.
//

#include "theme-icon.h"

#include <QUrl>
#include <QPainter>
#include <QFile>
#include <QImageReader>
#include <QDebug>
#include <QGuiApplication>
#include <QPalette>
#include <QImage>
#include <QtMath>
#include <QPainterPath>

#define COLOR_DIFFERENCE 10

QColor ThemeIcon::symbolicColor = QColor(31, 32, 34, 192);

ThemeIcon::ThemeIcon(QQuickItem *parent) : QQuickPaintedItem(parent)
{

}

//void ThemeIcon::paint(QPainter *painter)
//{
//    QRect rect(0, 0, static_cast<int>(width()), static_cast<int>(height()));
//
//    qDebug() << "radius:" << m_radius << height();
//
//    QPixmap scaledPixmap = m_icon.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//    qDebug() << "scaledPixmap:" << scaledPixmap.size();
//
//    QPixmap radiusPixmap(rect.size());
//    radiusPixmap.fill(Qt::transparent);
//    QPainter pixmapPainter(&radiusPixmap);
//
//    QPainterPath clipPath;
//    clipPath.addRoundedRect(rect, m_radius, m_radius);
//    pixmapPainter.setClipPath(clipPath);
//    //抗锯齿
//    pixmapPainter.setRenderHint(QPainter::Antialiasing);
//    //平滑过渡
//    pixmapPainter.setRenderHint(QPainter::SmoothPixmapTransform);
//
//    pixmapPainter.drawPixmap(rect, scaledPixmap);
//
//    painter->drawPixmap(rect, radiusPixmap);
//}

void ThemeIcon::paint(QPainter *painter)
{
    //默认居中绘制
    QRect rect(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    QPixmap target = m_rawIcon.pixmap({128, 128});

    painter->save();
    //抗锯齿,平滑过渡
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_disabled) {
        QPainter p(&target);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), QGuiApplication::palette().color(QPalette::Disabled, QPalette::ButtonText));

    } else if (m_highLight) {
        bool isPureColor = true;
        if(!m_forceHighlight) {
            isPureColor = isPixmapPureColor(target);
        }
        if (isPureColor) {
            QPainter p(&target);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), QGuiApplication::palette().color(QPalette::HighlightedText));
        }
    }

    if (m_radius > 0) {
        int radius = qMin(m_radius, qMin((rect.height() / 2), (rect.width() / 2)));
        QPainterPath path;
        path.addRoundedRect(rect, radius, radius);
        painter->setClipPath(path);
    }

    painter->drawPixmap(rect, target, target.rect());
    painter->restore();
}

void ThemeIcon::setIcon(const QIcon& icon)
{
    m_rawIcon = icon;
    update();
}

QIcon ThemeIcon::getIcon()
{
    return m_rawIcon;
}

QString ThemeIcon::getSource()
{
    return m_source;
}

void ThemeIcon::setSource(const QString &source)
{
    if (source.isEmpty()) {
        qWarning() << "ThemeIcon: source is empty!";
        return;
    }

    QUrl url(source);
    QString schema = url.scheme();

    if (!schema.isEmpty()) {
        QString path = url.path();
        if (!path.isEmpty()) {
            if (schema == QLatin1String("qrc")) {
                path.prepend(QLatin1String(":"));
            }
            readImage(path);

        } else {
            qWarning() << "Error: ThemeIcon: source is invalid! schema:" << schema;
            return;
        }
    } else {
        //qrc path: :/xxx/xxx.png
        if (source.startsWith(QLatin1String("/")) || source.startsWith(QLatin1String(":/"))) {
            readImage(source);

        } else {
            if (!QIcon::hasThemeIcon(source)) {
                qWarning() << "Error: ThemeIcon: icon dose not exists. name:" << source;
                return;
            }
            QIcon icon = QIcon::fromTheme(source);
            setIcon(icon);
        }
    }
}

QString ThemeIcon::getFallBack()
{
    return m_fallback;
}

void ThemeIcon::setFallBack(const QString &fallback)
{
    if (fallback.isEmpty()) {
        qWarning() << "ThemeIcon: fallback is empty!";
        return;
    }

    if (m_rawIcon.isNull()) {
        setSource(fallback);
    }
}


void ThemeIcon::readImage(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        qDebug() << "Error: ThemeIcon: " << QString("File not found: %1").arg(path);
        return;
    }

    if (!file.open(QFileDevice::ReadOnly)) {
        qWarning() << "Error: ThemeIcon: " << QString("Cannot open: %1").arg(path);
        return;
    }

    QImageReader imageReader(&file);
    QImage image;
    QPixmap pixmap;

    if (!imageReader.read(&image)) {
        qWarning() << "Error: ThemeIcon: " << QString("Error decoding: %1").arg(path);
        return;
    }

    pixmap = QPixmap::fromImage(image);
    m_source = path;
    file.close();
    setIcon(pixmap);
}

void ThemeIcon::setIcon(const QPixmap &pixmap)
{
    QIcon icon(pixmap);
    m_rawIcon.swap(icon);
    update();
}

bool ThemeIcon::isHighLight() const
{
    return m_highLight;
}

void ThemeIcon::setHighLight(bool highLight)
{
//    qDebug() << "set high-light" << highLight;
    m_highLight = highLight;
    update();
}

bool ThemeIcon::isForceHighlight() const
{
    return m_forceHighlight;
}

void ThemeIcon::setForceHighLight(bool force)
{
    m_forceHighlight = force;
}

bool ThemeIcon::disable() const
{
    return m_disabled;
}

void ThemeIcon::setDisable(bool disable)
{
    m_disabled = disable;
    update();
}

//copy from ukui-platform-theme
bool ThemeIcon::isPixmapPureColor(const QPixmap &pixmap)
{
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
                int dr = qAbs(color.red() - symbolicColor.red());
                int dg = qAbs(color.green() - symbolicColor.green());
                int db = qAbs(color.blue() - symbolicColor.blue());
                if (dr > COLOR_DIFFERENCE || dg > COLOR_DIFFERENCE || db > COLOR_DIFFERENCE)
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

int ThemeIcon::radius()
{
    return m_radius;
}

void ThemeIcon::setRadius(int radius)
{
    m_radius = radius < 0 ? 0 : radius;
}
