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
#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QDateTime>
#include <QVBoxLayout>

#include <libkydate.h>
#include "customlabel.h"

enum dateinfo
{
    DATE,
    TIME,
};

class LockDialogModel;
extern float scale;
class TimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(LockDialogModel *model, QWidget *parent = nullptr);
    ~TimeWidget();
    void updateUI();

Q_SIGNALS:

public Q_SLOTS:
    void update_datatime();
    void timerStart();
    void timerStop();
    void updateTimeFont(QString fontFamily);
    void updateTimeFontSize(double fontSize);

private:
    QString getLongFormatDate(int type);

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    QTimer *m_timer = nullptr;
    CustomLabel *m_t_label = nullptr;
    CustomLabel *m_d_label = nullptr;
    double m_curFontSize;
    double m_ptToPx = 1.0;
};

#endif // TIMEWIDGET_H
