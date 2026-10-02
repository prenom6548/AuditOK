/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef SCHEDULEWIDGET_H
#define SCHEDULEWIDGET_H
#include <QApplication>
#include <QString>
#include <QPalette>
#include <QCheckBox>
#include <QDateEdit>
#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimeEdit>
#include <QButtonGroup>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QObject>
#include <QRadioButton>
#include <QTimeEdit>
#include <QDialog>
#include <QComboBox>
#include <QDebug>
#include <QGSettings>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QContextMenuEvent>
#include <QGraphicsDropShadowEffect>
#include <QRadioButton>
#include "schedulestruct.h"
#include "kysdk/applications/klunardateedit.h"
#include "kysdk/applications/kdialog.h"

//提醒规则
enum AlarmType
{
    Alarm_None, //从不
    Alarm_Begin, //日程开始时
    Alarm_5Min_Front, //提前5分钟
    Alarm_10Min_Front, //提前10分钟
    Alarm_15Min_Front, //提前15分钟
    Alarm_30Min_Front, //提前30分钟
    Alarm_1Hour_Front, //提前1小时
    Alarm_1Day_Front, //提前1天
    Alarm_2Day_Front, //提前2天
    Alarm_1Day9_Front, //1天前上午9点
    Alarm_Begin9_Front, //日程当天上午9点
    Alarm_2Day9_Front, //2天前上午9点
    Alarm_1Week9_Front, //1周前上午9点
};
//重复规则
enum RRuleType
{
    RRule_None, //从不
    RRule_Day, //每天
    RRule_Week, //每周
    RRule_Month, //每月
    RRule_Year, //每年
};

using namespace kdk;
class TextEdit;
class CSchceduleDlg : public KDialog
{
    Q_OBJECT
public:
    explicit CSchceduleDlg(QWidget *parent = 0);
    ~CSchceduleDlg() Q_DECL_OVERRIDE;
    //设置提醒规则
    AlarmType alarm;
    RRuleType rrule;

    //监听主题的Gsettings
    QGSettings * m_styleGsettings = nullptr;
    TextEdit *m_textEdit = nullptr;
    KLunarDateEdit *m_pBeginDateEdit = nullptr;
    KLunarDateEdit *m_pEndDateEdit = nullptr;
    int m_beginHour;
    int m_endHour;
    int m_beginMinute;
    int m_endMinute;
    int end_day;
    int m_height;
    int end_month;
    QString timeLong;
    QString timemodel;
    QString m_text;
    QString idd;
    QList<MarkInfo> m_markInfoList;
    QTime m_startTime;
    QTime m_endTime;

    QCheckBox *m_lunarcheckBox = nullptr;
    QLabel *m_lunarLabel = nullptr;

    QComboBox *cmb_beginTimeEdit = nullptr;
    QPushButton *btn_quxiao = nullptr;
    QPushButton *btn_queding = nullptr;

    bool isDark = false; //是否是深色
    bool isDefault = false; //是否是深浅

    QString getText();
    QString escapeUrlToHtmlTags(QString &text);
    void resetToDefault();

    void updateMarkText(const MarkInfo &info);
    void updateCrontabEntry();
    // 设置小三角起始位置;
    void setStartPos(int startX);
    void Setinfo(MarkInfo info);
    void setStartDate(const QDate &date);
    void setEndDate(const QDate &date); //用于后期使用
    // 设置中间区域widget;
    void setCenterWidget(QWidget *widget);
    void setStartTimeMinute(const QString startTime);
    void setEndTimeMinute(const QString endTime);
    void setStartTimeHour(const QString tartTime);
    void setEndTimeHour(const QString endTime);
    void setId(QString idd);
    void setFormat(QString dateString);
    QString  getFormat();
    void setStatusFromId(QString id);
    //更新数据库中日程信息
    void updateScheduleInfo(const MarkInfo &info);
    void setProperty(int type, bool isAllDay = true);
    void setAlarmType(AlarmType type);
    AlarmType getAlarmType();
    void setRRuleType(RRuleType type);
    RRuleType getRRuleType() ;
    static QPalette lightPalette(QWidget *widget);
    static void setFramePalette(QWidget  *widget, QPalette &pal);

signals:
    void signalViewtransparentFrame(int type);
    void addManyDaysSignal(const MarkInfo &info);
    void updateMarkInfoSignal(const MarkInfo &info);
    void cancelScheduleEditSignal();
    void updateInfo(MarkInfo info);
    void addToDatabase(const MarkInfo &info);
    void timeIntervalSelection(int startTime, int endTime);
    void sigWindowClosed();

protected:
    void keyPressEvent(QKeyEvent *event) override; //键盘事件
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *event) override;

private:
    void initWindow();
    void bindConnect();
    void initTooltipStyle();

private:
    KLunarDateEdit *m_pFrequencyTimeEdit = nullptr;
    QWidget *m_endrepeattimesWidget = nullptr;
    QWidget *m_endrepeatWidget = nullptr;
    QWidget *m_pMainWidget = nullptr;
    QWidget *rminWidget = nullptr;
    QWidget *repeatWidget = nullptr;
    QWidget *frequencyWidget = nullptr;
    QLabel *lbl_frequencyLabel = nullptr;
    QLabel *lbl_classLabel = nullptr;
    QLabel *lbl_remindSetLabel = nullptr;
    QLabel *lbl_beginrepeatLabel = nullptr;
    QLabel *lbl_beginTimeLabel = nullptr;
    QLabel *lbl_endTimeLabel = nullptr;
    QLabel *lbl_allDayLabel = nullptr;

    QCheckBox *chk_allDaycheckBox = nullptr;

    QComboBox *cmb_rmindCombox = nullptr;
    QComboBox *cmb_rmindComboxallday = nullptr;
    QComboBox *cmb_repeatCombox = nullptr;
    QComboBox *cmb_frequencyBox = nullptr;
    QComboBox *cmb_endTimeEdit = nullptr;

    QHBoxLayout *m_pTextContentlayout = nullptr;
    QHBoxLayout *lbl_allDayLabellayout = nullptr;
    QHBoxLayout *m_pBeginTimelayout = nullptr;
    QHBoxLayout *m_pEndTimelayout = nullptr;
    QWidget *buttonWidget = nullptr;
    QHBoxLayout *frequencyLayout = nullptr;
    QHBoxLayout *buttonLayout = nullptr;
    QDateEdit *m_endRepeatDate = nullptr;
    QVBoxLayout *vlayout = nullptr;
    QString m_context;
    QString TooltipStyle;
    QString textColor;
    QString backgroundColor;
    QString TooltipBorderColor;
    bool m_createAllDay;
    MarkInfo m_info;//标记信息
private:
    // 小三角起始位置;
    int m_startX;
    // 小三角的宽度;
    int m_triangleWidth;
    // 小三角高度;
    int m_triangleHeight;
    int m_type; // 1新建 0 编辑日程
    QGSettings *calendar_gsettings = nullptr;
    QGSettings *colorGsettings = nullptr;
    QGSettings * styleGsettings = nullptr;
    QDate m_date;
    bool tanchuang = false;
    bool isStartTimeChanged = false;
    bool m_showLunar = false;

    QStringList getLocale(QString &language, QString &locale);
    QString getRealText(const TextEdit *textEdit); //获取格式文本
    void addPeriodOfTime();
    void addRemindMe();
    void addRemindMeallday();
    void ReminderTime(const QString &startTime);
    void onRepeatComboxTextChanged(const QString &text);
    void onfrequencyComboxTextChanged(const QString &text);
    void updateEndTimeValidation();
    void validateEndTimeFormat1();
    void validateEndTimeFormat2();
    bool getShowLunar();


public slots:
    void setAllDaySlot(bool isChecked);
    void setRepeatSlot(bool isChecked);
    void updateEndTime(const QString &startTime);
    void slotTextChanged();
    void handleComboBoxTextChanged(const QString &text); //用于设置提醒时间
    void handleComboBoxTextChangedallday(const QString &text);
    void slotConfirmClick();
    void setStartTime(const QString &startTime);
    void setEndTime(const QString &endTime);
    void slotfrequencyBox(const QString &text);
    void updateEndDateFromBeginDate();
    void onPaletteChanged();
    void handleComboBoxDoubleClick(const QPoint &pos);
};

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}
    ~TextEdit() {}

protected:
    void contextMenuEvent(QContextMenuEvent *event)
    {
        QMenu *menu = createStandardContextMenu();//默认的标准右键菜单，如果不需要刻意完全自己实现
        menu->setPalette(this->palette());
        menu->exec(event->globalPos());
        delete menu;
    }
};

#endif // SCHEDULEWIDGET_H

