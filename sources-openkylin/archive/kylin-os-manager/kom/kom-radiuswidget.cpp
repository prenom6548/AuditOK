#include "kom-radiuswidget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
namespace kom
{
RadiusWidget::RadiusWidget(QWidget *parent) : QWidget(parent)
{
    m_colorRole = QPalette::Base;
}

void RadiusWidget::setColorRole(QPalette::ColorRole role)
{
    m_colorRole = role;
}

void RadiusWidget::setRadius(int r)
{
    m_radius = r;
}

void RadiusWidget::paintEvent(QPaintEvent *e)
{
    QColor col = qApp->palette().color(m_colorRole);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(col);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), m_radius, m_radius);
}
} // namespace kom
