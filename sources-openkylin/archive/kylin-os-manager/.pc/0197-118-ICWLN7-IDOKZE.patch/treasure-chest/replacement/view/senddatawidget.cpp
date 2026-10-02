#include <QDebug>

#include "senddatawidget.h"


SendDataWidget::SendDataWidget(QWidget *parent)
{
    initUI();
    initStyle();
    initLayout();
    initConn();
}

SendDataWidget::~SendDataWidget()
{

}

void SendDataWidget::initUI()
{
    // m_widget = new QWidget(this);
    m_widget = new OldSysInfo(this);

    m_labelInfo = new K::TextLabel(this);
    m_btnBack = new QPushButton(this);   
    m_layoutVMain = new QVBoxLayout();
    m_layoutF = new QFormLayout();
}

void SendDataWidget::initStyle()
{
    this->setFixedSize(GlobalData::g_sizeWidget);
    this->setBackgroundRole(QPalette::Base);
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明

    m_labelInfo->SetText(GlobalData::g_textOldSysInfo);
    m_labelInfo->SetPixelSize(24, true);


    m_btnBack->setText(GlobalData::g_textBackBtn);

}

void SendDataWidget::initLayout()
{
    m_layoutVMain->addSpacing(25);
    m_layoutVMain->addWidget(m_labelInfo, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(32);
    m_layoutVMain->addWidget(m_widget, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(32);
    m_layoutVMain->addWidget(m_btnBack, 0, Qt::AlignHCenter);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);
    this->setLayout(m_layoutVMain);
}

void SendDataWidget::initConn()
{
    connect(m_btnBack, &QPushButton::clicked, this, &SendDataWidget::slotBackPage);
}

void SendDataWidget::slotBackPage()
{
    m_widget->slotStopShare();
    emit sigBackPage();
}

