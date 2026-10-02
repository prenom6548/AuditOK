/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "waittingdialog.h"
#include "Qt/windowmanage.hpp"
#include <windowmanager/windowmanager.h>
#include <QDebug>

WaittingDialog::WaittingDialog(QWidget *parent)
    : QDialog(parent),
      m_layout(new QVBoxLayout()),
      m_mainVlayout(new QVBoxLayout()),
      m_mainHLayout(new QHBoxLayout()),
      m_mainWidget(new QWidget()),
      m_titleHBoxLayout(new QHBoxLayout()),
      m_closeButton(new QPushButton()),
      m_text(new QLabel()),
      m_progress(new QProgressBar())
{
    init();
    initConnect();
    this->setWindowModality(Qt::ApplicationModal);
    WindowManage::removeHeader(this);
    setWindowTitle(tr("Scanner"));

}

void WaittingDialog::init()
{
    m_progress->setOrientation(Qt::Horizontal);
    m_progress->setMinimum(0);
    m_progress->setMaximum(0);
    m_progress->setFixedHeight(16);

    m_closeButton->setIcon(QIcon::fromTheme ("window-close-symbolic"));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize (QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);

    m_titleHBoxLayout->addStretch();
    m_titleHBoxLayout->addWidget(m_closeButton);
    m_titleHBoxLayout->setContentsMargins(0, 4, 4, 4);
    m_titleHBoxLayout->setAlignment(Qt::AlignCenter);

    m_text->setText(tr("Searching for scanner..."));
    m_mainVlayout->addWidget(m_text);
    m_mainVlayout->addSpacing(16);
    m_mainVlayout->addWidget(m_progress);
    m_mainWidget->setLayout(m_mainVlayout);

    m_mainHLayout->addWidget(m_mainWidget);
    m_mainHLayout->setContentsMargins(24, 0, 24, 32);

    m_layout->addLayout(m_titleHBoxLayout);
    m_layout->addLayout(m_mainHLayout);

    this->setLayout(m_layout);
    this->setFixedSize(450, 170);
    m_progress->setFocus();
}

void WaittingDialog::initConnect()
{
    connect(m_closeButton, &QPushButton::clicked, this, &WaittingDialog::close);
}

void WaittingDialog::setText(QString text)
{
   m_text->setText(text);
}


