/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef BATTERYWIDGET_H
#define BATTERYWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QDBusInterface>
#include <QPushButton>
#include <QPoint>
#include "lock-dialog/lockdialogmodel.h"

typedef enum {
    battery_state_unknown = 0,
    battery_state_charging,
    battery_state_discharging,
    battery_state_empty,
    battery_state_fully,
    battery_state_pending_charge,
    battery_state_pending_discharge,
    battery_state_last
} BatteryState;

class BatteryWidget : public QWidget
{
    Q_OBJECT
public:
    BatteryWidget(LockDialogModel *model, QWidget *parent = nullptr);

    void initUi();
    void setupComponent();
    void setPoint(QPoint point);

    void setHasBattery(bool hasBattery);
    void setIconName(QString iconName);
    void setAcOnlineState(bool acOnlineState);
    void setBatteryState(int iBatteryState);
    void setBatteryPercentage(int iBatteryPercentage);
    void setShowPowerLeftTime(int iShowPowerLeftTime);
    void setBatteryLeftTime(qlonglong timeToFull, qlonglong timeToEmpty);
    void setPowerStateLabelForAcOnlineChanged();
    void setPowerStateLabelForBatteryInfoChanged();

protected:
    void paintEvent(QPaintEvent *event);
    void changeEvent(QEvent *event);

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    QLabel *mModeLabel;
    QPushButton *mIconBtn;
    QLabel *mValueLabel;
    QLabel *mStatusLabel;
    QPoint mPoint;
    double m_nCurBatteryValue;
    int m_nCurBatteryState = battery_state_last;
    bool m_hasBattery = true;
    bool m_isAcOnlineState = true;
    bool m_batteryStateChanged = false;
    int m_showBatteryLeftTime = 0;
    qint64 m_timeToFull;
    qint64 m_timeToEmpty;
    QString m_iconName;

private Q_SLOTS:
    void refreshTranslate();
};

#endif // BATTERYWIDGET_H
