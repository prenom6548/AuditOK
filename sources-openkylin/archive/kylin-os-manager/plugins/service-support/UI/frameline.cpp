#include <QApplication>

#include "frameline.h"

FrameLine::FrameLine(QWidget *parent) : QFrame(parent)
{
    this->setFixedHeight(2);
}

void FrameLine::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QColor color = qApp->palette().color(QPalette::WindowText);
    color.setAlphaF(0.08);
    p.save();
    p.setBrush(color);
    p.setOpacity(0.5);
    p.setPen(Qt::transparent);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    return QFrame::paintEvent(e);
}
