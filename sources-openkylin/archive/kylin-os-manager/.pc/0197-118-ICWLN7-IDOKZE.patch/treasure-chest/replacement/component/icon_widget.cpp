#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QPen>
#include <QApplication>
#include <QMouseEvent>

#include "icon_widget.h"

IconWidget::IconWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
}

IconWidget::~IconWidget()
{

}

void IconWidget::setLabelIcon(QSize iconSize, QIcon icon, QString str)
{
    // this->setFixedSize(size);
    m_labelIcon->setFixedSize(iconSize);
    m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(iconSize)));
    m_labelInfo->SetText(str);
}

void IconWidget::initUI()
{    
    m_labelIcon = new QLabel(this);
    m_labelInfo = new K::TextLabel(this);
    m_layoutV = new QVBoxLayout(this);

    m_layoutV->addStretch();
    m_layoutV->addWidget(m_labelIcon, 0, Qt::AlignHCenter);
    m_layoutV->addWidget(m_labelInfo, 0, Qt::AlignHCenter);
    m_layoutV->addStretch();
    m_layoutV->setSpacing(0);
    m_layoutV->setMargin(0);

    return;
}
