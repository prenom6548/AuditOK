#include <QDebug>
#include <QApplication>

#include "../globaldata.h"
#include "status_widget.h"

StatusWidget::StatusWidget(QWidget *parent)
{
    initUI();
}

StatusWidget::~StatusWidget()
{

}

void StatusWidget::initUI()
{
    m_label = new K::TextLabel(this);
    m_labelInfo = new K::TextLabel(this);
    m_btnHighColor = new kdk::KPushButton(this);
    m_btnHighColor->setObjectName("btn");

    m_vlayout = new QVBoxLayout(this);
    m_vlayout->addWidget(m_label, 0, Qt::AlignHCenter);
    m_vlayout->addSpacing(4);
    m_vlayout->addWidget(m_btnHighColor, 0, Qt::AlignHCenter);
    m_vlayout->addSpacing(2);
    m_vlayout->addWidget(m_labelInfo, 0, Qt::AlignHCenter);
    m_vlayout->addStretch();
    m_vlayout->setSpacing(0);
    m_vlayout->setMargin(0);

    m_label->setFixedHeight(40);
    m_label->SetFontColor(QColor(89, 89, 89));
    m_label->SetPixelSize(14, true);
    m_labelInfo->SetPixelSize(12, false);

    m_btnHighColor->setFixedSize(8, 8);
    m_btnHighColor->setBorderRadius(4);
    m_btnHighColor->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_btnHighColor->setBackgroundColorHighlight(true);

    setNomarlStatus();
}

void StatusWidget::setHighColorStatus()
{
    QColor highLightColor = QApplication::palette().highlight().color();
    m_label->SetFontColor(highLightColor);
    m_label->SetPixelSize(18, true);
    
    m_btnHighColor->setBackgroundColorHighlight(true);
    
    m_labelInfo->show();
}

void StatusWidget::setNomarlStatus()
{
    m_labelInfo->hide();
    m_btnHighColor->setBackgroundColorHighlight(false);
    m_label->SetFontColor(QColor(89, 89, 89));
    m_label->SetPixelSize(14, true);
}

void StatusWidget::setStatusText(QString str1, QString str2)
{
    m_label->SetText(str1);
    m_labelInfo->SetText(str2);
}