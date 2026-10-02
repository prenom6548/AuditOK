/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "previewlabel.h"
#include <QPainter>
#include <QtMath>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOption>
#include <QPainterPath>
#include <QGuiApplication>

PreviewLabel::PreviewLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
        QLabel(text, parent, f)
{
    setupScreenMonitoring();
    updateSize();
}

PreviewLabel::PreviewLabel(QWidget *parent, Qt::WindowFlags f):
        QLabel(parent, f)
{
    setupScreenMonitoring();
    updateSize();
}

PreviewLabel::~PreviewLabel()
{
    // 断开所有屏幕连接
    auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        disconnect(screen, &QScreen::geometryChanged, this, &PreviewLabel::updateSize);
    }

    // 断开应用程序信号
    disconnect(qApp, &QGuiApplication::screenAdded, this, nullptr);
    disconnect(qApp, &QGuiApplication::screenRemoved, this, nullptr);
}

void PreviewLabel::setupScreenMonitoring()
{
    // 监听所有屏幕的变化
    auto screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        connect(screen, &QScreen::geometryChanged, this, &PreviewLabel::updateSize);
    }

    // 监听屏幕添加/移除
    connect(qApp, &QGuiApplication::screenAdded, this, &PreviewLabel::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &PreviewLabel::onScreenRemoved);
}

void PreviewLabel::onScreenAdded(QScreen *screen)
{
    if (screen) {
        // 连接新添加的屏幕
        connect(screen, &QScreen::geometryChanged, this, &PreviewLabel::updateSize);
        updateSize(); // 屏幕添加后需要更新尺寸
    }
}

void PreviewLabel::onScreenRemoved(QScreen *screen)
{
    if (screen) {
        disconnect(screen, &QScreen::geometryChanged, this, &PreviewLabel::updateSize);
        updateSize(); // 屏幕移除后需要更新尺寸
    }
}

void PreviewLabel::updateSize()
{
    QSize screenSize = maxScreenSize();
    QSize targetSize = calculateScaledSize(screenSize, QSize(288, 162));
    if (targetSize.height() <= 0 || targetSize.width() <= 0) {
        targetSize = QSize(288, 162);
    }
    setFixedSize(targetSize);
}

void PreviewLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (displayType == "color") {
        QStylePainter painter(this);
        QStyleOption opt;
        opt.init(this);
        QPainterPath path;
        path.addRoundedRect(this->rect(), 8, 8);
        painter.setClipPath(path);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(mColor);
        painter.drawRect(opt.rect);
        return;
    }

    QPainter p(this);
    if (this->m_pixmap.isNull()) {
        return;
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QSize screenSize = maxScreenSize();
    QPixmap pixmap = scaleExistingPixmap(m_pixmap);
    // 防御：避免 screenSize 宽高为 0 导致除 0 崩溃（屏幕插拔瞬间可能返回无效尺寸）
    if (screenSize.width() <= 0 || screenSize.height() <= 0) {
        qWarning() << "PreviewLabel::paintEvent invalid screen size:" << screenSize
                   << ", skip painting to avoid division by zero.";
        p.restore();
        return;
    }

    QSize mSize(pixmap.width() * this->width()/screenSize.width(),
                pixmap.height() * this->height()/screenSize.height());
    qDebug()<<"the destkop picture size:" <<pixmap.size()<<"screen size:"<<screenSize<<"window size:"<<this->size()<<"window picture size"<<mSize;
    if (pixmap.isNull() || mSize.height() <= 0 || mSize.width() <= 0 || (m_pixmap.width() * m_pixmap.height() < mSize.width() * mSize.height())) {
        pixmap = m_pixmap;
    } else {
        pixmap = pixmap.scaled(mSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QPainterPath path;
    path.addRoundedRect(this->rect(), 8, 8);
    p.setClipPath(path);
    if (mode == "centered") {
        //居中

        p.drawPixmap((this->size().width() - pixmap.rect().width()) / 2,
                     (this->size().height() - pixmap.rect().height()) / 2,
                     pixmap);

    } else if (mode == "stretched") {
        //拉伸
        p.drawPixmap(this->rect(), pixmap, pixmap.rect());
    } else if (mode == "scaled") {
        //填充
        p.drawPixmap(this->rect(), pixmap, getSourceRect(pixmap));
    } else if (mode == "wallpaper") {
        //平铺
        int drawedWidth = 0;
        int drawedHeight = 0;
        while (1) {
            drawedWidth = 0;
            while (1) {
                p.drawPixmap(drawedWidth, drawedHeight, pixmap);
                drawedWidth += pixmap.width();
                if (drawedWidth >= this->size().width()) {
                    break;
                }
            }
            drawedHeight += pixmap.height();
            if (drawedHeight >= this->size().height()) {
                break;
            }
        }
    } else if (mode == "zoom") {
        p.drawPixmap(getDestRect(pixmap), pixmap, pixmap.rect());
    } else if (mode == "spanned") {
        p.drawPixmap(this->rect(), pixmap, getSourceRect(pixmap, this->geometry()));
    } else {
        p.drawPixmap(rect().adjusted(0, 0, 0, 0), pixmap, pixmap.rect());
    }
    p.restore();
}

void PreviewLabel::setLocalPixmap(QPixmap pixmap)
{
    displayType = "picture";
    this->m_pixmap = pixmap;
}

QPixmap PreviewLabel::scaleExistingPixmap(const QPixmap& originalPixmap)
{
    if (originalPixmap.isNull()) {
        return m_pixmap;
    }

    // 获取当前最大屏幕尺寸（需要实现这个函数）
    QSize screenSize = maxScreenSize();

    // 计算目标尺寸
    QSize targetSize = calculateWallpaperTargetSize(originalPixmap.size(), screenSize);

    if (!targetSize.isValid()) {
        return originalPixmap;
    }

    // 转换为 QImage 进行格式统一和缩放
    QImage img = originalPixmap.toImage();

    // 格式转换
    if (img.format() != QImage::Format_RGB888) {
        img = img.convertToFormat(QImage::Format_RGB888);
    }

    // 缩放
    if (img.size() != targetSize) {
        img = img.scaled(targetSize,
                        Qt::KeepAspectRatioByExpanding,
                        Qt::SmoothTransformation);
    }

    return QPixmap::fromImage(std::move(img));
}

QSize PreviewLabel::maxScreenSize() const
{
    QSize maxSize = this->size();
    const auto screens = qApp->screens();

    // 当屏幕正在插拔或尚未就绪时，可能出现没有任何屏幕的情况，
    // 这时直接返回 (0, 0) 会导致后续除 0 崩溃，这里做防御处理。
    if (screens.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "screens is empty" << this->size();
        if (!this->size().isEmpty()) {
            return this->size();
        }
        // 再退化到一个固定的预览默认值
        return QSize(288, 162);
    }

    for (QScreen* screen : screens) {
        QSize size = screen->size();
        // get the device pixel ratio
        qreal devicePixelRatio = screen->devicePixelRatio();
        // calculate the actual pixel size needed considering the device pixel ratio
        size *= devicePixelRatio;
        maxSize.setWidth(qMax(size.width(), maxSize.width()));
        maxSize.setHeight(qMax(size.height(), maxSize.height()));
    }
    return maxSize;
}

QSize PreviewLabel::calculateWallpaperTargetSize(const QSize &originalSize, const QSize &screenSize) const
{
    // Invalid size check
    if (!originalSize.isValid() || !screenSize.isValid()) {
        qWarning() << "Invalid size detected:"
                  << "Original:" << originalSize
                  << "Screen:" << screenSize;
        return QSize();
    }

    // Calculate aspect ratios
    const qreal sourceRatio = static_cast<qreal>(originalSize.width()) / originalSize.height();
    const qreal screenRatio = static_cast<qreal>(screenSize.width()) / screenSize.height();

    // Maximum allowed scale multiplier to prevent excessive memory usage
    const qreal MAX_SCALE_MULTIPLIER = 1.5;

    QSize targetSize;

    if (sourceRatio >= screenRatio) {
        // Image is wider than screen (relative to aspect ratios)
        // Use screen height as base and calculate width to maintain aspect ratio
        int targetHeight = screenSize.height();
        int baseWidth = targetHeight * sourceRatio;

        // Limit maximum width to prevent excessive scaling
        int maxAllowedWidth = screenSize.width() * MAX_SCALE_MULTIPLIER;
        int targetWidth = qMin(baseWidth, maxAllowedWidth);

        // If width was limited, adjust height to maintain aspect ratio
        if (targetWidth < baseWidth) {
            targetHeight = targetWidth / sourceRatio;
        }

        targetSize = QSize(targetWidth, targetHeight);
    } else {
        // Image is taller than screen (relative to aspect ratios)
        // Use screen width as base and calculate height to maintain aspect ratio
        int targetWidth = screenSize.width();
        int baseHeight = targetWidth / sourceRatio;

        // Limit maximum height to prevent excessive scaling
        int maxAllowedHeight = screenSize.height() * MAX_SCALE_MULTIPLIER;
        int targetHeight = qMin(baseHeight, maxAllowedHeight);

        // If height was limited, adjust width to maintain aspect ratio
        if (targetHeight < baseHeight) {
            targetWidth = targetHeight * sourceRatio;
        }

        targetSize = QSize(targetWidth, targetHeight);
    }

    // Verify if aspect ratio is maintained
    qreal targetRatio = static_cast<qreal>(targetSize.width()) / targetSize.height();
    qreal ratioDiff = qAbs(targetRatio - sourceRatio);

    // Allow for a small floating-point error (0.01)
    if (ratioDiff > 0.01) {
        qWarning() << "Aspect ratio mismatch detected:"
                  << "Source:" << sourceRatio
                  << "Target:" << targetRatio;
        // Force aspect ratio correction
        targetSize.setWidth(targetSize.height() * sourceRatio);
    }

    return targetSize;
}

QSize PreviewLabel::calculateScaledSize(const QSize &sourceSize, const QSize &maxSize) const
{
    // 检查输入参数的有效性
    if (sourceSize.isEmpty() || maxSize.isEmpty()) {
        return QSize(288, 162);
    }

    // 计算宽高比的缩放比例
    double widthRatio = static_cast<double>(maxSize.width()) / sourceSize.width();
    double heightRatio = static_cast<double>(maxSize.height()) / sourceSize.height();

    // 选择较小的比例，确保两个方向都不会超出最大尺寸
    double scaleRatio = qMin(widthRatio, heightRatio);

    // 计算缩放后的尺寸
    int scaledWidth = qRound(sourceSize.width() * scaleRatio);
    int scaledHeight = qRound(sourceSize.height() * scaleRatio);

    return QSize(scaledWidth, scaledHeight);
}

QRect PreviewLabel::getSourceRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if ((width / height) == screenScale) {
        return pixmap.rect();
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(this->rect().height()) / qreal(this->rect().width());
    }

    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = pixmap.size();
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(isShortX ? shortEdge : longEdge);
        sourceSize.setHeight(isShortX ? longEdge : shortEdge);
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (pixmap.width() > sourceSize.width()) {
        offsetX = (pixmap.width() - sourceSize.width()) / 2;
    }

    if (pixmap.height() > sourceSize.height()) {
        offsetY = (pixmap.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, sourceSize);
}

QRect PreviewLabel::getSourceRect(const QPixmap &pixmap, const QRect &screenGeometry)
{
    QRect virtualGeometry = this->geometry();
    qreal pixWidth = pixmap.width();
    qreal pixHeight = pixmap.height();

    QSize sourceSize = pixmap.size();
    sourceSize.setWidth(screenGeometry.width() / virtualGeometry.width() * pixWidth);
    sourceSize.setHeight(screenGeometry.height() / virtualGeometry.height() * pixHeight);

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (screenGeometry.x() > 0) {
        offsetX = (screenGeometry.x() / virtualGeometry.width() * pixWidth);
    }

    if (screenGeometry.y() > 0) {
        offsetY = (screenGeometry.y() / virtualGeometry.height() * pixHeight);
    }

    QPoint offsetPoint = pixmap.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, sourceSize);
}

QRect PreviewLabel::getDestRect(const QPixmap &pixmap)
{
    qreal screenScale = qreal(this->rect().width()) / qreal(this->rect().height());
    qreal pixmapScale = qreal(pixmap.width())  / qreal(pixmap.height());
    qreal width = pixmap.width();
    qreal height = pixmap.height();

    if (pixmapScale == screenScale) {
        return this->rect();
    }

    qreal scaleWidth = this->rect().width() / width;
    qreal scaleHeight = this->rect().height() / height;
    qreal realPixmapWidth = 0;
    qreal realPixmapHeight = 0;

    if(pixmapScale < screenScale){
        //图片比例小于屏幕比例时，按照图片和屏幕高度比进行缩放
        realPixmapWidth = width * scaleHeight;
        realPixmapHeight = this->rect().height();
    }else{
        //图片比例大于屏幕比例时，按照图片与屏幕宽度比进行缩放
        realPixmapWidth = this->rect().width();
        realPixmapHeight = height * scaleWidth;
    }

    QSize sourceSize = this->size();
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (this->rect().width() == realPixmapWidth) {
        offsetY = (this->rect().height() - realPixmapHeight) / 2;
        sourceSize.setHeight(realPixmapHeight);
    } else if (this->rect().height() == realPixmapHeight) {
        offsetX = (this->rect().width() - realPixmapWidth) / 2;
        sourceSize.setWidth(realPixmapWidth);
    }

    QPoint offsetPoint = this->rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

void PreviewLabel::setMode(QString mode)
{
    this->mode = mode;
}

void PreviewLabel::setColor(const QColor &color)
{
    displayType = "color";
    mColor = color;
}
