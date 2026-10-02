/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */
#include "upttcpupolicyconfig.h"

#include <QDebug>

UpttCpuPolicyConfig::UpttCpuPolicyConfig(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    m_policyLabel = new QLabel("工作模式：", this);

    m_policyLabel->setContentsMargins(0, 0, 30, 0);

    m_policyGroup = new QButtonGroup(this);

    m_performanceRadio = new QRadioButton("性能模式", this);
    m_balanceRadio     = new QRadioButton("平衡模式", this);
    m_powersaveRadio   = new QRadioButton("节能模式", this);
    m_powersaveRadio->setChecked(true);

    m_policyGroup->addButton(m_performanceRadio, 0);
    m_policyGroup->addButton(m_balanceRadio, 1);
    m_policyGroup->addButton(m_powersaveRadio, 2);

    mainLayout->setSpacing(20);
    mainLayout->addWidget(m_policyLabel);
    mainLayout->addWidget(m_performanceRadio);
    mainLayout->addWidget(m_balanceRadio);
    mainLayout->addWidget(m_powersaveRadio);
    mainLayout->addStretch();

}

UpttCpuPolicyConfig::~UpttCpuPolicyConfig() {}

int UpttCpuPolicyConfig::getPowerPolicyBattery()
{
    return m_policyGroup->checkedId();
}
