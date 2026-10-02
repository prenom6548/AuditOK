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

class BatteryWidget : public QWidget
{
    Q_OBJECT
public:
    BatteryWidget(LockDialogModel *model, QWidget *parent = nullptr);

    void initUi();
    void setupComponent();
    void setPoint(QPoint point);

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
    int m_nCurBatteryValue;
    int m_nCurBatteryState;

private Q_SLOTS:
    void onBatteryChanged(QStringList args);
    void onBatteryStatusChanged(QString iconName);
    void refreshTranslate();
};

#endif // BATTERYWIDGET_H
