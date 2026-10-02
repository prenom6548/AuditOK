#include "adaptivelabel.h"
#include "commonfunc.h"
#include <QPainterPath>

AdaptiveLabel::AdaptiveLabel(QWidget *parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
}

void AdaptiveLabel::setPixmap(const QPixmap &pixmap)
{
    originalPixmap = pixmap;
}

void AdaptiveLabel::setScale(float scale)
{
    m_scale = scale;
    update();
}

void AdaptiveLabel::paintEvent(QPaintEvent *event)
{
    if (originalPixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const int labelWidth = width();
    const int labelHeight = height();
    const QSize pixmapSize = originalPixmap.size();

    const qint64 ratioDiff = qAbs(labelWidth * pixmapSize.height() - labelHeight * pixmapSize.width());
    const qint64 tolerance = 0.01 * labelHeight * pixmapSize.height();

    QRect sourceRect(0, 0, pixmapSize.width(), pixmapSize.height());
    QRect targetRect(0, 0, labelWidth, labelHeight);

    // 按照短边适应的方式显示图片，保持图片不变形，并且在比例不一致时会对图片进行裁剪
    if (ratioDiff > tolerance) {
        if (labelWidth * pixmapSize.height() > labelHeight * pixmapSize.width()) {
            const int newHeight = (pixmapSize.width() * labelHeight) / labelWidth;
            sourceRect = QRect(0, (pixmapSize.height() - newHeight)/2, pixmapSize.width(), newHeight);
        } else {
            const int newWidth = (pixmapSize.height() * labelWidth) / labelHeight;
            sourceRect = QRect((pixmapSize.width() - newWidth)/2, 0, newWidth, pixmapSize.height());
        }
    }
    QPainterPath path;
    path.addRoundedRect(targetRect, 26 * m_scale, 26 * m_scale);
    painter.setClipPath(path);
    painter.drawPixmap(targetRect, originalPixmap, sourceRect);
}
