/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DATETIMEUI_H
#define DATETIMEUI_H

#include "timeBtn.h"
#include "timelabel.h"
#include "clockwidget.h"
#include "calendarwidget.h"
#include "dategroupwidget.h"
#include "dateedit.h"
#include "settinggroup.h"
#include "ukccframe.h"
#include "addbutton.h"
#include "comboxwidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <kpushbutton.h>
#include <klabel.h>
#include <QGSettings>

using namespace kdk;

class DatetimeUi : public QWidget
{
    Q_OBJECT
public:
    DatetimeUi();
    ~DatetimeUi();
    void setTimeMode(QString mode);
    void setSyncResultLabel(int flag);
    void setNtpServers(QStringList sl);
    void setNtpServer(QString s);
    void setTimezoneStr(QString str);
    void addOtherTimezone(const QString &timezone, const QString & timezoneName);
    void setAddTimezoneBtnEnabled(bool b);
    void resetDateFormat();
    void resetDateTime();
    void updateDate();
    KPushButton* getChangeTZbutton() {return changeTZbutton;}
    QRadioButton* getSyncRadioButton() {return syncRadioButton;}
    QRadioButton* getManualRadioButton() {return manualRadioButton;}
    ComboxWidget* getComboxWidget() {return ntpSerWidget;}
    void setTimeFrameEnable(bool enable);

// 初始化
private:
    void initUi();
    void initObject();
    void initCurrentDate();
    void initOtherTimezone();
    void initSignals();
    void styleSettingChangeSlot(const QString &key);

private slots:
    void updateDateFormat(QString date);

private:
    QVBoxLayout *uiLayout            = nullptr;
    QVBoxLayout *currentDateLayout   = nullptr;
    QVBoxLayout *otherTimezoneLayout = nullptr;

private:
    // 当前时间
    KLabel          *currentTitleLabel = nullptr;
    ClockWidget     *clockWidget       = nullptr;
    TimeLabel       *timeLabel         = nullptr;
    KLabel        *mDateLabel        = nullptr;
    KPushButton     *changeTZbutton    = nullptr;
    KLabel        *setTimeModeLabel  = nullptr;
    QRadioButton    *manualRadioButton = nullptr;
    QRadioButton    *syncRadioButton   = nullptr;
    KLabel        *syncResultLabel   = nullptr;
    QGSettings    *m_fontSetting     = nullptr;
    KLabel        *setDateLabel      = nullptr;
    DategroupWidget *dategroupWidget   = nullptr;
    ComboxWidget    *ntpSerWidget      = nullptr;
    KLabel        *setNtpLabel       = nullptr;
    QLineEdit       *setNtpLineEdit    = nullptr;
    KPushButton     *setNtpButton      = nullptr;
    // 其他时区时间
    KLabel *otherTimezoneTitleLabel = nullptr;
    AddButton  *addTimezoneButton       = nullptr;

private:
    QString      timezoneStr   = "";
    QButtonGroup *timeBtnGroup = nullptr;
    int          toggledBtn = -1;
    int          previousToggledBtn = -1;

Q_SIGNALS:
     // 时间日期改变
    void dateTimeChanged(QDate date, QTime time);
    // 时间模式改变
    void timeModeChanged(int id, QString mode);
    // 修改时区按钮点击
    void timezoneButtonClicked();
    // NTP服务器修改
    void ntpServerChanged(QString server);
    // 添加时区按钮点击
    void addTimezoneButtonClicked();
    // 时区移除
    void timezoneRemoved(QString zone);
};

#endif // DATETIMEUI_H
