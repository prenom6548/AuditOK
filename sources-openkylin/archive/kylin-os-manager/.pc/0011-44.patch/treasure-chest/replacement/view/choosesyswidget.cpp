#include <QDebug>

#include "choosesyswidget.h"
#include "../globaldata.h"

ChooseSysWidget::ChooseSysWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initStyle();
    initLayout();
    initConn();
}

ChooseSysWidget::~ChooseSysWidget()
{

}

ChooseSysWidget *ChooseSysWidget::getInstance()
{
    static ChooseSysWidget *instance = nullptr;
    if (nullptr == instance) {
        instance = new ChooseSysWidget();
    }
    return instance;
}

void ChooseSysWidget::initUI()
{
    m_labelHeadline = new TextLabel(this);
    m_linuxRadioBtn = new RadioWidget(false);
    m_winRadioBtn = new RadioWidget(false);
    m_btnNext = new QPushButton(this);
    m_btnBack = new QPushButton(this);

    m_layoutHHeadline = new QHBoxLayout();
    m_layoutHBtn = new QHBoxLayout();
    m_layoutVMain = new QVBoxLayout();

    GlobalData::getInstance();
    return;
}

void ChooseSysWidget::initStyle()
{
    this->setFixedSize(GlobalData::g_sizeWidget);
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明

    m_linuxRadioBtn->setLabelIcon(GlobalData::g_iconLinuxSys);
    m_linuxRadioBtn->setLabelText(SysType::LinuxSys);
    m_winRadioBtn->setLabelIcon(GlobalData::g_iconWinSys);
    m_winRadioBtn->setLabelText(SysType::WinSys);

    m_labelHeadline->SetText(GlobalData::g_textChooseOldSysType);
    m_labelHeadline->SetPixelSize(24, true);

    m_btnNext->setText(GlobalData::g_textNextBtn);
    m_btnNext->setEnabled(false);

    m_btnBack->setText(GlobalData::g_textBackBtn);
    m_btnBack->setFlat(false);
}

void ChooseSysWidget::initLayout()
{
    m_layoutHBtn->addStretch();
    m_layoutHBtn->addWidget(m_btnBack);
    m_layoutHBtn->addSpacing(16);
    m_layoutHBtn->addWidget(m_btnNext);
    m_layoutHBtn->addStretch();

    m_layoutVMain->addSpacing(24);
    m_layoutVMain->addWidget(m_labelHeadline, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(40);
    m_layoutVMain->addWidget(m_linuxRadioBtn, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(16);
    m_layoutVMain->addWidget(m_winRadioBtn, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(76);
    m_layoutVMain->addLayout(m_layoutHBtn);
    m_layoutVMain->addSpacing(88);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);

    this->setLayout(m_layoutVMain);
}

void ChooseSysWidget::initConn()
{
    connect(m_linuxRadioBtn , &RadioWidget::sigClicked, this, &ChooseSysWidget::slotCilckLinuxRadio);
    connect(m_linuxRadioBtn , &RadioWidget::sigCannel, this, &ChooseSysWidget::slotCannelLinuxRadio);
    connect(m_winRadioBtn , &RadioWidget::sigCannel, this, &ChooseSysWidget::slotCannelWinRadio);
    connect(m_winRadioBtn , &RadioWidget::sigClicked, this, &ChooseSysWidget::slotCilckWinRadio);
    connect(m_btnNext, &QPushButton::clicked, this, &ChooseSysWidget::slotNextPadge);
    connect(m_btnBack, &QPushButton::clicked, this, &ChooseSysWidget::slotBackPage);
}

void ChooseSysWidget::slotCilckWinRadio()
{
    qDebug() << "===" << m_isClickWin << m_isClickLinux;
    // 触发点击事件：老设备取消点击
    if (m_linuxRadioBtn->getCilckStyle()) {
        m_linuxRadioBtn->setClickStyle();
    }
    if (m_isClickLinux) {
        m_isClickLinux = false;
    }
    m_chooseType = SysType::WinSys;
    m_isClickWin = true;
    m_btnNext->setEnabled(true);
    GlobalData::getInstance()->setHighColorBtn(m_btnNext);
    qDebug() << "===" << m_isClickWin << m_isClickLinux;
    return;
    
}

void ChooseSysWidget::slotCilckLinuxRadio()
{
    qDebug() << "---" << m_isClickWin << m_isClickLinux;
    if (m_winRadioBtn->getCilckStyle()) {
        m_winRadioBtn->setClickStyle();
    }
    if (m_isClickWin) {
        m_isClickWin = false;
    }
    m_chooseType = SysType::LinuxSys;
    m_isClickLinux = true;
    m_btnNext->setEnabled(true);
    GlobalData::getInstance()->setHighColorBtn(m_btnNext);
    qDebug() << "---" << m_isClickWin << m_isClickLinux;
}

void ChooseSysWidget::slotCannelLinuxRadio()
{
    // 判断当前点击状态，Linux 1 Win 0，Linux 0 Win 0，Linux 0 Win 1
    if (m_isClickLinux) {
        m_isClickLinux = false;
        m_btnNext->setEnabled(false);
        m_chooseType = SysType::NobadySys;
        qDebug() << "---" << m_isClickWin << m_isClickLinux;
        return;
    } 
}

void ChooseSysWidget::slotCannelWinRadio()
{
    // 判断当前点击状态，Linux 1 Win 0，Linux 0 Win 0，Linux 0 Win 1
    if (m_isClickWin) {
        m_isClickWin = false;
        m_btnNext->setEnabled(false);
        m_chooseType = SysType::NobadySys;
        qDebug() << "===" << m_isClickWin << m_isClickLinux;
        return;
    } 
}

void ChooseSysWidget::slotBackPage()
{
    emit sigBackPage();
}

void ChooseSysWidget::slotNextPadge()
{
    emit sigNextPage(m_chooseType);
}
