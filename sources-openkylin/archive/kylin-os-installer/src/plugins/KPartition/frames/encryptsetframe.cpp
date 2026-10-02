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

#include "encryptsetframe.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDebug>
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
#include "../../KUserRegister/checkedlineedit.h"
#include "../../PluginService/sysInfo/kpasswordcheck.h"
#include <QToolTip>
#include <QLine>
#include <QScrollBar>
#include "../PluginService/kcommand.h"

using namespace KInstaller;
namespace KServer {
EncryptSetFrame::EncryptSetFrame(QWidget *parent) : QDialog(parent)
  ,m_tTip(nullptr)
  ,pwdIsOK(false)
  ,cpwdIsOK(false)
{
   // this->setMinimumHeight(350);
//    this->setMinimumWidth(480);
//    this->setFixedWidth(480);
    this->setObjectName("encryptsetframe");
    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Dialog);

    m_passwordSting = tr("password");
    m_comfirmPasswdString = tr("confirm password");

    m_isTPMEncrypt = getTPMHarddisk();

    initUI();
    addStyleSheet();
    initAllConnect();
 	initFocusPolicy();
    addLineditAction();
    KServer::KPasswordCheck::init();
    translateStr();
    qDebug()<< Q_FUNC_INFO << this->styleSheet();

    m_pPasswordBox->setFocus();
    if(m_isTPMEncrypt){
        setSoftEncryptStatus(false);
	m_pOkButton->setFocus();
    }
}

void EncryptSetFrame::initUI()
{
    initTPMEncryptWidget();
    initCloseButton();
    initPasswdWidget();
    initConmfirmPasswdWidget();
    initCheckpwdSafeWidget();
    initButtonWidget();

    m_pMainLayout = new QVBoxLayout;
    m_pMainLayout->setContentsMargins(24, 12, 12, 24);
    m_pMainLayout->setSpacing(0);
    m_pMainLayout->addWidget(m_pCloseButtonWidget,Qt::AlignRight);
    m_pMainLayout->addWidget(m_pTPMEncryptWidget);

    QFrame *pLine = new QFrame;
    pLine->setFrameShape(QFrame::HLine);
    pLine->setFixedHeight(1);
    pLine->setStyleSheet("color:#4b4b4b;");// 757575
    m_pMainLayout->addWidget(pLine);

    m_pSoftEncrypt = new QRadioButton(tr("Software Encryption"));//软件加密
    m_pSoftEncrypt->setAutoExclusive(false);
    m_pSoftEncrypt->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    if(!m_isTPMEncrypt)
    {
        m_pSoftEncrypt->setCheckable(true);
        m_pSoftEncrypt->setChecked(true);
        m_pSoftEncrypt->setEnabled(false);
    }
    m_pMainLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Expanding));
    m_pMainLayout->addWidget(m_pSoftEncrypt);
    m_pMainLayout->addItem(new QSpacerItem(2, 5, QSizePolicy::Fixed));
    m_pMainLayout->addWidget(m_pCheckpwdSafeWidget);
    m_pMainLayout->addItem(new QSpacerItem(2, 10, QSizePolicy::Fixed));

    QGridLayout *pSoftInLayout = new QGridLayout;
    pSoftInLayout->setContentsMargins(0, 0, 0, 0);
    pSoftInLayout->setSpacing(0);
    pSoftInLayout->setAlignment(Qt::AlignLeft);

    pSoftInLayout->addItem(new QSpacerItem(22, 6, QSizePolicy::Fixed),0,0);
    pSoftInLayout->addWidget(m_pPasswdWidget,0,1);
    
    pSoftInLayout->addItem(new QSpacerItem(2, 2, QSizePolicy::Fixed),1,0);
    pSoftInLayout->addWidget(m_pPasswordPromptWidget,1,1);
    
    pSoftInLayout->addItem(new QSpacerItem(2, 6, QSizePolicy::Fixed),2,0);
    pSoftInLayout->addWidget(m_pConfirmPasswdWidget,2,1);
    
    pSoftInLayout->addItem(new QSpacerItem(2, 2, QSizePolicy::Fixed),3,0);
    pSoftInLayout->addWidget(m_pConfirmTipInfoWidget,3,1);

    m_pMainLayout->addLayout(pSoftInLayout);
    m_pMainLayout->addWidget(m_pButtonWidget,Qt::AlignRight);
    this->setLayout(m_pMainLayout);
}

// 硬件加密
void EncryptSetFrame::initTPMEncryptWidget()
{
    m_pTPMEncryptWidget = new QWidget();
    m_pTPMEncryptWidget->setContentsMargins(0, 0, 0, 0);

    m_pTPMEncryptLayout = new QVBoxLayout();
    m_pTPMEncryptLayout->setContentsMargins(0, 0, 0, 0);
    m_pTPMEncryptLayout->setSpacing(0);
    m_pTPMEncryptLayout->setAlignment(Qt::AlignLeft);

    m_pTPMEncrypt = new QRadioButton(tr("TPM Full Disk Encryption"));//可信芯片全盘加密
    m_pTPMEncrypt->setAutoExclusive(false);
    if(m_isTPMEncrypt)
    {
        m_pTPMEncrypt->setCheckable(true);
        m_pTPMEncrypt->setChecked(true);
    }
    else
    {
        m_pTPMEncrypt->setChecked(false);
        m_pTPMEncrypt->setCheckable(false);
        m_pTPMEncrypt->setEnabled(false);
    }

    QHBoxLayout* pLabelLayout = new QHBoxLayout();
    pLabelLayout->setContentsMargins(0, 0, 0, 0);
    pLabelLayout->setSpacing(0);
    pLabelLayout->setAlignment(Qt::AlignLeft);

    m_pTPMEncryptLabel = new QLabel;
    m_pTPMEncryptLabel->setObjectName("TPMEncryptLabel");
    m_pTPMEncryptLabel->setMinimumWidth(400);
    m_pTPMEncryptLabel->setWordWrap(true);
    m_pTPMEncryptLabel->setText(tr("Please backup the secret key to the USB drive as soon as possible after entering the system"));  //进入系统后请尽快将秘钥备份至U盘

    QLabel *pSvgLabel = new QLabel;
    pSvgLabel->setPixmap(QPixmap(QIcon(":/res/svg/finger.svg").pixmap(QSize(24, 24))));   //添加svg图片，并设置其大小
//    m_pCheckpwdSafeHBoxLayout->addItem(new QSpacerItem(22, 10, QSizePolicy::Fixed));
//    m_pCheckpwdSafeHBoxLayout->addWidget(pSvgLabel);

    pLabelLayout->addItem(new QSpacerItem(22, 10, QSizePolicy::Fixed));
    pLabelLayout->addWidget(pSvgLabel);
    pLabelLayout->addItem(new QSpacerItem(8, 10, QSizePolicy::Fixed));
    pLabelLayout->addWidget(m_pTPMEncryptLabel);

//    m_pTPMEncryptLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Expanding));
    m_pTPMEncryptLayout->addWidget(m_pTPMEncrypt);
    m_pTPMEncryptLayout->addItem(new QSpacerItem(8, 5, QSizePolicy::Fixed));
    m_pTPMEncryptLayout->addLayout(pLabelLayout);
    m_pTPMEncryptLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
    m_pTPMEncryptWidget->setLayout(m_pTPMEncryptLayout);
}

void EncryptSetFrame::initCloseButton() {
    m_pCloseButtonWidget = new QWidget();
    m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pCloseButtonHLayout = new QHBoxLayout;
    m_pCloseButtonHLayout->setAlignment(Qt::AlignRight);
    m_pCloseButtonHLayout->setContentsMargins(0, 0, 0, 0);
    m_pCloseButtonHLayout->setSpacing(0);

    m_pCloseButton = new QPushButton;
    m_pCloseButton->setObjectName("closeButton");
    m_pCloseButton->setFlat(true);
    m_pCloseButton->setIcon(QIcon(":/res/svg/close.svg"));
    m_pCloseButton->setIconSize(QSize(36, 36));
    m_pCloseButton->installEventFilter(this);

//    m_pCloseButtonHLayout->addSpacerItem(new QSpacerItem(440, 4, QSizePolicy::Fixed));
    m_pCloseButtonHLayout->addWidget(m_pCloseButton);
    m_pCloseButtonWidget->setLayout(m_pCloseButtonHLayout);
}

void EncryptSetFrame::initPasswdWidget()
{
    // 密码label和输入框
    m_pPasswdWidget = new QWidget();
    m_pPasswdWidget->setMinimumWidth(480);
//    m_pPasswdWidget->setStyleSheet("border: 2px solid rgba(255,255,255,0.0784);");
    m_pPasswdWidget->setContentsMargins(0, 0, 0, 0);

    m_pPassWdHLayout = new QHBoxLayout();
    m_pPassWdHLayout->setContentsMargins(0, 0, 0, 0);
    m_pPassWdHLayout->setSpacing(0);

    m_pPasswdLabel = new QLabel();
//    m_pPasswdLabel->setFixedWidth(56);
//    m_passwordSting = tr("password");
   
    m_pPasswdLabel->setToolTip(m_passwordSting);
    m_pPasswdLabel->setText(SetFormatBody(m_passwordSting, m_pPasswdLabel));
    m_pPasswdLabel->setObjectName("passwordLabel");
    m_pPasswdLabel->setStyleSheet("QToolTip{ \
                                         background: rgba(255,255,255,0.39);\
                                         filter: blur(32px);\
                                         opacity: 1;\
                                         border-radius: 4px; \
                                         font-size: 16px; \
                                         font-family: Noto Sans CJK SC; \
                                         font-weight: 400; \
                                         line-height: 35px; \
                                         color: #000000; \
                                         letter-spacing: 50px;}");
    m_pPasswdLabel->setFixedWidth(GetFormatWidth(m_pPasswdLabel)+15);

    // LineEdit字段
    m_pPasswordBox = new QLineEdit();
    m_pPasswordBox->setObjectName("passwdLineEdit");
   // m_pPasswordBox->setVisible(true);
//    m_pPasswordBox->setFixedSize(352, 36);
    m_pPasswordBox->setFixedHeight(36);
    
    m_pPasswordBox->setFixedWidth(520-48-36-m_pPasswdLabel->width());
    m_pPasswordBox->setEchoMode(QLineEdit::Password);
    m_pPasswordBox->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

//    m_pPassWdHLayout->addItem(new QSpacerItem(24, 40, QSizePolicy::Fixed));
    m_pPassWdHLayout->addWidget(m_pPasswdLabel, 0, Qt::AlignCenter);
    m_pPassWdHLayout->addItem(new QSpacerItem(24, 2, QSizePolicy::Fixed));

    m_pPassWdHLayout->addWidget(m_pPasswordBox);
    m_pPassWdHLayout->addItem(new QSpacerItem(24, 66, QSizePolicy::Fixed));
    m_pPasswdWidget->setLayout(m_pPassWdHLayout);

    // 中间提示密码种类不符合要求字段
    m_pPasswordPromptWidget = new QWidget();
    m_pPasswordPromptWidget->setContentsMargins(0, 0, 0, 0);

    m_pPasswordPromptHBoxLayout = new QHBoxLayout();
    m_pPasswordPromptHBoxLayout->setAlignment(Qt::AlignLeft);
    m_pPasswordPromptHBoxLayout->setContentsMargins(0, 0, 24, 0);
    m_pPasswordPromptHBoxLayout->setSpacing(0);

    m_pPasswordPrompt = new QLabel;
    m_pPasswordPrompt->setObjectName("passwordPrompt");
    m_pPasswordPrompt->setMinimumWidth(m_pPasswordBox->width());
    m_pPasswordPrompt->setWordWrap(true);
    m_pPasswordPrompt->setText(tr("The password contains less than two types of characters"));
    m_pPasswordPrompt->setVisible(false);

    m_pPasswordPromptHBoxLayout->addItem(new QSpacerItem(520-m_pPasswordBox->width()-12-46, 6, QSizePolicy::Expanding));
    m_pPasswordPromptHBoxLayout->addWidget(m_pPasswordPrompt);
    m_pPasswordPromptWidget->setLayout(m_pPasswordPromptHBoxLayout);
}

void EncryptSetFrame::initConmfirmPasswdWidget()
{
    // 确认密码Label和输入框
    m_pConfirmPasswdWidget = new QWidget();
    m_pConfirmPasswdWidget->setMinimumWidth(480);
    m_pConfirmPasswdWidget->setContentsMargins(0, 0, 0, 0);

    m_pConfirmPasswdLayout = new QHBoxLayout();
    m_pConfirmPasswdLayout->setContentsMargins(0, 0, 0, 0);
    m_pConfirmPasswdLayout->setSpacing(0);


    m_pConfirmPasswdLabel = new QLabel();
//    m_pConfirmPasswdLabel->setFixedWidth(56);
//    m_pConfirmPasswdLabel->setMinimumWidth(56);
//    m_comfirmPasswdString = tr("confirm password");
    m_pConfirmPasswdLabel->setToolTip(m_comfirmPasswdString);
    m_pConfirmPasswdLabel->setText(SetFormatBody(m_comfirmPasswdString, m_pConfirmPasswdLabel));
    m_pConfirmPasswdLabel->setObjectName("confirmpasswordLabel");

    m_pConfirmPasswdLabel->setStyleSheet("QToolTip{ \
                                         background: rgba(255,255,255,0.39);\
                                         filter: blur(32px);\
                                         opacity: 1;\
                                         border-radius: 4px; \
                                         font-size: 16px; \
                                         font-family: Noto Sans CJK SC; \
                                         font-weight: 400; \
                                         line-height: 35px; \
                                         color: #000000; \
                                         letter-spacing: 50px; }");
//                                         text-align: center; }");
    m_pConfirmPasswdLabel->setFixedWidth(GetFormatWidth(m_pConfirmPasswdLabel)+15);

    m_pPasswordCanfirmBox = new QLineEdit();
    m_pPasswordCanfirmBox->setObjectName("confirmpasswdLineEdit");
//    m_pPasswordCanfirmBox->setFixedSize(352, 36);
    m_pPasswordCanfirmBox->setFixedHeight(36);
    m_pPasswordCanfirmBox->setFixedWidth(520-48-36-m_pConfirmPasswdLabel->width());
    m_pPasswordCanfirmBox->setEchoMode(QLineEdit::Password);
    m_pPasswordCanfirmBox->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

//    m_pConfirmPasswdLayout->addItem(new QSpacerItem(24, 2, QSizePolicy::Fixed));
    m_pConfirmPasswdLayout->addWidget(m_pConfirmPasswdLabel, 0, Qt::AlignCenter);
    m_pConfirmPasswdLayout->addItem(new QSpacerItem(24, 2, QSizePolicy::Fixed));

    m_pConfirmPasswdLayout->addWidget(m_pPasswordCanfirmBox);
    m_pConfirmPasswdLayout->addItem(new QSpacerItem(24, 66, QSizePolicy::Fixed));
    m_pConfirmPasswdWidget->setLayout(m_pConfirmPasswdLayout);

    // 两次密码不一致提示字符串
    m_pConfirmTipInfoWidget = new QWidget();
    m_pConfirmTipInfoWidget->setContentsMargins(0, 0, 0, 0);

    m_pConfirmTipInfoHBoxLayout = new QHBoxLayout();
    m_pConfirmTipInfoHBoxLayout->setAlignment(Qt::AlignLeft);
    m_pConfirmTipInfoHBoxLayout->setContentsMargins(0, 0, 24, 0);
    m_pConfirmTipInfoHBoxLayout->setSpacing(0);

    m_pConfirmTipInfoLabel = new QLabel();
    m_pConfirmTipInfoLabel->setFixedWidth(m_pPasswordCanfirmBox->width());
    m_pConfirmTipInfoLabel->setObjectName("confirmTipInfoLabel");
    m_pConfirmTipInfoLabel->setText(tr("The two passwords are different"));
    m_pConfirmTipInfoLabel->setVisible(false);

    m_pConfirmTipInfoHBoxLayout->addItem(new QSpacerItem(520-m_pPasswordCanfirmBox->width()-12-46, 10, QSizePolicy::Expanding));
    m_pConfirmTipInfoHBoxLayout->addWidget(m_pConfirmTipInfoLabel);
//    m_pConfirmTipInfoHBoxLayout->addItem(new QSpacerItem(192, 10, QSizePolicy::Expanding));
    m_pConfirmTipInfoWidget->setLayout(m_pConfirmTipInfoHBoxLayout);
}

// 提示妥善保管密码
void EncryptSetFrame::initCheckpwdSafeWidget()
{
    m_pCheckpwdSafeWidget = new QWidget();
//    m_pCheckpwdSafeWidget->setStyleSheet("border: 2px solid rgba(255,255,255,0.0784);");
    m_pCheckpwdSafeWidget->setContentsMargins(0, 0, 0, 0);

    m_pCheckpwdSafeHBoxLayout = new QHBoxLayout();
    m_pCheckpwdSafeHBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_pCheckpwdSafeHBoxLayout->setSpacing(0);
    m_pCheckpwdSafeHBoxLayout->setAlignment(Qt::AlignLeft);

    m_pCheckpwdSafeLabel = new QLabel;
    m_pCheckpwdSafeLabel->setObjectName("CheckpwdSafeLabel");

    m_pCheckpwdSafeLabel->setMinimumWidth(400);
    m_pCheckpwdSafeLabel->setWordWrap(true);
    m_pCheckpwdSafeLabel->setText(tr("Please keep your password properly.If you forget it,you will not be able to access the disk data."));  //请妥善保管您的密码，如忘记密码，将无法访问磁盘数据.
    m_pCheckpwdSafeSvgLabel = new QLabel;

    m_pCheckpwdSafeSvgLabel->setPixmap(QPixmap(QIcon(":/res/svg/finger.svg").pixmap(QSize(24, 24))));   //添加svg图片，并设置其大小
    m_pCheckpwdSafeHBoxLayout->addItem(new QSpacerItem(22, 10, QSizePolicy::Fixed));
    m_pCheckpwdSafeHBoxLayout->addWidget(m_pCheckpwdSafeSvgLabel);
    m_pCheckpwdSafeHBoxLayout->addItem(new QSpacerItem(8, 10, QSizePolicy::Fixed));
    m_pCheckpwdSafeHBoxLayout->addWidget(m_pCheckpwdSafeLabel);
    m_pCheckpwdSafeHBoxLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Expanding));
    m_pCheckpwdSafeWidget->setLayout(m_pCheckpwdSafeHBoxLayout);
}

void EncryptSetFrame::initButtonWidget()
{
    // 取消和确认按钮界面
    m_pButtonWidget = new QWidget();
    m_pButtonWidget->setContentsMargins(0, 0, 0, 0);

    m_pButtonHBoxLayout = new QHBoxLayout();
    m_pButtonHBoxLayout->setAlignment(Qt::AlignRight);
    m_pButtonHBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_pButtonHBoxLayout->setSpacing(15);

    m_pCancelButton = new QPushButton;
    m_pCancelButton->setObjectName("cancel");
    m_pCancelButton->setFixedSize(96, 36);

    m_pOkButton = new QPushButton;
    m_pOkButton->setObjectName("OKButton");
    m_pOkButton->setFixedSize(96, 36);
    m_pOkButton->setDefault(true);
    m_pOkButton->setDisabled(false);

    m_pButtonHBoxLayout->addWidget(m_pCancelButton);
    m_pButtonHBoxLayout->addWidget(m_pOkButton);
    m_pButtonHBoxLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed));
    m_pButtonWidget->setLayout(m_pButtonHBoxLayout);
}

QString EncryptSetFrame::SetFormatBody(QString text, QLabel *label)
{
    QFontMetrics fontMetrics(label->font());
    int LableWidth = label->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if (fontSize > LableWidth - 3) {
        formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
        return formatBody;
    }
    return text;
}

int EncryptSetFrame::GetFormatWidth(QLabel *label)
{
    QFontMetrics fontMetrics(label->font());
    int fontSize1 = fontMetrics.width(m_passwordSting);
    int fontSize2 = fontMetrics.width(m_comfirmPasswdString);
    return (fontSize1>fontSize2)?fontSize1:fontSize2;
}

void EncryptSetFrame::initAllConnect()
{
    connect(m_pCloseButton, &QPushButton::clicked, [=]{execid = 1;reject();});
    connect(m_pCancelButton, &QPushButton::clicked, [=]{ execid = 1;
        reject();
    });
    connect(m_pOkButton, &QPushButton::clicked, [=]{
        if( (m_pTPMEncrypt->isChecked()) ||
            ((m_pSoftEncrypt->isChecked())& (pwdIsOK & cpwdIsOK) )
          )
        {
            execid = 1;
            accept();
        }
    });

    QObject::connect(m_pPasswordBox, &QLineEdit::textChanged, this, &EncryptSetFrame::checkPasswd);
    QObject::connect(m_pPasswordCanfirmBox, &QLineEdit::textChanged, this, &EncryptSetFrame::checkComfirmPasswd);

    if(m_isTPMEncrypt)
    {
        connect(m_pTPMEncrypt, &QRadioButton::clicked, this, &EncryptSetFrame::checkTPMEncrypt);
        connect(m_pSoftEncrypt, &QRadioButton::clicked, this, &EncryptSetFrame::checkSoftEncrypt);
    }
}

void EncryptSetFrame::checkTPMEncrypt(bool checked)
{
    if(m_pTPMEncrypt->isChecked())
    {
        m_pTPMEncrypt->setChecked(true);
        setSoftEncryptStatus(false);
        m_pOkButton->setDisabled(false);
    }
    else
    {
        m_pTPMEncrypt->setChecked(false);
        setSoftEncryptStatus(true);
    }
}

void EncryptSetFrame::checkSoftEncrypt(bool checked)
{
    if(m_isTPMEncrypt)
    {
        if(m_pSoftEncrypt->isChecked())
        {
            m_pTPMEncrypt->setChecked(false);
//            m_pSoftEncrypt->setChecked(true);
            setSoftEncryptStatus(true);
        }
        else
        {
             m_pTPMEncrypt->setChecked(true);
//             m_pSoftEncrypt->setChecked(false);
             setSoftEncryptStatus(false);
             m_pOkButton->setDisabled(false);
        }
    }
}

void EncryptSetFrame::showEvent(QShowEvent *event)
{
    m_pTPMEncryptWidget->setFocus();
    update();
}

void EncryptSetFrame::addStyleSheet()
{
    QFile file(":/res/qss/encryptsetframe.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}


void EncryptSetFrame::translateStr()
{
    m_pCancelButton->setText(tr("Cancel"));
    m_pOkButton->setText(tr("OK"));

}

void EncryptSetFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
//      m_pMainLayout->update();
    } else {
        QWidget::changeEvent(event);
    }
}

void EncryptSetFrame::resizeEvent(QResizeEvent *event)
{
    QRect screenRect = QApplication::desktop()->screenGeometry();
    this->move(screenRect.width() / 2 - this->width() / 2, screenRect.height() / 2 - this->height() / 2);
}

void EncryptSetFrame::setMessageInfo(QString info)
{
    m_pCheckpwdSafeLabel->setText(info);
    m_pCheckpwdSafeLabel->adjustSize();
}
void EncryptSetFrame::setCancleHidden()
{
    m_pCancelButton->setHidden(true);
    m_pCancelButton->setEnabled(false);
}

void EncryptSetFrame::paintEvent(QPaintEvent *event)
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

void EncryptSetFrame::addLineditAction()
{
    m_pPasswdAction = new QAction();
    m_pPasswdAction->setIcon(QIcon(":/res/svg/code.svg"));
    m_pPasswordBox->addAction(m_pPasswdAction, QLineEdit::TrailingPosition);
    connect(m_pPasswdAction, &QAction::triggered, this, &EncryptSetFrame::setActionIcon);

    m_ppasswdComfirmAction = new QAction();
    m_ppasswdComfirmAction->setIcon(QIcon(":/res/svg/code.svg"));
    m_pPasswordCanfirmBox->addAction(m_ppasswdComfirmAction, QLineEdit::TrailingPosition);
    connect(m_ppasswdComfirmAction, &QAction::triggered, this, &EncryptSetFrame::setComfirmActionIcon);
}

bool EncryptSetFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "closeButton") {
        if(event->type() == QEvent::Enter) {
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget(true, this);
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));
            m_tTip->move(x()+width()-m_pCloseButton->width()-13, y()+m_pCloseButton->height()+15);
            m_tTip->show();
            connect(m_pCloseButton, &QPushButton::clicked, [=]{m_tTip->close();});

        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void EncryptSetFrame::markLineEditCorrect(LabelState state, bool flag)
{
    switch (state) {
    case LabelState::Pwd:
	if(m_pPasswordBox->text().isEmpty()==true) {
		break;
	}
        if(flag) {
            m_pPasswordBox->setStyleSheet("QLineEdit{border: 0px solid #5BADFF; }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
        } else {
            m_pPasswordBox->setStyleSheet("QLineEdit{border-color: #FD625E }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
        }
        m_pPasswordBox->style()->unpolish(m_pPasswordBox);
        m_pPasswordBox->style()->polish(m_pPasswordBox);
        break;
    case LabelState::CPwd:
        if(flag) {
            m_pPasswordCanfirmBox->setStyleSheet("QLineEdit{border: 0px solid #5BADFF; }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
        } else {
            m_pPasswordCanfirmBox->setStyleSheet("QLineEdit{border-color: #FD625E }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
        }
        m_pPasswordCanfirmBox->style()->unpolish(m_pPasswordCanfirmBox);
        m_pPasswordCanfirmBox->style()->polish(m_pPasswordCanfirmBox);
        break;
    default:

        break;
    }

    if(m_pSoftEncrypt->isChecked())
    {
        m_pOkButton->setEnabled(pwdIsOK & cpwdIsOK);
	if(pwdIsOK & cpwdIsOK)
	{
		m_pOkButton->setFocus();
	}
    }
	update();
    QApplication::processEvents(QEventLoop::AllEvents,100);
}


void EncryptSetFrame::checkPasswd(QString pwd)
{
    m_LabelState = LabelState::Pwd;
    pwd = m_pPasswordBox->text();
    m_state = KCheckPassWD(pwd, errstr, "");
    if(pwd.isEmpty()) {
        m_pPasswordPrompt->setVisible(false);
        pwdIsOK = false;
        m_flag = true;
    } else if(m_state == CheckState::OK) {
        m_pPasswordPrompt->setVisible(false);
        pwdIsOK = true;
        m_flag = true;
    } else {
        m_pPasswordPrompt->setText(errstr);
        m_pPasswordPrompt->setVisible(true);
        pwdIsOK = false;
        m_flag = false;
    }
    markLineEditCorrect(m_LabelState, m_flag);

    if (m_pPasswordCanfirmBox->text().isEmpty())
    {
        cpwdIsOK = false;
    }
    else
        checkComfirmPasswd(m_pPasswordCanfirmBox->text());

    //translateStr();
}

void EncryptSetFrame::checkComfirmPasswd(QString comfirmpwd)
{
    QString pwd = m_pPasswordBox->text();
    m_LabelState = LabelState::CPwd;

    if (comfirmpwd.isEmpty()==true) {
        cpwdIsOK = false;
        m_flag = true;
        if(pwd.isEmpty())
            m_pConfirmTipInfoLabel->setVisible(false);
        else
            m_pConfirmTipInfoLabel->setVisible(true);
    } else if (comfirmpwd.compare(pwd)==0) {
        m_pConfirmTipInfoLabel->setVisible(false);
        cpwdIsOK = true;
        m_flag = true;
    } else {
        m_pConfirmTipInfoLabel->setVisible(true);
        cpwdIsOK = false;
        m_flag = false;
    }
    markLineEditCorrect(m_LabelState, m_flag);
}

void EncryptSetFrame::setActionIcon()
{
    if(!m_bPassWdAction) {
        m_pPasswdAction->setIcon(QIcon(":/res/svg/plaincode.svg"));
        m_pPasswordBox->setEchoMode(QLineEdit::Normal);
        m_bPassWdAction = true;
    } else {
        m_pPasswdAction->setIcon(QIcon(":/res/svg/code.svg"));
        m_pPasswordBox->setEchoMode(QLineEdit::Password);
        m_bPassWdAction = false;
    }
}

void EncryptSetFrame::setComfirmActionIcon()
{
    if(!m_bPassWdConfirmAction) {
        m_ppasswdComfirmAction->setIcon(QIcon(":/res/svg/plaincode.svg"));
        m_pPasswordCanfirmBox->setEchoMode(QLineEdit::Normal);
        m_bPassWdConfirmAction = true;
    } else {
        m_ppasswdComfirmAction->setIcon(QIcon(":/res/svg/code.svg"));   // 不能看密码
        m_pPasswordCanfirmBox->setEchoMode(QLineEdit::Password);
        m_bPassWdConfirmAction = false;
    }
}

bool EncryptSetFrame::getTPMHarddisk()
{
    QString error = "";
    QString output = "";
    QString path = "";
    int excid = -1;
    QStringList args;
    args.append("-c");
    args.append("ls /dev/tpm*");
    bool isok = KServer::KCommand::getInstance()->RunScripCommand("sh", args, path, output, error, excid);
    if(isok)
        //return true;
        return false;
    else
        return false;
}

void EncryptSetFrame::setSoftEncryptStatus(bool checked)
{
    m_pSoftEncrypt->setChecked(checked);
    m_pPasswordBox->setDisabled(!checked);
    m_pPasswordCanfirmBox->setDisabled(!checked);
}

void EncryptSetFrame::initFocusPolicy()
{
    this->setTabOrder(m_pTPMEncrypt, m_pSoftEncrypt);
    this->setTabOrder(m_pSoftEncrypt, m_pPasswordBox);
    this->setTabOrder(m_pPasswordBox, m_pPasswordPrompt);
    this->setTabOrder(m_pPasswordPrompt, m_pCancelButton);
    this->setTabOrder(m_pCancelButton, m_pOkButton);
    this->setTabOrder(m_pOkButton, m_pCloseButton);
}

}

