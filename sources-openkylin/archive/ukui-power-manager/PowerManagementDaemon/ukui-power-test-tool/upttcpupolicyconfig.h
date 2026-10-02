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
#ifndef UPTTCPUPOLICYCONFIG_H
#define UPTTCPUPOLICYCONFIG_H

#include <QWidget>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>

class UpttCpuPolicyConfig : public QWidget
{
    Q_OBJECT
public:
    explicit UpttCpuPolicyConfig(QWidget *parent = nullptr);
    ~UpttCpuPolicyConfig();

    int getPowerPolicyBattery();

private:
    QLabel *m_policyLabel;
    QButtonGroup *m_policyGroup;
    QRadioButton *m_powersaveRadio;
    QRadioButton *m_balanceRadio;
    QRadioButton *m_performanceRadio;
};

#endif // UPTTCPUPOLICYCONFIG_H
