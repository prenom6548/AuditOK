#include <QDebug>

#include "progressbar.h"

ProgressBar::ProgressBar(QWidget *parent) : QWidget(parent)
{
    initUI();
}

ProgressBar::~ProgressBar()
{

}

void ProgressBar::setFileName(QString str)
{
    m_labelFileName->SetText(str);
}

void ProgressBar::setBarValue(qint64 num)
{
    m_bar->setValue(num);
    int perce = (num * 100)/m_bar->maximum();
    m_labelPerce->SetText(QString::number(perce).append("%"));
}

void ProgressBar::setSpeedValue(QString str)
{
    m_labelSpeed->SetText(str);
}

void ProgressBar::SetBarRange(qint64 min, qint64 max)
{
    qDebug() << " ^^^^^^^^^^^^^^^^^^^^^ " << max;
    m_bar->setRange(min, max);
}

void ProgressBar::initUI()
{    
    this->setFixedSize(430, 56);

    m_labelFileName = new K::TextLabel(this);
    m_labelPerce = new K::TextLabel(this);
    m_labelSpeed = new K::TextLabel(this);
    m_bar = new kdk::KProgressBar(this);
    m_bar->setFixedSize(428, 16);
    m_bar->setTextVisible(false);
    m_bar->setState(kdk::ProgressBarState::NormalProgress);  //设置KprogressBar状态

    m_layoutH = new QHBoxLayout();
    m_layoutH->addWidget(m_labelPerce);
    m_layoutH->addStretch();
    m_layoutH->addWidget(m_labelSpeed);

    m_layoutV = new QVBoxLayout(this);

    m_layoutV->addStretch();
    m_layoutV->addWidget(m_labelFileName);
    m_layoutV->addWidget(m_bar);
    m_layoutV->addLayout(m_layoutH);
    m_layoutV->addStretch();
    m_layoutV->setSpacing(0);
    m_layoutV->setMargin(0);

    m_labelPerce->SetFontColor(QColor(166,166,166));
    m_labelSpeed->SetFontColor(QColor(166,166,166));

    return;
}
