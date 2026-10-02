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
#include "leftsuccesspagewidget.h"
#include "include/theme.h"
#include <kysdk/applications/gsettingmonitor.h>
#include "globalsignal.h"

LeftSuccessPageWidget::LeftSuccessPageWidget(QWidget *parent) :
    QWidget(parent),
    m_connectSuccessIcon(new QLabel(this)),
    m_connectSuccessText(new QLabel(this)),
    m_connectSuccessVLayout(new QVBoxLayout(this))
{
    setupGui();
    initTheme();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &LeftSuccessPageWidget::initTheme);
    connect(g_user_signal, &GlobalUserSignal::updateConnectSuccessTextSignal, this, &LeftSuccessPageWidget::updateText);
}

void LeftSuccessPageWidget::initTheme(){
    if (isDarkTheme()) {
        m_connectSuccessIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_dark.svg").scaled(LeftSuccessPageIconSize));
    } else {
        m_connectSuccessIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_light.svg").scaled(LeftSuccessPageIconSize));
    }
}

void LeftSuccessPageWidget::updateText(bool hasdevice)
{
    if(hasdevice){
        m_connectSuccessText->setText(tr("Connect scanners, please click scan button to start scanning."));
    }else{
        m_connectSuccessText->setText(tr("No scanner detected, plug in a new scanner to refresh the device list."));
    }
}

bool LeftSuccessPageWidget::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}

void LeftSuccessPageWidget::setupGui()
{
    setMinimumSize(LeftSuccessPageWidth, LeftSuccessPageHeight);
    m_connectSuccessIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_light.svg").scaled(LeftSuccessPageIconSize));
    m_connectSuccessIcon->setFixedSize(LeftSuccessPageIconSize);

    m_connectSuccessText->setMinimumSize(550, 50);
    m_connectSuccessText->setMaximumSize(550, 100);
    m_connectSuccessText->setWordWrap(true);
    m_connectSuccessText->setEnabled(false);
    m_connectSuccessText->setText(tr("Connect scanners, please click scan button to start scanning."));
    m_connectSuccessText->setAlignment(Qt::AlignCenter);


    m_connectSuccessVLayout->setSpacing(0);
    m_connectSuccessVLayout->addStretch();
    m_connectSuccessVLayout->addWidget(m_connectSuccessIcon, 0,  Qt::AlignCenter | Qt::AlignHCenter | Qt::AlignVCenter);
    m_connectSuccessVLayout->addSpacing(8);
    m_connectSuccessVLayout->addWidget(m_connectSuccessText, 0,  Qt::AlignCenter | Qt::AlignHCenter | Qt::AlignVCenter);
    m_connectSuccessVLayout->addStretch();
    m_connectSuccessVLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_connectSuccessVLayout);

}
