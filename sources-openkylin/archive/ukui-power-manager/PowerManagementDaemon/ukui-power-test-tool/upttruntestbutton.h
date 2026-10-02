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
#ifndef UPTTRUNTESTBUTTON_H
#define UPTTRUNTESTBUTTON_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>

#include "upower-dbus/upm_upowerdbus.h"

class UpttRunTestButton : public QWidget
{
    Q_OBJECT
public:
    explicit UpttRunTestButton(QWidget *parent = nullptr);
    ~UpttRunTestButton();

    void stopTest();

private:
    QPushButton *m_runEnduranceTestButton;
    QPushButton *m_runEnergyRateTestButton;
    QPushButton *m_stopTestButton;

    QTimer *m_acOnlineHintTimer;

    enum test_state {
        stop_test,
        run_endurance_test,
        run_energy_rate_test
    };

    int m_testState = stop_test;

    void initRunTestButtonUI();
    void initAcOnlineHintTimer();

private slots:
    void dealRunEnduranceTestButtonClicked();
    void dealRunEnergyRateTestButtonClicked();
    void dealStopTestButtonClicked();
    void dealAcOnlineStateChanged();

signals:
    void runEnduranceTestSignal();
    void runEnergyRateTestSignal();
    void stopTestSignal();
    void acOnlineTestNotRunSignal();
};

#endif // UPTTRUNTESTBUTTON_H
