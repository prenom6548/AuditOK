#include <QDebug>

#include "choosedevicewidget.h"
#include "../globaldata.h"

ChooseDeviceWidget::ChooseDeviceWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initStyle();
    initLayout();
    initConn();
}

ChooseDeviceWidget::~ChooseDeviceWidget()
{

}

ChooseDeviceWidget *ChooseDeviceWidget::getInstance()
{
    static ChooseDeviceWidget *instance = nullptr;
    if (nullptr == instance) {
        instance = new ChooseDeviceWidget();
    }
    return instance;
}

void ChooseDeviceWidget::initUI()
{
    m_labelHeadline = new TextLabel(this);
    m_newRadioBtn = new RadioWidget(true);
    m_oldRadioBtn = new RadioWidget(true);
    m_btnNext = new QPushButton(this);

    m_layoutHHeadline = new QHBoxLayout();
    m_layoutHRadioBtn = new QHBoxLayout();
    m_layoutHBtn = new QHBoxLayout();
    m_layoutVMain = new QVBoxLayout();

    GlobalData::getInstance();
    return;
}

void ChooseDeviceWidget::initStyle()
{
    this->setFixedSize(GlobalData::g_sizeWidget);
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明

    m_newRadioBtn->setLabelIcon(GlobalData::g_iconNewSys);
    m_newRadioBtn->setLabelText(SysType::NewSys);
    m_oldRadioBtn->setLabelIcon(GlobalData::g_iconOldSys);
    m_oldRadioBtn->setLabelText(SysType::OldSys);

    m_labelHeadline->SetText(GlobalData::g_textChooseDeviceType);
    m_labelHeadline->SetPixelSize(24, true);

    m_btnNext->setText(GlobalData::g_textNextBtn);
    m_btnNext->setEnabled(false);
    GlobalData::getInstance()->setHighColorBtn(m_btnNext);
}

void ChooseDeviceWidget::initLayout()
{
    m_layoutHRadioBtn->addStretch();
    m_layoutHRadioBtn->addWidget(m_oldRadioBtn);
    m_layoutHRadioBtn->addSpacing(16);
    m_layoutHRadioBtn->addWidget(m_newRadioBtn);
    m_layoutHRadioBtn->addStretch();

    m_layoutHBtn->addStretch();
    m_layoutHBtn->addWidget(m_btnNext);
    m_layoutHBtn->addStretch();

    m_layoutVMain->addSpacing(25);
    m_layoutVMain->addWidget(m_labelHeadline, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(32);
    m_layoutVMain->addLayout(m_layoutHRadioBtn);
    m_layoutVMain->addSpacing(32);
    m_layoutVMain->addLayout(m_layoutHBtn);
    m_layoutVMain->addSpacing(88);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);

    this->setLayout(m_layoutVMain);
}

void ChooseDeviceWidget::initConn()
{
    connect(m_newRadioBtn , &RadioWidget::sigClicked, this, &ChooseDeviceWidget::slotCilckNewRadio);
    connect(m_newRadioBtn , &RadioWidget::sigCannel, this, &ChooseDeviceWidget::slotCannelNewRadio);
    connect(m_oldRadioBtn , &RadioWidget::sigCannel, this, &ChooseDeviceWidget::slotCannelOldRadio);
    connect(m_oldRadioBtn , &RadioWidget::sigClicked, this, &ChooseDeviceWidget::slotCilckOldRadio);
    connect(m_btnNext, &QPushButton::clicked, this, &ChooseDeviceWidget::slotNextBtnClick);
}

void ChooseDeviceWidget::slotCilckOldRadio()
{
    // 触发点击事件：老设备取消点击
    if (m_newRadioBtn->getCilckStyle()) {
        m_newRadioBtn->setClickStyle();
    }
    if (m_isClickNew) {
        m_isClickNew = false;
    }
    m_chooseType = SysType::OldSys;
    m_isClickOld = true;
    m_btnNext->setEnabled(true);
    return;
    
}

void ChooseDeviceWidget::slotCilckNewRadio()
{
    qDebug() << "---" << m_isClickOld << m_isClickNew;
    if (m_oldRadioBtn->getCilckStyle()) {
        m_oldRadioBtn->setClickStyle();
    }
    if (m_isClickOld) {
        m_isClickOld = false;
    }
    m_chooseType = SysType::NewSys;
    m_isClickNew = true;
    m_btnNext->setEnabled(true);
    qDebug() << "---" << m_isClickOld << m_isClickNew;
}

void ChooseDeviceWidget::slotCannelNewRadio()
{
    // 判断当前点击状态，new 1 old 0，new 0 old 0，new 0 old 1
    if (m_isClickNew) {
        m_isClickNew = false;
        m_btnNext->setEnabled(false);
        m_chooseType = SysType::NobadySys;
        qDebug() << "---" << m_isClickOld << m_isClickNew;
        return;
    } 
}

void ChooseDeviceWidget::slotCannelOldRadio()
{
    // 判断当前点击状态，new 1 old 0，new 0 old 0，new 0 old 1
    if (m_isClickOld) {
        m_isClickOld = false;
        m_btnNext->setEnabled(false);
        m_chooseType = SysType::NobadySys;
        return;
    } 
}

void ChooseDeviceWidget::slotNextBtnClick()
{
    emit sigNextPage(m_chooseType);
}
