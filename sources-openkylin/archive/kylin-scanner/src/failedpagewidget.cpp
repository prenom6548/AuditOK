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

#include "failedpagewidget.h"
#include <QDebug>
#include <kysdk/applications/gsettingmonitor.h>

#include <QPainter>
#include <QStyleOption>
#include <kysdk/applications/gsettingmonitor.h>
#include "include/theme.h"

FailedPageWidget::FailedPageWidget(QWidget *parent) :
    QWidget(parent),
    m_failedPageIcon(new QLabel(this)),
    m_failedPageText(new QLabel(this)),
    m_failedPageButton(new QPushButton(this)),
    m_failedPageVLayout(new QVBoxLayout(this))
{
    setupGui();
    initConnect();
    initTheme();

}
void FailedPageWidget::initTheme(){
    if (isDarkTheme()) {
        themeColor = "black";
        m_failedPageIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_dark.svg").scaled(FailedPageIconSize));

    } else {
        themeColor = "white";
        m_failedPageIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_light.svg").scaled(FailedPageIconSize));
    }
}
bool FailedPageWidget::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();
    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}

void FailedPageWidget::setupGui()
{
    m_failedPageIcon->setFixedSize(FailedPageIconSize);

    m_failedPageText->setEnabled(false);
    m_failedPageText->setText(tr("No available scan devices"));

    m_failedPageButton->setProperty("isImportant", true);
    m_failedPageButton->setMinimumSize(FailedPageButtonSize);
    m_failedPageButton->setText(tr("Connect"));
    m_failedPageButton->adjustSize();
    m_failedPageButton->setFocusPolicy(Qt::StrongFocus);
    m_failedPageButton->setDefault(true);

    m_failedPageVLayout->setSpacing(0);
    m_failedPageVLayout->addStretch();
    m_failedPageVLayout->addWidget(m_failedPageIcon, 0,  Qt::AlignCenter);
    m_failedPageVLayout->addSpacing(8);
    m_failedPageVLayout->addWidget(m_failedPageText, 0,  Qt::AlignCenter);
    m_failedPageVLayout->addSpacing(24);
    m_failedPageVLayout->addWidget(m_failedPageButton, 0,  Qt::AlignCenter);
    m_failedPageVLayout->addStretch();
    m_failedPageVLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_failedPageVLayout);

}

void FailedPageWidget::initConnect()
{
    connect(m_failedPageButton, &QPushButton::clicked, this, [=](){
        emit scanButtonClicked();
    });
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange,this,&FailedPageWidget::fontSizeChanged);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &FailedPageWidget::initTheme);
    connect(GlobalUserSignal::getInstance(),&GlobalUserSignal::rotationChangedSig,this,&FailedPageWidget::rotateChangedSlot);
}

void FailedPageWidget::fontSizeChanged()
{
    float systemFontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font;
    font.setPointSize(systemFontSize);
    m_failedPageButton->setFont(font);

}

void FailedPageWidget::clickfiledPageButton()
{
    m_failedPageButton->click();
}

void FailedPageWidget::rotateChangedSlot(bool isPCMode){
    if(isPCMode){
        m_failedPageButton->setMinimumSize(FailedPageButtonSize);
    }else{
        m_failedPageButton->setMinimumSize(FailedPageButtonSizePADMODE);
    }
}
