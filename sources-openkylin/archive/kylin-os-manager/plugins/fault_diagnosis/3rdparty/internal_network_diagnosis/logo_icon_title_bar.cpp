/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "logo_icon_title_bar.h"
#include <QDebug>
#include "size_para.h"
LogoIconTitleBar::LogoIconTitleBar(QWidget *parent) : m_titleIcon(nullptr), m_titleLabel(nullptr)
{
    m_titleIcon = new QPushButton(parent);
    m_titleIcon->setIcon(QIcon::fromTheme("kylin-os-manager"));
    m_titleIcon->setIconSize(TITLE_ICON_SIZE);
    m_titleIcon->setFixedSize(TITLE_BTN_SIZE);
    m_titleIcon->setStyleSheet(BTN_TO_LABEL_STYLE);

    m_titleLabel = new QLabel(parent);

    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleLabel->setMinimumHeight(20);

    m_HLayoutLogoName = new QHBoxLayout(parent);
    m_HLayoutLogoName->setSpacing(0);
    m_HLayoutLogoName->setMargin(0);
    m_HLayoutLogoName->setContentsMargins(8, 8, 0, 0);
    m_HLayoutLogoName->addWidget(m_titleIcon);
    m_HLayoutLogoName->addSpacing(10);
    m_HLayoutLogoName->addWidget(m_titleLabel);
    m_HLayoutLogoName->addStretch();

    this->setLayout(m_HLayoutLogoName);
}

void LogoIconTitleBar::setTitleName(QString titleName)
{
    m_titleLabel->hide();
    m_titleLabel->setText(titleName);
    m_titleLabel->show();
}

void LogoIconTitleBar::setFontSize(QFont ft)
{
    m_titleLabel->setFont(ft);
}
