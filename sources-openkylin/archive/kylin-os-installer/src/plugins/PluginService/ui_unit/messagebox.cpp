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

#include "messagebox.h"
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
#include <QPainterPath>
#include <QKeyEvent>
#include "../../KUserRegister/checkedlineedit.h"
#include "../sysInfo/kpasswordcheck.h"
using namespace KInstaller;
namespace KServer {
MessageBox::MessageBox(QWidget *parent) : QDialog(parent)
  ,m_tTip(nullptr)
{
    this->setFixedWidth(520);
    this->setMinimumHeight(164);
    this->setObjectName("MessageBox");
    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
//                   Qt::WindowDoesNotAcceptFocus |
                   Qt::Dialog);
    initUI();
    addStyleSheet();
    initAllConnect();
    translateStr();
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
//    this->setStyleSheet("QFrame{border-radius: 10px;}");
    qDebug()<< Q_FUNC_INFO << this->styleSheet();

}

void MessageBox::initUI()
{
    m_pmainVlayout = new QVBoxLayout;
    m_pmainVlayout->setSpacing(0);
    m_pmainVlayout->setContentsMargins(24, 0, 0, 24);

    /*****   关闭窗口按钮    *****/
    m_pCloseButtonWidget = new QWidget();
    m_pCloseButtonWidget->setFixedHeight(48);
    m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);
//    m_pCloseButtonWidget->setStyleSheet("border:2px solid #FFFFFF;");
    QHBoxLayout* p_closeButtonHlayout = new QHBoxLayout;
    p_closeButtonHlayout->setContentsMargins(0, 4, 5, 5);
    p_closeButtonHlayout->setSpacing(0);
    m_pCloseButton = new QPushButton;
    m_pCloseButton->setObjectName("close");
    m_pCloseButton->setFlat(true);
    m_pCloseButton->setIcon(QIcon(":/data/svg/close.svg"));//bug163244
    m_pCloseButton->setIconSize(QSize(36, 36));
    m_pCloseButton->setFixedSize(36, 36);
    m_pCloseButton->installEventFilter(this);

    p_closeButtonHlayout->addSpacerItem(new QSpacerItem(455, 0, QSizePolicy::Expanding));
    p_closeButtonHlayout->addWidget(m_pCloseButton);
    m_pCloseButtonWidget->setLayout(p_closeButtonHlayout);

    /***** 中间窗口，图标和内容窗口 *****/
    m_pCenterWidget = new QWidget();
    m_pCenterWidget->setContentsMargins(0, 0, 24, 0);

    QHBoxLayout *p_center = new QHBoxLayout();
    p_center->setSpacing(0);
    p_center->setContentsMargins(0, 0, 0, 0);

    m_pIconLabelWidget = new QWidget();
    m_pIconLabelWidget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *p_IconLabelVlayout = new QVBoxLayout;
    p_IconLabelVlayout->setContentsMargins(0, 0, 0, 0);
//    m_pIconLabelWidget->setStyleSheet("border:2px solid #FFFFFF;");
    //添加svg图片，并设置其大小
    m_pIconlabel = new QLabel;
    m_pIconlabel->setFixedSize(23, 23);
    m_pIconlabel->setPixmap(QPixmap(QIcon(":/data/svg/tip.svg").pixmap(QSize(23, 23))));
    p_IconLabelVlayout->setAlignment(Qt::AlignTop);
    p_IconLabelVlayout->addWidget(m_pIconlabel, Qt::AlignTop);
    m_pIconLabelWidget->setLayout(p_IconLabelVlayout);

    m_pLabelInfoWidget = new QWidget();
    m_pLabelInfoWidget->setContentsMargins(0, 0, 0, 0);
//        m_pLabelInfoWidget->setStyleSheet("border:2px solid #FFFFFF;");

    QVBoxLayout *p_infoLabelVlayout2 = new QVBoxLayout;
    p_infoLabelVlayout2->setContentsMargins(0, 0, 0, 0);
    p_infoLabelVlayout2->setSpacing(16);

    m_pMainLableInfoOne = new QLabel();
    m_pMainLableInfoOne->setMinimumWidth(440);
    m_pMainLableInfoOne->setWordWrap(true);
    m_pMainLableInfoOne->setObjectName("info");

    m_pBelowLabelInfo = new QLabel();
    m_pBelowLabelInfo->setMinimumWidth(440);
    m_pBelowLabelInfo->setWordWrap(true);

    p_infoLabelVlayout2->addWidget(m_pMainLableInfoOne, Qt::AlignTop);
    p_infoLabelVlayout2->addWidget(m_pBelowLabelInfo, Qt::AlignTop);
    m_pLabelInfoWidget->setLayout(p_infoLabelVlayout2);


    p_center->addWidget(m_pIconLabelWidget);
    p_center->addItem(new QSpacerItem(8, 0, QSizePolicy::Fixed));
    p_center->addWidget(m_pLabelInfoWidget);
    m_pCenterWidget->setLayout(p_center);

    m_pButtonGrupWidget = new QWidget();
    m_pButtonGrupWidget->setContentsMargins(0, 0, 24, 0);

    QHBoxLayout *p_ButtonHlayout = new QHBoxLayout;
    p_ButtonHlayout->setContentsMargins(0, 0, 0, 0);
    p_ButtonHlayout->setSpacing(24);

    m_pCancelButton = new QPushButton;
    m_pCancelButton->setObjectName("cancel");
    m_pCancelButton->setFixedSize(96, 36);
    m_pCancelButton->installEventFilter(this);

    m_pOkButton = new QPushButton;
    m_pOkButton->setObjectName("OK");
    m_pOkButton->setFixedSize(96, 36);
    m_pOkButton->installEventFilter(this);
//    m_pOkButton->setDefault(true);

    p_ButtonHlayout->addItem(new QSpacerItem(376, 16, QSizePolicy::Expanding));
    p_ButtonHlayout->addWidget(m_pCancelButton);
    p_ButtonHlayout->addWidget(m_pOkButton);
    m_pButtonGrupWidget->setLayout(p_ButtonHlayout);

    m_pmainVlayout->addWidget(m_pCloseButtonWidget);
    m_pmainVlayout->addWidget(m_pCenterWidget);
//    m_pButtonGrupWidget->setStyleSheet("border:2px solid #FFFFFF;");
    m_pmainVlayout->addItem(new QSpacerItem(16, 32, QSizePolicy::Fixed));
    m_pmainVlayout->addWidget(m_pButtonGrupWidget);

    this->setLayout(m_pmainVlayout);
}

void MessageBox::initAllConnect()
{
    connect(m_pCloseButton, &QPushButton::clicked, [=]{execid = 1;reject();});
    connect(m_pCancelButton, &QPushButton::clicked, [=]{ execid = 1;
        reject();
    });
    connect(m_pOkButton, &QPushButton::clicked, [=]{
        execid = 1;
        accept();
    });
}

void MessageBox::addStyleSheet()
{
    QFile file(":/data/qss/MessageBox.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);

    QString strT;
    if(m_strBelowLabelColor.isEmpty())
    {
        strT = "font-size: 14px; \
                font-family: Noto Sans CJK SC; \
                font-weight: 400; \
                line-height: 24px; \
                color: #FF963C; \
                opacity: 1;";
    }
    else
    {
        strT = "font-size: 14px; \
                     font-family: Noto Sans CJK SC; \
                     font-weight: 400; \
                     line-height: 24px; \
                     opacity: 1; ";
        strT += m_strBelowLabelColor;
    }
    m_pBelowLabelInfo->setStyleSheet(strT);
}

void MessageBox::translateStr()
{
//    m_title->setText(tr("Close"));
    m_pCancelButton->setText(tr("Cancel"));
    if(m_strOKInfo.isEmpty())
        m_pOkButton->setText(tr("OK"));
    else
        m_pOkButton->setText(m_strOKInfo);
//    m_info->setText(tr("the installer is about to exit and the computer will be shut down."));
}

void MessageBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void MessageBox::resizeEvent(QResizeEvent *event)
{
//    InstallerMainWidget::resizeEvent(event);
    // Your code here
    QRect screenRect = QApplication::desktop()->screenGeometry();

    this->move(screenRect.width() / 2 - this->width() / 2, screenRect.height() / 2 - this->height() / 2);
}

void MessageBox::setMessageInfo(QString info)
{
    if (info == nullptr) {
        return ;
    }
    QStringList infoList = info.split("\n");
    m_pMainLableInfoOne->setText(infoList.at(0));

    if (infoList.length() > 1) {
        QString yellowInfoString;
        yellowInfoString.clear();
        for (int i = 1; i < infoList.length(); i++) {
            if (i == 1) {
                yellowInfoString += infoList.at(i);
            } else {
                yellowInfoString = yellowInfoString + "\n" + infoList.at(i);
            }
        }
        m_pBelowLabelInfo->setText(yellowInfoString);
    } else {
        m_pBelowLabelInfo->setVisible(false);
    }
    return;
}

void MessageBox::setCancleHidden()
{
    m_pCancelButton->setHidden(true);
    m_pCancelButton->setEnabled(false);
}

void MessageBox::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    QPixmap pixmap = QPixmap(":/data/png/bg_message.jpg");
//    QPalette pal(this->palette());
//    pal.setBrush(backgroundRole(), QBrush(pixmap));
//    this->setAutoFillBackground(true);
//    this->setPalette(pal);

    painter.setBrush(QBrush(pixmap));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() );
    rect.setHeight(rect.height() );
    painter.drawRoundedRect(rect, 6, 6);
//    也可用QPainterPath 绘制代替
//    painter.drawRoundedRect(rect, 15, 15);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 6, 6);
        painter.drawPath(painterPath);
    }
    QDialog::paintEvent(event);
}

bool MessageBox::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        QHoverEvent *eve = (QHoverEvent*)event;
        if(eve->type() == QEvent::Enter) {

            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget(true, this);
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));

//            m_tTip->move(m_pCloseButton->x() + (m_pCloseButton->width() - m_tTip->width()) / 2, m_pCloseButton->y() + m_pCloseButton->height() + 4);
            m_tTip->move(x()+width()-m_pCloseButton->width()-13, y()+m_pCloseButton->height()+15);
//            w->move(pos.x() - 20, pos.y() - w->height());
            m_tTip->show();
            connect(m_pCloseButton, &QPushButton::clicked, [=]{m_tTip->close();});

        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }

    if(watched->objectName() == "OK") {
        if(event->type() == QEvent::Enter) {
            m_isOnOkButton = true;
        }else if(event->type() == QEvent::Leave){
            m_isOnOkButton = false;
        }
    }
    if(watched->objectName() == "cancel") {
        if(event->type() == QEvent::Enter) {
            m_isOnCancelButton = true;
            m_isOnOkButton = false;
        }else if(event->type() == QEvent::Leave){
            m_isOnCancelButton = false;
            m_isOnOkButton = true;
        }
    }
    if(watched->objectName() == "close") {
        if(event->type() == QEvent::Enter) {
            m_isOnCloseButton = true;
            m_isOnOkButton = false;
        }else if(event->type() == QEvent::Leave){
            m_isOnCloseButton = false;
            m_isOnOkButton = true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void MessageBox::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << event->key() << "key pressed!";
    qDebug() << Q_FUNC_INFO << "keyPressEvent";

    if(event->key() == Qt::Key_Up)
    {

    }
    else if(event->key() == Qt::Key_Down)
    {

    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(m_isOnOkButton ||
           (!m_isOnCloseButton && !m_isOnCancelButton)){//防止受QT初始化影响
            emit m_pOkButton->clicked(true);
        }else{
            emit m_pCloseButton->clicked(false);
        }
    }
    else if(event->key() == Qt::Key_Backspace)
    {

    }
    else if(event->key() == Qt::Key_Escape)
    {
        execid = 1;
        reject();
    }
        QWidget::keyPressEvent(event);
}

void MessageBox::setOKInfo(QString strInfo)
{
    m_strOKInfo = strInfo;
    translateStr();
}

void MessageBox::clearOKInfo(void)
{
    m_strOKInfo.clear();
    translateStr();
}

void MessageBox::setBelowLabelColor(QString strInfo)
{
    m_strBelowLabelColor = strInfo;
    addStyleSheet();
}

void MessageBox::clearBelowLabelColor(void)
{
    m_strBelowLabelColor.clear();
    addStyleSheet();
}

}


