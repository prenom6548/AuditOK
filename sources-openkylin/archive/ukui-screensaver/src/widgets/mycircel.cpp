#include "mycircel.h"
#include <QPainter>
#include <QPalette>

MyCircel::MyCircel(int width, int height, QWidget *parent) : QWidget(parent)
{
    this->setMaximumSize(width, height);
}

MyCircel::~MyCircel() {}

void MyCircel::paintEvent(QPaintEvent *paintEvent)
{
    // qt画板获取主题色
    QPalette palette;
    QColor themeColor = palette.color(QPalette::Active, QPalette::Highlight);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::Qt4CompatiblePainting);
    painter.setBrush(themeColor);
    painter.setPen(themeColor);
    painter.drawEllipse(1, 1, 8, 8);

    return QWidget::paintEvent(paintEvent);
}
