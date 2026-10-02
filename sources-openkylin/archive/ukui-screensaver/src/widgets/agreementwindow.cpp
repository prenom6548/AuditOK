/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#define CENTER_WIDTH 960
#define CENTER_HEIGHT 722
#define ENSURE_BUTTON_HEIGHT 48
#define BORDER_WIDTH 40
AgreementWindow::AgreementWindow(bool hideTitle, QString title, QString text, QWidget *parent)
    : m_hideTitle(hideTitle), m_title(title), m_text(text), QWidget(parent)
{
    initUI();
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
    ensureBtn->setFixedSize(240, 48);
    ensureBtn->setDefault(true);
    ensureBtn->setFocusPolicy(Qt::StrongFocus);
    ensureBtn->setProperty("isImportant", true);
    // ensureBtn->setStyleSheet("#ensureBtn{background-color:rgba(55, 144, 250, 1);border-radius: 6px;}");
    // ensureBtn->setStyleSheet("QPushButton{text-align:center;background-color:#3790FA;border-radius: 6px;color:white}
    //		      QPushButton::hover{background-color:#3489EE;}
    //		      QPushButton::pressed {background-color:#2C73C8;}");
    connect(ensureBtn, &QPushButton::clicked, this, &AgreementWindow::switchPage);

    titleLbl = new QLabel(centerWidget);
    titleLbl->setText(m_title);
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

    browser->setText(m_text);

    layout->addSpacing(24);
    layout->addWidget(titleLbl);
    layout->addWidget(browser);
    layout->setAlignment(ensureBtn, Qt::AlignCenter);
    layout->setAlignment(titleLbl, Qt::AlignCenter);

    if (m_hideTitle)
        titleLbl->hide();
}

void AgreementWindow::switchPage()
{
    emit switchToGreeter();
}

void AgreementWindow::resizeEvent(QResizeEvent *event)
{
    int m_width = CENTER_WIDTH, m_height = CENTER_HEIGHT - ENSURE_BUTTON_HEIGHT - BORDER_WIDTH;
    if (this->width() < (CENTER_WIDTH + 2 * BORDER_WIDTH))
        m_width = (width() - BORDER_WIDTH) / 2;
    if (this->height() < (CENTER_HEIGHT + 2 * BORDER_WIDTH))
        m_height = (height() - BORDER_WIDTH) / 2;

    if (centerWidget) {
        centerWidget->setGeometry((width() - m_width) / 2, (height() - m_height) / 2, m_width, m_height);

        if (ensureBtn)
            ensureBtn->move((width() - ensureBtn->width()) / 2, centerWidget->geometry().bottom() + 40);
    }
}
