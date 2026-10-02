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

#include "licenseframe.h"

#include <QDebug>

#include "../PluginService/klinguist.h"
#include <QTextCodec>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

#include <QScrollBar>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
using namespace KServer;
namespace KInstaller {


LicenseFrame::LicenseFrame(QWidget *parent) : MiddleFrameManager(parent),
    lang_bl(true),
    scrollbarInt(0),
    tmpDouble(0)
{
    this->initNextBnt();
    m_nextBtn->setEnabled(false);
    initUI();
    initAllConnect();
    addStyleSheet();
    setBoardTabOrder();
    translateStr();
    this->setFocus();
}

void LicenseFrame::initUI()
{
    // 定义一个横向 layout, 居中对其，边距为左 300,上 20,右 300,下 20,
    licneseLayout = new QGridLayout();
    licneseLayout->setSpacing(0);
    licneseLayout->setContentsMargins(0, 0, 0, 0);
//    m_Widget->setLayout(licneseLayout);
    m_Widget->setLayout(licneseLayout);

    licneseLayout->setColumnStretch(0, 2);
    licneseLayout->setColumnStretch(1, 5);
    licneseLayout->setColumnStretch(2, 2);

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");
    licneseLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    licneseLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    licneseLayout->addItem(new QSpacerItem(10, 34, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    // 定义一个 QTextBrowser，来用显示许可协议
    license_text = new QTextBrowser();
    license_text->setObjectName("licensetext");
    license_text->setAlignment(Qt::AlignCenter);
    license_text->setContextMenuPolicy(Qt::NoContextMenu);
    license_text->installEventFilter(this);
    license_text->viewport()->installEventFilter(this);
    connect(license_text->verticalScrollBar(),&QScrollBar::valueChanged,this,&LicenseFrame::checkScrollEnd);
    loadLicense();
//    licneseLayout->setRowStretch(3, 4);
//    license_text->setMaximumSize(1500, 1050);
//    license_text->setMinimumSize(800, 500);

    license_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    license_text->setWindowFlags( Qt::FramelessWindowHint);

    licneseLayout->addWidget(license_text, 3, 1, 1, 1, Qt::AlignCenter);
    licneseLayout->addItem(new QSpacerItem(10, 36, QSizePolicy::Fixed, QSizePolicy::Fixed), 4, 1, 1, 1);

    m_timetip = new QCheckBox;
    m_timetip->setFocusPolicy(Qt::TabFocus);
    m_timetip->installEventFilter(this);
    licneseLayout->addWidget(m_timetip, 5, 1, 1, 1, Qt::AlignCenter);
    licneseLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Expanding), 6, 1, 1, 1);
//    m_timetip->setCheckable(false);
//    m_timetip->setEnabled(false);
    initFocusPolicy();


}

void LicenseFrame::setBoardTabOrder()
{
   // QWidget::setTabOrder(license_text, m_nextBtn);
}

void LicenseFrame::initFocusPolicy()
{
    QWidget::setTabOrder(license_text, m_timetip);
    QWidget::setTabOrder(m_timetip, m_nextBtn);
}

QString LicenseFrame::readSettingIni()
{
    return KServer::ReadSettingIni("config", "language");

}
void LicenseFrame::loadLicense()
{
    // 打开需要显示的许可协议文件
    if(readSettingIni() == "en_US") {       //英文
        path = ":/res/file/EULAE-new";
    } else if(readSettingIni() == "bo_CN")  //藏文 BUG120265安装语言选择“藏文”，用户使用许可协议语言不是藏
    {
        //path = ":/res/file/EULABO";
        path = ":/res/file/EULA-new";
    } else if(readSettingIni() == "zh_HK")  //中文繁体
    {
        path = ":/res/file/EULAHK-new";
    }
    else if(readSettingIni() == "mn_MN")    //蒙古文
    {
        //path = ":/res/file/EULAMN";
        path = ":/res/file/EULA-new";
    }
    else
        path = ":/res/file/EULA-new";           //中文简体

    license_text->clear();
    QFile file(path);

    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "license Can not open";

    QTextStream in(&file);
    // 设置字体编码格式
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    in.setCodec(codec);

    // 读取文件内容进显示控件
    // license_text->setText(in.readAll());
    QString strAll = in.readAll();
    QStringList qsList = strAll.split("\n");
    m_fileStr.clear();
    bool typeFirstTitle=true;
    foreach(QString qsSingle, qsList)
    {
        if((qsSingle.size() > 1) && ('X' == qsSingle[0]) && ('X' == qsSingle[1]))// 首行缩进
        {
            qsSingle.remove(0,2);//去除做标记的XX字符串
            m_fileStr += "<p style='margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; text-indent:26px; '>" + qsSingle + "</p>";
        }
        else if((qsSingle.size() > 1) && ('C' == qsSingle[0]) && ('C' == qsSingle[1]))// 居中显示
        {
            if(!typeFirstTitle)
            {
                m_fileStr +=  "<p style='margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; text-indent:0px; '>&nbsp;&nbsp;</p>";
            }

            qsSingle.remove(0,2);//去除做标记的CC字符串
            m_fileStr +=  "<div align=center> <p style='margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; '>" + qsSingle + "</p> </div>";
            typeFirstTitle=false;
        }
        else if((qsSingle.size() > 1) && ('R' == qsSingle[0]) && ('R' == qsSingle[1]))// 右对齐显示
        {
            qsSingle.remove(0,2);//去除做标记的RR字符串
            m_fileStr += "<div align=right> <p style='margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; '>" + qsSingle + "</p> </div>";
        }
        else
            m_fileStr +=  "<p style='margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; text-indent:0px; '>" + qsSingle + "</p>";
    }
    license_text->setText(m_fileStr);
    license_text->setLineWrapMode(QTextBrowser::WidgetWidth);
    license_text->setLineWrapColumnOrWidth(license_text->width());
    license_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 //   license_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    license_text->verticalScrollBar()->hasTracking();
    QRect rect = KServer::GetScreenRect();
    license_text->setFixedSize(rect.width() / 2, rect.height() / 2);
    license_text->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);//禁掉右侧滚轮对鼠标右键的反应,解决bug155673

    file.close();
}

void LicenseFrame::initAllConnect()
{
    connect(this->m_timetip, &QCheckBox::clicked, this, &LicenseFrame::nextBtnAvailable);
    connect(license_text->verticalScrollBar(), &QScrollBar::valueChanged, this, &LicenseFrame::nextBtnAvailable);

}
void LicenseFrame::addStyleSheet()
{
    QFile file(":/res/qss/KyLicense.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}
void LicenseFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
        qDebug() << "1:" << lang_bl;
        lang_bl = !lang_bl;
        tmpDouble =1.0 * license_text->verticalScrollBar()->value() / license_text->verticalScrollBar()->maximum();
        loadLicense();
//        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->maximum() * tmpDouble);
        qDebug() << "2:" << lang_bl;
    } else {
        QWidget::changeEvent(event);
    }
}



void LicenseFrame::translateStr()
{
    m_mainTitle->setText(tr("Read License Agreement"));//请阅读许可协议
    m_timetip->setText(tr("I have read and agree to the terms of the agreement"));//我已经阅读并同意协议条款
//    license_text->setHtml(QObject::tr(m_fileStr));
    m_nextBtn->setText(tr("Next"));
}

void LicenseFrame::nextBtnAvailable()
{
    if(m_timetip->isChecked()) {
//        m_nextBtn->setEnabled(true);
//        m_nextBtn->setStyleSheet("background:#1D8EFF;");
//        scrollbarInt = license_text->verticalScrollBar()->value();
//        if(scrollbarInt) {
//            if(scrollbarInt >= (license_text->verticalScrollBar()->maximum())) {
//                m_nextBtn->setEnabled(true);
//                m_nextBtn->setStyleSheet("background:#1D8EFF;");
//            }
//        }
        m_nextBtn->setEnabled(true);
//        m_nextBtn->setStyleSheet("background:#1D8EFF;");
    } else {
        m_nextBtn->setEnabled(false);
    }
//2021-3-5
//    scrollbarInt = license_text->verticalScrollBar()->value();
//    if(scrollbarInt) {
//        if(scrollbarInt >= (license_text->verticalScrollBar()->maximum())) {
//            m_timetip->setCheckable(true);
//            m_timetip->setEnabled(true);
//            if(m_timetip->isChecked()) {
//                m_nextBtn->setEnabled(true);
//                m_nextBtn->setStyleSheet("background:#1D8EFF;");
//            } else {
//                m_nextBtn->setEnabled(false);
//            }
//        }
//    }

}

void LicenseFrame::checkScrollEnd(int value)
{
    QScrollBar *scrollBar = license_text->verticalScrollBar();
    if(value ==scrollBar->maximum())
    {
        m_timetip->setFocus();
    }
}

void LicenseFrame::clickNextButton()
{
    emit signalStackPageChanged(1);
}

void LicenseFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug()<< Q_FUNC_INFO << event->key() << "key pressed!";
    if(event->key() == Qt::Key_Up)
    {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()-50);
    }
    else if(event->key() == Qt::Key_Down)
    {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()+50);
    }
    else if (event->key() == Qt::Key_PageUp) {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()-500);
    }
    else if (event->key() == Qt::Key_PageDown) {
        license_text->verticalScrollBar()->setValue(license_text->verticalScrollBar()->value()+500);
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(m_nextBtn->isEnabled() == true)
            //clickNextButton();
            emit enterpressed();
            //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
        //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

bool LicenseFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==license_text &&event->type()==QEvent::FocusIn)
    {
        license_text->verticalScrollBar()->setValue(0);
    }
    if(watched==license_text &&event->type()==QEvent::KeyPress)
    {
        QKeyEvent *ev=(QKeyEvent*)event;
        if(ev->key() == Qt::Key_Tab)
        {
            license_text->setStyleSheet("QTextBrowser#licensetext:focus {border-bottom-left-radius:0px;border-top-left-radius:0px;border-left: 4px solid #5BADFF;outline: none;}");
        }
    }
    if((watched==license_text ||watched==license_text->viewport())&&event->type() == QEvent::MouseButtonPress)
    {
        license_text->setStyleSheet("QTextBrowser#licensetext:focus {border-bottom-left-radius:0px;border-top-left-radius:0px;border-left: 4px solid transparent;outline: none;}");
    }
    if(watched==m_timetip&&event->type()==QEvent::KeyPress)
    {
        QKeyEvent *ev=(QKeyEvent*)event;
        if(ev->key() == Qt::Key_Up||ev->key() == Qt::Key_Down||ev->key() == Qt::Key_Left||ev->key() == Qt::Key_Right)
        {
                return true;
        }
    }
    return QObject::eventFilter(watched,event);
}

}
