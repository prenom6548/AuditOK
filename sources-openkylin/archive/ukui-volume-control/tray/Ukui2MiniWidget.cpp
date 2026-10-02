/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui2MiniWidget.h"

#include <QHBoxLayout>
#include <QDebug>

Ukui2MiniWidget::Ukui2MiniWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& systemVolumeSliderMap,
                                 QPushButton* switchBtn,
                                 QWidget* parent) :
    m_sysVolumeSliderItemMap(systemVolumeSliderMap), m_pSwitchModuleBtn(switchBtn), QWidget(parent)
{
    initUi();
    initSlots();
}

void Ukui2MiniWidget::initUi()
{
    setFixedSize(345, 100);

    m_pLabel = new QLabel();
    m_pIconBtn = new QPushButton();
    m_pIconBtn->setAccessibleName(tr("Output device"));

//    m_pSwithModuleBtn->setIconSize(QSize(32, 32));
    m_pIconBtn->setFixedSize(36, 36);
    m_pIconBtn->setIconSize(QSize(16, 16));
    m_pIconBtn->setIcon(QIcon::fromTheme("audio-card"));
    m_pIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                              "padding-left:0px;}"
                              "QPushButton:hover {"
                              "background-color: #00000000;"
                              "color: white;}");

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pLabel);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Expanding));
    hLayout->setContentsMargins(16, 16, 8, 0);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_sysVolumeSliderItemMap[0]->getWidget());
    vLayout->setContentsMargins(0, 0, 0, 8);
    setLayout(vLayout);
}

void Ukui2MiniWidget::initSlots()
{
//    connect(m_pSwithModuleBtn, SIGNAL(pressed()), this, SLOT());
}

void Ukui2MiniWidget::setDescription(const QString& name)
{
    m_pLabel->setText(name);
}
