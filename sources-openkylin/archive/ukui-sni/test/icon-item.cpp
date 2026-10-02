#include "icon-item.h"
#include <QRect>
#include <QImage>
#include <QtMath>
#include <QGuiApplication>
#include <QPalette>

#define COLOR_DIFFERENCE 10

QColor IconItem::symbolicColor = QColor(31, 32, 34, 192);

IconItem::IconItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
}

QVariant IconItem::getIcon() const
{
    return m_icon;
}

void IconItem::setIcon(QVariant icon)
{
    m_icon = icon;
    update();
}

QVariant IconItem::getOverlayIcon() const
{
    return m_overlayIcon;
}

void IconItem::setOverlayIcon(QVariant overlayIcon)
{
    m_overlayIcon = overlayIcon;
    update();
}

bool IconItem::getIsInverseColor()
{
    return m_isInverseColor;
}

bool IconItem::setIsInverseColor(bool isInverseColor)
{
    m_isInverseColor = isInverseColor;
    update();
}

void IconItem::paint(QPainter *painter)
{
    QIcon icon = m_icon.value<QIcon>();
    QPixmap pix = icon.pixmap(QSize(width(), height()));

    if (m_isInverseColor) {
        if (isPixmapPureColor(pix)) {
            QPainter p(&pix);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(pix.rect(), QGuiApplication::palette().color(QPalette::HighlightedText));
        }
    }

    painter->drawPixmap(QRect(0,0,width(), height()), pix);

    QIcon overlayIcon = m_overlayIcon.value<QIcon>();
    if (overlayIcon.isNull()) {
        return;
    }
    QPainter overlayPainter(&pix);
    int iconSize = qMin(width(), height());
    int overlaySize;

    if (iconSize < 32) {
        overlaySize = 8;
    } else if (iconSize <= 48) {
        overlaySize = 16;
    } else if (iconSize <= 96) {
        overlaySize = 22;
    } else if (iconSize < 256) {
        overlaySize = 32;
    } else {
        overlaySize = 64;
    }
    QPixmap pixmap = overlayIcon.pixmap(overlaySize, overlaySize);
    pixmap.setDevicePixelRatio(pix.devicePixelRatio());
    int margin = pixmap.devicePixelRatio() * 0.05 * iconSize;
    QPoint startPoint;
    startPoint = QPoint(width() - overlaySize - margin, height() - overlaySize - margin);
    startPoint /= pix.devicePixelRatio();
    overlayPainter.drawPixmap(startPoint, pixmap);
    qWarning() << overlaySize;
    painter->drawPixmap(QRect(0,0,width(), height()), pix);
}

bool IconItem::isPixmapPureColor(const QPixmap &pixmap)
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
