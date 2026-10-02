/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "setPartitionsSize.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QBitmap>
#include <QStyleOption>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QEvent>
#include <QAction>
#include <QDebug>
#include <QHoverEvent>
#include <QDesktopWidget>
#include <QFile>
#include <QEvent>
#include <QTextStream>
#include <QScrollBar>
#include "../../KUserRegister/checkedlineedit.h"
#include "../../PluginService/sysInfo/kpasswordcheck.h"
#include <QToolTip>

using namespace KInstaller;
namespace KServer {
SetPartitionsSize::SetPartitionsSize(QWidget *parent) : QDialog(parent)
//  ,m_tTip(nullptr)
//  ,pwdIsOK(false)
//  ,cpwdIsOK(false)
{
    this->setMinimumHeight(316);
//    this->setMinimumWidth(480);
    this->setFixedWidth(480);
    this->setObjectName("SetPartitionsSize");
    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Dialog);

//    m_passwordSting = tr("password");
//    m_comfirmPasswdString = tr("confirm password");

    initUI();
    addStyleSheet();
    initAllConnect();
//    addLineditAction();
//    KServer::KPasswordCheck::init();
    translateStr();

    qDebug()<< Q_FUNC_INFO << this->styleSheet();
}

void SetPartitionsSize::initUI()
{
    initCloseButton();
    initSysandDataQScWidget();
    initChangeSizeWidget();
    initButtonWidget();

    m_pMainVLayout = new QVBoxLayout;
    m_pMainVLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainVLayout->setSpacing(0);
    m_pMainVLayout->addWidget(m_pCloseButtonWidget);
    m_pMainVLayout->addItem(new QSpacerItem(2, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_pMainVLayout->addWidget(m_pTatolSizeWidget,0,Qt::AlignCenter);
    m_pMainVLayout->addItem(new QSpacerItem(2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed));

    m_pDeliveLine = new QFrame(this);
    m_pDeliveLine->setFrameShape(QFrame::HLine);
    m_pDeliveLine->setFrameShadow(QFrame::Sunken);
    m_pDeliveLine->setFixedWidth(440);
    m_pDeliveLine->setFixedHeight(2);
    m_pDeliveLine->setStyleSheet("QFrame{background:#434a49;}");//757575
    m_pMainVLayout->addWidget(m_pDeliveLine,0,Qt::AlignCenter);

//QFrame *pDeliveLine1 = new QFrame(this);
//    pDeliveLine1->setFrameShape(QFrame::HLine);
//    pDeliveLine1->setFrameShadow(QFrame::Sunken);
//    pDeliveLine1->setFixedWidth(440);
////    pDeliveLine1->setMaximumHeight(8);
//    pDeliveLine1->setFixedHeight(8);
//    pDeliveLine1->setStyleSheet("QFrame{background:#ff148d;}");//757575
//    m_pMainVLayout->addWidget(pDeliveLine1,0,Qt::AlignCenter);

//QFrame *pDeliveLin2 = new QFrame(this);
//        pDeliveLin2->setFrameShape(QFrame::HLine);
//        pDeliveLin2->setFrameShadow(QFrame::Sunken);
//        pDeliveLin2->setFixedWidth(440);
////        pDeliveLin2->setMaximumHeight(8);
//        pDeliveLin2->setFixedHeight(8);
//        pDeliveLin2->setStyleSheet("QFrame{color:#62ff4a;}");//757575
//        m_pMainVLayout->addWidget(pDeliveLin2,0,Qt::AlignCenter);

//    m_pMainVLayout->addWidget(m_pPasswordPromptWidget);
    m_pMainVLayout->addItem(new QSpacerItem(2, 5, QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_pMainVLayout->addWidget(m_pChangeSizeWidget,0,Qt::AlignCenter);
    m_pMainVLayout->addItem(new QSpacerItem(2, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
//    m_pMainVLayout->addWidget(m_pConfirmTipInfoWidget);
//    m_pMainVLayout->addItem(new QSpacerItem(2, 4, QSizePolicy::Fixed));
//    m_pMainVLayout->addWidget(m_pCheckpwdSafeWidget);
//    m_pMainVLayout->addItem(new QSpacerItem(2, 28, QSizePolicy::Expanding));

    m_pMainVLayout->addWidget(m_pButtonWidget);
    m_pMainVLayout->addItem(new QSpacerItem(2, 24, QSizePolicy::Fixed, QSizePolicy::Fixed));
    this->setLayout(m_pMainVLayout);
}

void SetPartitionsSize::initCloseButton() {
    m_pCloseButtonWidget = new QWidget();
    m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pCloseButtonHLayout = new QHBoxLayout;
    m_pCloseButtonHLayout->setAlignment(Qt::AlignRight);
    m_pCloseButtonHLayout->setContentsMargins(0, 4, 4, 0);
    m_pCloseButtonHLayout->setSpacing(0);

    m_pCloseButton = new QPushButton;
    m_pCloseButton->setObjectName("closeButton");
    m_pCloseButton->setFlat(true);
    m_pCloseButton->setIcon(QIcon(":/res/svg/close.svg"));
    m_pCloseButton->setIconSize(QSize(36, 36));
    m_pCloseButton->setFixedSize(36, 36);
    m_pCloseButton->installEventFilter(this);
    m_pCloseButton->setStyleSheet("QPushButton{border-radius:5px;} QPushButton:hover{background:rgba(255, 255, 255, 0.35);}");//bug220086

    m_pCloseButtonHLayout->addSpacerItem(new QSpacerItem(440, 4, QSizePolicy::Fixed));
    m_pCloseButtonHLayout->addWidget(m_pCloseButton);
    m_pCloseButtonWidget->setLayout(m_pCloseButtonHLayout);
}


void SetPartitionsSize::initSysandDataQScWidget()
{
    m_pTatolSizeWidget = new QWidget();
    m_pTatolSizeWidget->setContentsMargins(0, 0, 0, 0);
    m_pTatolSizeWidget->setFixedWidth(440);

    m_pTatolSizeVLayout = new QVBoxLayout;
    m_pTatolSizeVLayout->setAlignment(Qt::AlignRight);
    m_pTatolSizeVLayout->setContentsMargins(0, 0, 0, 0);
    m_pTatolSizeVLayout->setSpacing(10);

    //总大小描述
    m_pTatolSizeLabel = new QLabel;
    m_pTatolSizeLabel->setObjectName("m_pTatolSizeLabel");
    m_pTatolSizeLabel->setMinimumWidth(332);
    m_pTatolSizeLabel->setWordWrap(true);
//    m_pTatolSizeLabel->setText(m_strDeviceName+"("+m_strDiskPath+") - "+QString::number(m_sizeTotal/1024/1024)+"MiB");
    m_pTatolSizeLabel->setVisible(true);
    m_pTatolSizeLabel->setStyleSheet("color:#ffffff;");
    m_pTatolSizeVLayout->addWidget(m_pTatolSizeLabel);

    //比例条
    m_pSysandDataSizeQB = new QProgressBar;;
    m_pSysandDataSizeQB->setObjectName("m_pSysandDataSizeQB");
//    m_pSysandDataSizeQB->setMaximum(m_sizeTotal/1024/1024);//比例条最大值
//    m_pSysandDataSizeQB->setMinimum(0);//比例条最小值
//    m_pSysandDataSizeQB->setValue(m_sizeDataCurrent/1024/1024);//比例条当前值
    m_pSysandDataSizeQB->setTextVisible(false);
    m_pSysandDataSizeQB->setOrientation(Qt::Horizontal);
    m_pSysandDataSizeQB->setFixedHeight(15);
    m_pSysandDataSizeQB->setStyleSheet("border-radius: 5px;");
    m_pTatolSizeVLayout->addWidget(m_pSysandDataSizeQB);

    //系统分区和数据分区大小描述
    QHBoxLayout *SizeHLayout = new QHBoxLayout;
    SizeHLayout->setAlignment(Qt::AlignRight);
    SizeHLayout->setContentsMargins(0, 0, 0, 0);
    SizeHLayout->setSpacing(8);
    m_pSysSizePic = new QLabel;
    m_pSysSizePic->setObjectName("m_pSysSizePic");
    m_pSysSizePic->setPixmap(QPixmap(":/res/pic/color/#0068B2.png").scaled(12, 12));
    SizeHLayout->addWidget(m_pSysSizePic);
    m_pSysSizeLabel = new QLabel;
    m_pSysSizeLabel->setObjectName("m_pSysSizeLabel");
    m_pSysSizeLabel->setMaximumWidth(180);
    m_pSysSizeLabel->setStyleSheet("color:#ffffff;");
//    m_pSysSizeLabel->setWordWrap(true);
//    QString strT = tr("System Partitions:")+QString::number(m_sizeSystemCurrent/1024/1024)+"MiB";
//    m_pSysSizeLabel->setText(strT);
    m_pSysSizeLabel->setVisible(true);
    SizeHLayout->addWidget(m_pSysSizeLabel);
    SizeHLayout->addSpacing(30);

    m_pDataSizePic = new QLabel;
    m_pDataSizePic->setObjectName("m_pDataSizePic");
    m_pDataSizePic->setPixmap(QPixmap(":/res/pic/color/0_0.png").scaled(12, 12));
    SizeHLayout->addWidget(m_pDataSizePic);
    m_pDataSizeLabel = new QLabel;
    m_pDataSizeLabel->setObjectName("m_pDataSizeLabel");
    m_pDataSizeLabel->setMaximumWidth(180);
    m_pDataSizeLabel->setStyleSheet("color:#ffffff;");
//    m_pDataSizeLabel->setWordWrap(true);
//    strT = tr("Data Partitions:")+QString::number(m_sizeDataCurrent/1024/1024)+"MiB";
//    m_pDataSizeLabel->setText(strT);
    m_pDataSizeLabel->setVisible(true);
    SizeHLayout->addWidget(m_pDataSizeLabel);
    SizeHLayout->addStretch();
    m_pTatolSizeVLayout->addLayout(SizeHLayout);

    m_pTatolSizeWidget->setLayout(m_pTatolSizeVLayout);
}

void SetPartitionsSize::initChangeSizeWidget()
{
    m_pChangeSizeWidget = new QWidget();
    m_pChangeSizeWidget->setContentsMargins(20, 0, 20, 0);
    m_pChangeSizeWidget->setFixedWidth(480);

    m_pChangeSizeGLayout = new QGridLayout;
    m_pChangeSizeGLayout->setAlignment(Qt::AlignRight);
    m_pChangeSizeGLayout->setContentsMargins(0, 0, 0, 0);
    m_pChangeSizeGLayout->setHorizontalSpacing(10);
    m_pChangeSizeGLayout->setVerticalSpacing(0);

    m_pChangeSizeLabel = new QLabel;
    m_pChangeSizeLabel->setObjectName("m_pChangeSizeLabel");
//    m_pChangeSizeLabel->setMaximumWidth(60);
    QString langu = KServer::ReadSettingIni("config", "language");
    if(langu.contains("en_US"))
        m_pChangeSizeLabel->setFixedWidth(90);
    else
        m_pChangeSizeLabel->setFixedWidth(60);
//    m_pChangeSizeLabel->setWordWrap(true);
    m_pChangeSizeLabel->setText(tr("Data Partitions"));//数据分区
    m_pChangeSizeLabel->setVisible(true);
    m_pChangeSizeLabel->setStyleSheet("color:#ffffff;");
    m_pChangeSizeGLayout->addWidget(m_pChangeSizeLabel,0,0);
//    m_pChangeSizeGLayout->addSpacing(10);

//    QVBoxLayout *pQSliderSizeVLayout = new QVBoxLayout;
////    pQSliderSizeVLayout->setAlignment(Qt::AlignRight);
//    pQSliderSizeVLayout->setContentsMargins(0, 0, 0, 0);
//    pQSliderSizeVLayout->setSpacing(0);

    m_pSizeQSlider = new QSlider;
    m_pSizeQSlider->setFixedHeight(30);
    m_pSizeQSlider->setObjectName("m_pSizeQSlider");
//    m_pSizeQSlider->setRange(m_nDataMiniSize/1024/1024,m_nDataMaxSize/1024/1024);
//    m_pSizeQSlider->setValue(m_sizeSystemCurrent/1024/1024);
    m_pSizeQSlider->setOrientation(Qt::Horizontal);
    if(langu.contains("en_US"))
        m_pSizeQSlider->setFixedWidth(240);
    else
        m_pSizeQSlider->setFixedWidth(270);
    m_pSizeQSlider->setStyleSheet("border-radius: 4px;");
//    connect(m_pSizeQSlider, &QAbstractSlider::valueChanged, this, &SetPartitionsSize::SetSysSize);
//    pQSliderSizeVLayout->addWidget(m_pSizeQSlider);
    m_pChangeSizeGLayout->addWidget(m_pSizeQSlider,0,1);
    m_pSizeQSlider->setStyleSheet("QSlider::handle:Horizontal{width: 20px;height: 10px;margin-top:-8px;margin-left:0px;margin-bottom:-8px;margin-right:0px;background: transparent;border-image:url(:/res/svg/sliderhandle.svg);}"
                                  "QSlider::groove:Horizontal{height: 6px;left:0px;right:0px;border:2px;border-radius:2px;background-color:rgb(213, 213, 213);}"
                                  "QSlider:focus{border:2px solid #5BADFF;border-radius: 5px;}"
                                  "QSlider{border:2px solid transparent;}");

    m_pDataSizeRangTit = new QLabel;
    m_pDataSizeRangTit->setObjectName("m_pDataSizeRangTit");
    if(langu.contains("en_US"))
        m_pDataSizeRangTit->setFixedWidth(240);
    else
        m_pDataSizeRangTit->setFixedWidth(270);
//    m_pDataSizeRangTit->setWordWrap(true);
    m_pDataSizeRangTit->setVisible(true);
    m_pDataSizeRangTit->setStyleSheet("color:#ffffff;font: 25 8pt Noto Sans CJK SC;");
//    pQSliderSizeVLayout->addWidget(m_pDataSizeRangTit);
    m_pChangeSizeGLayout->addWidget(m_pDataSizeRangTit,1,1);

//    m_pChangeSizeGLayout->addLayout(pQSliderSizeVLayout);
//    m_pChangeSizeGLayout->addSpacing(16);

    QWidget *SizeLineWidget = new QWidget();
    SizeLineWidget->setContentsMargins(0, 0, 0, 0);
    SizeLineWidget->setFixedSize(90, 36);
    SizeLineWidget->setStyleSheet("color:#ffffff;background:rgba(33,46,54,0.39)");//#383838

    QHBoxLayout *SizeLineHLayout = new QHBoxLayout;
    SizeLineHLayout->setAlignment(Qt::AlignRight);
    SizeLineHLayout->setContentsMargins(0, 0, 0, 0);
    SizeLineHLayout->setSpacing(0);

    m_pSize = new QLineEdit;
    m_pSize->setObjectName("m_pSize");
    m_pSize->setVisible(true);
//    m_pSize->setFixedSize(65, 36);
    m_pSize->setStyleSheet("QLineEdit#m_pSize{color:#FFFFFF;background:rgba(0,0,0,0.25);height:48px;opacity:1;border-radius:6px;}");
//    m_pSize->setText("MiB");
    m_pSize->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_pSize->installEventFilter(this);
    SizeLineHLayout->addWidget(m_pSize);

    m_pUnitLabel = new QLabel;
    m_pUnitLabel->setObjectName("m_pUnitLabel");
//    m_pUnitLabel->setMaximumWidth(100);
    m_pUnitLabel->setWordWrap(false);
    m_pUnitLabel->setText("GB");
    m_pUnitLabel->setVisible(true);
    m_pUnitLabel->setFixedHeight(36);
    m_pUnitLabel->setStyleSheet("color:#ffffff;background:rgba(0,0,0,0.25)");
    SizeLineHLayout->addWidget(m_pUnitLabel);

    SizeLineWidget->setLayout(SizeLineHLayout);
    m_pChangeSizeGLayout->addWidget(SizeLineWidget,0,2);

    m_pChangeSizeWidget->setLayout(m_pChangeSizeGLayout);
}

void SetPartitionsSize::initButtonWidget()
{
    // 取消和确认按钮界面
    m_pButtonWidget = new QWidget();
    m_pButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pButtonHBoxLayout = new QHBoxLayout();
    m_pButtonHBoxLayout->setAlignment(Qt::AlignRight);
    m_pButtonHBoxLayout->setContentsMargins(0, 0, 24, 0);
    m_pButtonHBoxLayout->setSpacing(24);

    m_pCancelButton = new QPushButton;
    m_pCancelButton->setObjectName("cancel");
    m_pCancelButton->setFixedSize(96, 36);
    m_pCancelButton->setEnabled(true);
    m_pCancelButton->setStyleSheet("QPushButton{color:#FFFFFF;background: #4B575D; border-radius:5px;} QPushButton:hover {background: #4BA0F5;} QPushButton:foucs {background: #0C61D5;} QPushButton:pressed {background: #0C61D5;}");

    m_pOkButton = new QPushButton;
    m_pOkButton->setObjectName("OKButton");
    m_pOkButton->setFixedSize(96, 36);
    m_pOkButton->setDefault(true);
    m_pOkButton->setEnabled(true);
    m_pOkButton->setStyleSheet("QPushButton{color:#FFFFFF;background: #1D8EFF; border-radius:5px;} QPushButton:hover {background: #4BA0F5;} QPushButton:foucs {background: #0C61D5;} QPushButton:pressed {background: #0C61D5;}QPushButton:disabled {background: #2D6BAA;color: rgba(255, 255, 255, 0.3);}");

    m_pButtonHBoxLayout->addWidget(m_pCancelButton);
    m_pButtonHBoxLayout->addWidget(m_pOkButton);
    m_pButtonWidget->setLayout(m_pButtonHBoxLayout);
}

void SetPartitionsSize::initAllConnect()
{
    connect(m_pCloseButton, &QPushButton::clicked, [=]{reject();});
    connect(m_pCancelButton, &QPushButton::clicked, [=]{reject();});
    connect(m_pOkButton, &QPushButton::clicked, [=]{
        checkAcceptAble();
    });

    connect(m_pSizeQSlider, &QAbstractSlider::valueChanged, this, &SetPartitionsSize::SetSysSize);
    QObject::connect(m_pSize, &QLineEdit::textEdited, this, &SetPartitionsSize::SaveSysSize);
//    QObject::connect(m_pSize, &QLineEdit::editingFinished, this, &SetPartitionsSize::FinishInputSize);
//    QObject::connect(m_pPasswordCanfirmBox, &QLineEdit::textChanged, this, &SetPartitionsSize::checkComfirmPasswd);
}

void SetPartitionsSize::addStyleSheet()
{
    QFile file(":/res/qss/encryptsetframe.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void SetPartitionsSize::translateStr()
{
    m_sizeSystemCurrent = m_sizeTotal - m_sizeSetting;
    m_pSysandDataSizeQB->setValue(m_sizeSystemCurrent/1024/1024);//比例条当前值MiB
    m_pSizeQSlider->setValue(m_sizeSetting/1024/1024);

    m_pCancelButton->setText(tr("Cancel"));
    m_pOkButton->setText(tr("OK"));
//    markLineEditCorrect(m_LabelState, m_flag);
    m_pTatolSizeLabel->setText(m_strDeviceName+"("+m_strDiskPath+") - "+QString::number(((double)m_sizeTotal)/1024/1024/1024,'f',1)+"GB");

    QString strT = tr("System Partitions:")+QString::number(((double)m_sizeSystemCurrent)/1024/1024/1024,'f',1)+"GB";//系统分区：
    m_pSysSizeLabel->setText(strT);
    strT = tr("Data Partitions:")+QString::number(((double)m_sizeSetting)/1024/1024/1024,'f',1)+"GB";//数据分区：
    m_pDataSizeLabel->setText(strT);

    m_pSize->setText(QString::number(((double)m_sizeSetting)/1024/1024/1024,'f',1));
//    m_pDataSizeRangTit->setText( tr("Size range from ")+QString::number(((double)m_nDataMiniSize)/1024/1024/1024,'f',1)+"GB"+tr(" to ")+QString::number(((double)m_nDataMaxSize)/1024/1024/1024,'f',1)+"GB");
    m_pDataSizeRangTit->setText( tr("Size should be within ")+QString::number(((double)m_nDataMiniSize)/1024/1024/1024,'f',1)+" - "+QString::number(((double)m_nDataMaxSize)/1024/1024/1024,'f',1)+"GB"+tr("etc"));
}

void SetPartitionsSize::resizeEvent(QResizeEvent *event)
{
    QRect screenRect = QApplication::desktop()->screenGeometry();
    this->move(screenRect.width() / 2 - this->width() / 2, screenRect.height() / 2 - this->height() / 2);
}

void SetPartitionsSize::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush(QColor("#212E36")));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() );
    rect.setHeight(rect.height() );
    painter.drawRoundedRect(rect, 12, 12);
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, 12, 12);
    painter.drawPath(painterPath);
    QDialog::paintEvent(event);
}

void SetPartitionsSize::SetSysSize(int nValue)
{
//    m_pSize->setText(QString::number(((double)nValue)/1024,'f',1));
    if(bInputBegin)
    {
        qint64 nT = (qint64)nValue*1024*1024;
        if(nValue <= m_pSizeQSlider->minimum())//防止滑块值溢出 bug228788
            m_sizeSetting = m_nDataMiniSize;
        else if(nValue >= m_pSizeQSlider->maximum())
            m_sizeSetting = m_nDataMaxSize;
        else
            m_sizeSetting = nT;

        translateStr();
        checkInput(m_sizeSetting);
    }
}

//void SetPartitionsSize::FinishInputSize()
//{
//    SaveSysSize(m_pSize->text());
//}

void SetPartitionsSize::SaveSysSize(QString strSize)
{
    if(bInputBegin)
    {
        qint64 qT = (qint64)(strSize.toDouble()*1024*1024*1024);
        if(qT >= m_nDataMiniSize && qT <= m_nDataMaxSize){
            m_sizeSetting = qT;
            translateStr();
        }
        checkInput(qT);
    }
}

void SetPartitionsSize::showEvent(QShowEvent *event)
{
    m_pSizeQSlider->setFocus();
}

void SetPartitionsSize::getDiskInfo(QString strDiskName, QString strDiskPath, qint64 nTatolSize, qint64 nDataSizeOri, qint64 nDataSizeSetted, qint64 nDataMiniSize, qint64 nDataMaxSize)
{
    if(strDiskName.isEmpty() || strDiskPath.isEmpty() || nTatolSize<=0 || nDataSizeOri>=nTatolSize || nDataMaxSize<0 || nDataMiniSize>nDataMaxSize)
    {
        qDebug()<< Q_FUNC_INFO <<" parameters error!";
        return;
    }
    m_strDeviceName = strDiskName;
    m_strDiskPath = strDiskPath;
    m_sizeTotal = nTatolSize;
    m_sizeDataCurrent = nDataSizeOri;
    if(0 >= nDataSizeSetted)
    {
        m_sizeSetting = m_sizeDataCurrent;
    }
    else
    {
        m_sizeSetting = nDataSizeSetted;
    }

    m_sizeSystemCurrent = nTatolSize - m_sizeSetting;
    m_nDataMiniSize = nDataMiniSize;
    m_nDataMaxSize = nDataMaxSize;
    m_pSysandDataSizeQB->setMaximum(m_sizeTotal/1024/1024);//比例条最大值MiB
    m_pSysandDataSizeQB->setMinimum(0);//比例条最小值

    m_pSizeQSlider->setRange(m_nDataMiniSize/1024/1024,m_nDataMaxSize/1024/1024);
    m_pSizeQSlider->setSingleStep(m_nDataMaxSize/1024/1024/10);
    translateStr();

    bInputBegin = true;
    m_acceptAble = true;
}

bool SetPartitionsSize::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "m_pSize") {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                qint64 qT = (qint64)(m_pSize->text().toDouble()*1024*1024*1024);
                if(qT >= m_nDataMiniSize && qT <= m_nDataMaxSize){
                    m_sizeSetting = qT;
                    translateStr();
                    checkInput(m_sizeSetting);
                    checkAcceptAble();
                }
                else{
                    m_pSize->setText(QString::number(((double)m_sizeSetting)/1024/1024/1024,'f',1));
                    checkInput(m_sizeSetting);
                }
                return true;
            } else if (keyEvent->key() == Qt::Key_Backspace) {
                m_pSize->clear();
//                return true;
            }
        }
    }

    if(object->objectName() == "closeButton") {
        if(event->type() == QEvent::Enter) {
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget(true, this);
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));
	    
	    m_tTip->move(x() + width() - m_pCloseButton->width() - 13, y() + m_pCloseButton->height() + 9);
            m_tTip->show();
            connect(m_pCloseButton, &QPushButton::clicked, [=]{m_tTip->close();});
        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }

    return QWidget::eventFilter(object,event);
}

void SetPartitionsSize::keyPressEvent(QKeyEvent *event)
{
//    qDebug() << Q_FUNC_INFO ;
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        checkInput(m_sizeSetting);
        checkAcceptAble();
    }
    else
        QWidget::keyPressEvent(event);
}

void SetPartitionsSize::checkInput(qint64 nSizeNow)
{
    if(nSizeNow >= m_nDataMiniSize && nSizeNow <= m_nDataMaxSize) {
        m_acceptAble = true;
        m_pDataSizeRangTit->setStyleSheet("color:#ffffff;font: 25 8pt Noto Sans CJK SC;");
        m_pOkButton->setEnabled(true);
    }
    else{
        m_acceptAble = false;
        m_pDataSizeRangTit->setStyleSheet("color:#FB9140;font: 57 8pt Noto Sans CJK SC Medium;");//橘色字体提示
        m_pOkButton->setEnabled(false);
    }
}

void SetPartitionsSize::checkAcceptAble()
{
    if(m_acceptAble) {
        accept();
    }
}

}

