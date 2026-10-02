#include "drawiconlabel.h"
#include <QPainter>
#include <QDebug>
#define TOP_LEFT_CORNER 1
#define TOP_RIGHT_CORNER 2
#define BOTTOM_RIGHT_CORNER 4
#define BOTTOM_LEFT_CORNER 8

DrawIconLabel::DrawIconLabel(int radius, QWidget *parent)
    : QLabel(parent)
    , m_radius(radius)
{
    setScaledContents(true);
}

DrawIconLabel::DrawIconLabel(const QPixmap &pixmap, int radius, QWidget *parent)
    : QLabel(parent)
    , m_pixmap(pixmap)
    , m_radius(radius)

{
    setScaledContents(true);
}

QPainterPath DrawIconLabel::transformRectToPath(const QRect &rect, int radius, int corner) const
{
    QPainterPath path;
    if(corner & TOP_LEFT_CORNER) {
        path.moveTo(rect.x() + radius, rect.y());
        path.arcTo(rect.x(), rect.y(), radius*2, radius*2, 90.f, 90.f);//画左上圆角
    } else {
        path.moveTo(rect.x(), rect.y());
    }

    if(corner & BOTTOM_LEFT_CORNER) {
        path.lineTo(rect.x(), rect.y() + rect.height() -radius);
        path.arcTo(rect.x(), rect.y() + rect.height() - radius *2, radius*2, radius*2, 180.0f, 90.f);
    } else {
        path.lineTo(rect.x(), rect.y() + rect.width());
    }

    if(corner & BOTTOM_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width() - radius,rect.y() + rect.height());
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y() + rect.height() - 2*radius,radius*2, radius*2, 270.0f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y() + rect.height());
    }

    if(corner & TOP_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width(), rect.y() + radius);
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y(), radius*2,radius*2, 0.f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y());
    }
    return path;
}

void DrawIconLabel::setIcon(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
}

void DrawIconLabel::setRadius(int radius)
{
    m_radius = radius - 2;
}

void DrawIconLabel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath picPath = transformRectToPath(this->rect(), m_radius, TOP_RIGHT_CORNER | BOTTOM_LEFT_CORNER | TOP_LEFT_CORNER | BOTTOM_RIGHT_CORNER);
    p.setClipPath(picPath);
    p.drawPixmap(this->rect(), m_pixmap);

    return QLabel::paintEvent(event);
}
