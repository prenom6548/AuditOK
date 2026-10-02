/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "agreementwindow.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QDebug>
#include <QScrollBar>

#define CONFIG_FILE "/usr/share/ukui-greeter/ukui-greeter.conf"

AgreementWindow::AgreementWindow(QWidget *parent) :
    QWidget(parent)
{
    init();
}

bool AgreementWindow::getShowLoginPrompt()
{
    return showLoginPrompt;
}

void AgreementWindow::initUI()
{
    centerWidget = new QWidget(this);
    centerWidget->setObjectName("centerWidget");
   // centerWidget->setFixedSize(960,744);
    centerWidget->setStyleSheet("#centerWidget{background-color: rgba(0,0,0,15%);border-radius: 12px;}");

    QVBoxLayout *layout = new QVBoxLayout(centerWidget);


    ensureBtn = new QPushButton(this);
    ensureBtn->setText(tr("I know"));
    ensureBtn->setObjectName("ensureBtn");
    ensureBtn->setFixedSize(240,48);
    ensureBtn->setDefault(true);
    ensureBtn->setFocusPolicy(Qt::StrongFocus);
    ensureBtn->setStyleSheet("#ensureBtn{background-color:rgba(55, 144, 250, 1);border-radius: 6px;}");
    ensureBtn->setStyleSheet("QPushButton{text-align:center;background-color:#3790FA;border-radius: 6px;color:white} \
			      QPushButton::hover{background-color:#3489EE;} \
			      QPushButton::pressed {background-color:#2C73C8;}");
    connect(ensureBtn, &QPushButton::clicked, this, &AgreementWindow::switchPage);

    titleLbl = new QLabel(centerWidget);
    titleLbl->setText(promptTitle);
    titleLbl->setObjectName("titleLbl");
    titleLbl->setStyleSheet("#titleLbl{color: white;font-size: 24px;}");

    browser = new QTextBrowser(centerWidget);
    browser->setObjectName("browser");
    browser->setAttribute(Qt::WA_TranslucentBackground);
    browser->setStyleSheet("#browser{background-color:transparent;border-radius: none;color: white;font-size: 16px;}");
    browser->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    browser->setContextMenuPolicy(Qt::NoContextMenu);
    browser->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    setFocusProxy(ensureBtn);

    bool res = loadText();
    if(!res || (promptTitle.isEmpty() && !hideTitle)){
        emit switchToGreeter();
    }

    layout->addSpacing(24);
    layout->addWidget(titleLbl);
    layout->addWidget(browser);
    layout->setAlignment(ensureBtn,Qt::AlignCenter);
    layout->setAlignment(titleLbl,Qt::AlignCenter);

    if(hideTitle)
        titleLbl->hide();
}

void AgreementWindow::switchPage()
{
    emit switchToGreeter();
}

void AgreementWindow::resizeEvent(QResizeEvent *event)
{
   // qDebug()<<""
    int m_width = 960,m_height = 722 - 88;
    if(this->width() < (960+80))
        m_width = (width() - 80)/2;
    if(this->height() < (722+80))
        m_height = (height() - 80)/2;

    centerWidget->setGeometry((width()-m_width)/2,(height()-m_height)/2,m_width,m_height);

    ensureBtn->move((width()-ensureBtn->width())/2,centerWidget->geometry().bottom() + 40);

}

bool AgreementWindow::loadText()
{
    if(!promptText.isEmpty()){
        browser->setText(promptText);
        return true;
    }

    if(promptTextFilePath.isEmpty())
       return false;
    QFile file(promptTextFilePath);
    if(!file.exists())
        return false;
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    if(str == "")
	return false;
    browser->setText(str);
    file.close();
    return true;

}

void AgreementWindow::init()
{
   QSettings *settings = new QSettings(CONFIG_FILE,QSettings::IniFormat);
   settings->setIniCodec(QTextCodec::codecForName("utf-8"));

   if(settings->contains("Greeter/showLoginPrompt")){
       showLoginPrompt = settings->value("Greeter/showLoginPrompt").toBool();
   }

   if(settings->contains("Greeter/hideTitle")){
       hideTitle = settings->value("Greeter/hideTitle").toBool();
   }

   if(settings->contains("Greeter/promptTitle")){
       promptTitle = settings->value("Greeter/promptTitle").toString();
   }

   if(settings->contains("Greeter/promptText")){
       promptText = settings->value("Greeter/promptText").toString();
   }

   if(settings->contains("Greeter/promptTextFilePath")){
       promptTextFilePath = settings->value("Greeter/promptTextFilePath").toString();
   }

}
