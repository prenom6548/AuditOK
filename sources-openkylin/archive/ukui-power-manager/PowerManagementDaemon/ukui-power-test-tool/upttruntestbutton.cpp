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
#include "upttruntestbutton.h"

UpttRunTestButton::UpttRunTestButton(QWidget *parent) : QWidget(parent)
{
    initRunTestButtonUI();

    initAcOnlineHintTimer();
}

UpttRunTestButton::~UpttRunTestButton()
{
    if (nullptr != m_acOnlineHintTimer) {
        delete m_acOnlineHintTimer;
    }
}

void UpttRunTestButton::initRunTestButtonUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    m_runEnduranceTestButton  = new QPushButton("续航测试", this);
    m_runEnergyRateTestButton = new QPushButton("功耗测试", this);
    m_stopTestButton = new QPushButton("停止测试", this);

    mainLayout->addStretch();
    mainLayout->addWidget(m_runEnduranceTestButton);
    mainLayout->addWidget(m_runEnergyRateTestButton);
    mainLayout->addWidget(m_stopTestButton);

    connect(m_runEnduranceTestButton, &QPushButton::clicked,
            this, &UpttRunTestButton::dealRunEnduranceTestButtonClicked);
    connect(m_runEnergyRateTestButton, &QPushButton::clicked,
            this, &UpttRunTestButton::dealRunEnergyRateTestButtonClicked);
    connect(m_stopTestButton, &QPushButton::clicked,
            this, &UpttRunTestButton::dealStopTestButtonClicked);
    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::acOnlineStateChanged,
            this, &UpttRunTestButton::dealAcOnlineStateChanged);
}

void UpttRunTestButton::initAcOnlineHintTimer()
{
    m_acOnlineHintTimer = new QTimer();
    connect(m_acOnlineHintTimer, &QTimer::timeout,
            this, &UpttRunTestButton::dealStopTestButtonClicked);
}

void UpttRunTestButton::stopTest()
{
    dealStopTestButtonClicked();
}

void UpttRunTestButton::dealRunEnduranceTestButtonClicked()
{
    m_runEnduranceTestButton->setEnabled(false);
    m_runEnergyRateTestButton->setEnabled(false);
    if (true == UpmUpowerDBus::self()->getAcOnlineState()) {
        m_acOnlineHintTimer->start(60000);
        emit acOnlineTestNotRunSignal();
    } else {
        emit runEnduranceTestSignal();
    }
    m_testState = run_endurance_test;
}

void UpttRunTestButton::dealRunEnergyRateTestButtonClicked()
{
    m_runEnduranceTestButton->setEnabled(false);
    m_runEnergyRateTestButton->setEnabled(false);
    if (true == UpmUpowerDBus::self()->getAcOnlineState()) {
        m_acOnlineHintTimer->start(60000);
        emit acOnlineTestNotRunSignal();
    } else {
        emit runEnergyRateTestSignal();
    }
    m_testState = run_energy_rate_test;
}

void UpttRunTestButton::dealStopTestButtonClicked()
{
    if (true == m_acOnlineHintTimer->isActive()) {
        m_acOnlineHintTimer->stop();
    }
    m_runEnduranceTestButton->setEnabled(true);
    m_runEnergyRateTestButton->setEnabled(true);
    m_testState = stop_test;
    emit stopTestSignal();
}

void UpttRunTestButton::dealAcOnlineStateChanged()
{
    if (true == UpmUpowerDBus::self()->getAcOnlineState()) {
        dealStopTestButtonClicked();
    } else {
        if (true == m_acOnlineHintTimer->isActive()) {
            m_acOnlineHintTimer->stop();
            if (run_endurance_test == m_testState) {
                emit runEnduranceTestSignal();
            } else if (run_energy_rate_test == m_testState) {
                emit runEnergyRateTestSignal();
            } else {
                ;
            }
        }
    }
}
