#include "centerstackwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>

centerStackWidget::centerStackWidget(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(480, 520);
    m_pStackWidget = new QStackedWidget();
    m_pStackWidget->setContentsMargins(0, 0, 0, 0);

    m_pHboxWidgetLayout = new QHBoxLayout();
    m_pHboxWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pHboxWidgetLayout->setSpacing(0);
    m_pHboxWidgetLayout->addWidget(m_pStackWidget);
    this->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_pHboxWidgetLayout);
}

void centerStackWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    p.setBrush(QBrush(QColor(38, 38, 38)));
    p.setOpacity(0.35);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(opt.rect, 6, 6);
    p.setRenderHint(QPainter::Antialiasing); //反锯齿
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
