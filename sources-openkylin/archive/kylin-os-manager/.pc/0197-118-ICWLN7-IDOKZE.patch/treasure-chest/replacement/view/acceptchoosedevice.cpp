#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDateTime>

#include "kom-configure.h"
#include "senddatawidget.h"
#include "acceptchoosedevice.h"

AcceptChooseDevice::AcceptChooseDevice(QWidget *parent)
{
    initUI();
}

AcceptChooseDevice::~AcceptChooseDevice()
{

}

void AcceptChooseDevice::initUI()
{
    this->setFixedSize(600, 280);

    m_labelId = new K::TextLabel(this);
    m_labelName = new K::TextLabel(this);
    m_labelPwd = new K::TextLabel(this);
    m_labelInfo = new K::TextLabel(this);
    m_labelTip = new K::TextLabel(this);
    
    m_comboBoxId = new QComboBox(this);
    m_editName = new QLineEdit(this);
    m_editPwd = new QLineEdit(this);

    m_layoutVMain = new QVBoxLayout();
    m_layoutHMain = new QHBoxLayout();
    m_layoutHLabel = new QHBoxLayout();
    m_layoutF = new QFormLayout();

    m_layoutF->setWidget(0,QFormLayout::LabelRole, m_labelId);
    m_layoutF->setWidget(0, QFormLayout::FieldRole, m_comboBoxId);
    m_layoutF->setWidget(1,QFormLayout::LabelRole, m_labelName);
    m_layoutF->setWidget(1, QFormLayout::FieldRole, m_editName);
    m_layoutF->setWidget(2,QFormLayout::LabelRole, m_labelPwd);
    m_layoutF->setWidget(2, QFormLayout::FieldRole, m_editPwd);
    m_layoutF->setWidget(3, QFormLayout::FieldRole, m_labelTip);
    m_layoutF->setHorizontalSpacing(16);
    m_layoutF->setVerticalSpacing(8);

    m_layoutHLabel->addStretch();
    m_layoutHLabel->addWidget(m_labelInfo);
    m_layoutHLabel->addStretch();

    m_layoutHMain->addStretch();
    m_layoutHMain->addLayout(m_layoutF);
    m_layoutHMain->addStretch();

    m_layoutVMain->addStretch();
    m_layoutVMain->addLayout(m_layoutHLabel);
    m_layoutVMain->addSpacing(24);
    m_layoutVMain->addLayout(m_layoutHMain);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);
    this->setLayout(m_layoutVMain);

    m_labelName->SetText(GlobalData::g_textOldSysUserName);
    m_labelId->SetText(GlobalData::g_textOldSysSignage);
    m_labelPwd->SetText(GlobalData::g_textOldSysCheckCode);

    m_comboBoxId->setFixedSize(310, 36);
    // m_comboBoxId->lineEdit()->setPlaceholderText(GlobalData::g_textReceChooseOld);
    m_editName->setFixedSize(310, 36);
    m_editName->setPlaceholderText(GlobalData::g_textRectRequired);
    m_editName->setReadOnly(true);
    m_editPwd->setFixedSize(310, 36);
    m_editPwd->setPlaceholderText(GlobalData::g_textRectRequired);

    m_labelInfo->SetText(GlobalData::g_textReceOldInfo);
    m_labelInfo->SetPixelSize(18, true);

    m_labelTip->SetText(GlobalData::g_textRectMsgCodeErr);
    m_labelTip->SetFontColor(QColor(243, 34, 45));
    m_labelTip->hide();

    // connect(m_comboBoxId, SIGNAL(currentIndexChanged(const QString &text)), this, SLOT(comboxChooseId(QString str)));

  connect(m_comboBoxId, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this , &AcceptChooseDevice::comboxChooseId);
}

void AcceptChooseDevice::startZeroConf()
{
    qDebug() << "star!!!";
    m_list.clear();
    m_hashIp.clear();
    m_comboBoxId->clear();
    connect(&m_zeroConf, &QZeroConf::serviceAdded, this, &AcceptChooseDevice::analyzName ,Qt::UniqueConnection);
    m_zeroConf.clearServiceTxtRecords();
    // m_zeroConf.startServicePublish("aaaaaaa", "_komt_replacement._tcp", "", 11437);
    m_zeroConf.startBrowser("_komt_replacement._tcp");
}

void AcceptChooseDevice::analyzName(QZeroConfService zcs)
{
    qDebug() << "serviceAdded: " << zcs->name() << "|"<< zcs->ip().toString() <<"|"<<zcs->domain();
    QStringList listName = zcs->name().split("###");
    QStringList listVale;
    if (m_list.contains(listName[0])) {
        return;
    }
    m_list.append(listName[0]);
    listVale << zcs->ip().toString() << listName[1];
    m_hashIp.insert(listName[0], listVale);
    m_comboBoxId->addItem(listName[0]);
}

void AcceptChooseDevice::stopZeroConf()
{
    m_editName->clear();
    m_editPwd->clear();
    m_zeroConf.stopBrowser();
}

void AcceptChooseDevice::comboxChooseId(const QString &str)
{
    QStringList list = m_hashIp.value(str);
    qDebug() <<m_hashIp.value(str);
    if (list.isEmpty()) {
        return;
    }
    m_stringIp = list.first();
    m_stringName = str + list.last();
    m_editName->setText(list.last());
    return;
}

QStringList AcceptChooseDevice::getInfo()
{
    m_stringPwd = m_editPwd->text();
    QStringList list;
    list.clear();
    list << m_stringIp << m_stringName << m_stringPwd;
    return list;
}

void AcceptChooseDevice::showOrHideTip(bool show)
{
    if (show) {
        m_labelTip->show();
    } else {
        m_labelTip->hide();
    }
    this->update();
}

bool AcceptChooseDevice::getAcceptDeviceInfo()
{
    if (m_editName->text() == "") {
        return false;
    } 
    if (m_editPwd->text() == "") {
        return false;
    }
    if (m_comboBoxId->currentText() == "") {
        return false;
    }
    return true;
}

void AcceptChooseDevice::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = QApplication::palette().window().color();
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    QColor highLightColor = opt.palette.highlight().color();
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 16, 16); // 绘制圆角
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);    
}
