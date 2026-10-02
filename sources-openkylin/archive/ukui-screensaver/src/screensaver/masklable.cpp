#include "masklable.h"
#include "commonfunc.h"
#include <QPainter>
#include <QPainterPath>
#include <QIcon>

MaskLable::MaskLable(QWidget *parent, bool isControlSaver)
    : QLabel{parent},
      m_bControlSaver(isControlSaver)
{

}

void MaskLable::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (!m_bControlSaver) {
        painter.setOpacity(0.45);
        painter.setClipping(true);
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor("#000000"));
        painter.setPen(Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 8, 8);
        QRect widgetRect = this->rect();
        QPixmap prevIcon = drawSymbolicColoredPixmap(
            QIcon::fromTheme("software-installed-symbolic").pixmap(QSize(40, 40)), "white");
        QPoint centerPos = widgetRect.center() - QPoint(20, 20);
        painter.drawPixmap(centerPos, prevIcon);
    } else {
        QColor highlightColor = palette().color(QPalette::Highlight);
        QRectF borderRect = rect().adjusted(1, 1, -1, -1);
        QPainterPath path;
        path.addRoundedRect(borderRect, 8, 8);
        painter.setClipPath(path);
        painter.setClipping(false);
        QPen pen(highlightColor, 2);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}
