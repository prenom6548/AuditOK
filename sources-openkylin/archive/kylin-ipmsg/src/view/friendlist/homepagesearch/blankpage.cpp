/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "blankpage.h"
#include "global/utils/global_data.h"
/* 适配 kySDK  */
#include <gsettingmonitor.h>

BlankPage::BlankPage(QWidget *parent) : QWidget(parent)
{
    init();
    initGsetting();
}

BlankPage::~BlankPage() {}

void BlankPage::init()
{
    this->setMinimumSize(QSize(350, 506));
    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //    this->setStyleSheet("background-color:#FFFFFF");
    m_blankLabel = new QLabel();
    m_blankLabel->setAccessibleName("kylin-ipmsg_blankpage_QLabel_mblankLabel");
    QIcon blankIcon = QIcon(":/res/kylin-ipmsg.svg");
    m_blankLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(QSize(96, 96))));
    m_blankTextLabel = new ::kdk::KLabel();
    m_blankTextLabel->setAccessibleName("kylin-ipmsg_blankpage_KLabel_mblankTextLabel");
    m_blankTextLabel->setText(tr("No Information"));

    // 空白页面布局
    m_hBlankLayout = new QHBoxLayout();
    m_vBlankLayout = new QVBoxLayout();
    m_hBlankLayout->setObjectName("kylin-ipmsg_blankpage_QHBoxLayout_mhBlankLayout");
    m_vBlankLayout->setObjectName("kylin-ipmsg_blankpage_QVBoxLayout_mvBlankLayout");

    m_hBlankLayout->addStretch();
    m_hBlankLayout->addWidget(m_blankLabel);
    m_hBlankLayout->addStretch();

    m_vBlankLayout->addSpacing(160);
    m_vBlankLayout->addLayout(m_hBlankLayout);
    m_vBlankLayout->setSpacing(8);
    m_vBlankLayout->addWidget(m_blankTextLabel);
    m_blankTextLabel->setAlignment(Qt::AlignHCenter);
    m_vBlankLayout->addStretch();

    this->setLayout(m_vBlankLayout);
}

void BlankPage::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        changeTheme();
    });
}

void BlankPage::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        this->setStyleSheet("background-color:#FFFFFF");
        m_blankTextLabel->setFontColor(QColor("#8C8C8C"));
    } else {
        this->setStyleSheet("background-color:#1d1d1d");
        m_blankTextLabel->setFontColor(QColor("#8F9399"));
        }
    if (searchblank == 1) {
        if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
            QIcon SearchIcon1 = QIcon(":/res/Search-web-light.svg");
            m_blankLabel->setPixmap(SearchIcon1.pixmap(SearchIcon1.actualSize(QSize(96, 96))));
        } else {
            QIcon SearchIcon2 = QIcon(":/res/Search-web-dark.svg");
            m_blankLabel->setPixmap(SearchIcon2.pixmap(SearchIcon2.actualSize(QSize(96, 96))));
        }
    }
}

void BlankPage::changeSearchText()
{
    m_blankTextLabel->setText(tr("No Relevant Contacts"));
    searchblank = 1;
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        QIcon SearchIcon1 = QIcon(":/res/Search-web-light.svg");
        m_blankLabel->setPixmap(SearchIcon1.pixmap(SearchIcon1.actualSize(QSize(96, 96))));
    } else {
        QIcon SearchIcon2 = QIcon(":/res/Search-web-dark.svg");
        m_blankLabel->setPixmap(SearchIcon2.pixmap(SearchIcon2.actualSize(QSize(96, 96))));
    }
}

void BlankPage::changeBlankText()
{
    m_blankTextLabel->setText(tr("No Information"));
    QIcon blankIcon = QIcon(":/res/kylin-ipmsg.svg");
    m_blankLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(QSize(96, 96))));
    searchblank = 0;
}
